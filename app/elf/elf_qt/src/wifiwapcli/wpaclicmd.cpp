#include "wpaclicmd.h"
#include <QTimer>
#include <QDebug>

#define CMD_SCAN "scan"
#define CMD_SCAN_RESULTS "scan_results"
#define CMD_STATUS "status"
#define CMD_ADD_NETWORK "add_network"
#define CMD_REMOVE_NETORK  "remove_network"
#define CMD_SET_NETOWRK_SSID  "set_network %1 ssid '\"%2\"'"
#define CMD_SET_NETOWRK_PSK   "set_network %1 psk '\"%2\"'"
#define CMD_SELECT_NETWORK "select_network %1"
#define CMD_REMOVE_NETWORK "remove_network %"
#define CMD_SAVE_CONFIG  "save_config"
#define CMD_DISCONNECT_CONNECT "disconnect"
#include "terminal.h"

using namespace fl_common;

class WpaclicmdPrivate{
public:
    QList<ScanInfo> scanResults;
    QString ko;
    QString cardName;
    bool bWpa_supplicantStartFinish=false;
    QString network=0;
    QTimer *timer=Q_NULLPTR;
    bool bRun =false;

    QString get(const QString &a, const QString &start, const QString &end)
    {
        //CTRL-EVENT-DISCONNECTED bssid=c2:5a:cd:8c:0f:f4 reason=3  //start ="bssid="  end="reaseo"    result ="c2:5a:cd:8c:0f:f4"
        QByteArray data;
        QString result;
        for(int i=0; i<a.size(); i++){
            data  +=a[i];
            if(data.right(start.size()) == start){
                data.clear();
            }else if(data.right(end.size()) == end)
            {
                result = data.left(data.size() - end.size());
                result = result.trimmed();
                break;
            }
        }

        return result;
    }

    QList<ScanInfo> pharseScanInfo(const QString &a)
    {
        QStringList list = a.split('\n');
        QList<ScanInfo>  wifiList;
        QString wifiName;
        for(int i=0; i<list.size(); i++){
            QStringList l =list[i].split('\t');
            l.removeAll("");
            if(l.size() ==5){
                ScanInfo scan;
                scan.bssid = l[0];
                scan.frequency = l[1].toInt();
                scan.signalLevel = l[2].toInt();
                scan.flags = l[3];
                scan.ssid =l[4];
                if(!wifiList.contains(scan)/* && wifiName!= scan.ssid*/)
                {
                    wifiList.append(scan);
                    wifiName = scan.ssid;
                }
            }
        }

        auto moreThan =[=](const ScanInfo & t1, const ScanInfo & t2){
            return t1.signalLevel > t2.signalLevel;
        };
        qSort(wifiList.begin(), wifiList.end(), moreThan);

        return wifiList;
    }

    void pharsStatusInfo(const QString &a, QMap<QString, QString> &maps){
        //    bssid=52:57:89:05:db:b9
        //    freq=2412
        //    ssid=wifi
        //    id=0
        //    mode=station
        //    pairwise_cipher=CCMP
        //    group_cipher=CCMP
        //    key_mgmt=WPA2-PSK
        //    wpa_state=COMPLETED
        //    ip_address=192.168.43.236
        //    p2p_device_address=30:95:87:7a:fb:48
        //    address=30:95:87:7a:fb:48
        //    uuid=1f7702ee-85b7-5974-9b6a-94b2ce28e72d
        //    ieee80211ac=1
        QStringList list =a.split('\n');
        foreach (QString d, list) {
            d +="###";
            if(d.contains("bssid="))
            {
                maps.insert("bssid", get(d, "bssid=", "###"));
            }else if(d.contains("ip_address"))
            {
                maps.insert("ip_address", get(d, "ip_address=", "###"));
            }else if(d.contains("key_mgmt"))
            {
                maps.insert("key_mgmt", get(d, "key_mgmt=", "###"));
            }else if(d.contains("freq="))
            {
                QString freq = get(d, "freq=", "###");
                if(freq.toInt()>=5000)
                {
                    maps.insert("freq", "5 GHz");
                }else{
                    maps.insert("freq", "2.4 GHz");
                }
            }else if(d.contains("ssid"))
            {
                maps.insert("ssid", get(d, "ssid=", "###"));
            }else if(d.contains("wpa_state="))
            {
                maps.insert("wpa_state", get(d, "wpa_state=", "###"));
            }
        }
    }
};

Wpaclicmd::Wpaclicmd(QObject *parent) :
    QThread(parent),
    m_d(new WpaclicmdPrivate)
{

    qRegisterMetaType<QList<ScanInfo>>("QList<ScanInfo>");
}

Wpaclicmd::~Wpaclicmd()
{
    this->stopWpa_supplicant();
    this->setStart(false);
    delete m_d;
}

