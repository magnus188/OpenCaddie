#include "storage/Repositories.h"

#include "domain/Scoring.h"
#include "domain/Statistics.h"

#include <QDateTime>
#include <QJsonArray>
#include <QJsonDocument>
#include <QSet>
#include <QSqlError>
#include <QSqlQuery>
#include <QUuid>

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <unordered_map>

namespace opencaddie::storage {
namespace {
QString nowIso() { return QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs); }
QString uuid() { return QUuid::createUuid().toString(QUuid::WithoutBraces); }
QString textOrEmpty(const QString &value) {
    return value.isNull() ? QStringLiteral("") : value;
}

QString scoringMode(const domain::ScoringMode mode) {
    return mode == domain::ScoringMode::Stableford ? QStringLiteral("stableford")
                                                   : QStringLiteral("stroke");
}

domain::ScoringMode scoringMode(const QString &value) {
    return value == QStringLiteral("stableford") ? domain::ScoringMode::Stableford
                                                 : domain::ScoringMode::StrokePlay;
}

QString fairway(const domain::FairwayResult value) {
    switch (value) {
    case domain::FairwayResult::Left:
        return QStringLiteral("left");
    case domain::FairwayResult::Centre:
        return QStringLiteral("centre");
    case domain::FairwayResult::Right:
        return QStringLiteral("right");
    case domain::FairwayResult::Missed:
        return QStringLiteral("missed");
    case domain::FairwayResult::NotRecorded:
        return {};
    }
    return {};
}

domain::FairwayResult fairway(const QString &value) {
    if (value == QStringLiteral("left"))
        return domain::FairwayResult::Left;
    if (value == QStringLiteral("centre"))
        return domain::FairwayResult::Centre;
    if (value == QStringLiteral("right"))
        return domain::FairwayResult::Right;
    if (value == QStringLiteral("missed"))
        return domain::FairwayResult::Missed;
    return domain::FairwayResult::NotRecorded;
}

QString clubType(const domain::ClubType value) {
    switch (value) {
    case domain::ClubType::Driver:
        return QStringLiteral("driver");
    case domain::ClubType::Wood:
        return QStringLiteral("wood");
    case domain::ClubType::Hybrid:
        return QStringLiteral("hybrid");
    case domain::ClubType::Iron:
        return QStringLiteral("iron");
    case domain::ClubType::Wedge:
        return QStringLiteral("wedge");
    case domain::ClubType::Putter:
        return QStringLiteral("putter");
    case domain::ClubType::Other:
        return QStringLiteral("other");
    }
    return QStringLiteral("other");
}

domain::ClubType clubType(const QString &value) {
    if (value == QStringLiteral("driver"))
        return domain::ClubType::Driver;
    if (value == QStringLiteral("wood"))
        return domain::ClubType::Wood;
    if (value == QStringLiteral("hybrid"))
        return domain::ClubType::Hybrid;
    if (value == QStringLiteral("iron"))
        return domain::ClubType::Iron;
    if (value == QStringLiteral("wedge"))
        return domain::ClubType::Wedge;
    if (value == QStringLiteral("putter"))
        return domain::ClubType::Putter;
    return domain::ClubType::Other;
}

bool validOptionalNumber(const std::optional<double> &value) {
    return !value || std::isfinite(*value);
}

bool validOptionalCoordinate(const std::optional<double> &value,
                             const double minimum, const double maximum) {
    return validOptionalNumber(value) &&
           (!value || (*value >= minimum && *value <= maximum));
}

bool validShotRecord(const ShotRecord &shot) {
    if (shot.roundId.isEmpty() || shot.participantId.isEmpty() || shot.hole < 1 ||
        shot.hole > 18 || shot.sequence < 1 ||
        shot.startLatitude.has_value() != shot.startLongitude.has_value() ||
        shot.endLatitude.has_value() != shot.endLongitude.has_value() ||
        !validOptionalCoordinate(shot.startLatitude, -90.0, 90.0) ||
        !validOptionalCoordinate(shot.startLongitude, -180.0, 180.0) ||
        !validOptionalCoordinate(shot.endLatitude, -90.0, 90.0) ||
        !validOptionalCoordinate(shot.endLongitude, -180.0, 180.0) ||
        !validOptionalNumber(shot.distanceMetres) ||
        (shot.distanceMetres && *shot.distanceMetres < 0.0) ||
        !validOptionalNumber(shot.lateralMetres) ||
        !validOptionalNumber(shot.accuracyMetres) ||
        (shot.accuracyMetres && *shot.accuracyMetres < 0.0)) {
        return false;
    }
    return std::ranges::all_of(shot.metrics, [](const ShotRecord::Metric &metric) {
        return !metric.key.trimmed().isEmpty() &&
               std::isfinite(metric.canonicalValue) &&
               (!metric.sourceValue || std::isfinite(*metric.sourceValue));
    });
}

bool saveScoreRow(QSqlDatabase database, const ActiveRound &round,
                  const domain::HoleDefinition &hole,
                  const domain::HoleScore &score) {
    if (score.hole != hole.number || score.hole < 1 || score.hole > round.holeCount ||
        score.strokes < 0) {
        return false;
    }
    QSqlQuery query(database);
    query.prepare(QStringLiteral(
        "INSERT INTO hole_scores(round_id,participant_id,hole,par,stroke_index,"
        "strokes,putts,penalties,fairway,gir,tee,notes,updated_at)"
        " VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?) "
        "ON CONFLICT(round_id,participant_id,hole) DO UPDATE SET "
        "par=excluded.par,stroke_index=excluded.stroke_index,"
        "strokes=excluded.strokes,putts=excluded.putts,"
        "penalties=excluded.penalties,fairway=excluded.fairway,gir=excluded.gir,"
        "tee=excluded.tee,notes=excluded.notes,updated_at=excluded.updated_at"));
    query.addBindValue(round.id);
    query.addBindValue(round.participantId);
    query.addBindValue(score.hole);
    query.addBindValue(hole.par);
    query.addBindValue(hole.strokeIndex);
    query.addBindValue(score.strokes);
    query.addBindValue(score.putts ? QVariant(*score.putts) : QVariant{});
    query.addBindValue(score.penalties);
    query.addBindValue(fairway(score.fairway));
    query.addBindValue(score.greenInRegulation
                           ? QVariant(*score.greenInRegulation)
                           : QVariant{});
    query.addBindValue(QString::fromStdString(score.tee));
    query.addBindValue(QString::fromStdString(score.notes));
    query.addBindValue(nowIso());
    return query.exec();
}

bool activeRoundIdentity(QSqlDatabase database, const ActiveRound &round,
                         const int hole) {
    QSqlQuery query(database);
    query.prepare(QStringLiteral(
        "SELECT 1 FROM rounds r JOIN participants p ON p.round_id=r.id "
        "WHERE r.id=? AND p.id=? AND r.status='in_progress' "
        "AND ? BETWEEN 1 AND r.hole_count"));
    query.addBindValue(round.id);
    query.addBindValue(round.participantId);
    query.addBindValue(hole);
    return query.exec() && query.next();
}

bool incrementStoredScore(QSqlDatabase database, const ActiveRound &round,
                          const domain::HoleDefinition &hole,
                          const bool incrementPutts) {
    QSqlQuery current(database);
    current.prepare(QStringLiteral(
        "SELECT strokes FROM hole_scores WHERE round_id=? AND participant_id=? "
        "AND hole=?"));
    current.addBindValue(round.id);
    current.addBindValue(round.participantId);
    current.addBindValue(hole.number);
    if (!current.exec())
        return false;
    if (!current.next()) {
        domain::HoleScore score{.hole = hole.number, .strokes = 1};
        if (incrementPutts)
            score.putts = 1;
        return saveScoreRow(database, round, hole, score);
    }
    if (current.value(0).toInt() >= 20)
        return false;
    current.finish();

    QSqlQuery update(database);
    update.prepare(incrementPutts
                       ? QStringLiteral(
                             "UPDATE hole_scores SET strokes=strokes+1,"
                             "putts=COALESCE(putts,0)+1,updated_at=? "
                             "WHERE round_id=? AND participant_id=? AND hole=?")
                       : QStringLiteral(
                             "UPDATE hole_scores SET strokes=strokes+1,updated_at=? "
                             "WHERE round_id=? AND participant_id=? AND hole=?"));
    update.addBindValue(nowIso());
    update.addBindValue(round.id);
    update.addBindValue(round.participantId);
    update.addBindValue(hole.number);
    return update.exec() && update.numRowsAffected() == 1;
}

bool decrementStoredScore(QSqlDatabase database, const ActiveRound &round,
                          const domain::HoleDefinition &hole,
                          const bool decrementPutts) {
    QSqlQuery current(database);
    current.prepare(QStringLiteral(
        "SELECT strokes,putts,penalties,fairway,gir,tee,notes FROM hole_scores "
        "WHERE round_id=? AND participant_id=? AND hole=?"));
    current.addBindValue(round.id);
    current.addBindValue(round.participantId);
    current.addBindValue(hole.number);
    if (!current.exec() || !current.next() || current.value(0).toInt() <= 0)
        return false;
    const int strokes = current.value(0).toInt();
    const bool autoOnly = strokes == 1 && current.value(2).toInt() == 0 &&
                          current.value(3).toString().isEmpty() &&
                          current.value(4).isNull() &&
                          current.value(5).toString().isEmpty() &&
                          current.value(6).toString().isEmpty() &&
                          (!decrementPutts || current.value(1).toInt() <= 1);
    current.finish();
    if (autoOnly) {
        QSqlQuery remove(database);
        remove.prepare(QStringLiteral(
            "DELETE FROM hole_scores WHERE round_id=? AND participant_id=? "
            "AND hole=?"));
        remove.addBindValue(round.id);
        remove.addBindValue(round.participantId);
        remove.addBindValue(hole.number);
        return remove.exec() && remove.numRowsAffected() == 1;
    }

    QSqlQuery update(database);
    update.prepare(decrementPutts
                       ? QStringLiteral(
                             "UPDATE hole_scores SET strokes=strokes-1,"
                             "putts=MAX(0,COALESCE(putts,0)-1),updated_at=? "
                             "WHERE round_id=? AND participant_id=? AND hole=?")
                       : QStringLiteral(
                             "UPDATE hole_scores SET strokes=strokes-1,updated_at=? "
                             "WHERE round_id=? AND participant_id=? AND hole=?"));
    update.addBindValue(nowIso());
    update.addBindValue(round.id);
    update.addBindValue(round.participantId);
    update.addBindValue(hole.number);
    return update.exec() && update.numRowsAffected() == 1;
}

bool adjustStoredPutts(QSqlDatabase database, const ActiveRound &round,
                       const domain::HoleDefinition &hole, const int delta) {
    if (delta == 0)
        return true;
    QSqlQuery update(database);
    update.prepare(delta > 0
                       ? QStringLiteral(
                             "UPDATE hole_scores SET "
                             "putts=COALESCE(putts,0)+1,updated_at=? "
                             "WHERE round_id=? AND participant_id=? AND hole=?")
                       : QStringLiteral(
                             "UPDATE hole_scores SET "
                             "putts=MAX(0,COALESCE(putts,0)-1),updated_at=? "
                             "WHERE round_id=? AND participant_id=? AND hole=?"));
    update.addBindValue(nowIso());
    update.addBindValue(round.id);
    update.addBindValue(round.participantId);
    update.addBindValue(hole.number);
    return update.exec() && update.numRowsAffected() == 1;
}

QString canonicalShotType(const QString &value) {
    if (value == QStringLiteral("tee"))
        return QStringLiteral("drive");
    static const QSet<QString> allowed{
        QStringLiteral("drive"), QStringLiteral("approach"),
        QStringLiteral("chip"), QStringLiteral("putt"),
        QStringLiteral("unknown"),
    };
    return allowed.contains(value) ? value : QStringLiteral("unknown");
}

struct ShotTypeCounts {
    std::array<int, 5> values{};

