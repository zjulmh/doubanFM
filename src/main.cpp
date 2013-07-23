#include "include/doubanfm.h"
#include <QTranslator>
#include <QResource>
#include <QDebug>
#include <QTextCodec>
#include <QApplication>
#include <QString>
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationName("DoubanFM");
    a.setWindowIcon(QIcon(":/icons/icon.png"));

    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));

    doubanFM m;
    m.setWindowTitle(QString("Douban FM"));
    m.show();

    return a.exec();
}
