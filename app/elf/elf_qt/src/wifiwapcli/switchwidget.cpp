#include "switchwidget.h"
#include <QPainter>
#include<QPen>
#include <QMouseEvent>
#include <QDebug>

SwitchWidget::SwitchWidget(QWidget *parent) : QWidget(parent),m_bOpen(false),
    m_ratio(0)
{
    this->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
}

void SwitchWidget::setText(const QString &title)
{
    m_text = title;
}

QString SwitchWidget::text()
{
    return m_text;
}

bool SwitchWidget::isOpen()
{
    return m_bOpen;
}

void SwitchWidget::setTextRatio(qreal ratio)
{
    if(ratio< 0.5)
    {
        m_ratio =0.5;
    }
    if(ratio >0.8)
    {
        m_ratio= 0.8;
    }
}

void SwitchWidget::setOpen(bool bOpen)
{
    m_bOpen =bOpen;
    this->update();
    click(bOpen);
}

void SwitchWidget::paintEvent(QPaintEvent *e)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    QRectF visualRect = this->rect().adjusted(1, 1, -1, -1);
    qreal textWidth = visualRect.width()*m_ratio;
    qreal textHeight = visualRect.height();

    QRectF tectRect = QRectF(visualRect.x(), visualRect.y(), textWidth, textHeight);
    QTextOption option( Qt::AlignVCenter);
    QRectF fixTextRect = painter.boundingRect(tectRect, m_text,option);
    painter.drawText(fixTextRect, m_text);

    QRectF switchRect(visualRect.x() + textWidth,
                     visualRect.y(),
                     visualRect.width()-textWidth,
                     visualRect.height());

    m_rect = switchRect;
    m_bOpen? painter.setBrush(Qt::blue) : painter.setBrush(Qt::gray);
    painter.setPen(Qt::transparent);

    int radius = switchRect.height()/2;
    painter.drawRoundedRect(switchRect, radius, radius);

    QPoint center(switchRect.x() +radius,
                  switchRect.y() +radius);

    if(m_bOpen)
    {
        center.setX(switchRect.x() +switchRect.width()-radius);
    }
    painter.setBrush(Qt::white);
    painter.drawEllipse(center,
                        visualRect.height()/2 -radius/6,
                        visualRect.height()/2 -radius/6);
}

void SwitchWidget::mousePressEvent(QMouseEvent *e)
{
    QWidget::mousePressEvent(e);
    if(m_rect.contains(e->pos()))
    {
        m_bOpen = !m_bOpen;
        this->update();
        emit click(m_bOpen);
    }else{
        qDebug()<<"not button rect";
    }
}

QSize SwitchWidget::sizeHint() const
{
    return QSize(80, 25);
}