    void add(const QString &rawType, const int amount = 1) {
        const QString type = canonicalShotType(rawType);
        const int index = type == QStringLiteral("drive")      ? 0
                          : type == QStringLiteral("approach") ? 1
                          : type == QStringLiteral("chip")     ? 2
                          : type == QStringLiteral("putt")     ? 3
                                                               : 4;
        values[static_cast<std::size_t>(index)] += amount;
    }

    [[nodiscard]] int total() const {
        int result = 0;
        for (const int value : values)
            result += value;
        return result;
    }

    [[nodiscard]] QVariantList distribution() const {
        static const std::array<const char *, 5> keys{
            "drive", "approach", "chip", "putt", "unknown"};
        QVariantList result;
        const int count = total();
        for (std::size_t index = 0; index < keys.size(); ++index) {
            result.push_back(QVariantMap{
                {QStringLiteral("key"), QString::fromLatin1(keys[index])},
                {QStringLiteral("count"), values[index]},
                {QStringLiteral("percentage"),
                 count > 0 ? 100.0 * static_cast<double>(values[index]) /
                                 static_cast<double>(count)
                           : 0.0},
            });
        }
        return result;
    }
};

bool execute(QSqlQuery &query) { return query.exec(); }
} // namespace

SettingsRepository::SettingsRepository(QSqlDatabase database)
    : m_database(std::move(database)) {}

QString SettingsRepository::value(const QString &key, const QString &fallback) const {
    QSqlQuery query(m_database);
    query.prepare(QStringLiteral("SELECT value FROM settings WHERE key=?"));
    query.addBindValue(key);
    return query.exec() && query.next() ? query.value(0).toString() : fallback;
}

bool SettingsRepository::setValue(const QString &key, const QString &value) {
    QSqlQuery query(m_database);
    query.prepare(
        QStringLiteral("INSERT INTO settings(key,value,updated_at) VALUES(?,?,?) "
                       "ON CONFLICT(key) DO UPDATE SET value=excluded.value,"
                       "updated_at=excluded.updated_at"));
    query.addBindValue(key);
    query.addBindValue(value);
    query.addBindValue(nowIso());
    return execute(query);
}

QVariantMap SettingsRepository::all() const {
    QVariantMap values;
    QSqlQuery query(m_database);
    if (query.exec(QStringLiteral("SELECT key,value FROM settings"))) {
        while (query.next()) {
            values.insert(query.value(0).toString(), query.value(1));
        }
    }
    return values;
}

bool SettingsRepository::reset() {
    QSqlQuery query(m_database);
    return query.exec(QStringLiteral("DELETE FROM settings"));
}

ClubRepository::ClubRepository(QSqlDatabase database)
    : m_database(std::move(database)) {}

bool ClubRepository::ensureDefaultProfile() {
    QSqlQuery count(m_database);
    if (!count.exec(QStringLiteral("SELECT 1 FROM profiles LIMIT 1")))
        return false;
    if (count.next())
        return true;

    QSqlQuery query(m_database);
    query.prepare(
        QStringLiteral("INSERT INTO profiles(id,name,handicap,is_default,created_at)"
                       " VALUES(?,?,?,?,?)"));
    query.addBindValue(QStringLiteral("local-player"));
    query.addBindValue(QStringLiteral("Player"));
    query.addBindValue(0.0);
    query.addBindValue(1);
    query.addBindValue(nowIso());
    return query.exec();
}

bool ClubRepository::ensureStarterBag() {
    static const QString initializedKey =
        QStringLiteral("starterBagInitialized");
    SettingsRepository settings(m_database);
    if (settings.value(initializedKey) == QStringLiteral("1"))
        return true;

    const QString profileId = defaultProfileId();
    if (profileId.isEmpty())
        return false;

    QSqlQuery count(m_database);
    count.prepare(QStringLiteral("SELECT COUNT(*) FROM clubs WHERE profile_id=?"));
    count.addBindValue(profileId);
    if (!count.exec() || !count.next())
        return false;
    if (count.value(0).toInt() > 0)
        return settings.setValue(initializedKey, QStringLiteral("1"));

    struct StarterClub {
        const char *name;
        double carryMetres;
        domain::ClubType type;
    };
    static const StarterClub starterClubs[] = {
        {"Driver", 215.0, domain::ClubType::Driver},
        {"5 iron", 175.0, domain::ClubType::Iron},
        {"7 iron", 145.0, domain::ClubType::Iron},
        {"Pitching wedge", 105.0, domain::ClubType::Wedge},
        {"Sand wedge", 75.0, domain::ClubType::Wedge},
        {"Putter", 10.0, domain::ClubType::Putter},
    };
    if (!m_database.transaction())
        return false;
    for (const auto &[name, carry, type] : starterClubs) {
        if (create(profileId, QString::fromLatin1(name), carry, type).isEmpty()) {
            m_database.rollback();
            return false;
        }
    }
    if (!m_database.commit())
        return false;
    return settings.setValue(initializedKey, QStringLiteral("1"));
}

QString ClubRepository::defaultProfileId() const {
    QSqlQuery query(m_database);
    if (query.exec(QStringLiteral(
            "SELECT id FROM profiles ORDER BY is_default DESC,created_at LIMIT 1")) &&
        query.next()) {
        return query.value(0).toString();
    }
    return {};
}

std::vector<domain::Club> ClubRepository::list(const QString &profileId) const {
    std::vector<domain::Club> clubs;
    QSqlQuery query(m_database);
    query.prepare(
        QStringLiteral("SELECT id,name,carry_metres,enabled,position,club_type FROM clubs "
                       "WHERE profile_id=? ORDER BY position,name"));
    query.addBindValue(profileId);
    if (!query.exec())
        return clubs;
    while (query.next()) {
        clubs.push_back({query.value(0).toString().toStdString(),
                         query.value(1).toString().toStdString(),
                         query.value(2).toDouble(), query.value(3).toBool(),
                         query.value(4).toInt(),
                         clubType(query.value(5).toString())});
    }
    return clubs;
}

QString ClubRepository::create(const QString &profileId, const QString &name,
                               const double carryMetres,
                               const domain::ClubType type, const bool enabled) {
    const QString id = uuid();
    QSqlQuery position(m_database);
    position.prepare(QStringLiteral(
        "SELECT COALESCE(MAX(position),-1)+1 FROM clubs WHERE profile_id=?"));
    position.addBindValue(profileId);
    const int order =
        position.exec() && position.next() ? position.value(0).toInt() : 0;

    QSqlQuery query(m_database);
    query.prepare(QStringLiteral(
        "INSERT INTO clubs(id,profile_id,name,carry_metres,enabled,position,"
        "created_at,updated_at,club_type) VALUES(?,?,?,?,?,?,?,?,?)"));
    query.addBindValue(id);
    query.addBindValue(profileId);
    query.addBindValue(name.trimmed());
    query.addBindValue(carryMetres);
    query.addBindValue(enabled);
    query.addBindValue(order);
    query.addBindValue(nowIso());
    query.addBindValue(nowIso());
    query.addBindValue(clubType(type));
    return query.exec() ? id : QString{};
}

bool ClubRepository::update(const domain::Club &club) {
    QSqlQuery query(m_database);
    query.prepare(
        QStringLiteral("UPDATE clubs SET name=?,carry_metres=?,enabled=?,position=?,"
                       "updated_at=?,club_type=? WHERE id=?"));
    query.addBindValue(QString::fromStdString(club.name).trimmed());
    query.addBindValue(club.carryMetres);
    query.addBindValue(club.enabled);
    query.addBindValue(club.position);
    query.addBindValue(nowIso());
    query.addBindValue(clubType(club.type));
    query.addBindValue(QString::fromStdString(club.id));
    return query.exec() && query.numRowsAffected() == 1;
}

bool ClubRepository::remove(const QString &id) {
    QSqlQuery query(m_database);
    query.prepare(QStringLiteral("DELETE FROM clubs WHERE id=?"));
    query.addBindValue(id);
    return query.exec() && query.numRowsAffected() == 1;
}

bool ClubRepository::reorder(const QStringList &orderedIds) {
    if (!m_database.transaction())
        return false;
    QSqlQuery query(m_database);
    query.prepare(
        QStringLiteral("UPDATE clubs SET position=?,updated_at=? WHERE id=?"));
    for (qsizetype index = 0; index < orderedIds.size(); ++index) {
        query.bindValue(0, index);
        query.bindValue(1, nowIso());
        query.bindValue(2, orderedIds.at(index));
        if (!query.exec()) {
            m_database.rollback();
            return false;
        }
    }
    return m_database.commit();
}

RoundRepository::RoundRepository(QSqlDatabase database)
    : m_database(std::move(database)) {}

std::optional<ActiveRound> RoundRepository::active() const {
    QSqlQuery query(m_database);
    if (!query.exec(QStringLiteral(
            "SELECT r.id,p.id,r.course_slug,r.course_name,r.hole_count,"
            "r.course_handicap,r.scoring_mode,r.current_hole,r.tee,"
            "r.weather_temperature_c,r.weather_wind_mps,"
            "r.weather_wind_direction_deg,r.weather_condition "
            "FROM rounds r JOIN participants p ON p.round_id=r.id "
            "WHERE r.status='in_progress' ORDER BY r.started_at DESC LIMIT 1")) ||
        !query.next()) {
        return std::nullopt;
    }
    ActiveRound round{query.value(0).toString(),
                      query.value(1).toString(),
                      query.value(2).toString(),
                      query.value(3).toString(),
                      query.value(4).toInt(),
                      query.value(5).toInt(),
                      scoringMode(query.value(6).toString()),
                      query.value(7).toInt(),
                      query.value(8).toString()};
    if (!query.value(9).isNull())
        round.weatherTemperatureC = query.value(9).toDouble();
    if (!query.value(10).isNull())
        round.weatherWindMps = query.value(10).toDouble();
    if (!query.value(11).isNull())
        round.weatherWindDirectionDegrees = query.value(11).toInt();
    round.weatherCondition = query.value(12).toString();
    return round;
}