void Wpaclicmd::startWpa_supplicant(const QString &ko, const QString &ModuleName, const QString &driverName, const QString &cardName)
{
    qDebug()<<"start startWpa_supplicant";

    QString moudleIsExit =QString("lsmod | grep %1 | grep -v grep").arg(ModuleName);

    if(ko != m_d->ko){
       stopWpa_supplicant();
    }
    if(!Terminal::execCmd(moudleIsExit).contains(ModuleName))
    {
        qDebug() << Terminal::execCmd("insmod " + ko);
        qDebug() << QString("insmod %1").arg(ko);
    }else{
        qDebug() << QString("driver: %1 already install").arg(ModuleName);
    }
    m_d->ko = ko;

    //修改wifi配置文件.
    Terminal::execCmd("echo \"#PSK/TKIP \nctrl_interface=/var/run/wpa_supplicant\" > /etc/wpa_supplicant.conf");

    auto driveInstallFinish = [=](){
        Terminal::execCmd(QString("ifconfig %1 up").arg(cardName));
        m_d->cardName = cardName;
        //启动 wpa 服务.  //wpa_supplicant -D nl80211 -i wlan0 -c /etc/wpa_supplicant.conf -B
        QString startSevrverCmd =QString("wpa_supplicant -D %1 -i %2 -c /etc/wpa_supplicant.conf -B")
                .arg(driverName)
                .arg(cardName);
        if(!Terminal::execCmd("ps -ef | grep -v grep | grep wpa_supplicant").contains(startSevrverCmd))
        {
            qDebug()<<"start =" << Terminal::execCmd(startSevrverCmd);
            qDebug()<<"start :"<< startSevrverCmd;
        }else{
            qDebug() <<"wpa_supplicant already start!";
        }
    };

    ///一秒后在启动服务，应为wifi网卡驱动可能没有安转好.
    QTimer::singleShot(1000, this, [=](){
        driveInstallFinish();
        emit wpa_supplicantStartFinish();
    });
}

void Wpaclicmd::stopWpa_supplicant()
{
    if(!m_d->ko.isEmpty() )
    {
        Terminal::execCmd(QString("ifconfig %1 up").arg(m_d->cardName));
        Terminal::execCmd(QString("rmmod %1").arg(m_d->ko));
        Terminal::killProcess("wpa_supplicant");
    }
}


void Wpaclicmd::setStart(bool bStart)
{
    if(m_d->timer == Q_NULLPTR)
    {
        m_d->timer = new QTimer(this);
        connect(m_d->timer, &QTimer::timeout, this, [=](){this->start();});
    }


    if(bStart)
    {
        m_d->timer->start(2000);
        this->start();
    }else{
        m_d->timer->stop();
        if(this->isRunning())
        {
            this->terminate();
            this->wait();
        }
        m_d->scanResults.clear();
        emit wifiScanResult(m_d->scanResults);
    }
}

void Wpaclicmd::connectWifi(const QString &ssid, const QString &psk)
{
    bool bOk;
    m_d->network = addNetwork().trimmed();
    m_d->network.toInt(&bOk);
    //    qDebug()<<"addnetwork" << m_d->network <<"" << m_d->cardName;
    if(bOk)
    {
        QString  result =setSSID(ssid);
        if(!result.contains("OK"))
        {
            qDebug()<<"setssid: error";
            return;
        }

        result =  setPSK(psk);
        if(!result.contains("OK"))
        {
            qDebug()<<"setPSK: error";
            return;
        }

        result =  selectNetwork(m_d->network);
        if(!result.contains("OK"))
        {
            qDebug()<<"selectNetworks: error";
            return;
        }

        //        result =  saveConfig();
        //        if(!result.contains("OK"))
        //        {
        //             qDebug()<<"saveConfig: error";
        //            return;
        //        }

        udhcpc(m_d->cardName);
    }
}

QString Wpaclicmd::disconnectWifi()
{
    return execCmd(CMD_DISCONNECT_CONNECT);
}

void Wpaclicmd::run()
{
    QString scanInfo = this->scan();
    if(scanInfo.contains("OK")){
        QString scanInfo  = scanResults();
        m_d->scanResults= m_d->pharseScanInfo(scanInfo);
    }

    QString statusInfo = this->status();
    QMap<QString, QString > info;
    m_d->pharsStatusInfo(statusInfo, info);
    emit wifiStatus(info);

    QString ssid = info.value("ssid");
    QString wpa_state = info.value("wpa_state");
    int connectIndex =-1;
    for(int i=0; i<m_d->scanResults.size(); i++)
    {
        if(m_d->scanResults[i].ssid == ssid)
        {
            connectIndex = i;
            m_d->scanResults[i].state =1;
            if(wpa_state =="COMPLETED"){
                m_d->scanResults[i].state =2;
            }
            break;
        }
    }

    if(connectIndex >=0)
    {
        m_d->scanResults.swap(0, connectIndex);
    }

    emit wifiScanResult(m_d->scanResults);
}

QString Wpaclicmd::scan()
{
    return execCmd(CMD_SCAN);
}

QString Wpaclicmd::scanResults()
{
    return  execCmd(CMD_SCAN_RESULTS);
}

QString Wpaclicmd::status()
{
    return execCmd(CMD_STATUS);
}

QString Wpaclicmd::addNetwork()
{
    return execCmd(CMD_ADD_NETWORK);
}

QString Wpaclicmd::removeNetWork()
{
    return execCmd(CMD_REMOVE_NETORK);
}

QString Wpaclicmd::setSSID(const QString &ssid)
{
    QString cmd= QString(CMD_SET_NETOWRK_SSID)
            .arg(m_d->network)
            .arg(ssid);
    return execCmd(cmd);
}

QString Wpaclicmd::setPSK(const QString &psk)
{
    QString cmd = QString(CMD_SET_NETOWRK_PSK)
            .arg(m_d->network)
            .arg(psk);

    return execCmd(cmd);
}

QString Wpaclicmd::selectNetwork(const QString &network)
{
    QString cmd = QString(CMD_SELECT_NETWORK)
            .arg(network);
    return execCmd(cmd);
}

void Wpaclicmd::udhcpc(const QString &cardName)
{
    static QString ucpcp = QString("udhcpc -i %1 -q &").arg(cardName);
    Terminal::killProcess(ucpcp);
    qDebug()<< ucpcp;
    system(ucpcp.toLocal8Bit().constData());
}

QString Wpaclicmd::saveConfig()
{
    return execCmd(CMD_SAVE_CONFIG);
}

QString Wpaclicmd::execCmd(const QString &cmd)
{
    return Terminal::execCmd(QString("wpa_cli %1 -i %2").arg(cmd).arg(m_d->cardName));
}
