// Copyright (c) 2011-2016 The Cryptonote developers
// Copyright (c) 2015-2016 XDN developers
// Copyright (c) 2018-2019 xDrop developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "VisibleMessagesModel.h"
#include "MessagesModel.h"
#include "SortedMessagesModel.h"

namespace WalletGui {

VisibleMessagesModel::VisibleMessagesModel() : QSortFilterProxyModel() {
  setSourceModel(&SortedMessagesModel::instance());
}

VisibleMessagesModel::~VisibleMessagesModel() {
}

bool VisibleMessagesModel::filterAcceptsColumn(int _sourceColumn, const QModelIndex& _sourceParent) const {
  return _sourceColumn == MessagesModel::COLUMN_DATE || _sourceColumn == MessagesModel::COLUMN_FROM || _sourceColumn == MessagesModel::COLUMN_TO  ||  _sourceColumn == MessagesModel::COLUMN_MESSAGE;
}

bool VisibleMessagesModel::filterAcceptsRow(int _sourceRow, const QModelIndex& _sourceParent) const {

	//QString type = sourceModel()->data( sourceModel()->index(_sourceRow, 0, _sourceParent) ).toString();
	//bool isSent = (type == "Sent");
	//QString from = sourceModel()->data( sourceModel()->index(_sourceRow, 2, _sourceParent) ).toString();
	//bool isSpam = !(from.size() > 0);
	//return !isSent && !isSpam;
	//return !isSpam;
	//return 1;

	QString status = sourceModel()->data( sourceModel()->index(_sourceRow, 2, _sourceParent) ).toString();
	bool isActive = (status == "Active");
	return isActive;
}




}
