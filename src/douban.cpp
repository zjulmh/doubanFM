#include "include/douban.h"
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>
#include <QUrl>
#include <QTextCodec>
#include <qjson/parser.h>
#include <QDebug>
#include <QtNetwork/QNetworkCookieJar>
#include <algorithm>
const QString DOUBAN_CAPTCHAID_ADDRESS = "http://douban.fm/j/new_captcha";
const QString DOUBAN_CAPTCHAIMAGE_ADDRESS = "http://douban.fm/misc/captcha?size=m&id=";
const QString DOUBAN_LOGIN_ADDRESS = "http://douban.fm/j/login";
const QString DOUBAN_SONGLIST_ADDRESS = "http://douban.fm/j/mine/playlist?";
const QString DOUBAN_FM_API_CHANNEL = "http://www.douban.com/j/app/radio/channels";
Douban::Douban(QWidget *parent) :
    QWidget(parent)
{
    qDebug() << Q_FUNC_INFO;
    for(int i = 0;i < DOUBAN_NETWORK_MANAGER_SIZE;i++)
        _manager[i] = NULL;
}
Douban::~Douban()
{
    for(int i = 0;i < DOUBAN_NETWORK_MANAGER_SIZE;i++)
        delete _manager[i];
}

void Douban::captchaIdRequest()
{
    qDebug() << Q_FUNC_INFO;
    if(_manager[0] == NULL)
    {
        _manager[0] = new QNetworkAccessManager(this);
        connect(_manager[0],SIGNAL(finished(QNetworkReply*)),this,SLOT(onReceiveCaptchaId(QNetworkReply*)));
    }
    _manager[0]->get(QNetworkRequest(QUrl(DOUBAN_CAPTCHAID_ADDRESS)));
}

void Douban::onReceiveCaptchaId(QNetworkReply *reply)
{
    qDebug() << Q_FUNC_INFO;
    QTextCodec *codec = QTextCodec::codecForName("utf-8");
    QString all = codec->toUnicode(reply->readAll());
    qDebug()  << all;
    _capture.captcha_id = all.remove("\"");
    reply->deleteLater();
    captchaImageRequest();
}

void Douban::captchaImageRequest()
{
    qDebug() << Q_FUNC_INFO;
    if(_manager[1] == NULL)
    {
        _manager[1] = new QNetworkAccessManager(this);
        connect(_manager[1],SIGNAL(finished(QNetworkReply*)),this,SLOT(onReceiveCaptchaImage(QNetworkReply*)));
    }
    qDebug() << DOUBAN_CAPTCHAIMAGE_ADDRESS + _capture.captcha_id;
    _manager[1]->get(QNetworkRequest(QUrl(DOUBAN_CAPTCHAIMAGE_ADDRESS + _capture.captcha_id)));
}

void Douban::onReceiveCaptchaImage(QNetworkReply *reply)
{
    qDebug() << Q_FUNC_INFO;
    const QByteArray data(reply->readAll());
    QPixmap pixmap;
    if(data.size())
    {
        pixmap.loadFromData(data);
        emit captchaImageLoad(pixmap);
    }
    else
        captchaIdRequest();
    reply->deleteLater();
}

void Douban::loginRequest(const QString alias, const QString password, const QString verification)
{
    qDebug() << Q_FUNC_INFO;
    QString args = QString("source=radio")
            + QString("&alias=") + alias
            + QString("&form_password=") + password
            + QString("&captcha_id=") + _capture.captcha_id
            + QString("&captcha_solution=") + verification
            + QString("&task=sync_channel_list");
    if(_manager[2] == NULL)
    {
        _manager[2] = new QNetworkAccessManager(this);
        _manager[2]->setCookieJar(new QNetworkCookieJar(this));
        connect(_manager[2],SIGNAL(finished(QNetworkReply*)),this,SLOT(onReceiveLogin(QNetworkReply*)));
    }
    QNetworkRequest request;
    request.setRawHeader("User-Agent","Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/28.0.1500.71 Safari/537.36");
    request.setRawHeader("Connection","keep-alive");
    request.setHeader(QNetworkRequest::ContentTypeHeader,QVariant("application/x-www-form-urlencoded"));
    request.setHeader(QNetworkRequest::ContentLengthHeader,QVariant(args.length()));
    request.setUrl(QUrl(DOUBAN_LOGIN_ADDRESS));
    _manager[2]->post(request,args.toAscii());
}
void Douban::onReceiveLogin(QNetworkReply *reply)
{
    qDebug() << Q_FUNC_INFO;
    _user.cookies.clear();
    _user.cookies = _manager[2]->cookieJar()->cookiesForUrl(QUrl(tr("http://douban.fm")));
    qDebug() << _user.cookies;
    QTextCodec *codec = QTextCodec::codecForName("utf-8");
    QString all = codec->toUnicode(reply->readAll());    
    QJson::Parser parser;
    bool ok;
    QVariant result = parser.parse(all.toAscii(),&ok);
    if(ok)
    {
        QVariantMap obj = result.toMap();
        if(obj["r"].toInt() != 0)
        {
            emit loginFailed(obj["err_msg"].toString());
            captchaIdRequest();
        }
        else
        {
            QVariantMap user_info = obj["user_info"].toMap();
            _user.ck = user_info["ck"].toString();
            _user.name = user_info["name"].toString();
            _user.uid = user_info["uid"].toString();
            _user.url = user_info["url"].toString();
            emit loginSucceed(_user);
        }
        QByteArray value;
        QNetworkCookie ckCookie("ck",value.append(_user.ck));
        ckCookie.setDomain(tr("douban.fm"));
        _user.cookies.push_back(ckCookie);
    }
    reply->deleteLater();
}

