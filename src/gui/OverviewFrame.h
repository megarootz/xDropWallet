// Copyright (c) 2011-2016 The Cryptonote developers
// Copyright (c) 2015-2016 XDN developers
// Copyright (c) 2018-2019 xDrop developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <QFrame>
#include <QStyledItemDelegate>

namespace Ui {
class OverviewFrame;
}

namespace WalletGui {

class OverviewFrame : public QFrame {
  Q_OBJECT
  Q_DISABLE_COPY(OverviewFrame)

public:
  OverviewFrame(QWidget* _parent);
  ~OverviewFrame();

  Q_SLOT void encryptWallet();
  
private:
  QScopedPointer<Ui::OverviewFrame> m_ui;


  void peerCountUpdated(quint64 _peer_count);
  void messagesCountUpdated(quint64 _messagesCount);
  void actualBalanceUpdated(quint64 _balance);
  void pendingBalanceUpdated(quint64 _balance);
  void reset();
};

}
