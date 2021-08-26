/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "context.h"
#include <QSet>
#include <QNetworkConfigurationManager>
#include <QFile>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <QDebug>
#include "storage.h"
#include "scheduler.h"
#include "feed.h"
#include "future.h"
#include "provisionalfeed.h"

namespace FeedCore {

struct Context::PrivData {
    Context *parent;
    Storage *storage;
    QSet<Feed*> feeds;
    qint64 updateInterval{ 0 };
    qint64 expireAge { 0 };
    Scheduler *updateScheduler;
    QNetworkConfigurationManager ncm;

    PrivData(Storage *storage, Context *parent);
    void configureUpdates(Feed *feed, const QDateTime &timestamp=QDateTime::currentDateTime()) const;
};

Context::Context(Storage *storage, QObject *parent)
    : QObject(parent),
      d{ std::make_unique<PrivData>(storage, this) }
{
    QObject::connect(&d->ncm, &QNetworkConfigurationManager::configurationAdded, d->updateScheduler, &Scheduler::clearErrors);
    QObject::connect(&d->ncm, &QNetworkConfigurationManager::configurationChanged, d->updateScheduler, &Scheduler::clearErrors);

    Future<Feed*> *getFeeds { d->storage->getFeeds() };
    QObject::connect(getFeeds, &BaseFuture::finished, this, [this, getFeeds] {
        populateFeeds(getFeeds->result());
    });
    d->updateScheduler->start();
}


Context::~Context() = default;

Context::PrivData::PrivData(Storage *storage, Context *parent) :
    parent(parent),
    storage(storage),
    updateScheduler(new Scheduler(parent))
{
    storage->setParent(parent);
}

void Context::PrivData::configureUpdates(Feed *feed, const QDateTime &timestamp) const
{
    auto updateMode{feed->updateMode()};
    if (updateMode==Feed::DefaultUpdateMode) {
        feed->setUpdateInterval(updateInterval);
    }
    if (updateMode==Feed::ManualUpdateMode) {
        updateScheduler->unschedule(feed);
    } else {
        updateScheduler->schedule(feed, timestamp);
    }
}

const QSet<Feed*> &Context::getFeeds()
{
    return d->feeds;
}

void Context::addFeed(Feed *feed)
{
    Future<Feed*> *q { d->storage->storeFeed(feed) };
    QObject::connect(q, &BaseFuture::finished, this, [this, q]{
        populateFeeds(q->result());
    });
}

Future<ArticleRef> *Context::getArticles(bool unreadFilter)
{
    if (unreadFilter) {
        return d->storage->getUnread();
    }
    return d->storage->getAll();
}

Future<ArticleRef> *Context::getStarred()
{
    return d->storage->getStarred();
}

void Context::requestUpdate()
{
    const auto &timestamp = QDateTime::currentDateTime();
    const auto &feeds = d->feeds;
    for (Feed *const entry : feeds) {
        entry->updater()->start(timestamp);
    }
}

void Context::abortUpdates()
{
    const auto &feeds = d->feeds;
    for(Feed *const entry : feeds) {
        entry->updater()->abort();
    }
}

qint64 Context::defaultUpdateInterval()
{
    return d->updateInterval;
}

void Context::setDefaultUpdateInterval(qint64 defaultUpdateInterval)
{
    if (d->updateInterval == defaultUpdateInterval) {
        return;
    }
    d->updateInterval = defaultUpdateInterval;
    for (Feed *feed : qAsConst(d->feeds)) {
        if (feed->updateMode() == Feed::DefaultUpdateMode) {
            feed->setUpdateInterval(defaultUpdateInterval);
        }
    }
    emit defaultUpdateIntervalChanged();
}

qint64 Context::expireAge()
{
    return d->expireAge;
}

void Context::setExpireAge(qint64 expireAge)
{
    if (d->expireAge == expireAge) {
        return;
    }
    d->expireAge = expireAge;
    for (Feed *feed : qAsConst(d->feeds)) {
        feed->setExpireAge(expireAge);
    }
    emit expireAgeChanged();
}

static QString urlToPath(const QUrl &url)
{
    QString path(url.toLocalFile());
#ifdef ANDROID
    // TODO maybe Qt has a better way to do this?
    if (path.isEmpty()) {
        if (url.scheme() == "content") {
            path = QLatin1String("content:") + url.path();
        }
    }
#endif
    return path;
}

void Context::exportOpml(const QUrl &url) const
{
    QFile file(urlToPath(url));
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return;
    }
    QXmlStreamWriter xml(&file);
    xml.writeStartDocument();
    xml.writeStartElement("opml");
    xml.writeAttribute("version", "1.0");
    xml.writeStartElement("head");
    xml.writeEndElement();
    xml.writeStartElement("body");
    for(auto *feed : qAsConst(d->feeds)) {
        xml.writeEmptyElement("outline");
        xml.writeAttribute("type", "rss");
        xml.writeAttribute("text", feed->name());
        xml.writeAttribute("xmlUrl", feed->url().toString());
    }
    xml.writeEndElement();
    xml.writeEndElement();
    file.close();
}

void Context::importOpml(const QUrl &url)
{
    QFile file(urlToPath(url));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "failed to open file" << url;
        return;
    }
    QXmlStreamReader xml(&file);
    while (!xml.atEnd()) {
        xml.readNext();
        if (xml.isStartElement() && xml.name()=="outline") {
            QXmlStreamAttributes attrs { xml.attributes() };
            if (!attrs.hasAttribute("xmlUrl")) {
                continue;
            }
            QUrl xmlUrl(attrs.value("xmlUrl").toString());
            if (!xmlUrl.isValid()) {
                continue;
            }
            ProvisionalFeed feed;
            feed.setUrl(xmlUrl);
            if (attrs.hasAttribute("text")) {
                feed.setName(attrs.value("text").toString());
            }
            addFeed(&feed);
        }
    }
    file.close();
}

void Context::populateFeeds(const QVector<Feed*> &feeds)
{
    const QDateTime timestamp = QDateTime::currentDateTime();
    for(const auto &feed : feeds) {
        d->feeds.insert(feed);
        feed->setExpireAge(d->expireAge);
        d->configureUpdates(feed, timestamp);
        QObject::connect(feed, &Feed::updateModeChanged, this, [this, feed]{
            d->configureUpdates(feed);
        });
        emit feedAdded(feed);
    }
}

}
