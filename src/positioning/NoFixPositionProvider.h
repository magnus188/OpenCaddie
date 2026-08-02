#pragma once

#include "positioning/PositionProvider.h"

namespace opencaddie::positioning {

class NoFixPositionProvider final : public PositionProvider {
    Q_OBJECT

public:
    using PositionProvider::PositionProvider;

    void start() override;
    void stop() override;
    [[nodiscard]] QString name() const override;
};

} // namespace opencaddie::positioning
