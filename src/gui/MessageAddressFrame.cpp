// Copyright (c) 2011-2016 The Cryptonote developers
// Copyright (c) 2015-2016 XDN developers
// Copyright (c) 2018-2019 xDrop developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <QApplication>
#include <QClipboard>
#include <QCompleter>
#include <QJsonDocument>
#include <QJsonObject>

#include "MessageAddressFrame.h"
#include "AddressBookModel.h"
#include "AddressBookDialog.h"
#include "MainWindow.h"
#include "ui_messageaddressframe.h"
#include "Settings.h"

namespace WalletGui {

namespace {
  Q_DECL_CONSTEXPR quint32 MESSAGE_ADDRESS_INPUT_INTERVAL = 1500;
}

MessageAddressFrame::MessageAddressFrame(QWidget* _parent) : QFrame(_parent),
  m_ui(new Ui::MessageAddressFrame){
  m_ui->setupUi(this);
  connect(m_ui->m_addressEdit, &QLineEdit::textChanged, this, &MessageAddressFrame::extractAddress);
}

MessageAddressFrame::~MessageAddressFrame() {

}

QString MessageAddressFrame::getAddress() const {
  return m_ui->m_addressEdit->text();
}

void MessageAddressFrame::setAddress(QString _address) {
  m_ui->m_addressEdit->setText(_address);
  m_ui->m_addressEdit->setCursorPosition(0);
  m_ui->m_addressEdit->clearFocus();
}

void MessageAddressFrame::extractAddress() {
  QList<Contact> contacts = AddressBookModel::instance().getContacts();
  QString _string = m_ui->m_addressEdit->text();

  for (const auto &contact: qAsConst(contacts)){
    if (contact.name.toLower() == _string.toLower()){
      m_ui->m_addressEdit->setText(QString("%1 <%2>").arg(contact.name).arg(contact.address));
      m_ui->m_addressEdit->setCursorPosition(0);
      m_ui->m_addressEdit->clearFocus();
    }
  }
}

void MessageAddressFrame::setCompleter(QCompleter* _completer) {
  m_ui->m_addressEdit->setCompleter(_completer);
}

void MessageAddressFrame::disableRemoveButton(bool _disable) {
  m_ui->m_removeButton->setDisabled(_disable);
}

void MessageAddressFrame::addressBookClicked() {
  AddressBookDialog dlg(&MainWindow::instance());
  if(dlg.exec() == QDialog::Accepted) {
    m_ui->m_addressEdit->setText(dlg.getAddress());
    m_ui->m_addressEdit->setCursorPosition(0);
    m_ui->m_addressEdit->clearFocus();
  }
}

void MessageAddressFrame::pasteClicked() {
  m_ui->m_addressEdit->setText(QApplication::clipboard()->text());
}

void MessageAddressFrame::addressEdited(const QString& _text) {
  Q_EMIT addressEditedSignal();
}

}
