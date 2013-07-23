#ifndef DOUBANFM_H
#define DOUBANFM_H

#include <QWidget>
#include <phonon/MediaObject>
#include <phonon/AudioOutput>
#include <QNetworkAccessManager>
#include <QSettings>
#include "douban.h"
#include "doubanlogin.h"
#include "douban_types.h"
#include "pausemaskwidget.h"
#include "channellist.h"
#include "qxtglobalshortcut.h"
namespace Ui {
class doubanFM;
}

class doubanFM : public QWidget
{
    Q_OBJECT
    
public:
    explicit doubanFM(QWidget *parent = 0);
    ~doubanFM();

signals:
    void sendlogoutRequest();
    void sendSongListRequest(const qint32 cid,const quint32 sid,const double pt);
    void sendSkipSongRequest(const qint32 cid,const quint32 sid,const double pt);
    void sendRateSongRequest(const qint32 cid,const quint32 sid,const double pt,const bool isRate);
    void sendEndSongRequest(const qint32 cid,const quint32 sid,const double pt);
    void sendByeSongRequest(const qint32 cid,const quint32 sid,const double pt);
    void albumImageRequest(const QString picUrl);
private slots:
    void onLoginButtonClicked();
    void onRateButtonClicked();
    void onSkipButtonClicked();
    void onPauseButtonClicked();
    void onTrashButtonClicked();
    void onAlbumImage(const QString picUrl);

    void stateChanged(Phonon::State newState,Phonon::State oldState);
    void playTick(qint64 time);
    void sourceChanged(const Phonon::MediaSource source);
    void onVolumneChange(int value);

    void receiveChannelChange(qint32 channelId);
    void receiveSongList(const QList<DoubanFMSong> songList);
    void receiveRateSong(const bool succeed);
    void receiveSkipSong(const bool succeed);
    void receiveByeSong(const bool succeed);
    void receiveEndSong(const bool succeed);
    void receiveChannelList(QList<DoubanChannel> channelList,bool isLogin);
    void loginSucceed(DoubanUser user);
    void logoutSucceed(bool succeed);
    void receiveAlbumImage(QNetworkReply *reply);
    void pauseMaskWidgetPressed();
private:
    Ui::doubanFM *ui;

    doubanLogin *_loginwidget;
    pauseMaskWidget *_pauseMaskWidget;
    channelList *_channelListWidget;
    Douban *_mydouban;

    QList<DoubanFMSong> _songList;
    QList<DoubanChannel> _channelList;
    QList<Phonon::MediaSource> _mediaSource;

    qint32 currentChannelId;
    quint32 currentSongId;
    quint32 currentSongIndex;

    Phonon::MediaObject *_mediaObject;
    Phonon::AudioOutput *_audioOutput;

    QNetworkAccessManager *albumImageManager;
    bool pauseState;

    QxtGlobalShortcut *nextShortCut;
    QxtGlobalShortcut *pauseShortCut;
    QxtGlobalShortcut *rateShortCut;
    QxtGlobalShortcut *trashShortCut;

    QSettings setting;

    void writeSettings();
    void readSettings();
};

#endif // DOUBANFM_H
