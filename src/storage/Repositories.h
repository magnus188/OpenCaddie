#pragma once

#include "domain/Types.h"

#include <QJsonObject>
#include <QSqlDatabase>
#include <QString>
#include <QVariantList>
#include <QVariantMap>

#include <optional>
#include <vector>

namespace opencaddie::storage {

class SettingsRepository {
public:
    explicit SettingsRepository(QSqlDatabase database);
    QString value(const QString& key, const QString& fallback = {}) const;
    bool setValue(const QString& key, const QString& value);
    QVariantMap all() const;
    bool reset();

private:
    QSqlDatabase m_database;
};

class ClubRepository {
public:
    explicit ClubRepository(QSqlDatabase database);
    bool ensureDefaultProfile();
    bool ensureStarterBag();
    QString defaultProfileId() const;
    std::vector<domain::Club> list(const QString& profileId) const;
    QString create(const QString& profileId, const QString& name,
                   double carryMetres);
    bool update(const domain::Club& club);
    bool remove(const QString& id);
    bool reorder(const QStringList& orderedIds);

private:
    QSqlDatabase m_database;
};

struct RoundStart {
    QString courseSlug;
    QString courseName;
    QString courseVersion;
    QString profileId;
    int holeCount = 18;
    domain::ScoringMode scoringMode = domain::ScoringMode::StrokePlay;
    int courseHandicap = 0;
    QString tee;
};

struct ActiveRound {
    QString id;
    QString participantId;
    QString courseSlug;
    QString courseName;
    int holeCount = 18;
    int courseHandicap = 0;
    domain::ScoringMode scoringMode = domain::ScoringMode::StrokePlay;
    int currentHole = 1;
};

class RoundRepository {
public:
    explicit RoundRepository(QSqlDatabase database);
    std::optional<ActiveRound> active() const;
    std::optional<ActiveRound> start(const RoundStart& start);
    bool saveScore(const ActiveRound& round, const domain::HoleDefinition& hole,
                   const domain::HoleScore& score);
    std::vector<domain::HoleScore> scores(const ActiveRound& round) const;
    bool setCurrentHole(const QString& roundId, int hole);
    bool finish(const QString& roundId);
    bool abandon(const QString& roundId);
    QVariantList history(const QString& search = {}) const;
    QVariantMap detail(const QString& roundId) const;
    bool remove(const QString& roundId);
    QJsonObject exportJson(const QString& roundId) const;
    QString exportCsv(const QString& roundId) const;

private:
    bool setStatus(const QString& roundId, const QString& status,
                   bool completed);
    QSqlDatabase m_database;
};

struct CachedCourse {
    QString slug;
    QString version;
    QString name;
    QString path;
    int qualityScore = 0;
    qint64 byteSize = 0;
    QString attribution;
};

class CourseRepository {
public:
    explicit CourseRepository(QSqlDatabase database);
    bool install(const CachedCourse& course);
    QVariantList list() const;
    std::optional<CachedCourse> current(const QString& slug) const;
    bool remove(const QString& slug, const QString& version);

private:
    QSqlDatabase m_database;
};

} // namespace opencaddie::storage
