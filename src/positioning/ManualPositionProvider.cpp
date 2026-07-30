#include "positioning/ManualPositionProvider.h"

namespace opencaddie::positioning {

void ManualPositionProvider::start() {}
void ManualPositionProvider::stop() {}
QString ManualPositionProvider::name() const {
    return QStringLiteral("manual-simulator");
}

void ManualPositionProvider::setPosition(const double latitude,
                                         const double longitude,
                                         const double accuracyMetres) {
    emit positionChanged({
        {latitude, longitude},
        accuracyMetres,
        std::chrono::system_clock::now(),
        true,
    });
}

} // namespace opencaddie::positioning

