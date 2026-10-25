#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
	void btn_tftp_status_clicked();
	void on_cb1_stateChanged(int arg1);

    void on_cb2_stateChanged(int arg1);

    void on_cb7_stateChanged(int arg1);

    void on_cb3_stateChanged(int arg1);

    void on_cb4_stateChanged(int arg1);

    void on_cb5_stateChanged(int arg1);

    void on_cb6_stateChanged(int arg1);

private:
    Ui::MainWindow *ui;
    int status = 0;
    unsigned char tftp_status = 0x0;

public:
    void getLocalInfo(void);
};
#endif // MAINWINDOW_H
