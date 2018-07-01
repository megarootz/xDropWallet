// Copyright (c) 2011-2016 The Cryptonote developers
// Copyright (c) 2015-2016 XDN developers
// Copyright (c) 2018-2019 xDrop developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <QFrame>
#include <QCompleter>

namespace Ui {
  class MessageAddressFrame;
}

namespace WalletGui {



class MessageAddressFrame : public QFrame {
  Q_OBJECT
  Q_DISABLE_COPY(MessageAddressFrame)

public:
  MessageAddressFrame(QWidget* _parent);
  ~MessageAddressFrame();

  QCompleter* ModelCompleter;

  QString getAddress() const;
  void disableRemoveButton(bool _disable);
  void setAddress(QString _address);
  void setCompleter(QCompleter* _completer);

    
private:
  QScopedPointer<Ui::MessageAddressFrame> m_ui;
  
  Q_SLOT void addressBookClicked();
  Q_SLOT void pasteClicked();
  Q_SLOT void extractAddress();
  Q_SLOT void addressEdited(const QString& _text);
  
Q_SIGNALS:
  void addressEditedSignal();
};

}
