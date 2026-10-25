#include <sys/ioctl.h>
#include <fcntl.h>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QFile>
#include <QFileDialog>
#include <QDateTime>
#include <QDate>
#include <QTime>
#include <iostream>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    playing = false;
    process = NULL;
    process2 = NULL;
    timer = NULL;

    btg = new QButtonGroup;
    btg->addButton(ui->radioMIC,0);
    btg->addButton(ui->radioLINEIN,1);

    ui->slider->setDisabled(true);
    ui->list->addItem(new QListWidgetItem("/elf/audio/test.mp3"));
    ui->list->setCurrentRow(0);
}

MainWindow::~MainWindow()
{
    delete ui;
    delete btg;
    delete process;
    delete process2;
}

void MainWindow::on_play_clicked()
{
    if(ui->list->currentItem() == NULL)
    {
        QMessageBox::about(this,"error","nothing selected!");
        return;
    }

    QString file = ui->list->currentItem()->text();

    QFile f(file);

    if(!f.exists())
    {
        QMessageBox::about(this,"error","the file doesn't exist!");
        return;
    }

    QString cmd = QString("/usr/bin/mplayer ")+file;

    if(timer)delete timer;
    if(process)delete process;

    timer = new QTimer();
    connect(timer,SIGNAL(timeout()),this,SLOT(timerdone()));
    //    timer->start(1000);  //一秒钟后开始触发，然后一秒一次

    process = new QProcess();
    process->setProcessChannelMode(QProcess::MergedChannels);
    connect(process,SIGNAL(readyReadStandardOutput()),this,SLOT(output()));

    process->start(cmd.toLatin1());
    //    process->write(" \n");

    if(!playing)
    {
        ui->pause->setText("pause");
        ui->slider->setDisabled(false);
    }

    playing = true;

    ui->slider->setMinimum(0);
    ui->slider->setValue(0);
    ui->slider->setDisabled(false);
}

void MainWindow::on_stop_clicked()
{
    if(process == NULL)
        return;
    process->write("q \n");
    delete process;
    delete timer;
    process = NULL;
    timer = NULL;
}

void MainWindow::on_add_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("open file"), " ",  tr("Allfile(*.*);;mp3file(*.mp3);;wmafile(*.wma);;wavefile(*.wav)"));
    ui->list->addItem(new QListWidgetItem(fileName));
    ui->list->setCurrentRow(0);
}

void MainWindow::timerdone()
{
    //   if(playing)
    //     process->write(" \n");
}

void MainWindow::output()
{
    QByteArray ba;
    int l,r;
    QString msg(process->readLine());
    QString tmp = msg.mid(0,2);
    if(tmp == "A:"){
        ba = msg.toLatin1();
        QString audioTime = msg.mid(2,6);

        l = msg.indexOf(tr("of "),0);
        r = msg.indexOf(tr(" ("),l);
        QString totalTime = msg.mid(l+3,r - l - 3);;
        bool ok;
        r = (int)totalTime.toDouble(&ok);
        l = (int)audioTime.toDouble(&ok);
        ui->slider->setMaximum(r);
        ui->slider->setValue(l);
        tmp.setNum(r/60,10);
        tmp.append(tr(":"));
        if(r%60 < 10){
            tmp.append(tr("0"));
            tmp.append(QString::number(r%60,10));
        } else
            tmp.append(QString::number(r%60,10));
        ui->length->setText(tmp);

        tmp.setNum(l/60,10);
        tmp.append(tr(":"));
        if(l%60 < 10){
            tmp.append(tr("0"));
            tmp.append(QString::number(l%60,10));
        } else
            tmp.append(QString::number(l%60,10));
        ui->pos->setText(tmp);
    }
}


void MainWindow::on_pause_clicked()
{
    if(process == NULL)
        return;

    if(playing)
    {
        ui->pause->setText("resume");
        ui->slider->setDisabled(true);
    }
    else
    {
        ui->pause->setText("pause");
        ui->slider->setDisabled(false);
    }

    process->write("p");

    playing = !playing;

}

void MainWindow::on_slider_sliderMoved(int position)
{
    Q_UNUSED(position)
    if(process == NULL)
        return;

    //    QString tmp = QString("seek ")+QString::number(ui->slider->value())+QString(" 2 \n");

    //    process->write(tmp.toAscii());
}

void MainWindow::on_del_clicked()
{
    ui->list->removeItemWidget(ui->list->takeItem(ui->list->currentRow()));
}

void MainWindow::on_record_clicked()
{
    QDateTime dt;
    QDate date;
    QTime time;

    if(process2)
    {
        delete process2;
        process2 = NULL;
    }


    if(ui->record->text() == QString("record"))
    {
        ui->record->setText("stop");

        process2 = new QProcess();

        dt.setDate(date.currentDate());
        dt.setTime(time.currentTime());
        filename = dt.toString("yyyyMMddhhmmss")+QString(".wav");

        if(btg->checkedId() == 0)
            process2->start("arecord -t wav -r 44100 -f S16_LE /tmp/"+ filename);
        else
            process2->start("arecord -t wav -c 2 -r 44100 /tmp/"+filename);
    }
    else
    {
        ui->record->setText("record");

        ui->list->addItem(new QListWidgetItem("/tmp/"+filename));
        ui->list->setCurrentRow(ui->list->count()-1);
    }
}


