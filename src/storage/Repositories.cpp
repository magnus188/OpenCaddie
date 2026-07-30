#include "storage/Repositories.h"

#include <QDateTime>
#include <QJsonArray>
#include <QJsonDocument>
#include <QSqlError>
#include <QSqlQuery>
#include <QUuid>

namespace opencaddie::storage {
namespace {
QString nowIso() { return QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs); }
QString uuid() { return QUuid::createUuid().toString(QUuid::WithoutBraces); }

QString scoringMode(const domain::ScoringMode mode) {
    return mode == domain::ScoringMode::Stableford ? QStringLiteral("stableford")
                                                    : QStringLiteral("stroke");
}

domain::ScoringMode scoringMode(const QString& value) {
    return value == QStringLiteral("stableford")
               ? domain::ScoringMode::Stableford
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

domain::FairwayResult fairway(const QString& value) {
    if (value == QStringLiteral("left")) return domain::FairwayResult::Left;
    if (value == QStringLiteral("centre")) return domain::FairwayResult::Centre;
    if (value == QStringLiteral("right")) return domain::FairwayResult::Right;
    if (value == QStringLiteral("missed")) return domain::FairwayResult::Missed;
    return domain::FairwayResult::NotRecorded;
}

bool execute(QSqlQuery& query) { return query.exec(); }
} // namespace

SettingsRepository::SettingsRepository(QSqlDatabase database)
    : m_database(std::move(database)) {}

QString SettingsRepository::value(const QString& key,
                                  const QString& fallback) const {
    QSqlQuery query(m_database);
    query.prepare(QStringLiteral("SELECT value FROM settings WHERE key=?"));
    query.addBindValue(key);
    return query.exec() && query.next() ? query.value(0).toString() : fallback;
}

bool SettingsRepository::setValue(const QString& key, const QString& value) {
    QSqlQuery query(m_database);
    query.prepare(QStringLiteral(
        "INSERT INTO settings(key,value,updated_at) VALUES(?,?,?) "
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
    if (!count.exec(QStringLiteral("SELECT 1 FROM profiles LIMIT 1"))) return false;
    if (count.next()) return true;

    QSqlQuery query(m_database);
    query.prepare(QStringLiteral(
        "INSERT INTO profiles(id,name,handicap,is_default,created_at)"
        " VALUES(?,?,?,?,?)"));
    query.addBindValue(QStringLiteral("local-player"));
    query.addBindValue(QStringLiteral("Player"));
    query.addBindValue(0.0);
    query.addBindValue(1);
    query.addBindValue(nowIso());
    return query.exec();
}

bool ClubRepository::ensureStarterBag() {
    const QString profileId = defaultProfileId();
    if (profileId.isEmpty()) return false;

    QSqlQuery count(m_database);
    count.prepare(QStringLiteral("SELECT COUNT(*) FROM clubs WHERE profile_id=?"));
    count.addBindValue(profileId);
    if (!count.exec() || !count.next()) return false;
    if (count.value(0).toInt() > 0) return true;

    static const std::pair<const char*, double> starterClubs[] = {
        {"Driver", 215.0},
        {"5 iron", 175.0},
        {"7 iron", 145.0},
        {"Pitching wedge", 105.0},
        {"Sand wedge", 75.0},
        {"Putter", 10.0},
    };
    if (!m_database.transaction()) return false;
    for (const auto& [name, carry] : starterClubs) {
        if (create(profileId, QString::fromLatin1(name), carry).isEmpty()) {
            m_database.rollback();
            return false;
        }
    }
    return m_database.commit();
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

std::vector<domain::Club> ClubRepository::list(const QString& profileId) const {
    std::vector<domain::Club> clubs;
    QSqlQuery query(m_database);
    query.prepare(QStringLiteral(
        "SELECT id,name,carry_metres,enabled,position FROM clubs "
        "WHERE profile_id=? ORDER BY position,name"));
    query.addBindValue(profileId);
    if (!query.exec()) return clubs;
    while (query.next()) {
        clubs.push_back({query.value(0).toString().toStdString(),
                         query.value(1).toString().toStdString(),
                         query.value(2).toDouble(), query.value(3).toBool(),
                         query.value(4).toInt()});
    }
    return clubs;
}

QString ClubRepository::create(const QString& profileId, const QString& name,
                               const double carryMetres) {
    const QString id = uuid();
    QSqlQuery position(m_database);
    position.prepare(QStringLiteral(
        "SELECT COALESCE(MAX(position),-1)+1 FROM clubs WHERE profile_id=?"));
    position.addBindValue(profileId);
    const int order = position.exec() && position.next() ? position.value(0).toInt() : 0;

    QSqlQuery query(m_database);
    query.prepare(QStringLiteral(
        "INSERT INTO clubs(id,profile_id,name,carry_metres,enabled,position,"
        "created_at,updated_at) VALUES(?,?,?,?,?,?,?,?)"));
    query.addBindValue(id);
    query.addBindValue(profileId);
    query.addBindValue(name.trimmed());
    query.addBindValue(carryMetres);
    query.addBindValue(1);
    query.addBindValue(order);
    query.addBindValue(nowIso());
    query.addBindValue(nowIso());
    return query.exec() ? id : QString{};
}

bool ClubRepository::update(const domain::Club& club) {
    QSqlQuery query(m_database);
    query.prepare(QStringLiteral(
        "UPDATE clubs SET name=?,carry_metres=?,enabled=?,position=?,"
        "updated_at=? WHERE id=?"));
    query.addBindValue(QString::fromStdString(club.name).trimmed());
    query.addBindValue(club.carryMetres);
    query.addBindValue(club.enabled);
    query.addBindValue(club.position);
    query.addBindValue(nowIso());
    query.addBindValue(QString::fromStdString(club.id));
    return query.exec() && query.numRowsAffected() == 1;
}

bool ClubRepository::remove(const QString& id) {
    QSqlQuery query(m_database);
    query.prepare(QStringLiteral("DELETE FROM clubs WHERE id=?"));
    query.addBindValue(id);
    return query.exec() && query.numRowsAffected() == 1;
}

bool ClubRepository::reorder(const QStringList& orderedIds) {
    if (!m_database.transaction()) return false;
    QSqlQuery query(m_database);
    query.prepare(QStringLiteral(
        "UPDATE clubs SET position=?,updated_at=? WHERE id=?"));
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
            "r.course_handicap,r.scoring_mode,r.current_hole "
            "FROM rounds r JOIN participants p ON p.round_id=r.id "
            "WHERE r.status='in_progress' ORDER BY r.started_at DESC LIMIT 1")) ||
        !query.next()) {
        return std::nullopt;
    }
    return ActiveRound{query.value(0).toString(), query.value(1).toString(),
                       query.value(2).toString(), query.value(3).toString(),
                       query.value(4).toInt(), query.value(5).toInt(),
                       scoringMode(query.value(6).toString()),
                       query.value(7).toInt()};
}

std::optional<ActiveRound> RoundRepository::start(const RoundStart& start) {
    if ((start.holeCount != 9 && start.holeCount != 18) ||
        !m_database.transaction()) {
        return std::nullopt;
    }
    const QString roundId = uuid();
    const QString participantId = uuid();
    QSqlQuery round(m_database);
    round.prepare(QStringLiteral(
        "INSERT INTO rounds(id,course_slug,course_name,course_version,profile_id,"
        "hole_count,scoring_mode,course_handicap,status,current_hole,started_at,tee)"
        " VALUES(?,?,?,?,?,?,?,?,?,?,?,?)"));
    round.addBindValue(roundId);
    round.addBindValue(start.courseSlug);
    round.addBindValue(start.courseName);
    round.addBindValue(start.courseVersion);
    round.addBindValue(start.profileId);
    round.addBindValue(start.holeCount);
    round.addBindValue(scoringMode(start.scoringMode));
    round.addBindValue(start.courseHandicap);
    round.addBindValue(QStringLiteral("in_progress"));
    round.addBindValue(1);
    round.addBindValue(nowIso());
    round.addBindValue(start.tee);
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
    return ActiveRound{roundId, participantId, start.courseSlug, start.courseName,
                       start.holeCount, start.courseHandicap, start.scoringMode, 1};
}

bool RoundRepository::saveScore(const ActiveRound& round,
                                const domain::HoleDefinition& hole,
                                const domain::HoleScore& score) {
    if (score.hole != hole.number || score.hole < 1 ||
        score.hole > round.holeCount || score.strokes < 0 ||
        !m_database.transaction()) {
        return false;
    }
    QSqlQuery query(m_database);
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
    if (!query.exec()) {
        m_database.rollback();
        return false;
    }
    return m_database.commit();
}

std::vector<domain::HoleScore>
RoundRepository::scores(const ActiveRound& round) const {
    std::vector<domain::HoleScore> values;
    QSqlQuery query(m_database);
    query.prepare(QStringLiteral(
        "SELECT hole,strokes,putts,penalties,fairway,gir,tee,notes "
        "FROM hole_scores WHERE round_id=? AND participant_id=? ORDER BY hole"));
    query.addBindValue(round.id);
    query.addBindValue(round.participantId);
    if (!query.exec()) return values;
    while (query.next()) {
        values.push_back(
            {query.value(0).toInt(),
             query.value(1).toInt(),
             query.value(2).isNull()
                 ? std::nullopt
                 : std::optional<int>{query.value(2).toInt()},
             query.value(3).toInt(),
             fairway(query.value(4).toString()),
             query.value(5).isNull()
                 ? std::nullopt
                 : std::optional<bool>{query.value(5).toBool()},
             query.value(6).toString().toStdString(),
             query.value(7).toString().toStdString()});
    }
    return values;
}

bool RoundRepository::setCurrentHole(const QString& roundId, const int hole) {
    QSqlQuery query(m_database);
    query.prepare(QStringLiteral(
        "UPDATE rounds SET current_hole=? WHERE id=? AND status='in_progress'"));
    query.addBindValue(hole);
    query.addBindValue(roundId);
    return query.exec() && query.numRowsAffected() == 1;
}

bool RoundRepository::finish(const QString& roundId) {
    return setStatus(roundId, QStringLiteral("completed"), true);
}

bool RoundRepository::abandon(const QString& roundId) {
    return setStatus(roundId, QStringLiteral("abandoned"), true);
}

bool RoundRepository::setStatus(const QString& roundId, const QString& status,
                                const bool completed) {
    QSqlQuery query(m_database);
    query.prepare(QStringLiteral(
        "UPDATE rounds SET status=?,completed_at=? WHERE id=?"));
    query.addBindValue(status);
    query.addBindValue(completed ? QVariant(nowIso()) : QVariant{});
    query.addBindValue(roundId);
    return query.exec() && query.numRowsAffected() == 1;
}

QVariantList RoundRepository::history(const QString& search) const {
    QVariantList rounds;
    QSqlQuery query(m_database);
    query.prepare(QStringLiteral(
        "SELECT id,course_name,status,started_at,completed_at,hole_count,"
        "scoring_mode FROM rounds WHERE course_name LIKE ? "
        "ORDER BY started_at DESC"));
    query.addBindValue(QStringLiteral("%%1%").arg(search));
    if (!query.exec()) return rounds;
    while (query.next()) {
        rounds.push_back(QVariantMap{
            {QStringLiteral("id"), query.value(0)},
            {QStringLiteral("courseName"), query.value(1)},
            {QStringLiteral("status"), query.value(2)},
            {QStringLiteral("startedAt"), query.value(3)},
            {QStringLiteral("completedAt"), query.value(4)},
            {QStringLiteral("holeCount"), query.value(5)},
            {QStringLiteral("scoringMode"), query.value(6)},
        });
    }
    return rounds;
}

QVariantMap RoundRepository::detail(const QString& roundId) const {
    QVariantMap result;
    QSqlQuery round(m_database);
    round.prepare(QStringLiteral(
        "SELECT course_name,status,started_at,completed_at,hole_count,"
        "scoring_mode,course_handicap,tee FROM rounds WHERE id=?"));
    round.addBindValue(roundId);
    if (!round.exec() || !round.next()) return result;
    result = {{QStringLiteral("id"), roundId},
              {QStringLiteral("courseName"), round.value(0)},
              {QStringLiteral("status"), round.value(1)},
              {QStringLiteral("startedAt"), round.value(2)},
              {QStringLiteral("completedAt"), round.value(3)},
              {QStringLiteral("holeCount"), round.value(4)},
              {QStringLiteral("scoringMode"), round.value(5)},
              {QStringLiteral("courseHandicap"), round.value(6)},
              {QStringLiteral("tee"), round.value(7)}};

    QVariantList scores;
    QSqlQuery score(m_database);
    score.prepare(QStringLiteral(
        "SELECT hole,par,stroke_index,strokes,putts,penalties,fairway,gir,tee,"
        "notes FROM hole_scores WHERE round_id=? ORDER BY hole"));
    score.addBindValue(roundId);
    if (score.exec()) {
        while (score.next()) {
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
    result.insert(QStringLiteral("scores"), scores);
    return result;
}

bool RoundRepository::remove(const QString& roundId) {
    QSqlQuery query(m_database);
    query.prepare(QStringLiteral("DELETE FROM rounds WHERE id=?"));
    query.addBindValue(roundId);
    return query.exec() && query.numRowsAffected() == 1;
}

QJsonObject RoundRepository::exportJson(const QString& roundId) const {
    return QJsonObject::fromVariantMap(detail(roundId));
}

QString RoundRepository::exportCsv(const QString& roundId) const {
    const auto data = detail(roundId);
    QString csv = QStringLiteral(
        "course,status,started_at,hole,par,index,strokes,putts,penalties,"
        "fairway,gir,tee,notes\n");
    const auto quote = [](QString value) {
        value.replace('"', QStringLiteral("\"\""));
        return QStringLiteral("\"%1\"").arg(value);
    };
    for (const auto& value : data.value(QStringLiteral("scores")).toList()) {
        const auto score = value.toMap();
        csv += QStringLiteral("%1,%2,%3,%4,%5,%6,%7,%8,%9,%10,%11,%12,%13\n")
                   .arg(quote(data.value(QStringLiteral("courseName")).toString()),
                        quote(data.value(QStringLiteral("status")).toString()),
                        quote(data.value(QStringLiteral("startedAt")).toString()),
                        score.value(QStringLiteral("hole")).toString(),
                        score.value(QStringLiteral("par")).toString(),
                        score.value(QStringLiteral("strokeIndex")).toString(),
                        score.value(QStringLiteral("strokes")).toString(),
                        score.value(QStringLiteral("putts")).toString(),
                        score.value(QStringLiteral("penalties")).toString(),
                        quote(score.value(QStringLiteral("fairway")).toString()),
                        score.value(QStringLiteral("gir")).toString(),
                        quote(score.value(QStringLiteral("tee")).toString()),
                        quote(score.value(QStringLiteral("notes")).toString()));
    }
    return csv;
}

CourseRepository::CourseRepository(QSqlDatabase database)
    : m_database(std::move(database)) {}

bool CourseRepository::install(const CachedCourse& course) {
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

std::optional<CachedCourse>
CourseRepository::current(const QString& slug) const {
    QSqlQuery query(m_database);
    query.prepare(QStringLiteral(
        "SELECT slug,version,name,path,quality_score,byte_size,attribution "
        "FROM cached_courses WHERE slug=? ORDER BY installed_at DESC LIMIT 1"));
    query.addBindValue(slug);
    if (!query.exec() || !query.next()) return std::nullopt;
    return CachedCourse{query.value(0).toString(), query.value(1).toString(),
                        query.value(2).toString(), query.value(3).toString(),
                        query.value(4).toInt(), query.value(5).toLongLong(),
                        query.value(6).toString()};
}

bool CourseRepository::remove(const QString& slug, const QString& version) {
    QSqlQuery query(m_database);
    query.prepare(
        QStringLiteral("DELETE FROM cached_courses WHERE slug=? AND version=?"));
    query.addBindValue(slug);
    query.addBindValue(version);
    return query.exec() && query.numRowsAffected() == 1;
}

} // namespace opencaddie::storage
