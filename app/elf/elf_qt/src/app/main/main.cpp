#include <QApplication>

#include "widgetfactory.h"
#include <QTranslator>
#include "mainwindow.h"
#include "messagebox.h"
#include <QCommandLineOption>
#include <qcommandlineparser.h>
#include <QDebug>
#include "conf.h"
#include "resourceloader.h"
#include "basepath.h"
#include <QStyleFactory>
#include "basefactory.h"
#include "bookwindow.h"
using namespace fl_common;


#ifndef EXTRA
class Generator {public:Generator(){}static void  create(const QString &){} static void initAppBefore(int argc, char *argv[]){Q_UNUSED(argc)Q_UNUSED(argv)}};
#else
#include "generator.h"
#endif

int ParserArgument(int argc, char *argv[], QMap<QString, int> &maps);

int main(int argc, char* argv[])
{
    Generator::initAppBefore(argc, argv);
    QApplication a(argc, argv);

    QApplication::setStyle(QStyleFactory::create("Fusion"));

    //2加载资源文件.
    ResourceLoader::instacne()->load(BasePath::instance()->resourcePath());

    //3.先创建主窗口.h
    MainWindow mw;

    WidgetFactory::instance()->setControl(&mw);

    //4加载平台不通用窗口
    Generator g;

    //5解析命令行参数.
    int id = ParserArgument(argc, argv, WidgetFactory::instance()->available());
    if(!WidgetFactory::instance()->available().values().contains(id)){
        return 0;
    }

    //6创建显示的字窗口
    if(id >0){
        mw.showWidget(id);
    }

    mw.resize(800, 480);
    mw.show();
    mw.showMaximized();

    int ret = a.exec();

    return ret;
}


//解析命令行参数.
int ParserArgument(int argc, char *argv[],  QMap<QString, int> &maps)
{
    int id = -1;
    QString arg;

    if(argc >1){
        arg = QString(argv[1]).trimmed();
        Generator::create(arg);
    }

    if((arg.isEmpty() || arg == "-h") || !maps.contains(arg)){
        qDebug()<<"argument: "<<maps.keys();
        return id;
    }

    id = maps.value(arg);

    return id;
}
