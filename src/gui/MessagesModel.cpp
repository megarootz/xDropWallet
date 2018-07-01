// Copyright (c) 2011-2016 The Cryptonote developers
// Copyright (c) 2015-2016 XDN developers
// Copyright (c) 2018-2019 xDrop developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <QDateTime>
#include <QFont>
#include <QMetaEnum>
#include <QPixmap>
#include <QTextStream>
#include <QJsonDocument>
#include <QJsonObject>

#include "CurrencyAdapter.h"
#include "NodeAdapter.h"
#include "MessagesModel.h"
#include "TransactionsModel.h"
#include "WalletAdapter.h"
#include "AddressBookModel.h"
#include "MainWindow.h"
#include "Settings.h"

namespace WalletGui {

enum class MessageType : quint8 {INPUT, OUTPUT};

const int MESSAGES_MODEL_COLUMN_COUNT =
  MessagesModel::staticMetaObject.enumerator(MessagesModel::staticMetaObject.indexOfEnumerator("Columns")).keyCount();

const QString MessagesModel::HEADER_FROM_KEY = "From";

MessagesModel& MessagesModel::instance() {
  static MessagesModel inst;
  return inst;
}

MessagesModel::MessagesModel() : QAbstractItemModel() {
  connect(&WalletAdapter::instance(), &WalletAdapter::reloadWalletTransactionsSignal, this, &MessagesModel::reloadWalletTransactions,
    Qt::QueuedConnection);
  connect(&WalletAdapter::instance(), &WalletAdapter::walletTransactionCreatedSignal, this,
    static_cast<void(MessagesModel::*)(CryptoNote::TransactionId)>(&MessagesModel::appendTransaction), Qt::QueuedConnection);
  connect(&WalletAdapter::instance(), &WalletAdapter::walletTransactionUpdatedSignal, this, &MessagesModel::updateWalletTransaction,
    Qt::QueuedConnection);
  connect(&NodeAdapter::instance(), &NodeAdapter::lastKnownBlockHeightUpdatedSignal, this, &MessagesModel::lastKnownHeightUpdated,
    Qt::QueuedConnection);
  connect(&WalletAdapter::instance(), &WalletAdapter::walletCloseCompletedSignal, this, &MessagesModel::reset,
    Qt::QueuedConnection);
}

MessagesModel::~MessagesModel() {
}

Qt::ItemFlags MessagesModel::flags(const QModelIndex& _index) const {
  Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemNeverHasChildren | Qt::ItemIsSelectable;
  if(_index.column() == COLUMN_MESSAGE) {
    //flags |= Qt::ItemIsSelectable;
  }

  return flags;

}

int MessagesModel::columnCount(const QModelIndex& _parent) const {
  return MESSAGES_MODEL_COLUMN_COUNT;
}

int MessagesModel::rowCount(const QModelIndex& _parent) const {
    return m_messages.size();
}

QVariant MessagesModel::headerData(int _section, Qt::Orientation _orientation, int _role) const {
  if(_orientation != Qt::Horizontal) {
    return QVariant();
  }

  switch(_role) {
  case Qt::DisplayRole:
    switch(_section) {
    case COLUMN_DATE:
      return tr("Blockchain timestamp");
    case COLUMN_TYPE:
      return tr("");
    case COLUMN_HEIGHT:
      return tr("Height");
    case COLUMN_MESSAGE:
      return tr("Message");
    case COLUMN_HASH:
      return tr("Transaction hash");
    case COLUMN_AMOUNT:
      return tr("Amount");
    case COLUMN_MESSAGE_SIZE:
      return tr("Message size");
    case COLUMN_FROM:
      return tr("From");
    case COLUMN_TO:
      return tr("To");
    default:
      break;
    }

  case ROLE_COLUMN:
    return _section;
  }

  return QVariant();
}

QVariant MessagesModel::data(const QModelIndex& _index, int _role) const {
  if(!_index.isValid()) {
    return QVariant();
  }


  CryptoNote::WalletLegacyTransaction transaction;
  CryptoNote::WalletLegacyTransfer transfer;
  CryptoNote::TransactionId transactionId = m_messages.value(_index.row()).transactionId;
  CryptoNote::TransferId transferId = m_messages.value(_index.row()).transferId;
  Message message = m_messages.value(_index.row()).message;
  

  WalletAdapter::instance().getTransaction(transactionId, transaction);
  WalletAdapter::instance().getTransfer(transferId, transfer);

  /*
  if(
    !WalletAdapter::instance().getTransaction(transactionId, transaction) ||   
    !WalletAdapter::instance().getTransfer(transferId, transfer)) {
    return QVariant();
  }
  */

  switch(_role) {
  case Qt::DisplayRole:
  case Qt::EditRole:
    return getDisplayRole(_index);

  case Qt::DecorationRole:{
    if (_index.column() == 5){
      return getDecorationRole(_index);

    }else{
      return getDecorationRole(_index);
    }
  }
  case Qt::FontRole:{
    switch(_index.column()) {    
      
      case COLUMN_DATE: {
      QFont messageFont;
      messageFont.setPointSize(8);
      messageFont.setFamily("Lato Light");
      messageFont.setItalic(true);
      return messageFont;
      }
      
      case COLUMN_FROM: {
      QFont messageFont;
      messageFont.setPointSize(9);
      messageFont.setFamily("Lato Light");
      return messageFont;
      }
      
      case COLUMN_TO: {
      QFont messageFont;
      messageFont.setPointSize(9);
      messageFont.setFamily("Lato Light");
      return messageFont;
      }
      

      case COLUMN_MESSAGE: {
      QFont messageFont;
      messageFont.setPointSize(9);
      messageFont.setFamily("Lato");
      return messageFont;
      }

      default:
        break;
    }

  }
  case Qt::BackgroundRole:{
//    if (_index.column() == 5){
//      QBrush redBackground(Qt::red);
//      return redBackground;
//    } 
  }

  case Qt::TextAlignmentRole:
    //return getAlignmentRole(_index);
      return Qt::AlignTop;

  default:
    return getUserRole(_index, _role, transactionId, transaction, transferId, transfer, message);
  }

  return QVariant();
}

QModelIndex MessagesModel::index(int _row, int _column, const QModelIndex& _parent) const {
  if(_parent.isValid()) {
    return QModelIndex();
  }

  return createIndex(_row, _column, _row);
}

QModelIndex MessagesModel::parent(const QModelIndex& _index) const {
  return QModelIndex();
}

QVariant MessagesModel::getDisplayRole(const QModelIndex& _index) const {
  switch(_index.column()) {


  case COLUMN_TYPE: {
    MessageType messageType = static_cast<MessageType>(_index.data(ROLE_TYPE).value<quint8>());
    if (messageType == MessageType::OUTPUT) {
      return tr("Sent");
    } else if(messageType == MessageType::INPUT) {
      return tr("Received");
    }

    return QVariant();
  }


  case COLUMN_DATE: {
    QDateTime date = _index.data(ROLE_DATE).toDateTime();
    return (date.isNull() || !date.isValid() ? "-" : _index.data(ROLE_DATE));
    //return (date.isNull() || !date.isValid() ? "-" : date.toString("h:m A\nM/d/yyyy"));
  }


  case COLUMN_HEIGHT: {
    quint64 height = _index.data(ROLE_HEIGHT).value<quint64>();
    return (height == CryptoNote::WALLET_LEGACY_UNCONFIRMED_TRANSACTION_HEIGHT ? "-" : QString::number(height));
  }

  case COLUMN_MESSAGE: {
    QString messageString = _index.data(ROLE_MESSAGE).toString();
    QTextStream messageStream(&messageString);
    //return messageStream.readLine();
    return messageString;
/*
    QLabel* messageLabel = new QLabel();
    messageLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    messageLabel->setText(messageString);
    messageLabel->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Preferred );
    messageLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    return QLabel(messageLabel->data()->toString());
*/
  }

  case COLUMN_HASH:
    return _index.data(ROLE_HASH).toByteArray().toHex().toUpper();

  case COLUMN_AMOUNT: {
    qint64 amount = _index.data(MessagesModel::ROLE_AMOUNT).value<qint64>();
    return CurrencyAdapter::instance().formatAmount(qAbs(amount));
  }

  case COLUMN_MESSAGE_SIZE:
    return QString::number(_index.data(ROLE_MESSAGE_SIZE).value<quint32>());

  case COLUMN_FULL_MESSAGE:
    return _index.data(ROLE_FULL_MESSAGE);

  case COLUMN_HAS_REPLY_TO:
    return !_index.data(ROLE_FROM_ADDRESS).toString().isEmpty();
  
  case COLUMN_FROM: 
    return _index.data(ROLE_FROM_LABEL).toString();

  case COLUMN_TO:
    return _index.data(ROLE_TO_LABEL).toString();

  default:
    break;
  }

  return QVariant();
}


QVariant MessagesModel::getDecorationRole(const QModelIndex& _index) const {
  switch(_index.column()) {
  case COLUMN_FROM: {
    if (!_index.data(ROLE_FROM_LABEL).toString().size()){
      QPixmap pixmap(":icons/anon");
      return pixmap.scaled(32, 32, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    }
    return QVariant();
    /*
    QLabel* messageLabel = new QLabel();
    messageLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    messageLabel->setText(_index.data(ROLE_MESSAGE).toString());
    messageLabel->setPixmap(pixmap);
    messageLabel->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Preferred );
    messageLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    return messageLabel;
    */
  }
  default:
    break;
  }

  return QVariant();
}

QVariant MessagesModel::getAlignmentRole(const QModelIndex& _index) const {
  return headerData(_index.column(), Qt::Horizontal, Qt::AlignTop);
}

QVariant MessagesModel::getUserRole(const QModelIndex& _index, int _role, CryptoNote::TransactionId _transactionId,
  CryptoNote::WalletLegacyTransaction& _transaction, CryptoNote::TransferId _transferId, const CryptoNote::WalletLegacyTransfer& _transfer,
  const Message& _message) const {
  switch(_role) {

      case ROLE_TYPE: {
        if(_transaction.totalAmount < 0) {
          return static_cast<quint8>(MessageType::OUTPUT);
        }
        return static_cast<quint8>(MessageType::INPUT);
      }

      case ROLE_DATE:
        return (_transaction.timestamp > 0 ? QDateTime::fromTime_t(_transaction.timestamp) : QDateTime());

      case ROLE_HEIGHT:
        return static_cast<quint64>(_transaction.blockHeight);

      case ROLE_MESSAGE:{
        return _message.getMessage();
      }

      case ROLE_FULL_MESSAGE:{
        return _message.getFullMessage();
      }
        
      case ROLE_HASH:
        return QByteArray(reinterpret_cast<char*>(&_transaction.hash), sizeof(_transaction.hash));

      case ROLE_AMOUNT:
        return static_cast<qint64>(_transaction.totalAmount);

      case ROLE_MESSAGE_SIZE:
        return _message.getMessage().size();

      case ROLE_ROW:
        return _index.row();

      case ROLE_FROM_LABEL: {
        if(WalletAdapter::instance().getAddress() == _index.data(ROLE_FROM_ADDRESS).toString()){
          return "Me";
        }else{
          return AddressBookModel::instance().getName(_index.data(ROLE_FROM_ADDRESS).toString());
        }
      }

      case ROLE_FROM_ADDRESS:
        return _message.getHeaderValue(HEADER_FROM_KEY);

      case ROLE_TO_LABEL:{
        MessageType messageType = static_cast<MessageType>(_index.data(ROLE_TYPE).value<quint8>());
        if(messageType == MessageType::INPUT) {
          return "Me";
        }else{
          return AddressBookModel::instance().getName(QString::fromStdString(_transfer.address));
        }
      }

      case ROLE_TO_ADDRESS:{
        MessageType messageType = static_cast<MessageType>(_index.data(ROLE_TYPE).value<quint8>());
        if(messageType == MessageType::INPUT) {
          return WalletAdapter::instance().getAddress();
        }else{
          return QString::fromStdString(_transfer.address);
        }
      }


  }

  return QVariant();
}

void MessagesModel::reloadWalletTransactions() {
  beginResetModel();
  //m_transactionRow.clear();
  m_messages.clear();
  endResetModel();

  quint32 rowCount = 0;
  for (CryptoNote::TransactionId transactionId = 0; transactionId < WalletAdapter::instance().getTransactionCount(); ++transactionId) {
    appendTransaction(transactionId, rowCount);
  }

  if (rowCount > 0) {
    beginInsertRows(QModelIndex(), 0, rowCount - 1);
    endInsertRows();
  }
}

void MessagesModel::appendTransaction(CryptoNote::TransactionId _transactionId, quint32& _insertedRowCount) {
  CryptoNote::WalletLegacyTransaction transaction;
  if (!WalletAdapter::instance().getTransaction(_transactionId, transaction)) {
    return;
  }

  for (quint32 i = 0; i < m_messages.size(); i++){
    if (m_messages[i].transactionId==_transactionId) {
      return;
    }
  }
  //m_transactionRow.insert(_transactionId, TransactionMessageId());
  //m_transactionRow[_transactionId] = TransactionMessageId();
  //m_transactionRow.insert(_transactionId, TransactionMessageId);

  if (transaction.messages.empty()) {
    return;
  }
/*
  m_transactionRow[_transactionId] = qMakePair(m_messages.size(), transaction.messages.size());
  for (quint32 i = 0; i < transaction.messages.size(); ++i) {
    Message message(QString::fromStdString(transaction.messages[i]));
    m_messages.append(TransactionMessageId(_transactionId, std::move(message)));
    ++_insertedRowCount;
  }
*/



  if (transaction.transferCount) {
    //m_transactionRow[_transactionId] = qMakePair(m_messages.size(), transaction.transferCount);
    //m_transactionRow[_transactionId] = struct(m_messages.size(), transaction.transferCount, transaction.messages.size());
    //m_transactionRow[_transactionId] = TransactionMessageId(_transactionId, transfer_id, std::move(message));
    for (CryptoNote::TransferId transfer_id = transaction.firstTransferId;
      transfer_id < transaction.firstTransferId + transaction.transferCount; ++transfer_id) {

      
      Message message(QString::fromStdString(transaction.messages[0]));

      TransactionMessageId _TransactionMessageId;
      _TransactionMessageId.transactionId=_transactionId;
      _TransactionMessageId.transferId=transfer_id;
      //_TransactionMessageId.transferId=transaction.firstTransferId;
      _TransactionMessageId.message=std::move(message);

      m_messages.append(_TransactionMessageId);

      ++_insertedRowCount;

    }
  }
  else{
/*  
  for (quint32 i = 0; i < transaction.messages.size(); ++i) {
      Message message(QString::fromStdString(transaction.messages[i]));
      TransactionMessageId _TransactionMessageId;
      _TransactionMessageId.transactionId=_transactionId;
      _TransactionMessageId.transferId=transaction.firstTransferId;
      _TransactionMessageId.message=std::move(message);

      m_messages.append(_TransactionMessageId);

      ++_insertedRowCount;
  }

*/


      Message message(QString::fromStdString(transaction.messages[0]));
      TransactionMessageId _TransactionMessageId;
      _TransactionMessageId.transactionId=_transactionId;
      _TransactionMessageId.transferId=CryptoNote::WALLET_LEGACY_INVALID_TRANSFER_ID;
      _TransactionMessageId.message=std::move(message);
      m_messages.append(_TransactionMessageId);
      ++_insertedRowCount;

  }



  // else {
//    m_messages.append(TransactionMessageId(_transactionId, CryptoNote::WALLET_LEGACY_INVALID_TRANSFER_ID));
    //m_transactionRow[_transactionId] = qMakePair(m_messages.size() - 1, 1);
    //m_transactionRow[_transactionId] = struct(m_messages.size() - 1, 1, 1);
  //  ++_insertedRowCount;
  //}  


  Q_EMIT messagesCountUpdatedSignal(m_messages.size());
}

void MessagesModel::appendTransaction(CryptoNote::TransactionId _transactionId) {
  //if (m_transactionRow.contains(_transactionId)) {
  
  for (quint32 i = 0; i < m_messages.size(); i++){
    if (m_messages[i].transactionId==_transactionId) {
      return;
    }
  }

  quint32 oldRowCount = rowCount();
  quint32 insertedRowCount = 0;
  for (quint64 transactionId = m_messages.size(); transactionId <= _transactionId; ++transactionId) {
    appendTransaction(transactionId, insertedRowCount);
  }

  if (insertedRowCount > 0) {
    beginInsertRows(QModelIndex(), oldRowCount, oldRowCount + insertedRowCount - 1);
    endInsertRows();
  }
}

void MessagesModel::updateWalletTransaction(CryptoNote::TransactionId _id) {
  //quint32 firstRow = m_messages.value(_id).first;
  //quint32 lastRow = firstRow + m_messages.value(_id).second - 1;
//  quint32 firstRow = m_transactionRow.value(_id).first;
//  quint32 lastRow = firstRow + m_transactionRow.value(_id).second - 1;
  //Q_EMIT dataChanged(index(firstRow, COLUMN_DATE), index(lastRow, COLUMN_HEIGHT));
}

void MessagesModel::lastKnownHeightUpdated(quint64 _height) {
  if(rowCount() > 0) {
    Q_EMIT dataChanged(index(0, COLUMN_DATE), index(rowCount() - 1, COLUMN_HEIGHT));
  }
}

void MessagesModel::reset() {
  beginResetModel();
  m_messages.clear();
//  m_transactionRow.clear();
  endResetModel();
}

}
