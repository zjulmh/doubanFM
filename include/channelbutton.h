#ifndef CHANNELBUTTON_H
#define CHANNELBUTTON_H

#include <QToolButton>

class channelButton : public QToolButton
{
    Q_OBJECT
public:
    explicit channelButton(QWidget *parent = 0,qint32 _channelId = 0,int _index = 0,QString objName = "");
    void restoreStyle();
signals:
    void channelChange(qint32 channelId,int index);
public slots:
    void onButtonClicked();
private:
    qint32 _channelId;
    int _index;

};

#endif // CHANNELBUTTON_H
