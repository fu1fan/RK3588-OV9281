#include "widgetcontrol.h"
#include "widgetfactory.h"
#include <QWidget>

void WidgetControl::resetTr()
{

}

MainControl *WidgetControl::control()
{
    return WidgetFactory::instance()->control();
}
