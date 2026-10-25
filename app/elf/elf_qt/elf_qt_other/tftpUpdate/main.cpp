#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    qputenv("QT_IM_MODULE","im");
    QApplication a(argc, argv);
    MainWindow w;
	w.showMaximized();
    return a.exec();
}
