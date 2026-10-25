#include "wifiwidget.h"
#include "ui_wifiwidget.h"
#include <QMessageBox>
#include "messagebox.h"
#include <QFile>
#include "settings.h"

using namespace fl_common;
WifiWidget::WifiWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WifiWidget)
{
    ui->setupUi(this);

    ui->hostip->setText("192.168.1.111");

    myprocess = new QProcess(this);
    connect(myprocess, SIGNAL(readyReadStandardOutput()),this, SLOT(result()));
    connect(myprocess, SIGNAL(readyReadStandardError()),this, SLOT(result()));

    //加载配置文件.
    UMap<UString, UString> map = Settings::instance()->getSection("[wifi]");
    QString names = map.value("names");
    ui->comboBox->addItems(Settings::instance()->valuetoList(names));
    ui->ssid->setText(map.value("ssid"));
    ui->pawd->setText(map.value("pawd"));
    ui->hostip->setText(map.value("ip"));
}

WifiWidget::~WifiWidget()
{
    delete ui;
}

void WifiWidget::result()
{
    QString abc = myprocess->readAllStandardOutput();
    ui->result->append(abc.trimmed());
    QString efg = myprocess->readAllStandardError();
    if(efg.length()>1)ui->result->append(efg.trimmed());
}


void WifiWidget::closeEvent(QCloseEvent *)
{
    myprocess->kill();
    destroy();
    exit(0);
}


void WifiWidget::on_connect_clicked()
{
    if(ui->ssid->text() == QString(""))
    {
        MessageBox::showMessage(this,"ssid cannot be empty!");
        return;
    }
    if(ui->pawd->text() == QString(""))
    {
        myprocess->start(tr("/usr/bin/cmddemo_wifi.sh -i %1 -s %2 -p %3")
                         .arg(ui->comboBox->currentText())
                         .arg(ui->ssid->text())
                         .arg("NONE"));
    }
    else
    {
        myprocess->start(tr("/usr/bin/cmddemo_wifi.sh -i %1 -s %2 -p %3")
                         .arg(ui->comboBox->currentText())
                         .arg(ui->ssid->text())
                         .arg(ui->pawd->text()));
    }
}

void WifiWidget::on_status_clicked()
{
    ui->result->clear();
    myprocess->start(tr("wpa_cli -i%1 status").arg(ui->comboBox->currentText()));
}

void WifiWidget::on_ping_clicked()
{
    ui->result->clear();
    if(ui->hostip->text() == QString(""))
    {
        MessageBox::showMessage(this,"hostname cannot be empty!");
        return;
    }

    myprocess->start(tr("ping %1 -w 5").arg(ui->hostip->text()));
    ui->result->append(QString("ping ")+ui->hostip->text());
}
