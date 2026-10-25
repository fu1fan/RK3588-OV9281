#include "generator.h"
#include"widgetfactory.h"
#include "audiorecorder.h"
#include "musicplayer.h"
#include "ubootmenuwidget.h"
#include "qtcamera.h"
#include <DWKeyboard/KeyboardGlobal.h>
#include "qplayer.h"
#include <QGuiApplication>
#include <QLineEdit>
#include <QTextEdit>
#include <QPlainTextEdit>
#include "browser.h"
#include "browserwindow.h"
#include "tabwidget.h"
#include  <QWebEngineSettings>

static Browser* g_browser = nullptr;
Generator::Generator(QObject *parent):QObject((parent))
{
    GlobalInit();
    qApp->installEventFilter(this);
}

Generator::~Generator()
{
    if(g_browser)
        delete g_browser;
}

void Generator::initAppBefore(int argc, char *argv[])
{
    if(argc >1 && QString(argv[1]).trimmed()=="simplebrowser"){
        QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
        QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    }
}

void Generator::create(const QString &name)
{
    WidgetFactory::instance()->addHelp(WidgetFactory::widget_extra+1, "audiorecorder");
    WidgetFactory::instance()->addHelp(WidgetFactory::widget_extra+2, "uboot");
    WidgetFactory::instance()->addHelp(WidgetFactory::widget_extra+3, "music");
    WidgetFactory::instance()->addHelp(WidgetFactory::widget_extra+4, "camera");
    WidgetFactory::instance()->addHelp(WidgetFactory::widget_extra+5, "video");
    WidgetFactory::instance()->addHelp(WidgetFactory::widget_extra+6, "simplebrowser");

    if(name == "audiorecorder")
        WidgetFactory::instance()->addWidget(new AudioRecorder, WidgetFactory::widget_extra+1, "audiorecorder");

    if(name == "uboot")
        WidgetFactory::instance()->addWidget(new UbootMenuWidget, WidgetFactory::widget_extra+2, "uboot");

    if(name == "music")
        WidgetFactory::instance()->addWidget(new MusicPlayer, WidgetFactory::widget_extra+3, "music");

    if(name == "camera")
        WidgetFactory::instance()->addWidget(new qtCamera, WidgetFactory::widget_extra+4, "camera");

    if(name == "video"){
        WidgetFactory::instance()->addWidget(new QPlayer, WidgetFactory::widget_extra+5, "video");
    }

    if(name == "simplebrowser") {
        QWebEngineSettings::defaultSettings()->setAttribute(QWebEngineSettings::PluginsEnabled, true);
        g_browser = new Browser;
        BrowserWindow *window = g_browser->createWindow();
        WidgetFactory::instance()->addWidget(window, WidgetFactory::widget_extra+6, "simplebrowser");
        WidgetFactory::instance()->control()->setWindowTitleVisible(false, 0);
        window->tabWidget()->setUrl(QUrl("https://www.elfboard.com/"));
    }
}

bool Generator::eventFilter(QObject *watched, QEvent *event)
{
    static bool mFocusIn = false;
    QLineEdit *lineEdt = dynamic_cast<QLineEdit*>(watched);
    bool bEdtInput = (lineEdt);
    if(bEdtInput&& !m_objects.contains(watched)){
            watched->installEventFilter(this);
    }

    if(bEdtInput && !lineEdt->topLevelWidget()->isModal()){
        if (event->type()==QEvent::FocusIn){
            mFocusIn = true;
        }
        else if (event->type()==QEvent::FocusOut){
            PlatformInputContextBase->FocusOut(watched);
            mFocusIn = false;
        }

        if (mFocusIn && event->type() == QEvent::MouseButtonPress) {
            QMouseEvent *e = (QMouseEvent *)event;
            PlatformInputContextBase->FocusIn(watched, e->globalPos());
        }

    }

    return QObject::eventFilter(watched,event);
}
