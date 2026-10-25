#include "hostapdcli.h"
#include "terminal.h"
#include <QTimer>
#include <QDebug>
using namespace fl_common;

#define AP_INTERFACE  "interface=%1\n"
#define AP_DRIVER "driver=%1\n"
#define AP_SSID "ssid=%1\n"
#define AP_CHANNEL "channel=%1\n"
#define AP_HW_MODE "hw_mode=%1\n"
#define AP_MACADDR_ACL "macaddr_acl="
#define AP_IGNORE_BROADCAST_SSID  "ignore_broadcast_ssid=%1\n"
#define AP_AUTH_ALGS   "auth_algs=%1\n"
#define AP_WPA "wpa=%1\n"
#define AP_WPA_PASSPHRASE   "wpa_passphrase=%1\n"
#define AP_WPA_KEY_MGMT  "wpa_key_mgmt=%1\n"
#define AP_WPA_PAIRWISE  "wpa_pairwise=%1\n"
#define AP_RSN_PAIRWISE  "rsn_pairwise=%1\n"
#define AP_CTR_INTERFACE  "ctrl_interface=%1\n"
#define AP_RELOAD "reload\n"
#define AP_ALL_STA "all_sta\n"

class HostapdcliPrivate
{
public:
    HostapdcliPrivate(Hostapdcli *_q):q(_q){}
    void handAutoMessage(const QByteArray &a);
    void handCmdMessage(const QByteArray &d);
    QString get(const QByteArray &a, const QByteArray &start, const QByteArray &end);
    void pharsAllStaInfo(const QByteArray &a, QList<ScanInfoAp> &infos);

    QString message;
    QString ssid;
    QString passphrase;
    QString driver;
    QString interface;
    QString ip;
    QTimer *timer;
    Hostapdcli *q;
};

Hostapdcli::Hostapdcli(QObject *parent):QObject(parent),
    m_d(new HostapdcliPrivate(this))
{
    m_d->timer =new QTimer(this);
    connect(m_d->timer, &QTimer::timeout, this, [=](){
        allsta();
    });
    m_d->timer->setInterval(2000);
}

Hostapdcli::~Hostapdcli()
{
    delete m_d;
}

void Hostapdcli::start_hostapd(const QString &ko, const QString &ModuleName, const QString &driverName, const QString &cardName, const QString &ip)
{
    qDebug()<<"start_hostapd";

    m_d->driver = driverName;
    m_d->interface = cardName;
    m_d->ip = ip;

    QString moudleIsExit =QString("lsmod | grep %1 | grep -v grep").arg(ModuleName);
    if(!Terminal::execCmd(moudleIsExit).contains(ModuleName))
    {
        qDebug() << Terminal::execCmd("insmod " + ko);
        qDebug() << QString("insmod %1").arg(ko);
    }else{
        qDebug() << QString("driver: %1 already install").arg(ModuleName);
    }

    QString defaultConfig = QString(
                "ctrl_interface=/var/run/hostapd\n"
                "interface=wlan0\n"
                "driver=%1\n"
                "ssid=wifi_test\n"
                "channel=9\n"
                "hw_mode=g\n"
                "macaddr_acl=0\n"
                "ignore_broadcast_ssid=0\n"
                "auth_algs=1\n"
                "wpa=3\n"
                "wpa_passphrase=12345678\n"
                "wpa_key_mgmt=WPA-PSK\n"
                "wpa_pairwise=TKIP\n"
                "rsn_pairwise=CCMP\n").arg(driverName) ;
    QString hostapdConf = QString("echo \"%1\" > /etc/hostapd.conf").arg(defaultConfig);

    Terminal::execCmd(hostapdConf);

    auto driveInstallFinish = [=](){
        Terminal::execCmd("echo 1 > /proc/sys/net/ipv4/ip_forward");
        Terminal::execCmd("iptables -t nat -A POSTROUTING -o eth0 -j MASQUERADE");
        Terminal::execCmd(QString("ifconfig %1 up").arg(cardName));
        Terminal::execCmd(QString("ifconfig %1 %2").arg(cardName).arg(ip));
        QString startSevrverCmd =QString("hostapd /etc/hostapd.conf &");
        if(!Terminal::execCmd("ps -ef | grep -v grep | grep \"hostapd /etc/hostapd.conf\"").contains("hostapd"))
        {
            system(startSevrverCmd.toLatin1().data());
            qDebug()<<"start :"<< startSevrverCmd;
        }else{
            qDebug() <<"hostapd already start!";
        }

        if(!Terminal::execCmd("ps -ef | grep -v grep | grep dnsmasq").contains("dnsmasq"))
        {
            Terminal::execCmd("/etc/init.d/dnsmasq start");
        }else{
            qDebug() <<"dnsmasq already start!";
        }

    };

    ///一秒后在启动服务，应为wifi网卡驱动可能没有安转好.
    QTimer::singleShot(1000, this, [=](){
        driveInstallFinish();
        emit hospapdStartFinish();
    });
}

