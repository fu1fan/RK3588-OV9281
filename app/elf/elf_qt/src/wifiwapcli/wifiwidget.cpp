#include "wifiwidget.h"
#include "ui_wifiwidget.h"
#include <QDebug>
#include "terminal.h"
#include <QProcess>
#include <QFile>
#include  <QDialog>
#include <QTimer>
#include "hostapdcli.h"
#include "messagebox.h"
#include<QMainWindow>
#include <QUrl>
#include "messagebox.h"
#include "wpaclicmd.h"
#include "settings.h"

using namespace fl_common;

WIFIWidget::WIFIWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WIFIWidget),
    m_wpaCli(Q_NULLPTR),
    m_hostapdcil(Q_NULLPTR),
    m_loadLbl(Q_NULLPTR),
    m_loadTimer(Q_NULLPTR)
{
    ui->setupUi(this);
    setListenLoad(false);
    loadData();
    createAP();
    createSTA();
}

WIFIWidget::~WIFIWidget()
{
    ui->staBtn->setOpen(false);
    delete ui;
}

void WIFIWidget::createAP()
{
    QString hasApMode =Settings::instance()->getValue("wpacli_ap", "false");
    if(hasApMode == "false"){
        ui->commandLinkButton_ap->setVisible(false);
        return;
    }
    m_hostModel =new WifiModel;
    WifiItemDelegate *delegate = new WifiItemDelegate;
    ui->listViewHost->setModel(m_hostModel);
    ui->listViewHost->setItemDelegate(delegate);
    ui->listViewHost->setSelectionMode(QAbstractItemView::SingleSelection);

    m_hostapdcil = new Hostapdcli(this);
    connect(m_hostapdcil, &Hostapdcli::hospapdStartFinish, this, [=](){
        m_hostapdcil->setStart(true);
        ui->wifiAPNameEdt->setText("wifi: wifi_test");
        ui->wifiAPPasswordEdt->setText("passowrd : 12345678");
    });
    connect(m_hostapdcil, &Hostapdcli::newStaInfo, this, [=](const QList<ScanInfoAp> &infos){
        m_hostModel->setDataHostap(infos);
    });
    connect(m_hostapdcil, &Hostapdcli::lostConnected, this, [=](const QString &ssid){
        m_hostModel->lostConnectHoseap(ssid);
    });

    ui->apBtn->setText(m_apConfig.cardName.toUpper());
    ui->wifiAPNameEdt->setReadOnly(true);
    ui->wifiAPPasswordEdt->setReadOnly(true);
    connect(ui->apBtn, &SwitchWidget::click, this, [=](bool bOpen){
        if(bOpen)
        {
            m_loadTimer->start();
            m_hostapdcil->start_hostapd(m_apConfig.ko,
                                        m_apConfig.moduleName,
                                        m_apConfig.driverName,
                                        m_apConfig.cardName,
                                        m_apConfig.ip);
        }else{
            m_loadTimer->stop();
            m_hostapdcil->stop_hostapd();
            ui->wifiAPNameEdt->clear();
            ui->wifiAPPasswordEdt->clear();
        }
    });

    connect(ui->commandLinkButton_sta, &QCommandLinkButton::clicked, this, [=](){

        if(ui->apBtn->isOpen())
        {
            int ret = MessageBox::showMessage(this, QString("%1 will close, is continue?").arg(m_staConfig.cardName), MessageBox::yes_no);
            if(ret == MessageBox::no)
            {
                return;
            }
            ui->apBtn->setOpen(false);
        }
        ui->stackedWidget->setCurrentIndex(0);

    });

    //更改系统随机数，不使用硬件随机数,WIFI热点导致分配不上.
    Terminal::execCmd("mv /dev/random /dev/random_copy");
    Terminal::execCmd("ln -s /dev/urandom /dev/random");
}