std::optional<ActiveRound> RoundRepository::start(const RoundStart &start) {
    if ((start.holeCount != 9 && start.holeCount != 18) ||
        (start.handicapIndexScale != 9 && start.handicapIndexScale != 18) ||
        !m_database.transaction()) {
        return std::nullopt;
    }
    const QString roundId = uuid();
    const QString participantId = uuid();
    QSqlQuery round(m_database);
    round.prepare(QStringLiteral(
        "INSERT INTO rounds(id,course_slug,course_name,course_version,profile_id,"
        "hole_count,scoring_mode,course_handicap,handicap_index_scale,status,"
        "current_hole,started_at,tee,weather_temperature_c,weather_wind_mps,"
        "weather_wind_direction_deg,weather_condition,weather_source,"
        "weather_recorded_at)"
        " VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)"));
    round.addBindValue(roundId);
    round.addBindValue(start.courseSlug);
    round.addBindValue(start.courseName);
    round.addBindValue(start.courseVersion);
    round.addBindValue(start.profileId);
    round.addBindValue(start.holeCount);
    round.addBindValue(scoringMode(start.scoringMode));
    round.addBindValue(start.courseHandicap);
    round.addBindValue(start.handicapIndexScale);
    round.addBindValue(QStringLiteral("in_progress"));
    round.addBindValue(1);
    round.addBindValue(nowIso());
    round.addBindValue(textOrEmpty(start.tee));
    round.addBindValue(start.weatherTemperatureC ? QVariant(*start.weatherTemperatureC)
                                                 : QVariant{});
    round.addBindValue(start.weatherWindMps ? QVariant(*start.weatherWindMps)
                                            : QVariant{});
    round.addBindValue(start.weatherWindDirectionDegrees
                           ? QVariant(*start.weatherWindDirectionDegrees)
                           : QVariant{});
    round.addBindValue(textOrEmpty(start.weatherCondition));
    round.addBindValue(textOrEmpty(start.weatherSource));
    const bool hasWeather = start.weatherTemperatureC || start.weatherWindMps ||
                            start.weatherWindDirectionDegrees ||
                            !start.weatherCondition.isEmpty() ||
                            !start.weatherSource.isEmpty();
    round.addBindValue(hasWeather ? QVariant(nowIso()) : QVariant{});
    if (!round.exec()) {
        m_database.rollback();
        return std::nullopt;
    }

    QSqlQuery participant(m_database);
    participant.prepare(QStringLiteral(
        "INSERT INTO participants(id,round_id,profile_id,display_name,handicap,"
        "position) SELECT ?,?,id,name,?,0 FROM profiles WHERE id=?"));
    participant.addBindValue(participantId);
    participant.addBindValue(roundId);
    participant.addBindValue(start.courseHandicap);
    participant.addBindValue(start.profileId);
    if (!participant.exec() || participant.numRowsAffected() != 1 ||
        !m_database.commit()) {
        m_database.rollback();
        return std::nullopt;
    }
    ActiveRound started{
        roundId,         participantId,        start.courseSlug,  start.courseName,
        start.holeCount, start.courseHandicap, start.scoringMode, 1, start.tee};
    started.weatherTemperatureC = start.weatherTemperatureC;
    started.weatherWindMps = start.weatherWindMps;
    started.weatherWindDirectionDegrees = start.weatherWindDirectionDegrees;
    started.weatherCondition = start.weatherCondition;
    return started;
}

bool RoundRepository::saveScore(const ActiveRound &round,
                                const domain::HoleDefinition &hole,
                                const domain::HoleScore &score) {
    if (!m_database.transaction()) {
        return false;
    }
    if (!saveScoreRow(m_database, round, hole, score)) {
        m_database.rollback();
        return false;
    }
    return m_database.commit();
}

std::vector<domain::HoleScore> RoundRepository::scores(const ActiveRound &round) const {
    std::vector<domain::HoleScore> values;
    QSqlQuery query(m_database);
    query.prepare(QStringLiteral(
        "SELECT hole,strokes,putts,penalties,fairway,gir,tee,notes "
        "FROM hole_scores WHERE round_id=? AND participant_id=? ORDER BY hole"));
    query.addBindValue(round.id);
    query.addBindValue(round.participantId);
    if (!query.exec())
        return values;
    while (query.next()) {
        values.push_back(
            {query.value(0).toInt(), query.value(1).toInt(),
             query.value(2).isNull() ? std::nullopt
                                     : std::optional<int>{query.value(2).toInt()},
             query.value(3).toInt(), fairway(query.value(4).toString()),
             query.value(5).isNull() ? std::nullopt
                                     : std::optional<bool>{query.value(5).toBool()},
             query.value(6).toString().toStdString(),
             query.value(7).toString().toStdString()});
    }
    return values;
}

bool RoundRepository::setCurrentHole(const QString &roundId, const int hole) {
    QSqlQuery query(m_database);
    query.prepare(QStringLiteral(
        "UPDATE rounds SET current_hole=? WHERE id=? AND status='in_progress'"));
    query.addBindValue(hole);
    query.addBindValue(roundId);
    return query.exec() && query.numRowsAffected() == 1;
}

bool RoundRepository::finish(const QString &roundId) {
    return setStatus(roundId, QStringLiteral("completed"), true);
}

bool RoundRepository::abandon(const QString &roundId) {
    return setStatus(roundId, QStringLiteral("abandoned"), true);
}

bool RoundRepository::setStatus(const QString &roundId, const QString &status,
                                const bool completed) {
    QSqlQuery query(m_database);
    query.prepare(
        QStringLiteral("UPDATE rounds SET status=?,completed_at=? WHERE id=?"));
    query.addBindValue(status);
    query.addBindValue(completed ? QVariant(nowIso()) : QVariant{});
    query.addBindValue(roundId);
    return query.exec() && query.numRowsAffected() == 1;
}

QVariantList RoundRepository::history(const QString &search,
                                      const QString &profileId) const {
    QVariantList rounds;
    QSqlQuery query(m_database);
    QString sql = QStringLiteral(
        "SELECT r.id,r.course_name,r.status,r.started_at,r.completed_at,"
        "r.hole_count,r.scoring_mode,"
        "COALESCE(SUM(CASE WHEN hs.strokes>0 THEN hs.strokes ELSE 0 END),0),"
        "COALESCE(SUM(CASE WHEN hs.strokes>0 AND hs.par BETWEEN 3 AND 6 "
        "THEN hs.par ELSE 0 END),0),"
        "COUNT(CASE WHEN hs.strokes>0 THEN 1 END),"
        "COUNT(CASE WHEN hs.strokes>0 AND hs.par BETWEEN 3 AND 6 THEN 1 END),"
        "r.weather_condition,r.weather_temperature_c,r.weather_wind_mps,"
        "COALESCE(SUM(CASE WHEN hs.strokes>0 AND hs.par BETWEEN 3 AND 6 AND "
        "hs.strokes=hs.par-2 "
        "THEN 1 ELSE 0 END),0),"
        "COALESCE(SUM(CASE WHEN hs.strokes>0 AND hs.par BETWEEN 3 AND 6 AND "
        "hs.strokes=hs.par-1 THEN 1 ELSE 0 END),0),"
        "COALESCE(SUM(CASE WHEN hs.strokes>0 AND hs.par BETWEEN 3 AND 6 AND "
        "hs.strokes=hs.par THEN 1 ELSE 0 END),0) "
        "FROM rounds r "
        "LEFT JOIN participants p ON p.round_id=r.id AND p.profile_id=r.profile_id "
        "LEFT JOIN hole_scores hs ON hs.round_id=r.id AND "
        "hs.participant_id=p.id "
        "WHERE r.course_name LIKE ?");
    if (!profileId.isEmpty()) {
        sql += QStringLiteral(" AND r.profile_id=?");
    }
    sql += QStringLiteral(
        " "
        "GROUP BY r.id,r.course_name,r.status,r.started_at,r.completed_at,"
        "r.hole_count,r.scoring_mode,r.weather_condition,"
        "r.weather_temperature_c,r.weather_wind_mps "
        "ORDER BY r.started_at DESC LIMIT 50");
    query.prepare(sql);
    query.addBindValue(QStringLiteral("%%1%").arg(search));
    if (!profileId.isEmpty())
        query.addBindValue(profileId);
    if (!query.exec())
        return rounds;
    while (query.next()) {
        const int gross = query.value(7).toInt();
        const int par = query.value(8).toInt();
        const int scoredHoles = query.value(9).toInt();
        const bool toParAvailable =
            scoredHoles > 0 && query.value(10).toInt() == scoredHoles;
        rounds.push_back(QVariantMap{
            {QStringLiteral("id"), query.value(0)},
            {QStringLiteral("courseName"), query.value(1)},
            {QStringLiteral("status"), query.value(2)},
            {QStringLiteral("startedAt"), query.value(3)},
            {QStringLiteral("completedAt"), query.value(4)},
            {QStringLiteral("holeCount"), query.value(5)},
            {QStringLiteral("scoringMode"), query.value(6)},
            {QStringLiteral("gross"), gross},
            {QStringLiteral("par"), par},
            {QStringLiteral("toPar"),
             toParAvailable ? QVariant(gross - par) : QVariant{}},
            {QStringLiteral("toParAvailable"), toParAvailable},
            {QStringLiteral("scoredHoles"), scoredHoles},
            {QStringLiteral("weatherCondition"), query.value(11)},
            {QStringLiteral("weatherTemperatureC"), query.value(12)},
            {QStringLiteral("weatherWindMps"), query.value(13)},
            {QStringLiteral("eagles"), query.value(14)},
            {QStringLiteral("birdies"), query.value(15)},
            {QStringLiteral("pars"), query.value(16)},
        });
    }
    query.finish();
    for (qsizetype index = 0; index < rounds.size(); ++index) {
        auto value = rounds.at(index).toMap();
        if (value.value(QStringLiteral("scoringMode")).toString() ==
            QStringLiteral("stableford")) {
            const auto summary =
                detail(value.value(QStringLiteral("id")).toString(), profileId)
                    .value(QStringLiteral("summary"))
                    .toMap();
            value.insert(QStringLiteral("stablefordPoints"),
                         summary.value(QStringLiteral("stablefordPoints")));
            value.insert(QStringLiteral("stablefordAvailable"),
                         summary.value(QStringLiteral("stablefordAvailable")));
            rounds[index] = value;
        }
    }
    return rounds;
}

