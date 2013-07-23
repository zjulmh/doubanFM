#include "include/channellist.h"
#include "ui_channellist.h"
#include <QDebug>
#include <QVBoxLayout>
#include <QWheelEvent>
#include <QPropertyAnimation>
channelList::channelList(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::channelList),
    isFold(true),
    currentChannelIndex(2),
    currentChannelId(-3)
{
    qDebug() << Q_FUNC_INFO;
    ui->setupUi(this);
    this->setAutoFillBackground(true);
    QPalette myPalette = this->palette();
    myPalette.setColor(QPalette::Window,QColor(0,0,0,100));
    this->setPalette(myPalette);
    QVBoxLayout *vlayout = new QVBoxLayout(ui->channelContainer);
    vlayout->setMargin(0);
    vlayout->setSpacing(10);
    vlayout->setSizeConstraint(QLayout::SetFixedSize);
    ui->channelContainer->setLayout(vlayout);
//    connect(ui->foldButton,SIGNAL(clicked()),this,SLOT(onReceiveFoldButtonClicked()));
}

channelList::~channelList()
{
    for(int i = 0;i < _channelListButton.size();i++)
    {
        delete _channelListButton[i];
    }
    delete ui;
}

void channelList::setchannelId(qint32 channelId)
{
    currentChannelId = channelId;   
}

int channelList::getInvisibleWidth()
{
    qDebug() << ui->channelContainer->width();
    return ui->channelContainer->width();
}

void channelList::onReceiveChannelList(const QList<DoubanChannel> channelList,bool isLogin)
{
    qDebug() << Q_FUNC_INFO;
    _channelList = channelList;
    _channelListButton.clear();
    DoubanChannel starchannel;
    starchannel.abbr_en = "star";
    starchannel.channel_id = -3;
    starchannel.name = "star radio";
    starchannel.name_en = "star radio";
    starchannel.seq_id = -3;
    _channelList.push_front(starchannel);

    qDebug() << _channelList.size();
    for(int i = 0;i < _channelList.size();++i)
    {
        qDebug() << _channelList[i].name_en;
        QString objName = QString("channel_") + QString::number(i);
        qDebug() << objName;
        channelButton *cb = new channelButton(ui->channelContainer,_channelList[i].channel_id,i,objName);

        cb->setText(_channelList[i].name_en);
        _channelListButton.push_back(cb);
        ui->channelContainer->layout()->addWidget(cb);
        connect(cb,SIGNAL(channelChange(qint32,int)),this,SLOT(onReceiveButtonClicked(qint32,int)));
        if(isLogin)
        {
            if(currentChannelId == _channelList[i].channel_id)
            {
                currentChannelIndex = i;
                _channelListButton[i]->setStyleSheet(QString("channelButton{border:none;background-color:rgb(132, 208, 174);color:red}")
                                                                   + QString("channelButton:hover{border:none;background-color:rgba(132, 208, 174,150);color:white;}"));
                _channelListButton[i]->onButtonClicked();
            }
        }
        else if(!isLogin && currentChannelId != -3)
        {
            if(currentChannelId == _channelList[i].channel_id)
            {
                currentChannelIndex = i;

                _channelListButton[i]->setStyleSheet(QString("channelButton{border:none;background-color:rgb(132, 208, 174);color:red}")
                                                                   + QString("channelButton:hover{border:none;background-color:rgba(132, 208, 174,150);color:white;}"));
                _channelListButton[i]->onButtonClicked();
            }
            _channelListButton[0]->setEnabled(false);
            _channelListButton[0]->setToolTip(tr("Need Login"));
        }
        else
        {
            currentChannelIndex = 0;
            currentChannelId = -3;
            _channelListButton[0]->setEnabled(false);
            _channelListButton[0]->setToolTip(tr("Need Login"));

        }
    }
    qDebug() << _channelListButton.size();
    if(currentChannelIndex * 53 >= 0)
        ui->channelContainer->move(0,0);
    else
        ui->channelContainer->move(0,106 - currentChannelIndex * 53);

}

