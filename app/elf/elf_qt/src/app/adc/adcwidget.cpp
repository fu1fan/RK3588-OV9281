#include "adcwidget.h"
#include <QWidget>
#include <QHBoxLayout>
#include "basefactory.h"
#include <QPainter>
#include "messagebox.h"
#include <QLCDNumber>
#include <QLabel>
#include <QHBoxLayout>
#include <QDebug>
using namespace fl_common;
class ADCItemWidgt: public QWidget WIDGETCONTROL{
public:
    //Adc
    ADCItemWidgt(const QString &name, QWidget *parent = NULL):QWidget(parent)
    {
        QHBoxLayout *hBox = new QHBoxLayout(this);
        nameLb = new QLabel(name);
        nameLb->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        hBox->addWidget(nameLb);

        valueLb = new QLabel("0");
        valueLb->setAlignment(Qt::AlignCenter);
        hBox->addWidget(valueLb);

        voltageLb = new QLabel("0.00");
        voltageLb->setAlignment(Qt::AlignCenter);
        hBox->addWidget(voltageLb);
        this->setMinimumHeight(40);
    }

    //标题.
    ADCItemWidgt(const QString &valueTitle, const QString &voltageTitle, QWidget *parent = NULL):QWidget(parent)
    {
        QHBoxLayout *hBox = new QHBoxLayout(this);
        nameLb = new QLabel();
        hBox->addWidget(nameLb);

        valueLb = new QLabel(valueTitle);
        valueLb->setAlignment(Qt::AlignCenter);
        hBox->addWidget(valueLb);

        voltageLb = new QLabel(voltageTitle);
        voltageLb->setAlignment(Qt::AlignCenter);
        hBox->addWidget(voltageLb);
        this->setMinimumHeight(40);
    }

    void setData(int v)
    {
        valueLb->setText(QString::number(v));
    }
private:
    QLabel *nameLb;
    QLabel *valueLb;
    QLabel *voltageLb;
};

AdcWidget::AdcWidget(QWidget *parent) : QWidget(parent)
{
    QVBoxLayout *vBox = new QVBoxLayout(this);

    ADCItemWidgt *titleWidgt = new ADCItemWidgt(tr("Value(0-4096)"), tr("Voltage(0-1.8V)"));
    vBox->addWidget(titleWidgt);

    for(int i=0; i<BaseFactory::instance()->adcCount(); i++){
        IAdc *adc = BaseFactory::instance()->adc(i);
        QString name = adc->name();
        if(!adc->open()){
            name +=tr("( Error )");
        }
        ADCItemWidgt *item = new ADCItemWidgt(name);
        connect(adc, &IAdc::readRead, item, [=, item](int v){item->setData(v);});
        vBox->addWidget(item);
        m_items <<item;
    }
    vBox->addStretch();
}

AdcWidget::~AdcWidget()
{
}

void AdcWidget::resetTr()
{
}

void AdcWidget::paintEvent(QPaintEvent *e)
{
    QPainter painter(this);
    for(int i=0; i<m_items.size(); i++){
        QRect r( m_items[i]->mapToParent(QPoint(0,0)), m_items[i]->size());
        (i%2 ==1)? painter.fillRect(r, QColor(222,222,222)): painter.fillRect(r, QColor(243,243,243));
    }
    QWidget::paintEvent(e);
}
