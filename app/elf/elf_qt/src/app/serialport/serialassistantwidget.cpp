#include "serialassistantwidget.h"
#include <QLabel>
#include "serialport.h"
#include <QSerialPortInfo>
#include <QDebug>
#include <QLineEdit>
#include <QTimer>
#include <QFileDialog>
#include <QIntValidator>
#include "ui_serialassistantwidget.h"
#include "messagebox.h"
#include <QStackedWidget>
#include "serialport.h"
#include <QListView>

SerialAssistantWidget::SerialAssistantWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SerialAssistantWidget),
    m_timer(new QTimer(this))
{
    ui->setupUi(this);

    m_serial = new  SerialPort(this);

    //准备度读串口信息.
    connect(m_serial, &SerialPort::readyRead, this, [=](){
        ui->textBrowser->insertPlainText(m_serial->readAll());
        ui->textBrowser->moveCursor(QTextCursor::End);
    });

    //串口错误.
    connect(m_serial, static_cast<void(QSerialPort::*)(QSerialPort::SerialPortError serialPortError)>(&QSerialPort::error), this,
            [=](QSerialPort::SerialPortError serialPortError){
        Q_UNUSED(serialPortError)
        ui->textBrowser->insertPlainText(m_serial->errorString());
        ui->textBrowser->moveCursor(QTextCursor::End);
    });

    //保存串口信息.

    ui->dataBitsBox->addItem("Data8", QVariant(QSerialPort::Data8));
    ui->dataBitsBox->addItem("Data5", QVariant(QSerialPort::Data5));
    ui->dataBitsBox->addItem("Data6", QVariant(QSerialPort::Data6));
    ui->dataBitsBox->addItem("Data7", QVariant(QSerialPort::Data7));

    ui->baudRateBox->addItem("Baud115200", QVariant(QSerialPort::Baud115200));
    ui->baudRateBox->addItem("Baud1200", QVariant(QSerialPort::Baud1200));
    ui->baudRateBox->addItem("Baud2400", QVariant(QSerialPort::Baud2400));
    ui->baudRateBox->addItem("Baud4800", QVariant(QSerialPort::Baud4800));
    ui->baudRateBox->addItem("Baud9600", QVariant(QSerialPort::Baud9600));
    ui->baudRateBox->addItem("Baud19200", QVariant(QSerialPort::Baud19200));
    ui->baudRateBox->addItem("Baud38400", QVariant(QSerialPort::Baud38400));
    ui->baudRateBox->addItem("Baud57600", QVariant(QSerialPort::Baud57600));

    ui->parityBox->addItem("NoParity", QVariant(QSerialPort::NoParity));
    ui->parityBox->addItem("EvenParity", QVariant(QSerialPort::EvenParity));
    ui->parityBox->addItem("OddParity", QVariant(QSerialPort::OddParity));
    ui->parityBox->addItem("SpaceParity", QVariant(QSerialPort::SpaceParity));
    ui->parityBox->addItem("MarkParity", QVariant(QSerialPort::MarkParity));

    ui->stopBitsBox->addItem("OneStop", QVariant(QSerialPort::OneStop));
    ui->stopBitsBox->addItem("OneAndHalfStop", QVariant(QSerialPort::OneAndHalfStop));
    ui->stopBitsBox->addItem("TwoStop", QVariant(QSerialPort::TwoStop));

    ui->flowControlBox->addItem("NoFlowControl", QVariant(QSerialPort::NoFlowControl));
    ui->flowControlBox->addItem("HardwareControl", QVariant(QSerialPort::HardwareControl));
    ui->flowControlBox->addItem("SoftwareControl", QVariant(QSerialPort::SoftwareControl));
    ui->openBtn->setCheckable(true);

    //定时发送命令到串口.
    connect(m_timer, &QTimer::timeout, this, [=](){
        QByteArray  a = ui->sendEdt->text().toLocal8Bit();
        if(ui->hexcheckBox->isChecked()){
            a =QByteArray::fromHex(a);
        }
        m_serial->writeDatas(a+"\r");
    });

    //定时器启动或者停止.
    connect(ui->timerCheckBox, &QCheckBox::clicked, this, [=](bool checked){
        if(!checked && m_timer->isActive()){
            m_timer->stop();
        }
    });

    QIntValidator *valid = new QIntValidator(10, 5000);
    ui->timeEdt->setValidator(valid);

    //获取所有可用的串口.
    foreach (QSerialPortInfo info, QSerialPortInfo::availablePorts()) {
        ui->serialPortInfoListBox->addItem(info.portName());
    }
}

