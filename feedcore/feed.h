#ifndef FEED_H
#define FEED_H
#include <QObject>
#include <QUrl>
#include <Syndication/Feed>
#include "enums.h"
#include "future.h"

namespace FeedCore {
class Updater;

class Feed : public QObject {
    Q_OBJECT
public:
    QString name() const;
    virtual void setName(const QString &name) { populateName(name); }
    QUrl url() const;
    void setUrl(const QUrl &url);
    int unreadCount() const;
    LoadStatus status() const;
    void setStatus(LoadStatus status);
    virtual Updater *updater() = 0;
    virtual Future<ArticleRef> *startItemQuery(bool unreadFilter)=0;
    virtual void updateFromSource(const Syndication::FeedPtr &feed){ assert(false); };
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged);
    Q_PROPERTY(QUrl url READ url NOTIFY urlChanged);
    Q_PROPERTY(int unreadCount READ unreadCount NOTIFY unreadCountChanged);
    Q_PROPERTY(FeedCore::Enums::LoadStatus status READ status NOTIFY statusChanged);
    Q_PROPERTY(Updater *updater READ updater CONSTANT);
signals:
    void nameChanged();
    void urlChanged();
    void unreadCountChanged(int delta);
    void statusChanged();
    void itemAdded(const FeedCore::ArticleRef &item);
protected:
    explicit Feed(QObject *parent = nullptr);
    bool populateName(const QString &name);
    void populateUrl(const QUrl &url);
    void populateUnreadCount(int unreadCount);
    void incrementUnreadCount(int delta=1);
    inline void decrementUnreadCount() { incrementUnreadCount(-1); };
private:
    QString m_name;
    QUrl m_url;
    int m_unreadCount = 0;
    LoadStatus m_status = LoadStatus::Idle;
};
}
#endif // STOREDFEED_H
