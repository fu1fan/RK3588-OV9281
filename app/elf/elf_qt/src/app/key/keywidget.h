#ifndef KEYWIDGET_H
#define KEYWIDGET_H

#include <QWidget>
#include "widgetcontrol.h"

class KeyLbl;
class QLabel;
class KeyWidget : public QWidget WIDGETCONTROL
{
    Q_OBJECT
public:
    explicit KeyWidget(QWidget *parent = nullptr);
    void resetTr();

private:
    QLabel *m_tip;
};

#endif // KEYWIDGET_H