SerialAssistantWidget::~SerialAssistantWidget()
{
    m_timer->stop();
    delete m_timer;
    m_serial->close();
    delete m_serial;
    delete ui;
}

void SerialAssistantWidget::setTabWidget(QWidget *tab)
{
    if(ui->switchWidget->layout())
        ui->switchWidget->layout()->addWidget(tab);
}

void SerialAssistantWidget::resetTr()
{
    ui->openBtn->isChecked()? ui->openBtn->setText(tr("close")): ui->openBtn->setText(tr("open"));
}

void SerialAssistantWidget::on_openBtn_clicked()
{
    if(ui->openBtn->isChecked()){
        ui->openBtn->setText(tr("close"));
        ui->textBrowser->clear();
        ui->sendEdt->clear();
        QVariantList arg;
        arg <<ui->serialPortInfoListBox->currentText()
           << ui->baudRateBox->currentData().toInt()
           << ui->dataBitsBox->currentData().toInt()
           <<ui->stopBitsBox->currentData().toInt()
          << ui->parityBox->currentData().toInt()
          <<ui->flowControlBox->currentData().toInt();
        m_serial->config(arg);

    }else{
        if(m_timer->isActive()){
            m_timer->stop();
        }
        m_serial->close();
        ui->openBtn->setText(tr("open"));
    }
}

void SerialAssistantWidget::on_clearTXBtn_clicked()
{
    ui->textBrowser->clear();
}

void SerialAssistantWidget::on_sendBtn_clicked()
{
    if(!ui->openBtn->isChecked())
    {
        MessageBox::showMessage(this , tr("please open serial"));
        return;
    }

    if(!ui->timerCheckBox->isChecked())
    {
        m_timer->setSingleShot(true);
        m_timer->start(0);
    }else{
        m_timer->setSingleShot(false);
        int mSeconds= ui->timeEdt->text().toInt();
        if(mSeconds <10){
            mSeconds=10;
        }else if(mSeconds >5000){
            mSeconds=5000;
        }
        m_timer->start(mSeconds);
    }
}

void SerialAssistantWidget::on_sendFileBtn_clicked()
{
    if(!ui->openBtn->isChecked()){
        MessageBox::showMessage(this , tr("please open serial"));
        return;
    }

    QString fileName = QFileDialog::getOpenFileName();
    QFile file(fileName);
    if(!fileName.isEmpty() && file.open(QFile::ReadOnly | QFile::Text))
    {
        QTextStream s(&file);
        QString line;
        while (s.readLineInto(&line)) {
            QByteArray a = line.toUtf8() +"\n";
            if(ui->hexcheckBox->isChecked()){
                a =QByteArray::fromHex(a);
            }
            m_serial->write(a);
        }
    }
}

MultiSerialWidget::MultiSerialWidget(QWidget *parent):QWidget(parent)
{
    m_statckWidget = new QStackedWidget;

    QHBoxLayout *hLayout = new QHBoxLayout(this);
    hLayout->setContentsMargins(0,0,0,0);
    hLayout->addWidget(m_statckWidget);

    m_pTabBar = new QTabBar;
    m_pTabBar->addTab(tr("Serail 1"));
    m_pTabBar->addTab(tr("Serial 2"));
    m_pTabBar->addTab(tr("Serial 3"));

    connect(m_pTabBar, &QTabBar::tabBarClicked, this, [=](int index){
        int currentIndex = index;
        if(currentIndex > m_statckWidget->count()-1){
            SerialAssistantWidget * assiant = new SerialAssistantWidget;
            currentIndex = m_statckWidget->addWidget(assiant);
        }
        m_statckWidget->setCurrentIndex(currentIndex);
        SerialAssistantWidget *assiant = dynamic_cast<SerialAssistantWidget*>(m_statckWidget->currentWidget()) ;
        assiant->setTabWidget(m_pTabBar);
    });

    m_pTabBar->tabBarClicked(0);
}

void MultiSerialWidget::resetTr()
{
    m_pTabBar->setTabText(0, tr("Serail 1"));
    m_pTabBar->setTabText(1, tr("Serial 2"));
    m_pTabBar->setTabText(2, tr("Serial 3"));

    for (int i=0; i< m_statckWidget->count(); i++) {
        WidgetControl *control = dynamic_cast<WidgetControl*>(m_statckWidget->widget(i));
        if(control)
            control->resetTr();
    }
}
