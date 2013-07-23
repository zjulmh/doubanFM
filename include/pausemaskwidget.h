#ifndef PAUSEMASKWIDGET_H
#define PAUSEMASKWIDGET_H

#include <QWidget>
#include <QLabel>
class pauseMaskWidget : public QWidget
{
    Q_OBJECT
public:
    explicit pauseMaskWidget(QWidget *parent = 0);
    ~pauseMaskWidget();
signals:
    void mousePressed();
    
public slots:
private:
    QLabel *la;
    void mousePressEvent(QMouseEvent *event);
    
};

#endif // PAUSEMASKWIDGET_H