QVariantMap RoundRepository::detail(const QString &roundId,
                                    const QString &profileId) const {
    QVariantMap result;
    QSqlQuery round(m_database);
    QString roundSql =
        QStringLiteral("SELECT course_name,status,started_at,completed_at,hole_count,"
                       "scoring_mode,course_handicap,tee,handicap_index_scale,"
                       "weather_temperature_c,weather_wind_mps,"
                       "weather_wind_direction_deg,weather_condition,weather_source,"
                       "weather_recorded_at FROM rounds WHERE id=?");
    if (!profileId.isEmpty()) {
        roundSql += QStringLiteral(" AND profile_id=?");
    }
    round.prepare(roundSql);
    round.addBindValue(roundId);
    if (!profileId.isEmpty())
        round.addBindValue(profileId);
    if (!round.exec() || !round.next())
        return result;
    result = {{QStringLiteral("id"), roundId},
              {QStringLiteral("courseName"), round.value(0)},
              {QStringLiteral("status"), round.value(1)},
              {QStringLiteral("startedAt"), round.value(2)},
              {QStringLiteral("completedAt"), round.value(3)},
              {QStringLiteral("holeCount"), round.value(4)},
              {QStringLiteral("scoringMode"), round.value(5)},
              {QStringLiteral("courseHandicap"), round.value(6)},
              {QStringLiteral("tee"), round.value(7)},
              {QStringLiteral("weather"),
               QVariantMap{
                   {QStringLiteral("temperatureC"), round.value(9)},
                   {QStringLiteral("windMps"), round.value(10)},
                   {QStringLiteral("windDirectionDegrees"), round.value(11)},
                   {QStringLiteral("condition"), round.value(12)},
                   {QStringLiteral("source"), round.value(13)},
                   {QStringLiteral("recordedAt"), round.value(14)},
               }}};

    QVariantList scores;
    int scoredHoles = 0;
    int validParHoles = 0;
    int gross = 0;
    int par = 0;
    int penalties = 0;
    int putts = 0;
    int puttHoles = 0;
    int fairwaysHit = 0;
    int fairwaysRecorded = 0;
    int fairwaysLeft = 0;
    int fairwaysCentre = 0;
    int fairwaysRight = 0;
    int fairwaysMissed = 0;
    int greensHit = 0;
    int greensRecorded = 0;
    int albatrosses = 0;
    int eagles = 0;
    int birdies = 0;
    int pars = 0;
    int bogeys = 0;
    int doublesOrWorse = 0;
    std::vector<domain::HoleDefinition> scoringDefinitions;
    QSqlQuery score(m_database);
    score.prepare(
        QStringLiteral("SELECT hs.hole,hs.par,hs.stroke_index,hs.strokes,hs.putts,"
                       "hs.penalties,hs.fairway,hs.gir,hs.tee,hs.notes "
                       "FROM hole_scores hs "
                       "JOIN participants p ON p.id=hs.participant_id AND "
                       "p.round_id=hs.round_id "
                       "JOIN rounds r ON r.id=hs.round_id "
                       "WHERE hs.round_id=? AND p.profile_id=r.profile_id "
                       "ORDER BY hs.hole"));
    score.addBindValue(roundId);
    if (score.exec()) {
        while (score.next()) {
            const int holePar = score.value(1).toInt();
            const int holeNumber = score.value(0).toInt();
            const int strokeIndex = score.value(2).toInt();
            const int strokes = score.value(3).toInt();
            scoringDefinitions.push_back({holeNumber, holePar, strokeIndex});
            if (strokes > 0) {
                ++scoredHoles;
                gross += strokes;
                if (holePar >= 3 && holePar <= 6) {
                    par += holePar;
                    ++validParHoles;
                }
                penalties += score.value(5).toInt();
                if (!score.value(4).isNull()) {
                    putts += score.value(4).toInt();
                    ++puttHoles;
                }
                const QString fairwayResult = score.value(6).toString();
                if (!fairwayResult.isEmpty()) {
                    ++fairwaysRecorded;
                    if (fairwayResult == QStringLiteral("centre")) {
                        ++fairwaysHit;
                        ++fairwaysCentre;
                    } else if (fairwayResult == QStringLiteral("left")) {
                        ++fairwaysLeft;
                    } else if (fairwayResult == QStringLiteral("right")) {
                        ++fairwaysRight;
                    } else if (fairwayResult == QStringLiteral("missed")) {
                        ++fairwaysMissed;
                    }
                }
                if (!score.value(7).isNull()) {
                    ++greensRecorded;
                    if (score.value(7).toBool())
                        ++greensHit;
                }
                switch (domain::classifyHole(holePar, strokes)) {
                case domain::HoleOutcome::AlbatrossOrBetter:
                    ++albatrosses;
                    break;
                case domain::HoleOutcome::Eagle:
                    ++eagles;
                    break;
                case domain::HoleOutcome::Birdie:
                    ++birdies;
                    break;
                case domain::HoleOutcome::Par:
                    ++pars;
                    break;
                case domain::HoleOutcome::Bogey:
                    ++bogeys;
                    break;
                case domain::HoleOutcome::DoubleBogeyOrWorse:
                    ++doublesOrWorse;
                    break;
                case domain::HoleOutcome::NotRecorded:
                    break;
                }
            }
            scores.push_back(QVariantMap{
                {QStringLiteral("hole"), score.value(0)},
                {QStringLiteral("par"), score.value(1)},
                {QStringLiteral("strokeIndex"), score.value(2)},
                {QStringLiteral("strokes"), score.value(3)},
                {QStringLiteral("putts"), score.value(4)},
                {QStringLiteral("penalties"), score.value(5)},
                {QStringLiteral("fairway"), score.value(6)},
                {QStringLiteral("gir"), score.value(7)},
                {QStringLiteral("tee"), score.value(8)},
                {QStringLiteral("notes"), score.value(9)},
            });
        }
    }
    const int storedIndexScale = round.value(8).toInt();
    const int indexScale = storedIndexScale == 9 || storedIndexScale == 18
                               ? storedIndexScale
                               : (round.value(4).toInt() == 18
                                      ? 18
                                      : domain::handicapIndexScale(scoringDefinitions));
    const bool stablefordAvailable =
        round.value(5).toString() == QStringLiteral("stableford") &&
        (indexScale == 9 || indexScale == 18) &&
        domain::canUseHandicapScoring(scoringDefinitions);
    int stablefordPoints = 0;
    if (stablefordAvailable) {
        for (qsizetype index = 0; index < scores.size(); ++index) {
            auto scoreMap = scores.at(index).toMap();
            const int points = domain::stablefordPoints(
                scoringDefinitions.at(static_cast<std::size_t>(index)),
                scoreMap.value(QStringLiteral("strokes")).toInt(),
                round.value(6).toInt(), indexScale);
            scoreMap.insert(QStringLiteral("stablefordPoints"), points);
            scores[index] = scoreMap;
            stablefordPoints += points;
        }
    }
    result.insert(QStringLiteral("scores"), scores);
    const bool toParAvailable = scoredHoles > 0 && validParHoles == scoredHoles;
    QSqlQuery longestDrive(m_database);
    longestDrive.prepare(
        QStringLiteral("SELECT MAX(s.distance_metres) FROM shots s "
                       "JOIN participants p ON p.id=s.participant_id "
                       "JOIN rounds r ON r.id=s.round_id AND p.round_id=r.id "
                       "WHERE s.round_id=? AND p.profile_id=r.profile_id AND "
                       "(s.shot_type='drive' OR s.shot_type='tee')"));
    longestDrive.addBindValue(roundId);
    const double longestDriveMetres =
        longestDrive.exec() && longestDrive.next() && !longestDrive.value(0).isNull()
            ? longestDrive.value(0).toDouble()
            : 0.0;
    ShotTypeCounts shotCounts;
    QSqlQuery shotTypes(m_database);
    shotTypes.prepare(QStringLiteral(
        "SELECT s.shot_type,COUNT(*) FROM shots s "
        "JOIN participants p ON p.id=s.participant_id "
        "JOIN rounds r ON r.id=s.round_id AND p.round_id=r.id "
        "WHERE s.round_id=? AND p.profile_id=r.profile_id GROUP BY s.shot_type"));
    shotTypes.addBindValue(roundId);
    if (shotTypes.exec()) {
        while (shotTypes.next())
            shotCounts.add(shotTypes.value(0).toString(), shotTypes.value(1).toInt());
    }
    result.insert(
        QStringLiteral("summary"),
        QVariantMap{
            {QStringLiteral("gross"), gross},
            {QStringLiteral("par"), par},
            {QStringLiteral("toPar"),
             toParAvailable ? QVariant(gross - par) : QVariant{}},
            {QStringLiteral("toParAvailable"), toParAvailable},
            {QStringLiteral("scoredHoles"), scoredHoles},
            {QStringLiteral("stablefordPoints"), stablefordPoints},
            {QStringLiteral("stablefordAvailable"), stablefordAvailable},
            {QStringLiteral("penalties"), penalties},
            {QStringLiteral("averagePutts"),
             puttHoles > 0 ? static_cast<double>(putts) / puttHoles : 0.0},
            {QStringLiteral("puttsRecorded"), puttHoles},
            {QStringLiteral("fairwayPct"),
             fairwaysRecorded > 0 ? 100.0 * fairwaysHit / fairwaysRecorded : 0.0},
            {QStringLiteral("fairwaysRecorded"), fairwaysRecorded},
            {QStringLiteral("fairwaysLeft"), fairwaysLeft},
            {QStringLiteral("fairwaysCentre"), fairwaysCentre},
            {QStringLiteral("fairwaysRight"), fairwaysRight},
            {QStringLiteral("fairwaysMissed"), fairwaysMissed},
            {QStringLiteral("girPct"),
             greensRecorded > 0 ? 100.0 * greensHit / greensRecorded : 0.0},
            {QStringLiteral("greensRecorded"), greensRecorded},
            {QStringLiteral("longestDriveMetres"), longestDriveMetres},
            {QStringLiteral("trackedStrokes"), shotCounts.total()},
            {QStringLiteral("scoredStrokes"), gross},
            {QStringLiteral("shotTypeDistribution"), shotCounts.distribution()},
            {QStringLiteral("albatrosses"), albatrosses},
            {QStringLiteral("eagles"), eagles},
            {QStringLiteral("birdies"), birdies},
            {QStringLiteral("pars"), pars},
            {QStringLiteral("bogeys"), bogeys},
            {QStringLiteral("doublesOrWorse"), doublesOrWorse},
        });
    return result;
}

bool RoundRepository::remove(const QString &roundId) {
    QSqlQuery query(m_database);
    query.prepare(QStringLiteral("DELETE FROM rounds WHERE id=?"));
    query.addBindValue(roundId);
    return query.exec() && query.numRowsAffected() == 1;
}

CourseAnalysisRepository::CourseAnalysisRepository(QSqlDatabase database)
    : m_database(std::move(database)) {}

bool CourseAnalysisRepository::saveLayups(const QString &courseSlug, const int hole,
                                           const QVariantList &points) {
    if (courseSlug.trimmed().isEmpty() || hole < 1 || hole > 18 ||
        !m_database.transaction()) {
        return false;
    }

    QSqlQuery remove(m_database);
    remove.prepare(QStringLiteral(
        "DELETE FROM course_analysis_layups WHERE course_slug=? AND hole=?"));
    remove.addBindValue(courseSlug);
    remove.addBindValue(hole);
    if (!remove.exec()) {
        m_database.rollback();
        return false;
    }

    QSqlQuery insert(m_database);
    insert.prepare(QStringLiteral(
        "INSERT INTO course_analysis_layups(course_slug,hole,sequence,x,y,"
        "updated_at) VALUES(?,?,?,?,?,?)"));
    for (qsizetype index = 0; index < points.size(); ++index) {
        const QVariantMap point = points.at(index).toMap();
        bool xValid = false;
        bool yValid = false;
        const double x = point.value(QStringLiteral("x")).toDouble(&xValid);
        const double y = point.value(QStringLiteral("y")).toDouble(&yValid);
        if (!xValid || !yValid || !std::isfinite(x) || !std::isfinite(y)) {
            m_database.rollback();
            return false;
        }
        insert.bindValue(0, courseSlug);
        insert.bindValue(1, hole);
        insert.bindValue(2, index + 1);
        insert.bindValue(3, x);
        insert.bindValue(4, y);
        insert.bindValue(5, nowIso());
        if (!insert.exec()) {
            m_database.rollback();
            return false;
        }
    }
    return m_database.commit();
}

