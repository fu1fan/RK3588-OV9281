#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include <QMessageBox>
#include <unistd.h>
#include <QDir>
#include <QDebug>
#include <string.h>


static char vp01_name[][30] = {"HDMI0", "HDMI1", "EDP0", "EDP1", "DP0", "DP1", "OFF"};	//7
static char vp2_name[][30] = {"HDMI0", "HDMI1", "EDP0", "EDP1", "DP0", "DP1", "MIPI0", "MIPI1", "OFF"}; // 9
static char vp3_name[][30] = { "MIPI0", "MIPI1", "RGB", "OFF"}; //4

#define		ARRARY_SIZE(name) (sizeof(name) / sizeof(name[0]))

int ncmp(char *p, char vp_name[][30], int size)
{
	for (int i = 0; i < size; i++) {
		if (strcmp(p, vp_name[i]) == 0)
			return i;
	}
	return size - 1;
}

int MainWindow::get_primary_index(char *p)
{
	char type[5][30] = {"", "", "", "", "OFF"};
	strcpy(type[0], vp01_name[vp0_index]);
	strcpy(type[1], vp01_name[vp1_index]);
	strcpy(type[2], vp2_name[vp2_index]);
	strcpy(type[3], vp3_name[vp3_index]);
	
	for (int i = 0; i < 5; i++) {
		qDebug() << type[i];
		if (strcmp(type[i], p) == 0)
		{
			return i;
		}
	}
	
	return 4;
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    
    ui->setupUi(this);
	setWindowState(Qt::WindowMaximized);
	setWindowFlags(Qt::FramelessWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);

	ui->dispinfo->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter | Qt::AlignCenter);
	ui->dispinfo->setText("hdmi0 and edp0 share same port, only one can be used.\n"
			"hdmi1 and edp1 share same port, only one can be used.\n"
			"only four VPs internally, so up to four interfaces can be activated\n"
			"hdmi edp dp can only be displayed on VP0 or VP1 or VP2.\n"
			"dsi0 dsi1 can only be displayed on VP2 or  VP3.\n"
			"rgb can only be displayed on VP3.\n");
	QFont ft;
	ft.setPointSize(16);
	ui->dispinfo->setStyleSheet("color:red");
	ui->dispinfo->setFont(ft);
	ui->dispinfo->setWordWrap(true);
	
	for (int i = 0; i < 4; i++) {
		char buffer[100];
		sprintf(buffer, "/usr/sbin/fw_printenv video_vp%d", i);
		qExec.start(buffer);
		qExec.waitForFinished();
		QString result = qExec.readAllStandardOutput();
		if (result.size() > 0) {
			sprintf(buffer, "%s", result.toLatin1().data());
			buffer[result.toLatin1().size() - 1] = '\0';
			switch (i) {
				case 0:
					ui->video_vp0->setText(buffer);
					{
						char *p = strstr(buffer, "=");
						vp0_index = ncmp(p + 1, vp01_name, ARRARY_SIZE(vp01_name));
					}
					break;
				case 1:
					ui->video_vp1->setText(buffer);
					{
						char *p = strstr(buffer, "=");
						vp1_index = ncmp(p + 1, vp01_name, ARRARY_SIZE(vp01_name));
					}

					break;
				case 2:
					ui->video_vp2->setText(buffer);
					{
						char *p = strstr(buffer, "=");
						vp2_index = ncmp(p + 1, vp2_name, ARRARY_SIZE(vp2_name));
					}
					break;
				case 3:
					ui->video_vp3->setText(buffer);
					{
						char *p = strstr(buffer, "=");
						vp3_index = ncmp(p + 1, vp3_name, ARRARY_SIZE(vp3_name));
					}
					break;
			}
		} else {
			switch (i) {
				case 0:
					ui->video_vp0->setText("video_vp0=OFF");
					vp0_index = 6;
					break;
				case 1:
					ui->video_vp1->setText("video_vp1=OFF");
					vp1_index = 6;
					break;
				case 2:
					ui->video_vp2->setText("video_vp2=OFF");
					vp2_index = 8;
					break;
				case 3:
					ui->video_vp3->setText("video_vp3=OFF");
					vp3_index = 3;
					break;
			}
		}
	}

	qDebug() << "vp0_index: "<< vp0_index << "  vp1_index: " <<vp1_index << " vp2_index: " << vp2_index << " vp3_index: " << vp3_index;

	primary_index = 0;
	qExec.start("/usr/sbin/fw_printenv primary_display");
	qExec.waitForFinished();
	QString result = qExec.readAllStandardOutput();
	if (result.size() > 0) {
		char buffer[100];
		sprintf(buffer, "%s", result.toLatin1().data());
		buffer[result.toLatin1().size() - 1] = '\0';
		char *p = strstr(buffer, "=");
		if (p != NULL) {
			primary_index = get_primary_index(p + 1);
		}
		ui->primary_display->setText(buffer);
	} else {
		ui->primary_display->setText("primary_display=OFF");
	}
	
	qExec.start("/usr/sbin/fw_printenv disp_type");
	qExec.waitForFinished();
	result = qExec.readAllStandardOutput();
	char *p = result.toLatin1().data();
	ui->disp_type->setText(p);

	connect(ui->video_vp0, &QPushButton::clicked, this, &MainWindow::video_vp0_clicked);
	connect(ui->video_vp1, &QPushButton::clicked, this, &MainWindow::video_vp1_clicked);
	connect(ui->video_vp2, &QPushButton::clicked, this, &MainWindow::video_vp2_clicked);
	connect(ui->video_vp3, &QPushButton::clicked, this, &MainWindow::video_vp3_clicked);
	connect(ui->disp_type, &QPushButton::clicked, this, &MainWindow::video_disp_type_clicked);
	connect(ui->primary_display, &QPushButton::clicked, this, &MainWindow::video_primary_display_clicked);
	

	connect(ui->exitBtn, &QPushButton::clicked, this, [=](){
			close();
			});
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::video_vp0_clicked()
{
	char buf[200];

	change_primary_display(vp01_name[vp0_index]);
	vp0_index = (vp0_index + 1) % 7;

	if (vp0_index < 4) {
		for (int i = 0; i < 4; i++) {
			if ((vp1_index < 4) && ((vp0_index == vp1_index) || ((vp0_index % 2) == (vp1_index % 2)))) {
				vp0_index = (vp0_index + 1) % 7;
				continue;
			}

			if ((vp2_index < 4) && ((vp0_index == vp2_index) || ((vp0_index % 2) == (vp2_index % 2)))) {
				vp0_index = (vp0_index + 1) % 7;
				continue;
			}
		}
	}

	if ( vp0_index >= 4 && vp0_index != 6) {
			for (int i = 0; i < 2; i++) {
			if ((vp1_index != 6) && (vp0_index == vp1_index)) {
				vp0_index = (vp0_index + 1) % 7;
				continue;
			}

			if ((vp2_index != 8) && (vp0_index == vp2_index)) {
				vp0_index = (vp0_index + 1) % 7;
				continue;
			}
		}	
	}

	if (vp0_index == 6) 
		change_primary_display(vp01_name[vp0_index]);

	sprintf(buf, "/usr/sbin/fw_setenv video_vp0 %s", vp01_name[vp0_index]);
	system(buf);
	sprintf(buf, "video_vp0=%s", vp01_name[vp0_index]);
	ui->video_vp0->setText(buf);
}

