#include "messagebox.h"
#include <QEventLoop>
#include <QPushButton>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPainter>
#include <QMessageBox>
#include <QApplication>
#include <QDesktopWidget>
#include <QMouseEvent>
#include <QDebug>
#include <QTimer>
#include <QPropertyAnimation>
#include <QFontMetrics>
#include "titlebar.h"

MessageBox::MessageBox(QWidget *parent, const QString &text, Button button) : QWidget(parent)
{
    this->resize(350, 250);
    m_pLoop = new QEventLoop(this);

    QLabel *label =new QLabel;
    label->setTextFormat(Qt::RichText);
    label->setWordWrap(true);
    label->setAlignment(Qt::AlignCenter);
    label->setText(text);


    QPushButton *yes = new QPushButton(tr("OK"));
    yes->setMinimumHeight(35);
    connect(yes, &QPushButton::pressed, this, [=](){
        m_pLoop->exit(Button::yes);
    });

    QPushButton *no = new QPushButton(tr("Cancel"));
    no->setMinimumHeight(35);
    connect(no, &QPushButton::clicked, this, [=](){
        m_pLoop->exit(Button::no);
    });

    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->setSpacing(30);
    hLayout->addStretch();
    hLayout->addWidget(yes);
    hLayout->addWidget(no);

    TitleBar * bar =  WidgetFactory::instance()->createTitleBar("MESSAGE");
    connect(bar, &TitleBar::quit, m_pLoop, &QEventLoop::quit);

    QVBoxLayout *VLayout = new QVBoxLayout(this);
    VLayout->setContentsMargins(15, 15, 15, 15);
    VLayout->addWidget(bar);
    VLayout->addSpacing(15);
    VLayout->addWidget(label);
    VLayout->addLayout(hLayout);

    switch (button) {
    case Button::yes:{
        no->setVisible(false);
    }break;

    case Button::no:{
        yes->setVisible(false);
    } break;

    default:
        break;
    }
}

int MessageBox::exec()
{
    this->setWindowFlags( Qt::FramelessWindowHint);
    this->show();
    QWidget *parentWidget = dynamic_cast<QWidget*>(parent());
    if(parentWidget != Q_NULLPTR && parentWidget->isVisible()) {
        int x = (parentWidget->width()-width())/2;
        int y = (parentWidget->height()-height())/2;
        move(x, y);
        filterChild(parentWidget);
    }else{
        foreach (QWidget *w, qApp->topLevelWidgets()) {
            this->filterChild(w);
        }
        int x = (qApp->desktop()->width()-width())/2;
        int y = (qApp->desktop()->height()-height())/2;
        move(x, y);
    }

    int ret = m_pLoop->exec();

    if(parentWidget != Q_NULLPTR)
    {
        //解除其它窗口部件操作.
        foreach (QObject *o, m_object) {
            o->removeEventFilter(this);
        }
        m_object.clear();
    }

    delete m_pLoop;
    return ret;
}

void MessageBox::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e)
    QWidget::paintEvent(e);
    QPainter painter(this);

    painter.fillRect(this->rect(), Qt::white);

    painter.setPen(Qt::gray);
    painter.drawRect(this->rect().adjusted(1, 1, -2, -2));

    QWidget::paintEvent(e);
}

bool MessageBox::eventFilter(QObject *watched, QEvent *event)
{
    //过滤所非消息窗口的鼠标点击和聚集事件，使只能操作当强消息窗口.
    QInputEvent  *pMouse = dynamic_cast<QInputEvent*>(event);
    QFocusEvent *pFocus = dynamic_cast<QFocusEvent*>(event);
    if(m_object.contains(watched)  && (pMouse|| pFocus)){
        event->ignore();
        return true;
    }

    return QWidget::eventFilter(watched, event);
}

void MessageBox::filterChild(QObject *o)
{
    if(o == this)
        return;

    foreach (QObject *child, o->children()) {

        if(child != this){
            m_object << child ;
            child->installEventFilter(this);
            filterChild(child);
        }
    }
}

int MessageBox::showMessage(QWidget *parent, const QString &text,  Button button)
{
    if(parent && !parent->isVisible())
    {
        parent = nullptr;
    }
    MessageBox tip(parent, text,  button);
    return tip.exec();
}

void MessageBox::showMessage(QWidget *parent, const QString &text, int seconds)
{
    if(parent && !parent->isVisible())
    {
        parent = nullptr;
    }

    if(seconds <=0)
        return ;
    MessageBox tip(parent, text,  MessageBox::no);
    QTimer timer;
    QObject::connect(&timer, &QTimer::timeout, &tip, [&tip](){
        tip.m_pLoop->exit();;
    });
    timer.start(seconds*1000);

    tip.exec();
}
