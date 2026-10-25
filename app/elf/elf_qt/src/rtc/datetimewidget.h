#ifndef DATETIMEWIDGET_H
#define DATETIMEWIDGET_H

#include <QWidget>
#include "processhandler.h"
#include <QThread>

namespace Ui {
class DateTimeWidget;
}

class QComboBox;
class DateTimeUpdater;
class  DateTimeWidget : public QWidget
{
    Q_OBJECT
public:
    explicit DateTimeWidget(QWidget *parent = nullptr);
    ~DateTimeWidget();

private:
    int currentvalue(QComboBox *d);

private slots:
    void on_applyBtn_clicked();

private:
    Ui::DateTimeWidget *ui;
    DateTimeUpdater *m_checker;
};

class DateTimeUpdater:public QThread PROCESSHANDLER
{
    Q_OBJECT
public:
    DateTimeUpdater(QObject *parent=Q_NULLPTR);
    ~DateTimeUpdater();
    bool mayUpdaeteNpt();

Q_SIGNALS:
    void updateDateTime(const QString &localDateTime, const QString &rtcDateTime);
    void updateRtc(const QDateTime &date);

    void updateNpt();
    void nptDateTime(const QDateTime &date, bool bOk);

protected:
    void run() override;
    void processMessage(const QString &info) override;
    long long NTPGetTimeBeijingMSeconds();
private:
    QString m_info;
    bool m_bCanWriteRtc;
};
#endif // DATETIMEWIDGET_H