void MainWindow::video_vp1_clicked()
{
	char buf[200];
	change_primary_display(vp01_name[vp1_index]);
	vp1_index = (vp1_index + 1) % 7;

	if (vp1_index < 4) {
		for (int i = 0; i < 4; i++) {
			if ((vp0_index < 4) && ((vp1_index == vp0_index) || ((vp1_index % 2) == (vp0_index % 2)))) {
				vp1_index = (vp1_index + 1) % 7;
				continue;
			}

			if ((vp2_index < 4) && ((vp1_index == vp2_index) || ((vp1_index % 2) == (vp2_index % 2)))) {
				vp1_index = (vp1_index + 1) % 7;
				continue;
			}
		}
	} 
	if ( vp1_index >= 4 && vp1_index != 6) {
			for (int i = 0; i < 2; i++) {
			if ((vp0_index != 6) && (vp1_index == vp0_index)) {
				vp1_index = (vp1_index + 1) % 7;
				continue;
			}

			if ((vp2_index != 8) && (vp1_index == vp2_index)) {
				vp1_index = (vp1_index + 1) % 7;
				continue;
			}
		}	
	}


	if (vp1_index == 6)
		change_primary_display(vp01_name[vp1_index]);

	sprintf(buf, "/usr/sbin/fw_setenv video_vp1 %s", vp01_name[vp1_index]);
	system(buf);
	sprintf(buf, "video_vp1=%s", vp01_name[vp1_index]);
	ui->video_vp1->setText(buf);
}

