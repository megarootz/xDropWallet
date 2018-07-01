#include <QModelIndex>
#include <QSize>

#include "ItemDelegatePaint.h"
#include "MessagesModel.h"
#include "WalletAdapter.h"

namespace WalletGui {

ItemDelegatePaint::ItemDelegatePaint(QObject *parent) : QStyledItemDelegate(parent)
{
}

void ItemDelegatePaint::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QString txt = index.model()->data( index, Qt::DisplayRole ).toString();
    QString to = index.model()->data( index, MessagesModel::ROLE_TO_LABEL).toString();

    if(to != "Me"){
        painter->fillRect(option.rect,QColor("#F0F0F0"));
    }else{
/*
    	if (index.column() == 2){
	      QPixmap pixmap(":icons/anon");
	      pixmap.scaled(32, 32, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
	      painter->drawPixmap(0, 0,Qt::AlignCenter, pixmap);
    	}
*/
    }

    if( option.state & QStyle::State_Selected )//we need this to show selection
    {
        painter->fillRect( option.rect, option.palette.highlight() );
    }


    QStyledItemDelegate::paint(painter,option,index);//standard processing
}

}