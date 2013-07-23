#include "include/pausemaskwidget.h"
#include <QDebug>
#include <QLabel>
pauseMaskWidget::pauseMaskWidget(QWidget *parent) :
    QWidget(parent)
{
    qDebug() << Q_FUNC_INFO;
    this->setMaximumSize(250,255);
    this->setMinimumSize(250,255);
    this->setAutoFillBackground(true);
    QPalette myPalette = this->palette();
    myPalette.setColor(QPalette::Window,QColor(0,0,0,150));
    this->setPalette(myPalette);


    la = new QLabel(this);
    la->setObjectName("continue");
    la->setMaximumSize(160,35);
    la->setMinimumSize(160,35);
    la->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    la->move(45,110);
    la->setText("Continue Playing");
    la->setStyleSheet("QLabel#continue{background-color:rgba(255,255,255,0);}QLabel#continue::hover{color:white;background-color:rgba(255,255,255,0);}");
}
pauseMaskWidget::~pauseMaskWidget()
{
    delete la;
}

void pauseMaskWidget::mousePressEvent(QMouseEvent *event)
{
    emit mousePressed();
}
