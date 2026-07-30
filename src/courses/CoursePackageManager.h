#pragma once

#include "storage/Repositories.h"

#include <QByteArray>
#include <QString>

#include <optional>

namespace opencaddie::courses {

class CoursePackageManager {
public:
    CoursePackageManager(QString coursesRoot,
                         storage::CourseRepository repository);

    std::optional<storage::CachedCourse> install(const QByteArray& archive,
                                                 QString* error);
    bool remove(const QString& slug, const QString& version, QString* error);
    [[nodiscard]] QString coursesRoot() const;

private:
    static QString safeSegment(const QString& value);
    static bool writeFile(const QString& root, const QString& relativePath,
                          const QByteArray& bytes, QString* error);

    QString m_coursesRoot;
    storage::CourseRepository m_repository;
};

} // namespace opencaddie::courses

