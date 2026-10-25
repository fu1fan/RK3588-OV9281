#include <QApplication>
#include <QTranslator>
#include "messagebox.h"
#include <QDebug>
#include "resourceloader.h"
#include "basepath.h"
#include <QStyleFactory>
#include "basefactory.h"
#include "appwindow.h"
#include "networkwidget.h"
#include "eventhandler.h"
using namespace fl_common;

int main(int argc, char* argv[])
{
    AppWindow::initGloablBeforeApplication();
    QApplication a(argc, argv);
    QApplication::setStyle(QStyleFactory::create("Fusion"));
    ResourceLoader::instacne()->load(BasePath::instance()->resourcePath());
    AppWindow::initGloabl();
    AppWindow::setkeyboard(false);

    AppWindow w(new NetworkWidget, "Network");
    w.show();

    return a.exec();
}
