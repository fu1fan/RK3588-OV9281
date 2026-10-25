#include "ubootmenuwidget.h"
#include "ui_ubootmenuwidget.h"
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include <QMessageBox>
#include <unistd.h>
#include <QDir>
#include <QDebug>

UbootMenuWidget::UbootMenuWidget(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::UbootMenuWidget)
{
    
    ui->setupUi(this);

	qExec.start("/usr/sbin/fw_printenv video_mipi_edp");
    qExec.waitForFinished();
    QString result = qExec.readAllStandardOutput();
    char *p = result.toLatin1().data();
	if (strstr(p + strlen("video_mipi_edp"), "mipi") != NULL) {
		ui->video_mipi->setText(tr("video-mipi:on"));
		QPalette   pal;
		pal.setColor(QPalette::Button, Qt::green);
		ui->video_mipi->setPalette(pal);
		
		ui->video_edp->setText(tr("video-edp:off"));
		pal.setColor(QPalette::Button, Qt::red);
		ui->video_edp->setPalette(pal);
	} else if (strstr(p + strlen("video_mipi_edp"), "edp") != NULL) {
		ui->video_edp->setText(tr("video-edp:on"));
		QPalette   pal;
		pal.setColor(QPalette::Button, Qt::green);
		ui->video_edp->setPalette(pal);

		ui->video_mipi->setText(tr("video-mipi:off"));
		pal.setColor(QPalette::Button, Qt::red);
		ui->video_mipi->setPalette(pal);
	} else {
		QPalette   pal;
		ui->video_edp->setText(tr("video-edp:off"));
		pal.setColor(QPalette::Button, Qt::red);
		ui->video_edp->setPalette(pal);

		ui->video_mipi->setText(tr("video-mipi:off"));
		pal.setColor(QPalette::Button, Qt::red);
		ui->video_mipi->setPalette(pal);
	}

	qExec.start("/usr/sbin/fw_printenv video_hdmi");
    qExec.waitForFinished();
    result = qExec.readAllStandardOutput();
    p = result.toLatin1().data();
	if (strstr(p + strlen("video_hdmi"), "hdmi") != NULL) {
		ui->video_hdmi->setText(tr("video-hdmi:on"));
		QPalette   pal;
		pal.setColor(QPalette::Button, Qt::green);
		ui->video_hdmi->setPalette(pal);
	} else {
		ui->video_hdmi->setText(tr("video-hdmi:off"));
		QPalette   pal;
		pal.setColor(QPalette::Button, Qt::red);
		ui->video_hdmi->setPalette(pal);
	}

	qExec.start("/usr/sbin/fw_printenv video_lvds");
    qExec.waitForFinished();
    result = qExec.readAllStandardOutput();
    p = result.toLatin1().data();
	if (strstr(p + strlen("video_lvds"), "lvds") != NULL) {
		ui->video_lvds->setText(tr("video-lvds:on"));
		QPalette   pal;
		pal.setColor(QPalette::Button, Qt::green);
		ui->video_lvds->setPalette(pal);
	} else {
		ui->video_lvds->setText(tr("video-lvds:off"));
		QPalette   pal;
		pal.setColor(QPalette::Button, Qt::red);
		ui->video_lvds->setPalette(pal);
	}

    connect(ui->video_mipi, &QPushButton::clicked, this, &UbootMenuWidget::video_mipi_clicked);
    connect(ui->video_hdmi, &QPushButton::clicked, this, &UbootMenuWidget::video_hdmi_clicked);
    connect(ui->video_lvds, &QPushButton::clicked, this, &UbootMenuWidget::video_lvds_clicked);
    connect(ui->video_edp, &QPushButton::clicked, this, &UbootMenuWidget::video_edp_clicked);
}

UbootMenuWidget::~UbootMenuWidget()
{
    delete ui;
}

void UbootMenuWidget::video_mipi_clicked()
{
	if (ui->video_mipi->text() == "video-mipi:on") {
		ui->video_mipi->setText(tr("video-mipi:off"));
		QPalette   pal;
		pal.setColor(QPalette::Button, Qt::red);
		ui->video_mipi->setPalette(pal);
		system("fw_setenv video_mipi_edp none");
	} else {
		ui->video_mipi->setText(tr("video-mipi:on"));
		QPalette   pal;
		pal.setColor(QPalette::Button, Qt::green);
		ui->video_mipi->setPalette(pal);
		system("fw_setenv video_mipi_edp mipi");

		ui->video_edp->setText(tr("video-edp:off"));
		pal.setColor(QPalette::Button, Qt::red);
		ui->video_edp->setPalette(pal);
	}
}

void UbootMenuWidget::video_lvds_clicked()
{
	if (ui->video_lvds->text() == "video-lvds:on") {
		ui->video_lvds->setText(tr("video-lvds:off"));
		QPalette   pal;
		pal.setColor(QPalette::Button, Qt::red);
		ui->video_lvds->setPalette(pal);
		system("fw_setenv video_lvds off");
	} else {
		ui->video_lvds->setText(tr("video-lvds:on"));
		QPalette   pal;
		pal.setColor(QPalette::Button, Qt::green);
		ui->video_lvds->setPalette(pal);
		system("fw_setenv video_lvds lvds");
	}
}

void UbootMenuWidget::video_edp_clicked()
{
	if (ui->video_edp->text() == "video-edp:on") {
		ui->video_edp->setText(tr("video-edp:off"));
		QPalette   pal;
		pal.setColor(QPalette::Button, Qt::red);
		ui->video_edp->setPalette(pal);
		system("fw_setenv video_mipi_edp none");
	} else {
		ui->video_edp->setText(tr("video-edp:on"));
		QPalette   pal;
		pal.setColor(QPalette::Button, Qt::green);
		ui->video_edp->setPalette(pal);
		system("fw_setenv video_mipi_edp edp");

		ui->video_mipi->setText(tr("video-mipi:off"));
		pal.setColor(QPalette::Button, Qt::red);
		ui->video_mipi->setPalette(pal);
	}
}

void UbootMenuWidget::video_hdmi_clicked()
{
	if (ui->video_hdmi->text() == "video-hdmi:on") {
		ui->video_hdmi->setText(tr("video-hdmi:off"));
		QPalette   pal;
		pal.setColor(QPalette::Button, Qt::red);
		ui->video_hdmi->setPalette(pal);
		system("fw_setenv video_hdmi off");
	} else {
		ui->video_hdmi->setText(tr("video-hdmi:on"));
		QPalette   pal;
		pal.setColor(QPalette::Button, Qt::green);
		ui->video_hdmi->setPalette(pal);
		system("fw_setenv video_hdmi hdmi");
	}
}


