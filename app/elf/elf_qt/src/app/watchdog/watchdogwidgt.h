#ifndef WATCHDOGWIDGT_H
#define WATCHDOGWIDGT_H

#include <QWidget>
#include "widgetcontrol.h"
namespace Ui {
class WatchDogWidgt;
}

class TimeCountdown;
class  WatchDogWidgt : public QWidget WIDGETCONTROL
{
    Q_OBJECT

public:
    explicit WatchDogWidgt(QWidget *parent = 0);
    ~WatchDogWidgt();

    void resetTr();

private slots:
    void on_openBtn_clicked();

    void on_checkBox_clicked();

    void on_setBtn_clicked();

    bool isInputError();
    void setInputEable(bool bEnable);

private:
    Ui::WatchDogWidgt *ui;
    TimeCountdown *m_timeControl;
};
#endif // WATCHDOGWIDGT_H
