#ifndef SPIWIDGET_H
#define SPIWIDGET_H

#include <QWidget>
#include "widgetcontrol.h"

namespace Ui {
class SpiWidget;
}

class SpiWidget : public QWidget WIDGETCONTROL
{
    Q_OBJECT

public:
    explicit SpiWidget(QWidget *parent = nullptr);
    ~SpiWidget();

    virtual void resetTr();

public slots:
    void currentSpiChange(int id);

private:
    Ui::SpiWidget *ui;
};

#endif // SPIWIDGET_H
