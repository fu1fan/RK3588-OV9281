#include "watchdogwidgt.h"
#include "ui_watchdogwidgt.h"
#include "iwatchdog.h"
#include <QValidator>
#include "messagebox.h"
#include <QTimer>
#include <QDoubleValidator>
#include <QIntValidator>
#include <QCoreApplication>
#include "basefactory.h"
#include <QDebug>
#include <QIntValidator>
#include <QDoubleValidator>
using namespace fl_common;
//显示喂狗次数和倒计时.
class TimeCountdown
{
public:
    TimeCountdown():mShow(nullptr){
        mShow = new QLabel;
        mShow->setAlignment(Qt::AlignCenter);
        mTimer = new QTimer(mShow);
        mTimer->setTimerType(Qt::PreciseTimer);
        QObject::connect(mTimer, &QTimer::timeout, mTimer, [=](){
            this->timeout();
        });
    }
    QWidget *widget(){
        return mShow;
    }

    void setText(const QString &text){
        mText = text;
        if(!mTimer->isActive()){
            this->showFeedCount(mCount);
        }
    }

    void showFeedCount(int count){
        mTimer->stop();
        mCount = count;
        if(mShow->font().pixelSize() != 24){
            QFont font = mShow->font();
            font.setPixelSize(24);
            mShow->setFont(font);
        }
        mShow->setText(mText  + QString::number(mCount));
    }

    void startCountDown(int seconds, int unit){
        mCount = seconds;
        mUnitSecs = unit;
        if(mShow->font().pixelSize() != 30){
            QFont font = mShow->font();
            font.setPixelSize(28);
            mShow->setFont(font);
        }
        mTimer->stop();
        mTimer->start(1000*unit);
        this->timeout();
    }
private:
    void timeout(){
        mShow->setText( mText + QString::number(mCount));
        mCount -=mUnitSecs;
    }

private:
    QString mText;
    QTimer *mTimer = nullptr;
    QLabel *mShow = nullptr;
    int mCount = 0;
    int mUnitSecs;
};

WatchDogWidgt::WatchDogWidgt(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WatchDogWidgt),
    m_timeControl(new TimeCountdown)
{
    ui->setupUi(this);

    ui->checkBox->setEnabled(false);

    QIntValidator *pIntV = new QIntValidator;
    pIntV->setRange(1, 10);
    ui->timeoutEdt->setValidator(pIntV);

    QDoubleValidator *pDoubleV = new QDoubleValidator;
    pDoubleV->setRange(0.1, 10.0);
    ui->feedEdt->setValidator(pDoubleV);

    connect(BaseFactory::instance()->watchdog(), &IWatchDog::feedCount, [=](int count){
        m_timeControl->showFeedCount(count);
    });

    ui->TimeDown->layout()->addWidget(m_timeControl->widget());

    QIntValidator *intValid = new QIntValidator;
    intValid->setRange(0, 100);
    ui->timeoutEdt->setValidator(intValid);
    ui->timeoutEdt->setPlaceholderText("0 ~ 100");
    ui->feedEdt->setValidator(intValid);
    ui->feedEdt->setPlaceholderText("0 ~ 100");
    this->resetTr();

    setInputEable(false);
}

WatchDogWidgt::~WatchDogWidgt()
{
    delete ui;
    delete m_timeControl;
}

void WatchDogWidgt::resetTr()
{
    ui->setBtn->setText(tr("Start"));
    ui->label_feeddog->setText(tr("Feed dog Interval:"));
    ui->label_feedgod_sec->setText(tr("seconds"));
    ui->label_timeout->setText(tr("Timeout"));
    ui->label_timeout_sec->setText(tr("seconds"));
    ui->checkBox->setText(tr("Feed dog"));
    ui->setBtn->setText(tr("Setting"));

    if(ui->checkBox->isChecked()){
        m_timeControl->setText(tr("Number of dogs fed: "));
    }else{
        m_timeControl->setText(tr("Board is about to restart: "));
    }

    this->setInputEable(BaseFactory::instance()->watchdog()->isOpen());

}

void WatchDogWidgt::on_openBtn_clicked()
{
    auto  setCheck = [=](bool bCheck){
        ui->checkBox->setEnabled(bCheck);
        ui->checkBox->blockSignals(true);
        ui->checkBox->setChecked(bCheck);
        ui->checkBox->blockSignals(false);
    };

    if(!BaseFactory::instance()->watchdog()->isOpen())
    {
        if(!BaseFactory::instance()->watchdog()->open()){
            MessageBox::showMessage(nullptr, tr("open falire"), 3);
            return;
        }

        setCheck(true);
        setInputEable(true);

    }else{
        BaseFactory::instance()->watchdog()->close();
        m_timeControl->showFeedCount(0);
        setCheck(false);
        setInputEable(false);
    }

}

void WatchDogWidgt::on_checkBox_clicked()
{
    if(isInputError())
        return;

    resetTr();

    if(!ui->checkBox->isChecked()){
        m_timeControl->startCountDown(ui->timeoutEdt->text().toInt(), ui->feedEdt->text().toInt());
    }else{
        m_timeControl->showFeedCount(0);
    }
    BaseFactory::instance()->watchdog()->setFeedEnable(ui->checkBox->isChecked());

}

void WatchDogWidgt::on_setBtn_clicked()
{
    if(isInputError())
        return;

    BaseFactory::instance()->watchdog()->setFeeddogInterval(ui->feedEdt->text().toInt());
    BaseFactory::instance()->watchdog()->setTimeout(ui->timeoutEdt->text().toInt());

    MessageBox::showMessage(this, "Setting is Ok!", 1);
}

bool WatchDogWidgt::isInputError()
{
    if(ui->timeoutEdt->text().isEmpty() || ui->feedEdt->text().isEmpty()) {
        MessageBox::showMessage(this, tr("Input is Invalid!"));
        return  true;
    }

    if(ui->timeoutEdt->text().toInt() <= ui->feedEdt->text().toInt()){
        MessageBox::showMessage(this, tr("Timeout must greater than 'Feed dog Interval'!"));
        return  true;
    }

    return false;
}

void WatchDogWidgt::setInputEable(bool bEnable)
{
    ui->timeoutEdt->setReadOnly(!bEnable);
    ui->feedEdt->setReadOnly(!bEnable);
    ui->setBtn->setEnabled(bEnable);

    bEnable ?  ui->openBtn->setText(tr("Stop")): ui->openBtn->setText(tr("Start"));
}
