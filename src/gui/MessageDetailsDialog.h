// Copyright (c) 2011-2016 The Cryptonote developers
// Copyright (c) 2015-2016 XDN developers
// Copyright (c) 2018-2019 xDrop developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <QDataWidgetMapper>
#include <QDialog>
#include <QModelIndex>

namespace Ui {
class MessageDetailsDialog;
}

namespace WalletGui {

class MessageDetailsDialog : public QDialog {
  Q_OBJECT

public:
  MessageDetailsDialog(const QModelIndex& _index, QWidget* _parent);
  ~MessageDetailsDialog();

private:
  QScopedPointer<Ui::MessageDetailsDialog> m_ui;
  QDataWidgetMapper m_dataMapper;
  QModelIndex m_modelIndex;

};

}