void channelList::onReceiveButtonClicked(qint32 channelId,int index)
{

    if(channelId != currentChannelId)
    {
        qDebug() << Q_FUNC_INFO << channelId << currentChannelId;
        quint32 temp = currentChannelIndex;
        _channelListButton[temp]->restoreStyle();
        currentChannelIndex = index;
        currentChannelId = channelId;
        emit channelChanged(_channelList[index].channel_id);
    }

}

void channelList::wheelEvent(QWheelEvent *event)
{
    qDebug() << Q_FUNC_INFO;
    if(event->x() < 235)
    {
        int numSteps = event->delta() / 8 / 15;
        int wholeheight = 53 * (_channelList.size() - 1) + 43;

        QPropertyAnimation *anim = new QPropertyAnimation(ui->channelContainer,"geometry");
        anim->setStartValue(ui->channelContainer->geometry());
        QRect endValue;
        if(ui->channelContainer->geometry().y() + numSteps * 2 * 53 > 0)
        {
            endValue = QRect(ui->channelContainer->geometry().x(),
                             0,
                             ui->channelContainer->geometry().width(),
                             ui->channelContainer->geometry().height());
        }
        else if(ui->channelContainer->geometry().y() + numSteps * 53 * 2 < 255 - wholeheight)
        {
            endValue = QRect(ui->channelContainer->geometry().x(),
                             255 - wholeheight,
                             ui->channelContainer->geometry().width(),
                             ui->channelContainer->geometry().height());
        }
        else
            endValue = QRect(ui->channelContainer->geometry().x(),
                             ui->channelContainer->geometry().y() + numSteps * 53 * 2,
                             ui->channelContainer->geometry().width(),
                             ui->channelContainer->geometry().height());
        anim->setEndValue(endValue);
        anim->setDuration(300);
        anim->setEasingCurve(QEasingCurve::Linear);
        anim->start();
    }
}


void channelList::enterEvent(QEvent *event)
{
    qDebug() << Q_FUNC_INFO;
    QPropertyAnimation *anim = new QPropertyAnimation(this,"geometry");
    isFold = false;
    anim->setStartValue(this->geometry());
    QRect endVal(0,this->geometry().y(),this->geometry().width(),this->geometry().height());
    anim->setEndValue(endVal);
    anim->setEasingCurve(QEasingCurve::Linear);
    anim->setDuration(300);
    anim->start();
    connect(anim,SIGNAL(finished()),this,SLOT(onAnimFinished()));
}
void channelList::leaveEvent(QEvent *event)
{
    qDebug() << Q_FUNC_INFO;
    QPropertyAnimation *anim = new QPropertyAnimation(this,"geometry");
    isFold = true;
    anim->setStartValue(this->geometry());
    QRect endVal(QRect(-getInvisibleWidth(),this->geometry().y(),this->geometry().width(),this->geometry().height()));
    anim->setEndValue(endVal);
    anim->setEasingCurve(QEasingCurve::Linear);
    anim->setDuration(300);
    anim->start();
    connect(anim,SIGNAL(finished()),this,SLOT(onAnimFinished()));
}
void channelList::unLockStarChannel()
{
    if(_channelListButton.size() != 0)
    {
        _channelListButton[0]->setEnabled(true);
        _channelListButton[0]->setToolTip(tr(""));
        if(currentChannelId == -3)
            _channelListButton[0]->onButtonClicked();
    }
}

void channelList::lockStarChannel()
{
    if(_channelListButton.size() != 0)
    {
        _channelListButton[0]->setEnabled(false);
        _channelListButton[0]->setToolTip(tr("Need Login"));
    }
}

void channelList::onAnimFinished()
{
    qDebug() << this->geometry();
    if(isFold)
    {
//        this->setStyleSheet("QWidget#channelList{background-color: rgba(255, 255, 255,100)}QWidget#foldButtonContainer{background-color:rgba(255,255,255,100);}");
        ui->foldButton->setIcon(QIcon(":/icons/channel_right.png"));
    }
    else
    {
//        this->setStyleSheet("QWidget#channelList{background-color: rgba(255, 255, 255,100)}QWidget#foldButtonContainer{background-color:white;}");
        ui->foldButton->setIcon(QIcon(":/icons/channel_left.png"));
    }
}
