/* WIFI 代理和模型显示接口*/
#ifndef WIFIITEMDELEGATE_H
#define WIFIITEMDELEGATE_H
#include <QAbstractItemDelegate>
#include <QAbstractListModel>
#include "wpaclicmd.h"
#include "hostapdcli.h"

class WifiItemDelegate : public QAbstractItemDelegate
{
public:
    WifiItemDelegate(QObject *parent = Q_NULLPTR);
    virtual void paint(QPainter *painter,
                       const QStyleOptionViewItem &option,
                       const QModelIndex &index) const ;
    virtual QSize sizeHint(const QStyleOptionViewItem &option,
                           const QModelIndex &index) const ;
    int getLevel(int singalLevel) const;
};

class WifiModel: public  QAbstractListModel
{
public:
    WifiModel(QObject *parent = Q_NULLPTR);
    void setData(const QList<ScanInfo> &list);

    void setDataHostap(const QList<ScanInfoAp> &infos);
    void lostConnectHoseap(const QString &ssid);

protected:
    int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
    bool canFetchMore(const QModelIndex &parent) const;
    void fetchMore(const QModelIndex &parent);

private:
    QList<ScanInfo> m_list;
    int m_fixCount;
    QString m_connName;
};

#endif // WIFIITEMDELEGATE_H
