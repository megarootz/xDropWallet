// Copyright (c) 2011-2016 The Cryptonote developers
// Copyright (c) 2015-2016 XDN developers
// Copyright (c) 2018-2019 xDrop developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <QSortFilterProxyModel>
#include "AddressBookDialog.h"
#include "AddressBookModel.h"

#include "ui_addressbookdialog.h"

namespace WalletGui {

AddressBookDialog::AddressBookDialog(QWidget* _parent) : QDialog(_parent), m_ui(new Ui::AddressBookDialog) {
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

  m_ui->setupUi(this);

  QSortFilterProxyModel *proxyModel = new QSortFilterProxyModel(this);
  proxyModel->setSourceModel(&AddressBookModel::instance());
  m_ui->m_addressBookView->setModel(proxyModel);
  proxyModel->setSortRole( AddressBookModel::ROLE_LABEL );
  proxyModel->sort( 0, Qt::AscendingOrder );

  if (AddressBookModel::instance().rowCount() > 0) {
    m_ui->m_addressBookView->setCurrentIndex(proxyModel->index(0, 0));
  }
}

AddressBookDialog::~AddressBookDialog() {
}

QString AddressBookDialog::getAddress() const {
  QString addressbookName = m_ui->m_addressBookView->currentIndex().data(AddressBookModel::ROLE_LABEL).toString();
  QString addressbookAddress = m_ui->m_addressBookView->currentIndex().data(AddressBookModel::ROLE_ADDRESS).toString();
  return QString("%1 <%2>").arg(addressbookName).arg(addressbookAddress);
}

}
