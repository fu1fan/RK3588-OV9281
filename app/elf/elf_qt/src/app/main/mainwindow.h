#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "widgetcontrol.h"
#include <QStackedWidget>

class MainWindowPrivate;
class MainWindow : public QWidget MAINCONTROL
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void setWindowTitleVisible(bool bVisible, int border = 10);

    void showWidget(int id);

    int currentWidget();

    void resetTr();

    bool eventFilter(QObject *object, QEvent *e);
private:
    MainWindowPrivate *m_d;

};

#endif // MAINWINDOW_H
