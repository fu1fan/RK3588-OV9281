#include "networkwidget.h"
#include "ui_networkwidget.h"
#include "inetwork.h"
#include <QDebug>
#include "terminal.h"
#include <QRegExpValidator>
#include <QCompleter>
#include <QStringListModel>
#include <QFile>
#include <QTimer>
#include "messagebox.h"
#include "basefactory.h"
#include <QRunnable>
#include "threadexec.h"
using namespace fl_common;

NetworkWidget::NetworkWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::NetworkWidget)
{
    ui->setupUi(this);

    QStringList cards = BaseFactory::instance()->wriednetwork()->availableCard();
    ui->cardCombo->addItems(cards);

    connect(ui->staticRadioBtn, &QRadioButton::toggled, this, [=](bool bCheck){
        ui->IPEdt->setEnabled(bCheck);
        ui->maskEdt->setEnabled(bCheck);
        ui->gatawayEdt->setEnabled(bCheck);
        ui->dnsEdt->setEnabled(bCheck);
        readNetwork();
    });

    connect(ui->cardCombo,  &QComboBox::currentTextChanged, this, [=](){
        ui->textBrowser->clear();
        if(ui->staticRadioBtn->isChecked()){
            readNetwork();
        }
    });

    connect(ui->enableChk,  &QCheckBox::stateChanged, this, [=](int state){
        QString cardName = ui->cardCombo->currentText().trimmed();
        ui->textBrowser->clear();

        if(state == Qt::Checked)
        {

        ThreadExec *exec = new fl_common::ThreadExec;
        exec->start<QString>([](const QString &cmd){INetwork::setUp(cmd, true);}, cardName);
        connect(exec, &ThreadExec::finish, exec, [=](){
                          ui->textBrowser->append(QString("The Network(%1) is Already Up").arg(cardName));
                      }, Qt::QueuedConnection);
        }else{
          INetwork::setUp(cardName, false);
          ui->textBrowser->append(QString("The Network(%1) is Already Down").arg(cardName));
        }
    });

    ui->staticRadioBtn->click();

    QRegExp regExp("((2[0-4]\\d|25[0-5]|[01]?\\d\\d?)\\.){3}(2[0-4]\\d|25[0-5]|[01]?\\d\\d?)");
    QRegExpValidator *pValidator= new QRegExpValidator(regExp, this);
    ui->IPEdt->setValidator(pValidator);
    ui->maskEdt->setValidator(pValidator);
    ui->dnsEdt->setValidator(pValidator);
    ui->gatawayEdt->setValidator(pValidator);

    connect(BaseFactory::instance()->wriednetwork(), &INetwork::runLogChange, this, [=](const QString &log){
        ui->textBrowser->append(log);
    });
}

NetworkWidget::~NetworkWidget()
{
    ProcessHandler::stop();
    delete ui;
}

void NetworkWidget::processMessage(const QString &info)
{
    ui->textBrowser->append(info);
}

void NetworkWidget::resetTr()
{
    ui->label_IP->setText(tr("IP:"));
    ui->label_masek->setText("Netmask:");
    ui->label_dns->setText(tr("DNS:"));
    ui->label_gateway->setText("Gataway:");
    ui->dhcpRadioBtn->setText(tr("DHCPC"));
    ui->staticRadioBtn->setText(tr("STATIC"));
    ui->setBtn->setText(tr("Setting"));
    ui->pingEdt->setText(tr("Ping"));
}

void NetworkWidget::readNetwork()
{
  //
    QString cardName = ui->cardCombo->currentText();
    QString gateway =INetwork::gateway();
    QString ip;
    QString mask;
    QStringList dns;
    QString broadcast;

    if(INetwork::isUp(cardName))
    {
        INetwork::getNetwork(cardName, ip, mask,broadcast, dns);
        ui->enableChk->setChecked(true);

    }else{
        ui->textBrowser->clear();
        ui->textBrowser->append(QString("The Network Not Open: %1").arg(cardName));
        bool bAuto = false;
        BaseFactory::instance()->wriednetwork()->readConf(cardName, ip, mask, gateway, dns, bAuto);
        ui->enableChk->setChecked(false);
    }

    if(dns.size()>0){
        ui->dnsEdt->setText(dns[dns.size()-1]);
    }
    ui->IPEdt->setText(ip);
    ui->maskEdt->setText(mask);
    ui->gatawayEdt->setText(gateway);

}

void NetworkWidget::on_setBtn_clicked()
{
    if(!NetworkingServer::isRoot())
    {
        MessageBox::showMessage(this, tr("Not root user."));
        return;
    }

    if(!NetworkingServer::isLinkUp(ui->cardCombo->currentText())){
        MessageBox::showMessage(this, tr("Please insert the network cable."));
        return;
    }

    if(ui->cardCombo->count() <=0){
        MessageBox::showMessage(this, tr("Not Device Find"));
        return;
    }

    //更新网络配置.
    if(ui->staticRadioBtn->isChecked())
    {
        if(ui->cardCombo->currentText().isEmpty() ||
                ui->IPEdt->text().isEmpty()|
                ui->maskEdt->text().isEmpty()||
                ui->gatawayEdt->text().isEmpty()||
                ui->dnsEdt->text().isEmpty())
        {
            MessageBox::showMessage(this, tr("Input Error"));
            return;
        }

        BaseFactory::instance()->wriednetwork()->setStatic(ui->cardCombo->currentText(),
                                     ui->IPEdt->text(),
                                     ui->maskEdt->text(),
                                     ui->gatawayEdt->text(),
                                     ui->dnsEdt->text());
    }else{
        BaseFactory::instance()->wriednetwork()->setAuto(ui->cardCombo->currentText());
    }

    ui->enableChk->blockSignals(true);
    ui->enableChk->setChecked(true);
    ui->enableChk->blockSignals(false);
    ui->textBrowser->clear();

    //重启网络。
    int state;
    if(BaseFactory::instance()->wriednetwork()->exec(&state)){
        state == 0 ? MessageBox::showMessage(this, tr("Set Network Ok!")): MessageBox::showMessage(this, tr("Set Network Config Ok,Plesase wait..."));

    }else{

        qDebug()<<state;
        qDebug()<<"exec falire";
    }
}

void NetworkWidget::on_pingBtn_clicked()
{
    QString s=ui->pingEdt->text();
    if(s.isEmpty()) {
        s = "www.elfboard.com";
    }

    s = QString("ping  %1 -c 5 -I %2").arg(s).arg(ui->cardCombo->currentText());
    qDebug()<<s;
    ui->textBrowser->append(s);
    this->start(s);

}
