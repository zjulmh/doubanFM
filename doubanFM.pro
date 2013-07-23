QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = doubanFM
TEMPLATE = app

RESOURCES += \
    icons.qrc

FORMS += \
    ui/channellist.ui \
    ui/doubanlogin.ui \
    ui/doubanfm.ui

HEADERS += \
    include/douban_types.h \
    include/douban.h \
    include/doubanlogin.h \
    include/pausemaskwidget.h \
    include/channellist.h \
    include/channelbutton.h \
    include/doubanfm.h \
    include/qxtglobalshortcut_p.h \
    include/qxtglobalshortcut.h \
    include/qxtglobal.h

SOURCES += \
    src/douban.cpp \
    src/doubanlogin.cpp \
    src/pausemaskwidget.cpp \
    src/channellist.cpp \
    src/channelbutton.cpp \
    src/doubanfm.cpp \
    src/main.cpp \
    src/qxtglobalshortcut_x11.cpp \
    src/qxtglobalshortcut.cpp

LIBS += -lqjson -lphonon -lX11

