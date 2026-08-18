#include "storage/Database.h"

#include <QDir>
#include <QFileInfo>
#include <QSqlError>
#include <QSqlQuery>
#include <QUuid>

namespace opencaddie::storage {
namespace {

const QStringList Migration1{
    QStringLiteral(
        "CREATE TABLE IF NOT EXISTS profiles ("
        "id TEXT PRIMARY KEY, name TEXT NOT NULL, handicap REAL NOT NULL DEFAULT 0,"
        "is_default INTEGER NOT NULL DEFAULT 0, created_at TEXT NOT NULL)"),
    QStringLiteral(
        "CREATE TABLE IF NOT EXISTS clubs ("
        "id TEXT PRIMARY KEY, profile_id TEXT NOT NULL REFERENCES profiles(id) "
        "ON DELETE CASCADE, name TEXT NOT NULL, carry_metres REAL NOT NULL "
        "CHECK(carry_metres > 0), enabled INTEGER NOT NULL DEFAULT 1,"
        "position INTEGER NOT NULL DEFAULT 0, created_at TEXT NOT NULL,"
        "updated_at TEXT NOT NULL)"),
    QStringLiteral("CREATE INDEX IF NOT EXISTS clubs_profile_position "
                   "ON clubs(profile_id, position)"),
    QStringLiteral("CREATE TABLE IF NOT EXISTS cached_courses ("
                   "slug TEXT NOT NULL, version TEXT NOT NULL, name TEXT NOT NULL,"
                   "path TEXT NOT NULL, quality_score INTEGER NOT NULL DEFAULT 0,"
                   "byte_size INTEGER NOT NULL DEFAULT 0, attribution TEXT NOT NULL,"
                   "installed_at TEXT NOT NULL, last_used_at TEXT,"
                   "PRIMARY KEY(slug, version))"),
    QStringLiteral(
        "CREATE TABLE IF NOT EXISTS rounds ("
        "id TEXT PRIMARY KEY, course_slug TEXT NOT NULL, course_name TEXT NOT NULL,"
        "course_version TEXT, profile_id TEXT REFERENCES profiles(id),"
        "hole_count INTEGER NOT NULL CHECK(hole_count IN (9,18)),"
        "scoring_mode TEXT NOT NULL CHECK(scoring_mode IN ('stroke','stableford')),"
        "course_handicap INTEGER NOT NULL DEFAULT 0,"
        "status TEXT NOT NULL CHECK(status IN ('in_progress','completed','abandoned')),"
        "current_hole INTEGER NOT NULL DEFAULT 1, started_at TEXT NOT NULL,"
        "completed_at TEXT, tee TEXT NOT NULL DEFAULT '')"),
    QStringLiteral("CREATE INDEX IF NOT EXISTS rounds_status_started "
                   "ON rounds(status, started_at DESC)"),
    QStringLiteral("CREATE TABLE IF NOT EXISTS participants ("
                   "id TEXT PRIMARY KEY, round_id TEXT NOT NULL REFERENCES rounds(id) "
                   "ON DELETE CASCADE, profile_id TEXT REFERENCES profiles(id),"
                   "display_name TEXT NOT NULL, handicap INTEGER NOT NULL DEFAULT 0,"
                   "device_id TEXT, position INTEGER NOT NULL DEFAULT 0)"),
    QStringLiteral(
        "CREATE TABLE IF NOT EXISTS hole_scores ("
        "round_id TEXT NOT NULL REFERENCES rounds(id) ON DELETE CASCADE,"
        "participant_id TEXT NOT NULL REFERENCES participants(id) ON DELETE CASCADE,"
        "hole INTEGER NOT NULL CHECK(hole BETWEEN 1 AND 18),"
        "par INTEGER, stroke_index INTEGER, strokes INTEGER NOT NULL DEFAULT 0,"
        "putts INTEGER, penalties INTEGER NOT NULL DEFAULT 0,"
        "fairway TEXT, gir INTEGER, tee TEXT NOT NULL DEFAULT '',"
        "notes TEXT NOT NULL DEFAULT '', updated_at TEXT NOT NULL,"
        "PRIMARY KEY(round_id, participant_id, hole))"),
    QStringLiteral(
        "CREATE TABLE IF NOT EXISTS settings ("
        "key TEXT PRIMARY KEY, value TEXT NOT NULL, updated_at TEXT NOT NULL)"),
    QStringLiteral("CREATE TABLE IF NOT EXISTS course_metadata_overrides ("
                   "course_slug TEXT NOT NULL, hole INTEGER NOT NULL,"
                   "par INTEGER, stroke_index INTEGER, updated_at TEXT NOT NULL,"
                   "PRIMARY KEY(course_slug, hole))"),
    QStringLiteral(
        "CREATE TABLE IF NOT EXISTS outbox ("
        "id TEXT PRIMARY KEY, entity_type TEXT NOT NULL, entity_id TEXT NOT NULL,"
        "operation TEXT NOT NULL, payload_json TEXT NOT NULL,"
        "created_at TEXT NOT NULL, delivered_at TEXT)"),
};

const QStringList Migration2{
    QStringLiteral("ALTER TABLE profiles ADD COLUMN handicap_source TEXT NOT NULL "
                   "DEFAULT 'local'"),
    QStringLiteral("ALTER TABLE profiles ADD COLUMN handicap_updated_at TEXT"),
    QStringLiteral("ALTER TABLE rounds ADD COLUMN weather_temperature_c REAL"),
    QStringLiteral("ALTER TABLE rounds ADD COLUMN weather_wind_mps REAL"),
    QStringLiteral("ALTER TABLE rounds ADD COLUMN weather_wind_direction_deg INTEGER"),
    QStringLiteral("ALTER TABLE rounds ADD COLUMN weather_condition TEXT NOT NULL "
                   "DEFAULT ''"),
    QStringLiteral(
        "ALTER TABLE rounds ADD COLUMN weather_source TEXT NOT NULL DEFAULT ''"),
    QStringLiteral("ALTER TABLE rounds ADD COLUMN weather_recorded_at TEXT"),
    QStringLiteral("CREATE TABLE IF NOT EXISTS shots ("
                   "id TEXT PRIMARY KEY,"
                   "round_id TEXT NOT NULL REFERENCES rounds(id) ON DELETE CASCADE,"
                   "participant_id TEXT NOT NULL REFERENCES participants(id) "
                   "ON DELETE CASCADE,"
                   "hole INTEGER NOT NULL CHECK(hole BETWEEN 1 AND 18),"
                   "sequence INTEGER NOT NULL CHECK(sequence > 0),"
                   "club_id TEXT REFERENCES clubs(id) ON DELETE SET NULL,"
                   "club_name TEXT NOT NULL DEFAULT '',"
                   "shot_type TEXT NOT NULL DEFAULT '',"
                   "start_latitude REAL,start_longitude REAL,"
                   "end_latitude REAL,end_longitude REAL,"
                   "distance_metres REAL CHECK(distance_metres IS NULL OR "
                   "distance_metres >= 0),"
                   "lateral_metres REAL,accuracy_metres REAL,"
                   "result TEXT NOT NULL DEFAULT '',source_provider TEXT NOT NULL "
                   "DEFAULT 'opencaddie',external_id TEXT,recorded_at TEXT NOT NULL,"
                   "UNIQUE(round_id,participant_id,hole,sequence),"
                   "UNIQUE(source_provider,external_id))"),
    QStringLiteral("CREATE INDEX IF NOT EXISTS shots_round_hole "
                   "ON shots(round_id,participant_id,hole,sequence)"),
    QStringLiteral("CREATE TABLE IF NOT EXISTS integration_accounts ("
                   "provider TEXT PRIMARY KEY,"
                   "status TEXT NOT NULL CHECK(status IN "
                   "('unavailable','available','connected','error')),"
                   "external_user_id TEXT,display_name TEXT,"
                   "capabilities_json TEXT NOT NULL DEFAULT '[]',"
                   "last_sync_at TEXT,last_error TEXT NOT NULL DEFAULT '',"
                   "updated_at TEXT NOT NULL)"),
    QStringLiteral("CREATE TABLE IF NOT EXISTS external_round_links ("
                   "provider TEXT NOT NULL,"
                   "external_id TEXT NOT NULL,"
                   "round_id TEXT NOT NULL REFERENCES rounds(id) ON DELETE CASCADE,"
                   "sync_state TEXT NOT NULL DEFAULT 'imported',"
                   "last_sync_at TEXT NOT NULL,"
                   "PRIMARY KEY(provider,external_id),"
                   "UNIQUE(provider,round_id))"),
};

const QStringList Migration3{
    QStringLiteral("CREATE TABLE IF NOT EXISTS shot_metrics ("
                   "shot_id TEXT NOT NULL REFERENCES shots(id) ON DELETE CASCADE,"
                   "metric_key TEXT NOT NULL,"
                   "canonical_value REAL NOT NULL,"
                   "canonical_unit TEXT NOT NULL,"
                   "source_value REAL,"
                   "source_unit TEXT NOT NULL DEFAULT '',"
                   "PRIMARY KEY(shot_id,metric_key))"),
};

const QStringList Migration4{
    QStringLiteral("CREATE INDEX IF NOT EXISTS rounds_profile_status_course_started "
                   "ON rounds(profile_id,status,course_slug,started_at DESC)"),
    QStringLiteral(
        "CREATE TRIGGER IF NOT EXISTS shots_identity_insert "
        "BEFORE INSERT ON shots "
        "WHEN NOT EXISTS(SELECT 1 FROM participants p "
        "WHERE p.id=NEW.participant_id AND p.round_id=NEW.round_id) "
        "OR NEW.hole>COALESCE((SELECT r.hole_count FROM rounds r "
        "WHERE r.id=NEW.round_id),0) "
        "BEGIN SELECT RAISE(ABORT,'shot identity does not match round'); END"),
    QStringLiteral(
        "CREATE TRIGGER IF NOT EXISTS shots_identity_update "
        "BEFORE UPDATE OF round_id,participant_id,hole ON shots "
        "WHEN NOT EXISTS(SELECT 1 FROM participants p "
        "WHERE p.id=NEW.participant_id AND p.round_id=NEW.round_id) "
        "OR NEW.hole>COALESCE((SELECT r.hole_count FROM rounds r "
        "WHERE r.id=NEW.round_id),0) "
        "BEGIN SELECT RAISE(ABORT,'shot identity does not match round'); END"),
};

const QStringList Migration5{
    QStringLiteral(
        "ALTER TABLE rounds ADD COLUMN handicap_index_scale INTEGER NOT NULL "
        "DEFAULT 0 CHECK(handicap_index_scale IN (0,9,18))"),
    QStringLiteral("UPDATE rounds SET handicap_index_scale=18 WHERE hole_count=18"),
    QStringLiteral(
        "CREATE TRIGGER IF NOT EXISTS hole_scores_identity_insert "
        "BEFORE INSERT ON hole_scores "
        "WHEN NOT EXISTS(SELECT 1 FROM participants p "
        "WHERE p.id=NEW.participant_id AND p.round_id=NEW.round_id) "
        "OR NEW.hole>COALESCE((SELECT r.hole_count FROM rounds r "
        "WHERE r.id=NEW.round_id),0) "
        "BEGIN SELECT RAISE(ABORT,'score identity does not match round'); END"),
    QStringLiteral(
        "CREATE TRIGGER IF NOT EXISTS hole_scores_identity_update "
        "BEFORE UPDATE OF round_id,participant_id,hole ON hole_scores "
        "WHEN NOT EXISTS(SELECT 1 FROM participants p "
        "WHERE p.id=NEW.participant_id AND p.round_id=NEW.round_id) "
        "OR NEW.hole>COALESCE((SELECT r.hole_count FROM rounds r "
        "WHERE r.id=NEW.round_id),0) "
        "BEGIN SELECT RAISE(ABORT,'score identity does not match round'); END"),
};

const QStringList Migration6{
    QStringLiteral(
        "CREATE TABLE IF NOT EXISTS course_analysis_layups ("
        "course_slug TEXT NOT NULL,hole INTEGER NOT NULL "
        "CHECK(hole BETWEEN 1 AND 18),sequence INTEGER NOT NULL "
        "CHECK(sequence > 0),x REAL NOT NULL,y REAL NOT NULL,"
        "updated_at TEXT NOT NULL,PRIMARY KEY(course_slug,hole,sequence))"),
    QStringLiteral(
        "CREATE TABLE IF NOT EXISTS round_layups ("
        "round_id TEXT NOT NULL REFERENCES rounds(id) ON DELETE CASCADE,"
        "hole INTEGER NOT NULL CHECK(hole BETWEEN 1 AND 18),"
        "sequence INTEGER NOT NULL CHECK(sequence > 0),"
        "x REAL NOT NULL,y REAL NOT NULL,"
        "PRIMARY KEY(round_id,hole,sequence))"),
    QStringLiteral("CREATE INDEX IF NOT EXISTS course_analysis_layups_course_hole "
                   "ON course_analysis_layups(course_slug,hole,sequence)"),
    QStringLiteral("CREATE INDEX IF NOT EXISTS round_layups_round_hole "
                   "ON round_layups(round_id,hole,sequence)"),
};

const QStringList Migration7{
    QStringLiteral(
        "ALTER TABLE clubs ADD COLUMN club_type TEXT NOT NULL DEFAULT 'other' "
        "CHECK(club_type IN ('driver','wood','hybrid','iron','wedge','putter',"
        "'other'))"),
    QStringLiteral(
        "UPDATE clubs SET club_type=CASE "
        "WHEN lower(trim(name)) LIKE '%putter%' THEN 'putter' "
        "WHEN lower(trim(name)) LIKE '%driver%' THEN 'driver' "
        "WHEN lower(trim(name)) LIKE '%hybrid%' "
        "OR lower(trim(name)) LIKE '%rescue%' THEN 'hybrid' "
        "WHEN lower(trim(name)) LIKE '%wood%' THEN 'wood' "
        "WHEN lower(trim(name)) LIKE '%wedge%' "
        "OR lower(trim(name)) IN ('pw','gw','aw','sw','lw') THEN 'wedge' "
        "WHEN lower(trim(name)) LIKE '%iron%' "
        "OR lower(trim(name)) LIKE '%jern%' THEN 'iron' "
        "ELSE 'other' END"),
};

constexpr int LatestSchemaVersion = 7;

} // namespace

Database::Database()
    : m_connectionName(QStringLiteral("opencaddie-%1")
                           .arg(QUuid::createUuid().toString(QUuid::WithoutBraces))) {}

Database::~Database() { close(); }

bool Database::open(const QString &path) {
    close();
    const QFileInfo info(path);
    if (!QDir().mkpath(info.absolutePath())) {
        m_lastError = QStringLiteral("Could not create database directory");
        return false;
    }

    auto database =
        QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), m_connectionName);
    database.setDatabaseName(path);
    database.setConnectOptions(QStringLiteral("QSQLITE_BUSY_TIMEOUT=5000"));
    if (!database.open()) {
        m_lastError = database.lastError().text();
        return false;
    }
    if (!execute(QStringLiteral("PRAGMA foreign_keys=ON")) ||
        !execute(QStringLiteral("PRAGMA journal_mode=WAL")) ||
        !execute(QStringLiteral("PRAGMA synchronous=FULL")) ||
        !execute(QStringLiteral("PRAGMA busy_timeout=5000"))) {
        return false;
    }
    return applyMigrations();
}

