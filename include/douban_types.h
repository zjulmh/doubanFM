#ifndef DOUBAN_TYPES_H
#define DOUBAN_TYPES_H
#include <QtNetwork/QNetworkCookie>
struct DoubanUser {
    QString uid;
    QString ck;
    QString name;
    QString url;
    QList<QNetworkCookie> cookies;
};

struct DoubanChannel {
    QString name;
    qint32 seq_id;
    QString abbr_en;
    qint32 channel_id;
    QString name_en;
};

struct DoubanFMSong {
    QString album;
    QString picture;
    QString ssid;
    QString artist;
    QString url;
    QString company;
    QString title;
    double rating_avg;
    quint32 length;
    QString subtype;
    QString public_time;
    quint32 sid;
    quint32 aid;
    quint32 kbps;
    QString albumtitle;
    bool like;
};

struct Captcha{
    QString captcha_id;
    QString captcha_solution;
};

#endif // DOUBAN_TYPES_H
