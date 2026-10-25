#ifndef RECORDERWIDGET_H
#define RECORDERWIDGET_H
#include "iwidget.h"


class RecorderWidget : public IWidget
{
    Q_OBJECT
   Q_PLUGIN_METADATA(IID PLUGIN_INTERFACE_iid FILE "audiorecoreder.json")
public:
    RecorderWidget(QWidget *parent=NULL);
    QString id();
};

#endif // RECORDERWIDGET_H
