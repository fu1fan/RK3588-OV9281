#include "backlightwidget.h"
#include "ui_backlightwidget.h"
#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QGuiApplication>
#include <QScreen>
#include <QDebug>
#include <QApplication>
#include <QScreen>
#include "messagebox.h"
#include "basefactory.h"

using namespace fl_common;
BacklightWidget::BacklightWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::BacklightWidget)
{
    ui->setupUi(this);

    createColowWidget();

    QFont f= ui->textBrowser->font();
    f.setPixelSize(20);
    ui->textBrowser->setFont(f);


    for(int i=0; i<BaseFactory::instance()->backlightCount(); i++)
        ui->combox->addItem(BaseFactory::instance()->backlight(i)->name());

    connect(ui->combox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            [=](int index){
        currentBacklightChange(index);
    });

    connect(ui->horizontalSlider, &SliderWidget::valueChanged, this, [=](int v){
        if(IBacklight *back = BaseFactory::instance()->backlight(ui->combox->currentIndex())){
            back->setValue(v);
        }
    });

    this->resetTr();

    currentBacklightChange(0);
}

BacklightWidget::~BacklightWidget()
{
    delete ui;
}

void BacklightWidget::resetTr()
{
    ui->label_red->setText(tr("R"));
    ui->label_green->setText(tr("G"));
    ui->label_blue->setText(tr("B"));
    ui->label_Screen->setText("Screen");

    QScreen *screen = qApp->primaryScreen();
    if(screen){
        ui->textBrowser->clear();
        ui->textBrowser->append(tr("Primary Screen info:"));
        ui->textBrowser->append(tr("resolution (pixel):") + QString(" %1 X %2").arg( screen->size().width()).arg(screen->size().height()));
        ui->textBrowser->append(tr("logical dots or pixels per inch:") + QString("%1").arg( screen->logicalDotsPerInch()));
        ui->textBrowser->append(tr("color depth:") + QString("%1").arg( screen->depth()));
        ui->textBrowser->append(tr("vertical refresh rate(Hz):") + QString("%1").arg( screen->refreshRate()));
    }
}

void BacklightWidget::currentBacklightChange(int index)
{

    IBacklight *back = BaseFactory::instance()->backlight(index);
    if(back == nullptr)
        return;

    this->resetTr();

    if(!back->isOpen()){
        if(!back->open()){
            ui->horizontalSlider->setEnabled(false);
            ui->horizontalSlider->reset();
            ui->textBrowser->append(QString("Open  %1 faire!").arg(back->devName()));
            return;
        }
    }

    ui->horizontalSlider->setEnabled(true);

    int min = 0;
    int max = 0;
    back->getRange(min, max);
    ui->horizontalSlider->setRange(min, max);
    ui->minLbl->setText(QString::number(min));
    ui->maxLbl->setText(QString::number(max));
    int currentValue = back->getValue();
    ui->horizontalSlider->setValue(currentValue);
}

void BacklightWidget::createColowWidget()
{
    int space = this->height()/32;
    ui->verticalLayout_4->setSpacing(space);
    auto createCircle = [=](QWidget *p, const QColor &color){
        p->setFixedSize(38, 38);
        QPixmap pixmap(p->size());
        QPainter painter(&pixmap);
        painter.fillRect(pixmap.rect(), p->palette().brush(QPalette::Background));


        painter.setPen(color);
        painter.setRenderHint(QPainter::Antialiasing);
        QRect rect = p->rect();
        rect =rect.adjusted(1,1,-1,-1);
        painter.setBrush(color);
        painter.drawEllipse(rect);
        QPalette palette = p->palette();
        p->setAutoFillBackground(true);
        palette.setBrush(QPalette::Background, pixmap);
        p->setPalette(palette);
    };
    createCircle(ui->redWidget, Qt::red);
    createCircle(ui->greenWidget, Qt::green);
    createCircle(ui->blueWidget, Qt::blue);
}
