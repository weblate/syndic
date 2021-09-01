/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef UNREADITEMMODEL_H
#define UNREADITEMMODEL_H
#include <QModelIndex>
#include <QQmlParserStatus>
#include <memory>
#include "feed.h"
#include "future.h"
#include "articleref.h"
namespace FeedCore {
class Context;
}
class ArticleListModel : public QAbstractListModel, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)
    Q_PROPERTY(bool unreadFilter READ unreadFilter WRITE setUnreadFilter NOTIFY unreadFilterChanged);
    Q_PROPERTY(FeedCore::Feed::LoadStatus status READ status NOTIFY statusChanged);
    Q_PROPERTY(FeedCore::Feed *feed READ feed WRITE setFeed NOTIFY feedChanged);
public:
    explicit ArticleListModel(QObject *parent=nullptr);
    ~ArticleListModel();
    FeedCore::Feed *feed() const;
    void setFeed(FeedCore::Feed *feed);
    bool unreadFilter() const;
    void setUnreadFilter(bool unreadFilter);
    FeedCore::LoadStatus status();
    Q_INVOKABLE void requestUpdate();
    Q_INVOKABLE void markAllRead();
    Q_INVOKABLE void removeRead();
    int rowCount(const QModelIndex &parent = QModelIndex()) const final;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const final;
    QHash<int, QByteArray> roleNames() const override;
    void classBegin() override;
    void componentComplete() override;
signals:
    void feedChanged();
    void unreadFilterChanged();
    void statusChanged();
private:
    struct PrivData;
    std::unique_ptr<PrivData> d;
    FeedCore::Future<FeedCore::ArticleRef> *getItems();
    void setStatusFromUpstream();
    void setStatus(FeedCore::LoadStatus status);
    void refresh();
    void onItemAdded(const FeedCore::ArticleRef &item);
    void insertAndNotify(int index, const FeedCore::ArticleRef &item);
    void refreshMerge();
    void onRefreshFinished(FeedCore::Future<FeedCore::ArticleRef> *sender);
    void onMergeFinished(FeedCore::Future<FeedCore::ArticleRef> *sender);
    void onStatusChanged();
};
#endif // UNREADITEMMODEL_H
