#include "recorderwidget.h"
#include <QHBoxLayout>
#include "mainwindow.h"

RecorderWidget::RecorderWidget(QWidget *parent):IWidget(parent)
{
    QHBoxLayout *hLayout = new QHBoxLayout(this);
    hLayout->setMargin(0);
    MainWindow *recorder = new MainWindow;
    hLayout->addWidget(recorder);
}

QString RecorderWidget::id()
{
    return "OKAUDIORECORDER";
}
