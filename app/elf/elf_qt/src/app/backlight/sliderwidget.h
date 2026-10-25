/* QSlider按值滑动,设置范围较小时，导致触摸看上去和显示位置不符合. 按照像素滑动纵行滑窗口接近值绘制.*/
#ifndef SLIDERWIDGET_H
#define SLIDERWIDGET_H

#include <QWidget>

class SliderWidgetPrivate;
class  SliderWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SliderWidget(QWidget *parent = nullptr);
    ~ SliderWidget();

    void setRange(int min, int max);
    int value();
    void setValue(int value);

Q_SIGNALS:
    void valueChanged(int v);

protected:
    void mousePressEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    void paintEvent(QPaintEvent *e);
    void resizeEvent(QResizeEvent *e);

    QSize sizeHint()const;

private:
    SliderWidgetPrivate *m_d;
};

#endif // SLIDERWIDGET_H
