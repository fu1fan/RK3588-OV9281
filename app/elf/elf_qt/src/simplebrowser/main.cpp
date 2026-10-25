#include <QApplication>
#include <QTranslator>
#include "messagebox.h"
#include <QDebug>
#include "resourceloader.h"
#include "basepath.h"
#include <QStyleFactory>
#include "basefactory.h"
#include "appwindow.h"
#include "browser.h"
#include "browserwindow.h"
#include <QWebEngineSettings>
#include "tabwidget.h"
using namespace fl_common;

int main(int argc, char* argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

    setenv("QT_WEBENGINE_DISABLE_SANDBOX", "1", 1);
    QApplication a(argc, argv);
    QApplication::setStyle(QStyleFactory::create("Fusion"));
    ResourceLoader::instacne()->load(BasePath::instance()->resourcePath());
    AppWindow::initGloabl();

    QWebEngineSettings::defaultSettings()->setAttribute(QWebEngineSettings::PluginsEnabled, true);
    Browser b;
    BrowserWindow *bw = b.createWindow();
    bw->tabWidget()->setUrl(QUrl("https://www.elfboard.com/"));
    AppWindow m(bw, "Browser");
    m.show();
    int ret = a.exec();

    return ret;
}
