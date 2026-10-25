#ifndef DATETIMEWIDGET_H
#define DATETIMEWIDGET_H

#include <QWidget>
#include "widgetcontrol.h"

namespace Ui {
class DateTimeWidget;
}

class QComboBox;
class NetworkChecker;
class  DateTimeWidget : public QWidget WIDGETCONTROL
{
    Q_OBJECT

public:
    explicit DateTimeWidget(QWidget *parent = nullptr);
    ~DateTimeWidget();

    void initUi();

private:
    long long NTPGetTimeBeijingMSeconds();
    int currentvalue(QComboBox *d);

private slots:
    void on_applyBtn_clicked();

private:
    Ui::DateTimeWidget *ui;
    NetworkChecker *m_checker;
};

class NetworkChecker:public QObject, public fl_common::ProcessHandler
{
    Q_OBJECT
public:
    NetworkChecker(QObject *parent=Q_NULLPTR);
    void processMessage(const QString &info) override;
    bool networkIsOk();

private:
    QString m_info;
    bool m_change;
};

#endif // DATETIMEWIDGET_H
