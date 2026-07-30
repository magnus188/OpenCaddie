#pragma once

#include "courses/CourseProvider.h"

#include <QNetworkAccessManager>
#include <QPointer>
#include <QUrl>

class QNetworkReply;

namespace opencaddie::courses {

class OpenGolfMapProvider final : public CourseProvider {
    Q_OBJECT
    Q_PROPERTY(QUrl baseUrl READ baseUrl WRITE setBaseUrl NOTIFY baseUrlChanged)
    Q_PROPERTY(bool reachable READ reachable NOTIFY reachableChanged)

public:
    explicit OpenGolfMapProvider(QObject* parent = nullptr);

    [[nodiscard]] QUrl baseUrl() const;
    void setBaseUrl(const QUrl& url);
    [[nodiscard]] bool reachable() const;

    void search(const QString& query) override;
    void fetchBundle(const QVariantMap& candidate,
                     const QString& distanceUnit) override;
    void cancel() override;
    void checkReachability();

signals:
    void baseUrlChanged();
    void reachableChanged();

private:
    QNetworkRequest requestFor(const QString& path) const;
    void setReachable(bool value);
    void handleImportReply(QNetworkReply* reply, QString distanceUnit);
    void requestBundle(const QString& slug, const QString& distanceUnit);

    QNetworkAccessManager m_network;
    QUrl m_baseUrl{QStringLiteral("http://localhost:3000")};
    QPointer<QNetworkReply> m_activeReply;
    bool m_reachable = false;
};

} // namespace opencaddie::courses

