#include "include/doubanfm.h"
#include "ui_doubanfm.h"
#include <QNetworkAccessManager>
#include <QSettings>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTime>
#include <QPixmap>
#include <QPropertyAnimation>
#include <QDebug>
#include "include/qxtglobalshortcut.h"
doubanFM::doubanFM(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::doubanFM)
{
    qDebug() << Q_FUNC_INFO;
    ui->setupUi(this);


    currentSongIndex = 0;
    pauseState = false;

    _channelListWidget = new channelList(this);
    _channelListWidget->setchannelId(currentChannelId);
    _channelListWidget->move(-_channelListWidget->getInvisibleWidth(),this->geometry().y());

    _pauseMaskWidget = new pauseMaskWidget(this);
    _pauseMaskWidget->move(250,0);
    _pauseMaskWidget->setVisible(false);


    _loginwidget = new doubanLogin(this);
    _loginwidget->move(this->geometry().x(),-_loginwidget->geometry().height());

    _mediaObject = new Phonon::MediaObject(this);
    _audioOutput = new Phonon::AudioOutput(this);
    Phonon::createPath(_mediaObject,_audioOutput);
    ui->seekSlider->setMediaObject(_mediaObject);


    albumImageManager = new QNetworkAccessManager(this);
    _mydouban = new Douban(this);

    readSettings();
    qDebug() << "read setting" << _mydouban->hasLogin();
    if(_mydouban->hasLogin())
    {
        ui->userLoginButton->setToolTip(tr("click to logout"));
        ui->userLabel->setText(_mydouban->getDoubanUser().name);
    }
    else
    {
        ui->userLoginButton->setToolTip(tr("click to login"));
        ui->userLabel->setText(tr("Logout"));
    }

    nextShortCut = new QxtGlobalShortcut(QKeySequence("Alt+s"),this);
    pauseShortCut = new QxtGlobalShortcut(QKeySequence("Alt+p"),this);
    rateShortCut = new QxtGlobalShortcut(QKeySequence("Alt+r"),this);
    trashShortCut = new QxtGlobalShortcut(QKeySequence("Alt+t"),this);

    connect(nextShortCut,SIGNAL(activated()),this,SLOT(onSkipButtonClicked()));
    connect(pauseShortCut,SIGNAL(activated()),this,SLOT(onPauseButtonClicked()));
    connect(rateShortCut,SIGNAL(activated()),this,SLOT(onRateButtonClicked()));
    connect(trashShortCut,SIGNAL(activated()),this,SLOT(onTrashButtonClicked()));


    //connect the mainwidget signals and the slots
    connect(ui->volumeSlider,SIGNAL(valueChanged(int)),this,SLOT(onVolumneChange(int)));
    connect(ui->pauseButton,SIGNAL(clicked()),this,SLOT(onPauseButtonClicked()));
    connect(ui->userLoginButton,SIGNAL(clicked()),this,SLOT(onLoginButtonClicked()));
    connect(_mediaObject,SIGNAL(tick(qint64)),this,SLOT(playTick(qint64)));
    connect(albumImageManager,SIGNAL(finished(QNetworkReply*)),this,SLOT(receiveAlbumImage(QNetworkReply*)));
    connect(this,SIGNAL(albumImageRequest(QString)),this,SLOT(onAlbumImage(QString)));
    connect(ui->rateSongButton,SIGNAL(clicked()),this,SLOT(onRateButtonClicked()));
    connect(ui->nextSongButton,SIGNAL(clicked()),this,SLOT(onSkipButtonClicked()));
    connect(ui->byeSongButton,SIGNAL(clicked()),this,SLOT(onTrashButtonClicked()));
    connect(_mediaObject,SIGNAL(currentSourceChanged(Phonon::MediaSource)),this,SLOT(sourceChanged(Phonon::MediaSource)));
    connect(_mediaObject,SIGNAL(stateChanged(Phonon::State,Phonon::State)),this,SLOT(stateChanged(Phonon::State,Phonon::State)));

//    connect the mainwidget signals and the slots of other widget
    connect(this,SIGNAL(sendlogoutRequest()),_mydouban,SLOT(logoutRequest()));
    connect(this,SIGNAL(sendSongListRequest(qint32,quint32,double)),_mydouban,SLOT(newSongListRequest(qint32,quint32,double)));
    connect(this,SIGNAL(sendSkipSongRequest(qint32,quint32,double)),_mydouban,SLOT(skipSongRequest(qint32,quint32,double)));
    connect(this,SIGNAL(sendRateSongRequest(qint32,quint32,double,bool)),_mydouban,SLOT(rateSongRequest(qint32,quint32,double,bool)));
    connect(this,SIGNAL(sendEndSongRequest(qint32,quint32,double)),_mydouban,SLOT(endSongRequest(qint32,quint32,double)));
    connect(this,SIGNAL(sendByeSongRequest(qint32,quint32,double)),_mydouban,SLOT(byeSongRequest(qint32,quint32,double)));

//  connect the signals of other widget and the mainwidget slot
    connect(_mydouban,SIGNAL(loginSucceed(DoubanUser)),this,SLOT(loginSucceed(DoubanUser)));
    connect(_mydouban,SIGNAL(sendSongList(QList<DoubanFMSong>)),this,SLOT(receiveSongList(QList<DoubanFMSong>)));
    connect(_mydouban,SIGNAL(sendRateSong(bool)),this,SLOT(receiveRateSong(bool)));
    connect(_mydouban,SIGNAL(sendSkipSong(bool)),this,SLOT(receiveSkipSong(bool)));
    connect(_mydouban,SIGNAL(sendByeSong(bool)),this,SLOT(receiveByeSong(bool)));
    connect(_mydouban,SIGNAL(sendEndSong(bool)),this,SLOT(receiveEndSong(bool)));
    connect(_mydouban,SIGNAL(sendChannelList(QList<DoubanChannel>,bool)),this,SLOT(receiveChannelList(QList<DoubanChannel>,bool)));
    connect(_mydouban,SIGNAL(sendLogoutSucceed(bool)),this,SLOT(logoutSucceed(bool)));
    connect(_pauseMaskWidget,SIGNAL(mousePressed()),this,SLOT(pauseMaskWidgetPressed()));
    connect(_channelListWidget,SIGNAL(channelChanged(qint32)),this,SLOT(receiveChannelChange(qint32)));


    //connect the other widget signal and slots
    connect(_mydouban,SIGNAL(captchaImageLoad(QPixmap)),_loginwidget,SLOT(onCaptchaImageLoad(QPixmap)));
    connect(_mydouban,SIGNAL(loginFailed(QString)),_loginwidget,SLOT(onLoginFailed(QString)));
    connect(_mydouban,SIGNAL(sendChannelList(QList<DoubanChannel>,bool)),_channelListWidget,SLOT(onReceiveChannelList(QList<DoubanChannel>,bool)));
    connect(_loginwidget,SIGNAL(loginRequest(QString,QString,QString)),_mydouban,SLOT(loginRequest(QString,QString,QString)));

    _mydouban->channelListRequest();
}

