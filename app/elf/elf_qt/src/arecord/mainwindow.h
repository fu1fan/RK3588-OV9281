#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QProcess>
#include <QButtonGroup>

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
    void on_play_clicked();

    void on_stop_clicked();

    void on_add_clicked();

    void timerdone();

    void output();

    void on_pause_clicked();

    void on_slider_sliderMoved(int position);

    void on_del_clicked();

    void on_record_clicked();

private:
    Ui::MainWindow *ui;
    QButtonGroup *btg;
    QTimer *timer;
    QProcess *process;
    QProcess *process2;
    QString  filename;
    bool playing;
};


#endif // MAINWINDOW_H
