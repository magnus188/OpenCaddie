#pragma once

#include "domain/Types.h"

#include <QObject>
#include <QVariantMap>

Q_DECLARE_METATYPE(opencaddie::domain::PositionFix)

namespace opencaddie::positioning {

class PositionProvider : public QObject {
    Q_OBJECT

public:
    using QObject::QObject;
    ~PositionProvider() override = default;

    virtual void start() = 0;
    virtual void stop() = 0;
    [[nodiscard]] virtual QString name() const = 0;

signals:
    void positionChanged(const opencaddie::domain::PositionFix& fix);
    void diagnosticsChanged(const QVariantMap& diagnostics);
};

} // namespace opencaddie::positioning
