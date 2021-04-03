#include "iconprovider.h"
#include <QNetworkReply>
#include <QTimer>
#include <QImageReader>
#include <QBuffer>
#include "context.h"
#include "feed.h"

namespace {
class IconImageResponse : public QQuickImageResponse {
public:
    QImage m_image;
    QString m_error;

    QString errorString() const override {
        return m_error;
    }

    QQuickTextureFactory *textureFactory() const override {
        return QQuickTextureFactory::textureFactoryForImage(m_image);
    }

    void onNetworkReplyFinished() {
        QNetworkReply *reply = qobject_cast<QNetworkReply*>(QObject::sender());
        bool success = m_image.loadFromData(reply->readAll());
        if (!success) {
            m_error = "failed to load image";
        }
        reply->deleteLater();
        emit finished();
    }
};
}

IconProvider::IconProvider(FeedCore::Context *ctx):
    m_nam(ctx->networkAccessManager())
{}

QQuickImageResponse *IconProvider::requestImageResponse(const QString &id, const QSize &requestedSize)
{
    auto *response = new IconImageResponse;
    QTimer::singleShot(0, m_nam, [nam=m_nam, id, response]{
        QNetworkRequest req(id.mid(1));
        req.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferCache);
        QNetworkReply *reply = nam->get(req);
        QObject::connect(reply, &QNetworkReply::finished, response, &IconImageResponse::onNetworkReplyFinished);
    });
    return response;
}

void IconProvider::discoverIcon(QNetworkAccessManager *nam, FeedCore::Feed *feed)
{
    QPointer<FeedCore::Feed> feed_ptr = feed;
    QTimer::singleShot(0, nam, [nam, feed_ptr]{
        if (!feed_ptr) { return; }
        QUrl iconUrl(feed_ptr->link());
        iconUrl.setPath("/favicon.ico");
        QNetworkRequest req(iconUrl);
        req.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferCache);
        QNetworkReply *reply = nam->get(req);
        FeedCore::Feed *feed = feed_ptr;
        QObject::connect(reply, &QNetworkReply::finished, feed, [reply, feed, iconUrl]{
            if (reply->error() != QNetworkReply::NoError) {
                // couldn't get file
                return;
            }
            QByteArray data = reply->readAll();
            QBuffer buffer(&data);
            buffer.open(QIODevice::ReadOnly);
            QImageReader reader(&buffer);
            bool success = reader.canRead();
            if (success) {
                feed->setIcon(iconUrl);
            } else {
                // unreadable image
            }
        });
    });
}
