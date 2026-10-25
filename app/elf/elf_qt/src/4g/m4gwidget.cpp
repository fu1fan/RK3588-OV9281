#include "m4gwidget.h"
#include <QDebug>
#include "terminal.h"
#include <QTimer>
#include "messagebox.h"
#include "ui_m4gwidget.h"
#include "basefactory.h"

using namespace fl_common;
M4gWidget::M4gWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::M4gWidget)
{
    ui->setupUi(this);

    for(int i=0; i<BaseFactory::instance()->simNetworkCount(); i++){
        ISIMNetwork *psim = BaseFactory::instance()->simNetowrk(i);
        ui->comboBox->addItem(BaseFactory::instance()->simNetowrk(i)->name());

        connect(psim, &ISIMNetwork::onlineChange, [=](){resetTr();});
        connect(psim, &ISIMNetwork::runLogChange, [=](const QString &log){
            ui->textBrowser->append(log);
        });
    }

    // APN 参数读取设置.
    connect(ui->tabWidget, &QTabWidget::currentChanged, [=](int index){
        ISIMNetwork *psim = BaseFactory::instance()->simNetowrk(ui->comboBox->currentIndex());
        if( psim->isOpen() && index == 1){
            QString net;
            QString authentication;
            QString userName ;
            QString password;
            psim->getAPN(net, authentication, userName, password);
            ui->userNameEdt->setText(userName);
            ui->passwordEdt->setText(password);
            ui->apnEdt->setText(net);
            int index = authentication.toInt();
            ui->authCombox->setCurrentIndex(index);
        }
    });

    //AT 读取定时器.
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this,  &M4gWidget::readInformation);

    ui->tabWidget->tabBar()->setVisible(false);
    ui->AtInfoWidget->setVisible(false);

    this->resetTr();

}

M4gWidget::~M4gWidget()
{
    delete ui;
}

void M4gWidget::resetTr()
{
    ui->label_ISP->setText(tr("ISP:"));
    ui->label_manufactuer->setText("Manufacturer:");
    ui->label_version->setText("Version:");
    ui->label_IMEI->setText(tr("IMEI:"));
    ui->label_RSSI->setText("RSSI:");

    ui->tabWidget->setTabText(0, tr("DIAL"));
    ui->tabWidget->setTabText(1, tr("APN"));

    ui->label_APN->setText(tr("APN:"));
    ui->label_authencation->setText(tr("Authencation:"));
    ui->label_user->setText("User Name:");
    ui->label_password->setText("Password:");

    ui->applyBtn->setText(tr("Setting"));

    ISIMNetwork *psim = BaseFactory::instance()->simNetowrk(ui->comboBox->currentIndex());
    if(psim->isOpen()){
        ui->connectBtn->setText(tr("Stop"));
    }else{
        ui->connectBtn->setText(tr("Start"));
    }

}


void M4gWidget::on_connectBtn_clicked()
{

    ISIMNetwork *psim = BaseFactory::instance()->simNetowrk(ui->comboBox->currentIndex());
    ui->tabWidget->tabBar()->setVisible(psim->supportAPN());
    if(psim->isOpen()){
        if(psim->mayExec()){
            ui->AtInfoWidget->setVisible(false);
        }

        psim->close();

        ui->comboBox->setEnabled(true);
        ui->connectBtn->setText(tr("Start"));
        this->clearUi();
    }else{

        if(!psim->open()){
            ui->textBrowser->append(tr("Unable to start"));
            return ;
        }

        ui->connectBtn->setText(tr("Stop"));
        ui->comboBox->setEnabled(false);

        if(psim->mayExec()){
            ui->AtInfoWidget->setVisible(true);
            readInformation();
            m_timer->start(5000);
        }
    }
}

void M4gWidget::on_applyBtn_clicked()
{
    ISIMNetwork *psim = BaseFactory::instance()->simNetowrk(ui->comboBox->currentIndex());
    QString dial_APN =ui->apnEdt->text();
    if(dial_APN.isEmpty()){
        MessageBox::showMessage(this, tr("APN is Emtpy"));
        return;
    }
    QString dial_username = ui->userNameEdt->text();
    QString dial_password = ui->passwordEdt->text();
    QString  dial_auth = QString::number(ui->authCombox->currentIndex());
    QString str;
    if(!psim->setAPN(dial_APN, dial_auth, dial_username, dial_password)){
        str = tr("Set Falire");
    }else{
        str = tr("Set APN OK, please restart connect!");
    }
    MessageBox::showMessage(this, str);
}

void M4gWidget::clearUi()
{
    ui->textBrowser->clear();
    ui->ISPLbl->clear();
    ui->IMEILbl->clear();
    ui->versionLbl->clear();
    ui->RSSILbl->clear();
    ui->IMEILbl->clear();
}

void M4gWidget::readInformation()
{
    ISIMNetwork *psim = BaseFactory::instance()->simNetowrk(ui->comboBox->currentIndex());
    if(psim->isOpen())
    {
        QString result;
        QStringList list;
        result = psim->exec("ATE0\r");
        result = psim->exec("AT\r");
        result = psim->exec("AT+CGMI\r");
        list = Terminal::getMatch(result, "[a-zA-Z0-9]+");
        if(list.size() >0 && list.contains("OK")){
            ui->manufacturerLbl->setText(list[0]);
        }

        result = psim->exec("AT+CGMR\r");
        list = Terminal::getMatch(result, "[a-zA-Z0-9]+");
        if(list.size() >0 && list.contains("OK")){
            ui->versionLbl->setText(list[0]);
        }

        result = psim->exec("AT+CGSN\r");
        list = Terminal::getMatch(result, "[a-zA-Z0-9]+");
        if(list.size() >0 && list.contains("OK")){
            ui->IMEILbl->setText(list[0]);
        }

        result = psim->exec("AT+CIMI\r");
        list = Terminal::getMatch(result, "[0-9]+");
        if(list.size() >0){
            QString cimi = list[0];
            QString isp;
            if (cimi.startsWith("46000") || cimi.startsWith("46002")) {
                isp = "Mobile";
            } else if (cimi.startsWith("46001")) {
                isp = "Unicom";
            } else if (cimi.startsWith("46003")) {
                isp = "Telecom ";
            }else {
                isp=QString("cimi:%1").arg(result);
            }
            if(!isp.isEmpty()){
                ui->ISPLbl->setText(isp);
            }
        }

        result = psim->exec("AT+QENG=\"servingcell\"\r");
        list = result.split(",");
        bool bQNG=false;
        if(list.size() == 18){
            //  +QENG: "servingcell","NOCONN", "LTE","FDD", 460, 00, 7F3C740, 219, 1300, 3, 5, 5, 3193, -87, -13, -53,-4,40
            //   QENG:  "servingcell","NOCONN","LTE","FDD", 460, 11, 12C4000, 84,   100, 1, 5, 5, 1680, -62, -11, -31,16,65
            if(list.size() == 18){
                result = list[15];
                ui->RSSILbl->setText(result);
                bQNG = true;
            }
        }

        if(!bQNG)
        {
            result = psim->exec("AT+CSQ\r");
            list = Terminal::getMatch(result, "[a-zA-Z0-9]+");
            if(list.size() >1 && list.contains("OK")){
                //RSSI和CSQ之间的对应关系式什么呢？OK，就是：CSQ值=(接收信号强度dBm+113)/2。
                int csq = list[1].toInt();
                int signal = -113 + csq*2;
                ui->RSSILbl->setText(QString::number(signal));
            }
        }

    }
}