void Hostapdcli::setStart(bool bStart)
{
    if(bStart)
    {
        this->start("hostapd_cli");
        m_d->timer->start();
    }else{
        m_d->timer->stop();
        this->stop();
    }
}

void Hostapdcli::stop_hostapd()
{
    qDebug()<<"stop_hostapd ";
    Terminal::killProcess("dnsmasq");
    Terminal::killProcess("hostapd");
    Terminal::execCmd(QString("ifconfig %1 down").arg(m_d->interface));
    this->stop();
    QList<ScanInfoAp> info;
    emit newStaInfo(info);
}

void Hostapdcli::setWifiName(const QString &ssid)
{
    m_d->ssid = ssid;
    send(QString(AP_SSID).arg(ssid).toLocal8Bit());
}

void Hostapdcli::setWifiPassword(const QString &passphrase)
{
    m_d->passphrase = passphrase;
    send(QString(AP_WPA_PASSPHRASE).arg(passphrase).toLocal8Bit());
}

void Hostapdcli::setInterface(const QString &interface)
{
    m_d->interface = interface;
    send(QString(AP_INTERFACE).arg(interface).toLocal8Bit());
}

void Hostapdcli::reload()
{
    send(AP_RELOAD);
}

void Hostapdcli::allsta()
{
    send(AP_ALL_STA);
}

void Hostapdcli::processMessage(const QString &d)
{
    m_d->message +=d;
    char enter ='\n';
    char end='>';

    QByteArray cmdData;
    QByteArray rowData;

    char lastChar;
    for(int i=0; i<m_d->message.size(); i++)
    {
        char c = m_d->message.at(i).toLatin1();
        cmdData +=c;
        rowData +=c;

        if(c == enter)
        {
            m_d->handAutoMessage(rowData);
            qDebug()<<"+++++++++++++++++"<< rowData;
            rowData.clear();
        }

        if(c == end  && lastChar == enter)
        {
            m_d->handCmdMessage(cmdData);
            qDebug()<<"#################"<< cmdData;
            cmdData.clear();
        }
        lastChar = c;
    }

    //保存未处理完消息.
    m_d->message = cmdData;
}

void HostapdcliPrivate::handAutoMessage(const QByteArray &a)
{
    if(a.contains("AP-STA-CONNECTED"))
    {
        QString ssid = get(a+"end", "AP-STA-CONNECTED", "end");
        emit q->lostConnected(ssid);
    }else if(a.contains("AP-STA-DISCONNECTED"))
    {
        QString ssid = get(a+"end", "AP-STA-DISCONNECTED", "end").trimmed();

        qDebug()<<"lose ssid ="<<ssid;

        emit q->lostConnected(ssid);
    }
}

