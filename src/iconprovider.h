/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef ICONPROVIDER_H
#define ICONPROVIDER_H
#include <QQuickAsyncImageProvider>

namespace FeedCore
{
class Feed;
class Context;
}

/**
 * The image provider for feed icons.
 *
 * Requests for feed icon URLs are routed through this class so that they
 * can be cached more aggressvely than normal network content.
 *
 * TODO This class currently also handles icon discovery, but that should probably
 * be moved to core.
 */
class IconProvider : public QQuickAsyncImageProvider
{
public:
    IconProvider();
    ~IconProvider();
    QQuickImageResponse *requestImageResponse(const QString &id, const QSize &requestedSize) override;
    static void discoverIcon(FeedCore::Feed *feed);

private:
    static IconProvider *s_instance;
    QSharedPointer<QNetworkAccessManager> m_nam;
};

#endif // ICONPROVIDER_H
