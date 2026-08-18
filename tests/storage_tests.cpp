#include "storage/Database.h"
#include "storage/Repositories.h"

#include <QCoreApplication>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QTemporaryDir>
#include <QUuid>

#include <cstdlib>
#include <iostream>
#include <limits>
#include <optional>

namespace {

bool createLegacyV1Database(const QString &path) {
    const QString connectionName =
        QStringLiteral("legacy-v1-%1")
            .arg(QUuid::createUuid().toString(QUuid::WithoutBraces));
    bool success = true;
    {
        auto database =
            QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), connectionName);
        database.setDatabaseName(path);
        if (!database.open()) {
            std::cerr << database.lastError().text().toStdString() << '\n';
            success = false;
        }

        const QStringList statements{
            QStringLiteral(
                "CREATE TABLE profiles (id TEXT PRIMARY KEY,name TEXT NOT NULL,"
                "handicap REAL NOT NULL DEFAULT 0,is_default INTEGER NOT NULL "
                "DEFAULT 0,created_at TEXT NOT NULL)"),
            QStringLiteral(
                "CREATE TABLE clubs (id TEXT PRIMARY KEY,profile_id TEXT NOT NULL "
                "REFERENCES profiles(id),name TEXT NOT NULL,carry_metres REAL "
                "NOT NULL,enabled INTEGER NOT NULL DEFAULT 1,position INTEGER "
                "NOT NULL DEFAULT 0,created_at TEXT NOT NULL,updated_at TEXT NOT "
                "NULL)"),
            QStringLiteral(
                "CREATE TABLE rounds (id TEXT PRIMARY KEY,course_slug TEXT NOT NULL,"
                "course_name TEXT NOT NULL,course_version TEXT,profile_id TEXT "
                "REFERENCES profiles(id),hole_count INTEGER NOT NULL,"
                "scoring_mode TEXT NOT NULL,course_handicap INTEGER NOT NULL "
                "DEFAULT 0,status TEXT NOT NULL,current_hole INTEGER NOT NULL "
                "DEFAULT 1,started_at TEXT NOT NULL,completed_at TEXT,tee TEXT "
                "NOT NULL DEFAULT '')"),
            QStringLiteral(
                "CREATE TABLE participants (id TEXT PRIMARY KEY,round_id TEXT NOT "
                "NULL REFERENCES rounds(id),profile_id TEXT REFERENCES profiles(id),"
                "display_name TEXT NOT NULL,handicap INTEGER NOT NULL DEFAULT 0,"
                "device_id TEXT,position INTEGER NOT NULL DEFAULT 0)"),
            QStringLiteral(
                "CREATE TABLE hole_scores (round_id TEXT NOT NULL REFERENCES "
                "rounds(id),participant_id TEXT NOT NULL REFERENCES participants(id),"
                "hole INTEGER NOT NULL,par INTEGER,stroke_index INTEGER,strokes "
                "INTEGER NOT NULL DEFAULT 0,putts INTEGER,penalties INTEGER NOT "
                "NULL DEFAULT 0,fairway TEXT,gir INTEGER,tee TEXT NOT NULL DEFAULT '',"
                "notes TEXT NOT NULL DEFAULT '',updated_at TEXT NOT NULL,"
                "PRIMARY KEY(round_id,participant_id,hole))"),
            QStringLiteral(
                "CREATE TABLE cached_courses (slug TEXT NOT NULL,version TEXT "
                "NOT NULL,name TEXT NOT NULL,path TEXT NOT NULL,quality_score "
                "INTEGER NOT NULL DEFAULT 0,byte_size INTEGER NOT NULL DEFAULT 0,"
                "attribution TEXT NOT NULL,installed_at TEXT NOT NULL,"
                "last_used_at TEXT,PRIMARY KEY(slug,version))"),
            QStringLiteral(
                "CREATE TABLE settings (key TEXT PRIMARY KEY,value TEXT NOT NULL,"
                "updated_at TEXT NOT NULL)"),
            QStringLiteral(
                "CREATE TABLE course_metadata_overrides (course_slug TEXT NOT "
                "NULL,hole INTEGER NOT NULL,par INTEGER,stroke_index INTEGER,"
                "updated_at TEXT NOT NULL,PRIMARY KEY(course_slug,hole))"),
            QStringLiteral(
                "CREATE TABLE outbox (id TEXT PRIMARY KEY,entity_type TEXT NOT "
                "NULL,entity_id TEXT NOT NULL,operation TEXT NOT NULL,"
                "payload_json TEXT NOT NULL,created_at TEXT NOT NULL,"
                "delivered_at TEXT)"),
            QStringLiteral(
                "INSERT INTO profiles VALUES('legacy-profile','Legacy player',"
                "14.2,1,'2025-01-01T10:00:00Z')"),
            QStringLiteral(
                "INSERT INTO clubs VALUES('legacy-driver','legacy-profile',"
                "'Driver',215,1,0,'2025-01-01T10:00:00Z',"
                "'2025-01-01T10:00:00Z')"),
            QStringLiteral(
                "INSERT INTO clubs VALUES('legacy-wood','legacy-profile',"
                "'5 wood',190,1,1,'2025-01-01T10:00:00Z',"
                "'2025-01-01T10:00:00Z')"),
            QStringLiteral(
                "INSERT INTO clubs VALUES('legacy-hybrid','legacy-profile',"
                "'4 hybrid',180,1,2,'2025-01-01T10:00:00Z',"
                "'2025-01-01T10:00:00Z')"),
            QStringLiteral(
                "INSERT INTO clubs VALUES('legacy-iron','legacy-profile',"
                "'7 iron',145,1,3,'2025-01-01T10:00:00Z',"
                "'2025-01-01T10:00:00Z')"),
            QStringLiteral(
                "INSERT INTO clubs VALUES('legacy-wedge','legacy-profile',"
                "'Sand wedge',75,1,4,'2025-01-01T10:00:00Z',"
                "'2025-01-01T10:00:00Z')"),
            QStringLiteral(
                "INSERT INTO clubs VALUES('legacy-putter','legacy-profile',"
                "'Putter',10,1,5,'2025-01-01T10:00:00Z',"
                "'2025-01-01T10:00:00Z')"),
            QStringLiteral(
                "INSERT INTO clubs VALUES('legacy-other','legacy-profile',"
                "'Old faithful',120,1,6,'2025-01-01T10:00:00Z',"
                "'2025-01-01T10:00:00Z')"),
            QStringLiteral("INSERT INTO rounds VALUES('legacy-round','legacy-course',"
                           "'Legacy Golf Club','v1','legacy-profile',18,'stroke',14,"
                           "'completed',18,'2025-01-01T10:00:00Z',"
                           "'2025-01-01T14:00:00Z','Yellow')"),
            QStringLiteral(
                "INSERT INTO participants VALUES('legacy-participant',"
                "'legacy-round','legacy-profile','Legacy player',14,NULL,0)"),
            QStringLiteral("INSERT INTO hole_scores VALUES('legacy-round',"
                           "'legacy-participant',1,4,1,5,2,0,'centre',0,'Yellow','',"
                           "'2025-01-01T10:15:00Z')"),
            QStringLiteral("PRAGMA user_version=1"),
        };

