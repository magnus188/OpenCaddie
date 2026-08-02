#include "platform/PiPowerProvider.h"

#include <QDir>
#include <QFile>
#include <QTextStream>

#include <algorithm>

namespace opencaddie::platform {

namespace {
QString readValue(const QString &path) {
    QFile file(path);
    return file.open(QIODevice::ReadOnly | QIODevice::Text)
               ? QString::fromUtf8(file.readAll()).trimmed()
               : QString{};
}
} // namespace

PiPowerProvider::PiPowerProvider(QString brightnessPath,
                                 QString powerSupplyRoot, QObject* parent)
    : PowerProvider(parent), m_brightnessPath(std::move(brightnessPath)),
      m_powerSupplyRoot(std::move(powerSupplyRoot)) {
    m_powerTimer.setInterval(30'000);
    connect(&m_powerTimer, &QTimer::timeout, this,
            &PiPowerProvider::refreshPower);
    refreshPower();
    m_powerTimer.start();
}

int PiPowerProvider::batteryPercent() const { return m_batteryPercent; }

bool PiPowerProvider::externalPower() const { return m_externalPower; }
int PiPowerProvider::brightness() const { return m_brightness; }

void PiPowerProvider::refreshPower() {
    int batteryPercent = -1;
    bool externalPower = true;
    bool powerTelemetryFound = false;
    bool externalStateFound = false;
    const QDir root(m_powerSupplyRoot);
    for (const QString &entry : root.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
        const QString base = root.filePath(entry);
        const QString type = readValue(QDir(base).filePath(QStringLiteral("type")));
        if (type.compare(QStringLiteral("Battery"), Qt::CaseInsensitive) == 0) {
            bool valid = false;
            const int capacity = readValue(
                QDir(base).filePath(QStringLiteral("capacity"))).toInt(&valid);
            if (valid) batteryPercent = std::clamp(capacity, 0, 100);
            const QString status = readValue(
                QDir(base).filePath(QStringLiteral("status")));
            if (!status.isEmpty()) {
                externalPower =
                    status.compare(QStringLiteral("Charging"), Qt::CaseInsensitive) == 0 ||
                    status.compare(QStringLiteral("Full"), Qt::CaseInsensitive) == 0;
                externalStateFound = true;
            }
            powerTelemetryFound = true;
        } else if (type.compare(QStringLiteral("Mains"), Qt::CaseInsensitive) == 0 ||
                   type.compare(QStringLiteral("USB"), Qt::CaseInsensitive) == 0 ||
                   type.compare(QStringLiteral("USB_C"), Qt::CaseInsensitive) == 0) {
            bool valid = false;
            const int online = readValue(
                QDir(base).filePath(QStringLiteral("online"))).toInt(&valid);
            if (valid) {
                externalPower = online != 0;
                externalStateFound = true;
            }
            powerTelemetryFound = true;
        }
    }
    if (!powerTelemetryFound || !externalStateFound) externalPower = true;
    if (batteryPercent == m_batteryPercent && externalPower == m_externalPower) return;
    m_batteryPercent = batteryPercent;
    m_externalPower = externalPower;
    emit powerChanged();
}

void PiPowerProvider::setBrightness(const int percent) {
    const int bounded = std::clamp(percent, 10, 100);
    QFile file(m_brightnessPath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream(&file) << qRound(255.0 * bounded / 100.0);
    }
    if (m_brightness != bounded) {
        m_brightness = bounded;
        emit brightnessChanged();
    }
}

} // namespace opencaddie::platform
