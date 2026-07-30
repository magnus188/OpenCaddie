#pragma once

#include "positioning/PositionProvider.h"

namespace opencaddie::positioning {

class ManualPositionProvider final : public PositionProvider {
    Q_OBJECT

public:
    using PositionProvider::PositionProvider;
    void start() override;
    void stop() override;
    [[nodiscard]] QString name() const override;

    Q_INVOKABLE void setPosition(double latitude, double longitude,
                                 double accuracyMetres = 5.0);
};

} // namespace opencaddie::positioning

