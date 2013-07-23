#ifndef DOUBANLOGIN_H
#define DOUBANLOGIN_H

#include <QWidget>
#include <QtNetwork/QNetworkCookie>
#include <QLabel>
#include "douban.h"
namespace Ui {
class doubanLogin;
}

class doubanLogin : public QWidget
{
    Q_OBJECT
    
public:
    explicit doubanLogin(QWidget *parent = 0);
    ~doubanLogin();
signals:
    void loginRequest(const QString alias,const QString password,const QString verification);
private slots:
    void onLoginButtonClicked();
    void onCancleButtonClicked();
public slots:

    void onLoginFailed(const QString errMsg);
    void onCaptchaImageLoad(QPixmap pixmap);
    
private:
    Ui::doubanLogin *ui;
    QLabel *captchaLabel;
};

#endif // DOUBANLOGIN_H
