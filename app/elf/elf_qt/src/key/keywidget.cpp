#include "keywidget.h"
#include <QSocketNotifier>
#include <QLoggingCategory>
#include <linux/input.h>
#include <fcntl.h>
#include <unistd.h>
#include <QHBoxLayout>
#include <QPainter>
#include <QPushButton>
#include <QGridLayout>
#include <QResizeEvent>
#include <QLabel>
#include "messagebox.h"
#include <QPainter>
#include "basefactory.h"
using namespace fl_common;
class KeyLbl: public QWidget
{
public:
    KeyLbl(int key, const QString &text, QWidget *parent = Q_NULLPTR):
        QWidget( parent),mText(text),mKey(key),mPress(false){}

    int key(){return mKey;}
    void paintEvent(QPaintEvent *e) override;
    void setPress(bool bPress){
        mPress = bPress;
        this->update();
    }
private:
    QString mText;
    int mKey;
    bool mPress;
};

void KeyLbl::paintEvent(QPaintEvent *e)
{
    QWidget::paintEvent(e);
    QPainter painter(this);
    if(mPress){
        painter.setPen(Qt::blue);
        QFont f = this->font();
        f.setPointSize(f.pointSize() +5);
        painter.setFont(f);
    }else{
        painter.setPen(Qt::black);
    }
    QRectF rect = painter.boundingRect(this->rect(), mText,  QTextOption(Qt::AlignCenter));
    painter.drawText(rect, mText);
    painter.setPen(Qt::gray);

    int h=(int)(height()-rect.height())/4;
    painter.drawLine(0, this->height()-h,   this->width(), this->height()-h);
}

KeyWidget::KeyWidget(QWidget *parent) : QWidget(parent)
{
    QGridLayout *gLayout =new QGridLayout(this);
    gLayout->setContentsMargins(50, 50, 50, 50);
    gLayout->setSpacing(20);

    BaseFactory::instance()->key()->open();

    QList<int > keys =  BaseFactory::instance()->key()->keyCodes();
    QList<QString> names =   BaseFactory::instance()->key()->keyNames();

    QList<KeyLbl*> buttons;
    int n=0;
    foreach (QString name, names) {
        if(!BaseFactory::instance()->key()->isOpen()){
            name +="( Error )";
        }
        KeyLbl *btn = new KeyLbl(keys[n], name);
        btn->setPress(false);
        buttons <<btn;
        gLayout->addWidget(btn, 0, n++, 3, 1);
    }

    m_tip = new QLabel();
    m_tip->setAlignment(Qt::AlignCenter);
    gLayout->addWidget(m_tip, 4, 0, 1, n);
    m_tip->setStyleSheet("color: gray");
    this->resetTr();

    connect(BaseFactory::instance()->key(), &IKey::keyPress, this, [buttons](int key, bool bPressed){
        foreach (KeyLbl *btn, buttons) {
            if(key == btn->key()) {
                btn->setPress(bPressed);
            }
        }
    });
}

void KeyWidget::resetTr()
{
    m_tip->setText(tr("Press the Key on the development board"));
}
