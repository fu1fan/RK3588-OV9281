#ifndef PINBALLWIDGET_H
#define PINBALLWIDGET_H

#include <QWidget>

class TimeCountdownPrivate;
class TimeCountdown  : public QWidget
{
    Q_OBJECT
public:
    explicit TimeCountdown(QWidget *parent = nullptr);
     ~ TimeCountdown();
    void paintEvent(QPaintEvent *e);

    void setValue(const QString & number);
    void setTips(const QString &info);
    void startDownTime(int maxSeconds);
private:
    TimeCountdownPrivate *m_d;
};

#endif // PINBALLWIDGET_H
