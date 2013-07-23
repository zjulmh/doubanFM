#include "include/doubanlogin.h"
#include "ui_doubanlogin.h"
#include <QDebug>
#include <QPropertyAnimation>
#include <QPalette>
doubanLogin::doubanLogin(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::doubanLogin)
{
    qDebug() << Q_FUNC_INFO;
    ui->setupUi(this);
    this->setAutoFillBackground(true);
    QPalette myPalette = this->palette();
    myPalette.setColor(QPalette::Window,QColor(255,255,255,180));
    this->setPalette(myPalette);

    captchaLabel = new QLabel(ui->captchaContainer);
    captchaLabel->resize(ui->captchaContainer->width(),ui->captchaContainer->height());
    connect(ui->cancleButton,SIGNAL(clicked()),this,SLOT(onCancleButtonClicked()));
    connect(ui->loginButton,SIGNAL(clicked()),this,SLOT(onLoginButtonClicked()));
}

doubanLogin::~doubanLogin()
{
    delete captchaLabel;
    delete ui;

}

void doubanLogin::onLoginButtonClicked()
{
    qDebug() << Q_FUNC_INFO;
    emit loginRequest(ui->aliasInput->text().trimmed(),ui->passwdInput->text().trimmed(),ui->verificaitonInput->text().trimmed());
//    ui->loginButton->setEnabled(false);
//    ui->cancleButton->setEnabled(false);
//    ui->aliasInput->setEnabled(false);
//    ui->passwdInput->setEnabled(false);
//    ui->verificaitonInput->setEnabled(false);
}

void doubanLogin::onCancleButtonClicked()
{
    qDebug() << this->geometry();
    QPropertyAnimation *anim = new QPropertyAnimation(this,"geometry");
    anim->setDuration(300);

    anim->setStartValue(this->geometry());
    anim->setEndValue(QRect(this->geometry().x(),-this->geometry().height(),this->geometry().width(),this->geometry().height()));
    anim->setEasingCurve(QEasingCurve::Linear);
    anim->start();
}

void doubanLogin::onLoginFailed(const QString errMsg)
{
    qDebug() << Q_FUNC_INFO;
    ui->errMsg->setText(errMsg);
//    ui->loginButton->setEnabled(true);
//    ui->cancleButton->setEnabled(true);
//    ui->aliasInput->setEnabled(true);
//    ui->passwdInput->setEnabled(true);
//    ui->verificaitonInput->setEnabled(true);

}


void doubanLogin::onCaptchaImageLoad(QPixmap pixmap)
{
    qDebug() << Q_FUNC_INFO;
    int w = captchaLabel->width();
    int h = captchaLabel->height();
    captchaLabel->setPixmap(pixmap.scaled(w,h,Qt::IgnoreAspectRatio));

}
