/* 依赖 hostapd_cli 处理WIFI Ap信息*/
#ifndef HOSTAPDCLI_H
#define HOSTAPDCLI_H
#include "processhandler.h"
#include <QObject>


typedef struct WIFI_INFO_AP{
    QString ssid;
    int signal;
    QString rx_bytes;
    QString tx_bytes;
    QString connected_time;

    bool operator ==(const WIFI_INFO_AP &info)
    {
        return info.ssid == ssid;
    }
    void clear(){
        ssid.clear();
    }
    bool isEmpty(){return ssid.isEmpty();}
} ScanInfoAp;

class HostapdcliPrivate;
class Hostapdcli: public QObject,public fl_common::ProcessHandler
{
    Q_OBJECT
public:
    Hostapdcli(QObject *parent=nullptr);
    ~Hostapdcli();

    //    m_hostapdcil->start_hostapd("/lib/modules/4.9.170/8821cs.ko",
    //                                "8821cs",
    //                                "nl80211",
    //                                "wlan0",
    //                                "172.16.0.50");
    void start_hostapd(const QString & ko,
                       const QString &ModuleName,
                       const QString &driverName,
                       const QString &cardName,
                       const QString &ip);
    void stop_hostapd();

    void setStart(bool bStart);

    void setWifiName(const QString &ssid);

    void setWifiPassword(const QString &passphrase);

    void setInterface(const QString &interface);

    void reload();

    void allsta();

Q_SIGNALS:
    void hospapdStartFinish();
    void newConnected(const QString &ssid);
    void lostConnected(const QString &ssid);
    void newStaInfo(const QList<ScanInfoAp> &infos);

protected:

    void processMessage(const QString &d) override;
    friend class HostapdcliPrivate;
private:
    HostapdcliPrivate *m_d;
};

#endif // HOSTAPDCLI_H
