#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QComboBox>
#include <QProcess>
#include <QPushButton>


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    
private slots:
	void video_vp0_clicked();
	void video_vp1_clicked();
	void video_vp2_clicked();
	void video_vp3_clicked();
	void video_primary_display_clicked();
	void video_disp_type_clicked();

private:
    Ui::MainWindow *ui;
	QProcess      qExec;
	int vp0_index;
	int vp1_index;
	int vp2_index;
	int vp3_index;
	int primary_index;

private:
	int get_primary_index(char *);
	void change_primary_display(char *p);
};

#endif // MAINWINDOW_H
