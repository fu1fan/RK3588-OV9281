#include "mainwindow.h"
#include <QVBoxLayout>
#include "widgetfactory.h"
#include <QRegion>
#include <QTabBar>
#include "widgetcontrol.h"
#include <QPair>
#include <QCoreApplication>
#include "backlightwidget.h"
#include <QDebug>
#include <QComboBox>
#include <QListView>

class MainWindowPrivate{
public:
    QVBoxLayout *vBox = nullptr;
    TitleBar * titleBar = nullptr;
    QTabBar *bar = nullptr;
    QWidget *currentWidgt = nullptr;
};

MainWindow::MainWindow(QWidget *parent) : QWidget(parent),m_d(new MainWindowPrivate)
{
    this->setWindowFlags(Qt::FramelessWindowHint);

    m_d->vBox = new QVBoxLayout(this);
    m_d->titleBar = WidgetFactory::instance()->createTitleBar(tr("ELF EMBEDDED"), tr("version:0.0.1"));
    connect(m_d->titleBar, &TitleBar::quit, this, [=](){
        qApp->quit();
    });
    m_d->vBox->addWidget(m_d->titleBar);

    this->setAutoFillBackground(true);
    QPalette p = this->palette();
    p.setBrush(QPalette::Background, Qt::white);
    this->setPalette(p);

    qApp->installEventFilter(this);

}

MainWindow::~MainWindow()
{
    delete m_d;
}

void MainWindow::showWidget(int id)
{
    QWidget *w = WidgetFactory::instance()->widget(id);
    if(w  && m_d->currentWidgt != w ){
        if(m_d->currentWidgt != nullptr){
            m_d->currentWidgt->hide();
        }
        m_d->currentWidgt = w;
        m_d->vBox->addWidget(w);
        if(!m_d->currentWidgt->isVisible())
            m_d->currentWidgt->show();
    }
}

int MainWindow::currentWidget()
{
    int id = -1;
    if(m_d->currentWidgt != nullptr){
        id =WidgetFactory::instance()->id(m_d->currentWidgt);
    }
    return id;
}

void MainWindow::setWindowTitleVisible(bool bVisible, int border)
{
    if(m_d->titleBar){
        m_d->titleBar->setVisible(bVisible);
        layout()->setContentsMargins(border, border, border, border);
    }
}

void MainWindow::resetTr()
{
    if(m_d->titleBar){
        m_d->titleBar->setText(tr("Mainwindow"));
    }
}

bool MainWindow::eventFilter(QObject *object, QEvent *e)
{
    static QList<QObject*> list;
    QComboBox *comBox = dynamic_cast<QComboBox*>(object);
    if(comBox && !list.contains(object)){
        list << object;
        comBox->setMinimumHeight(35);
        comBox->setView(new QListView());
        comBox->setStyleSheet("QComboBox QAbstractItemView::item {min-height:30px; }");
    }

    return QWidget::eventFilter(object, e);
}