        if (success) {
            QSqlQuery query(database);
            for (const auto &statement : statements) {
                if (!query.exec(statement)) {
                    std::cerr << "Could not create legacy fixture: "
                              << query.lastError().text().toStdString() << '\n';
                    success = false;
                    break;
                }
            }
        }
        database.close();
    }
    QSqlDatabase::removeDatabase(connectionName);
    return success;
}

bool createLegacyV2Database(const QString &path) {
    const QString connectionName =
        QStringLiteral("legacy-v2-%1")
            .arg(QUuid::createUuid().toString(QUuid::WithoutBraces));
    bool success = true;
    {
        auto database =
            QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), connectionName);
        database.setDatabaseName(path);
        if (!database.open()) {
            success = false;
        } else {
            QSqlQuery query(database);
            success =
                query.exec(QStringLiteral(
                    "CREATE TABLE rounds(id TEXT PRIMARY KEY,profile_id TEXT,"
                    "status TEXT,course_slug TEXT,started_at TEXT,"
                    "hole_count INTEGER)")) &&
                query.exec(QStringLiteral(
                    "CREATE TABLE clubs(id TEXT PRIMARY KEY,profile_id TEXT,"
                    "name TEXT NOT NULL,carry_metres REAL NOT NULL,"
                    "enabled INTEGER NOT NULL DEFAULT 1,"
                    "position INTEGER NOT NULL DEFAULT 0,created_at TEXT NOT NULL,"
                    "updated_at TEXT NOT NULL)")) &&
                query.exec(
                    QStringLiteral("CREATE TABLE participants(id TEXT PRIMARY KEY,"
                                   "round_id TEXT)")) &&
                query.exec(QStringLiteral("CREATE TABLE hole_scores(round_id TEXT,"
                                          "participant_id TEXT,hole INTEGER)")) &&
                query.exec(QStringLiteral(
                    "CREATE TABLE shots(id TEXT PRIMARY KEY,round_id TEXT,"
                    "participant_id TEXT,hole INTEGER)")) &&
                query.exec(QStringLiteral(
                    "INSERT INTO rounds VALUES('v2-round','v2-profile',"
                    "'completed','v2-course','2025-01-01T10:00:00Z',18)")) &&
                query.exec(
                    QStringLiteral("INSERT INTO participants VALUES('v2-participant',"
                                   "'v2-round')")) &&
                query.exec(QStringLiteral(
                    "INSERT INTO shots VALUES('preserved-v2-shot','v2-round',"
                    "'v2-participant',1)")) &&
                query.exec(QStringLiteral("PRAGMA user_version=2"));
            if (!success) {
                std::cerr << "Could not create v2 fixture: "
                          << query.lastError().text().toStdString() << '\n';
            }
        }
        database.close();
    }
    QSqlDatabase::removeDatabase(connectionName);
    return success;
}

int distributionCount(const QVariantMap &statistics, const QString &list,
                      const QString &key) {
    for (const auto &value : statistics.value(list).toList()) {
        const auto row = value.toMap();
        if (row.value(QStringLiteral("key")).toString() == key) {
            return row.value(QStringLiteral("count")).toInt();
        }
    }
    return 0;
}

int scoreCount(const QVariantMap &statistics, const QString &key) {
    return distributionCount(statistics, QStringLiteral("scoreDistribution"), key);
}

} // namespace

