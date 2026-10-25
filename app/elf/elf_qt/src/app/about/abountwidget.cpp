#include "abountwidget.h"
#include <QCoreApplication>
#include <QDir>
#include <QTimer>
#include <QPainter>
#include <QCoreApplication>
#include <QTimer>
#include "terminal.h"
#include "ui_about.h"
using namespace fl_common;
AbountWidget::AbountWidget(QWidget *parent) : QWidget(parent),
    ui(new Ui::AboutWidget)
{
    ui->setupUi(this);
    ui->textBrowser->append("loading ...");
    QFont f= ui->textBrowser->font();
    f.setPixelSize(20);
    ui->textBrowser->setFont(f);

    m_fetcher = new FetcherSystem;

    connect(m_fetcher, &FetcherSystem::infoUpdate, this, [=](
            const QString &storage,
            const QString &memTotal,
            const QString &memFree,
            const QString &memAvailable,
            const QString &kerVersion,
            const QString &userCpu){


        ui->textBrowser->clear();
        ui->textBrowser->append(QString("[Kernal Version]: %1").arg(kerVersion.trimmed()));
        ui->textBrowser->append("\n");
        ui->textBrowser->append(QString("[Cpu]: %1").arg(userCpu.trimmed()));
        ui->textBrowser->append("\n");
        ui->textBrowser->append(QString("[MemTotal]: %1 M  [MemFree] %2 M  [MemAvailable]%3 M")
                                .arg(memTotal)
                                .arg(memFree)
                                .arg(memAvailable)
                                );
        ui->textBrowser->append("\n");
        ui->textBrowser->append(storage);
        m_fetcher->start();
    });

    m_fetcher->setStart(true);
}

AbountWidget::~AbountWidget()
{
    m_fetcher->setStart(false);
    delete ui;
}

FetcherSystem::FetcherSystem(QObject *parent):QThread(parent)
{
}

void FetcherSystem::setStart(bool bStart)
{
    m_bQuit = !bStart;
    if(m_bQuit){
        this->terminate();
        this->wait(1000);
    }else{
        start();
    }
}


void FetcherSystem::run()
{
    while (!m_bQuit) {

        QString storage;
        QString memTotal;
        QString memFree;
        QString memAvailable;
        QString kerVersion;
        QString userCpu;
        //获取CPU 信息.
        userCpu = Terminal::execCmd("top -b -n 1 | grep Cpu");

        //获取内存信息.
        storage = Terminal::execCmd("df -h");

        //获取内核信息.
        kerVersion = Terminal::execCmd("uname -a");


        //获取内存信息.
        //MemTotal:        3191916 kB
        //MemFree:          470732 kB
        //MemAvailable:     455120 kB
        //Buffers:           15200 kB
        //Cached:           368716 kB
        //SwapCached:        31984 kB
        QString result = Terminal::execCmd("cat /proc/meminfo");
        QStringList list = Terminal::getMatch(result, "\\d+");
        if(list.size()>=3){
            memTotal=list[0];
            memFree=list[1];
            memAvailable=list[2];
            memTotal = QString::number(memTotal.toInt()/1024);
            memFree = QString::number(memFree.toInt()/1024);
            memAvailable = QString::number(memAvailable.toInt()/1024);
        }

        emit infoUpdate(storage, memTotal, memFree, memAvailable,kerVersion, userCpu);
        msleep(1000);
    }

}