QVariantList CourseAnalysisRepository::layups(const QString &courseSlug,
                                               const int hole) const {
    QVariantList points;
    QSqlQuery query(m_database);
    query.prepare(QStringLiteral(
        "SELECT x,y FROM course_analysis_layups WHERE course_slug=? AND hole=? "
        "ORDER BY sequence"));
    query.addBindValue(courseSlug);
    query.addBindValue(hole);
    if (!query.exec())
        return points;
    while (query.next()) {
        points.push_back(QVariantMap{{QStringLiteral("x"), query.value(0)},
                                     {QStringLiteral("y"), query.value(1)}});
    }
    return points;
}

int CourseAnalysisRepository::analyzedHoleCount(const QString &courseSlug) const {
    QSqlQuery query(m_database);
    query.prepare(QStringLiteral(
        "SELECT COUNT(DISTINCT hole) FROM course_analysis_layups "
        "WHERE course_slug=?"));
    query.addBindValue(courseSlug);
    return query.exec() && query.next() ? query.value(0).toInt() : 0;
}

bool CourseAnalysisRepository::importToRound(const QString &courseSlug,
                                             const QString &roundId,
                                             const int holeCount) {
    if (courseSlug.trimmed().isEmpty() || roundId.trimmed().isEmpty() ||
        holeCount < 1 || holeCount > 18) {
        return false;
    }
    QSqlQuery round(m_database);
    round.prepare(QStringLiteral(
        "SELECT hole_count FROM rounds WHERE id=? AND course_slug=?"));
    round.addBindValue(roundId);
    round.addBindValue(courseSlug);
    if (!round.exec() || !round.next() || round.value(0).toInt() != holeCount ||
        !m_database.transaction()) {
        return false;
    }
    QSqlQuery remove(m_database);
    remove.prepare(QStringLiteral("DELETE FROM round_layups WHERE round_id=?"));
    remove.addBindValue(roundId);
    if (!remove.exec()) {
        m_database.rollback();
        return false;
    }
    QSqlQuery insert(m_database);
    insert.prepare(QStringLiteral(
        "INSERT INTO round_layups(round_id,hole,sequence,x,y) "
        "SELECT ?,hole,sequence,x,y FROM course_analysis_layups "
        "WHERE course_slug=? AND hole<=?"));
    insert.addBindValue(roundId);
    insert.addBindValue(courseSlug);
    insert.addBindValue(holeCount);
    if (!insert.exec()) {
        m_database.rollback();
        return false;
    }
    return m_database.commit();
}

QVariantList CourseAnalysisRepository::roundLayups(const QString &roundId,
                                                    const int hole) const {
    QVariantList points;
    QSqlQuery query(m_database);
    query.prepare(QStringLiteral(
        "SELECT x,y FROM round_layups WHERE round_id=? AND hole=? "
        "ORDER BY sequence"));
    query.addBindValue(roundId);
    query.addBindValue(hole);
    if (!query.exec())
        return points;
    while (query.next()) {
        points.push_back(QVariantMap{{QStringLiteral("x"), query.value(0)},
                                     {QStringLiteral("y"), query.value(1)}});
    }
    return points;
}

QJsonObject RoundRepository::exportJson(const QString &roundId) const {
    return QJsonObject::fromVariantMap(detail(roundId));
}

QString RoundRepository::exportCsv(const QString &roundId) const {
    const auto data = detail(roundId);
    QString csv = QStringLiteral(
        "course,status,started_at,hole,par,index,strokes,stableford_points,"
        "putts,penalties,"
        "fairway,gir,tee,notes,weather_condition,weather_temperature_c,"
        "weather_wind_mps,weather_source\n");
    const auto quote = [](QString value) {
        value.replace('"', QStringLiteral("\"\""));
        return QStringLiteral("\"%1\"").arg(value);
    };
    for (const auto &value : data.value(QStringLiteral("scores")).toList()) {
        const auto score = value.toMap();
        const auto weather = data.value(QStringLiteral("weather")).toMap();
        csv += QStringLiteral("%1,%2,%3,%4,%5,%6,%7,%8,%9,%10,%11,%12,%13,%14,%15,"
                              "%16,%17,%18\n")
                   .arg(quote(data.value(QStringLiteral("courseName")).toString()),
                        quote(data.value(QStringLiteral("status")).toString()),
                        quote(data.value(QStringLiteral("startedAt")).toString()),
                        score.value(QStringLiteral("hole")).toString(),
                        score.value(QStringLiteral("par")).toString(),
                        score.value(QStringLiteral("strokeIndex")).toString(),
                        score.value(QStringLiteral("strokes")).toString(),
                        score.value(QStringLiteral("stablefordPoints")).toString(),
                        score.value(QStringLiteral("putts")).toString(),
                        score.value(QStringLiteral("penalties")).toString(),
                        quote(score.value(QStringLiteral("fairway")).toString()),
                        score.value(QStringLiteral("gir")).toString(),
                        quote(score.value(QStringLiteral("tee")).toString()),
                        quote(score.value(QStringLiteral("notes")).toString()),
                        quote(weather.value(QStringLiteral("condition")).toString()),
                        weather.value(QStringLiteral("temperatureC")).toString(),
                        weather.value(QStringLiteral("windMps")).toString(),
                        quote(weather.value(QStringLiteral("source")).toString()));
    }
    return csv;
}

ShotRepository::ShotRepository(QSqlDatabase database)
    : m_database(std::move(database)) {}

bool ShotRepository::upsert(const ShotRecord &shot) {
    if (!validShotRecord(shot))
        return false;
    const QString source = shot.sourceProvider.isEmpty() ? QStringLiteral("opencaddie")
                                                         : shot.sourceProvider;
    QString shotId = shot.id;
    QString recordedAt = shot.recordedAt;
    if (!shot.externalId.isEmpty()) {
        QSqlQuery existing(m_database);
        existing.prepare(QStringLiteral(
            "SELECT id,round_id,participant_id,hole,sequence,recorded_at "
            "FROM shots "
            "WHERE source_provider=? AND external_id=?"));
        existing.addBindValue(source);
        existing.addBindValue(shot.externalId);
        if (!existing.exec())
            return false;
        if (existing.next()) {
            const bool sameIdentity =
                existing.value(1).toString() == shot.roundId &&
                existing.value(2).toString() == shot.participantId &&
                existing.value(3).toInt() == shot.hole &&
                existing.value(4).toInt() == shot.sequence;
            if (!sameIdentity)
                return false;
            shotId = existing.value(0).toString();
            if (recordedAt.isEmpty()) {
                recordedAt = existing.value(5).toString();
            }
        }
    }
    QSqlQuery existingIdentity(m_database);
    existingIdentity.prepare(QStringLiteral("SELECT id,recorded_at FROM shots "
                                            "WHERE round_id=? AND participant_id=? "
                                            "AND hole=? AND sequence=?"));
    existingIdentity.addBindValue(shot.roundId);
    existingIdentity.addBindValue(shot.participantId);
    existingIdentity.addBindValue(shot.hole);
    existingIdentity.addBindValue(shot.sequence);
    if (!existingIdentity.exec())
        return false;
    if (existingIdentity.next()) {
        const QString identityId = existingIdentity.value(0).toString();
        if (!shotId.isEmpty() && shotId != identityId)
            return false;
        shotId = identityId;
        if (recordedAt.isEmpty()) {
            recordedAt = existingIdentity.value(1).toString();
        }
    }
    existingIdentity.finish();
    if (shotId.isEmpty())
        shotId = uuid();
    if (recordedAt.isEmpty())
        recordedAt = nowIso();

    QSqlQuery identity(m_database);
    identity.prepare(QStringLiteral("SELECT r.hole_count FROM rounds r "
                                    "JOIN participants p ON p.round_id=r.id "
                                    "WHERE r.id=? AND p.id=?"));
    identity.addBindValue(shot.roundId);
    identity.addBindValue(shot.participantId);
    if (!identity.exec() || !identity.next() || shot.hole > identity.value(0).toInt()) {
        return false;
    }
    identity.finish();

    if (!m_database.transaction())
        return false;
    QSqlQuery query(m_database);
    query.prepare(QStringLiteral(
        "INSERT INTO shots(id,round_id,participant_id,hole,sequence,club_id,"
        "club_name,shot_type,start_latitude,start_longitude,end_latitude,"
        "end_longitude,distance_metres,lateral_metres,accuracy_metres,result,"
        "source_provider,external_id,recorded_at)"
        " VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?) "
        "ON CONFLICT(round_id,participant_id,hole,sequence) DO UPDATE SET "
        "club_id=excluded.club_id,club_name=excluded.club_name,"
        "shot_type=excluded.shot_type,start_latitude=excluded.start_latitude,"
        "start_longitude=excluded.start_longitude,"
        "end_latitude=excluded.end_latitude,"
        "end_longitude=excluded.end_longitude,"
        "distance_metres=excluded.distance_metres,"
        "lateral_metres=excluded.lateral_metres,"
        "accuracy_metres=excluded.accuracy_metres,result=excluded.result,"
        "source_provider=excluded.source_provider,"
        "external_id=excluded.external_id,recorded_at=excluded.recorded_at"));
    query.addBindValue(shotId);
    query.addBindValue(shot.roundId);
    query.addBindValue(shot.participantId);
    query.addBindValue(shot.hole);
    query.addBindValue(shot.sequence);
    query.addBindValue(shot.clubId.isEmpty() ? QVariant{} : shot.clubId);
    query.addBindValue(textOrEmpty(shot.clubName));
    query.addBindValue(textOrEmpty(shot.shotType));
    query.addBindValue(shot.startLatitude ? QVariant(*shot.startLatitude) : QVariant{});
    query.addBindValue(shot.startLongitude ? QVariant(*shot.startLongitude)
                                           : QVariant{});
    query.addBindValue(shot.endLatitude ? QVariant(*shot.endLatitude) : QVariant{});
    query.addBindValue(shot.endLongitude ? QVariant(*shot.endLongitude) : QVariant{});
    query.addBindValue(shot.distanceMetres ? QVariant(*shot.distanceMetres)
                                           : QVariant{});
    query.addBindValue(shot.lateralMetres ? QVariant(*shot.lateralMetres) : QVariant{});
    query.addBindValue(shot.accuracyMetres ? QVariant(*shot.accuracyMetres)
                                           : QVariant{});
    query.addBindValue(textOrEmpty(shot.result));
    query.addBindValue(source);
    query.addBindValue(shot.externalId.isEmpty() ? QVariant{} : shot.externalId);
    query.addBindValue(recordedAt);
    if (!query.exec()) {
        m_database.rollback();
        return false;
    }

    if (shot.replaceMetrics || !shot.metrics.empty()) {
        QSqlQuery clearMetrics(m_database);
        clearMetrics.prepare(
            QStringLiteral("DELETE FROM shot_metrics WHERE shot_id=?"));
        clearMetrics.addBindValue(shotId);
        if (!clearMetrics.exec()) {
            m_database.rollback();
            return false;
        }
        for (const auto &metric : shot.metrics) {
            QSqlQuery saveMetric(m_database);
            saveMetric.prepare(QStringLiteral(
                "INSERT INTO shot_metrics(shot_id,metric_key,canonical_value,"
                "canonical_unit,source_value,source_unit) VALUES(?,?,?,?,?,?)"));
            saveMetric.addBindValue(shotId);
            saveMetric.addBindValue(metric.key.trimmed());
            saveMetric.addBindValue(metric.canonicalValue);
            saveMetric.addBindValue(textOrEmpty(metric.canonicalUnit));
            saveMetric.addBindValue(metric.sourceValue ? QVariant(*metric.sourceValue)
                                                       : QVariant{});
            saveMetric.addBindValue(textOrEmpty(metric.sourceUnit));
            if (!saveMetric.exec()) {
                m_database.rollback();
                return false;
            }
        }
    }
    if (!m_database.commit()) {
        m_database.rollback();
        return false;
    }
    return true;
}

