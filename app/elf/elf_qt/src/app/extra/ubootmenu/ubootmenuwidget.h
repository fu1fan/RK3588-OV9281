#ifndef UBOOTMENUWIDGET_H
#define UBOOTMENUWIDGET_H

#include <QMainWindow>
#include <QComboBox>
#include <QProcess>


namespace Ui {
class UbootMenuWidget;
}

class UbootMenuWidget : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit UbootMenuWidget(QWidget *parent = 0);
    ~UbootMenuWidget();
    
private slots:
	void video_mipi_clicked();
	void video_lvds_clicked();
	void video_edp_clicked();
	void video_hdmi_clicked();

private:
    Ui::UbootMenuWidget *ui;
	QProcess      qExec;
};

#endif
