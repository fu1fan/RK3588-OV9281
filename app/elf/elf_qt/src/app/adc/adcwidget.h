#ifndef ADCWIDGET_H
#define ADCWIDGET_H

#include <QWidget>
#include "widgetcontrol.h"
#include <QList>

class ADCItemWidgt;
class AdcWidget : public QWidget WIDGETCONTROL
{
    Q_OBJECT
public:
    explicit AdcWidget(QWidget *parent = nullptr);

   ~ AdcWidget();

    void resetTr();

    void paintEvent(QPaintEvent *e);

private:
    QList<ADCItemWidgt*> m_items;
};

#endif // ADCWIDGET_H
