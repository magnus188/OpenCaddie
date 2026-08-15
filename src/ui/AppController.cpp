#include "ui/AppController.h"

#include "courses/OpenGolfMapProvider.h"
#include "domain/ClubRecommendation.h"
#include "domain/Geo.h"
#include "domain/Scoring.h"
#include "domain/Statistics.h"
#include "integrations/IntegrationCatalog.h"

#include <QColor>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>
#include <QStandardPaths>

#include <algorithm>
#include <cmath>
#include <unordered_map>

namespace opencaddie::ui {
namespace {
constexpr auto DemoSlug = "opencaddie-demo";
constexpr auto DemoResourceRoot = ":/qt/qml/OpenCaddie/assets/demo";

QString fairwayName(const domain::FairwayResult result) {
    switch (result) {
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

domain::FairwayResult fairwayValue(const QString &value) {
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

QUrl resourceUrl(const QString &relativePath) {
    return QUrl(
        QStringLiteral("qrc:/qt/qml/OpenCaddie/assets/demo/%1").arg(relativePath));
}
} // namespace

AppController::AppController(storage::Database *database,
                             courses::CourseProvider *provider,
                             positioning::PositionProvider *positionProvider,
                             platform::PowerProvider *powerProvider,
                             QString coursesRoot, QObject *parent)
    : QObject(parent), m_database(database), m_provider(provider),
      m_positionProvider(positionProvider), m_powerProvider(powerProvider),
      m_settings(database->connection()), m_clubs(database->connection()),
      m_rounds(database->connection()), m_courseAnalyses(database->connection()),
      m_statisticsRepository(database->connection()),
      m_courseRepository(database->connection()),
      m_packages(std::move(coursesRoot), m_courseRepository),
      m_holeSelector(1, 3, 35.0) {
    m_integrations = integrations::integrationCatalog();
    m_freshnessTimer.setInterval(1'000);
    connect(&m_freshnessTimer, &QTimer::timeout, this, &AppController::updateLiveData);
    m_messageTimer.setSingleShot(true);
    m_messageTimer.setInterval(4'000);
    connect(&m_messageTimer, &QTimer::timeout, this, [this] {
        m_message.clear();
        emit messageChanged();
    });

    connect(m_provider, &courses::CourseProvider::searchCompleted, this,
            [this](const QVariantList &results) {
                m_searching = false;
                m_searchResults = results;
                emit searchingChanged();
                emit searchResultsChanged();
            });
    connect(m_provider, &courses::CourseProvider::downloadProgress, this,
            [this](const qint64 received, const qint64 total) {
                m_downloadProgress = total > 0 ? static_cast<double>(received) /
                                                     static_cast<double>(total)
                                               : 0.0;
                emit downloadProgressChanged();
            });
    connect(m_provider, &courses::CourseProvider::bundleReady, this,
            [this](const QByteArray &bytes) {
                qint64 usedBytes = 0;
                for (const auto &value : m_courseRepository.list()) {
                    usedBytes +=
                        value.toMap().value(QStringLiteral("byteSize")).toLongLong();
                }
                const qint64 limitBytes =
                    static_cast<qint64>(m_cacheLimitMb) * 1024 * 1024;
                if (usedBytes + bytes.size() > limitBytes) {
                    m_downloading = false;
                    m_downloadProgress = 0.0;
                    emit downloadingChanged();
                    emit downloadProgressChanged();
                    showMessage(tr("Course cache limit reached. Remove a course "
                                   "or increase the limit in Settings."));
                    return;
                }
                QString error;
                const auto installed = m_packages.install(bytes, &error);
                m_downloading = false;
                m_downloadProgress = 0.0;
                emit downloadingChanged();
                emit downloadProgressChanged();
                if (!installed) {
                    showMessage(error);
                    return;
                }
                reloadCourses();
                showMessage(tr("%1 is ready offline.").arg(installed->name));
            });
    connect(m_provider, &courses::CourseProvider::errorOccurred, this,
            [this](const QString &error) {
                m_searching = false;
                m_downloading = false;
                emit searchingChanged();
                emit downloadingChanged();
                showMessage(error);
            });
    if (auto *remote = qobject_cast<courses::OpenGolfMapProvider *>(m_provider)) {
        connect(remote, &courses::OpenGolfMapProvider::reachableChanged, this,
                &AppController::courseServiceChanged);
    }
    connect(m_positionProvider, &positioning::PositionProvider::positionChanged, this,
            &AppController::handlePosition);
}

bool AppController::initialize() {
    if (!m_database || !m_database->isOpen() || !m_clubs.ensureDefaultProfile() ||
        !m_clubs.ensureStarterBag()) {
        return false;
    }
    loadSettings();
    reloadCourses();
    reloadClubs();
    refreshHistory();
    refreshStatistics();
    m_activeRound = m_rounds.active();
    if (m_activeRound) {
        m_courseSlug = m_activeRound->courseSlug;
        m_courseName = m_activeRound->courseName;
        if (m_courseSlug == QString::fromLatin1(DemoSlug)) {
            loadDemoCourseData();
        } else if (const auto course = m_courseRepository.current(m_courseSlug)) {
            loadCourseData(course->path);
        }
        m_holeSelector.selectManually(m_activeRound->currentHole);
        loadCurrentScore();
        rebuildScorecard();
    }
    m_freshnessTimer.start();
    emit languageChangeRequested(m_language);
    return true;
}

QString AppController::screen() const { return m_screen; }
QVariantList AppController::courses() const { return m_courses; }
QVariantList AppController::searchResults() const { return m_searchResults; }
QVariantList AppController::clubs() const { return m_clubValues; }
QVariantList AppController::history() const { return m_history; }
QVariantMap AppController::roundDetail() const { return m_roundDetail; }
QVariantMap AppController::statistics() const { return m_statistics; }
QVariantMap AppController::coursePlan() const { return m_coursePlan; }
QUrl AppController::coursePlanMapSource() const { return m_coursePlanMapSource; }
int AppController::plannerHole() const { return m_plannerHole; }
QVariantList AppController::courseAnalysisLayups() const {
    return m_courseAnalyses.layups(m_selectedCourseSlug, m_plannerHole);
}
bool AppController::selectedCourseHasAnalysis() const {
    return selectedCourseAnalyzedHoleCount() > 0;
}
int AppController::selectedCourseAnalyzedHoleCount() const {
    return m_courseAnalyses.analyzedHoleCount(m_selectedCourseSlug);
}
QVariantList AppController::integrations() const { return m_integrations; }
QVariantList AppController::scorecard() const { return m_scorecard; }
QString AppController::coursePickerMode() const { return m_coursePickerMode; }
bool AppController::searching() const { return m_searching; }
bool AppController::downloading() const { return m_downloading; }
double AppController::downloadProgress() const { return m_downloadProgress; }
bool AppController::hasActiveRound() const { return m_activeRound.has_value(); }
QString AppController::courseName() const { return m_courseName; }
QString AppController::courseSlug() const { return m_courseSlug; }
QString AppController::selectedCourseSlug() const { return m_selectedCourseSlug; }
QString AppController::selectedCourseName() const { return m_selectedCourseName; }
QUrl AppController::mapSource() const { return m_mapSource; }
int AppController::currentHole() const {
    return m_activeRound ? m_activeRound->currentHole : 1;
}
int AppController::holeCount() const {
    return m_activeRound ? m_activeRound->holeCount : 18;
}
int AppController::par() const { return currentNavigation().par; }
int AppController::strokeIndex() const { return currentNavigation().strokeIndex; }
int AppController::strokes() const { return m_currentScore.strokes; }
int AppController::putts() const { return m_currentScore.putts.value_or(0); }
int AppController::penalties() const { return m_currentScore.penalties; }
QString AppController::fairway() const { return fairwayName(m_currentScore.fairway); }
bool AppController::gir() const {
    return m_currentScore.greenInRegulation.value_or(false);
}
QString AppController::notes() const {
    return QString::fromStdString(m_currentScore.notes);
}
double AppController::frontDistance() const { return m_frontDistance; }
double AppController::centreDistance() const { return m_centreDistance; }
double AppController::backDistance() const { return m_backDistance; }
QString AppController::clubAdvice() const { return m_clubAdvice; }
QString AppController::clubDelta() const { return m_clubDelta; }
QString AppController::gpsStatus() const { return m_gpsStatus; }
double AppController::gpsAccuracy() const { return m_gpsAccuracy; }
double AppController::playerX() const { return m_playerX; }
double AppController::playerY() const { return m_playerY; }
bool AppController::playerVisible() const { return m_playerVisible; }
QVariantList AppController::roundLayups() const {
    return m_activeRound
               ? m_courseAnalyses.roundLayups(m_activeRound->id, currentHole())
               : QVariantList{};
}
bool AppController::metric() const { return m_metric; }
QString AppController::language() const { return m_language; }
bool AppController::darkMode() const { return m_darkMode; }
double AppController::textScale() const { return m_textScale; }
bool AppController::showAdvancedScores() const { return m_showAdvancedScores; }
bool AppController::automaticHoleAdvance() const { return m_automaticHoleAdvance; }
int AppController::courseHandicap() const { return m_courseHandicap; }
double AppController::recommendationBias() const {
    return fromMetres(m_recommendationBiasMetres);
}
int AppController::cacheLimitMb() const { return m_cacheLimitMb; }
bool AppController::celebrationsEnabled() const { return m_celebrationsEnabled; }
int AppController::brightness() const {
    return m_powerProvider ? m_powerProvider->brightness() : 80;
}
QVariantMap AppController::mapColors() const { return m_mapColors; }
QString AppController::openGolfMapServer() const { return m_openGolfMapServer; }
bool AppController::openGolfMapReachable() const {
    if (const auto *remote =
            qobject_cast<const courses::OpenGolfMapProvider *>(m_provider)) {
        return remote->reachable();
    }
    return true;
}
bool AppController::playsLikeAvailable() const { return m_playsLikeAvailable; }
double AppController::playsLikeDistance() const { return m_playsLike.totalMetres(); }
int AppController::playsLikeWindDelta() const {
    return qRound(fromMetres(m_playsLike.windMetres));
}
int AppController::playsLikeTemperatureDelta() const {
    return qRound(fromMetres(m_playsLike.temperatureMetres));
}
int AppController::playsLikeConditionDelta() const {
    return qRound(fromMetres(m_playsLike.conditionMetres));
}
int AppController::windRelativeDegrees() const { return m_windRelativeDegrees; }

domain::WeatherConditions AppController::activeWeather() const {
    domain::WeatherConditions weather;
    if (!m_activeRound)
        return weather;
    weather.temperatureC = m_activeRound->weatherTemperatureC;
    weather.windSpeedMps = m_activeRound->weatherWindMps;
    weather.windFromDegrees = m_activeRound->weatherWindDirectionDegrees;
    weather.condition = m_activeRound->weatherCondition.toStdString();
    return weather;
}

bool AppController::weatherAvailable() const {
    return m_activeRound &&
           (m_activeRound->weatherTemperatureC || m_activeRound->weatherWindMps ||
            m_activeRound->weatherWindDirectionDegrees ||
            !m_activeRound->weatherCondition.isEmpty());
}
double AppController::weatherWindMps() const {
    return m_activeRound ? m_activeRound->weatherWindMps.value_or(0.0) : 0.0;
}
double AppController::weatherTemperatureC() const {
    return m_activeRound ? m_activeRound->weatherTemperatureC.value_or(0.0) : 0.0;
}
QString AppController::weatherCondition() const {
    return m_activeRound ? m_activeRound->weatherCondition : QString();
}

QString AppController::message() const { return m_message; }
QString AppController::celebrationKind() const { return m_celebrationKind; }
int AppController::celebrationSequence() const { return m_celebrationSequence; }

bool AppController::isAllowedScreen(const QString &screen) {
    static const QSet<QString> allowed{
        QStringLiteral("WelcomeScreen"),
        QStringLiteral("CourseLibraryScreen"),
        QStringLiteral("CourseSearchScreen"),
        QStringLiteral("CoursePlannerScreen"),
        QStringLiteral("CoursePlannerMapScreen"),
        QStringLiteral("RoundSetupScreen"),
        QStringLiteral("LiveHoleScreen"),
        QStringLiteral("HoleScoreScreen"),
        QStringLiteral("RoundMapScreen"),
        QStringLiteral("ScorecardScreen"),
        QStringLiteral("HistoryScreen"),
        QStringLiteral("RoundDetailScreen"),
        QStringLiteral("StatsScreen"),
        QStringLiteral("BagScreen"),
        QStringLiteral("SettingsScreen"),
        QStringLiteral("SettingsDisplayScreen"),
        QStringLiteral("SettingsRoundScreen"),
        QStringLiteral("SettingsMapScreen"),
        QStringLiteral("SettingsConnectivityScreen"),
        QStringLiteral("SettingsIntegrationsScreen"),
        QStringLiteral("WifiScreen"),
    };
    return allowed.contains(screen);
}

void AppController::setScreen(const QString &screen) {
    resetNavigation({}, screen);
}

bool AppController::canGoBack() const { return !m_backStack.isEmpty(); }
int AppController::navigationDirection() const { return m_navigationDirection; }

void AppController::navigateTo(const QString &screen) {
    QString target = screen;
    if (target == QStringLiteral("RoundDetailScreen") && m_roundDetail.isEmpty()) {
        if (m_history.isEmpty()) {
            target = QStringLiteral("HistoryScreen");
        } else {
            selectHistoryRound(
                m_history.constFirst().toMap().value(QStringLiteral("id")).toString());
        }
    }
    if (!isAllowedScreen(target) || m_screen == target)
        return;
    if (!m_backStack.isEmpty() && m_backStack.constLast() == target) {
        goBack();
        return;
    }
    m_backStack.append(m_screen);
    m_navigationDirection = 1;
    m_screen = target;
    emit screenChanged();
    if (target == QStringLiteral("StatsScreen"))
        refreshStatistics();
}

void AppController::goBack() {
    if (m_backStack.isEmpty()) {
        const QString root = m_activeRound ? QStringLiteral("LiveHoleScreen")
                                           : QStringLiteral("WelcomeScreen");
        if (m_screen != root)
            resetNavigation({}, root);
        return;
    }
    m_navigationDirection = -1;
    m_screen = m_backStack.takeLast();
    emit screenChanged();
}

void AppController::resetNavigation(QStringList stack, const QString &current) {
    if (!isAllowedScreen(current))
        return;
    if (m_screen == current && m_backStack == stack)
        return;
    m_backStack = std::move(stack);
    m_navigationDirection = 0;
    if (m_screen != current) {
        m_screen = current;
        emit screenChanged();
        if (current == QStringLiteral("StatsScreen"))
            refreshStatistics();
    } else {
        emit screenChanged();
    }
}

void AppController::reloadCourses() {
    m_courses = {
        QVariantMap{
            {QStringLiteral("slug"), QString::fromLatin1(DemoSlug)},
            {QStringLiteral("version"), QStringLiteral("bundled-v1")},
            {QStringLiteral("name"), tr("OpenCaddie Demo Course")},
            {QStringLiteral("path"), QString()},
            {QStringLiteral("qualityScore"), 100},
            {QStringLiteral("byteSize"), 0},
            {QStringLiteral("attribution"),
             QStringLiteral("© OpenStreetMap contributors (ODbL)")},
            {QStringLiteral("installedAt"), tr("Bundled")},
        },
    };
    m_courses.append(m_courseRepository.list());
    emit coursesChanged();
}

void AppController::reloadClubs() {
    m_clubValues.clear();
    for (const auto &club : m_clubs.list(m_clubs.defaultProfileId())) {
        m_clubValues.push_back(QVariantMap{
            {QStringLiteral("id"), QString::fromStdString(club.id)},
            {QStringLiteral("name"), QString::fromStdString(club.name)},
            {QStringLiteral("carryMetres"), club.carryMetres},
            {QStringLiteral("carry"), fromMetres(club.carryMetres)},
            {QStringLiteral("unit"), m_metric ? tr("m") : tr("yd")},
            {QStringLiteral("enabled"), club.enabled},
            {QStringLiteral("position"), club.position},
        });
    }
    emit clubsChanged();
}

void AppController::loadSettings() {
    m_metric = m_settings.value(QStringLiteral("units"), QStringLiteral("metric")) ==
               QStringLiteral("metric");
    m_language = m_settings.value(QStringLiteral("language"), QStringLiteral("en"));
    m_darkMode = m_settings.value(QStringLiteral("palette"), QStringLiteral("dark")) ==
                 QStringLiteral("dark");
    m_textScale =
        m_settings.value(QStringLiteral("textScale"), QStringLiteral("1.0")).toDouble();
    m_showAdvancedScores =
        m_settings.value(QStringLiteral("showAdvancedScores"),
                         QStringLiteral("true")) == QStringLiteral("true");
    m_automaticHoleAdvance =
        m_settings.value(QStringLiteral("automaticHoleAdvance"),
                         QStringLiteral("true")) == QStringLiteral("true");
    m_courseHandicap = std::clamp(
        m_settings.value(QStringLiteral("courseHandicap"), QStringLiteral("0"))
            .toInt(),
        -10, 54);
    m_recommendationBiasMetres =
        m_settings
            .value(QStringLiteral("recommendationBiasMetres"), QStringLiteral("0"))
            .toDouble();
    m_cacheLimitMb = std::clamp(
        m_settings.value(QStringLiteral("cacheLimitMb"), QStringLiteral("1024"))
            .toInt(),
        128, 8192);
    m_celebrationsEnabled =
        m_settings.value(QStringLiteral("celebrationsEnabled"),
                         QStringLiteral("true")) == QStringLiteral("true");
    if (m_powerProvider) {
        m_powerProvider->setBrightness(std::clamp(
            m_settings.value(QStringLiteral("brightness"), QStringLiteral("80"))
                .toInt(),
            10, 100));
    }
    m_openGolfMapServer = m_settings.value(QStringLiteral("openGolfMapServer"),
                                           QStringLiteral("http://localhost:3000"));
    m_mapColors = {
        {QStringLiteral("rough"),
         m_settings.value(QStringLiteral("map.rough"), QStringLiteral("#315C35"))},
        {QStringLiteral("fairway"),
         m_settings.value(QStringLiteral("map.fairway"), QStringLiteral("#2FCB63"))},
        {QStringLiteral("green"),
         m_settings.value(QStringLiteral("map.green"), QStringLiteral("#8ED66B"))},
        {QStringLiteral("tee"),
         m_settings.value(QStringLiteral("map.tee"), QStringLiteral("#70B85B"))},
        {QStringLiteral("bunker"),
         m_settings.value(QStringLiteral("map.bunker"), QStringLiteral("#E0C27A"))},
        {QStringLiteral("water"),
         m_settings.value(QStringLiteral("map.water"), QStringLiteral("#2BA7D7"))},
        {QStringLiteral("wood"),
         m_settings.value(QStringLiteral("map.wood"), QStringLiteral("#1A5B35"))},
        {QStringLiteral("path"),
         m_settings.value(QStringLiteral("map.path"), QStringLiteral("#8B8174"))},
        {QStringLiteral("hole_line"), QStringLiteral("#F7F8F2")},
        {QStringLiteral("pin"), QStringLiteral("#D94D3E")},
    };
    if (auto *remote = qobject_cast<courses::OpenGolfMapProvider *>(m_provider)) {
        remote->setBaseUrl(QUrl(m_openGolfMapServer));
        remote->checkReachability();
    }
}

void AppController::searchCourses(const QString &query) {
    m_searching = true;
    m_searchResults.clear();
    emit searchingChanged();
    emit searchResultsChanged();
    m_provider->search(query);
}

void AppController::downloadCourse(const QVariantMap &candidate) {
    if (m_downloading)
        return;
    m_downloading = true;
    m_downloadProgress = 0.0;
    emit downloadingChanged();
    emit downloadProgressChanged();
    m_provider->fetchBundle(candidate, m_metric ? QStringLiteral("meters")
                                                : QStringLiteral("yards"));
}

void AppController::removeCourse(const QString &slug, const QString &version) {
    if (slug == QString::fromLatin1(DemoSlug))
        return;
    QString error;
    if (!m_packages.remove(slug, version, &error)) {
        showMessage(error);
        return;
    }
    reloadCourses();
    showMessage(tr("Offline course removed."));
}

void AppController::openCoursePicker(const QString &mode) {
    const QString resolved = mode == QStringLiteral("plan")
                                 ? QStringLiteral("plan")
                                 : QStringLiteral("start");
    if (resolved != m_coursePickerMode) {
        m_coursePickerMode = resolved;
        emit coursePickerModeChanged();
    }
    navigateTo(QStringLiteral("CourseLibraryScreen"));
}

void AppController::activateCourse(const QString &slug) {
    if (m_coursePickerMode == QStringLiteral("plan")) {
        planCourse(slug);
    } else {
        prepareRound(slug);
    }
}

void AppController::planCourse(const QString &slug) {
    const auto iterator =
        std::ranges::find_if(m_courses, [&slug](const QVariant &value) {
            return value.toMap().value(QStringLiteral("slug")).toString() == slug;
        });
    if (iterator == m_courses.end())
        return;

    const QVariantMap selected = iterator->toMap();
    m_selectedCourseSlug = slug;
    m_selectedCourseName = selected.value(QStringLiteral("name")).toString();
    emit selectionChanged();
    emit courseAnalysisChanged();
    const QMap<int, HoleNavigation> liveNavigation = m_navigation;
    const QUrl liveMapSource = m_mapSource;
    if (slug == QString::fromLatin1(DemoSlug)) {
        loadDemoCourseData();
    } else {
        loadCourseData(selected.value(QStringLiteral("path")).toString());
    }
    if (m_navigation.isEmpty() || m_mapSource.isEmpty()) {
        showMessage(tr("Course map data is invalid."));
        return;
    }
    m_coursePlanMapSource = m_mapSource;
    if (m_plannerHole != 1) {
        m_plannerHole = 1;
        emit plannerHoleChanged();
    }
    rebuildCoursePlan(selected);
    if (m_activeRound) {
        m_navigation = liveNavigation;
        m_mapSource = liveMapSource;
        emit roundChanged();
    }
    navigateTo(QStringLiteral("CoursePlannerScreen"));
}

void AppController::prepareRound(const QString &slug) {
    const auto iterator =
        std::ranges::find_if(m_courses, [&slug](const QVariant &value) {
            return value.toMap().value(QStringLiteral("slug")).toString() == slug;
        });
    if (iterator == m_courses.end())
        return;
    m_selectedCourseSlug = slug;
    m_selectedCourseName = iterator->toMap().value(QStringLiteral("name")).toString();
    emit selectionChanged();
    emit courseAnalysisChanged();
    navigateTo(QStringLiteral("RoundSetupScreen"));
}

void AppController::startRound(const QString &slug, const int holes,
                               const bool stableford, const int courseHandicap,
                               const QString &tee,
                               const bool importCourseAnalysis) {
    if (m_activeRound) {
        showMessage(tr("Finish or abandon the active round first."));
        return;
    }
    const auto iterator =
        std::ranges::find_if(m_courses, [&slug](const QVariant &value) {
            return value.toMap().value(QStringLiteral("slug")).toString() == slug;
        });
    if (iterator == m_courses.end()) {
        showMessage(tr("The selected course is not available offline."));
        return;
    }
    const QVariantMap selected = iterator->toMap();
    m_courseSlug = slug;
    m_courseName = selected.value(QStringLiteral("name")).toString();
    if (slug == QString::fromLatin1(DemoSlug)) {
        loadDemoCourseData();
    } else {
        loadCourseData(selected.value(QStringLiteral("path")).toString());
    }
    if (m_navigation.isEmpty() || m_mapSource.isEmpty()) {
        showMessage(tr("Course map data is invalid."));
        return;
    }
    int handicapIndexScale = holes == 9 ? 9 : 18;
    if (stableford) {
        std::vector<domain::HoleDefinition> definitions;
        for (int hole = 1; hole <= holes; ++hole) {
            if (!m_navigation.contains(hole)) {
                showMessage(
                    tr("Stableford needs valid par and stroke index for every hole."));
                return;
            }
            definitions.push_back(
                {hole, m_navigation[hole].par, m_navigation[hole].strokeIndex});
        }
        if (!domain::canUseHandicapScoring(definitions)) {
            showMessage(tr("Correct missing par/index data before using Stableford."));
            return;
        }
        handicapIndexScale = domain::handicapIndexScale(definitions);
    }
    storage::RoundStart roundStart{
        slug,
        m_courseName,
        selected.value(QStringLiteral("version")).toString(),
        m_clubs.defaultProfileId(),
        holes,
        stableford ? domain::ScoringMode::Stableford : domain::ScoringMode::StrokePlay,
        courseHandicap,
        tee,
    };
    roundStart.handicapIndexScale = handicapIndexScale;
    if (slug == QString::fromLatin1(DemoSlug)) {
        roundStart.weatherTemperatureC = 17.0;
        roundStart.weatherWindMps = 4.2;
        roundStart.weatherWindDirectionDegrees = 225;
        roundStart.weatherCondition = QStringLiteral("partly_cloudy");
        roundStart.weatherSource = QStringLiteral("simulator");
    }
    const auto started = m_rounds.start(roundStart);
    if (!started) {
        showMessage(tr("Could not start the round."));
        return;
    }
    if (importCourseAnalysis &&
        !m_courseAnalyses.importToRound(slug, started->id, holes)) {
        static_cast<void>(m_rounds.remove(started->id));
        showMessage(tr("Could not import the course analysis."));
        return;
    }
    m_activeRound = started;
    m_celebratedHoles.clear();
    m_nearGreenTrigger.reset(1);
    m_holeSelector.selectManually(1);
    loadCurrentScore();
    rebuildScorecard();
    emit roundChanged();
    emit liveChanged();
    setScreen(QStringLiteral("LiveHoleScreen"));
}

void AppController::resumeRound() {
    if (!m_activeRound)
        return;
    if (m_courseSlug == QString::fromLatin1(DemoSlug)) {
        loadDemoCourseData();
    } else if (const auto course = m_courseRepository.current(m_courseSlug)) {
        loadCourseData(course->path);
    }
    loadCurrentScore();
    updateLiveData();
    setScreen(QStringLiteral("LiveHoleScreen"));
}

void AppController::abandonRound() {
    if (!m_activeRound)
        return;
    if (!m_rounds.abandon(m_activeRound->id)) {
        showMessage(tr("Could not abandon the round."));
        return;
    }
    m_activeRound.reset();
    refreshHistory();
    emit roundChanged();
    setScreen(QStringLiteral("WelcomeScreen"));
}

void AppController::finishRound() {
    if (!m_activeRound)
        return;
    celebrateCurrentHole();
    saveCurrentScore();
    const QString finishedRoundId = m_activeRound->id;
    if (!m_rounds.finish(finishedRoundId)) {
        showMessage(tr("Could not finish the round."));
        return;
    }
    m_activeRound.reset();
    refreshHistory();
    refreshStatistics();
    selectHistoryRound(finishedRoundId);
    emit roundChanged();
    showMessage(tr("Round saved."));
    resetNavigation({QStringLiteral("WelcomeScreen"), QStringLiteral("HistoryScreen")},
                    QStringLiteral("RoundDetailScreen"));
}

void AppController::previousHole() {
    if (m_activeRound)
        setHole(std::max(1, currentHole() - 1));
}

void AppController::nextHole() {
    if (!m_activeRound)
        return;
    if (currentHole() >= holeCount())
        return;
    setHole(currentHole() + 1);
}

void AppController::setHole(const int hole) {
    if (!m_activeRound || hole < 1 || hole > m_activeRound->holeCount)
        return;
    if (hole != currentHole())
        celebrateCurrentHole();
    saveCurrentScore();
    if (!m_rounds.setCurrentHole(m_activeRound->id, hole))
        return;
    m_activeRound->currentHole = hole;
    m_holeSelector.selectManually(hole);
    m_nearGreenTrigger.reset(hole);
    loadCurrentScore();
    updateLiveData();
    emit liveChanged();
    emit roundChanged();
}

void AppController::changeStrokes(const int delta) {
    if (!m_activeRound)
        return;
    m_currentScore.strokes = std::clamp(m_currentScore.strokes + delta, 0, 20);
    saveCurrentScore();
}

void AppController::changePutts(const int delta) {
    if (!m_activeRound)
        return;
    m_currentScore.putts = std::clamp(m_currentScore.putts.value_or(0) + delta, 0, 12);
    saveCurrentScore();
}

void AppController::changePenalties(const int delta) {
    if (!m_activeRound)
        return;
    m_currentScore.penalties = std::clamp(m_currentScore.penalties + delta, 0, 12);
    saveCurrentScore();
}

void AppController::setFairway(const QString &value) {
    m_currentScore.fairway = fairwayValue(value);
    saveCurrentScore();
}

void AppController::setGir(const bool value) {
    m_currentScore.greenInRegulation = value;
    saveCurrentScore();
}

void AppController::setNotes(const QString &value) {
    m_currentScore.notes = value.left(1'000).toStdString();
    saveCurrentScore();
}

bool AppController::saveHoleScore(const int strokes, const int putts,
                                  const int penalties, const QString &fairway,
                                  const bool gir, const QString &notes) {
    if (!m_activeRound) return false;
    m_currentScore.hole = currentHole();
    m_currentScore.strokes = std::clamp(strokes, 1, 20);
    m_currentScore.putts = std::clamp(putts, 0, 12);
    m_currentScore.penalties = std::clamp(penalties, 0, 12);
    m_currentScore.fairway = fairwayValue(fairway);
    m_currentScore.greenInRegulation = gir;
    m_currentScore.notes = notes.left(1'000).toStdString();
    const bool saved = saveCurrentScore();
    if (saved) {
        static_cast<void>(m_nearGreenTrigger.update(
            currentHole(), m_centreDistance, true, true));
    }
    return saved;
}

void AppController::addClub(const QString &name, const double carryDisplayUnits) {
    if (name.trimmed().isEmpty() || carryDisplayUnits <= 0.0)
        return;
    if (m_clubs.create(m_clubs.defaultProfileId(), name, toMetres(carryDisplayUnits))
            .isEmpty()) {
        showMessage(tr("Could not add the club."));
        return;
    }
    reloadClubs();
    updateLiveData();
}

void AppController::updateClub(const QString &id, const QString &name,
                               const double carryDisplayUnits, const bool enabled) {
    auto values = m_clubs.list(m_clubs.defaultProfileId());
    const auto iterator = std::ranges::find_if(values, [&id](const auto &club) {
        return QString::fromStdString(club.id) == id;
    });
    if (iterator == values.end())
        return;
    iterator->name = name.toStdString();
    iterator->carryMetres = toMetres(carryDisplayUnits);
    iterator->enabled = enabled;
    if (!m_clubs.update(*iterator))
        showMessage(tr("Could not update the club."));
    reloadClubs();
    updateLiveData();
}

void AppController::removeClub(const QString &id) {
    if (!m_clubs.remove(id))
        showMessage(tr("Could not remove the club."));
    reloadClubs();
    updateLiveData();
}

void AppController::reorderClubs(const QVariantList &ids) {
    QStringList ordered;
    for (const auto &id : ids)
        ordered.push_back(id.toString());
    if (!m_clubs.reorder(ordered))
        showMessage(tr("Could not reorder clubs."));
    reloadClubs();
}

void AppController::refreshHistory(const QString &query) {
    m_history = m_rounds.history(query, m_clubs.defaultProfileId());
    emit historyChanged();
}

void AppController::refreshStatistics(const QString &courseSlug) {
    const QVariantList courses = m_statistics.value(QStringLiteral("courses")).toList();
    m_statistics =
        m_statisticsRepository.overview(courseSlug, m_clubs.defaultProfileId());
    if (!courseSlug.isEmpty() && !courses.isEmpty()) {
        m_statistics.insert(QStringLiteral("courses"), courses);
    }
    emit statisticsChanged();
}

void AppController::selectHistoryRound(const QString &roundId) {
    m_roundDetail = m_rounds.detail(roundId, m_clubs.defaultProfileId());
    emit historyChanged();
}

bool AppController::deleteRound(const QString &roundId) {
    if (m_activeRound && m_activeRound->id == roundId) {
        showMessage(tr("Finish or abandon the active round first."));
        return false;
    }
    const bool removed = m_rounds.remove(roundId);
    if (removed) {
        refreshHistory();
        refreshStatistics();
        if (m_roundDetail.value(QStringLiteral("id")).toString() == roundId) {
            m_roundDetail.clear();
            emit historyChanged();
        }
    }
    return removed;
}

QString AppController::exportRound(const QString &roundId, const QString &format) {
    const QString exportRoot =
        QDir(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation))
            .filePath(QStringLiteral("OpenCaddie Exports"));
    if (!QDir().mkpath(exportRoot))
        return {};
    const QString extension = format.toLower() == QStringLiteral("csv")
                                  ? QStringLiteral("csv")
                                  : QStringLiteral("json");
    const QString path =
        QDir(exportRoot)
            .filePath(QStringLiteral("round-%1.%2").arg(roundId, extension));
    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly))
        return {};
    const QByteArray content = extension == QStringLiteral("csv")
                                   ? m_rounds.exportCsv(roundId).toUtf8()
                                   : QJsonDocument(m_rounds.exportJson(roundId))
                                         .toJson(QJsonDocument::Indented);
    if (file.write(content) != content.size() || !file.commit())
        return {};
    showMessage(tr("Exported to %1").arg(path));
    return path;
}

void AppController::setMetric(const bool metric) {
    if (m_metric == metric)
        return;
    m_metric = metric;
    saveSetting(QStringLiteral("units"),
                metric ? QStringLiteral("metric") : QStringLiteral("imperial"));
    reloadClubs();
    updateLiveData();
    emit settingsChanged();
}

bool AppController::saveCourseAnalysis(const int hole,
                                       const QVariantList &points) {
    if (m_selectedCourseSlug.isEmpty() || points.size() < 2 ||
        !m_courseAnalyses.saveLayups(m_selectedCourseSlug, hole, points)) {
        showMessage(tr("Could not save the hole analysis."));
        return false;
    }
    emit courseAnalysisChanged();
    showMessage(tr("Analysis saved for hole %1.").arg(hole));
    return true;
}

bool AppController::clearCourseAnalysis(const int hole) {
    if (m_selectedCourseSlug.isEmpty() ||
        !m_courseAnalyses.saveLayups(m_selectedCourseSlug, hole, {})) {
        showMessage(tr("Could not clear the hole analysis."));
        return false;
    }
    emit courseAnalysisChanged();
    showMessage(tr("Analysis cleared for hole %1.").arg(hole));
    return true;
}

void AppController::setLanguage(const QString &language) {
    if ((language != QStringLiteral("en") && language != QStringLiteral("nb")) ||
        m_language == language) {
        return;
    }
    m_language = language;
    saveSetting(QStringLiteral("language"), language);
    emit languageChangeRequested(language);
    emit settingsChanged();
}

void AppController::setDarkMode(const bool darkMode) {
    if (m_darkMode == darkMode)
        return;
    m_darkMode = darkMode;
    saveSetting(QStringLiteral("palette"),
                darkMode ? QStringLiteral("dark") : QStringLiteral("light"));
    emit settingsChanged();
}

void AppController::setTextScale(const double scale) {
    const double bounded = std::clamp(scale, 0.8, 1.5);
    if (qFuzzyCompare(m_textScale, bounded))
        return;
    m_textScale = bounded;
    saveSetting(QStringLiteral("textScale"), QString::number(bounded));
    emit settingsChanged();
}

void AppController::setShowAdvancedScores(const bool visible) {
    if (m_showAdvancedScores == visible)
        return;
    m_showAdvancedScores = visible;
    saveSetting(QStringLiteral("showAdvancedScores"),
                visible ? QStringLiteral("true") : QStringLiteral("false"));
    emit settingsChanged();
}

void AppController::setAutomaticHoleAdvance(const bool enabled) {
    if (m_automaticHoleAdvance == enabled)
        return;
    m_automaticHoleAdvance = enabled;
    saveSetting(QStringLiteral("automaticHoleAdvance"),
                enabled ? QStringLiteral("true") : QStringLiteral("false"));
    emit settingsChanged();
}

void AppController::setCourseHandicap(const int handicap) {
    const int bounded = std::clamp(handicap, -10, 54);
    if (m_courseHandicap == bounded)
        return;
    m_courseHandicap = bounded;
    saveSetting(QStringLiteral("courseHandicap"), QString::number(bounded));
    emit settingsChanged();
}

void AppController::setPlannerHole(const int hole) {
    const int maximum = std::max(1, m_coursePlan.value(QStringLiteral("holeCount"))
                                        .toInt());
    const int bounded = std::clamp(hole, 1, maximum);
    if (m_plannerHole == bounded)
        return;
    m_plannerHole = bounded;
    emit plannerHoleChanged();
    emit courseAnalysisChanged();
}

void AppController::setRecommendationBias(const double displayUnits) {
    const double metres = std::clamp(toMetres(displayUnits), -30.0, 30.0);
    if (qFuzzyCompare(m_recommendationBiasMetres, metres))
        return;
    m_recommendationBiasMetres = metres;
    saveSetting(QStringLiteral("recommendationBiasMetres"), QString::number(metres));
    updateLiveData();
    emit settingsChanged();
}

void AppController::setCacheLimitMb(const int megabytes) {
    const int bounded = std::clamp(megabytes, 128, 8192);
    if (m_cacheLimitMb == bounded)
        return;
    m_cacheLimitMb = bounded;
    saveSetting(QStringLiteral("cacheLimitMb"), QString::number(bounded));
    emit settingsChanged();
}

void AppController::setCelebrationsEnabled(const bool enabled) {
    if (m_celebrationsEnabled == enabled)
        return;
    m_celebrationsEnabled = enabled;
    saveSetting(QStringLiteral("celebrationsEnabled"),
                enabled ? QStringLiteral("true") : QStringLiteral("false"));
    emit settingsChanged();
}

void AppController::setBrightness(const int brightness) {
    const int bounded = std::clamp(brightness, 10, 100);
    if (!m_powerProvider || m_powerProvider->brightness() == bounded)
        return;
    m_powerProvider->setBrightness(bounded);
    saveSetting(QStringLiteral("brightness"), QString::number(bounded));
    emit settingsChanged();
}

void AppController::setOpenGolfMapServer(const QString &server) {
    const QUrl url(server.trimmed());
    const bool localHttp = url.scheme() == QStringLiteral("http") &&
                           (url.host() == QStringLiteral("localhost") ||
                            url.host() == QStringLiteral("127.0.0.1"));
    if (!url.isValid() || (url.scheme() != QStringLiteral("https") && !localHttp)) {
        showMessage(tr("Use HTTPS (HTTP is allowed for localhost only)."));
        return;
    }
    m_openGolfMapServer = url.toString(QUrl::RemovePath | QUrl::StripTrailingSlash);
    saveSetting(QStringLiteral("openGolfMapServer"), m_openGolfMapServer);
    if (auto *remote = qobject_cast<courses::OpenGolfMapProvider *>(m_provider)) {
        remote->setBaseUrl(QUrl(m_openGolfMapServer));
        remote->checkReachability();
    }
    emit settingsChanged();
}

void AppController::setMapColor(const QString &key, const QString &color) {
    static const QSet<QString> allowed{
        QStringLiteral("rough"), QStringLiteral("fairway"), QStringLiteral("green"),
        QStringLiteral("tee"),   QStringLiteral("bunker"),  QStringLiteral("water"),
        QStringLiteral("wood"),  QStringLiteral("path"),
    };
    const QColor parsed(color);
    if (!allowed.contains(key) || !parsed.isValid())
        return;
    m_mapColors.insert(key, parsed.name(QColor::HexRgb));
    saveSetting(QStringLiteral("map.%1").arg(key), parsed.name(QColor::HexRgb));
    emit settingsChanged();
}

void AppController::resetSettings() {
    if (!m_settings.reset()) {
        showMessage(tr("Could not reset settings."));
        return;
    }
    loadSettings();
    reloadClubs();
    m_powerProvider->setBrightness(80);
    emit settingsChanged();
    showMessage(tr("Settings reset."));
}

QString AppController::distanceText(const double metres) const {
    if (!std::isfinite(metres) || metres <= 0.0)
        return QStringLiteral("—");
    return m_metric ? tr("%1 m").arg(qRound(metres))
                    : tr("%1 yd").arg(qRound(metres * 1.0936133));
}

void AppController::loadDemoCourseData() {
    m_mapSource = resourceUrl(QStringLiteral("render-model.json"));
    loadCourseData(QString::fromLatin1(DemoResourceRoot));
    // The bundled simulator fixture repeats a validated sample hole so all
    // 9/18-hole flows can be exercised without an internet connection.
    const std::array<int, 18> pars{4, 4, 3, 5, 4, 4, 3, 5, 4,
                                   4, 3, 5, 4, 4, 3, 5, 4, 4};
    const std::array<int, 18> indexes{7, 1, 15, 3, 11, 5, 17, 9,  13,
                                      8, 2, 16, 4, 12, 6, 18, 10, 14};
    const HoleNavigation source = m_navigation.value(1);
    for (int hole = 1; hole <= 18; ++hole) {
        HoleNavigation copy = source;
        copy.number = hole;
        copy.par = pars[static_cast<std::size_t>(hole - 1)];
        copy.strokeIndex = indexes[static_cast<std::size_t>(hole - 1)];
        m_navigation.insert(hole, copy);
    }
}

void AppController::loadCourseData(const QString &packagePath) {
    const QString navigationPath =
        packagePath.startsWith(':')
            ? packagePath + QStringLiteral("/navigation.json")
            : QDir(packagePath).filePath(QStringLiteral("navigation.json"));
    const QString renderPath =
        packagePath.startsWith(':')
            ? packagePath + QStringLiteral("/render-model.json")
            : QDir(packagePath).filePath(QStringLiteral("render-model.json"));
    QJsonParseError renderParseError;
    const QJsonDocument renderDocument =
        QJsonDocument::fromJson(readAsset(renderPath), &renderParseError);
    const QJsonObject renderRoot = renderDocument.object();
    const QJsonObject renderModel = renderRoot.contains(QStringLiteral("model"))
                                        ? renderRoot.value(QStringLiteral("model")).toObject()
                                        : renderRoot;
    if (renderParseError.error != QJsonParseError::NoError ||
        !renderDocument.isObject() ||
        renderModel.value(QStringLiteral("holes")).toArray().isEmpty()) {
        m_navigation.clear();
        m_mapSource = QUrl{};
        emit roundChanged();
        return;
    }
    if (!packagePath.startsWith(':')) {
        m_mapSource = QUrl::fromLocalFile(renderPath);
    }
    const QJsonObject document =
        QJsonDocument::fromJson(readAsset(navigationPath)).object();
    m_navigation.clear();
    for (const auto &value : document.value(QStringLiteral("holes")).toArray()) {
        const QJsonObject object = value.toObject();
        HoleNavigation navigation;
        navigation.number = object.value(QStringLiteral("number")).toInt();
        navigation.par = object.value(QStringLiteral("par")).toInt();
        navigation.strokeIndex = object.value(QStringLiteral("handicap")).toInt();
        const QJsonArray tees = object.value(QStringLiteral("tees")).toArray();
        if (!tees.isEmpty()) {
            const QJsonObject tee = tees.first().toObject();
            navigation.teeLabel = tee.value(QStringLiteral("label")).toString();
            const QJsonArray localCentre =
                tee.value(QStringLiteral("localCentre")).toArray();
            if (localCentre.size() == 2) {
                navigation.teeX = localCentre[0].toDouble();
                navigation.teeY = localCentre[1].toDouble();
            }
        }
        const QJsonObject green = object.value(QStringLiteral("green")).toObject();
        const QJsonArray centre = green.value(QStringLiteral("centre")).toArray();
        if (centre.size() == 2) {
            navigation.centre = {centre[1].toDouble(), centre[0].toDouble()};
        }
        const QJsonArray rings = green.value(QStringLiteral("polygon")).toArray();
        if (!rings.isEmpty()) {
            for (const auto &pointValue : rings.first().toArray()) {
                const auto point = pointValue.toArray();
                if (point.size() == 2) {
                    navigation.polygon.push_back(
                        {point[1].toDouble(), point[0].toDouble()});
                }
            }
        }
        const QJsonObject projection =
            object.value(QStringLiteral("projection")).toObject();
        const QJsonObject origin =
            projection.value(QStringLiteral("origin")).toObject();
        navigation.projection = {
            {origin.value(QStringLiteral("lat")).toDouble(),
             origin.value(QStringLiteral("lng")).toDouble()},
            projection.value(QStringLiteral("earthRadiusMeters")).toDouble(6'378'137.0),
            projection.value(QStringLiteral("rotationRadians")).toDouble(),
        };
        if (navigation.number > 0) {
            m_navigation.insert(navigation.number, navigation);
        }
    }
    emit roundChanged();
}

void AppController::rebuildCoursePlan(const QVariantMap &course) {
    const QString renderPath =
        m_mapSource.isLocalFile()
            ? m_mapSource.toLocalFile()
            : m_mapSource.toString().startsWith(QStringLiteral("qrc:/"))
                  ? QStringLiteral(":") + m_mapSource.path()
                  : m_mapSource.toString();
    const QJsonObject root =
        QJsonDocument::fromJson(readAsset(renderPath)).object();
    const QJsonObject model = root.contains(QStringLiteral("model"))
                                  ? root.value(QStringLiteral("model")).toObject()
                                  : root;
    const QJsonArray renderedHoles = model.value(QStringLiteral("holes")).toArray();
    QMap<int, QJsonObject> renderByHole;
    for (const auto &value : renderedHoles) {
        const QJsonObject hole = value.toObject();
        renderByHole.insert(hole.value(QStringLiteral("number")).toInt(), hole);
    }
    const QJsonObject fallbackRender =
        renderedHoles.isEmpty() ? QJsonObject{} : renderedHoles.first().toObject();

    QMap<int, QVariantMap> performanceByHole;
    const QVariantList performance = m_statisticsRepository.holePerformance(
        course.value(QStringLiteral("slug")).toString(),
        m_clubs.defaultProfileId());
    for (const auto &value : performance) {
        const QVariantMap row = value.toMap();
        performanceByHole.insert(row.value(QStringLiteral("hole")).toInt(), row);
    }

    QVariantList holes;
    int totalPar = 0;
    double totalDistanceMetres = 0.0;
    for (auto iterator = m_navigation.cbegin(); iterator != m_navigation.cend();
         ++iterator) {
        const HoleNavigation &navigation = iterator.value();
        const QJsonObject rendered =
            renderByHole.value(navigation.number, fallbackRender);
        double lengthMetres =
            rendered.value(QStringLiteral("lengthMeters")).toDouble();
        const QJsonArray renderedTees = rendered.value(QStringLiteral("tees")).toArray();
        QString teeLabel = navigation.teeLabel;
        if (!renderedTees.isEmpty()) {
            const QJsonObject tee = renderedTees.first().toObject();
            if (lengthMetres <= 0.0)
                lengthMetres = tee.value(QStringLiteral("meters")).toDouble();
            if (teeLabel.isEmpty())
                teeLabel = tee.value(QStringLiteral("label")).toString();
        }
        const QVariantMap performanceRow =
            performanceByHole.value(navigation.number);
        const int played = performanceRow.value(QStringLiteral("played")).toInt();
        holes.push_back(QVariantMap{
            {QStringLiteral("number"), navigation.number},
            {QStringLiteral("par"), navigation.par},
            {QStringLiteral("index"), navigation.strokeIndex},
            {QStringLiteral("lengthMetres"), lengthMetres},
            {QStringLiteral("teeLabel"),
             teeLabel.isEmpty() ? tr("Default") : teeLabel},
            {QStringLiteral("teeX"), navigation.teeX},
            {QStringLiteral("teeY"), navigation.teeY},
            {QStringLiteral("played"), played},
            {QStringLiteral("average"),
             played > 0 ? performanceRow.value(QStringLiteral("average"))
                        : QVariant{}},
            {QStringLiteral("best"),
             played > 0 ? performanceRow.value(QStringLiteral("best"))
                        : QVariant{}},
        });
        if (navigation.par >= 3 && navigation.par <= 6)
            totalPar += navigation.par;
        totalDistanceMetres += std::max(0.0, lengthMetres);
    }

    const QVariantMap courseStatistics = m_statisticsRepository.overview(
        course.value(QStringLiteral("slug")).toString(),
        m_clubs.defaultProfileId());
    m_coursePlan = {
        {QStringLiteral("name"), course.value(QStringLiteral("name"))},
        {QStringLiteral("slug"), course.value(QStringLiteral("slug"))},
        {QStringLiteral("holeCount"), holes.size()},
        {QStringLiteral("par"), totalPar},
        {QStringLiteral("distanceMetres"), totalDistanceMetres},
        {QStringLiteral("rounds"), courseStatistics.value(QStringLiteral("rounds"))},
        {QStringLiteral("holes"), holes},
        {QStringLiteral("attribution"),
         course.value(QStringLiteral("attribution"))},
    };
    emit coursePlanChanged();
}

QByteArray AppController::readAsset(const QString &path) const {
    QFile file(path);
    return file.open(QIODevice::ReadOnly) ? file.readAll() : QByteArray{};
}

void AppController::handlePosition(const domain::PositionFix &fix) {
    m_lastFix = fix;
    updateLiveData();
}

void AppController::updateLiveData() {
    if (!m_activeRound)
        return;
    const bool usable = domain::isUsableFix(m_lastFix, std::chrono::system_clock::now(),
                                            25.0, std::chrono::seconds(10));
    m_gpsAccuracy = m_lastFix.accuracyMetres;
    if (!m_lastFix.valid) {
        m_gpsStatus = tr("Waiting for GPS");
    } else if (!usable) {
        m_gpsStatus =
            m_lastFix.accuracyMetres > 25.0 ? tr("Low accuracy") : tr("Stale GPS fix");
    } else {
        m_gpsStatus = tr("GPS ready");
    }

    if (m_lastFix.valid && m_automaticHoleAdvance && m_navigation.size() > 1) {
        std::vector<domain::HoleProximity> candidates;
        candidates.reserve(static_cast<std::size_t>(m_navigation.size()));
        for (auto iterator = m_navigation.cbegin(); iterator != m_navigation.cend();
             ++iterator) {
            candidates.push_back(
                {iterator.key(),
                 domain::haversineMetres(m_lastFix.point, iterator.value().centre)});
        }
        const int selected = m_holeSelector.update(candidates);
        if (selected != currentHole() && selected <= holeCount()) {
            setHole(selected);
            return;
        }
    }

    const auto navigation = currentNavigation();
    if (m_lastFix.valid) {
        const auto distances = domain::distancesToGreen(
            m_lastFix.point, navigation.polygon, navigation.centre);
        m_frontDistance = distances.frontMetres;
        m_centreDistance = distances.centreMetres;
        m_backDistance = distances.backMetres;
        const auto local =
            domain::projectToLocal(m_lastFix.point, navigation.projection);
        m_playerX = local.first;
        m_playerY = local.second;
        m_playerVisible = true;
    } else {
        m_frontDistance = 0.0;
        m_centreDistance = 0.0;
        m_backDistance = 0.0;
        m_playerVisible = false;
    }

    if (m_nearGreenTrigger.update(
            currentHole(), m_centreDistance,
            usable && m_screen == QStringLiteral("LiveHoleScreen"),
            m_currentScore.strokes > 0)) {
        emit scoreEntryRequested(currentHole());
    }

    m_playsLike = {};
    m_playsLikeAvailable = false;
    m_windRelativeDegrees = 0;
    if (usable && m_centreDistance > 0.0) {
        const domain::WeatherConditions weather = activeWeather();
        const bool hasWeather = weather.temperatureC.has_value() ||
                                (weather.windSpeedMps && weather.windFromDegrees) ||
                                !weather.condition.empty();
        if (hasWeather) {
            const double bearing =
                domain::initialBearingDegrees(m_lastFix.point, navigation.centre);
            m_playsLike = domain::computePlaysLike(m_centreDistance, bearing, weather);
            m_playsLikeAvailable = true;
            if (weather.windFromDegrees) {
                const int relative =
                    qRound(static_cast<double>(*weather.windFromDegrees) - bearing);
                m_windRelativeDegrees = ((relative % 360) + 360) % 360;
            }
        }
    }

    m_clubAdvice.clear();
    m_clubDelta.clear();
    if (usable) {
        const double effectiveDistance =
            m_playsLikeAvailable ? m_playsLike.totalMetres() : m_centreDistance;
        const auto advice =
            domain::recommendClub(m_clubs.list(m_clubs.defaultProfileId()),
                                  effectiveDistance, m_recommendationBiasMetres);
        if (advice) {
            m_clubAdvice = QString::fromStdString(advice->club.name);
            const double delta = fromMetres(advice->deltaMetres);
            m_clubDelta = tr("%1%2 %3")
                              .arg(delta >= 0.0 ? QStringLiteral("+") : QString())
                              .arg(qRound(delta))
                              .arg(m_metric ? tr("m") : tr("yd"));
        }
    }
    emit liveChanged();
}

void AppController::loadCurrentScore() {
    m_currentScore = {.hole = currentHole()};
    if (m_activeRound) {
        for (const auto &score : m_rounds.scores(*m_activeRound)) {
            if (score.hole == currentHole()) {
                m_currentScore = score;
                break;
            }
        }
    }
    emit scoreChanged();
}

bool AppController::saveCurrentScore() {
    if (!m_activeRound)
        return false;
    m_currentScore.hole = currentHole();
    if (!m_rounds.saveScore(*m_activeRound, currentDefinition(), m_currentScore)) {
        showMessage(tr("Score could not be saved."));
        return false;
    }
    rebuildScorecard();
    emit scoreChanged();
    return true;
}

void AppController::celebrateCurrentHole() {
    if (!m_celebrationsEnabled || !m_activeRound || m_currentScore.strokes <= 0 ||
        m_celebratedHoles.contains(currentHole())) {
        return;
    }
    const auto outcome =
        domain::classifyHole(currentDefinition().par, m_currentScore.strokes);
    switch (outcome) {
    case domain::HoleOutcome::AlbatrossOrBetter:
    case domain::HoleOutcome::Eagle:
        m_celebrationKind = QStringLiteral("eagle");
        break;
    case domain::HoleOutcome::Birdie:
        m_celebrationKind = QStringLiteral("birdie");
        break;
    case domain::HoleOutcome::Par:
        m_celebrationKind = QStringLiteral("par");
        break;
    default:
        return;
    }
    m_celebratedHoles.insert(currentHole());
    ++m_celebrationSequence;
    emit celebrationChanged();
}

void AppController::rebuildScorecard() {
    m_scorecard.clear();
    if (!m_activeRound) {
        emit scorecardChanged();
        return;
    }
    std::unordered_map<int, domain::HoleScore> byHole;
    for (const auto &score : m_rounds.scores(*m_activeRound)) {
        byHole.emplace(score.hole, score);
    }
    std::vector<domain::HoleDefinition> courseDefinitions;
    courseDefinitions.reserve(static_cast<std::size_t>(m_navigation.size()));
    for (auto iterator = m_navigation.cbegin(); iterator != m_navigation.cend();
         ++iterator) {
        courseDefinitions.push_back(
            {iterator.key(), iterator->par, iterator->strokeIndex});
    }
    const int indexScale = domain::handicapIndexScale(courseDefinitions);
    for (int hole = 1; hole <= m_activeRound->holeCount; ++hole) {
        const auto navigation = m_navigation.value(hole);
        const auto found = byHole.find(hole);
        const int score = found == byHole.end() ? 0 : found->second.strokes;
        const int stableford =
            score > 0 ? domain::stablefordPoints(
                            {hole, navigation.par, navigation.strokeIndex}, score,
                            m_activeRound->courseHandicap, indexScale)
                      : 0;
        m_scorecard.push_back(QVariantMap{
            {QStringLiteral("hole"), hole},
            {QStringLiteral("par"), navigation.par},
            {QStringLiteral("index"), navigation.strokeIndex},
            {QStringLiteral("strokes"), score},
            {QStringLiteral("stableford"), stableford},
            {QStringLiteral("putts"),
             found == byHole.end() ? 0 : found->second.putts.value_or(0)},
            {QStringLiteral("penalties"),
             found == byHole.end() ? 0 : found->second.penalties},
        });
    }
    emit scorecardChanged();
}

AppController::HoleNavigation AppController::currentNavigation() const {
    if (m_navigation.contains(currentHole())) {
        return m_navigation.value(currentHole());
    }
    HoleNavigation fallback;
    fallback.number = currentHole();
    fallback.strokeIndex = currentHole();
    return fallback;
}

domain::HoleDefinition AppController::currentDefinition() const {
    const auto navigation = currentNavigation();
    return {currentHole(), navigation.par, navigation.strokeIndex};
}

double AppController::toMetres(const double displayUnits) const {
    return m_metric ? displayUnits : displayUnits * 0.9144;
}

double AppController::fromMetres(const double metres) const {
    return m_metric ? metres : metres * 1.0936133;
}

void AppController::showMessage(const QString &message) {
    m_message = message;
    emit messageChanged();
    m_messageTimer.start();
}

void AppController::saveSetting(const QString &key, const QString &value) {
    if (!m_settings.setValue(key, value)) {
        showMessage(tr("Setting could not be saved."));
    }
}

} // namespace opencaddie::ui