int main(int argc, char **argv) {
    QCoreApplication application(argc, argv);
    QTemporaryDir directory;
    if (!directory.isValid()) {
        std::cerr << "Could not create temporary directory\n";
        return EXIT_FAILURE;
    }

    const QString databasePath = directory.filePath(QStringLiteral("test.sqlite"));
    opencaddie::storage::Database database;
    if (!database.open(databasePath)) {
        std::cerr << database.lastError().toStdString() << '\n';
        return EXIT_FAILURE;
    }
    if (database.schemaVersion() != 7) {
        std::cerr << "Migration version mismatch\n";
        return EXIT_FAILURE;
    }

    {
        const QString starterPath =
            directory.filePath(QStringLiteral("starter.sqlite"));
        opencaddie::storage::Database starterDatabase;
        if (!starterDatabase.open(starterPath))
            return EXIT_FAILURE;
        opencaddie::storage::ClubRepository starterClubs(
            starterDatabase.connection());
        if (!starterClubs.ensureDefaultProfile() ||
            !starterClubs.ensureStarterBag()) {
            return EXIT_FAILURE;
        }
        const auto values =
            starterClubs.list(starterClubs.defaultProfileId());
        using opencaddie::domain::ClubType;
        if (values.size() != 6 || values.at(0).type != ClubType::Driver ||
            values.at(1).type != ClubType::Iron ||
            values.at(3).type != ClubType::Wedge ||
            values.at(5).type != ClubType::Putter) {
            std::cerr << "Starter bag club types were not persisted\n";
            return EXIT_FAILURE;
        }
        QSqlQuery emptyStarter(starterDatabase.connection());
        if (!emptyStarter.exec(QStringLiteral("DELETE FROM clubs")) ||
            !starterClubs.ensureStarterBag() ||
            !starterClubs.list(starterClubs.defaultProfileId()).empty()) {
            std::cerr << "An intentionally empty bag was repopulated\n";
            return EXIT_FAILURE;
        }
    }

    {
        opencaddie::storage::SettingsRepository settings(database.connection());
        if (!settings.setValue(QStringLiteral("courseHandicap"),
                               QStringLiteral("17")) ||
            settings.value(QStringLiteral("courseHandicap")) !=
                QStringLiteral("17")) {
            std::cerr << "Profile handicap setting was not persisted\n";
            return EXIT_FAILURE;
        }
    }

    QString profile;
    {
        opencaddie::storage::ClubRepository clubs(database.connection());
        if (!clubs.ensureDefaultProfile())
            return EXIT_FAILURE;
        profile = clubs.defaultProfileId();
        using opencaddie::domain::ClubType;
        const auto clubId = clubs.create(profile, QStringLiteral("7 iron"), 145.0,
                                         ClubType::Iron, false);
        auto storedClubs = clubs.list(profile);
        if (clubId.isEmpty() || storedClubs.size() != 1 ||
            storedClubs.front().type != ClubType::Iron ||
            storedClubs.front().enabled) {
            return EXIT_FAILURE;
        }
        storedClubs.front().type = ClubType::Hybrid;
        storedClubs.front().enabled = true;
        if (!clubs.update(storedClubs.front())) {
            return EXIT_FAILURE;
        }
        storedClubs = clubs.list(profile);
        if (storedClubs.front().type != ClubType::Hybrid ||
            !storedClubs.front().enabled) {
            return EXIT_FAILURE;
        }

        opencaddie::storage::RoundRepository rounds(database.connection());
        opencaddie::storage::RoundStart start;
        start.courseSlug = QStringLiteral("fixture");
        start.courseName = QStringLiteral("Fixture Golf Club");
        start.courseVersion = QStringLiteral("v1");
        start.profileId = profile;
        start.holeCount = 18;
        start.scoringMode = opencaddie::domain::ScoringMode::Stableford;
        start.courseHandicap = 12;
        start.tee = QStringLiteral("Yellow");
        start.handicapIndexScale = 18;
        start.weatherTemperatureC = 17.0;
        start.weatherWindMps = 4.2;
        start.weatherCondition = QStringLiteral("Partly cloudy");
        start.weatherSource = QStringLiteral("simulator");
        const auto round = rounds.start(start);
        if (!round)
            return EXIT_FAILURE;

        opencaddie::storage::CourseAnalysisRepository analyses(
            database.connection());
        const QVariantList analyzedRoute{
            QVariantMap{{QStringLiteral("x"), 12.0},
                        {QStringLiteral("y"), 24.0}},
            QVariantMap{{QStringLiteral("x"), 92.5},
                        {QStringLiteral("y"), 128.0}},
            QVariantMap{{QStringLiteral("x"), 145.0},
                        {QStringLiteral("y"), 201.5}},
        };
        if (!analyses.saveLayups(QStringLiteral("fixture"), 1, analyzedRoute) ||
            analyses.analyzedHoleCount(QStringLiteral("fixture")) != 1 ||
            analyses.layups(QStringLiteral("fixture"), 1) != analyzedRoute ||
            !analyses.importToRound(QStringLiteral("fixture"), round->id, 18) ||
            analyses.roundLayups(round->id, 1) != analyzedRoute ||
            analyses.importToRound(QStringLiteral("other-course"), round->id,
                                   18)) {
            std::cerr << "Course analysis persistence/import failed\n";
            return EXIT_FAILURE;
        }
        const QVariantList changedRoute{
            QVariantMap{{QStringLiteral("x"), 20.0},
                        {QStringLiteral("y"), 30.0}},
            QVariantMap{{QStringLiteral("x"), 100.0},
                        {QStringLiteral("y"), 140.0}},
        };
        if (!analyses.saveLayups(QStringLiteral("fixture"), 1, changedRoute) ||
            analyses.roundLayups(round->id, 1) != analyzedRoute) {
            std::cerr << "Imported analysis was not snapshotted for the round\n";
            return EXIT_FAILURE;
        }
        const QVariantList invalidRoute{
            QVariantMap{{QStringLiteral("x"),
                         std::numeric_limits<double>::infinity()},
                        {QStringLiteral("y"), 30.0}},
        };
        if (analyses.saveLayups(QStringLiteral("fixture"), 1, invalidRoute) ||
            analyses.layups(QStringLiteral("fixture"), 1) != changedRoute) {
            std::cerr << "Invalid course analysis was accepted or corrupted data\n";
            return EXIT_FAILURE;
        }

        using opencaddie::domain::FairwayResult;
        if (!rounds.saveScore(*round, {1, 4, 7},
                              {.hole = 1,
                               .strokes = 5,
                               .putts = 2,
                               .fairway = FairwayResult::Centre,
                               .greenInRegulation = false}) ||
            !rounds.saveScore(*round, {2, 4, 3},
                              {.hole = 2,
                               .strokes = 3,
                               .putts = 1,
                               .fairway = FairwayResult::Centre,
                               .greenInRegulation = true}) ||
            !rounds.saveScore(*round, {3, 5, 1},
                              {.hole = 3,
                               .strokes = 5,
                               .putts = 2,
                               .fairway = FairwayResult::Left,
                               .greenInRegulation = true}) ||
            !rounds.saveScore(*round, {4, 5, 9},
                              {.hole = 4,
                               .strokes = 3,
                               .putts = 1,
                               .fairway = FairwayResult::Centre,
                               .greenInRegulation = true})) {
            return EXIT_FAILURE;
        }

        opencaddie::storage::ShotRepository shots(database.connection());
        opencaddie::storage::ShotRecord drive;
        drive.roundId = round->id;
        drive.participantId = round->participantId;
        drive.hole = 1;
        drive.sequence = 1;
        drive.clubName = QStringLiteral("Driver");
        drive.shotType = QStringLiteral("drive");
        drive.distanceMetres = 242.5;
        drive.accuracyMetres = 4.0;
        drive.sourceProvider = QStringLiteral("trackman_csv");
        drive.externalId = QStringLiteral("fixture-drive-1");
        drive.recordedAt = QStringLiteral("2026-07-01T10:00:00.000Z");
        drive.metrics = {
            {.key = QStringLiteral("ball_speed"),
             .canonicalValue = 68.2,
             .canonicalUnit = QStringLiteral("m/s"),
             .sourceValue = 152.6,
             .sourceUnit = QStringLiteral("mph")},
            {.key = QStringLiteral("spin_rate"),
             .canonicalValue = 2'240.0,
             .canonicalUnit = QStringLiteral("rpm")},
        };
        drive.replaceMetrics = true;
        if (!shots.upsert(drive)) {
            std::cerr << "Initial shot insert failed\n";
            return EXIT_FAILURE;
        }
        drive.recordedAt.clear();
        if (!shots.upsert(drive)) {
            std::cerr << "Repeated shot upsert failed\n";
            return EXIT_FAILURE;
        }
        QSqlQuery metricCount(database.connection());
        if (!metricCount.exec(QStringLiteral(
                "SELECT COUNT(*),MIN(s.recorded_at),MAX(s.recorded_at) "
                "FROM shot_metrics sm JOIN shots s ON s.id=sm.shot_id")) ||
            !metricCount.next() || metricCount.value(0).toInt() != 2 ||
            metricCount.value(1).toString() !=
                QStringLiteral("2026-07-01T10:00:00.000Z") ||
            metricCount.value(2).toString() !=
                QStringLiteral("2026-07-01T10:00:00.000Z")) {
            std::cerr << "Shot metrics were not persisted idempotently\n";
            return EXIT_FAILURE;
        }
        metricCount.finish();

        auto clearedMetrics = drive;
        clearedMetrics.metrics.clear();
        clearedMetrics.replaceMetrics = true;
        if (!shots.upsert(clearedMetrics) ||
            !metricCount.exec(QStringLiteral("SELECT COUNT(*) FROM shot_metrics")) ||
            !metricCount.next() || metricCount.value(0).toInt() != 0) {
            std::cerr << "Authoritative metric removal failed\n";
            return EXIT_FAILURE;
        }
        metricCount.finish();

        auto wrongParticipant = drive;
        wrongParticipant.participantId = QStringLiteral("other-participant");
        wrongParticipant.sequence = 2;
        wrongParticipant.externalId = QStringLiteral("wrong-participant");
        if (shots.upsert(wrongParticipant)) {
            std::cerr << "Cross-round shot identity was accepted\n";
            return EXIT_FAILURE;
        }
        auto malformedShot = drive;
        malformedShot.sequence = 2;
        malformedShot.externalId = QStringLiteral("malformed-shot");
        malformedShot.distanceMetres = std::numeric_limits<double>::infinity();
        if (shots.upsert(malformedShot)) {
            std::cerr << "Infinite shot distance was accepted\n";
            return EXIT_FAILURE;
        }
        malformedShot.distanceMetres = 200.0;
        malformedShot.accuracyMetres = -1.0;
        if (shots.upsert(malformedShot)) {
            std::cerr << "Negative shot accuracy was accepted\n";
            return EXIT_FAILURE;
        }
        malformedShot.accuracyMetres = 2.0;
        malformedShot.startLatitude = 91.0;
        malformedShot.startLongitude = 10.0;
        if (shots.upsert(malformedShot)) {
            std::cerr << "Out-of-range shot coordinate was accepted\n";
            return EXIT_FAILURE;
        }

        const auto scoreForHole = [&rounds, &round](const int hole)
            -> std::optional<opencaddie::domain::HoleScore> {
            for (const auto &score : rounds.scores(*round)) {
                if (score.hole == hole)
                    return score;
            }
            return std::nullopt;
        };
        opencaddie::storage::ShotRecord tracked;
        tracked.roundId = round->id;
        tracked.participantId = round->participantId;
        tracked.hole = 4;
        tracked.sequence = 2;
        tracked.clubId = clubId;
        tracked.clubName = QStringLiteral("7 iron");
        tracked.shotType = QStringLiteral("drive");
        if (shots.appendTrackedStroke(tracked, *round, {4, 5, 9}) ||
            !shots.list(round->id, round->participantId, 4).empty() ||
            scoreForHole(4)->strokes != 3) {
            std::cerr << "Tracked stroke sequence validation was not atomic\n";
            return EXIT_FAILURE;
        }

        tracked.sequence = 1;
        tracked.startLatitude = 59.0;
        tracked.startLongitude = 10.0;
        tracked.endLatitude = 59.001;
        tracked.endLongitude = 10.002;
        tracked.distanceMetres = 157.5;
        tracked.accuracyMetres = 6.0;
        if (!shots.appendTrackedStroke(tracked, *round, {4, 5, 9})) {
            std::cerr << "Tracked GPS stroke append failed\n";
            return EXIT_FAILURE;
        }
        auto trackedShots = shots.list(round->id, round->participantId, 4);
        auto trackedScore = scoreForHole(4);
        if (trackedShots.size() != 1 || !trackedShots.front().startLatitude ||
            !trackedShots.front().endLongitude ||
            trackedShots.front().clubId != clubId ||
            trackedShots.front().clubName != QStringLiteral("7 iron") ||
            trackedShots.front().distanceMetres != 157.5 || !trackedScore ||
            trackedScore->strokes != 4 || trackedScore->putts != 1 ||
            trackedScore->fairway != FairwayResult::Centre ||
            trackedScore->greenInRegulation != true) {
            std::cerr << "Tracked GPS coordinates or score were not persisted\n";
            return EXIT_FAILURE;
        }

        opencaddie::storage::ShotRecord noGps;
        noGps.roundId = round->id;
        noGps.participantId = round->participantId;
        noGps.hole = 4;
        noGps.sequence = 2;
        noGps.shotType = QStringLiteral("unknown");
        if (!shots.appendTrackedStroke(noGps, *round, {4, 5, 9})) {
            std::cerr << "Locationless tracked stroke append failed\n";
            return EXIT_FAILURE;
        }
        trackedShots = shots.list(round->id, round->participantId, 4);
        trackedScore = scoreForHole(4);
        if (trackedShots.size() != 2 || trackedShots.back().startLatitude ||
            trackedShots.back().endLatitude || trackedShots.back().distanceMetres ||
            !trackedScore || trackedScore->strokes != 5 ||
            trackedScore->putts != 1) {
            std::cerr << "No-GPS stroke invented geometry or missed the score\n";
            return EXIT_FAILURE;
        }
        if (!shots.updateLastTrackedStrokeType(*round, {4, 5, 9},
                                               QStringLiteral("putt")) ||
            scoreForHole(4)->putts != 2 ||
            shots.list(round->id, round->participantId, 4).back().shotType !=
                QStringLiteral("putt") ||
            !shots.updateLastTrackedStrokeType(*round, {4, 5, 9},
                                               QStringLiteral("chip")) ||
            scoreForHole(4)->putts != 1 ||
            !shots.removeLastTrackedStroke(*round, {4, 5, 9}) ||
            shots.list(round->id, round->participantId, 4).size() != 1 ||
            scoreForHole(4)->strokes != 4) {
            std::cerr << "Tracked type correction or undo did not synchronize score\n";
            return EXIT_FAILURE;
        }

        const opencaddie::domain::HoleScore manualCorrection{
            .hole = 4,
            .strokes = 7,
            .putts = 1,
            .penalties = 2,
            .fairway = FairwayResult::Right,
            .greenInRegulation = false,
            .tee = "Red",
            .notes = "manual correction",
        };
        noGps.sequence = 2;
        noGps.shotType = QStringLiteral("chip");
        if (!rounds.saveScore(*round, {4, 5, 9}, manualCorrection) ||
            !shots.appendTrackedStroke(noGps, *round, {4, 5, 9})) {
            std::cerr << "Append after a manual score correction failed\n";
            return EXIT_FAILURE;
        }
        trackedScore = scoreForHole(4);
        if (!trackedScore || trackedScore->strokes != 8 ||
            trackedScore->putts != 1 || trackedScore->penalties != 2 ||
            trackedScore->fairway != FairwayResult::Right ||
            trackedScore->greenInRegulation != false ||
            trackedScore->tee != "Red" ||
            trackedScore->notes != "manual correction" ||
            !shots.removeLastTrackedStroke(*round, {4, 5, 9}) ||
            scoreForHole(4)->strokes != 7 ||
            !shots.removeLastTrackedStroke(*round, {4, 5, 9}) ||
            scoreForHole(4)->strokes != 6) {
            std::cerr << "Tracking overwrote manual score fields\n";
            return EXIT_FAILURE;
        }

        const opencaddie::domain::HoleScore originalFourth{
            .hole = 4,
            .strokes = 3,
            .putts = 1,
            .fairway = FairwayResult::Centre,
            .greenInRegulation = true,
        };
        if (!rounds.saveScore(*round, {4, 5, 9}, originalFourth))
            return EXIT_FAILURE;
        QSqlQuery rollbackTrigger(database.connection());
        if (!rollbackTrigger.exec(QStringLiteral(
                "CREATE TEMP TRIGGER reject_tracked_score BEFORE UPDATE ON "
                "hole_scores WHEN NEW.hole=4 BEGIN SELECT RAISE(ABORT,'forced "
                "score failure'); END"))) {
            std::cerr << "Could not create transaction rollback fixture\n";
            return EXIT_FAILURE;
        }
        noGps.sequence = 1;
        noGps.shotType = QStringLiteral("unknown");
        if (shots.appendTrackedStroke(noGps, *round, {4, 5, 9}) ||
            !shots.list(round->id, round->participantId, 4).empty() ||
            scoreForHole(4)->strokes != 3 ||
            !rollbackTrigger.exec(
                QStringLiteral("DROP TRIGGER reject_tracked_score"))) {
            std::cerr << "Failed score update did not roll back tracked stroke\n";
            return EXIT_FAILURE;
        }

        auto teeShot = drive;
        teeShot.hole = 2;
        teeShot.sequence = 1;
        teeShot.shotType = QStringLiteral("tee");
        teeShot.distanceMetres = 200.0;
        teeShot.externalId = QStringLiteral("fixture-tee-2");
        teeShot.metrics.clear();
        teeShot.replaceMetrics = false;
        auto externalUnknown = teeShot;
        externalUnknown.hole = 3;
        externalUnknown.shotType = QStringLiteral("bunker_escape");
        externalUnknown.distanceMetres.reset();
        externalUnknown.externalId = QStringLiteral("fixture-unknown-3");
        QSqlQuery guestParticipant(database.connection());
        guestParticipant.prepare(QStringLiteral(
            "INSERT INTO participants(id,round_id,profile_id,display_name,handicap,"
            "position) VALUES('guest-participant',?,NULL,'Guest',0,1)"));
        guestParticipant.addBindValue(round->id);
        auto guestShot = externalUnknown;
        guestShot.participantId = QStringLiteral("guest-participant");
        guestShot.hole = 1;
        guestShot.shotType = QStringLiteral("chip");
        guestShot.externalId = QStringLiteral("guest-shot-ignored");
        if (!shots.upsert(teeShot) || !shots.upsert(externalUnknown) ||
            !guestParticipant.exec() || !shots.upsert(guestShot)) {
            std::cerr << "External shot aggregation fixtures failed\n";
            return EXIT_FAILURE;
        }
        QSqlQuery triggerCheck(database.connection());
        if (triggerCheck.exec(
                QStringLiteral("UPDATE shots SET participant_id='other-participant' "
                               "WHERE external_id='fixture-drive-1'"))) {
            std::cerr << "Database shot identity trigger did not fire\n";
            return EXIT_FAILURE;
        }
        triggerCheck.finish();
        QSqlQuery scoreTriggerCheck(database.connection());
        scoreTriggerCheck.prepare(
            QStringLiteral("UPDATE hole_scores SET participant_id='other-participant' "
                           "WHERE round_id=? AND hole=1"));
        scoreTriggerCheck.addBindValue(round->id);
        if (scoreTriggerCheck.exec()) {
            std::cerr << "Database score identity trigger did not fire\n";
            return EXIT_FAILURE;
        }
        scoreTriggerCheck.finish();

        noGps.sequence = 1;
        noGps.shotType = QStringLiteral("unknown");
        if (!shots.appendTrackedStroke(noGps, *round, {4, 5, 9})) {
            std::cerr << "Could not create resumable tracked stroke\n";
            return EXIT_FAILURE;
        }

        opencaddie::storage::ShotRecord firstAndOnly;
        firstAndOnly.roundId = round->id;
        firstAndOnly.participantId = round->participantId;
        firstAndOnly.hole = 5;
        firstAndOnly.sequence = 1;
        firstAndOnly.shotType = QStringLiteral("putt");
        if (!shots.appendTrackedStroke(firstAndOnly, *round, {5, 4, 11}) ||
            !scoreForHole(5) || scoreForHole(5)->strokes != 1 ||
            scoreForHole(5)->putts != 1 ||
            !shots.removeLastTrackedStroke(*round, {5, 4, 11}) ||
            scoreForHole(5)) {
            std::cerr << "Undo did not clean up an automatically created score\n";
            return EXIT_FAILURE;
        }
    }

    // Re-open before completion to exercise WAL recovery and score durability.
    database.close();
    if (!database.open(databasePath))
        return EXIT_FAILURE;
    {
        opencaddie::storage::SettingsRepository settings(database.connection());
        if (settings.value(QStringLiteral("courseHandicap")) !=
            QStringLiteral("17")) {
            std::cerr << "Profile handicap setting did not survive reopen\n";
            return EXIT_FAILURE;
        }
        opencaddie::storage::RoundRepository reopened(database.connection());
        const auto resumed = reopened.active();
        if (!resumed || reopened.scores(*resumed).size() != 4 ||
            reopened.scores(*resumed).front().strokes != 5) {
            std::cerr << "Committed scores did not survive reopen\n";
            return EXIT_FAILURE;
        }
        if (resumed->weatherTemperatureC != 17.0 || resumed->weatherWindMps != 4.2 ||
            resumed->weatherWindDirectionDegrees.has_value() ||
            resumed->weatherCondition != QStringLiteral("Partly cloudy")) {
            std::cerr << "Round weather did not survive reopen\n";
            return EXIT_FAILURE;
        }
        opencaddie::storage::ShotRepository resumedShots(database.connection());
        const auto restoredTracked = resumedShots.list(
            resumed->id, resumed->participantId, 4);
        if (restoredTracked.size() != 1 ||
            restoredTracked.front().shotType != QStringLiteral("unknown") ||
            restoredTracked.front().endLatitude ||
            !resumedShots.removeLastTrackedStroke(*resumed, {4, 5, 9}) ||
            reopened.scores(*resumed).back().strokes != 3) {
            std::cerr << "Tracked stroke did not resume or undo after reopen\n";
            return EXIT_FAILURE;
        }
        if (!reopened.finish(resumed->id))
            return EXIT_FAILURE;

        opencaddie::storage::StatisticsRepository statistics(database.connection());
        const auto overview = statistics.overview({}, profile);
        if (overview.value(QStringLiteral("rounds")).toInt() != 1 ||
            overview.value(QStringLiteral("holes")).toInt() != 4 ||
            scoreCount(overview, QStringLiteral("eagle")) != 1 ||
            scoreCount(overview, QStringLiteral("birdie")) != 1 ||
            scoreCount(overview, QStringLiteral("par")) != 1 ||
            distributionCount(overview, QStringLiteral("fairwayDistribution"),
                              QStringLiteral("centre")) != 3 ||
            distributionCount(overview, QStringLiteral("fairwayDistribution"),
                              QStringLiteral("left")) != 1 ||
            !overview.value(QStringLiteral("longestDriveRecorded")).toBool() ||
            overview.value(QStringLiteral("longestDriveMetres")).toDouble() != 242.5 ||
            overview.value(QStringLiteral("trackedStrokes")).toInt() != 3 ||
            overview.value(QStringLiteral("scoredStrokes")).toInt() != 16 ||
            distributionCount(overview, QStringLiteral("shotTypeDistribution"),
                              QStringLiteral("drive")) != 2 ||
            distributionCount(overview, QStringLiteral("shotTypeDistribution"),
                              QStringLiteral("unknown")) != 1 ||
            overview.value(QStringLiteral("weatherRounds")).toInt() != 1) {
            std::cerr << "Statistics aggregation mismatch\n";
            return EXIT_FAILURE;
        }
        if (statistics.overview(QStringLiteral("missing"), profile)
                .value(QStringLiteral("rounds"))
                .toInt() != 0 ||
            statistics.overview(QStringLiteral("missing"), profile)
                    .value(QStringLiteral("trackedStrokes"))
                    .toInt() != 0) {
            std::cerr << "Course statistics filter mismatch\n";
            return EXIT_FAILURE;
        }
        const auto holePerformance =
            statistics.holePerformance(QStringLiteral("fixture"), profile);
        const auto firstHolePerformance = holePerformance.value(0).toMap();
        if (holePerformance.size() != 4 ||
            firstHolePerformance.value(QStringLiteral("hole")).toInt() != 1 ||
            firstHolePerformance.value(QStringLiteral("played")).toInt() != 1 ||
            firstHolePerformance.value(QStringLiteral("average")).toDouble() != 5.0 ||
            firstHolePerformance.value(QStringLiteral("best")).toInt() != 5 ||
            !statistics.holePerformance(QStringLiteral("missing"), profile)
                 .isEmpty()) {
            std::cerr << "Per-hole statistics aggregation mismatch\n";
            return EXIT_FAILURE;
        }

        const auto detail = reopened.detail(resumed->id, profile);
        const auto detailSummary =
            detail.value(QStringLiteral("summary")).toMap();
        if (detail.value(QStringLiteral("scores")).toList().size() != 4 ||
            detail.value(QStringLiteral("summary"))
                    .toMap()
                    .value(QStringLiteral("gross"))
                    .toInt() != 16 ||
            detail.value(QStringLiteral("summary"))
                    .toMap()
                    .value(QStringLiteral("toPar"))
                    .toInt() != -2 ||
            !detail.value(QStringLiteral("summary"))
                 .toMap()
                 .value(QStringLiteral("toParAvailable"))
                 .toBool() ||
            detail.value(QStringLiteral("summary"))
                    .toMap()
                    .value(QStringLiteral("scoredHoles"))
                    .toInt() != 4 ||
            detail.value(QStringLiteral("summary"))
                    .toMap()
                    .value(QStringLiteral("stablefordPoints"))
                    .toInt() != 14 ||
            detail.value(QStringLiteral("summary"))
                    .toMap()
                    .value(QStringLiteral("longestDriveMetres"))
                    .toDouble() != 242.5 ||
            detailSummary.value(QStringLiteral("trackedStrokes")).toInt() != 3 ||
            detailSummary.value(QStringLiteral("scoredStrokes")).toInt() != 16 ||
            distributionCount(detailSummary,
                              QStringLiteral("shotTypeDistribution"),
                              QStringLiteral("drive")) != 2 ||
            distributionCount(detailSummary,
                              QStringLiteral("shotTypeDistribution"),
                              QStringLiteral("unknown")) != 1 ||
            reopened.history({}, profile)
                    .front()
                    .toMap()
                    .value(QStringLiteral("birdies"))
                    .toInt() != 1 ||
            reopened.history({}, profile)
                    .front()
                    .toMap()
                    .value(QStringLiteral("stablefordPoints"))
                    .toInt() != 14 ||
            !reopened.detail(resumed->id, QStringLiteral("other-profile")).isEmpty() ||
            !reopened.history({}, QStringLiteral("other-profile")).isEmpty() ||
            !reopened.exportCsv(resumed->id)
                 .contains(QStringLiteral("weather_source"))) {
            std::cerr << "Round detail aggregation mismatch\n";
            return EXIT_FAILURE;
        }

        opencaddie::storage::IntegrationRepository integrations(database.connection());
        if (!integrations.upsert(
                {.provider = QStringLiteral("toptracer"),
                 .status = QStringLiteral("available"),
                 .displayName = QStringLiteral("Partner sandbox"),
                 .reportedCapabilities = {QStringLiteral("shots.read"),
                                          QStringLiteral("weather.read")}}) ||
            integrations.list().size() != 1 ||
            integrations.upsert({.provider = QStringLiteral("invalid"),
                                 .status = QStringLiteral("secret-token")}) ||
            integrations.upsert(
                {.provider = QStringLiteral("toptracer"),
                 .status = QStringLiteral("available"),
                 .reportedCapabilities = {QStringLiteral("unverified.superuser")}})) {
            std::cerr << "Integration state validation mismatch\n";
            return EXIT_FAILURE;
        }

        opencaddie::storage::RoundStart missingParStart;
        missingParStart.courseSlug = QStringLiteral("missing-par");
        missingParStart.courseName = QStringLiteral("Missing Par Course");
        missingParStart.profileId = profile;
        missingParStart.holeCount = 18;
        missingParStart.scoringMode = opencaddie::domain::ScoringMode::StrokePlay;
        missingParStart.handicapIndexScale = 18;
        const auto missingParRound = reopened.start(missingParStart);
        if (!missingParRound) {
            std::cerr << "Could not start missing-par fixture\n";
            return EXIT_FAILURE;
        }
        if (!reopened.saveScore(*missingParRound, {1, 7, 0},
                                {.hole = 1, .strokes = 5})) {
            std::cerr << "Could not save missing-par fixture\n";
            return EXIT_FAILURE;
        }
        if (!reopened.finish(missingParRound->id)) {
            std::cerr << "Could not finish missing-par fixture\n";
            return EXIT_FAILURE;
        }
        const auto missingParDetail = reopened.detail(missingParRound->id, profile);
        const auto missingParHistory =
            reopened.history(QStringLiteral("Missing Par"), profile);
        const auto statisticsWithMissingPar = statistics.overview({}, profile);
        if (missingParDetail.value(QStringLiteral("summary"))
                .toMap()
                .value(QStringLiteral("toParAvailable"))
                .toBool() ||
            missingParDetail.value(QStringLiteral("summary"))
                .toMap()
                .value(QStringLiteral("toPar"))
                .isValid() ||
            missingParHistory.size() != 1 ||
            missingParHistory.front()
                .toMap()
                .value(QStringLiteral("toParAvailable"))
                .toBool() ||
            missingParHistory.front().toMap().value(QStringLiteral("eagles")).toInt() !=
                0 ||
            statisticsWithMissingPar.value(QStringLiteral("rounds")).toInt() != 2 ||
            statisticsWithMissingPar.value(QStringLiteral("toParRounds")).toInt() !=
                1 ||
            statisticsWithMissingPar.value(QStringLiteral("trend")).toList().size() !=
                1 ||
            scoreCount(statisticsWithMissingPar, QStringLiteral("eagle")) != 1) {
            std::cerr << "Missing par polluted to-par statistics\n";
            return EXIT_FAILURE;
        }
    }

    database.close();
    const QString legacyPath = directory.filePath(QStringLiteral("legacy.sqlite"));
    if (!createLegacyV1Database(legacyPath))
        return EXIT_FAILURE;
    opencaddie::storage::Database migrated;
    if (!migrated.open(legacyPath) || migrated.schemaVersion() != 7) {
        std::cerr << "Legacy migration failed: " << migrated.lastError().toStdString()
                  << '\n';
        return EXIT_FAILURE;
    }
    QSqlQuery preserved(migrated.connection());
    if (!preserved.exec(QStringLiteral("SELECT p.handicap,p.handicap_source,hs.strokes,"
                                       "r.weather_condition,r.handicap_index_scale "
                                       "FROM profiles p "
                                       "JOIN rounds r ON r.profile_id=p.id "
                                       "JOIN hole_scores hs ON hs.round_id=r.id "
                                       "WHERE r.id='legacy-round'")) ||
        !preserved.next() || preserved.value(0).toDouble() != 14.2 ||
        preserved.value(1).toString() != QStringLiteral("local") ||
        preserved.value(2).toInt() != 5 || !preserved.value(3).toString().isEmpty() ||
        preserved.value(4).toInt() != 18) {
        std::cerr << "Legacy data was not preserved\n";
        return EXIT_FAILURE;
    }
    preserved.finish();
    if (!preserved.exec(QStringLiteral(
            "SELECT name,club_type FROM clubs ORDER BY position"))) {
        std::cerr << "Migrated club types could not be read\n";
        return EXIT_FAILURE;
    }
    QStringList migratedClubTypes;
    while (preserved.next()) {
        migratedClubTypes.push_back(preserved.value(0).toString() + "=" +
                                    preserved.value(1).toString());
    }
    const QStringList expectedClubTypes{
        QStringLiteral("Driver=driver"),
        QStringLiteral("5 wood=wood"),
        QStringLiteral("4 hybrid=hybrid"),
        QStringLiteral("7 iron=iron"),
        QStringLiteral("Sand wedge=wedge"),
        QStringLiteral("Putter=putter"),
        QStringLiteral("Old faithful=other"),
    };
    if (migratedClubTypes != expectedClubTypes) {
        std::cerr << "Legacy club types were not backfilled\n";
        return EXIT_FAILURE;
    }
    preserved.finish();
    opencaddie::storage::SettingsRepository migratedSettings(migrated.connection());
    opencaddie::storage::CourseRepository migratedCourses(migrated.connection());
    opencaddie::storage::RoundRepository migratedRounds(migrated.connection());
    if (!migratedSettings.setValue(QStringLiteral("migration-check"),
                                   QStringLiteral("ok")) ||
        migratedSettings.value(QStringLiteral("migration-check")) !=
            QStringLiteral("ok") ||
        !migratedCourses.list().isEmpty() ||
        migratedRounds.detail(QStringLiteral("legacy-round"))
                .value(QStringLiteral("scores"))
                .toList()
                .size() != 1) {
        std::cerr << "Migrated v1 database is not fully usable\n";
        return EXIT_FAILURE;
    }

    const QString legacyV2Path = directory.filePath(QStringLiteral("legacy-v2.sqlite"));
    if (!createLegacyV2Database(legacyV2Path))
        return EXIT_FAILURE;
    opencaddie::storage::Database migratedV2;
    if (!migratedV2.open(legacyV2Path) || migratedV2.schemaVersion() != 7) {
        std::cerr << "Version 2 migration failed: "
                  << migratedV2.lastError().toStdString() << '\n';
        return EXIT_FAILURE;
    }
    QSqlQuery v2Preserved(migratedV2.connection());
    if (!v2Preserved.exec(QStringLiteral(
            "SELECT COUNT(*) FROM shots WHERE id='preserved-v2-shot'")) ||
        !v2Preserved.next() || v2Preserved.value(0).toInt() != 1 ||
        !v2Preserved.exec(
            QStringLiteral("SELECT COUNT(*) FROM pragma_table_info('shot_metrics')")) ||
        !v2Preserved.next() || v2Preserved.value(0).toInt() != 6 ||
        !v2Preserved.exec(
            QStringLiteral("SELECT COUNT(*) FROM pragma_table_info('round_layups')")) ||
        !v2Preserved.next() || v2Preserved.value(0).toInt() != 5) {
        std::cerr << "Version 2 shot data was not preserved\n";
        return EXIT_FAILURE;
    }

    std::cout << "All storage tests passed\n";
    return EXIT_SUCCESS;
}
