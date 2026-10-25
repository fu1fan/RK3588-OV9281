#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDesktopWidget>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
	setWindowState(Qt::WindowMaximized);
	setWindowFlags(Qt::FramelessWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_pushButton_clicked()
{

#if 1
	 QDesktopWidget *desktop = QApplication::desktop();
    int screencount = desktop->screenCount();
    int primaryscreen = desktop->primaryScreen();
    qDebug() << "screencount " << screencount << " primaryscreen " << primaryscreen;
    int offset_width = 0;

    for (int i = 0; i < screencount; i++) {
        if (i == primaryscreen) {
			offset_width += desktop->screenGeometry(i).width();
			continue;
        } else {
			QWidget * w = new QWidget();
			w->setWindowModality(Qt::NonModal);
			w->setMinimumSize(600,400);
			QPushButton *btn = new QPushButton(w);
			btn->move(300,200);
			btn->setText("Exit");
			connect(btn, &QPushButton::clicked, this, [=](){
					w->close();
					});
			w->move(offset_width, 0);
			w->show();
        }
        offset_width += desktop->screenGeometry(i).width();
		qDebug() << "resolution: " << desktop->screenGeometry(i).size();
    }
#endif
}

void MainWindow::on_pushButton_2_clicked()
{
    close();
}
