#ifndef TITLEBAR_H
#define TITLEBAR_H

#include <QWidget>
#include <QLabel>

class TitleBarPrivate;
class TitleBar : public QWidget
{
    Q_OBJECT
public:
    explicit TitleBar(const QString &title, const QString &version, QWidget *parent = nullptr);

    explicit TitleBar(const QString &title=QString(), QWidget *parent = nullptr);

    ~TitleBar();

    virtual void setText(const QString &title);

Q_SIGNALS:
    void quit();

private:
    TitleBarPrivate *m_d;
};

#endif // TITLEBAR_H
