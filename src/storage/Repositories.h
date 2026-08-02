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
    QString value(const QString &key, const QString &fallback = {}) const;
    bool setValue(const QString &key, const QString &value);
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
    std::vector<domain::Club> list(const QString &profileId) const;
    QString create(const QString &profileId, const QString &name, double carryMetres);
    bool update(const domain::Club &club);
    bool remove(const QString &id);
    bool reorder(const QStringList &orderedIds);

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
    int handicapIndexScale = 18;
    std::optional<double> weatherTemperatureC;
    std::optional<double> weatherWindMps;
    std::optional<int> weatherWindDirectionDegrees;
    QString weatherCondition;
    QString weatherSource;
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
    std::optional<ActiveRound> start(const RoundStart &start);
    bool saveScore(const ActiveRound &round, const domain::HoleDefinition &hole,
                   const domain::HoleScore &score);
    std::vector<domain::HoleScore> scores(const ActiveRound &round) const;
    bool setCurrentHole(const QString &roundId, int hole);
    bool finish(const QString &roundId);
    bool abandon(const QString &roundId);
    QVariantList history(const QString &search = {},
                         const QString &profileId = {}) const;
    QVariantMap detail(const QString &roundId, const QString &profileId = {}) const;
    bool remove(const QString &roundId);
    QJsonObject exportJson(const QString &roundId) const;
    QString exportCsv(const QString &roundId) const;

  private:
    bool setStatus(const QString &roundId, const QString &status, bool completed);
    QSqlDatabase m_database;
};

class CourseAnalysisRepository {
  public:
    explicit CourseAnalysisRepository(QSqlDatabase database);
    bool saveLayups(const QString &courseSlug, int hole,
                    const QVariantList &points);
    QVariantList layups(const QString &courseSlug, int hole) const;
    int analyzedHoleCount(const QString &courseSlug) const;
    bool importToRound(const QString &courseSlug, const QString &roundId,
                       int holeCount);
    QVariantList roundLayups(const QString &roundId, int hole) const;

  private:
    QSqlDatabase m_database;
};

struct ShotRecord {
    struct Metric {
        QString key;
        double canonicalValue = 0.0;
        QString canonicalUnit;
        std::optional<double> sourceValue;
        QString sourceUnit;
    };

    QString id;
    QString roundId;
    QString participantId;
    int hole = 1;
    int sequence = 1;
    QString clubId;
    QString clubName;
    QString shotType;
    std::optional<double> startLatitude;
    std::optional<double> startLongitude;
    std::optional<double> endLatitude;
    std::optional<double> endLongitude;
    std::optional<double> distanceMetres;
    std::optional<double> lateralMetres;
    std::optional<double> accuracyMetres;
    QString result;
    QString sourceProvider = QStringLiteral("opencaddie");
    QString externalId;
    QString recordedAt;
    std::vector<Metric> metrics;
    bool replaceMetrics = false;
};

class ShotRepository {
  public:
    explicit ShotRepository(QSqlDatabase database);
    bool upsert(const ShotRecord &shot);

  private:
    QSqlDatabase m_database;
};

class StatisticsRepository {
  public:
    explicit StatisticsRepository(QSqlDatabase database);
    QVariantMap overview(const QString &courseSlug = {},
                         const QString &profileId = {}) const;
    QVariantList holePerformance(const QString &courseSlug,
                                 const QString &profileId = {}) const;

  private:
    QSqlDatabase m_database;
};

struct IntegrationAccountState {
    QString provider;
    QString status;
    QString externalUserId;
    QString displayName;
    // Informational cache only. Verified service-side token scopes authorize
    // operations; this mutable SQLite metadata never does.
    QStringList reportedCapabilities;
    QString lastSyncAt;
    QString lastError;
};

class IntegrationRepository {
  public:
    explicit IntegrationRepository(QSqlDatabase database);
    bool upsert(const IntegrationAccountState &state);
    QVariantList list() const;

  private:
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
    bool install(const CachedCourse &course);
    QVariantList list() const;
    std::optional<CachedCourse> current(const QString &slug) const;
    bool remove(const QString &slug, const QString &version);

  private:
    QSqlDatabase m_database;
};

} // namespace opencaddie::storage
