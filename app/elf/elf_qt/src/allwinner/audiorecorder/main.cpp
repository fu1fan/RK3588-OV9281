#include <QApplication>
#include <QTranslator>
#include "messagebox.h"
#include <QDebug>
#include "resourceloader.h"
#include "basepath.h"
#include <QStyleFactory>
#include "basefactory.h"
#include "appwindow.h"
#include "widget.h"

using namespace fl_common;

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);
    QApplication::setStyle(QStyleFactory::create("Fusion"));
    ResourceLoader::instacne()->load(BasePath::instance()->resourcePath());
    AppWindow::initGloabl();

    AppWindow m(new Widget, "audiorecorder");
    m.show();
    int ret = a.exec();

    return ret;
}
