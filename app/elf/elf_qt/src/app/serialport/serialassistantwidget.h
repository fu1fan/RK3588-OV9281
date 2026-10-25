#ifndef SERIALASSISTANTWIDGET_H
#define SERIALASSISTANTWIDGET_H
#include <QWidget>
#include "widgetcontrol.h"
namespace Ui {
class SerialAssistantWidget;
}

namespace fl_common {
class SerialPort;
}
using namespace  fl_common;
class QTimer;
class SerialAssistantWidget :
        public QWidget WIDGETCONTROL
{
    Q_OBJECT
public:
    explicit SerialAssistantWidget(QWidget *parent = nullptr);
    ~SerialAssistantWidget();

    void setTabWidget(QWidget *tab);
    void resetTr();

private slots:
    void on_openBtn_clicked();

    void on_clearTXBtn_clicked();

    void on_sendBtn_clicked();

    void on_sendFileBtn_clicked();

private:
    Ui::SerialAssistantWidget *ui;
    QString m_cmd;
    class QTimer *m_timer;
    QString m_startID;
    SerialPort *m_serial;

};

class QStackedWidget;
class QTabBar;
class MultiSerialWidget: public QWidget WIDGETCONTROL
{
    Q_OBJECT
public:
    MultiSerialWidget(QWidget *parent = nullptr);
    void resetTr();

private:
    QStackedWidget *m_statckWidget;
    QTabBar *m_pTabBar ;
};

#endif // MAINWINDOW_H
