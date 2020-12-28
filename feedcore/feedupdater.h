#ifndef FEEDUPDATER_H
#define FEEDUPDATER_H

#include <Syndication/Feed>
#include <QObject>
#include <time.h>
#include <memory>

#include "enums.h"

namespace FeedCore {

class Feed;

class FeedUpdater : public QObject
{
    Q_OBJECT
public:
    explicit FeedUpdater(Feed *feed, time_t updateInterval, time_t lastUpdate, QObject *parent);
    ~FeedUpdater();

    virtual void run() = 0;

    void start();
    void start(time_t timestamp);
    QString error();
    Feed *feed();
    time_t nextUpdate();
    bool needsUpdate(time_t timestamp);
    bool updateIfNecessary(time_t timestamp);

protected:
    void finish();
    void setError(const QString &errorMsg);

private:
    struct PrivData;
    std::unique_ptr<PrivData> priv;
};

}

#endif // FEEDUPDATER_H
