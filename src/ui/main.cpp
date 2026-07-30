#include "connectivity/MockNetworkManager.h"
#include "connectivity/NetworkManagerDbus.h"
#include "courses/OpenGolfMapProvider.h"
#include "platform/MockPowerProvider.h"
#include "platform/PiPowerProvider.h"
#include "positioning/GpsdPositionProvider.h"
#include "positioning/RouteReplayPositionProvider.h"
#include "storage/Database.h"
#include "ui/AppController.h"
#include "ui/CourseMapItem.h"
#include "ui/TranslationManager.h"

#include <QCommandLineParser>
#include <QDir>
#include <QFont>
#include <QFontDatabase>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QQuickWindow>
#include <QStandardPaths>
#include <QTimer>

#include <memory>

int main(int argc, char* argv[]) {
    QCoreApplication::setOrganizationName(QStringLiteral("OpenCaddie"));
    QCoreApplication::setOrganizationDomain(QStringLiteral("opencaddie.org"));
    QCoreApplication::setApplicationName(QStringLiteral("OpenCaddie"));
    QCoreApplication::setApplicationVersion(QStringLiteral("0.1.0"));
    QQuickStyle::setStyle(QStringLiteral("Basic"));

    QGuiApplication application(argc, argv);
    QCommandLineParser parser;
    parser.setApplicationDescription(
        QStringLiteral("OpenCaddie local-first golf computer"));
    parser.addHelpOption();
    parser.addVersionOption();
    const QCommandLineOption simulate(
        QStringList{QStringLiteral("s"), QStringLiteral("simulate")},
        QStringLiteral("Use recorded GPS and mock hardware."));
    const QCommandLineOption device(
        QStringList{QStringLiteral("device")},
        QStringLiteral("Force Raspberry Pi hardware providers."));
    const QCommandLineOption windowed(
        QStringList{QStringLiteral("w"), QStringLiteral("windowed")},
        QStringLiteral("Keep the simulator in an 800x480 window."));
    const QCommandLineOption dataDirectory(
        QStringList{QStringLiteral("data-dir")},
        QStringLiteral("Override the local data directory."),
        QStringLiteral("path"));
    const QCommandLineOption route(
        QStringList{QStringLiteral("route")},
        QStringLiteral("CSV route used by the GPS simulator."),
        QStringLiteral("path"));
    const QCommandLineOption demoRound(
        QStringList{QStringLiteral("demo-round")},
        QStringLiteral("Start the embedded demo round (simulator only)."));
    const QCommandLineOption screenshot(
        QStringList{QStringLiteral("screenshot")},
        QStringLiteral("Save a simulator screenshot and exit."),
        QStringLiteral("path"));
    const QCommandLineOption language(
        QStringList{QStringLiteral("language")},
        QStringLiteral("Override simulator language (en or nb)."),
        QStringLiteral("language"));
    parser.addOptions(
        {simulate, device, windowed, dataDirectory, route, demoRound, screenshot,
         language});
    parser.process(application);

#if defined(Q_OS_LINUX)
    const bool simulator = parser.isSet(simulate) && !parser.isSet(device);
#else
    const bool simulator = !parser.isSet(device);
#endif
    const QString defaultDataRoot =
        simulator
            ? QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
            : QStringLiteral("/var/lib/opencaddie");
    const QString dataRoot =
        parser.isSet(dataDirectory) ? parser.value(dataDirectory)
                                    : defaultDataRoot;
    if (!QDir().mkpath(dataRoot)) {
        qCritical("Could not create OpenCaddie data directory");
        return EXIT_FAILURE;
    }

    opencaddie::storage::Database database;
    if (!database.open(QDir(dataRoot).filePath(QStringLiteral("user.sqlite")))) {
        qCritical("Database initialization failed: %s",
                  qUtf8Printable(database.lastError()));
        return EXIT_FAILURE;
    }

    std::unique_ptr<opencaddie::positioning::PositionProvider> positionProvider;
    std::unique_ptr<opencaddie::connectivity::NetworkManager> networkManager;
    std::unique_ptr<opencaddie::platform::PowerProvider> powerProvider;
    if (simulator) {
        const QString routePath =
            parser.isSet(route)
                ? parser.value(route)
                : QStringLiteral(
                      ":/qt/qml/OpenCaddie/assets/routes/demo-route.csv");
        positionProvider =
            std::make_unique<
                opencaddie::positioning::RouteReplayPositionProvider>(
                routePath);
        networkManager =
            std::make_unique<opencaddie::connectivity::MockNetworkManager>();
        powerProvider =
            std::make_unique<opencaddie::platform::MockPowerProvider>();
    } else {
        positionProvider =
            std::make_unique<opencaddie::positioning::GpsdPositionProvider>();
        networkManager =
            std::make_unique<opencaddie::connectivity::NetworkManagerDbus>();
        powerProvider =
            std::make_unique<opencaddie::platform::PiPowerProvider>();
    }
    opencaddie::courses::OpenGolfMapProvider courseProvider;
    opencaddie::ui::AppController controller(
        &database, &courseProvider, positionProvider.get(), powerProvider.get(),
        QDir(dataRoot).filePath(QStringLiteral("courses")));
    if (!controller.initialize()) {
        qCritical("OpenCaddie services could not be initialized");
        return EXIT_FAILURE;
    }
    if (simulator && parser.isSet(demoRound) && !controller.hasActiveRound()) {
        controller.prepareRound(QStringLiteral("opencaddie-demo"));
        controller.startRound(QStringLiteral("opencaddie-demo"), 18, false, 0,
                              QStringLiteral("Yellow"));
    }
    if (simulator && parser.isSet(language)) {
        controller.setLanguage(parser.value(language));
    }

    const QStringList fontPaths{
        QStringLiteral(
            ":/qt/qml/OpenCaddie/assets/fonts/Inter-Regular.ttf"),
        QStringLiteral(
            ":/qt/qml/OpenCaddie/assets/fonts/Inter-Medium.ttf"),
        QStringLiteral(
            ":/qt/qml/OpenCaddie/assets/fonts/Inter-SemiBold.ttf"),
        QStringLiteral(":/qt/qml/OpenCaddie/assets/fonts/Inter-Bold.ttf"),
    };
    for (const auto& fontPath : fontPaths) {
        QFontDatabase::addApplicationFont(fontPath);
    }
    application.setFont(QFont(QStringLiteral("Inter")));

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty(QStringLiteral("app"),
                                             &controller);
    engine.rootContext()->setContextProperty(QStringLiteral("network"),
                                             networkManager.get());
    engine.rootContext()->setContextProperty(QStringLiteral("power"),
                                             powerProvider.get());
    engine.rootContext()->setContextProperty(QStringLiteral("simulator"),
                                             simulator);
    opencaddie::ui::TranslationManager translation(&engine);
    QObject::connect(&controller,
                     &opencaddie::ui::AppController::languageChangeRequested,
                     &translation,
                     &opencaddie::ui::TranslationManager::setLanguage);
    translation.setLanguage(controller.language());

    engine.loadFromModule("OpenCaddie", "Main");
    if (engine.rootObjects().isEmpty()) return EXIT_FAILURE;
    if (!simulator || !parser.isSet(windowed)) {
        if (auto* window = qobject_cast<QQuickWindow*>(engine.rootObjects().first())) {
            window->showFullScreen();
        }
    }
    positionProvider->start();
    networkManager->scan();
    if (simulator && parser.isSet(screenshot)) {
        const QString screenshotPath = parser.value(screenshot);
        QTimer::singleShot(2'000, &application,
                           [&application, &engine, screenshotPath] {
            auto* window = qobject_cast<QQuickWindow*>(
                engine.rootObjects().value(0));
            if (!window || !window->grabWindow().save(screenshotPath)) {
                application.exit(EXIT_FAILURE);
                return;
            }
            application.exit(EXIT_SUCCESS);
        });
    }
    const int result = application.exec();
    positionProvider->stop();
    return result;
}
