// Copyright (c) 2018-2019 xDrop developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef ITEMDELEGATEPAINT_H
#define ITEMDELEGATEPAINT_H

#include <QStyledItemDelegate>
#include <QPainter>

namespace WalletGui {

class ItemDelegatePaint : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit ItemDelegatePaint(QObject *parent = 0);
    ItemDelegatePaint(const QString &txt, QObject *parent = 0);

protected:
    void paint( QPainter *painter,
                const QStyleOptionViewItem &option,
                const QModelIndex &index ) const;

signals:

public slots:

};


}
#endif // ITEMDELEGATEPAINT_H