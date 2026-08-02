#include "courses/OpenGolfMapProvider.h"

#include <QCoreApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrlQuery>

namespace opencaddie::courses {

OpenGolfMapProvider::OpenGolfMapProvider(QObject* parent)
    : CourseProvider(parent) {}

QUrl OpenGolfMapProvider::baseUrl() const { return m_baseUrl; }

void OpenGolfMapProvider::setBaseUrl(const QUrl& url) {
    QUrl normalized = url;
    normalized.setPath({});
    normalized.setQuery(QString{});
    normalized.setFragment({});
    if (!normalized.isValid() || normalized == m_baseUrl) return;
    m_baseUrl = normalized;
    emit baseUrlChanged();
}

bool OpenGolfMapProvider::reachable() const { return m_reachable; }

QNetworkRequest OpenGolfMapProvider::requestFor(const QString& path) const {
    QUrl url = m_baseUrl;
    url.setPath(path);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader,
                      QStringLiteral("OpenCaddie/%1").arg(
                          QCoreApplication::applicationVersion()));
    request.setTransferTimeout(30'000);
    return request;
}

void OpenGolfMapProvider::search(const QString& query) {
    const QString trimmed = query.trimmed();
    if (trimmed.size() < 2) {
        emit searchCompleted({});
        return;
    }
    cancel();
    auto request = requestFor(QStringLiteral("/api/v1/courses/search"));
    QUrl url = request.url();
    QUrlQuery urlQuery;
    urlQuery.addQueryItem(QStringLiteral("q"), trimmed);
    url.setQuery(urlQuery);
    request.setUrl(url);
    m_activeReply = m_network.get(request);
    connect(m_activeReply, &QNetworkReply::finished, this, [this] {
        const QPointer<QNetworkReply> reply = m_activeReply;
        m_activeReply.clear();
        if (!reply) return;
        const QByteArray body = reply->readAll();
        if (reply->error() != QNetworkReply::NoError) {
            setReachable(false);
            emit errorOccurred(tr("Course search failed: %1")
                                   .arg(reply->errorString()));
            reply->deleteLater();
            return;
        }
        const auto document = QJsonDocument::fromJson(body);
        if (!document.isObject()) {
            emit errorOccurred(tr("The course service returned invalid data."));
            reply->deleteLater();
            return;
        }
        QVariantList results;
        for (const auto& value :
             document.object().value(QStringLiteral("results")).toArray()) {
            results.push_back(value.toObject().toVariantMap());
        }
        setReachable(true);
        emit searchCompleted(results);
        reply->deleteLater();
    });
}

void OpenGolfMapProvider::fetchBundle(const QVariantMap& candidate,
                                      const QString& distanceUnit) {
    cancel();
    QJsonObject candidateJson = QJsonObject::fromVariantMap(candidate);
    const QJsonDocument document(
        QJsonObject{{QStringLiteral("candidate"), candidateJson}});
    auto request = requestFor(QStringLiteral("/api/v1/courses"));
    request.setHeader(QNetworkRequest::ContentTypeHeader,
                      QStringLiteral("application/json"));
    m_activeReply = m_network.post(request, document.toJson(QJsonDocument::Compact));
    connect(m_activeReply, &QNetworkReply::finished, this,
            [this, distanceUnit] {
                const QPointer<QNetworkReply> reply = m_activeReply;
                m_activeReply.clear();
                if (reply) handleImportReply(reply, distanceUnit);
            });
}

void OpenGolfMapProvider::handleImportReply(QNetworkReply* reply,
                                            const QString distanceUnit) {
    const QByteArray body = reply->readAll();
    if (reply->error() != QNetworkReply::NoError) {
        emit errorOccurred(tr("Course import failed: %1")
                               .arg(reply->errorString()));
        reply->deleteLater();
        return;
    }
    const auto document = QJsonDocument::fromJson(body);
    const QString slug =
        document.object().value(QStringLiteral("slug")).toString();
    reply->deleteLater();
    if (slug.isEmpty()) {
        emit errorOccurred(tr("The course service did not return a course ID."));
        return;
    }
    requestBundle(slug, distanceUnit);
}

void OpenGolfMapProvider::requestBundle(const QString& slug,
                                        const QString& distanceUnit) {
    const QJsonObject body{
        {QStringLiteral("course"), slug},
        {QStringLiteral("fileType"), QStringLiteral("zip")},
        {QStringLiteral("assetFormat"), QStringLiteral("svg")},
        {QStringLiteral("scope"), QStringLiteral("bundle")},
        {QStringLiteral("distanceUnit"), distanceUnit},
        {QStringLiteral("includeLabels"), false},
        {QStringLiteral("includeRenderModel"), true},
        {QStringLiteral("includeNavigation"), true},
        {QStringLiteral("autoImport"), false},
    };
    auto request = requestFor(QStringLiteral("/api/v1/graphics"));
    request.setHeader(QNetworkRequest::ContentTypeHeader,
                      QStringLiteral("application/json"));
    m_activeReply = m_network.post(
        request, QJsonDocument(body).toJson(QJsonDocument::Compact));
    connect(m_activeReply, &QNetworkReply::downloadProgress, this,
            &OpenGolfMapProvider::downloadProgress);
    connect(m_activeReply, &QNetworkReply::finished, this, [this] {
        const QPointer<QNetworkReply> reply = m_activeReply;
        m_activeReply.clear();
        if (!reply) return;
        const QByteArray bytes = reply->readAll();
        if (reply->error() != QNetworkReply::NoError) {
            emit errorOccurred(tr("Course download failed: %1")
                                   .arg(reply->errorString()));
        } else if (reply->header(QNetworkRequest::ContentTypeHeader)
                       .toString()
                       .startsWith(QStringLiteral("application/zip"))) {
            emit bundleReady(bytes);
        } else {
            emit errorOccurred(tr("The course service returned the wrong file type."));
        }
        reply->deleteLater();
    });
}

void OpenGolfMapProvider::cancel() {
    if (m_activeReply) {
        m_activeReply->abort();
        m_activeReply->deleteLater();
        m_activeReply.clear();
    }
}

void OpenGolfMapProvider::checkReachability() {
    const QUrl requestedBase = m_baseUrl;
    auto request = requestFor(QStringLiteral("/api/v1/health"));
    auto* reply = m_network.get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply, requestedBase] {
        if (m_baseUrl == requestedBase) {
            setReachable(reply->error() == QNetworkReply::NoError);
        }
        reply->deleteLater();
    });
}

void OpenGolfMapProvider::setReachable(const bool value) {
    if (m_reachable == value) return;
    m_reachable = value;
    emit reachableChanged();
}

} // namespace opencaddie::courses
