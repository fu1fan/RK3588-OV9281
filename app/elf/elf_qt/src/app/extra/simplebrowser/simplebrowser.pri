QT += webenginewidgets

FORMS += \
    $$PWD/certificateerrordialog.ui \
    $$PWD/downloadmanagerwidget.ui \
    $$PWD/downloadwidget.ui \
    $$PWD/passworddialog.ui

HEADERS += \
    $$PWD/browser.h \
    $$PWD/browserwindow.h \
    $$PWD/downloadmanagerwidget.h \
    $$PWD/downloadwidget.h \
    $$PWD/tabwidget.h \
    $$PWD/webpage.h \
    $$PWD/webpopupwindow.h \
    $$PWD/webview.h

SOURCES += \
    $$PWD/browser.cpp \
    $$PWD/browserwindow.cpp \
    $$PWD/downloadmanagerwidget.cpp \
    $$PWD/downloadwidget.cpp \
    $$PWD/tabwidget.cpp \
    $$PWD/webpage.cpp \
    $$PWD/webpopupwindow.cpp \
    $$PWD/webview.cpp

INCLUDEPATH +=$$PWD
