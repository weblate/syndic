/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef UPDATABLEFEED_H
#define UPDATABLEFEED_H
#include "feed.h"
#include <Syndication/Feed>
#include <Syndication/Item>
namespace FeedCore {
class Updater;

class UpdatableFeed : public Feed
{
public:
    virtual Updater *updater() final;
protected:
    explicit UpdatableFeed(QObject *parent);
private:
    virtual void updateFromSource(const Syndication::FeedPtr &feed);
    virtual void updateSourceArticle(const Syndication::ItemPtr &article) = 0;
    class UpdaterImpl;
    UpdaterImpl *m_updater;
};

}

#endif // UPDATABLEFEED_H
