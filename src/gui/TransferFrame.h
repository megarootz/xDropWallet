// Copyright (c) 2011-2016 The Cryptonote developers
// Copyright (c) 2015-2016 XDN developers
// Copyright (c) 2018-2019 xDrop developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <QFrame>
#include <QCompleter>

namespace Ui {
class TransferFrame;
}

namespace WalletGui {

class TransferFrame : public QFrame {
  Q_OBJECT
  Q_DISABLE_COPY(TransferFrame)

public:
  TransferFrame(QWidget* _parent);
  ~TransferFrame();

  QCompleter* ModelCompleter;

  QString getAddress() const;
  QString getLabel() const;
  qreal getAmount() const;
  QString getAmountString() const;
  QString getComment() const;

  void disableRemoveButton(bool _disable);
  void setAddress(const QString& _address);
  void setCompleter(QCompleter* _completer);


private:
  QScopedPointer<Ui::TransferFrame> m_ui;

  Q_SLOT void addressBookClicked();
  Q_SLOT void commentEdited(const QString& _text);
  Q_SLOT void addressEdited(const QString& _text);
  Q_SLOT void pasteClicked();
  Q_SLOT void extractAddress();

Q_SIGNALS:
  void commentEditedSignal();
  void addressEditedSignal();
};

}