doubanFM::~doubanFM()
{
    writeSettings();
    _mediaObject->stop();
    delete _mydouban;
    delete _mediaObject;
    delete _audioOutput;
    delete albumImageManager;
    delete _pauseMaskWidget;
    delete _loginwidget;
    delete _channelListWidget;
    delete ui;

}

//mainwidget signals and slots
void doubanFM::onLoginButtonClicked()
{
    qDebug() << Q_FUNC_INFO;
    if(!_mydouban->hasLogin())
    {
        _mydouban->captchaIdRequest();
        QPropertyAnimation *anim = new QPropertyAnimation(_loginwidget,"geometry");
        anim->setStartValue(_loginwidget->geometry());
        anim->setEndValue(QRect(0,0,_loginwidget->width(),_loginwidget->height()));
        anim->setDuration(300);
        anim->setEasingCurve(QEasingCurve::Linear);
        anim->start();

    }
    else
    {
        emit sendlogoutRequest();
    }
}

void doubanFM::onPauseButtonClicked()
{
    qDebug() << Q_FUNC_INFO;
    if(!pauseState)
    {
        pauseState = true;
        _mediaObject->pause();
        _pauseMaskWidget->setVisible(true);
    }
    else
    {
        pauseState = false;
        _mediaObject->play();
        _pauseMaskWidget->setVisible(false);
    }
}

void doubanFM::onVolumneChange(int value)
{
    _audioOutput->setVolume(value / 100.0);
}

void doubanFM::playTick(qint64 time)
{
    QTime displayTime(0,(time / 60000) % 60,(time / 1000) % 60);
    ui->runningtime->setText(displayTime.toString("m:ss"));
}

void doubanFM::onAlbumImage(const QString picUrl)
{
    qDebug() << Q_FUNC_INFO;
    QString picUrlhighres = picUrl;
    picUrlhighres.replace("mpic","lpic");
    albumImageManager->get(QNetworkRequest(QUrl(picUrlhighres)));
}

