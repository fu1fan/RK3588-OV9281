#ifndef WIFIWIDGET_H
#define WIFIWIDGET_H

#include <QWidget>

#include "wifiview.h"
namespace Ui {
class WIFIWidget;
}

class Hostapdcli;
class QLabel;
class Wpaclicmd;
class  WIFIWidget : public QWidget, public fl_common::ProcessHandler
{
    Q_OBJECT
public:
    struct WIFIMODE{
        QString cardName;  //wlan0
        QString driverName; //nl80211
        QString moduleName; //8821cs
        QString ko;//lib/modules/4.9.170/8821cs.koFile.
        QString ip;
    };
    explicit WIFIWidget(QWidget *parent = 0);
    ~WIFIWidget();

    virtual void processMessage(const QString &info)override;
    void createAP();
    void createSTA();
    void setListenLoad(bool bListen);
    void loadData(const QString &moduleName=QString());

private slots:
    void on_connect_clicked();

    void on_pingBtn_clicked();

    void on_returnBtn_clicked();

    void on_openTestBtn_clicked();

protected:
    Ui::WIFIWidget *ui;

    QString  m_Name;
    QString m_wifiName;
    QString m_connBssid;
    WIFIMODE m_staConfig;
    WifiModel *m_wpaModel;
    Wpaclicmd *m_wpaCli;

    WIFIMODE m_apConfig;
    WifiModel *m_hostModel;
    Hostapdcli *m_hostapdcil;

    QLabel *m_loadLbl;
    QTimer *m_loadTimer;
};

#endif // M4GWIDGET_H