void WIFIWidget::createSTA()
{    
    m_wpaCli = new Wpaclicmd(this);
    m_wpaModel =new WifiModel;
    ui->listViewWpa->setModel(m_wpaModel);
    ui->listViewWpa->setItemDelegate(new WifiItemDelegate);
    ui->listViewWpa->setSelectionMode(QAbstractItemView::SingleSelection);

    connect(m_wpaCli, &Wpaclicmd::wpa_supplicantStartFinish, [=](){
        m_wpaCli->setStart(true);
    });

    connect(m_wpaCli , &Wpaclicmd::wifiScanResult, this, [=](const QList<ScanInfo> & info){
        m_wpaModel->setData(info);
    }, Qt::QueuedConnection);


    ui->statusLbl->setTextFormat(Qt::AutoText);
    ui->statusLbl->setWordWrap(true);
    connect(m_wpaCli, &Wpaclicmd::wifiStatus, this,  [=](const QMap<QString, QString > &infos){

        QString bssid = infos.value("bssid");
        if(bssid.isEmpty())
        {
            ui->statusLbl->clear();
            return;
        }

        QString showInfo("ssid :  %1\nencryption :  %2\nfreq :  %3\nmac :  %4 \n ");
        showInfo = showInfo.arg(infos.value("ssid"))
                .arg(infos.value("key_mgmt"))
                .arg(infos.value("freq"))
                .arg(infos.value("bssid"));
        ui->statusLbl->setText(showInfo);
    }, Qt::QueuedConnection);

    connect(ui->listViewWpa ,&QListView::clicked, this, [=](const QModelIndex &index){
        QList<QVariant> list = index.data(Qt::DisplayRole).toList();
        QString wifiName = list[4].toString();
        ui->wifiNameEdt->setText(wifiName);
    });

    ui->staBtn->setText(m_staConfig.cardName.toUpper());
    connect(ui->staBtn, &SwitchWidget::click, this, [=](bool bOpen){
        this->setListenLoad(bOpen);

        if(ui->comboBox->currentText().isEmpty())
        {
            MessageBox::showMessage(this, tr("No available devices"));
            ui->staBtn->blockSignals(true);
            ui->staBtn->setOpen(false);
            ui->staBtn->blockSignals(false);
            setListenLoad(false);
            return;
        }

        if(bOpen){
            ui->comboBox->setEnabled(false);
            m_wpaCli->startWpa_supplicant(m_staConfig.ko,
                                          m_staConfig.moduleName,
                                          m_staConfig.driverName,
                                          m_staConfig.cardName);
        }else{
            ui->comboBox->setEnabled(true);
            m_wpaCli->setStart(false);
        }
    });

    connect(ui->comboBox, &QComboBox::currentTextChanged, this, [=](const QString &moduleName){
        loadData(moduleName);
    });

    connect(ui->commandLinkButton_ap, &QCommandLinkButton::clicked, this, [=](){

        if(ui->staBtn->isOpen())
        {
            int ret = MessageBox::showMessage(this, QString("%1 will close, is continue?").arg(m_staConfig.cardName), MessageBox::yes_no);
            if(ret == MessageBox::no)
            {
                return;
            }

            ui->staBtn->setOpen(false);
        }
        ui->stackedWidget->setCurrentIndex(1);

    });

}

void WIFIWidget::setListenLoad(bool bListen)
{
    if(m_loadTimer== Q_NULLPTR)
    {
        m_loadLbl = new QLabel(this);
        m_loadLbl->setStyleSheet("color: blue;");
        m_loadTimer = new QTimer(this);
        m_loadTimer->setInterval(500);
        connect(m_loadTimer, &QTimer::timeout, [=](){
            if(ui->stackedWidget->currentIndex() == 0)
            {
                if(!m_wpaModel->hasIndex(0, 0))
                {
                    m_loadLbl->setText("search ...");
                }else{
                    m_loadLbl->clear();
                }
            }
        });
    }

    if(bListen)
    {
        m_loadLbl->clear();
        m_loadTimer->start();
        QRect  rect = ui->listViewWpa->rect();
        m_loadLbl->setFixedSize(100, 50);
        m_loadLbl->move(rect.x() + rect.width()/2, rect.y() + rect.height()/2);
        m_loadLbl->show();
    }else{
        m_loadTimer->stop();
        m_loadLbl->clear();
    }
}

void WIFIWidget::loadData(const QString &moduleName)
{
    static QList<UMap<UString, UString> > list = Settings::instance()->getSections("[wpacli]");

    auto setConfig = [=](const UMap<UString, UString> &map){
        m_staConfig.moduleName=map.value("moduleName");
        m_staConfig.cardName = map.value("cardName");
        m_staConfig.driverName =map.value("driverName");
        m_staConfig.ko = map.value("ko");

        m_apConfig.cardName= map.value("cardNameAP");
        m_apConfig.driverName =map.value("driverNameAP");
        m_apConfig.moduleName=map.value("moduleNameAP");
        m_apConfig.ko =map.value("koAP");
    };

    if(moduleName.isEmpty()){
        if(ui->comboBox->currentText().isEmpty())
        {
            for (UMap<UString, UString> map : list) {
                ui->comboBox->addItem(map.value("moduleName"));
            }

            if(list.size() >0)
                setConfig(list[0]);
        }
    }else{
        for (UMap<UString, UString> map : list) {
            if(map.value("moduleName") == moduleName){
                setConfig(map);
            }
        }
    }
}


void WIFIWidget::on_connect_clicked()
{
    if(!ui->staBtn->isOpen())
    {
        MessageBox::showMessage(this, tr("Wifi Not Open!"));
        return ;
    }

    QString wifiName= ui->wifiNameEdt->text().trimmed();
    QString wifiPassword= ui->WifiPasswordEdt->text().trimmed();
    if(wifiName.isEmpty() && wifiPassword.isEmpty())
    {
        MessageBox::showMessage(this, tr("Wifi name or password is empty!"));
        return ;
    }

    m_wpaCli->connectWifi(wifiName, wifiPassword);
}

void WIFIWidget::on_pingBtn_clicked()
{
    QString cmd;
    if(ui->addressEdt->text().trimmed().isEmpty())
    {
        cmd = QString("ping www.elfboard.com  -c 5 -I %2").arg(m_staConfig.cardName);
    }else{
        cmd = QString("ping %1 -c 5 -I %2").arg(ui->addressEdt->text()).arg(m_staConfig.cardName);
    }
    ui->textBrowser->clear();
    ui->textBrowser->append(cmd);
    this->start(cmd);
}

void WIFIWidget::on_returnBtn_clicked()
{
    ui->stackedWidget->setCurrentIndex(0);
}

void WIFIWidget::on_openTestBtn_clicked()
{
    ui->textBrowser->clear();
    ui->stackedWidget->setCurrentIndex(2);
}

void WIFIWidget::processMessage(const QString &info)
{
    ui->textBrowser->append(info);
}