void doubanFM::receiveAlbumImage(QNetworkReply *reply)
{
    qDebug() << Q_FUNC_INFO;
    if(QNetworkReply::NoError != reply->error())
    {
        reply->deleteLater();
        return;
    }
    const QByteArray data(reply->readAll());
    int w = ui->albumImageLabel->width();
    int h = ui->albumImageLabel->height();
    if(data.size())
    {
        QPixmap pixmap;
        pixmap.loadFromData(data);
        ui->albumImageLabel->setPixmap(pixmap.scaled(w,h,Qt::IgnoreAspectRatio));
    }
    else
    {
        ui->albumImageLabel->setText(tr("No Album Image"));
    }
    reply->deleteLater();
}

void doubanFM::onRateButtonClicked()
{
    qDebug() << Q_FUNC_INFO;
    qDebug() << currentChannelId;
    double pt = _mediaObject->currentTime() / 1000.0;
    if(_songList[currentSongIndex].like)
        emit sendRateSongRequest(currentChannelId,currentSongId,pt,false);
    else
        emit sendRateSongRequest(currentChannelId,currentSongId,pt,true);
    ui->rateSongButton->setEnabled(false);
}

void doubanFM::onSkipButtonClicked()
{
    qDebug() << Q_FUNC_INFO;
    if(_mediaObject->state() == Phonon::StoppedState)
    {
        emit sendSongListRequest(currentChannelId,currentSongId,0.0);;
        return;
    }
    else if(_mediaObject->state() == Phonon::PausedState)
    {
        _mediaObject->play();
        pauseState = false;
    }
    double pt = _mediaObject->currentTime() / 1000.0;
    emit sendSkipSongRequest(currentChannelId,currentSongId,pt);
}

void doubanFM::onTrashButtonClicked()
{
    qDebug() << Q_FUNC_INFO;
    double pt = _mediaObject->currentTime() / 1000.0;
    emit sendByeSongRequest(currentChannelId,currentSongId,pt);
}

void doubanFM::sourceChanged(const Phonon::MediaSource source)
{
    qDebug() << Q_FUNC_INFO;
    int index = _mediaSource.indexOf(source);
    if(index < 0)
        return;
    currentSongIndex = index;
    qDebug() << "currentSongIndex = " << currentSongIndex;
    currentSongId = _songList[currentSongIndex].sid;
    ui->artistLabel->setText(_songList[currentSongIndex].artist);
    ui->albumLabel->setText(QString("&lt;")
                            + _songList[currentSongIndex].albumtitle
                            + QString("&gt;")
                            + _songList[currentSongIndex].public_time);
    ui->songLabel->setText(_songList[currentSongIndex].title);
    if(_songList[currentSongIndex].like)
    {
        ui->rateSongButton->setIcon(QIcon(":/icons/heart.png"));
        ui->rateSongButton->setToolTip(tr("Unrate the song"));
    }
    else
    {
        ui->rateSongButton->setIcon(QIcon(":/icons/heart_empty.png"));;
        ui->rateSongButton->setToolTip(tr("Rate the song"));
    }

    emit albumImageRequest(_songList[currentSongIndex].picture);
}

//other widget signals and mainwidget slots
void doubanFM::loginSucceed(DoubanUser user)
{
    qDebug() << Q_FUNC_INFO;
    ui->userLabel->setText(user.name);
    QPropertyAnimation *anim = new QPropertyAnimation(_loginwidget,"geometry");
    anim->setStartValue(_loginwidget->geometry());
    anim->setEndValue(QRect(0,-_loginwidget->height(),_loginwidget->width(),_loginwidget->height()));
    anim->setDuration(300);
    anim->setEasingCurve(QEasingCurve::Linear);
    anim->start();
    _channelListWidget->unLockStarChannel();
    if(currentChannelId == -3)
        emit sendSongListRequest(currentChannelId,currentSongId,0.0);

}

void doubanFM::receiveSongList(const QList<DoubanFMSong> songList)
{
    qDebug() << Q_FUNC_INFO;
    _songList = songList;
    qDebug() << songList.size();
    _mediaSource.clear();
    foreach (DoubanFMSong song, _songList) {
        _mediaSource.append(song.url);
    }
    _mediaObject->clear();
    _mediaObject->setQueue(_mediaSource);
    _mediaObject->play();
}

void doubanFM::receiveRateSong(const bool succeed)
{
    qDebug() << Q_FUNC_INFO;
    ui->rateSongButton->setEnabled(true);
    qDebug() << succeed;
    if(succeed)
    {
        if(_songList[currentSongIndex].like)
        {
            _songList[currentSongIndex].like = false;
            ui->rateSongButton->setIcon(QIcon(":/icons/heart_empty.png"));
        }
        else
        {
            _songList[currentSongIndex].like = true;
            ui->rateSongButton->setIcon(QIcon(":/icons/heart.png"));
        }
    }
}

