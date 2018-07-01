// Copyright (c) 2011-2016 The Cryptonote developers
// Copyright (c) 2015-2016 XDN developers
// Copyright (c) 2018-2019 xDrop developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <QFrame>
#include <QItemSelection>
#include <QSortFilterProxyModel>


namespace Ui {
class AddressBookFrame;
}

namespace WalletGui {

class AddressBookFrame : public QFrame {
  Q_OBJECT
  Q_DISABLE_COPY(AddressBookFrame)

public:
  AddressBookFrame(QWidget* _parent);
  ~AddressBookFrame();

  Q_SLOT void addClicked(QString _address = "");

private:
  QScopedPointer<Ui::AddressBookFrame> m_ui;
  QSortFilterProxyModel *proxyModel;

  Q_SLOT void copyClicked();
  Q_SLOT void deleteClicked();
  Q_SLOT void payClicked();
  Q_SLOT void msgClicked();
  Q_SLOT void currentAddressChanged(const QModelIndex& _index);

Q_SIGNALS:
  void payToSignal(const QModelIndex& _index);
  void replyToSignal(const QModelIndex& _index);
};

}
