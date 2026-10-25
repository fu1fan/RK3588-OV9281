#include <QApplication>
#include <QTranslator>
#include "messagebox.h"
#include <QDebug>
#include "resourceloader.h"
#include "basepath.h"
#include <QStyleFactory>
#include "basefactory.h"
#include "appwindow.h"
#include "qplayer.h"
#include <QSplashScreen>
#include <QScreen>
#include <QPainter>
using namespace fl_common;

int main(int argc, char* argv[])
{
    if(!qEnvironmentVariableIsSet("QT_GSTREAMER_WIDGET_SINK")){
        qputenv("QT_GSTREAMER_USE_OPENGL_PLUGIN", "1");
    }
    QApplication a(argc, argv);
    QApplication::setStyle(QStyleFactory::create("Fusion"));
    ResourceLoader::instacne()->load(BasePath::instance()->resourcePath());
    AppWindow::initGloabl();
    AppWindow::loadSplashText("Video is Readying ...");

    AppWindow m(new QPlayer, "Player");
    m.show();
    int ret = a.exec();

    return ret;
}



