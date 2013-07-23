#ifndef CHANNELLIST_H
#define CHANNELLIST_H

#include <QWidget>
#include "channelbutton.h"
#include "douban_types.h"
namespace Ui{
class channelList;
}
class channelList : public QWidget
{
    Q_OBJECT
public:
    explicit channelList(QWidget *parent = 0);
    ~channelList();
    int getInvisibleWidth();
    void setchannelId(qint32 channelId);
    void unLockStarChannel();
    void lockStarChannel();
signals:
    void channelChanged(qint32 channelId);
public slots:
    void onReceiveChannelList(const QList<DoubanChannel> channelList,bool isLogin);
    void onReceiveButtonClicked(qint32 channelId,int index);
//    void onReceiveFoldButtonClicked();
    void onAnimFinished();
private:
    Ui::channelList *ui;
    QList<channelButton *> _channelListButton;
    QList <DoubanChannel> _channelList;
    void wheelEvent(QWheelEvent *);
    void enterEvent(QEvent *event);
    void leaveEvent(QEvent *event);
    bool isFold;
    int currentChannelIndex;
    qint32 currentChannelId;
};

#endif // CHANNELLIST_H
