#include "connectivity/MockBluetoothProvider.h"
#include "connectivity/MockNetworkManager.h"
#include "platform/PiPowerProvider.h"
#include "positioning/NoFixPositionProvider.h"
#include "positioning/ProviderSelection.h"

#include <QCoreApplication>
#include <QDir>
#include <QEventLoop>
#include <QFile>
#include <QTemporaryDir>
#include <QTimer>

#include <cstdlib>
#include <iostream>

namespace {
void require(const bool condition, const char *message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        std::exit(EXIT_FAILURE);
    }
}

void writeValue(const QString &path, const QByteArray &value) {
    QFile file(path);
    require(file.open(QIODevice::WriteOnly), "telemetry fixture opens");
    require(file.write(value) == value.size(), "telemetry fixture writes");
}
} // namespace

int main(int argc, char **argv) {
    QCoreApplication application(argc, argv);

    opencaddie::connectivity::MockNetworkManager network;
    QEventLoop scanLoop;
    QObject::connect(&network,
                     &opencaddie::connectivity::NetworkManager::scanningChanged,
                     &scanLoop, [&] {
                         if (!network.scanning()) scanLoop.quit();
                     });
    network.scan();
    QTimer::singleShot(2'000, &scanLoop, &QEventLoop::quit);
    scanLoop.exec();
    network.connectNetwork(QStringLiteral("Guest"), {}, false);
    require(network.connectedSignalStrength() == 42,
            "connected Wi-Fi exposes the matching signal strength");
    network.forgetNetwork(QStringLiteral("Guest"));
    require(network.connectedSignalStrength() == -1,
            "disconnected Wi-Fi clears signal strength");

    opencaddie::connectivity::MockBluetoothProvider bluetooth;
    require(!bluetooth.connected(), "simulator Bluetooth defaults disconnected");
    bluetooth.setConnected(true, QStringLiteral("Rangefinder"));
    require(bluetooth.connected() &&
                bluetooth.deviceName() == QStringLiteral("Rangefinder"),
            "simulator Bluetooth supports a connected device state");

    QTemporaryDir temporary;
    require(temporary.isValid(), "temporary directory");
    const QString supplyRoot = temporary.filePath(QStringLiteral("power_supply"));
    const QString battery = QDir(supplyRoot).filePath(QStringLiteral("BAT0"));
    const QString mains = QDir(supplyRoot).filePath(QStringLiteral("AC"));
    require(QDir().mkpath(battery) && QDir().mkpath(mains),
            "power supply fixture directories");
    writeValue(QDir(battery).filePath(QStringLiteral("type")), "Battery\n");
    writeValue(QDir(battery).filePath(QStringLiteral("capacity")), "68\n");
    writeValue(QDir(battery).filePath(QStringLiteral("status")), "Charging\n");
    writeValue(QDir(mains).filePath(QStringLiteral("type")), "Mains\n");
    writeValue(QDir(mains).filePath(QStringLiteral("online")), "1\n");
    opencaddie::platform::PiPowerProvider power(
        temporary.filePath(QStringLiteral("brightness")), supplyRoot);
    require(power.batteryPercent() == 68,
            "standard Linux battery capacity is exposed");
    require(power.externalPower(), "standard Linux mains state is exposed");
    opencaddie::platform::PiPowerProvider externalOnly(
        temporary.filePath(QStringLiteral("brightness-external")),
        temporary.filePath(QStringLiteral("no-power-telemetry")));
    require(externalOnly.batteryPercent() == -1 && externalOnly.externalPower(),
            "missing fuel gauge uses external-power fallback without a fake percent");

    opencaddie::positioning::NoFixPositionProvider noFix;
    bool receivedNoFix = false;
    QObject::connect(&noFix,
                     &opencaddie::positioning::PositionProvider::positionChanged,
                     [&](const opencaddie::domain::PositionFix &fix) {
                         receivedNoFix = !fix.valid;
                     });
    noFix.start();
    require(receivedNoFix,
            "normal simulator provider reports no fix instead of replaying GPS");
    using opencaddie::positioning::ProviderMode;
    using opencaddie::positioning::selectProviderMode;
    require(selectProviderMode(true, false, false) == ProviderMode::NoFix,
            "default simulator selects no-fix positioning");
    require(selectProviderMode(true, true, false) == ProviderMode::RouteReplay,
            "demo round selects the bundled replay route");
    require(selectProviderMode(true, false, true) == ProviderMode::RouteReplay,
            "explicit route selects route replay");
    require(selectProviderMode(false, false, false) == ProviderMode::Gpsd,
            "device mode selects gpsd");

    std::cout << "Provider tests passed\n";
    return EXIT_SUCCESS;
}