void doubanFM::receiveSkipSong(const bool succeed)
{
    qDebug() << Q_FUNC_INFO;
    qDebug() << succeed;
    qDebug() << "mediaSource size = " << _mediaSource.size();
    if(succeed)
    {
        if(currentSongIndex < _mediaSource.size() - 1)
        {
            _mediaObject->setCurrentSource(_mediaSource.at(currentSongIndex + 1));
            _mediaObject->play();
        }
        else
        {
            double pt = _mediaObject->currentTime() / 1000.0;
            emit sendSongListRequest(currentChannelId,currentSongId,pt);
        }
    }

}

void doubanFM::receiveByeSong(const bool succeed)
{
    qDebug() << Q_FUNC_INFO;
    if(succeed)
    {
        if(currentSongIndex < _mediaSource.size() -  1)
        {
            _mediaObject->setCurrentSource(_mediaSource.at(currentSongIndex + 1));
            _mediaObject->play();
        }
        else
        {
            double pt = _mediaObject->currentTime() / 1000.0;
            emit sendSongListRequest(currentChannelId,currentSongId,pt);
        }
    }
}

void doubanFM::receiveEndSong(const bool succeed)
{
    qDebug() << Q_FUNC_INFO;
}

void doubanFM::receiveChannelList(QList<DoubanChannel> channelList,bool isLogin)
{
    qDebug() << Q_FUNC_INFO;
    _channelList = channelList;
    if(isLogin || (!isLogin && currentChannelId != -3))
        emit sendSongListRequest(currentChannelId,currentSongId,0.0);
    else
        onLoginButtonClicked();
}

void doubanFM::logoutSucceed(bool succeed)
{
    if(succeed)
    {
        ui->userLabel->setText(tr("Logout"));
        ui->userLoginButton->setToolTip(tr("CLikc to login"));
        _channelListWidget->lockStarChannel();
    }
}

void doubanFM::pauseMaskWidgetPressed()
{
    qDebug() << Q_FUNC_INFO;
    pauseState = false;
    _pauseMaskWidget->setVisible(false);
    _mediaObject->play();
}

void doubanFM::receiveChannelChange(qint32 channelId)
{
    qDebug() << Q_FUNC_INFO;
    currentChannelId = channelId;
    double pt = _mediaObject->currentTime() / 1000.0;
    emit sendSongListRequest(currentChannelId,currentSongId,pt);
}

void doubanFM::stateChanged(Phonon::State newState, Phonon::State oldState)
{
    qDebug() << Q_FUNC_INFO;
    switch(newState)
    {
    case Phonon::ErrorState:
        qDebug() << newState;
        break;
    case Phonon::PlayingState:
        qDebug() << newState;
        break;
    case Phonon::PausedState:
        qDebug() << newState;
        if(oldState == Phonon::PlayingState && !pauseState)
            emit sendSongListRequest(currentChannelId,currentSongId,1.0);;
        break;
    case Phonon::LoadingState:
        qDebug() << newState;
        break;
    case Phonon::StoppedState:
        qDebug() << newState;
        emit sendSongListRequest(currentChannelId,currentSongId,1.0);
        break;
    case Phonon::BufferingState:
        qDebug() << newState;
        break;
    }
}

void doubanFM::writeSettings()
{
    setting.setValue("currentSongId",currentSongId);
    setting.setValue("currentChannelId",currentChannelId);
    QVariantMap user;
    DoubanUser curUser = _mydouban->getDoubanUser();
    user.insert("name",curUser.name);
    user.insert("ck",curUser.ck);
    user.insert("uid",curUser.uid);
    user.insert("url",curUser.url);
    setting.setValue("user",user);
    QByteArray data;
    foreach (QNetworkCookie cookie, curUser.cookies) {
        data.append(cookie.toRawForm()).append("\n");
    }

    setting.setValue("cookies",data);
}

void doubanFM::readSettings()
{

    currentSongId = setting.value("currentSongId",0).toInt();
    currentChannelId = setting.value("currentChannelId").toInt();
    QVariantMap user;
    user = setting.value("user",0).toMap();
    QByteArray data = setting.value("cookies",0).toByteArray();
    DoubanUser savedUser;
    savedUser.ck = user["ck"].toString();
    savedUser.name = user["name"].toString();
    savedUser.uid = user["uid"].toString();
    savedUser.url = user["url"].toString();
    savedUser.cookies = QNetworkCookie::parseCookies(data);
    qDebug() << savedUser.cookies << savedUser.ck << savedUser.name;
    _mydouban->setDoubanUser(savedUser);
    _channelListWidget->setchannelId(currentChannelId);
}