void Douban::logoutRequest()
{
    qDebug() << Q_FUNC_INFO;
    if(_manager[3] == NULL)
    {
        _manager[3] = new QNetworkAccessManager(this);
        connect(_manager[3],SIGNAL(finished(QNetworkReply*)),this,SLOT(onReceiveLogout(QNetworkReply*)));
    }
    QString logoutUrl = QString("http://douban.fm/partner/logout?source=radio&ck=") + _user.ck + QString("&no_login=y");
    _manager[3]->cookieJar()->setCookiesFromUrl(_user.cookies,QUrl(logoutUrl));
    QNetworkRequest request;
    request.setUrl(QUrl(logoutUrl));
    _manager[3]->get(request);
}

void Douban::onReceiveLogout(QNetworkReply *reply)
{

    qDebug() << Q_FUNC_INFO;
    QTextCodec *codec = QTextCodec::codecForName("utf-8");
    _user.cookies.clear();
    _user.cookies = _manager[3]->cookieJar()->cookiesForUrl(QUrl(tr("http://douban.fm")));
    qDebug() << _user.cookies;
    _user.ck = "";
    _user.name = "";
    _user.uid = "";
    _user.url = "";
    QString all = codec->toUnicode(reply->readAll());
    qDebug() << all;
    emit sendLogoutSucceed(true);
    reply->deleteLater();

}

void Douban::newSongListRequest(const qint32 cid,const quint32 sid,const double pt)
{
    qDebug() << Q_FUNC_INFO;
    QString str = QString("          ");
    int randomnum = qrand();
    str.setNum(randomnum,16);
    QString args = QString("type=n")
            + QString("&sid=") + QString::number(sid,10)
            + QString("&pt=") + QString::number(pt)
            + QString("&channel=") + QString::number(cid,10)
            + QString("&pb=64&from=mainsite")
            + QString("&r=") + str;

    qDebug() << DOUBAN_SONGLIST_ADDRESS + args;
    if(_manager[4] == NULL)
    {
        _manager[4] = new QNetworkAccessManager(this);
        connect(_manager[4],SIGNAL(finished(QNetworkReply*)),this,SLOT(onReceiveSongList(QNetworkReply*)));
    }
    _manager[4]->cookieJar()->setCookiesFromUrl(_user.cookies,QUrl(DOUBAN_SONGLIST_ADDRESS + args));
//    QVariant var;
//    var.setValue(_user.cookies);
    QNetworkRequest request;
//    request.setHeader(QNetworkRequest::CookieHeader,var);
    request.setUrl(QUrl(DOUBAN_SONGLIST_ADDRESS + args));
    _manager[4]->get(request);

}

void Douban::onReceiveSongList(QNetworkReply *reply)
{
    qDebug() << Q_FUNC_INFO;

    QTextCodec *codec = QTextCodec::codecForName("utf-8");
    QString all = codec->toUnicode(reply->readAll());
    qDebug() << all;
    QJson::Parser parser;
    bool ok;
    QVariant result = parser.parse(all.toAscii(),&ok);
    if(ok)
    {
        QVariantMap obj = result.toMap();
        if(obj["r"] == 0)
        {
            _songList.clear();
            QVariantList songs = obj["song"].toList();
            foreach (QVariant item, songs) {
                QVariantMap ds = item.toMap();
                DoubanFMSong song;
                song.album = ds["album"].toString();
                song.picture = ds["picture"].toString();
                song.ssid = ds["ssid"].toString();
                song.artist = ds["artist"].toString();
                song.url = ds["url"].toString();
                song.company = ds["company"].toString();
                song.title = ds["title"].toString();
                song.public_time = ds["public_time"].toString();
                song.sid = ds["sid"].toUInt();
                song.aid = ds["aid"].toUInt();
                song.albumtitle = ds["albumtitle"].toString();
                song.like = ds["like"].toBool();
                _songList.push_back(song);
            }
            emit sendSongList(_songList);
        }
    }
    reply->deleteLater();

}