std::vector<ShotRecord> ShotRepository::list(const QString &roundId,
                                             const QString &participantId,
                                             const int hole) const {
    std::vector<ShotRecord> shots;
    if (roundId.isEmpty() || participantId.isEmpty() || hole < 1 || hole > 18)
        return shots;
    QSqlQuery query(m_database);
    query.prepare(QStringLiteral(
        "SELECT id,round_id,participant_id,hole,sequence,club_id,club_name,"
        "shot_type,start_latitude,start_longitude,end_latitude,end_longitude,"
        "distance_metres,lateral_metres,accuracy_metres,result,source_provider,"
        "external_id,recorded_at FROM shots WHERE round_id=? AND participant_id=? "
        "AND hole=? ORDER BY sequence"));
    query.addBindValue(roundId);
    query.addBindValue(participantId);
    query.addBindValue(hole);
    if (!query.exec())
        return shots;
    const auto optionalDouble = [&query](const int column) -> std::optional<double> {
        return query.value(column).isNull()
                   ? std::nullopt
                   : std::optional<double>{query.value(column).toDouble()};
    };
    while (query.next()) {
        ShotRecord shot;
        shot.id = query.value(0).toString();
        shot.roundId = query.value(1).toString();
        shot.participantId = query.value(2).toString();
        shot.hole = query.value(3).toInt();
        shot.sequence = query.value(4).toInt();
        shot.clubId = query.value(5).toString();
        shot.clubName = query.value(6).toString();
        shot.shotType = query.value(7).toString();
        shot.startLatitude = optionalDouble(8);
        shot.startLongitude = optionalDouble(9);
        shot.endLatitude = optionalDouble(10);
        shot.endLongitude = optionalDouble(11);
        shot.distanceMetres = optionalDouble(12);
        shot.lateralMetres = optionalDouble(13);
        shot.accuracyMetres = optionalDouble(14);
        shot.result = query.value(15).toString();
        shot.sourceProvider = query.value(16).toString();
        shot.externalId = query.value(17).toString();
        shot.recordedAt = query.value(18).toString();
        shots.push_back(std::move(shot));
    }
    return shots;
}

bool ShotRepository::appendTrackedStroke(const ShotRecord &input,
                                         const ActiveRound &round,
                                         const domain::HoleDefinition &hole) {
    ShotRecord shot = input;
    shot.sourceProvider = shot.sourceProvider.isEmpty()
                              ? QStringLiteral("opencaddie")
                              : shot.sourceProvider;
    if (!validShotRecord(shot) || !shot.metrics.empty() ||
        shot.sourceProvider != QStringLiteral("opencaddie") ||
        canonicalShotType(shot.shotType) != shot.shotType ||
        shot.roundId != round.id || shot.participantId != round.participantId ||
        shot.hole != hole.number) {
        return false;
    }
    if (shot.id.isEmpty())
        shot.id = uuid();
    if (shot.recordedAt.isEmpty())
        shot.recordedAt = nowIso();
    if (!m_database.transaction())
        return false;
    if (!activeRoundIdentity(m_database, round, shot.hole)) {
        m_database.rollback();
        return false;
    }

    QSqlQuery previous(m_database);
    previous.prepare(QStringLiteral(
        "SELECT COALESCE(MAX(sequence),0) FROM shots WHERE round_id=? AND "
        "participant_id=? AND hole=?"));
    previous.addBindValue(round.id);
    previous.addBindValue(round.participantId);
    previous.addBindValue(hole.number);
    if (!previous.exec() || !previous.next() ||
        shot.sequence != previous.value(0).toInt() + 1) {
        m_database.rollback();
        return false;
    }
    previous.finish();

    QSqlQuery query(m_database);
    query.prepare(QStringLiteral(
        "INSERT INTO shots(id,round_id,participant_id,hole,sequence,club_id,"
        "club_name,shot_type,start_latitude,start_longitude,end_latitude,"
        "end_longitude,distance_metres,lateral_metres,accuracy_metres,result,"
        "source_provider,external_id,recorded_at)"
        " VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)"));
    query.addBindValue(shot.id);
    query.addBindValue(shot.roundId);
    query.addBindValue(shot.participantId);
    query.addBindValue(shot.hole);
    query.addBindValue(shot.sequence);
    query.addBindValue(shot.clubId.isEmpty() ? QVariant{} : shot.clubId);
    query.addBindValue(textOrEmpty(shot.clubName));
    query.addBindValue(shot.shotType);
    query.addBindValue(shot.startLatitude ? QVariant(*shot.startLatitude) : QVariant{});
    query.addBindValue(shot.startLongitude ? QVariant(*shot.startLongitude)
                                           : QVariant{});
    query.addBindValue(shot.endLatitude ? QVariant(*shot.endLatitude) : QVariant{});
    query.addBindValue(shot.endLongitude ? QVariant(*shot.endLongitude) : QVariant{});
    query.addBindValue(shot.distanceMetres ? QVariant(*shot.distanceMetres)
                                           : QVariant{});
    query.addBindValue(shot.lateralMetres ? QVariant(*shot.lateralMetres) : QVariant{});
    query.addBindValue(shot.accuracyMetres ? QVariant(*shot.accuracyMetres)
                                           : QVariant{});
    query.addBindValue(textOrEmpty(shot.result));
    query.addBindValue(QStringLiteral("opencaddie"));
    query.addBindValue(QVariant{});
    query.addBindValue(shot.recordedAt);
    if (!query.exec() ||
        !incrementStoredScore(m_database, round, hole,
                              shot.shotType == QStringLiteral("putt")) ||
        !m_database.commit()) {
        m_database.rollback();
        return false;
    }
    return true;
}

bool ShotRepository::removeLastTrackedStroke(
    const ActiveRound &round, const domain::HoleDefinition &hole) {
    if (!m_database.transaction())
        return false;
    if (!activeRoundIdentity(m_database, round, hole.number)) {
        m_database.rollback();
        return false;
    }
    QSqlQuery latest(m_database);
    latest.prepare(QStringLiteral(
        "SELECT id,source_provider,shot_type FROM shots WHERE round_id=? AND participant_id=? "
        "AND hole=? ORDER BY sequence DESC LIMIT 1"));
    latest.addBindValue(round.id);
    latest.addBindValue(round.participantId);
    latest.addBindValue(hole.number);
    if (!latest.exec() || !latest.next() ||
        latest.value(1).toString() != QStringLiteral("opencaddie")) {
        m_database.rollback();
        return false;
    }
    const QString shotId = latest.value(0).toString();
    const bool wasPutt = latest.value(2).toString() == QStringLiteral("putt");
    latest.finish();
    QSqlQuery remove(m_database);
    remove.prepare(QStringLiteral("DELETE FROM shots WHERE id=?"));
    remove.addBindValue(shotId);
    if (!remove.exec() || remove.numRowsAffected() != 1 ||
        !decrementStoredScore(m_database, round, hole, wasPutt) ||
        !m_database.commit()) {
        m_database.rollback();
        return false;
    }
    return true;
}

bool ShotRepository::updateLastTrackedStrokeType(
    const ActiveRound &round, const domain::HoleDefinition &hole,
    const QString &shotType) {
    if (canonicalShotType(shotType) != shotType || !m_database.transaction()) {
        return false;
    }
    if (!activeRoundIdentity(m_database, round, hole.number)) {
        m_database.rollback();
        return false;
    }
    QSqlQuery latest(m_database);
    latest.prepare(QStringLiteral(
        "SELECT shot_type FROM shots WHERE round_id=? AND participant_id=? "
        "AND hole=? ORDER BY sequence DESC LIMIT 1"));
    latest.addBindValue(round.id);
    latest.addBindValue(round.participantId);
    latest.addBindValue(hole.number);
    if (!latest.exec() || !latest.next()) {
        m_database.rollback();
        return false;
    }
    const QString previousType = latest.value(0).toString();
    latest.finish();
    if (previousType == shotType) {
        return m_database.commit();
    }

    QSqlQuery update(m_database);
    update.prepare(QStringLiteral(
        "UPDATE shots SET shot_type=? WHERE id=(SELECT id FROM shots "
        "WHERE round_id=? AND participant_id=? AND hole=? "
        "AND source_provider='opencaddie' AND sequence=(SELECT MAX(sequence) "
        "FROM shots WHERE round_id=? AND participant_id=? AND hole=?))"));
    update.addBindValue(shotType);
    update.addBindValue(round.id);
    update.addBindValue(round.participantId);
    update.addBindValue(hole.number);
    update.addBindValue(round.id);
    update.addBindValue(round.participantId);
    update.addBindValue(hole.number);
    const int puttDelta = (shotType == QStringLiteral("putt") ? 1 : 0) -
                          (previousType == QStringLiteral("putt") ? 1 : 0);
    if (!update.exec() || update.numRowsAffected() != 1 ||
        !adjustStoredPutts(m_database, round, hole, puttDelta) ||
        !m_database.commit()) {
        m_database.rollback();
        return false;
    }
    return true;
}

StatisticsRepository::StatisticsRepository(QSqlDatabase database)
    : m_database(std::move(database)) {}

QVariantList StatisticsRepository::holePerformance(
    const QString &courseSlug, const QString &profileId) const {
    QVariantList holes;
    if (courseSlug.isEmpty())
        return holes;

    QSqlQuery query(m_database);
    QString sql = QStringLiteral(
        "SELECT hs.hole,COUNT(*),AVG(hs.strokes),MIN(hs.strokes) "
        "FROM rounds r "
        "JOIN participants p ON p.round_id=r.id AND p.profile_id=r.profile_id "
        "JOIN hole_scores hs ON hs.round_id=r.id AND hs.participant_id=p.id "
        "WHERE r.status='completed' AND r.course_slug=? AND hs.strokes>0");
    if (!profileId.isEmpty())
        sql += QStringLiteral(" AND r.profile_id=?");
    sql += QStringLiteral(" GROUP BY hs.hole ORDER BY hs.hole");
    query.prepare(sql);
    query.addBindValue(courseSlug);
    if (!profileId.isEmpty())
        query.addBindValue(profileId);
    if (!query.exec())
        return holes;

    while (query.next()) {
        holes.push_back(QVariantMap{
            {QStringLiteral("hole"), query.value(0)},
            {QStringLiteral("played"), query.value(1)},
            {QStringLiteral("average"), query.value(2)},
            {QStringLiteral("best"), query.value(3)},
        });
    }
    return holes;
}

