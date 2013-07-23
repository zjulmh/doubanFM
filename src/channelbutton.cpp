#include "include/channelbutton.h"
#include <QToolButton>
#include <QDebug>
channelButton::channelButton(QWidget *parent,qint32 channelId,int index,QString objName) :
    QToolButton(parent)
{
    qDebug() << Q_FUNC_INFO;
    this->_channelId = channelId;
    this->_index = index;
    this->setObjectName(objName);
    this->setMinimumSize(234,43);
    this->setMaximumSize(234,43);
    this->setStyleSheet(QString("channelButton#%1{border:none;background-color:rgb(132, 208, 174);color:black}").arg(objName)
                        + QString("channelButton#%1:hover{border:none;background-color:rgba(132, 208, 174);color:rgba(255,255,255,255);}").arg(objName));
//    this->setStyleSheet(QString("channelButton{border:none;background-color: rgb(157, 214, 197);}")
//                        + QString("channelButton:hover{color:white;border:none;background-color: rgba(157, 214, 197,150);}"));
    connect(this,SIGNAL(clicked()),this,SLOT(onButtonClicked()));
}


void channelButton::onButtonClicked()
{
    qDebug() << Q_FUNC_INFO;
    this->setStyleSheet(QString("channelButton#%1{border:none;background-color:rgb(157, 214, 197);color:red}").arg(this->objectName())
                        + QString("channelButton#%1:hover{border:none;background-color:rgba(157, 214, 197,255);color:rgba(255,0,0,100);}").arg(this->objectName()));
    emit channelChange(_channelId,_index);
}

void channelButton::restoreStyle()
{
    qDebug() << Q_FUNC_INFO;

    this->setStyleSheet(QString("channelButton#%1{border:none;background-color:rgb(132, 208, 174);color:black}").arg(this->objectName())
                        + QString("channelButton#%1:hover{border:none;background-color:rgba(132, 208, 174,255);color:rgba(255,255,255,255);}").arg(this->objectName()));

}

