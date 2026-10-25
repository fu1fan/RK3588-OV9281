#include "titlebar.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include "resourceloader.h"
#include <QPainter>

class  TitleBarPrivate{
public:
    QLabel *verLbl = nullptr;
    QLabel *textLbl = nullptr;
    QPushButton *closeBtn = nullptr;
};

#include <QDebug>
using namespace fl_common;

TitleBar::TitleBar(const QString &title, const QString &version, QWidget *parent):QWidget(parent),
    m_d(new TitleBarPrivate)
{
    QHBoxLayout *pHLayout = new QHBoxLayout;
    pHLayout->setContentsMargins(0,0,0,0);

    m_d->closeBtn = new QPushButton;
    m_d->closeBtn->setFlat(true);
    m_d->closeBtn->setFixedSize(32, 32);
    connect(m_d->closeBtn, &QPushButton::clicked, this, &TitleBar::quit);
    m_d->closeBtn->setStyleSheet(QString("border-image:url(%1)").arg(ResourceLoader::instacne()->rc("close")));

    m_d->textLbl = new QLabel(title);
    m_d->textLbl->setStyleSheet("color: blue; font: bold 18px;");

    m_d->verLbl = new QLabel(version);
    m_d->verLbl->setStyleSheet("color: blue");

    pHLayout->addWidget(m_d->textLbl);
    pHLayout->addStretch();
    pHLayout->addWidget(m_d->verLbl);
    pHLayout->addSpacing(30);
    pHLayout->addWidget(m_d->closeBtn);

    QVBoxLayout *pVLayout = new QVBoxLayout(this);
    QFrame *pLine = new QFrame;
    pLine->setFrameShape(QFrame::HLine);
    pLine->setLineWidth(2);
    pLine->setFrameShadow(QFrame::Raised);
    pVLayout->addLayout(pHLayout);
    pVLayout->addWidget(pLine);
    pVLayout->setContentsMargins(10, 10, 10, 0);
    pVLayout->setSpacing(10);

    this->setFixedHeight(50);
}

TitleBar::TitleBar(const QString &title, QWidget *parent):QWidget(parent),
  m_d(new TitleBarPrivate)
{
    QHBoxLayout *pHLayout = new QHBoxLayout;
    pHLayout->setContentsMargins(0,0,0,0);

    m_d->textLbl = new QLabel(title);
    m_d->textLbl->setAlignment(Qt::AlignLeft| Qt::AlignVCenter);
    m_d->textLbl->setStyleSheet("color: blue; font: bold 14px;");

    m_d->closeBtn = new QPushButton;
    m_d->closeBtn->setStyleSheet(QString("border-image:url(%1);").arg(ResourceLoader::instacne()->rc("close")));
    m_d->closeBtn->setFlat(true);
    m_d->closeBtn->setFixedSize(22, 22);
    connect(m_d->closeBtn, &QPushButton::clicked, this, &TitleBar::quit);

    pHLayout->addWidget(m_d->textLbl);
    pHLayout->addSpacing(30);
    pHLayout->addWidget(m_d->closeBtn);

    QVBoxLayout *pVLayout = new QVBoxLayout(this);
    QFrame *pLine = new QFrame;
    pLine->setFrameShape(QFrame::HLine);
    pLine->setLineWidth(2);
    pLine->setFrameShadow(QFrame::Raised);
    pVLayout->addLayout(pHLayout);
    pVLayout->addWidget(pLine);
    pVLayout->setContentsMargins(5, 5, 5, 0);
    pVLayout->setSpacing(10);

    this->setFixedHeight(35);
}

TitleBar::~TitleBar()
{
    delete m_d;
}

void TitleBar::setText(const QString &title)
{
    m_d->textLbl->setText(title);
}
