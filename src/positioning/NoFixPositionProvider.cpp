#include "positioning/NoFixPositionProvider.h"

#include <chrono>

namespace opencaddie::positioning {

void NoFixPositionProvider::start() {
    emit positionChanged({{}, 0.0, std::chrono::system_clock::now(), false});
    emit diagnosticsChanged({
        {QStringLiteral("provider"), name()},
        {QStringLiteral("state"), QStringLiteral("no-fix")},
    });
}

void NoFixPositionProvider::stop() {}

QString NoFixPositionProvider::name() const {
    return QStringLiteral("Simulator (no GPS fix)");
}

} // namespace opencaddie::positioning