void Douban::channelListRequest()
{
    qDebug() << Q_FUNC_INFO;

    if(_manager[5] == NULL)
    {
        _manager[5] = new QNetworkAccessManager(this);
        connect(_manager[5],SIGNAL(finished(QNetworkReply*)),this,SLOT(onReceiveChannelList(QNetworkReply*)));
    }
    _manager[5]->get(QNetworkRequest(QUrl(DOUBAN_FM_API_CHANNEL)));
}

void Douban::onReceiveChannelList(QNetworkReply *reply)
{
    qDebug() << Q_FUNC_INFO;
    QTextCodec *codec = QTextCodec::codecForName("utf-8");
    QString all = codec->toUnicode(reply->readAll());

    bool ok;
    QJson::Parser parser;
    QVariant result = parser.parse(all.toAscii(),&ok);

    if(ok)
    {
        _channelList.clear();
        QVariantMap obj = result.toMap();
        QVariantList channels = obj["channels"].toList();
        foreach(const QVariant item,channels)
        {
            QVariantMap channel = item.toMap();
            DoubanChannel dc;
            dc.name = channel["name"].toString();
            dc.name_en = channel["name_en"].toString();
            dc.seq_id = channel["seq_id"].toInt();
            dc.channel_id = channel["channel_id"].toInt();
            dc.abbr_en = channel["abbr_en"].toString();
            _channelList.push_back(dc);
        }
        bool isLogin = this->hasLogin();
        emit sendChannelList(_channelList,isLogin);
    }
    reply->deleteLater();
}

void Douban::rateSongRequest(const qint32 cid, const quint32 sid, const double pt,bool rateIt)
{
    qDebug() << Q_FUNC_INFO;
    QString str = QString("          ");
    int randomnum = qrand();
    str.setNum(randomnum,16);
    QString args = QString("type=")
            + (rateIt ? QString("r") : QString("u"))
            + QString("&sid=") + QString::number(sid,10)
            + QString("&channel=") + QString::number(cid,10)
            + QString("&pt=") + QString::number(pt,'f',3)
            + QString("&pb=64&from=mainsite&r=")
            + str;
    if(_manager[6] == NULL)
    {
        _manager[6] = new QNetworkAccessManager(this);
        connect(_manager[6],SIGNAL(finished(QNetworkReply*)),this,SLOT(onReceiveRateSong(QNetworkReply*)));
    }
    _manager[6]->cookieJar()->setCookiesFromUrl(_user.cookies,QUrl(DOUBAN_SONGLIST_ADDRESS + args));
    QNetworkRequest request;
    request.setUrl(QUrl(DOUBAN_SONGLIST_ADDRESS + args));
    _manager[6]->get(request);
}

void Douban::onReceiveRateSong(QNetworkReply *reply)
{
    qDebug() << Q_FUNC_INFO;
    QTextCodec *codec = QTextCodec::codecForName("utf-8");
    QString all = codec->toUnicode(reply->readAll());
    qDebug() << all;
    QJson::Parser parser;
    bool ok;
    QVariant result = parser.parse(all.toAscii(),&ok);
    if(ok)
    {
        QVariantMap obj = result.toMap();
        if(obj["r"] == 0)
        {
            emit sendRateSong(true);
        }
        else
            emit sendRateSong(false);
    }
    reply->deleteLater();
}

void Douban::byeSongRequest(const qint32 cid, const quint32 sid, const double pt)
{
    qDebug() << Q_FUNC_INFO;
    QString str = QString("          ");
    int randomnum = qrand();
    str.setNum(randomnum,16);
    QString args = QString("type=b")
            + QString("&sid=") + QString::number(sid,10)
            + QString("&channel=") + QString::number(cid,10)
            + QString("&pt=") + QString::number(pt,'f',3)
            + QString("&pb=64&from=mainsite&r=")
            + str;
    if(_manager[7] == NULL)
    {
        _manager[7] = new QNetworkAccessManager(this);
        connect(_manager[7],SIGNAL(finished(QNetworkReply*)),this,SLOT(onReceiveByeSong(QNetworkReply*)));
    }
    _manager[7]->cookieJar()->setCookiesFromUrl(_user.cookies,QUrl(DOUBAN_SONGLIST_ADDRESS + args));
    QNetworkRequest request;
    request.setUrl(QUrl(DOUBAN_SONGLIST_ADDRESS + args));
    _manager[7]->get(request);
}

