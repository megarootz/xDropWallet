// Copyright (c) 2011-2016 The Cryptonote developers
// Copyright (c) 2015-2016 XDN developers
// Copyright (c) 2018-2019 xDrop developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <QClipboard>

#include "SendMessageFrame.h"
#include "CurrencyAdapter.h"
#include "MainWindow.h"
#include "Message.h"
#include "MessageAddressFrame.h"
#include "MessagesModel.h"
#include "WalletAdapter.h"
#include "WalletEvents.h"
#include "AddressBookModel.h"

#include "ui_sendmessageframe.h"

namespace WalletGui {

Q_DECL_CONSTEXPR quint64 MESSAGE_AMOUNT = UINT64_C(100);
Q_DECL_CONSTEXPR quint64 MESSAGE_CHAR_PRICE = UINT64_C(100);
Q_DECL_CONSTEXPR quint64 MINIMAL_MESSAGE_FEE = 0;
Q_DECL_CONSTEXPR int DEFAULT_MESSAGE_MIXIN = 0;

Q_DECL_CONSTEXPR quint32 MINUTE_SECONDS = 60;
Q_DECL_CONSTEXPR quint32 HOUR_SECONDS = 60 * MINUTE_SECONDS;
Q_DECL_CONSTEXPR int MIN_TTL = 5 * MINUTE_SECONDS;
Q_DECL_CONSTEXPR int MAX_TTL = 14 * HOUR_SECONDS;
Q_DECL_CONSTEXPR int TTL_STEP = 5 * MINUTE_SECONDS;

SendMessageFrame::SendMessageFrame(QWidget* _parent) : QFrame(_parent), m_ui(new Ui::SendMessageFrame) {
  m_ui->setupUi(this);
  //m_ui->m_feeSpin->setMinimum(CurrencyAdapter::instance().formatAmount(MESSAGE_AMOUNT + MINIMAL_MESSAGE_FEE).toDouble());
  //m_ui->m_feeSpin->setValue(m_ui->m_feeSpin->minimum());
  
  connect(&WalletAdapter::instance(), &WalletAdapter::walletActualBalanceUpdatedSignal, this, &SendMessageFrame::walletActualBalanceUpdated,
    Qt::QueuedConnection);

  m_ui->m_tickerLabel->setText(CurrencyAdapter::instance().getCurrencyTicker().toUpper());
  m_ui->m_sendButton->setShortcut(QKeySequence(tr("Ctrl+Return")));

  m_ui->m_feeSpin->setSuffix(" " + CurrencyAdapter::instance().getCurrencyTicker().toUpper());

  connect(&WalletAdapter::instance(), &WalletAdapter::walletSendMessageCompletedSignal, this, &SendMessageFrame::sendMessageCompleted,
    Qt::QueuedConnection);
  reset();
}

SendMessageFrame::~SendMessageFrame() {
}

void SendMessageFrame::setAddress(const QString& _address) {
  //m_ui->m_feeSpin->setValue(MESSAGE_AMOUNT + MINIMAL_MESSAGE_FEE);
  m_ui->m_messageTextEdit->clear();
  for (MessageAddressFrame* addressFrame : m_addressFrames) {
    delete addressFrame;
  }
  addRecipientClicked();
  m_ui->m_messageTextEdit->clear();
  m_ui->m_messageTextEdit->setFocus();
  m_ui->m_addReplyToCheck->setChecked(true);
  QString _name = AddressBookModel::instance().getName(_address).toString();
  if (_name!="")
    m_addressFrames.last()->setAddress(QString("%1 <%2>").arg(_name).arg(_address));
  else
    m_addressFrames.last()->setAddress(_address);
}

void SendMessageFrame::sendMessageCompleted(CryptoNote::TransactionId _transactionId, bool _error, const QString& _errorText) {
  Q_UNUSED(_transactionId);
  if (_error) {
    QCoreApplication::postEvent(
      &MainWindow::instance(),
      new ShowMessageEvent(_errorText, QtCriticalMsg));
  } else {
    reset();
  }
}

void SendMessageFrame::walletActualBalanceUpdated(quint64 _balance) {
  m_ui->m_balanceLabel->setText(CurrencyAdapter::instance().formatAmount(_balance));
}


void SendMessageFrame::reset() {
  m_ui->m_feeSpin->setValue(MESSAGE_AMOUNT + MINIMAL_MESSAGE_FEE);
  m_ui->m_messageTextEdit->clear();
  for (MessageAddressFrame* addressFrame : m_addressFrames) {
    addressFrame->deleteLater();
  }
  addRecipientClicked();
}

QString SendMessageFrame::extractAddress(const QString& _addressString) const {
  QString address = _addressString;
  if (_addressString.contains('<')) {
    int startPos = _addressString.indexOf('<');
    int endPos = _addressString.indexOf('>');
    address = _addressString.mid(startPos + 1, endPos - startPos - 1);
  }
  return address;
}

void SendMessageFrame::recalculateFeeValue() {
  QString messageText = m_ui->m_messageTextEdit->toPlainText();
  quint32 messageSize = messageText.length();
  if (messageSize > 0) {
    --messageSize;
  }

  quint64 fee = MESSAGE_AMOUNT * m_addressFrames.size();
  if (m_ui->m_ttlCheck->checkState() == Qt::Checked) {
    fee += MINIMAL_MESSAGE_FEE + messageSize * MESSAGE_CHAR_PRICE;
  }else{
    fee = MINIMAL_MESSAGE_FEE;
  }

  m_ui->m_feeSpin->setMinimum(CurrencyAdapter::instance().formatAmount(fee).toDouble());
  m_ui->m_feeSpin->setValue(m_ui->m_feeSpin->minimum());
}

void SendMessageFrame::recalculateHeight(){
  size_t height = 10;
  for (size_t i = 0; i < m_addressFrames.size(); ++i) {
    height += (m_addressFrames[i]->height());
  }
  m_ui->m_messageAddressScrollArea->setFixedHeight(height);
}
void SendMessageFrame::addRecipientClicked() {
  if (m_addressFrames.size() == 9) return;
  MessageAddressFrame* newAddress = new MessageAddressFrame(m_ui->m_messageAddressScrollArea);
  m_ui->m_addressesLayout->insertWidget(m_addressFrames.size(), newAddress);
  m_addressFrames.append(newAddress);
  if (m_addressFrames.size() == 1) {
    newAddress->disableRemoveButton(true);
  } else {
    m_addressFrames[0]->disableRemoveButton(false);
  }

  QCompleter* ModelCompleter = new QCompleter(this);
  ModelCompleter->setModel(&AddressBookModel::instance());
  ModelCompleter->setCaseSensitivity(Qt::CaseInsensitive);
  ModelCompleter->setCompletionMode(QCompleter::PopupCompletion);
  ModelCompleter->setModelSorting(QCompleter::CaseInsensitivelySortedModel);
  ModelCompleter->setFilterMode(Qt::MatchContains);
  ModelCompleter->setCompletionColumn(AddressBookModel::COLUMN_LABEL);
  ModelCompleter->setCompletionRole(AddressBookModel::ROLE_LABEL);
  newAddress->setCompleter(ModelCompleter);

  connect(newAddress, &MessageAddressFrame::destroyed, [this](QObject* _obj) {
    m_addressFrames.removeOne(static_cast<MessageAddressFrame*>(_obj));
    if (m_addressFrames.size() == 1) {
      m_addressFrames[0]->disableRemoveButton(true);
    }
    recalculateFeeValue();
    recalculateHeight();
  });

  recalculateFeeValue();
  recalculateHeight();
}

void SendMessageFrame::messageTextChanged() {
  recalculateFeeValue();
}


void SendMessageFrame::sendClicked() {
  if (!WalletAdapter::instance().isOpen()) {
    return;
  }

  QVector<CryptoNote::WalletLegacyTransfer> transfers;
  QVector<CryptoNote::TransactionMessage> messages;
  QString messageString = m_ui->m_messageTextEdit->toPlainText();

  if (messageString.trimmed() == ""){
    return;
  }

  if (m_ui->m_addReplyToCheck->isChecked()) {
    MessageHeader header;
    header.append(qMakePair(QString(MessagesModel::HEADER_FROM_KEY), WalletAdapter::instance().getAddress()));
    messageString = Message::makeTextMessage(messageString, header);
  }

  transfers.reserve(m_addressFrames.size());
  for (MessageAddressFrame* addressFrame : m_addressFrames) {
    QString address = extractAddress(addressFrame->getAddress()).trimmed();
    if (address != "" && address != WalletAdapter::instance().getAddress()){
      if (!CurrencyAdapter::instance().validateAddress(address)) {
        QCoreApplication::postEvent(&MainWindow::instance(), new ShowMessageEvent(tr("Invalid recipient address"), QtCriticalMsg));
        return;
      }
      transfers.append({address.toStdString(), MESSAGE_AMOUNT});
      messages.append({messageString.toStdString(), address.toStdString()});
    }
  }

  quint64 fee = CurrencyAdapter::instance().parseAmount(m_ui->m_feeSpin->cleanText());
  if (m_ui->m_ttlCheck->checkState() == Qt::Checked) {
    fee -= MESSAGE_AMOUNT * transfers.size();
    if (fee > CurrencyAdapter::instance().parseAmount(m_ui->m_balanceLabel->text()) ||  CurrencyAdapter::instance().parseAmount(m_ui->m_balanceLabel->text()) < MESSAGE_AMOUNT * transfers.size()) {
      QCoreApplication::postEvent(&MainWindow::instance(), new ShowMessageEvent(tr("Insufficient funds.\r\n\nAsk someone to send you coins or use your CPU to mine for new ones."), QtCriticalMsg));
      return;
    }
  }

  quint64 ttl = 0;
  if (m_ui->m_ttlCheck->checkState() == Qt::Unchecked) {
    ttl = QDateTime::currentDateTimeUtc().toTime_t() + MIN_TTL;
  }

  if (ttl == 0 && fee < MINIMAL_MESSAGE_FEE) {
    QCoreApplication::postEvent(&MainWindow::instance(), new ShowMessageEvent(tr("Incorrect fee value"), QtCriticalMsg));
    return;
  }

  if (WalletAdapter::instance().isOpen()) {
    WalletAdapter::instance().sendMessage(transfers, fee, 2, messages, ttl);
  }
}

void SendMessageFrame::ttlCheckStateChanged(int _state) {
  recalculateFeeValue();
}


}
