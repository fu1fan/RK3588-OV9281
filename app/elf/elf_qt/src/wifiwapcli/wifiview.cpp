#include "wifiview.h"
#include <QPainter>
#include <QStyleOptionViewItem>
#include <QStyleOption>
#include <QRect>
#include <QTextOption>
#include <QDebug>
#include <QCoreApplication>
#include <QPalette>
#include <math.h>

WifiItemDelegate::WifiItemDelegate(QObject *parent):QAbstractItemDelegate(parent)
{

}

void WifiItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QList<QVariant> list  = index.data(Qt::DisplayRole).toList();
    //    QString bssid =list[0].toString();
    QString wifiNmae = list[4].toString();
    //    QString flag = list[3].toString();
    int  connectState = list[5].toInt();
    QString text = wifiNmae;
    int  sig = list[2].toInt();
    painter->setRenderHint(QPainter::Antialiasing);
    int width = option.rect.width();
    int height = option.rect.height();
    int x = option.rect.x();
    int y= option.rect.y();
    int nameWidth =width -40;

    if(option.state &QStyle::State_Selected)
    {
        painter->save();
        QPen pen(Qt::transparent);
        painter->setPen(pen);
        painter->setBrush(QColor(236, 238,237));
        painter->setOpacity(0.7);
        painter->drawRect(x, y, width, height);
        painter->restore();
    }

    QRectF nameRect(x,y, nameWidth, height);
    painter->save();
    QTextOption o(Qt::AlignLeft);
    QRectF fitNameRect =painter->boundingRect(nameRect, text, o);
    if(connectState > 0)
    {
        QTextOption bottom(Qt::AlignCenter);
        QString text ="connected";
        painter->setPen(Qt::blue);
        if(connectState !=2)
        {
           text="connect..."; painter->setPen(Qt::gray);
        }

        QRectF connectRect = painter->boundingRect(nameRect, text, bottom);
        painter->drawText(connectRect, text);
    }

    painter->drawText(fitNameRect, text);
    painter->setPen(QColor(215, 212,209));
    painter->drawLine(x+5, y+height, x+width-5, y+height+1);
    painter->restore();

    QRectF sigRect(x+ nameWidth, y+ 10, height, height);
    int startAngle = 40 * 16;
    int spanAngle = 100 * 16;
    QPen pen(Qt::gray);
    pen.setWidthF(1.8);
    int level =getLevel(sig);
    for(int i=0; i<3; i++){
        painter->save();
        if(i< 4-level)
        {
            pen.setColor(Qt::gray);
        }else{
            pen.setColor(Qt::black);
        }
        painter->setPen(pen);
        painter->drawArc(sigRect, startAngle, spanAngle);
        painter->restore();
        int adj =6;
        sigRect.adjust(adj, adj,-adj,-adj);
    }

    QRectF centerRect(x+ nameWidth, y+ 10, height, height);
    int r = (height/2)-2;
    centerRect.adjust(r, r, -r, -r);
    QColor color;
    level ==0 ? color = Qt::gray: color = Qt::black;
    painter->setPen(color);
    painter->setBrush(color);
    painter->drawEllipse(centerRect);
}

QSize WifiItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option)
    Q_UNUSED(index)
    return  QSize(200, 40);
}


int WifiItemDelegate::getLevel(int singalLevel) const
{
    int ret = 1;
    if( singalLevel >= -66){
        ret =4;
    }else if(singalLevel >=-77 &&singalLevel<-66){
        ret =3;
    }else if(singalLevel >=-88 && singalLevel<-77){
        ret =2;
    }else if(singalLevel >=-100 && singalLevel<-88){
        ret =1;
    }else{
        ret =0;
    }
    return ret;
}


WifiModel::WifiModel(QObject *parent):QAbstractListModel(parent)
{
}

QList<ScanInfo> removeRepeat(const QList<ScanInfo> &list)
{
    QList<ScanInfo> results;
    foreach (ScanInfo info, list) {
        if(!results.contains(info)){
            results <<info;
        }
    }
    return results;
}

void WifiModel::setData(const QList<ScanInfo> &list)
{
    if(list == m_list)
    {
        return;
    }
    beginResetModel();
    m_list = list;
    m_fixCount =0;
    endResetModel();
}

void WifiModel::setDataHostap(const QList<ScanInfoAp> &infos)
{
    QList<ScanInfo> list;
    foreach (ScanInfoAp info, infos) {
        ScanInfo l;
        l.ssid = info.ssid;
        l.signalLevel = info.signal;
        list << l;
    }

    setData(list);
}

void WifiModel::lostConnectHoseap(const QString &ssid)
{
    QList<ScanInfo> list = m_list;
    ScanInfo info;
    info.ssid = ssid;
    list.removeOne(info);
    setData(list);
}

int WifiModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_list.size();
}

QVariant WifiModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
    {
        return QVariant();
    }
    if (index.row() >= m_list.size() || index.row() < 0)
        return QVariant();
    if(role == Qt::DisplayRole)
    {
        ScanInfo d =m_list[index.row()];
        QList<QVariant> list;
        list << d.bssid
             <<d.frequency
            << d.signalLevel
            <<d.flags
           << d.ssid
           <<d.state;
        return QVariant(list);
    }
    return QVariant();
}

bool WifiModel::canFetchMore(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_fixCount< m_list.size();
}

void WifiModel::fetchMore(const QModelIndex &parent)
{
    Q_UNUSED(parent)
    int remainder = m_list.size() - m_fixCount;
    int itemsToFetch = qMin(10, remainder);
    beginInsertRows(QModelIndex(), m_fixCount, m_fixCount+itemsToFetch-1);
    m_fixCount += itemsToFetch;
    endInsertRows();
}
