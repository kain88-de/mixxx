#include <QNetworkReply>

#include <lastfm/ws.h>
#include <lastfm/Tag.h>
#include <lastfm/Track.h>

#include "lastfmclient.h"

const int LastFmClient::m_DefaultTimeout = 5000; // msec

LastFmClient::LastFmClient(QObject *parent)
            : QObject(parent),
              m_timeouts(m_DefaultTimeout, this) {
    lastfm::ws::ApiKey = "154ac2038b19ca6d5d3ef109eca3a7f8";
    lastfm::ws::SharedSecret = "c084cb4e9cc877fff7675dccd1633962";
}

void LastFmClient::start(int id, const QString& artist, const QString& title) {
    lastfm::MutableTrack track;
    track.setArtist(artist);
    track.setTitle(title);

    QNetworkReply* reply = track.getTopTags();
    connect(reply, SIGNAL(finished()), SLOT(requestFinished()));
    m_requests[reply] = id;

    m_timeouts.addReply(reply);
}


void LastFmClient::requestFinished() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply)
        return;

    reply->deleteLater();
    if (!m_requests.contains(reply))
        return;
    int id = m_requests.take(reply);

    TagCounts tags = lastfm::Tag::list(reply);
    emit finished(id, tags);
}


void LastFmClient::cancel(int id) {
    QNetworkReply* reply = m_requests.key(id);
    m_requests.remove(reply);
    delete reply;
}

void LastFmClient::cancelAll() {
    qDeleteAll(m_requests.keys());
    m_requests.clear();
}