QVariantMap StatisticsRepository::overview(const QString &courseSlug,
                                           const QString &profileId) const {
    struct RoundValues {
        QString id;
        QString courseSlug;
        QString courseName;
        QString startedAt;
        int configuredHoles = 18;
        int holes = 0;
        int validParHoles = 0;
        int gross = 0;
        int par = 0;
        int putts = 0;
        int puttHoles = 0;
        int penalties = 0;
        int fairwaysHit = 0;
        int fairwaysRecorded = 0;
        int fairwaysLeft = 0;
        int fairwaysCentre = 0;
        int fairwaysRight = 0;
        int fairwaysMissed = 0;
        int greensHit = 0;
        int greensRecorded = 0;
        int albatrosses = 0;
        int eagles = 0;
        int birdies = 0;
        int pars = 0;
        int bogeys = 0;
        int doublesOrWorse = 0;
    };

    QSqlQuery query(m_database);
    QString sql = QStringLiteral(
        "SELECT r.id,r.course_slug,r.course_name,r.started_at,r.hole_count,"
        "hs.par,hs.strokes,hs.putts,hs.penalties,hs.fairway,hs.gir "
        "FROM rounds r "
        "JOIN participants p ON p.round_id=r.id AND p.profile_id=r.profile_id "
        "LEFT JOIN hole_scores hs ON hs.round_id=r.id AND "
        "hs.participant_id=p.id "
        "WHERE r.status='completed'");
    if (!profileId.isEmpty()) {
        sql += QStringLiteral(" AND r.profile_id=?");
    }
    if (!courseSlug.isEmpty()) {
        sql += QStringLiteral(" AND r.course_slug=?");
    }
    sql += QStringLiteral(" ORDER BY r.started_at,r.id,hs.hole");
    query.prepare(sql);
    if (!profileId.isEmpty())
        query.addBindValue(profileId);
    if (!courseSlug.isEmpty())
        query.addBindValue(courseSlug);

    std::vector<RoundValues> roundValues;
    std::unordered_map<std::string, std::size_t> roundIndex;
    if (query.exec()) {
        while (query.next()) {
            const QString id = query.value(0).toString();
            const std::string key = id.toStdString();
            auto found = roundIndex.find(key);
            if (found == roundIndex.end()) {
                const std::size_t index = roundValues.size();
                roundIndex.emplace(key, index);
                roundValues.push_back(
                    {id, query.value(1).toString(), query.value(2).toString(),
                     query.value(3).toString(), query.value(4).toInt()});
                found = roundIndex.find(key);
            }
            if (query.value(6).isNull() || query.value(6).toInt() <= 0) {
                continue;
            }
            auto &round = roundValues.at(found->second);
            const int holePar = query.value(5).toInt();
            const int strokes = query.value(6).toInt();
            ++round.holes;
            round.gross += strokes;
            if (holePar >= 3 && holePar <= 6) {
                round.par += holePar;
                ++round.validParHoles;
            }
            round.penalties += query.value(8).toInt();
            if (!query.value(7).isNull()) {
                round.putts += query.value(7).toInt();
                ++round.puttHoles;
            }
            const QString fairwayResult = query.value(9).toString();
            if (!fairwayResult.isEmpty()) {
                ++round.fairwaysRecorded;
                if (fairwayResult == QStringLiteral("centre")) {
                    ++round.fairwaysHit;
                    ++round.fairwaysCentre;
                } else if (fairwayResult == QStringLiteral("left")) {
                    ++round.fairwaysLeft;
                } else if (fairwayResult == QStringLiteral("right")) {
                    ++round.fairwaysRight;
                } else if (fairwayResult == QStringLiteral("missed")) {
                    ++round.fairwaysMissed;
                }
            }
            if (!query.value(10).isNull()) {
                ++round.greensRecorded;
                if (query.value(10).toBool())
                    ++round.greensHit;
            }
            switch (domain::classifyHole(holePar, strokes)) {
            case domain::HoleOutcome::AlbatrossOrBetter:
                ++round.albatrosses;
                break;
            case domain::HoleOutcome::Eagle:
                ++round.eagles;
                break;
            case domain::HoleOutcome::Birdie:
                ++round.birdies;
                break;
            case domain::HoleOutcome::Par:
                ++round.pars;
                break;
            case domain::HoleOutcome::Bogey:
                ++round.bogeys;
                break;
            case domain::HoleOutcome::DoubleBogeyOrWorse:
                ++round.doublesOrWorse;
                break;
            case domain::HoleOutcome::NotRecorded:
                break;
            }
        }
    }

    roundValues.erase(
        std::remove_if(roundValues.begin(), roundValues.end(),
                       [](const RoundValues &round) { return round.holes == 0; }),
        roundValues.end());

    int holes = 0;
    int gross = 0;
    int par = 0;
    int putts = 0;
    int puttHoles = 0;
    int penalties = 0;
    int fairwaysHit = 0;
    int fairwaysRecorded = 0;
    int fairwaysLeft = 0;
    int fairwaysCentre = 0;
    int fairwaysRight = 0;
    int fairwaysMissed = 0;
    int greensHit = 0;
    int greensRecorded = 0;
    int albatrosses = 0;
    int eagles = 0;
    int birdies = 0;
    int pars = 0;
    int bogeys = 0;
    int doublesOrWorse = 0;
    double bestToPar = std::numeric_limits<double>::infinity();
    std::vector<double> normalizedRoundScores;
    QVariantList trend;
    for (const auto &round : roundValues) {
        holes += round.holes;
        gross += round.gross;
        par += round.par;
        putts += round.putts;
        puttHoles += round.puttHoles;
        penalties += round.penalties;
        fairwaysHit += round.fairwaysHit;
        fairwaysRecorded += round.fairwaysRecorded;
        fairwaysLeft += round.fairwaysLeft;
        fairwaysCentre += round.fairwaysCentre;
        fairwaysRight += round.fairwaysRight;
        fairwaysMissed += round.fairwaysMissed;
        greensHit += round.greensHit;
        greensRecorded += round.greensRecorded;
        albatrosses += round.albatrosses;
        eagles += round.eagles;
        birdies += round.birdies;
        pars += round.pars;
        bogeys += round.bogeys;
        doublesOrWorse += round.doublesOrWorse;
        if (round.validParHoles == round.holes) {
            const int toPar = round.gross - round.par;
            const double normalized =
                static_cast<double>(toPar) * 18.0 / static_cast<double>(round.holes);
            normalizedRoundScores.push_back(normalized);
            bestToPar = std::min(bestToPar, normalized);
            trend.push_back(QVariantMap{
                {QStringLiteral("roundId"), round.id},
                {QStringLiteral("courseSlug"), round.courseSlug},
                {QStringLiteral("courseName"), round.courseName},
                {QStringLiteral("startedAt"), round.startedAt},
                {QStringLiteral("gross"), round.gross},
                {QStringLiteral("par"), round.par},
                {QStringLiteral("toPar"), toPar},
                {QStringLiteral("normalizedToPar"), normalized},
            });
        }
    }
    while (trend.size() > 8)
        trend.removeFirst();

    double averageToPar = 0.0;
    double consistency = 0.0;
    if (!normalizedRoundScores.empty()) {
        for (const double value : normalizedRoundScores) {
            averageToPar += value;
        }
        averageToPar /= static_cast<double>(normalizedRoundScores.size());
        if (normalizedRoundScores.size() > 1) {
            for (const double value : normalizedRoundScores) {
                const double difference = value - averageToPar;
                consistency += difference * difference;
            }
            consistency = std::sqrt(consistency /
                                    static_cast<double>(normalizedRoundScores.size()));
        }
    }

    QSqlQuery longestDrive(m_database);
    QString longestDriveSql =
        QStringLiteral("SELECT MAX(s.distance_metres) FROM shots s "
                       "JOIN rounds r ON r.id=s.round_id "
                       "JOIN participants p ON p.id=s.participant_id AND "
                       "p.round_id=r.id AND p.profile_id=r.profile_id "
                       "WHERE r.status='completed' AND "
                       "(s.shot_type='drive' OR s.shot_type='tee')");
    if (!profileId.isEmpty()) {
        longestDriveSql += QStringLiteral(" AND r.profile_id=?");
    }
    if (!courseSlug.isEmpty()) {
        longestDriveSql += QStringLiteral(" AND r.course_slug=?");
    }
    longestDrive.prepare(longestDriveSql);
    if (!profileId.isEmpty())
        longestDrive.addBindValue(profileId);
    if (!courseSlug.isEmpty())
        longestDrive.addBindValue(courseSlug);
    const double longestDriveMetres =
        longestDrive.exec() && longestDrive.next() && !longestDrive.value(0).isNull()
            ? longestDrive.value(0).toDouble()
            : 0.0;

    ShotTypeCounts shotCounts;
    QSqlQuery shotTypes(m_database);
    QString shotTypesSql = QStringLiteral(
        "SELECT s.shot_type,COUNT(*) FROM shots s "
        "JOIN rounds r ON r.id=s.round_id "
        "JOIN participants p ON p.id=s.participant_id AND p.round_id=r.id "
        "AND p.profile_id=r.profile_id WHERE r.status='completed'");
    if (!profileId.isEmpty())
        shotTypesSql += QStringLiteral(" AND r.profile_id=?");
    if (!courseSlug.isEmpty())
        shotTypesSql += QStringLiteral(" AND r.course_slug=?");
    shotTypesSql += QStringLiteral(" GROUP BY s.shot_type");
    shotTypes.prepare(shotTypesSql);
    if (!profileId.isEmpty())
        shotTypes.addBindValue(profileId);
    if (!courseSlug.isEmpty())
        shotTypes.addBindValue(courseSlug);
    if (shotTypes.exec()) {
        while (shotTypes.next())
            shotCounts.add(shotTypes.value(0).toString(), shotTypes.value(1).toInt());
    }

    QSqlQuery weather(m_database);
    QString weatherSql =
        QStringLiteral("SELECT COUNT(*) FROM rounds WHERE status='completed' AND "
                       "weather_recorded_at IS NOT NULL");
    if (!profileId.isEmpty()) {
        weatherSql += QStringLiteral(" AND profile_id=?");
    }
    if (!courseSlug.isEmpty()) {
        weatherSql += QStringLiteral(" AND course_slug=?");
    }
    weather.prepare(weatherSql);
    if (!profileId.isEmpty())
        weather.addBindValue(profileId);
    if (!courseSlug.isEmpty())
        weather.addBindValue(courseSlug);
    const int weatherRounds =
        weather.exec() && weather.next() ? weather.value(0).toInt() : 0;

    const int outcomeTotal =
        albatrosses + eagles + birdies + pars + bogeys + doublesOrWorse;
    const auto outcome = [outcomeTotal](const QString &key, const int count) {
        return QVariantMap{
            {QStringLiteral("key"), key},
            {QStringLiteral("count"), count},
            {QStringLiteral("percentage"),
             outcomeTotal > 0 ? 100.0 * count / outcomeTotal : 0.0},
        };
    };
    const auto fairwayOutcome = [fairwaysRecorded](const QString &key,
                                                   const int count) {
        return QVariantMap{
            {QStringLiteral("key"), key},
            {QStringLiteral("count"), count},
            {QStringLiteral("percentage"),
             fairwaysRecorded > 0 ? 100.0 * count / fairwaysRecorded : 0.0},
        };
    };

    QVariantMap result{
        {QStringLiteral("courseSlug"), courseSlug},
        {QStringLiteral("rounds"), static_cast<int>(roundValues.size())},
        {QStringLiteral("toParRounds"), static_cast<int>(normalizedRoundScores.size())},
        {QStringLiteral("holes"), holes},
        {QStringLiteral("grossStrokes"), gross},
        {QStringLiteral("averageToPar"), averageToPar},
        {QStringLiteral("bestToPar"), std::isfinite(bestToPar) ? bestToPar : 0.0},
        {QStringLiteral("consistency"), consistency},
        {QStringLiteral("averagePutts"),
         puttHoles > 0 ? static_cast<double>(putts) / puttHoles : 0.0},
        {QStringLiteral("penalties"), penalties},
        {QStringLiteral("fairwayPct"),
         fairwaysRecorded > 0 ? 100.0 * fairwaysHit / fairwaysRecorded : 0.0},
        {QStringLiteral("girPct"),
         greensRecorded > 0 ? 100.0 * greensHit / greensRecorded : 0.0},
        {QStringLiteral("longestDriveMetres"), longestDriveMetres},
        {QStringLiteral("longestDriveRecorded"), longestDriveMetres > 0.0},
        {QStringLiteral("trackedStrokes"), shotCounts.total()},
        {QStringLiteral("scoredStrokes"), gross},
        {QStringLiteral("shotTypeDistribution"), shotCounts.distribution()},
        {QStringLiteral("puttsRecorded"), puttHoles},
        {QStringLiteral("fairwaysRecorded"), fairwaysRecorded},
        {QStringLiteral("fairwayDistribution"),
         QVariantList{
             fairwayOutcome(QStringLiteral("left"), fairwaysLeft),
             fairwayOutcome(QStringLiteral("centre"), fairwaysCentre),
             fairwayOutcome(QStringLiteral("right"), fairwaysRight),
             fairwayOutcome(QStringLiteral("missed"), fairwaysMissed),
         }},
        {QStringLiteral("greensRecorded"), greensRecorded},
        {QStringLiteral("weatherRounds"), weatherRounds},
        {QStringLiteral("trend"), trend},
        {QStringLiteral("scoreDistribution"),
         QVariantList{
             outcome(QStringLiteral("albatross"), albatrosses),
             outcome(QStringLiteral("eagle"), eagles),
             outcome(QStringLiteral("birdie"), birdies),
             outcome(QStringLiteral("par"), pars),
             outcome(QStringLiteral("bogey"), bogeys),
             outcome(QStringLiteral("double_or_worse"), doublesOrWorse),
         }},
    };

    if (courseSlug.isEmpty()) {
        QVariantList courses;
        QSqlQuery courseQuery(m_database);
        QString courseSql =
            QStringLiteral("SELECT course_slug,MAX(course_name) FROM rounds "
                           "WHERE status='completed'");
        if (!profileId.isEmpty()) {
            courseSql += QStringLiteral(" AND profile_id=?");
        }
        courseSql +=
            QStringLiteral(" GROUP BY course_slug ORDER BY MAX(started_at) DESC");
        courseQuery.prepare(courseSql);
        if (!profileId.isEmpty())
            courseQuery.addBindValue(profileId);
        if (courseQuery.exec()) {
            while (courseQuery.next()) {
                QVariantMap course =
                    overview(courseQuery.value(0).toString(), profileId);
                course.insert(QStringLiteral("courseName"), courseQuery.value(1));
                courses.push_back(course);
            }
        }
        result.insert(QStringLiteral("courses"), courses);
    }
    return result;
}