void Database::close() {
    if (!QSqlDatabase::contains(m_connectionName)) {
        return;
    }
    {
        auto database = QSqlDatabase::database(m_connectionName, false);
        if (database.isValid()) {
            database.close();
        }
    }
    QSqlDatabase::removeDatabase(m_connectionName);
}

bool Database::isOpen() const {
    return QSqlDatabase::contains(m_connectionName) &&
           QSqlDatabase::database(m_connectionName, false).isOpen();
}

QSqlDatabase Database::connection() const {
    return QSqlDatabase::database(m_connectionName, false);
}

QString Database::lastError() const { return m_lastError; }

int Database::schemaVersion() const {
    QSqlQuery query(connection());
    if (!query.exec(QStringLiteral("PRAGMA user_version")) || !query.next()) {
        return 0;
    }
    return query.value(0).toInt();
}

bool Database::applyMigrations() {
    auto database = connection();
    int version = schemaVersion();
    if (version > LatestSchemaVersion) {
        m_lastError = QStringLiteral("Database schema is newer than this build");
        return false;
    }
    if (version == LatestSchemaVersion) {
        return true;
    }

    if (!database.transaction()) {
        return setError(QStringLiteral("Could not start migration"));
    }
    if (version < 1) {
        for (const auto &statement : Migration1) {
            if (!execute(statement)) {
                database.rollback();
                return false;
            }
        }
        version = 1;
    }
    if (version < 2) {
        for (const auto &statement : Migration2) {
            if (!execute(statement)) {
                database.rollback();
                return false;
            }
        }
        version = 2;
    }
    if (version < 3) {
        for (const auto &statement : Migration3) {
            if (!execute(statement)) {
                database.rollback();
                return false;
            }
        }
        version = 3;
    }
    if (version < 4) {
        for (const auto &statement : Migration4) {
            if (!execute(statement)) {
                database.rollback();
                return false;
            }
        }
        version = 4;
    }
    if (version < 5) {
        for (const auto &statement : Migration5) {
            if (!execute(statement)) {
                database.rollback();
                return false;
            }
        }
        version = 5;
    }
    if (version < 6) {
        for (const auto &statement : Migration6) {
            if (!execute(statement)) {
                database.rollback();
                return false;
            }
        }
        version = 6;
    }
    if (version < 7) {
        for (const auto &statement : Migration7) {
            if (!execute(statement)) {
                database.rollback();
                return false;
            }
        }
    }
    if (!execute(QStringLiteral("PRAGMA user_version=%1").arg(LatestSchemaVersion))) {
        database.rollback();
        return false;
    }
    if (!database.commit()) {
        return setError(QStringLiteral("Could not commit migration"));
    }
    return true;
}

bool Database::execute(const QString &sql) {
    QSqlQuery query(connection());
    if (!query.exec(sql)) {
        m_lastError = QStringLiteral("%1: %2").arg(query.lastError().text(), sql);
        return false;
    }
    return true;
}

bool Database::setError(const QString &context) {
    m_lastError =
        QStringLiteral("%1: %2").arg(context, connection().lastError().text());
    return false;
}

} // namespace opencaddie::storage
