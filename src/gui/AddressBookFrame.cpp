// Copyright (c) 2011-2016 The Cryptonote developers
// Copyright (c) 2015-2016 XDN developers
// Copyright (c) 2018-2019 xDrop developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <QApplication>
#include <QClipboard>
#include <QSortFilterProxyModel>
#include <QtWidgets>

#include "CurrencyAdapter.h"
#include "AddressBookModel.h"
#include "AddressBookFrame.h"
#include "MainWindow.h"
#include "NewAddressDialog.h"
#include "WalletEvents.h"

#include "ui_addressbookframe.h"

namespace WalletGui {

AddressBookFrame::AddressBookFrame(QWidget* _parent) : QFrame(_parent), m_ui(new Ui::AddressBookFrame) {
  m_ui->setupUi(this);


  proxyModel = new QSortFilterProxyModel(this);
  proxyModel->setSourceModel(&AddressBookModel::instance());
  m_ui->m_addressBookView->setModel(proxyModel);
  proxyModel->setDynamicSortFilter(true);
  proxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
  proxyModel->sort(0, Qt::AscendingOrder);

  connect(m_ui->m_addressBookView->selectionModel(), &QItemSelectionModel::currentChanged, this, &AddressBookFrame::currentAddressChanged);
}

AddressBookFrame::~AddressBookFrame() {
}

void AddressBookFrame::addClicked(QString _address) {
  NewAddressDialog dlg(&MainWindow::instance());
  dlg.setAddress(_address);
  if (dlg.exec() == QDialog::Accepted) {
    QString label = dlg.getLabel();
    QString address = dlg.getAddress();
    if (!CurrencyAdapter::instance().validateAddress(address)) {
      QCoreApplication::postEvent(&MainWindow::instance(), new ShowMessageEvent(tr("Invalid address"), QtCriticalMsg));
      return;
    } else if (label.trimmed().isEmpty()) {
      QCoreApplication::postEvent(&MainWindow::instance(), new ShowMessageEvent(tr("Name cannot be empty"), QtCriticalMsg));
      return;
    }

    AddressBookModel::instance().addEntry(label, address);
  }
}

void AddressBookFrame::copyClicked() {
  QApplication::clipboard()->setText(m_ui->m_addressBookView->currentIndex().data(AddressBookModel::ROLE_ADDRESS).toString());
  QCoreApplication::postEvent(&MainWindow::instance(), new ShowMessageEvent(tr("Successfully copied to your clipboard"), QtInfoMsg));
}

void AddressBookFrame::payClicked() {
  Q_EMIT payToSignal(m_ui->m_addressBookView->currentIndex());
}

void AddressBookFrame::msgClicked() {
  Q_EMIT replyToSignal(m_ui->m_addressBookView->currentIndex());
}

void AddressBookFrame::deleteClicked() {
  QItemSelectionModel *selectionModel = m_ui->m_addressBookView->selectionModel();
  QModelIndexList indexes = selectionModel->selectedRows();
  foreach (QModelIndex index, indexes) {
      int row = proxyModel->mapToSource(index).row();
      AddressBookModel::instance().removeRows(row, 1, QModelIndex());
  }
}

void AddressBookFrame::currentAddressChanged(const QModelIndex& _index) {
  m_ui->m_copyAddressButton->setEnabled(_index.isValid());
  m_ui->m_deleteAddressButton->setEnabled(_index.isValid());
  m_ui->m_payAddressButton->setEnabled(_index.isValid());
  m_ui->m_msgAddressButton->setEnabled(_index.isValid());
}

}
