#pragma once

#include "courses/CoursePackageManager.h"
#include "courses/CourseProvider.h"
#include "domain/HoleSelector.h"
#include "platform/PowerProvider.h"
#include "positioning/PositionProvider.h"
#include "storage/Database.h"
#include "storage/Repositories.h"

#include <QMap>
#include <QObject>
#include <QTimer>
#include <QUrl>
#include <QVariantList>
#include <QVariantMap>

namespace opencaddie::ui {

class AppController final : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString screen READ screen WRITE setScreen NOTIFY screenChanged)
    Q_PROPERTY(QVariantList courses READ courses NOTIFY coursesChanged)
    Q_PROPERTY(QVariantList searchResults READ searchResults
                   NOTIFY searchResultsChanged)
    Q_PROPERTY(QVariantList clubs READ clubs NOTIFY clubsChanged)
    Q_PROPERTY(QVariantList history READ history NOTIFY historyChanged)
    Q_PROPERTY(QVariantMap roundDetail READ roundDetail NOTIFY historyChanged)
    Q_PROPERTY(QVariantList scorecard READ scorecard NOTIFY scorecardChanged)
    Q_PROPERTY(bool searching READ searching NOTIFY searchingChanged)
    Q_PROPERTY(bool downloading READ downloading NOTIFY downloadingChanged)
    Q_PROPERTY(double downloadProgress READ downloadProgress
                   NOTIFY downloadProgressChanged)
    Q_PROPERTY(bool hasActiveRound READ hasActiveRound NOTIFY roundChanged)
    Q_PROPERTY(QString courseName READ courseName NOTIFY roundChanged)
    Q_PROPERTY(QString courseSlug READ courseSlug NOTIFY roundChanged)
    Q_PROPERTY(QString selectedCourseSlug READ selectedCourseSlug
                   NOTIFY selectionChanged)
    Q_PROPERTY(QString selectedCourseName READ selectedCourseName
                   NOTIFY selectionChanged)
    Q_PROPERTY(QUrl mapSource READ mapSource NOTIFY roundChanged)
    Q_PROPERTY(int currentHole READ currentHole NOTIFY liveChanged)
    Q_PROPERTY(int holeCount READ holeCount NOTIFY roundChanged)
    Q_PROPERTY(int par READ par NOTIFY liveChanged)
    Q_PROPERTY(int strokeIndex READ strokeIndex NOTIFY liveChanged)
    Q_PROPERTY(int strokes READ strokes NOTIFY scoreChanged)
    Q_PROPERTY(int putts READ putts NOTIFY scoreChanged)
    Q_PROPERTY(int penalties READ penalties NOTIFY scoreChanged)
    Q_PROPERTY(QString fairway READ fairway NOTIFY scoreChanged)
    Q_PROPERTY(bool gir READ gir NOTIFY scoreChanged)
    Q_PROPERTY(QString notes READ notes NOTIFY scoreChanged)
    Q_PROPERTY(double frontDistance READ frontDistance NOTIFY liveChanged)
    Q_PROPERTY(double centreDistance READ centreDistance NOTIFY liveChanged)
    Q_PROPERTY(double backDistance READ backDistance NOTIFY liveChanged)
    Q_PROPERTY(QString clubAdvice READ clubAdvice NOTIFY liveChanged)
    Q_PROPERTY(QString clubDelta READ clubDelta NOTIFY liveChanged)
    Q_PROPERTY(QString gpsStatus READ gpsStatus NOTIFY liveChanged)
    Q_PROPERTY(double gpsAccuracy READ gpsAccuracy NOTIFY liveChanged)
    Q_PROPERTY(double playerX READ playerX NOTIFY liveChanged)
    Q_PROPERTY(double playerY READ playerY NOTIFY liveChanged)
    Q_PROPERTY(bool playerVisible READ playerVisible NOTIFY liveChanged)
    Q_PROPERTY(bool metric READ metric WRITE setMetric NOTIFY settingsChanged)
    Q_PROPERTY(QString language READ language WRITE setLanguage
                   NOTIFY settingsChanged)
    Q_PROPERTY(bool darkMode READ darkMode WRITE setDarkMode
                   NOTIFY settingsChanged)
    Q_PROPERTY(double textScale READ textScale WRITE setTextScale
                   NOTIFY settingsChanged)
    Q_PROPERTY(bool showAdvancedScores READ showAdvancedScores
                   WRITE setShowAdvancedScores NOTIFY settingsChanged)
    Q_PROPERTY(bool automaticHoleAdvance READ automaticHoleAdvance
                   WRITE setAutomaticHoleAdvance NOTIFY settingsChanged)
    Q_PROPERTY(double recommendationBias READ recommendationBias
                   WRITE setRecommendationBias NOTIFY settingsChanged)
    Q_PROPERTY(int cacheLimitMb READ cacheLimitMb WRITE setCacheLimitMb
                   NOTIFY settingsChanged)
    Q_PROPERTY(bool diagnosticLogging READ diagnosticLogging
                   WRITE setDiagnosticLogging NOTIFY settingsChanged)
    Q_PROPERTY(QVariantMap mapColors READ mapColors NOTIFY settingsChanged)
    Q_PROPERTY(QString openGolfMapServer READ openGolfMapServer
                   WRITE setOpenGolfMapServer NOTIFY settingsChanged)
    Q_PROPERTY(QString message READ message NOTIFY messageChanged)

