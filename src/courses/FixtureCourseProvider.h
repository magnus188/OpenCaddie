#pragma once

#include "courses/CourseProvider.h"

namespace opencaddie::courses {

class FixtureCourseProvider final : public CourseProvider {
    Q_OBJECT

public:
    explicit FixtureCourseProvider(QString fixtureArchive,
                                   QObject* parent = nullptr);
    void search(const QString& query) override;
    void fetchBundle(const QVariantMap& candidate,
                     const QString& distanceUnit) override;
    void cancel() override;

private:
    QString m_fixtureArchive;
};

} // namespace opencaddie::courses