void HostapdcliPrivate::handCmdMessage(const QByteArray &d)
{
    QList<ScanInfoAp>  list;
    pharsAllStaInfo(d, list);
    if(!list.isEmpty())
    {
        emit q->newStaInfo(list);
//        for(int i=0; i<list.size(); i++)
//        {
//            qDebug()<<"**************"<< list[i].ssid;
//        }

    }

    return;
    QString cmdHead;
    QByteArray data;
    int index =0;
    for(index =0; index<d.size(); index++){
        cmdHead+=d[index];
        if(d[index] == '\n'){
            data = d.mid(index).trimmed();
            break;
        }
    }

    if(cmdHead.isEmpty())
    {
        return;
    }

//    qDebug()<<"cmdhad ="<<cmdHead ;
//    qDebug()<<"data ="<< data;

    if(cmdHead.contains(AP_RELOAD))
    {
        return;
    }else if(cmdHead.contains(AP_ALL_STA))
    {
        QList<ScanInfoAp>  list;
        pharsAllStaInfo(data, list);
        emit q->newStaInfo(list);
        return;
    }if(cmdHead.contains(QString(AP_INTERFACE).arg(interface)))
    {

    }else if(cmdHead.contains(QString(AP_DRIVER).arg(driver)))
    {

    }else if(cmdHead.contains(QString(AP_SSID).arg(ssid)))
    {

    }else if(cmdHead.contains(QString(AP_WPA_PASSPHRASE).arg(passphrase)))
    {

    }

    q->reload();
}

QString HostapdcliPrivate::get(const QByteArray &a, const QByteArray &start, const QByteArray &end)
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

void HostapdcliPrivate::pharsAllStaInfo(const QByteArray &a,  QList<ScanInfoAp>  &infos)
{
    QList<QByteArray>list = a.split('\n');

    ScanInfoAp ap;
    foreach (QByteArray row, list) {
        row +="end";
        if(row.contains("dot11RSNAStatsSTAAddress"))
        {
            ap.ssid = get(row, "dot11RSNAStatsSTAAddress=" , "end");
        }else if(row.contains("rx_bytes"))
        {
            ap.rx_bytes = get(row, "rx_bytes=" , "end");
        }else if(row.contains("tx_bytes"))
        {
            ap.tx_bytes = get(row, "tx_bytes=" , "end");
        }else if(row.contains("signal"))
        {
            ap.signal=get(row, "signal=" , "end").toInt();
        }else if(row.contains("connected_time"))
        {
            ap.connected_time = get(row, "connected_time=" , "end");
            infos << ap;
        }

    }
    //        4c:49:e3:19:65:aa
    //        flags=[AUTH][ASSOC][AUTHORIZED]
    //        aid=0
    //        capability=0x0
    //        listen_interval=0
    //        supported_rates=
    //        timeout_next=NULLFUNC POLL
    //        dot11RSNAStatsSTAAddress=4c:49:e3:19:65:aa
    //        dot11RSNAStatsVersion=1
    //        dot11RSNAStatsSelectedPairwiseCipher=00-0f-ac-4
    //        dot11RSNAStatsTKIPLocalMICFailures=0
    //        dot11RSNAStatsTKIPRemoteMICFailures=0
    //        hostapdWPAPTKState=11
    //        hostapdWPAPTKGroupState=0
    //        rx_packets=160
    //        tx_packets=38
    //        rx_bytes=0
    //        tx_bytes=0
    //        inactive_msec=256
    //        signal=-73
    //        rx_rate_info=0
    //        tx_rate_info=0
    //        connected_time=11
    //        supp_op_classes=515153547374757677787c7d7e7f8082
    //        92:14:5b:9a:65:cd
    //        flags=[AUTH][ASSOC][AUTHORIZED]
    //        aid=0
    //        capability=0x0
    //        listen_interval=0
    //        supported_rates=
    //        timeout_next=NULLFUNC POLL
    //        dot11RSNAStatsSTAAddress=92:14:5b:9a:65:cd
    //        dot11RSNAStatsVersion=1
    //        dot11RSNAStatsSelectedPairwiseCipher=00-0f-ac-4
    //        dot11RSNAStatsTKIPLocalMICFailures=0
    //        dot11RSNAStatsTKIPRemoteMICFailures=0
    //        hostapdWPAPTKState=11
    //        hostapdWPAPTKGroupState=0
    //        rx_packets=1516
    //        tx_packets=1159
    //        rx_bytes=0
    //        tx_bytes=0
    //        inactive_msec=323
    //        signal=-57
    //        rx_rate_info=0
    //        tx_rate_info=0
    //        connected_time=236
}
