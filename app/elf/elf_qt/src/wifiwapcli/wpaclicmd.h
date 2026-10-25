#ifndef WPACLICMD_H
#define WPACLICMD_H

#include <QObject>
#include <QThread>
#include "processhandler.h"
class ScanInfo{
public:
    QString bssid;
    int frequency ;
    int signalLevel;
    QString  flags;
    QString ssid;
    int state=0;
    bool operator ==(const ScanInfo &info)
    {
        return info.ssid == ssid  && state == info.state;
    }
    void clear(){
        ssid.clear();
    }
    bool isEmpty(){return ssid.isEmpty();}
} ;


class WpaclicmdPrivate;
class Wpaclicmd : public QThread
{
    Q_OBJECT
public:
    explicit Wpaclicmd(QObject *parent = nullptr);
    ~ Wpaclicmd();

    //        m_wpaCli->startWpa_supplicant("/lib/modules/4.9.170/8821cs.ko",
    //                                      "8821cs",
    //                                      "nl80211",
    //                                      "wlan0");
    //启动Wpa_supplicant服务.
    void startWpa_supplicant(const QString & ko,
                             const QString &ModuleName,
            const QString &driverName, const QString &cardName);


    void stopWpa_supplicant();

    //启动 wap_cli 客户.
    void setStart(bool bStart);
    void connectWifi(const QString &ssid, const QString &psk);
    QString disconnectWifi();

protected:
    void run();

Q_SIGNALS:
    void wpa_supplicantStartFinish();
    void wifiScanResult(const QList<ScanInfo> & info);
    void wifiStatus(const QMap<QString, QString > &info);


private:
    QString scan();
    QString scanResults();
    QString status();
    QString addNetwork();
    QString removeNetWork();
    QString setSSID(const QString &ssid);
    QString setPSK(const QString &psk);
    QString selectNetwork(const QString &network);
    void udhcpc(const QString & cardName);
    QString saveConfig();
    QString execCmd(const QString &cmd);

private:
    WpaclicmdPrivate *m_d;
};

#endif // WPACLICMD_H
