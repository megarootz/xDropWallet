// Copyright (c) 2011-2016 The Cryptonote developers
// Copyright (c) 2015-2016 XDN developers
// Copyright (c) 2018-2019 xDrop developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "NewAddressDialog.h"

#include "ui_newaddressdialog.h"

namespace WalletGui {

NewAddressDialog::NewAddressDialog(QWidget* _parent) : QDialog(_parent), m_ui(new Ui::NewAddressDialog) {
 	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
  m_ui->setupUi(this);
}

NewAddressDialog::~NewAddressDialog() {
}

QString NewAddressDialog::getAddress() const {
  return m_ui->m_addressEdit->text();
}


QString NewAddressDialog::getLabel() const {
  return m_ui->m_labelEdit->text();
}

void NewAddressDialog::setAddress(QString _address) const {
  m_ui->m_addressEdit->setText(_address);
  m_ui->m_addressEdit->setCursorPosition(0);
  m_ui->m_addressEdit->clearFocus();
}

void NewAddressDialog::setLabel(QString _label) const {
  m_ui->m_labelEdit->setText(_label);
  m_ui->m_labelEdit->setCursorPosition(0);
  m_ui->m_labelEdit->clearFocus();
}

}
