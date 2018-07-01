// Copyright (c) 2011-2016 The Cryptonote developers
// Copyright (c) 2015-2016 XDN developers
// Copyright (c) 2018-2019 xDrop developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "CurrencyAdapter.h"
#include "OverviewFrame.h"
#include "WalletAdapter.h"
#include "MessagesModel.h"
#include "Settings.h"
#include "NodeAdapter.h"

#include "ui_overviewframe.h"

namespace WalletGui {

OverviewFrame::OverviewFrame(QWidget* _parent) : QFrame(_parent), m_ui(new Ui::OverviewFrame){
  m_ui->setupUi(this);
  
  connect(&WalletAdapter::instance(), &WalletAdapter::changeWalletPasswordSignal, this, &OverviewFrame::encryptWallet, Qt::QueuedConnection);
  connect(&MessagesModel::instance(), &MessagesModel::messagesCountUpdatedSignal, this, &OverviewFrame::messagesCountUpdated, Qt::QueuedConnection);
  connect(&WalletAdapter::instance(), &WalletAdapter::walletActualBalanceUpdatedSignal, this, &OverviewFrame::actualBalanceUpdated, Qt::QueuedConnection);
  connect(&WalletAdapter::instance(), &WalletAdapter::walletPendingBalanceUpdatedSignal, this, &OverviewFrame::pendingBalanceUpdated, Qt::QueuedConnection);
  connect(&WalletAdapter::instance(), &WalletAdapter::walletCloseCompletedSignal, this, &OverviewFrame::reset, Qt::QueuedConnection);
  connect(&NodeAdapter::instance(), &NodeAdapter::peerCountUpdatedSignal, this, &OverviewFrame::peerCountUpdated, Qt::QueuedConnection);
  
  m_ui->m_tickerLabel1->setText(CurrencyAdapter::instance().getCurrencyTicker().toUpper());
  m_ui->m_tickerLabel2->setText(CurrencyAdapter::instance().getCurrencyTicker().toUpper());
  m_ui->m_getStartedLabel->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Preferred );

  reset();
}

OverviewFrame::~OverviewFrame() {
}

void OverviewFrame::encryptWallet() {
  if (Settings::instance().isEncrypted()){
    m_ui->m_walletEncryptedLabel->setText(tr("On"));
    m_ui->m_walletEncryptedLabel->setStyleSheet("color: green;");
  } else {
    m_ui->m_walletEncryptedLabel->setText(tr("Off"));
    m_ui->m_walletEncryptedLabel->setStyleSheet("color: red;");
  }  
}

void OverviewFrame::peerCountUpdated(quint64 _peerCount) {
  m_ui->m_peersCountLabel->setText(QString(tr("%1").arg(_peerCount)));
  quint64 peersCount = NodeAdapter::instance().getPeerCount();
}

void OverviewFrame::messagesCountUpdated(quint64 _messagesCount) {
  m_ui->m_messagesCountLabel->setText(QVariant(_messagesCount).toString());
  quint64 messagesCount = MessagesModel::instance().rowCount();
}

void OverviewFrame::actualBalanceUpdated(quint64 _balance) {
  m_ui->m_actualBalanceLabel->setText(CurrencyAdapter::instance().formatAmount(_balance));
  quint64 pendingBalance = WalletAdapter::instance().getPendingBalance();
}

void OverviewFrame::pendingBalanceUpdated(quint64 _balance) {
  m_ui->m_pendingBalanceLabel->setText(CurrencyAdapter::instance().formatAmount(_balance));
  quint64 actualBalance = WalletAdapter::instance().getActualBalance();
}

void OverviewFrame::reset() {
  actualBalanceUpdated(0);
  pendingBalanceUpdated(0);
  messagesCountUpdated(0);
  peerCountUpdated(0);
  encryptWallet();
}

}
