#include "spiwidget.h"
#include "ui_spiwidget.h"
#include "ispi.h"
#include "basefactory.h"
#include "ispi.h"
#include "messagebox.h"
#include <QDebug>
#include <QDebug>
#include <QButtonGroup>

using namespace fl_common;

SpiWidget::SpiWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SpiWidget)
{
    ui->setupUi(this);

    for(int i=0; i<BaseFactory::instance()->spiCount(); i++)
    {
        ui->spidevlists->addItem( BaseFactory::instance()->spi(i)->name());
    }

    //spi 改变.
    connect(ui->spidevlists, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            [=](int index){
        currentSpiChange(index);
    });

    //spi 发送信息.
    connect(ui->sendBtn, &QPushButton::clicked, this, [=](){
        if(ISpi *spi = BaseFactory::instance()->spi(ui->spidevlists->currentIndex())){
            QString recevie = spi->send(ui->editsend->text());
            ui->editrecv->append(recevie.toUtf8());
        }
    });

    QButtonGroup *CheckBoxGroup = new QButtonGroup(this);
    CheckBoxGroup->setExclusive(false);
    CheckBoxGroup->addButton(ui->loop, 0);
    CheckBoxGroup->addButton(ui->clockphase, 1);
    CheckBoxGroup->addButton(ui->clockpolarity, 2);
    CheckBoxGroup->addButton(ui->lsb, 3);
    CheckBoxGroup->addButton(ui->cshigh,4);
    CheckBoxGroup->addButton(ui->wire,5);
    CheckBoxGroup->addButton(ui->nocs,6);
    CheckBoxGroup->addButton(ui->ready,7);
    connect(CheckBoxGroup, static_cast<void(QButtonGroup::*)(int)>(&QButtonGroup::buttonPressed),
            [=](){
        unsigned char mode = 0;
        if(ui->loop->isChecked())
            mode |= SPI_LOOP;

        if(ui->clockphase->isChecked())
            mode |= SPI_CPHA;

        if(ui->clockpolarity->isChecked())
            mode |= SPI_CPOL;

        if(ui->lsb->isChecked())
            mode |= SPI_LSB_FIRST;

        if(ui->cshigh->isChecked())
            mode |= SPI_CS_HIGH;

        if(ui->wire->isChecked())
            mode |= SPI_3WIRE;

        if(ui->nocs->isChecked())
            mode |= SPI_NO_CS;

        if(ui->ready->isChecked())
            mode |= SPI_READY;

        if(ISpi *spi = BaseFactory::instance()->spi(ui->spidevlists->currentIndex()))
            spi->setmode(mode);
    });

    //打开第一个Spi
    this->currentSpiChange(0);
}

SpiWidget::~SpiWidget()
{
    delete ui;
}

void SpiWidget::resetTr()
{ 
    ui->receiveLbl->setText(tr("Receive:"));
    ui->sendBtn->setText(tr("send"));
    ui->spedevLbl->setText(tr("SPIDEV"));
    ui->delay->setText(tr("delay"));
    ui->speedLbl->setText(tr("speed"));
    ui->loop->setText(tr("loop"));
    ui->clockphase->setText(tr("clockphase"));
    ui->clockpolarity->setText(tr("clock polarity"));
    ui->lsb->setText(tr("lsb"));
    ui->cshigh->setText(tr("cs-high"));
    ui->wire->setText(tr("3wire"));
    ui->nocs->setText(tr("no-cs"));
    ui->ready->setText(tr("read"));

}

void SpiWidget::currentSpiChange(int id)
{
    ISpi *spi = BaseFactory::instance()->spi(id);
    if(spi == nullptr)
        return;

    if(!spi->isOpen()){

        int speedHz = ui->speed->text().toInt();
        int nBit  =ui->bits->text().toInt();
        int delayUsecs = ui->delay->text().toInt();

        qDebug()<<spi->devName() <<"/"<<spi->name();
        if(!spi->open(speedHz, nBit, delayUsecs)){
            ui->spedevLbl->setText(tr("SPIDEV") +"( Error )");
        }
    }
}

void SpiWidget::on_clearBtn_clicked()
{
    ui->editrecv->clear();
}
