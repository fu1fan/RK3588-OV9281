#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtDebug>
#include <net/if.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/utsname.h>
#include <linux/sockios.h>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    getLocalInfo();
	setWindowState(Qt::WindowMaximized);
	setWindowFlags(Qt::FramelessWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
	

	ui->ServerIP->setText("192.168.1.9");

    connect(ui->Exit, &QPushButton::clicked, this, [=](){
        close();
    });

    connect(ui->BtnStat, &QPushButton::clicked, this, &MainWindow::btn_tftp_status_clicked);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::getLocalInfo(void)
{
    struct ifreq buf[16];
    struct ifconf ifc;
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    ifc.ifc_len = sizeof(buf);
    ifc.ifc_buf = (caddr_t)buf;
    if (!ioctl(fd, SIOCGIFCONF, (char *)&ifc))
    {
        for (int i = 0; i < ifc.ifc_len / sizeof(struct ifreq); i++) {

            if (::strcmp(ifc.ifc_req[i].ifr_name, "eth0") ==0 ) {

                //ipaddr
                ioctl(fd, SIOCGIFADDR, (char *)&buf[i]);
                ui->IPAddr->setText((char *)inet_ntoa(((struct sockaddr_in *)&(buf[i].ifr_addr))->sin_addr));
                //gatway
                ioctl(fd, SIOCGIFBRDADDR, &buf[i]);
                ui->Gateway->setText((char *)inet_ntoa(((struct sockaddr_in *)&(buf[i].ifr_broadaddr))->sin_addr));
                //netmask
                ioctl(fd, SIOCGIFNETMASK, &buf[i]);
                ui->Netmask->setText((char *)inet_ntoa(((struct sockaddr_in *)&(buf[i].ifr_netmask))->sin_addr));
                break;
            }
        }
    }
    ::close(fd);
}

void MainWindow::btn_tftp_status_clicked()
{
	char buffer[100];
    status = (status + 1) % 2;
    if (status) {
        ui->BtnStat->setText("Tftp: On");
        sprintf(buffer, "/usr/sbin/fw_setenv elf_tftp on");
		system(buffer);

		sprintf(buffer, "/usr/sbin/fw_setenv elf_ipaddr %s", ui->IPAddr->text().toLatin1().data());
		system(buffer);

		sprintf(buffer, "/usr/sbin/fw_setenv elf_gatewayip %s", ui->Gateway->text().toLatin1().data());
		system(buffer);

		sprintf(buffer, "/usr/sbin/fw_setenv elf_netmask %s", ui->Netmask->text().toLatin1().data());
		system(buffer);

		sprintf(buffer, "/usr/sbin/fw_setenv elf_serverip %s", ui->ServerIP->text().toLatin1().data());
		system(buffer);

		sprintf(buffer, "/usr/sbin/fw_setenv elf_tftp_status %02x", tftp_status);
		system(buffer);

		ui->IPAddr->setEnabled(false);
		ui->Netmask->setEnabled(false);
		ui->Gateway->setEnabled(false);
		ui->ServerIP->setEnabled(false);
		ui->cb1->setEnabled(false);
		ui->cb2->setEnabled(false);
		ui->cb3->setEnabled(false);
		ui->cb4->setEnabled(false);
		ui->cb5->setEnabled(false);
		ui->cb6->setEnabled(false);
		ui->cb7->setEnabled(false);

    } else {
        ui->BtnStat->setText("Tftp: Off");
        sprintf(buffer, "/usr/sbin/fw_setenv elf_tftp off");
		system(buffer);
		
		ui->IPAddr->setEnabled(true);
		ui->Netmask->setEnabled(true);
		ui->Gateway->setEnabled(true);
		ui->ServerIP->setEnabled(true);
		ui->cb1->setEnabled(true);
		ui->cb2->setEnabled(true);
		ui->cb3->setEnabled(true);
		ui->cb4->setEnabled(true);
		ui->cb5->setEnabled(true);
		ui->cb6->setEnabled(true);
		ui->cb7->setEnabled(true);
    }

	system("sync");
}

void MainWindow::on_cb1_stateChanged(int arg1)
{
    if (ui->cb1->isChecked()) {
        tftp_status = tftp_status | (0x01 << 0);
    } else {
        tftp_status = tftp_status & (~(0x01 << 0));
    }
}

void MainWindow::on_cb2_stateChanged(int arg1)
{
    if (ui->cb2->isChecked()) {
        tftp_status = tftp_status | (0x01 << 1);
    } else {
        tftp_status = tftp_status & (~(0x01 << 1));
    }


}

void MainWindow::on_cb7_stateChanged(int arg1)
{

    if (ui->cb7->isChecked()) {
        tftp_status = tftp_status | (0x01 << 6);
    } else {
        tftp_status = tftp_status & (~(0x01 << 6));
    }
}

void MainWindow::on_cb3_stateChanged(int arg1)
{
    if (ui->cb3->isChecked()) {
        tftp_status = tftp_status | (0x01 << 2);
    } else {
        tftp_status = tftp_status & (~(0x01 << 2));
    }



}

void MainWindow::on_cb4_stateChanged(int arg1)
{
    if (ui->cb4->isChecked()) {
        tftp_status = tftp_status | (0x01 << 3);
    } else {
        tftp_status = tftp_status & (~(0x01 << 3));
    }


}

void MainWindow::on_cb5_stateChanged(int arg1)
{
    if (ui->cb5->isChecked()) {
        tftp_status = tftp_status | (0x01 << 4);
    } else {
        tftp_status = tftp_status & (~(0x01 << 4));
    }


}

void MainWindow::on_cb6_stateChanged(int arg1)
{
    if (ui->cb6->isChecked()) {
        tftp_status = tftp_status | (0x01 << 5);
    } else {
        tftp_status = tftp_status & (~(0x01 << 5));
    }
}
