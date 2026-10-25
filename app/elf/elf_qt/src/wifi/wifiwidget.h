#ifndef WIFIWIDGET_H
#define WIFIWIDGET_H
#include <QWidget>
#include <QProcess>

namespace Ui {
class WifiWidget;
}

class WifiWidget : public QWidget
{
    Q_OBJECT
    
public:
    explicit WifiWidget(QWidget *parent = nullptr);
    ~WifiWidget();

private slots:
    void result();
    void on_connect_clicked();
    void on_status_clicked();
    void on_ping_clicked();

private:
    Ui::WifiWidget *ui;
    QProcess *myprocess;
protected:
    void closeEvent(QCloseEvent *);

};

#endif
