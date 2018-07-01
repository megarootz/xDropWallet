// Copyright (c) 2011-2016 The Cryptonote developers
// Copyright (c) 2015-2016 XDN developers
// Copyright (c) 2018-2019 xDrop developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <QAbstractItemModel>
#include <QSortFilterProxyModel>
#include <QJsonArray>
#include <IWalletLegacy.h>

#include "Message.h"

namespace WalletGui {

//typedef QPair<CryptoNote::TransactionId, Message> TransactionMessageId;
//typedef QPair<CryptoNote::TransactionId, CryptoNote::TransferId> TransactionMessageId;
  //typedef QPair<CryptoNote::TransactionId, CryptoNote::TransferId> TransactionMessageId;
  typedef struct TransactionMessageId{
    CryptoNote::TransactionId transactionId;
    CryptoNote::TransferId transferId;
    Message message;
  };


class MessagesModel : public QAbstractItemModel {
  Q_OBJECT
  Q_ENUMS(Columns)
  Q_ENUMS(Roles)

public:
  enum Columns{
    COLUMN_TYPE = 0, COLUMN_DATE, COLUMN_FROM, COLUMN_TO, COLUMN_HEIGHT, COLUMN_MESSAGE, COLUMN_HASH, COLUMN_AMOUNT, COLUMN_MESSAGE_SIZE,
      COLUMN_FULL_MESSAGE, COLUMN_HAS_REPLY_TO
  };

  enum Roles {
    ROLE_DATE = Qt::UserRole, ROLE_TYPE, ROLE_HEIGHT, ROLE_MESSAGE, ROLE_FULL_MESSAGE, ROLE_COLUMN, ROLE_ROW,
      ROLE_HASH, ROLE_AMOUNT, ROLE_MESSAGE_SIZE, ROLE_FROM_LABEL, ROLE_FROM_ADDRESS, ROLE_TO_LABEL, ROLE_TO_ADDRESS
  };

  static const QString HEADER_FROM_KEY;
  quint64 MESSAGES_READ_COUNT;

  static MessagesModel& instance();

  Qt::ItemFlags flags(const QModelIndex& _index) const Q_DECL_OVERRIDE;
  int columnCount(const QModelIndex& _parent = QModelIndex()) const Q_DECL_OVERRIDE;
  int rowCount(const QModelIndex& _parent = QModelIndex()) const Q_DECL_OVERRIDE;

  QVariant headerData(int _section, Qt::Orientation _orientation, int _role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
  QVariant data(const QModelIndex& _index, int _role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
  QModelIndex index(int _row, int _column, const QModelIndex& _parent = QModelIndex()) const Q_DECL_OVERRIDE;
  QModelIndex	parent(const QModelIndex& _index) const Q_DECL_OVERRIDE;

private:
  QVector<TransactionMessageId> m_messages;
  //QVector<TransactionMessageId> m_messages;
  //TransactionMessageId m_messages;
  //QHash<CryptoNote::TransactionId, QPair<quint32, quint32> > m_transactionRow;
  //QHash<TransactionMessageId> m_transactionRow;
  QJsonArray m_addressBook;

  MessagesModel();
  ~MessagesModel();

  QVariant getDisplayRole(const QModelIndex& _index) const;
  QVariant getDecorationRole(const QModelIndex& _index) const;
  QVariant getAlignmentRole(const QModelIndex& _index) const;
  QVariant getUserRole(const QModelIndex& _index, int _role, CryptoNote::TransactionId _transactionId, CryptoNote::WalletLegacyTransaction& _transaction, CryptoNote::TransferId _transferId, const CryptoNote::WalletLegacyTransfer& _transfer, const Message& _message) const;

  void reloadWalletTransactions();
  void appendTransaction(CryptoNote::TransactionId _id, quint32& _row_count);
  void appendTransaction(CryptoNote::TransactionId _id);
  void updateWalletTransaction(CryptoNote::TransactionId _id);
  void lastKnownHeightUpdated(quint64 _height);
  void reset();

Q_SIGNALS:
  void messagesCountUpdatedSignal(quint64 _messagesCount);



};

}
