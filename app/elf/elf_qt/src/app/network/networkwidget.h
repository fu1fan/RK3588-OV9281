#ifndef NETWORKWIDGET_H
#define NETWORKWIDGET_H

#include <QWidget>
#include "widgetcontrol.h"

namespace Ui {
class NetworkWidget;
}

class  NetworkWidget : public QWidget WIDGETCONTROLPROGRESS
{
    Q_OBJECT
public:
    explicit NetworkWidget(QWidget *parent = nullptr);
    virtual ~NetworkWidget();
    void processMessage(const QString &info) ;
    void resetTr();

private slots:
    void on_setBtn_clicked();
    void on_pingBtn_clicked();
    void readNetwork();

private:
    Ui::NetworkWidget *ui;
};

#endif // NETWORKWIDGET_H
