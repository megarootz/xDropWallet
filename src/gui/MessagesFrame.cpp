// Copyright (c) 2011-2016 The Cryptonote developers
// Copyright (c) 2015-2016 XDN developers
// Copyright (c) 2018-2019 xDrop developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <QApplication>
#include <QMessageBox>

#include "MessagesFrame.h"
#include "MainWindow.h"
#include "MessageDetailsDialog.h"
#include "MessagesModel.h"
#include "SortedMessagesModel.h"
#include "VisibleMessagesModel.h"
#include "WalletAdapter.h"
#include "AddressBookModel.h"
#include "ItemDelegatePaint.h"



#include "ui_messagesframe.h"

namespace WalletGui {

MessagesFrame::MessagesFrame(QWidget* _parent) : QFrame(_parent), m_ui(new Ui::MessagesFrame),
  m_visibleMessagesModel(new VisibleMessagesModel){
  m_ui->setupUi(this);
  m_ui->m_messagesView->setModel(m_visibleMessagesModel.data());
  m_ui->m_replyButton->setEnabled(false);
  m_ui->m_detailsButton->setEnabled(false);
  m_ui->m_addContactButton->setEnabled(false);
  m_ui->m_messagesView->setSelectionMode(QAbstractItemView::SingleSelection);
  m_ui->m_messagesView->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_ui->m_messagesView->horizontalHeader()->setStretchLastSection(true);
  m_ui->m_messagesView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
  m_ui->m_messagesView->horizontalHeader()->setHighlightSections(false);
  m_ui->m_messagesView->verticalHeader()->setStretchLastSection(true);
  m_ui->m_messagesView->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
  m_ui->m_messagesView->verticalHeader()->hide();
  m_ui->m_messagesView->resizeColumnsToContents();
  m_ui->m_messagesView->resizeRowsToContents();
  m_ui->m_messagesView->setItemDelegate(new ItemDelegatePaint);
  m_ui->m_replyButton->setShortcut(QKeySequence(tr("r")));
  m_ui->m_replyButton->style()->styleHint(QStyle::SH_UnderlineShortcut);

/*  
  QModelIndex _index = m_ui->m_messagesView->selectionModel()->currentIndex();
  connect(m_dlg, SIGNAL(prevClickedSignal()), this, SLOT(gotoPrev()));
  connect(m_dlg, SIGNAL(nextClickedSignal()), this, SLOT(gotoNext()));
  connect(m_dlg, SIGNAL(replyToSignal()), this, SLOT(replyClicked()));
*/
  connect(m_ui->m_messagesView->selectionModel(), &QItemSelectionModel::currentChanged, this, &MessagesFrame::currentMessageChanged);
}

MessagesFrame::~MessagesFrame() {
}

void MessagesFrame::currentMessageChanged(const QModelIndex& _currentIndex) {
  m_ui->m_addContactButton->setEnabled(
    _currentIndex.isValid() //Message is valid
    && 
    ((
      _currentIndex.data(MessagesModel::ROLE_TO_ADDRESS).toString().size() //Receivers address is known
      &&
      !AddressBookModel::instance().getName(_currentIndex.data(MessagesModel::ROLE_TO_ADDRESS).toString()).toString().size() //Receivers address not in addressbook
      &&
      WalletAdapter::instance().getAddress() != _currentIndex.data(MessagesModel::ROLE_TO_ADDRESS).toString() //Receiver is not us
    )
    ||
    (
      _currentIndex.data(MessagesModel::ROLE_FROM_ADDRESS).toString().size() //Sender address is known
      &&
      !AddressBookModel::instance().getName(_currentIndex.data(MessagesModel::ROLE_FROM_ADDRESS).toString()).toString().size() //Senders address not in addressbook
      &&
      WalletAdapter::instance().getAddress() != _currentIndex.data(MessagesModel::ROLE_FROM_ADDRESS).toString() //Senders is not us
    ))
  );

  m_ui->m_replyButton->setFocus();
  m_ui->m_replyButton->setEnabled(_currentIndex.isValid() && !_currentIndex.data(MessagesModel::ROLE_FROM_ADDRESS).toString().isEmpty() && 
    WalletAdapter::instance().getAddress() != _currentIndex.data(MessagesModel::ROLE_FROM_ADDRESS).toString());

  m_ui->m_detailsButton->setEnabled(_currentIndex.isValid());
}

void MessagesFrame::messageDoubleClicked(const QModelIndex& _index) {
  if (!_index.isValid()) {
    return;
  }
  if (_index.data(MessagesModel::ROLE_FROM_ADDRESS).toString().size() > 0){
    if (WalletAdapter::instance().getAddress() == _index.data(MessagesModel::ROLE_FROM_ADDRESS).toString()){
      m_ui->m_detailsButton->click();    
      return;
    }else{
      m_ui->m_replyButton->click();    
      return;
    }
  }else{
    m_ui->m_detailsButton->click();    
    return;
  }
}

void MessagesFrame::replyClicked() {
  QModelIndex _index = m_ui->m_messagesView->selectionModel()->currentIndex();
  if (!_index.isValid()) {
    return;
  }
  Q_EMIT replyToSignal(_index);
}


void MessagesFrame::gotoPrev() {
  m_ui->m_messagesView->selectionModel()->select (
                    QItemSelection (
                        m_ui->m_messagesView->model()->index(m_ui->m_messagesView->currentIndex().row() - 1, 0),
                        m_ui->m_messagesView->model()->index(m_ui->m_messagesView->currentIndex().row() - 1, m_ui->m_messagesView->model ()->columnCount ()))
                    ,
                    QItemSelectionModel::Select
                                                  ); 
}

void MessagesFrame::gotoNext() {
  m_ui->m_messagesView->selectionModel()->select (
                    QItemSelection (
                        m_ui->m_messagesView->model()->index(m_ui->m_messagesView->currentIndex().row() , 0),
                        m_ui->m_messagesView->model()->index(m_ui->m_messagesView->currentIndex().row() + 1, m_ui->m_messagesView->model ()->columnCount ()))
                    ,
                    QItemSelectionModel::Select
                                                  );  
}

void MessagesFrame::showDetailsClicked() {
  QModelIndex _index = m_ui->m_messagesView->selectionModel()->currentIndex();
  if (!_index.isValid()) {
    return;
  }
  MessageDetailsDialog dlg(_index, &MainWindow::instance());
  if(dlg.exec()) return;
}

void MessagesFrame::addContactClicked() {
  QModelIndex _index = m_ui->m_messagesView->selectionModel()->currentIndex();
  if (!_index.isValid()) {
    return;
  }
  Q_EMIT saveContactSignal(_index);
}



}
