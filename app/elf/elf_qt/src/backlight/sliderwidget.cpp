#include "sliderwidget.h"
#include <QPainter>
#include <QMouseEvent>
#include <qmath.h>
#include <QDebug>
#include <QDebug>

class SliderWidgetPrivate
{
public:
    int min =0;
    int max=10;
    int value=0;
    qreal mousePress;
    qreal margin=2.0;
    qreal sliderWidth ;
    qreal sliderHeight;
    qreal sliderH;

    qreal blockHeight;
    qreal blockWidth;
    qreal minHeight;
    qreal maxHeight;

    void handBlockShowRange();
    void handValueRang();
    void handleMousepress();
    void handleValue();
};


SliderWidget::SliderWidget(QWidget *parent) : QWidget(parent),
    m_d(new SliderWidgetPrivate)
{
    setMaximumWidth(30);
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
}

SliderWidget::~SliderWidget()
{
    delete m_d;
}

void SliderWidget::setRange(int min, int max)
{
    m_d->max = max;
    m_d->min = min;
}

int SliderWidget::value()
{
    return m_d->value;
}

void SliderWidget::setValue(int value)
{
    if(value >= m_d->min && value <= m_d->max){
        m_d->value =value;
        m_d->handleValue();
        this->update();
    }
}

void SliderWidget::reset()
{
    m_d->value = m_d->min;
    m_d->handleValue();
    this->update();
}

void SliderWidget::mousePressEvent(QMouseEvent *e)
{
    mouseMoveEvent(e);
}

void SliderWidget::mouseMoveEvent(QMouseEvent *e)
{
    m_d->mousePress = e->pos().y();
    m_d->handleMousepress();
    emit valueChanged(m_d->value);
    this->update();
}

void SliderWidget::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e)
    QPainter painter(this);
    QRectF sliderRect(m_d->margin, m_d->margin, m_d->sliderWidth, m_d->sliderHeight);
    painter.drawRect(sliderRect);
    QRectF fillRect(m_d->margin, m_d->mousePress,   m_d->sliderWidth, height()-m_d->mousePress);
    painter.setBrush(Qt::blue);
    painter.drawRect(fillRect);

    QRectF blockRect(1, m_d->mousePress,  m_d->blockWidth, m_d->blockHeight);
    painter.setBrush(Qt::gray);
    painter.drawRect(blockRect);

    painter.setPen(Qt::darkGray);
    painter.drawText(QRect(m_d->margin,
                           m_d->sliderHeight/2 -m_d->sliderWidth/2,
                           m_d->sliderWidth, m_d->sliderWidth),
                     QString::number(m_d->value), QTextOption(Qt::AlignCenter));

}

void SliderWidget::resizeEvent(QResizeEvent *e)
{
    QWidget::resizeEvent(e);
    m_d->sliderWidth = width() - m_d->margin*2;
    m_d->blockHeight = width()/2;
    m_d->sliderHeight = height()-m_d->blockHeight;
    m_d->blockWidth = width()-2;
    m_d->sliderH = height() -m_d->margin-m_d->blockHeight;

    //    qDebug()<<"sliderHeight"<<m_d->sliderHeight;
    //    qDebug()<<"value "<<m_d->value;
    //    qDebug()<<"blockHeight "<<m_d->blockHeight;
    //    qDebug()<<"value "<<m_d->value;
    //    qDebug()<<"sliderH "<<m_d->sliderH;
    //    qDebug()<<"max-min "<<m_d->max -m_d->min;
    m_d->handleValue();
    //    qDebug()<<"mouse : "<< m_d->mousePress;
}

QSize SliderWidget::sizeHint() const
{
    return QSize(25, 100);
}

void SliderWidgetPrivate::handValueRang()
{
    if(value <= min){
        value =min;
    }else if(value >= max){
        value = max;
    }
}

void SliderWidgetPrivate::handBlockShowRange()
{
    //处理滑块最小位置.
    if(mousePress> sliderHeight){
        mousePress = sliderHeight;
    }
    //处理滑块最大位置.
    if(mousePress <(margin)) {
        mousePress =margin;
    }
}


void SliderWidgetPrivate::handleMousepress()
{
    handBlockShowRange();
    qreal dev = (max-min)/sliderH;
    value = (sliderHeight-mousePress)*dev + min;
    handValueRang();
}

void  SliderWidgetPrivate::handleValue()
{
    qreal dev = (max-min)/sliderH ;
    mousePress = sliderHeight - (value-min)/dev;
    handBlockShowRange();
}
