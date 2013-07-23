#ifndef DOUBAN_H
#define DOUBAN_H

#include <QWidget>
#include "douban_types.h"
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkCookie>
#include <QPixmap>
const int DOUBAN_NETWORK_MANAGER_SIZE = 10;
class Douban : public QWidget
{
    Q_OBJECT
public:
    explicit Douban(QWidget *parent = 0);
    ~Douban();
    bool hasLogin();
    void setDoubanUser(DoubanUser user);
    DoubanUser getDoubanUser();
private:
    DoubanUser _user;
    Captcha _capture;
    QList<DoubanFMSong> _songList;
    QList<DoubanChannel> _channelList;
    //0 : get the captcha id
    //1 : get the captcha image : need the captcha id
    //2 : login request : need the user alias,password and verification
    //3 : logout request
    //4 : new song list request :need channel id , song id and past time,type = n
    //5 : new channel list request
    //6 : rate song request :need channel id , song id and past time ,type = r/u
    //7 : bye song request : need channel id ,song id and past time ,type = b
    //8 : skip song request : need channel id ,song id and past time,type = s
    //9 : end song request : need channel id ,song id and past time,type = e
    QNetworkAccessManager *_manager[DOUBAN_NETWORK_MANAGER_SIZE];
signals:
    void loginSucceed(DoubanUser user);
    void captchaImageLoad(QPixmap captchaImage);
    void loginFailed(const QString errMsg);
    void sendSongList(QList<DoubanFMSong> songList);
    void sendChannelList(QList<DoubanChannel> channelList,bool isLogin);
    void sendRateSong(bool succeed);
    void sendByeSong(bool succeed);
    void sendSkipSong(bool succeed);
    void sendEndSong(bool succeed);
    void sendLogoutSucceed(bool succeed);

private slots:
    void onReceiveCaptchaId(QNetworkReply *reply);
    void onReceiveCaptchaImage(QNetworkReply *reply);
    void onReceiveLogin(QNetworkReply *reply);
    void onReceiveLogout(QNetworkReply *reply);
    void onReceiveSongList(QNetworkReply *reply);
    void onReceiveChannelList(QNetworkReply *reply);
    void onReceiveRateSong(QNetworkReply *reply);
    void onReceiveSkipSong(QNetworkReply *reply);
    void onReceiveByeSong(QNetworkReply *reply);
    void onReceiveEndSong(QNetworkReply *reply);
public slots:
    void captchaIdRequest();
    void captchaImageRequest();
    void loginRequest(const QString alias,const QString password,const QString verification);
    void logoutRequest();
    void newSongListRequest(const qint32 cid,const quint32 sid,const double pt);
    void channelListRequest();
    void rateSongRequest(const qint32 cid,const quint32 sid,const double pt,bool rateIt);
    void byeSongRequest(const qint32 cid,const quint32 sid,const double pt);
    void skipSongRequest(const qint32 cid,const quint32 sid,const double pt);
    void endSongRequest(const qint32 cid,const quint32 sid,const double pt);
};

#endif // DOUBAN_H
