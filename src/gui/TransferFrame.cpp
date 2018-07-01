// Copyright (c) 2011-2016 The Cryptonote developers
// Copyright (c) 2015-2016 XDN developers
// Copyright (c) 2018-2019 xDrop developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <QApplication>
#include <QClipboard>

#include "TransferFrame.h"
#include "AddressBookModel.h"
#include "AddressBookDialog.h"
#include "MainWindow.h"
#include "CurrencyAdapter.h"

#include "ui_transferframe.h"

namespace WalletGui {


TransferFrame::TransferFrame(QWidget* _parent) : QFrame(_parent), m_ui(new Ui::TransferFrame){
  m_ui->setupUi(this);
  setAttribute(Qt::WA_DeleteOnClose);
  m_ui->m_amountSpin->setSuffix(" " + CurrencyAdapter::instance().getCurrencyTicker().toUpper());
  connect(m_ui->m_addressEdit, &QLineEdit::textChanged, this, &TransferFrame::extractAddress);  
}

TransferFrame::~TransferFrame() {
}

QString TransferFrame::getAddress() const {
  QString address = m_ui->m_addressEdit->text().trimmed();
  if (address.contains('<')) {
    int startPos = address.indexOf('<');
    int endPos = address.indexOf('>');
    address = address.mid(startPos + 1, endPos - startPos - 1);
  }

  return address;
}

QString TransferFrame::getLabel() const {
  return m_ui->m_labelEdit->text().trimmed();
}

qreal TransferFrame::getAmount() const {
  return m_ui->m_amountSpin->value();
}

QString TransferFrame::getAmountString() const {
  return m_ui->m_amountSpin->cleanText();
}

QString TransferFrame::getComment() const {
  return m_ui->m_transactionCommentEdit->text();
}

void TransferFrame::setCompleter(QCompleter* _completer) {
  m_ui->m_addressEdit->setCompleter(_completer);
}

void TransferFrame::disableRemoveButton(bool _disable) {
  m_ui->m_removeButton->setDisabled(_disable);
}

void TransferFrame::setAddress(const QString& _address) {
  m_ui->m_addressEdit->setText(_address);
  m_ui->m_addressEdit->setCursorPosition(0);
  m_ui->m_addressEdit->clearFocus();  
}

void TransferFrame::extractAddress() {
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

void TransferFrame::addressBookClicked() {
  AddressBookDialog dlg(&MainWindow::instance());
  if(dlg.exec() == QDialog::Accepted) {
    m_ui->m_addressEdit->setText(dlg.getAddress());
    m_ui->m_addressEdit->setCursorPosition(0);
    m_ui->m_addressEdit->clearFocus();      
  }
}

void TransferFrame::commentEdited(const QString& _text) {
  Q_EMIT commentEditedSignal();
}

void TransferFrame::addressEdited(const QString& _text) {
  Q_EMIT addressEditedSignal();
}

void TransferFrame::pasteClicked() {
  m_ui->m_addressEdit->setText(QApplication::clipboard()->text());
}

}