public:
    AppController(storage::Database* database, courses::CourseProvider* provider,
                  positioning::PositionProvider* positionProvider,
                  platform::PowerProvider* powerProvider,
                  QString coursesRoot, QObject* parent = nullptr);

    bool initialize();

    [[nodiscard]] QString screen() const;
    void setScreen(const QString& screen);
    [[nodiscard]] QVariantList courses() const;
    [[nodiscard]] QVariantList searchResults() const;
    [[nodiscard]] QVariantList clubs() const;
    [[nodiscard]] QVariantList history() const;
    [[nodiscard]] QVariantMap roundDetail() const;
    [[nodiscard]] QVariantList scorecard() const;
    [[nodiscard]] bool searching() const;
    [[nodiscard]] bool downloading() const;
    [[nodiscard]] double downloadProgress() const;
    [[nodiscard]] bool hasActiveRound() const;
    [[nodiscard]] QString courseName() const;
    [[nodiscard]] QString courseSlug() const;
    [[nodiscard]] QString selectedCourseSlug() const;
    [[nodiscard]] QString selectedCourseName() const;
    [[nodiscard]] QUrl mapSource() const;
    [[nodiscard]] int currentHole() const;
    [[nodiscard]] int holeCount() const;
    [[nodiscard]] int par() const;
    [[nodiscard]] int strokeIndex() const;
    [[nodiscard]] int strokes() const;
    [[nodiscard]] int putts() const;
    [[nodiscard]] int penalties() const;
    [[nodiscard]] QString fairway() const;
    [[nodiscard]] bool gir() const;
    [[nodiscard]] QString notes() const;
    [[nodiscard]] double frontDistance() const;
    [[nodiscard]] double centreDistance() const;
    [[nodiscard]] double backDistance() const;
    [[nodiscard]] QString clubAdvice() const;
    [[nodiscard]] QString clubDelta() const;
    [[nodiscard]] QString gpsStatus() const;
    [[nodiscard]] double gpsAccuracy() const;
    [[nodiscard]] double playerX() const;
    [[nodiscard]] double playerY() const;
    [[nodiscard]] bool playerVisible() const;
    [[nodiscard]] bool metric() const;
    void setMetric(bool metric);
    [[nodiscard]] QString language() const;
    void setLanguage(const QString& language);
    [[nodiscard]] bool darkMode() const;
    void setDarkMode(bool darkMode);
    [[nodiscard]] double textScale() const;
    void setTextScale(double scale);
    [[nodiscard]] bool showAdvancedScores() const;
    void setShowAdvancedScores(bool visible);
    [[nodiscard]] bool automaticHoleAdvance() const;
    void setAutomaticHoleAdvance(bool enabled);
    [[nodiscard]] double recommendationBias() const;
    void setRecommendationBias(double displayUnits);
    [[nodiscard]] int cacheLimitMb() const;
    void setCacheLimitMb(int megabytes);
    [[nodiscard]] bool diagnosticLogging() const;
    void setDiagnosticLogging(bool enabled);
    [[nodiscard]] QVariantMap mapColors() const;
    [[nodiscard]] QString openGolfMapServer() const;
    void setOpenGolfMapServer(const QString& server);
    [[nodiscard]] QString message() const;

    Q_INVOKABLE void searchCourses(const QString& query);
    Q_INVOKABLE void downloadCourse(const QVariantMap& candidate);
    Q_INVOKABLE void removeCourse(const QString& slug, const QString& version);
    Q_INVOKABLE void prepareRound(const QString& slug);
    Q_INVOKABLE void startRound(const QString& slug, int holes,
                                bool stableford, int courseHandicap,
                                const QString& tee);
    Q_INVOKABLE void resumeRound();
    Q_INVOKABLE void abandonRound();
    Q_INVOKABLE void finishRound();
    Q_INVOKABLE void previousHole();
    Q_INVOKABLE void nextHole();
    Q_INVOKABLE void setHole(int hole);
    Q_INVOKABLE void changeStrokes(int delta);
    Q_INVOKABLE void changePutts(int delta);
    Q_INVOKABLE void changePenalties(int delta);
    Q_INVOKABLE void setFairway(const QString& value);
    Q_INVOKABLE void setGir(bool value);
    Q_INVOKABLE void setNotes(const QString& value);
    Q_INVOKABLE void addClub(const QString& name, double carryDisplayUnits);
    Q_INVOKABLE void updateClub(const QString& id, const QString& name,
                                double carryDisplayUnits, bool enabled);
    Q_INVOKABLE void removeClub(const QString& id);
    Q_INVOKABLE void reorderClubs(const QVariantList& ids);
    Q_INVOKABLE void refreshHistory(const QString& query = {});
    Q_INVOKABLE void selectHistoryRound(const QString& roundId);
    Q_INVOKABLE bool deleteRound(const QString& roundId);
    Q_INVOKABLE QString exportRound(const QString& roundId,
                                    const QString& format);
    Q_INVOKABLE void setMapColor(const QString& key, const QString& color);
    Q_INVOKABLE void resetSettings();
    Q_INVOKABLE QString distanceText(double metres) const;