IntegrationRepository::IntegrationRepository(QSqlDatabase database)
    : m_database(std::move(database)) {}

bool IntegrationRepository::upsert(const IntegrationAccountState &state) {
    static const QSet<QString> statuses{
        QStringLiteral("unavailable"), QStringLiteral("available"),
        QStringLiteral("connected"), QStringLiteral("error")};
    static const QSet<QString> providers{
        QStringLiteral("garmin_golf"), QStringLiteral("garmin_connect"),
        QStringLiteral("trackman"), QStringLiteral("toptracer"),
        QStringLiteral("golfbox_no")};
    static const QSet<QString> knownCapabilities{
        QStringLiteral("rounds.read"),        QStringLiteral("scorecards.read"),
        QStringLiteral("shots.read"),         QStringLiteral("launch_metrics.read"),
        QStringLiteral("weather.read"),       QStringLiteral("activities.read"),
        QStringLiteral("files.fit.read"),     QStringLiteral("files.gpx.read"),
        QStringLiteral("files.tcx.read"),     QStringLiteral("sessions.read"),
        QStringLiteral("handicap.read"),      QStringLiteral("whs_scores.submit"),
        QStringLiteral("whs_scores.correct"), QStringLiteral("tee_times.search"),
        QStringLiteral("bookings.create"),    QStringLiteral("bookings.cancel"),
        QStringLiteral("csv.import"),
    };
    if (!providers.contains(state.provider) || !statuses.contains(state.status)) {
        return false;
    }
    QJsonArray capabilities;
    for (const auto &capability : state.reportedCapabilities) {
        if (!knownCapabilities.contains(capability))
            return false;
        capabilities.push_back(capability);
    }
    QSqlQuery query(m_database);
    query.prepare(QStringLiteral(
        "INSERT INTO integration_accounts(provider,status,external_user_id,"
        "display_name,capabilities_json,last_sync_at,last_error,updated_at)"
        " VALUES(?,?,?,?,?,?,?,?) "
        "ON CONFLICT(provider) DO UPDATE SET status=excluded.status,"
        "external_user_id=excluded.external_user_id,"
        "display_name=excluded.display_name,"
        "capabilities_json=excluded.capabilities_json,"
        "last_sync_at=excluded.last_sync_at,last_error=excluded.last_error,"
        "updated_at=excluded.updated_at"));
    query.addBindValue(state.provider);
    query.addBindValue(state.status);
    query.addBindValue(state.externalUserId.isEmpty() ? QVariant{}
                                                      : state.externalUserId);
    query.addBindValue(textOrEmpty(state.displayName));
    query.addBindValue(
        QString::fromUtf8(QJsonDocument(capabilities).toJson(QJsonDocument::Compact)));
    query.addBindValue(state.lastSyncAt.isEmpty() ? QVariant{} : state.lastSyncAt);
    query.addBindValue(textOrEmpty(state.lastError));
    query.addBindValue(nowIso());
    return query.exec();
}

QVariantList IntegrationRepository::list() const {
    QVariantList accounts;
    QSqlQuery query(m_database);
    if (!query.exec(
            QStringLiteral("SELECT provider,status,external_user_id,display_name,"
                           "capabilities_json,last_sync_at,last_error "
                           "FROM integration_accounts ORDER BY provider"))) {
        return accounts;
    }
    while (query.next()) {
        const auto capabilities = QJsonDocument::fromJson(query.value(4).toByteArray())
                                      .array()
                                      .toVariantList();
        accounts.push_back(QVariantMap{
            {QStringLiteral("provider"), query.value(0)},
            {QStringLiteral("status"), query.value(1)},
            {QStringLiteral("externalUserId"), query.value(2)},
            {QStringLiteral("displayName"), query.value(3)},
            {QStringLiteral("reportedCapabilities"), capabilities},
            {QStringLiteral("lastSyncAt"), query.value(5)},
            {QStringLiteral("lastError"), query.value(6)},
        });
    }
    return accounts;
}

CourseRepository::CourseRepository(QSqlDatabase database)
    : m_database(std::move(database)) {}

bool CourseRepository::install(const CachedCourse &course) {
    QSqlQuery query(m_database);
    query.prepare(QStringLiteral(
        "INSERT INTO cached_courses(slug,version,name,path,quality_score,"
        "byte_size,attribution,installed_at,last_used_at) VALUES(?,?,?,?,?,?,?,?,?) "
        "ON CONFLICT(slug,version) DO UPDATE SET name=excluded.name,"
        "path=excluded.path,quality_score=excluded.quality_score,"
        "byte_size=excluded.byte_size,attribution=excluded.attribution,"
        "installed_at=excluded.installed_at"));
    query.addBindValue(course.slug);
    query.addBindValue(course.version);
    query.addBindValue(course.name);
    query.addBindValue(course.path);
    query.addBindValue(course.qualityScore);
    query.addBindValue(course.byteSize);
    query.addBindValue(course.attribution);
    query.addBindValue(nowIso());
    query.addBindValue(nowIso());
    return query.exec();
}

QVariantList CourseRepository::list() const {
    QVariantList courses;
    QSqlQuery query(m_database);
    if (!query.exec(QStringLiteral(
            "SELECT c.slug,c.version,c.name,c.path,c.quality_score,c.byte_size,"
            "c.attribution,c.installed_at FROM cached_courses c "
            "WHERE c.version=(SELECT c2.version FROM cached_courses c2 "
            "WHERE c2.slug=c.slug ORDER BY c2.installed_at DESC LIMIT 1) "
            "ORDER BY c.name"))) {
        return courses;
    }
    while (query.next()) {
        courses.push_back(QVariantMap{
            {QStringLiteral("slug"), query.value(0)},
            {QStringLiteral("version"), query.value(1)},
            {QStringLiteral("name"), query.value(2)},
            {QStringLiteral("path"), query.value(3)},
            {QStringLiteral("qualityScore"), query.value(4)},
            {QStringLiteral("byteSize"), query.value(5)},
            {QStringLiteral("attribution"), query.value(6)},
            {QStringLiteral("installedAt"), query.value(7)},
        });
    }
    return courses;
}

std::optional<CachedCourse> CourseRepository::current(const QString &slug) const {
    QSqlQuery query(m_database);
    query.prepare(QStringLiteral(
        "SELECT slug,version,name,path,quality_score,byte_size,attribution "
        "FROM cached_courses WHERE slug=? ORDER BY installed_at DESC LIMIT 1"));
    query.addBindValue(slug);
    if (!query.exec() || !query.next())
        return std::nullopt;
    return CachedCourse{query.value(0).toString(), query.value(1).toString(),
                        query.value(2).toString(), query.value(3).toString(),
                        query.value(4).toInt(),    query.value(5).toLongLong(),
                        query.value(6).toString()};
}

bool CourseRepository::remove(const QString &slug, const QString &version) {
    QSqlQuery query(m_database);
    query.prepare(
        QStringLiteral("DELETE FROM cached_courses WHERE slug=? AND version=?"));
    query.addBindValue(slug);
    query.addBindValue(version);
    return query.exec() && query.numRowsAffected() == 1;
}

} // namespace opencaddie::storage