void MainWindow::video_vp2_clicked()
{
	char buf[200];
	change_primary_display(vp2_name[vp2_index]);
	vp2_index = (vp2_index + 1) % 9;

	if (vp2_index < 4) {
		for (int i = 0; i < 4; i++) {
			if ((vp0_index < 4) && ((vp2_index == vp0_index) || ((vp2_index % 2) == (vp0_index % 2)))) {
				vp2_index = (vp2_index + 1) % 9;
				continue;
			}

			if ((vp1_index < 4) && ((vp2_index == vp1_index) || ((vp2_index % 2) == (vp1_index % 2)))) {
				vp2_index = (vp2_index + 1) % 9;
				continue;
			}
		}
	} 
	if ( vp2_index >= 4 && vp2_index < 6) {	// dp0 dp1
		for (int i = 0; i < 2; i++) {
			if ((vp0_index != 6) && (vp2_index == vp0_index)) {
				vp2_index = (vp2_index + 1) % 9;
				continue;
			}

			if ((vp1_index != 6) && (vp2_index == vp1_index)) {
				vp2_index = (vp2_index + 1) % 9;
				continue;
			}
		}	
	}

	if (vp2_index >= 6 && vp2_index != 8) {
		for (int i = 0; i < 2; i++) {
			if (vp3_index < 2 && (vp2_index % 3 == vp3_index)) {	//mipi0 mipi1
				vp2_index = (vp2_index + 1) % 9;
			}
		}	
	}

	
	if (vp2_index == 8)
		change_primary_display(vp2_name[vp2_index]);

	sprintf(buf, "/usr/sbin/fw_setenv video_vp2 %s", vp2_name[vp2_index]);
	system(buf);
	sprintf(buf, "video_vp2=%s", vp2_name[vp2_index]);
	ui->video_vp2->setText(buf);
}

void MainWindow::video_vp3_clicked()
{
	char buf[200];

	change_primary_display(vp3_name[vp3_index]);
	vp3_index = (vp3_index + 1) % 4;

	if (vp3_index < 2) {
		for (int i = 0; i < 2; i++) {
			if ((vp2_index >= 6 && vp2_index < 8) && (vp2_index % 3 == vp3_index)) {	// mipi0 mipi1
				vp3_index = (vp3_index + 1) % 4;
			}
		}
	}

	if (vp3_index == 3)
		change_primary_display(vp3_name[vp3_index]);

	sprintf(buf, "/usr/sbin/fw_setenv video_vp3 %s", vp3_name[vp3_index]);
	system(buf);
	sprintf(buf, "video_vp3=%s", vp3_name[vp3_index]);
	ui->video_vp3->setText(buf);
}

void MainWindow::video_disp_type_clicked()
{
	if (ui->disp_type->text() == "disp_type=sync") {
		ui->disp_type->setText(tr("disp_type=async"));
		system("/usr/sbin/fw_setenv disp_type async");
	} else {
		ui->disp_type->setText(tr("disp_type=sync"));
		system("/usr/sbin/fw_setenv disp_type sync ");
	}
}

void MainWindow::video_primary_display_clicked()
{
	char buffer[200];
	char type[5][30] = {"", "", "", "", "OFF"};
	strcpy(type[0], vp01_name[vp0_index]);
	strcpy(type[1], vp01_name[vp1_index]);
	strcpy(type[2], vp2_name[vp2_index]);
	strcpy(type[3], vp3_name[vp3_index]);

	qDebug() << "primary_index: "<< primary_index;
	
	primary_index = (primary_index + 1) % 5;
	for (int i = 0; i < 5; i++) {
		qDebug() << "type: " << type[primary_index];
		if (::strcmp(type[primary_index], "OFF") == 0) {
			primary_index = (primary_index + 1) % 5;
			continue;
		} 
		sprintf(buffer, "primary_display=%s", type[primary_index]);
		ui->primary_display->setText(buffer);

		sprintf(buffer, "/usr/sbin/fw_setenv primary_display %s", type[primary_index]);
		system(buffer);
		return ;
	}
	ui->primary_display->setText("primary_display=OFF");
	system("/usr/sbin/fw_setenv primary_display OFF");
}

void MainWindow::change_primary_display(char *p)
{
	int len = 0;
	char buff[100];
	sprintf(buff, "primary_display=%s", p);
	len = strlen(buff);
	if (buff[len - 1] == '\n')
		buff[strlen(buff) - 1] = '\0';
	if (strcmp(ui->primary_display->text().toLatin1().data(), buff) == 0)
	{
		ui->primary_display->setText("primary_display=OFF");
		system("/usr/sbin/fw_setenv primary_display OFF");
		primary_index = 4;
	}
}