signals:
    void screenChanged();
    void coursesChanged();
    void searchResultsChanged();
    void clubsChanged();
    void historyChanged();
    void selectionChanged();
    void scorecardChanged();
    void searchingChanged();
    void downloadingChanged();
    void downloadProgressChanged();
    void roundChanged();
    void liveChanged();
    void scoreChanged();
    void settingsChanged();
    void messageChanged();
    void languageChangeRequested(const QString& language);

private:
    struct HoleNavigation {
        int number = 1;
        int par = 4;
        int strokeIndex = 1;
        domain::GeoPoint centre;
        std::vector<domain::GeoPoint> polygon;
        domain::LocalProjection projection;
    };

    void reloadCourses();
    void reloadClubs();
    void loadSettings();
    void loadCourseData(const QString& packagePath);
    void loadDemoCourseData();
    QByteArray readAsset(const QString& path) const;
    void handlePosition(const domain::PositionFix& fix);
    void updateLiveData();
    void loadCurrentScore();
    void saveCurrentScore();
    void rebuildScorecard();
    HoleNavigation currentNavigation() const;
    domain::HoleDefinition currentDefinition() const;
    double toMetres(double displayUnits) const;
    double fromMetres(double metres) const;
    void showMessage(const QString& message);
    void saveSetting(const QString& key, const QString& value);

    storage::Database* m_database;
    courses::CourseProvider* m_provider;
    positioning::PositionProvider* m_positionProvider;
    platform::PowerProvider* m_powerProvider;
    storage::SettingsRepository m_settings;
    storage::ClubRepository m_clubs;
    storage::RoundRepository m_rounds;
    storage::CourseRepository m_courseRepository;
    courses::CoursePackageManager m_packages;
    domain::HoleSelector m_holeSelector;
    QTimer m_freshnessTimer;
    QTimer m_messageTimer;

    QString m_screen = QStringLiteral("WelcomeScreen");
    QVariantList m_courses;
    QVariantList m_searchResults;
    QVariantList m_clubValues;
    QVariantList m_history;
    QVariantMap m_roundDetail;
    QVariantList m_scorecard;
    bool m_searching = false;
    bool m_downloading = false;
    double m_downloadProgress = 0.0;
    std::optional<storage::ActiveRound> m_activeRound;
    QString m_courseName;
    QString m_courseSlug;
    QString m_selectedCourseSlug;
    QString m_selectedCourseName;
    QUrl m_mapSource;
    QMap<int, HoleNavigation> m_navigation;
    domain::PositionFix m_lastFix;
    double m_frontDistance = 0.0;
    double m_centreDistance = 0.0;
    double m_backDistance = 0.0;
    QString m_clubAdvice;
    QString m_clubDelta;
    QString m_gpsStatus;
    double m_gpsAccuracy = 0.0;
    double m_playerX = 0.0;
    double m_playerY = 0.0;
    bool m_playerVisible = false;
    domain::HoleScore m_currentScore;
    bool m_metric = true;
    QString m_language = QStringLiteral("en");
    bool m_darkMode = true;
    double m_textScale = 1.0;
    bool m_showAdvancedScores = true;
    bool m_automaticHoleAdvance = true;
    double m_recommendationBiasMetres = 0.0;
    int m_cacheLimitMb = 1024;
    bool m_diagnosticLogging = false;
    QVariantMap m_mapColors;
    QString m_openGolfMapServer = QStringLiteral("http://localhost:3000");
    QString m_message;
};

} // namespace opencaddie::ui
