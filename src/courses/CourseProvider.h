#pragma once

#include <QObject>
#include <QVariantList>
#include <QVariantMap>

namespace opencaddie::courses {

class CourseProvider : public QObject {
    Q_OBJECT

public:
    using QObject::QObject;
    ~CourseProvider() override = default;

    virtual void search(const QString& query) = 0;
    virtual void fetchBundle(const QVariantMap& candidate,
                             const QString& distanceUnit) = 0;
    virtual void cancel() = 0;

signals:
    void searchCompleted(const QVariantList& results);
    void downloadProgress(qint64 received, qint64 total);
    void bundleReady(const QByteArray& bytes);
    void errorOccurred(const QString& message);
};

} // namespace opencaddie::courses

