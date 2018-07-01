// Copyright (c) 2011-2016 The Cryptonote developers
// Copyright (c) 2015-2016 XDN developers
// Copyright (c) 2018-2019 xDrop developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <QFileDialog>
#include <QMessageBox>

#include "MessageDetailsDialog.h"
#include "MainWindow.h"
#include "MessagesModel.h"
#include "WalletAdapter.h"
#include "SendMessageFrame.h"

#include "ui_messagedetailsdialog.h"

namespace WalletGui {

MessageDetailsDialog::MessageDetailsDialog(const QModelIndex& _index, QWidget* _parent) : QDialog(_parent),
  m_ui(new Ui::MessageDetailsDialog) {
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
  m_ui->setupUi(this);
  m_modelIndex = MessagesModel::instance().index(_index.data(MessagesModel::ROLE_ROW).toInt(), 0);
  m_dataMapper.setModel(&MessagesModel::instance());
  m_dataMapper.addMapping(m_ui->m_heightLabel, MessagesModel::COLUMN_HEIGHT, "text");
  m_dataMapper.addMapping(m_ui->m_hashLabel, MessagesModel::COLUMN_HASH, "text");
  m_dataMapper.addMapping(m_ui->m_amountLabel, MessagesModel::COLUMN_AMOUNT, "text");
  m_dataMapper.addMapping(m_ui->m_sizeLabel, MessagesModel::COLUMN_MESSAGE_SIZE, "text");
  m_dataMapper.addMapping(m_ui->m_timeLabel, MessagesModel::COLUMN_DATE, "text");
  m_dataMapper.addMapping(m_ui->m_sentToLabel, MessagesModel::COLUMN_TO, "text");
  m_dataMapper.addMapping(m_ui->m_fromLabel, MessagesModel::COLUMN_FROM, "text");
  m_dataMapper.addMapping(m_ui->m_messageTextEdit, MessagesModel::COLUMN_MESSAGE, "plainText");
  m_dataMapper.setCurrentModelIndex(m_modelIndex);
}

MessageDetailsDialog::~MessageDetailsDialog() {
}

}
