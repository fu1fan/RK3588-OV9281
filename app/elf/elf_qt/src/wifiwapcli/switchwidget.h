/*WIFI 开关Widget*/
#ifndef SWITCHWIDGET_H
#define SWITCHWIDGET_H

#include <QWidget>

class SwitchWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SwitchWidget(QWidget *parent = nullptr);
    void setText(const QString &title);
    QString text();
    void setOpen(bool bOpen);
    bool isOpen();
    void setTextRatio(qreal ratio);

Q_SIGNALS:
    void click(bool bOpen);

protected:
    void paintEvent(QPaintEvent *e) override;
    void mousePressEvent(QMouseEvent*e) override;
    QSize sizeHint() const override;

private:
    bool m_bOpen;
    QString m_text;
    QRectF m_rect;
    qreal m_ratio;
};

#endif // SWITCHWIDGET_H
