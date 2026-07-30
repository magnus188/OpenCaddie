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
    QStringLiteral(
        "CREATE INDEX IF NOT EXISTS clubs_profile_position "
        "ON clubs(profile_id, position)"),
    QStringLiteral(
        "CREATE TABLE IF NOT EXISTS cached_courses ("
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
    QStringLiteral(
        "CREATE INDEX IF NOT EXISTS rounds_status_started "
        "ON rounds(status, started_at DESC)"),
    QStringLiteral(
        "CREATE TABLE IF NOT EXISTS participants ("
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
    QStringLiteral(
        "CREATE TABLE IF NOT EXISTS course_metadata_overrides ("
        "course_slug TEXT NOT NULL, hole INTEGER NOT NULL,"
        "par INTEGER, stroke_index INTEGER, updated_at TEXT NOT NULL,"
        "PRIMARY KEY(course_slug, hole))"),
    QStringLiteral(
        "CREATE TABLE IF NOT EXISTS outbox ("
        "id TEXT PRIMARY KEY, entity_type TEXT NOT NULL, entity_id TEXT NOT NULL,"
        "operation TEXT NOT NULL, payload_json TEXT NOT NULL,"
        "created_at TEXT NOT NULL, delivered_at TEXT)"),
};

} // namespace

Database::Database()
    : m_connectionName(
          QStringLiteral("opencaddie-%1").arg(QUuid::createUuid().toString(
              QUuid::WithoutBraces))) {}

Database::~Database() { close(); }

bool Database::open(const QString& path) {
    close();
    const QFileInfo info(path);
    if (!QDir().mkpath(info.absolutePath())) {
        m_lastError = QStringLiteral("Could not create database directory");
        return false;
    }

    auto database = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"),
                                               m_connectionName);
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
    if (version > 1) {
        m_lastError = QStringLiteral("Database schema is newer than this build");
        return false;
    }
    if (version == 1) {
        return true;
    }

    if (!database.transaction()) {
        return setError(QStringLiteral("Could not start migration"));
    }
    for (const auto& statement : Migration1) {
        if (!execute(statement)) {
            database.rollback();
            return false;
        }
    }
    if (!execute(QStringLiteral("PRAGMA user_version=1"))) {
        database.rollback();
        return false;
    }
    if (!database.commit()) {
        return setError(QStringLiteral("Could not commit migration"));
    }
    return true;
}

bool Database::execute(const QString& sql) {
    QSqlQuery query(connection());
    if (!query.exec(sql)) {
        m_lastError =
            QStringLiteral("%1: %2").arg(query.lastError().text(), sql);
        return false;
    }
    return true;
}

bool Database::setError(const QString& context) {
    m_lastError = QStringLiteral("%1: %2")
                      .arg(context, connection().lastError().text());
    return false;
}

} // namespace opencaddie::storage