void Douban::onReceiveByeSong(QNetworkReply *reply)
{
    qDebug() << Q_FUNC_INFO;
    QTextCodec *codec = QTextCodec::codecForName("utf-8");
    QString all = codec->toUnicode(reply->readAll());
    qDebug() << all;
    QJson::Parser parser;
    bool ok;
    QVariant result = parser.parse(all.toAscii(),&ok);
    if(ok)
    {
        QVariantMap obj = result.toMap();
        if(obj["r"] == 0)
        {
            emit sendByeSong(true);
        }
        else
            emit sendByeSong(false);
    }
    reply->deleteLater();
}

void Douban::skipSongRequest(const qint32 cid, const quint32 sid, const double pt)
{
    qDebug() << Q_FUNC_INFO;
    QString str = QString("          ");
    int randomnum = qrand();
    str.setNum(randomnum,16);
    QString args = QString("type=s")
            + QString("&sid=") + QString::number(sid,10)
            + QString("&channel=") + QString::number(cid,10)
            + QString("&pt=") + QString::number(pt,'f',3)
            + QString("&pb=64&from=mainsite&r=")
            + str;
    qDebug() << args;
    if(_manager[8] == NULL)
    {
        _manager[8] = new QNetworkAccessManager(this);
        connect(_manager[8],SIGNAL(finished(QNetworkReply*)),this,SLOT(onReceiveSkipSong(QNetworkReply*)));
    }
    _manager[8]->cookieJar()->setCookiesFromUrl(_user.cookies,QUrl(DOUBAN_SONGLIST_ADDRESS + args));
    QNetworkRequest request;
    request.setUrl(QUrl(DOUBAN_SONGLIST_ADDRESS + args));
    _manager[8]->get(request);
}

void Douban::onReceiveSkipSong(QNetworkReply *reply)
{
    qDebug() << Q_FUNC_INFO;
    QTextCodec *codec = QTextCodec::codecForName("utf-8");
    QString all = codec->toUnicode(reply->readAll());
    qDebug() << all;
    QJson::Parser parser;
    bool ok;

    QVariant result = parser.parse(all.toAscii(),&ok);
    if(ok)
    {
        QVariantMap obj = result.toMap();
        if(obj["r"] == 0)
            emit sendSkipSong(true);
        else
            emit sendSkipSong(false);
    }
    reply->deleteLater();
}

void Douban::endSongRequest(const qint32 cid, const quint32 sid, const double pt)
{
    qDebug() << Q_FUNC_INFO;
    QString str = QString("          ");
    int randomnum = qrand();
    str.setNum(randomnum,16);
    QString args = QString("type=s")
            + QString("&sid=") + QString::number(sid,10)
            + QString("&channel=") + QString::number(cid,10)
            + QString("&pt=") + QString::number(pt,'f',3)
            + QString("&pb=64&from=mainsite&r=")
            + str;
    if(_manager[9] == NULL)
    {
        _manager[9] = new QNetworkAccessManager(this);
        connect(_manager[9],SIGNAL(finished(QNetworkReply*)),this,SLOT(onReceiveEndSong(QNetworkReply*)));
    }
    _manager[9]->cookieJar()->setCookiesFromUrl(_user.cookies,QUrl(DOUBAN_SONGLIST_ADDRESS + args));
    QNetworkRequest request;
    request.setUrl(QUrl(DOUBAN_SONGLIST_ADDRESS + args));
    _manager[9]->get(request);
}

void Douban::onReceiveEndSong(QNetworkReply *reply)
{
    qDebug() << Q_FUNC_INFO;
    QTextCodec *codec = QTextCodec::codecForName("utf-8");
    QString all = codec->toUnicode(reply->readAll());
    qDebug() << all;
    QJson::Parser parser;
    bool ok;
    QVariant result = parser.parse(all.toAscii(),&ok);
    if(ok)
    {
        QVariantMap obj = result.toMap();
        if(obj["r"] == 0)
            emit sendEndSong(true);
        else
            emit sendEndSong(false);
    }

    reply->deleteLater();
}

bool Douban::hasLogin(){
    qDebug() << Q_FUNC_INFO;
    return !_user.ck.isEmpty()
            && !_user.cookies.isEmpty()
            && !_user.name.isEmpty()
            && !_user.uid.isEmpty()
            && !_user.url.isEmpty();
}

void Douban::setDoubanUser(DoubanUser user)
{
    _user = user;
}

DoubanUser Douban::getDoubanUser()
{
    return _user;
}
