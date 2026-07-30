#include "courses/FixtureCourseProvider.h"

#include <QFile>
#include <QTimer>

namespace opencaddie::courses {

FixtureCourseProvider::FixtureCourseProvider(QString fixtureArchive,
                                             QObject* parent)
    : CourseProvider(parent), m_fixtureArchive(std::move(fixtureArchive)) {}

void FixtureCourseProvider::search(const QString& query) {
    const QVariantMap course{
        {QStringLiteral("id"), QStringLiteral("way/12345")},
        {QStringLiteral("name"), QStringLiteral("OpenCaddie Demo Course")},
        {QStringLiteral("displayName"),
         QStringLiteral("OpenCaddie Demo Course, Oslo, Norway")},
        {QStringLiteral("lat"), 59.95},
        {QStringLiteral("lng"), 10.62},
        {QStringLiteral("bbox"),
         QVariantList{59.948, 10.618, 59.952, 10.622}},
        {QStringLiteral("osmType"), QStringLiteral("way")},
        {QStringLiteral("osmId"), 12345},
        {QStringLiteral("country"), QStringLiteral("Norway")},
    };
    QTimer::singleShot(50, this, [this, course, query] {
        emit searchCompleted(
            query.trimmed().isEmpty() ? QVariantList{} : QVariantList{course});
    });
}

void FixtureCourseProvider::fetchBundle(const QVariantMap&,
                                        const QString&) {
    QFile file(m_fixtureArchive);
    if (!file.open(QIODevice::ReadOnly)) {
        emit errorOccurred(tr("Simulator course fixture is missing."));
        return;
    }
    emit downloadProgress(file.size(), file.size());
    emit bundleReady(file.readAll());
}

void FixtureCourseProvider::cancel() {}

} // namespace opencaddie::courses

