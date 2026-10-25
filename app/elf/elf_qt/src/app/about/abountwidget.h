#ifndef ABOUNTWIDGET_H
#define ABOUNTWIDGET_H

#include <QWidget>
#include <QThread>
#include "widgetcontrol.h"
namespace Ui {
class AboutWidget;
}

class FetcherSystem;
class AbountWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AbountWidget(QWidget *parent = nullptr);
    ~AbountWidget();
private:
    Ui::AboutWidget *ui;
    FetcherSystem *m_fetcher;
};


class FetcherSystem : public QThread
{
    Q_OBJECT
public:
    FetcherSystem(QObject *parent=Q_NULLPTR);
    void setStart(bool bStart);

signals:
    void infoUpdate(const QString &storage,
                    const QString &memTotal,
                    const QString &memFree,
                    const QString &memAvailable,
                    const QString &kerVersion,
                    const QString &userCpu);


private:
    virtual void run();
    bool m_bQuit = false;
};
#endif // ABOUNTWIDGET_H
