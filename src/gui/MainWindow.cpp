// Copyright (c) 2011-2016 The Cryptonote developers
// Copyright (c) 2015-2016 XDN developers
// Copyright (c) 2018-2019 xDrop developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <QApplication>
#include <QCloseEvent>
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QSystemTrayIcon>
#include <QTimer>
#include <QTextStream>

#include <QPaintEvent>
#include <QPixmap>
#include <QPainter>
#include <QRect>
#include <QIcon>
#include <QFontDatabase>

#include <Common/Base58.h>
#include <Common/Util.h>

#include "AboutDialog.h"
#include "AddressBookModel.h"
#include "ChangePasswordDialog.h"
#include "CurrencyAdapter.h"
#include "ExitWidget.h"
#include "ImportKeyDialog.h"
#include "MainWindow.h"
#include "MessagesModel.h"
#include "MessagesButton.h"
#include "NewPasswordDialog.h"
#include "NodeAdapter.h"
#include "PasswordDialog.h"
#include "Settings.h"
#include "WalletAdapter.h"
#include "WalletEvents.h"
#include "TransactionsModel.h"
#include "ui_mainwindow.h"


namespace WalletGui {

MainWindow* MainWindow::m_instance = nullptr;

quint16 MESSAGES_READ_COUNT = 0;

MainWindow& MainWindow::instance() {
  if (m_instance == nullptr) {
    m_instance = new MainWindow;
  }

  return *m_instance;
}

MainWindow::MainWindow() : QMainWindow(), m_ui(new Ui::MainWindow), m_trayIcon(nullptr), m_tabActionGroup(new QActionGroup(this)),
  m_isAboutToQuit(false) {
  setWindowFlags(Qt::Window);
  m_ui->setupUi(this);
  m_connectionStateIconLabel = new QLabel(this);
  m_encryptionStateIconLabel = new QLabel(this);
  m_synchronizationStateIconLabel = new QLabel(this);

  connectToSignals();
  initUi();

  walletClosed();
}

MainWindow::~MainWindow() {
}

void MainWindow::connectToSignals() {
  connect(&WalletAdapter::instance(), &WalletAdapter::openWalletWithPasswordSignal, this, &MainWindow::askForWalletPassword, Qt::QueuedConnection);
  connect(&WalletAdapter::instance(), &WalletAdapter::walletSynchronizationProgressUpdatedSignal, this, &MainWindow::walletSynchronizationInProgress,
    Qt::QueuedConnection);
  connect(&WalletAdapter::instance(), &WalletAdapter::walletSynchronizationCompletedSignal, this, &MainWindow::walletSynchronized,
    Qt::QueuedConnection);
  connect(&WalletAdapter::instance(), &WalletAdapter::walletStateChangedSignal, this, &MainWindow::setStatusBarText);
  connect(&WalletAdapter::instance(), &WalletAdapter::walletInitCompletedSignal, this, &MainWindow::walletOpened);
  connect(&WalletAdapter::instance(), &WalletAdapter::walletCloseCompletedSignal, this, &MainWindow::walletClosed);
  connect(&WalletAdapter::instance(), &WalletAdapter::walletTransactionCreatedSignal, this, [this](CryptoNote::TransactionId _transactionId) {
  #ifdef Q_OS_WIN
    CryptoNote::WalletLegacyTransaction transaction;
    CryptoNote::WalletLegacyTransfer transfer;

    if (WalletAdapter::instance().getTransaction(_transactionId, transaction)) {
     if (transaction.totalAmount > 0){
      if (transaction.messages.size() > 0){
        if (Settings::instance().isMessageNotificationsEnabled()){
          QString fromString;
          QString messageString = Message(QString::fromStdString(transaction.messages[0])).getMessage();
          QString fromAddressString = Message(QString::fromStdString(transaction.messages[0])).getHeaderValue(MessagesModel::HEADER_FROM_KEY);
          if (fromAddressString.size()) fromString = AddressBookModel::instance().getName(fromAddressString).toString();
          else fromString = QString(tr("New Message"));
          m_trayIcon->show();
          m_trayIcon->showMessage(
            fromString,
            messageString, 
            QPixmap(":icons/messages"), 
            15000
          );
        }
      }else{
        if (Settings::instance().isPaymentNotificationsEnabled()){
          QApplication::alert(this);
          m_trayIcon->show();
          m_trayIcon->showMessage(
            QString(tr("New Pending Transaction")), 
            QString(tr("Received %1 %2")).arg(QVariant(CurrencyAdapter::instance().formatAmount(transaction.totalAmount)).toString()).arg(CurrencyAdapter::instance().getCurrencyTicker().toUpper()), 
            QPixmap(":icons/receive"), 
            15000
          );
        }
      }

     }
    }
  #endif    
  });

  connect(&WalletAdapter::instance(), &WalletAdapter::walletSendTransactionCompletedSignal, this, [this](CryptoNote::TransactionId _transactionId, int _error, const QString& _errorString) {
    if (_error == 0) {
      m_ui->m_transactionsAction->trigger();
    }
  });

  connect(&WalletAdapter::instance(), &WalletAdapter::walletSendMessageCompletedSignal, this, [this](CryptoNote::TransactionId _transactionId, int _error, const QString& _errorString) {
    if (_error == 0) {
      m_ui->m_messagesAction->trigger();
    }
  });

  connect(&NodeAdapter::instance(), &NodeAdapter::peerCountUpdatedSignal, this, &MainWindow::peerCountUpdated, Qt::QueuedConnection);
  connect(&MessagesModel::instance(), &MessagesModel::messagesCountUpdatedSignal, this, &MainWindow::messagesCountUpdated, Qt::QueuedConnection);
  connect(m_ui->m_exitAction, &QAction::triggered, qApp, &QApplication::quit);
  connect(m_ui->m_messagesFrame, &MessagesFrame::replyToSignal, this, &MainWindow::replyTo);  
  connect(m_ui->m_messagesFrame, &MessagesFrame::saveContactSignal, this, &MainWindow::saveContact);
  connect(m_ui->m_addressBookFrame, &AddressBookFrame::replyToSignal, this, &MainWindow::sendTo);
  connect(m_ui->m_addressBookFrame, &AddressBookFrame::payToSignal, this, &MainWindow::payTo);
}

void MainWindow::initUi() {
  setWindowTitle(QString("%1 Wallet").arg(CurrencyAdapter::instance().getCurrencyDisplayName()));
#ifdef Q_OS_WIN
  if (QSystemTrayIcon::isSystemTrayAvailable()) {
    m_trayIcon = new QSystemTrayIcon(QPixmap(":images/logo-ico"), this);
    m_trayIcon->setToolTip(QString("%1 Wallet %2").arg(CurrencyAdapter::instance().getCurrencyDisplayName()).arg(Settings::instance().getVersion()));
    connect(m_trayIcon, &QSystemTrayIcon::messageClicked, this, &MainWindow::messageClicked);
    connect(m_trayIcon, &QSystemTrayIcon::activated, this, &MainWindow::trayActivated);
    m_ui->m_closeToTrayAction->setChecked(Settings::instance().isCloseToTrayEnabled());
    m_ui->m_minimizeToTrayAction->setChecked(Settings::instance().isMinimizeToTrayEnabled());
  }
  m_ui->m_paymentNotificationsAction->setChecked(Settings::instance().isPaymentNotificationsEnabled());
  m_ui->m_messageNotificationsAction->setChecked(Settings::instance().isMessageNotificationsEnabled());
#endif

  m_ui->m_startOnLoginAction->setChecked(Settings::instance().isStartOnLoginEnabled());


  MESSAGES_READ_COUNT = Settings::instance().getReadMessages();

  QFontDatabase::applicationFontFamilies(QFontDatabase::addApplicationFont(":/fonts/lato-regular")).at(0);
  QFontDatabase::applicationFontFamilies(QFontDatabase::addApplicationFont(":/fonts/lato-black")).at(0);
  QFontDatabase::applicationFontFamilies(QFontDatabase::addApplicationFont(":/fonts/lato-bold")).at(0);
  QFontDatabase::applicationFontFamilies(QFontDatabase::addApplicationFont(":/fonts/lato-light")).at(0);
  QFontDatabase::applicationFontFamilies(QFontDatabase::addApplicationFont(":/fonts/lato-lighti")).at(0);
  QFontDatabase::applicationFontFamilies(QFontDatabase::addApplicationFont(":/fonts/lato-hair")).at(0);
  QFontDatabase::applicationFontFamilies(QFontDatabase::addApplicationFont(":/fonts/lato-hairi")).at(0);
  QFontDatabase::applicationFontFamilies(QFontDatabase::addApplicationFont(":/fonts/lato-regular")).at(0);
  QFontDatabase::applicationFontFamilies(QFontDatabase::addApplicationFont(":/fonts/lato-bold")).at(0);
  QFontDatabase::applicationFontFamilies(QFontDatabase::addApplicationFont(":/fonts/lato-black")).at(0);

  m_ui->m_aboutCryptonoteAction->setText(QString(tr("About %1 Wallet")).arg(CurrencyAdapter::instance().getCurrencyDisplayName()));

  m_ui->m_overviewFrame->hide();
  m_ui->m_sendFrame->hide();
  m_ui->m_receiveFrame->hide();
  m_ui->m_transactionsFrame->hide();
  m_ui->m_addressBookFrame->hide();
  m_ui->m_messagesFrame->hide();
  m_ui->m_sendMessageFrame->hide();
  m_ui->m_miningFrame->hide();
  m_ui->m_depositsFrame->hide();

  m_tabActionGroup->addAction(m_ui->m_overviewAction);
  m_tabActionGroup->addAction(m_ui->m_sendAction);
  m_tabActionGroup->addAction(m_ui->m_receiveAction);
  m_tabActionGroup->addAction(m_ui->m_transactionsAction);
  m_tabActionGroup->addAction(m_ui->m_addressBookAction);
  m_tabActionGroup->addAction(m_ui->m_messagesAction);
  m_tabActionGroup->addAction(m_ui->m_sendMessageAction);
  m_tabActionGroup->addAction(m_ui->m_miningAction);
  m_tabActionGroup->addAction(m_ui->m_depositsAction);

  connect(m_tabActionGroup, SIGNAL(triggered(QAction*)), SLOT(switchMenu(QAction*)));

  m_ui->toolBar->setOrientation(Qt::Vertical);
  m_ui->toolBar->setContextMenuPolicy(Qt::PreventContextMenu);

  m_ui->m_messagesAction->toggle();
  m_ui->m_overviewAction->toggle();

  encryptedFlagChanged(false);
  statusBar()->addPermanentWidget(m_connectionStateIconLabel);
  statusBar()->addPermanentWidget(m_encryptionStateIconLabel);
  statusBar()->addPermanentWidget(m_synchronizationStateIconLabel);
  m_connectionStateIconLabel->setPixmap(QPixmap(":icons/disconnected").scaled(16, 16, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
  
#ifdef Q_OS_MAC
  installDockHandler();
#endif

#ifndef Q_OS_WIN
  m_ui->m_minimizeToTrayAction->deleteLater();
  m_ui->m_closeToTrayAction->deleteLater();
#endif
}

#ifdef Q_OS_WIN
void MainWindow::minimizeToTray(bool _on) {
  if (_on) {
    hide();
    m_trayIcon->show();
  } else {
    showNormal();
    activateWindow();
    m_trayIcon->hide();
  }
}
#endif

void MainWindow::scrollToTransaction(const QModelIndex& _index) {
  m_ui->m_transactionsAction->setChecked(true);
  m_ui->m_transactionsFrame->scrollToTransaction(_index);
}

void MainWindow::switchMenu(QAction* act) {

  m_ui->m_overviewButton->setChecked(false);
  m_ui->m_sendButton->setChecked(false);
  m_ui->m_receiveButton->setChecked(false);
  m_ui->m_transactionsButton->setChecked(false);
  m_ui->m_addressBookButton->setChecked(false);
  m_ui->m_messagesButton->setChecked(false);
  m_ui->m_sendMessageButton->setChecked(false);
  m_ui->m_miningButton->setChecked(false);

  QString name = act->objectName();
  if (name == "m_overviewAction") {
    m_ui->m_overviewButton->setChecked(true);
  }else if (name == "m_sendAction") {
    m_ui->m_sendButton->setChecked(true);
  }else if (name == "m_receiveAction") {
    m_ui->m_receiveButton->setChecked(true);
  }else if (name == "m_transactionsAction") {
    m_ui->m_transactionsButton->setChecked(true);
  }else if (name == "m_addressBookAction") {
    m_ui->m_addressBookButton->setChecked(true);
  }else if (name == "m_messagesAction") {
    MESSAGES_READ_COUNT = MessagesModel::instance().rowCount();
    Settings::instance().setReadMessages(MESSAGES_READ_COUNT);
    showUnread(0);
    m_ui->m_messagesButton->setChecked(true);
  }else if (name == "m_sendMessageAction") {
    m_ui->m_sendMessageButton->setChecked(true);
  }else if (name == "m_miningAction") {
    m_ui->m_miningButton->setChecked(true);
  }
}

void MainWindow::quit() {
  if (!m_isAboutToQuit) {
    ExitWidget* exitWidget = new ExitWidget(nullptr);
    exitWidget->show();
  
#ifdef Q_OS_WIN
    m_trayIcon->hide();
#endif    
  
    m_isAboutToQuit = true;
    close();
  }
}

#ifdef Q_OS_MAC
void MainWindow::restoreFromDock() {
  if (m_isAboutToQuit) {
    return;
  }

  showNormal();
}
#endif

void MainWindow::closeEvent(QCloseEvent* _event) {
#ifdef Q_OS_WIN
  if (m_isAboutToQuit) {
    QMainWindow::closeEvent(_event);
    return;
  } else if (Settings::instance().isCloseToTrayEnabled()) {
    minimizeToTray(true);
    _event->ignore();
  } else {
    QApplication::quit();
    return;
  }
#elif defined(Q_OS_LINUX)
  if (!m_isAboutToQuit) {
    QApplication::quit();
    return;
  }
#endif
  QMainWindow::closeEvent(_event);

}

#ifdef Q_OS_WIN
void MainWindow::changeEvent(QEvent* _event) {
  QMainWindow::changeEvent(_event);
  if (!QSystemTrayIcon::isSystemTrayAvailable()) {
    QMainWindow::changeEvent(_event);
    return;
  }

  switch (_event->type()) {
  case QEvent::WindowStateChange:
    if(Settings::instance().isMinimizeToTrayEnabled()) {
      minimizeToTray(isMinimized());
    }
    break;
  default:
    break;
  }

  QMainWindow::changeEvent(_event);
}
#endif

bool MainWindow::event(QEvent* _event) {
  switch (static_cast<WalletEventType>(_event->type())) {
    case WalletEventType::ShowMessage:
    showMessage(static_cast<ShowMessageEvent*>(_event)->messageText(), static_cast<ShowMessageEvent*>(_event)->messageType());
    return true;
  }

  return QMainWindow::event(_event);
}

void MainWindow::createWallet() {
  QString filePath = QFileDialog::getSaveFileName(this, tr("New wallet file"),
  #ifdef Q_OS_WIN
      QApplication::applicationDirPath(),
  #else
      QDir::homePath(),
  #endif
      tr("Wallets (*.wallet)")
      );

    if (!filePath.isEmpty() && !filePath.endsWith(".wallet")) {
      filePath.append(".wallet");
    }

    if (!filePath.isEmpty() && !QFile::exists(filePath)) {
      if (WalletAdapter::instance().isOpen()) {
        WalletAdapter::instance().close();
      }

      WalletAdapter::instance().setWalletFile(filePath);
      WalletAdapter::instance().open("");
    }
}

void MainWindow::openWallet() {
  QString filePath = QFileDialog::getOpenFileName(this, tr("Open .wallet/.keys file"),
#ifdef Q_OS_WIN
    QApplication::applicationDirPath(),
#else
    QDir::homePath(),
#endif
    tr("Wallet (*.wallet *.keys)"));

  if (!filePath.isEmpty()) {
    if (WalletAdapter::instance().isOpen()) {
      WalletAdapter::instance().close();
    }

    WalletAdapter::instance().setWalletFile(filePath);
    WalletAdapter::instance().open("");
  }
}

void MainWindow::importKey() {
  ImportKeyDialog dlg(this);
  if (dlg.exec() == QDialog::Accepted) {
    QString keyString = dlg.getKeyString().trimmed();
    QString filePath = dlg.getFilePath();
    if (keyString.isEmpty() || filePath.isEmpty()) {
      return;
    }

    if (!filePath.endsWith(".wallet")) {
      filePath.append(".wallet");
    }

    uint64_t addressPrefix;
    std::string data;
    CryptoNote::AccountKeys keys;
    if (Tools::Base58::decode_addr(keyString.toStdString(), addressPrefix, data) && addressPrefix == CurrencyAdapter::instance().getAddressPrefix() &&
      data.size() == sizeof(keys)) {
      std::memcpy(&keys, data.data(), sizeof(keys));
      if (WalletAdapter::instance().isOpen()) {
        WalletAdapter::instance().close();
      }

      WalletAdapter::instance().setWalletFile(filePath);
      WalletAdapter::instance().createWithKeys(keys);
    }
  }
}

void MainWindow::backupWallet() {
  QString filePath = QFileDialog::getSaveFileName(this, tr("Backup wallet to..."),
  #ifdef Q_OS_WIN
      QApplication::applicationDirPath(),
  #else
      QDir::homePath(),
  #endif
      tr("Wallets (*.wallet)")
      );
    if (!filePath.isEmpty() && !filePath.endsWith(".wallet")) {
      filePath.append(".wallet");
    }

    if (!filePath.isEmpty() && !QFile::exists(filePath)) {
      WalletAdapter::instance().backup(filePath);
    }
}

void MainWindow::resetWallet() {
  Q_ASSERT(WalletAdapter::instance().isOpen());
  if (QMessageBox::warning(this, tr("Warning"), tr("Your wallet will be reset and restored from blockchain but YOU WILL LOSE ALL YOUR SENT MESSAGES!\n\n" "Are you sure?\n\nThere is no way to undo."), QMessageBox::Ok, QMessageBox::Cancel) == QMessageBox::Ok) {
    WalletAdapter::instance().reset();
    WalletAdapter::instance().open("");
  }
}

void MainWindow::encryptWallet() {
  if (Settings::instance().isEncrypted()) {
    bool error = false;
    do {
      ChangePasswordDialog dlg(this);
      if (dlg.exec() == QDialog::Rejected) {
        return;
      }

      QString oldPassword = dlg.getOldPassword();
      QString newPassword = dlg.getNewPassword();
      error = !WalletAdapter::instance().changePassword(oldPassword, newPassword);
    } while (error);
  } else {
    NewPasswordDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted) {
      QString password = dlg.getPassword();
      if (password.isEmpty()) {
        return;
      }

      encryptedFlagChanged(WalletAdapter::instance().changePassword("", password));
    }
  }
}

void MainWindow::aboutQt() {
  QMessageBox::aboutQt(this);
}

void MainWindow::setStartOnLogin(bool _on) {
  Settings::instance().setStartOnLoginEnabled(_on);
  m_ui->m_startOnLoginAction->setChecked(Settings::instance().isStartOnLoginEnabled());
}

void MainWindow::setMinimizeToTray(bool _on) {
#ifdef Q_OS_WIN
  Settings::instance().setMinimizeToTrayEnabled(_on);
#endif
}

void MainWindow::setCloseToTray(bool _on) {
#ifdef Q_OS_WIN
  Settings::instance().setCloseToTrayEnabled(_on);
#endif
}

void MainWindow::setMessageNotifications(bool _on) {
#ifdef Q_OS_WIN
  Settings::instance().setMessageNotificationsEnabled(_on);
#endif
}

void MainWindow::setPaymentNotifications(bool _on) {
#ifdef Q_OS_WIN
  Settings::instance().setPaymentNotificationsEnabled(_on);
#endif
}

void MainWindow::about() {
  AboutDialog dlg(this);
  dlg.exec();
}

void MainWindow::setStatusBarText(const QString& _text) {
  statusBar()->showMessage(_text);
}

void MainWindow::showMessage(const QString& _text, QtMsgType _type) {
  switch (_type) {
  case QtInfoMsg:
    QMessageBox::information(this, tr("Info"), _text);
    break;
  case QtCriticalMsg:
    QMessageBox::critical(this, tr("Error"), _text);
    break;
  case QtDebugMsg:
    QMessageBox::information(this, tr("Wallet"), _text);
    break;
  default:
    break;
  }
}

void MainWindow::askForWalletPassword(bool _error) {
  PasswordDialog dlg(_error, this);
  if (dlg.exec() == QDialog::Accepted) {
    QString password = dlg.getPassword();
    WalletAdapter::instance().open(password);
  }
}

void MainWindow::encryptedFlagChanged(bool _encrypted) {
  m_ui->m_encryptWalletAction->setEnabled(!_encrypted);
  m_ui->m_changePasswordAction->setEnabled(_encrypted);
  QString encryptionIconPath = _encrypted ? ":icons/encrypted" : ":icons/decrypted";
  QPixmap encryptionIcon = QPixmap(encryptionIconPath).scaled(16, 16, Qt::KeepAspectRatio, Qt::SmoothTransformation);
  m_encryptionStateIconLabel->setPixmap(encryptionIcon);
  QString encryptionLabelTooltip = _encrypted ? tr("Encrypted") : tr("Not encrypted");
  m_encryptionStateIconLabel->setToolTip(encryptionLabelTooltip);
  m_ui->m_overviewFrame->encryptWallet();
}

void MainWindow::peerCountUpdated(quint64 _peerCount) {
  QString connectionIconPath = _peerCount > 0 ? ":icons/connected" : ":icons/disconnected";
  QPixmap connectionIcon = QPixmap(connectionIconPath).scaled(16, 16, Qt::KeepAspectRatio, Qt::SmoothTransformation);
  m_connectionStateIconLabel->setPixmap(connectionIcon);
  m_connectionStateIconLabel->setToolTip(QString(tr("%1 peers").arg(_peerCount)));
}

void MainWindow::messagesCountUpdated(quint64 _messagesCount) {

  if (m_ui->m_messagesFrame->isVisible()){
    MESSAGES_READ_COUNT = MessagesModel::instance().rowCount();
    showUnread(0);
  }else{
  
    QString newCount = QString("%1").arg(_messagesCount);
    QString oldCount = QString("%1").arg(MESSAGES_READ_COUNT);
    if (newCount.toInt() > oldCount.toInt()){
      showUnread(newCount.toInt() - oldCount.toInt());
    }else{
      showUnread(0);
    }
  }
}

void MainWindow::showUnread(quint64 _number){
  if (_number > 99) _number = 99;
  m_ui->m_messagesButton->unreadCount = _number;
  m_ui->m_messagesButton->update();
  if (_number){
    if (_number > 9) {
      QPixmap pixmap(":images/logo-ico");
      QPainter paint(&pixmap);
      paint.setRenderHint(QPainter::Antialiasing);
      paint.setPen(Qt::NoPen);
      paint.setBrush(QColor("#ef5858"));
      paint.drawEllipse(QPointF(46,46), 17,17);
      QFont font;
      paint.setPen(QColor("#fbfbfc"));
      font.setPixelSize(22);
      font.setFamily("Lato Bold");
      paint.setFont(font);
      paint.drawText(27,32, 38, 30, Qt::AlignHCenter | Qt::AlignTop, QString("%1").arg(_number));
      paint.save();
      paint.end();
      QIcon windowIcon(pixmap);
      this->window()->setWindowIcon(windowIcon);
    }else{
      QPixmap pixmap(":images/logo-ico");
      QPainter paint(&pixmap);
      paint.setRenderHint(QPainter::Antialiasing);
      paint.setPen(Qt::NoPen);
      paint.setBrush(QColor("#ef5858"));
      paint.drawEllipse(QPointF(46,46), 17,17);
      QFont font;
      paint.setPen(QColor("#fbfbfc"));
      font.setPixelSize(28);
      font.setFamily("Lato Black");
      paint.setFont(font);
      paint.drawText(26, 29, 38, 30, Qt::AlignHCenter | Qt::AlignTop, QString("%1").arg(_number));
      paint.save();
      paint.end();
      QIcon windowIcon(pixmap);
      this->window()->setWindowIcon(windowIcon);
    }
  }else{
    QPixmap pixmap(":images/logo-ico");
    QPainter paint(&pixmap);
    QIcon windowIcon(pixmap);
    MainWindow::setWindowIcon(windowIcon);
  }
}
void MainWindow::walletSynchronizationInProgress(uint32_t _current, uint32_t _total) {
  QPixmap syncIcon = QPixmap(":icons/syncing").scaled(16, 16, Qt::KeepAspectRatio, Qt::SmoothTransformation);
  m_synchronizationStateIconLabel->setPixmap(syncIcon);

  m_synchronizationStateIconLabel->setToolTip(tr("Synchronization in progress"));
}

void MainWindow::walletSynchronized(int _error, const QString& _error_text) {
  QPixmap syncIcon = QPixmap(":icons/transaction").scaled(16, 16, Qt::KeepAspectRatio, Qt::SmoothTransformation);
  m_synchronizationStateIconLabel->setPixmap(syncIcon);
  QString syncLabelTooltip = _error > 0 ? tr("Not synchronized") : tr("Synchronized");
  m_synchronizationStateIconLabel->setToolTip(syncLabelTooltip);
}

void MainWindow::walletOpened(bool _error, const QString& _error_text) {
  if (!_error) {
    m_encryptionStateIconLabel->show();
    m_synchronizationStateIconLabel->show();
    m_ui->m_backupWalletAction->setEnabled(true);
    m_ui->m_resetAction->setEnabled(true);
    encryptedFlagChanged(Settings::instance().isEncrypted());
    m_ui->m_overviewButton->setEnabled(true);
    m_ui->m_sendButton->setEnabled(true);
    m_ui->m_receiveButton->setEnabled(true);
    m_ui->m_transactionsButton->setEnabled(true);
    m_ui->m_addressBookButton->setEnabled(true);
    m_ui->m_messagesButton->setEnabled(true);
    m_ui->m_sendMessageButton->setEnabled(true);
    m_ui->m_miningButton->setEnabled(true);
    QList<QAction*> tabActions = m_tabActionGroup->actions();
    Q_FOREACH(auto action, tabActions) {
      action->setEnabled(true);
    }
    m_ui->m_overviewAction->trigger();
    m_ui->m_overviewFrame->show();
  } else {
    walletClosed();
  }
}

void MainWindow::walletClosed() {
  m_ui->m_overviewButton->setEnabled(false);
  m_ui->m_sendButton->setEnabled(false);
  m_ui->m_receiveButton->setEnabled(false);
  m_ui->m_transactionsButton->setEnabled(false);
  m_ui->m_addressBookButton->setEnabled(false);
  m_ui->m_messagesButton->setEnabled(false);
  m_ui->m_sendMessageButton->setEnabled(false);
  m_ui->m_miningButton->setEnabled(false);  
  m_ui->m_backupWalletAction->setEnabled(false);
  m_ui->m_encryptWalletAction->setEnabled(false);
  m_ui->m_changePasswordAction->setEnabled(false);
  m_ui->m_resetAction->setEnabled(false);
  m_ui->m_overviewFrame->hide();
  m_ui->m_sendFrame->hide();
  m_ui->m_transactionsFrame->hide();
  m_ui->m_addressBookFrame->hide();
  m_ui->m_messagesFrame->hide();
  m_ui->m_sendMessageFrame->hide();
  m_ui->m_miningFrame->hide();
  m_ui->m_depositsFrame->hide();
  m_encryptionStateIconLabel->hide();
  m_synchronizationStateIconLabel->hide();
  QList<QAction*> tabActions = m_tabActionGroup->actions();
  Q_FOREACH(auto action, tabActions) {
    action->setEnabled(false);
  }
#ifdef Q_OS_WIN
  m_trayIcon->hide();
#endif
}

QString MainWindow::extractContact(const QModelIndex& _index) {
  if (
    _index.data(MessagesModel::ROLE_FROM_ADDRESS).toString().size() //Sender address is known
    &&
    !AddressBookModel::instance().getName(_index.data(MessagesModel::ROLE_FROM_ADDRESS).toString()).toString().size() //Sender address not in addressbook
    &&
    WalletAdapter::instance().getAddress() != _index.data(MessagesModel::ROLE_FROM_ADDRESS).toString() //Sender is not us
    ){
    return _index.data(MessagesModel::ROLE_FROM_ADDRESS).toString();
  }
  if (
    _index.data(MessagesModel::ROLE_TO_ADDRESS).toString().size() //Receiver address is known
    &&
    !AddressBookModel::instance().getName(_index.data(MessagesModel::ROLE_TO_ADDRESS).toString()).toString().size() //Receiver address not in addressbook
    &&
    WalletAdapter::instance().getAddress() != _index.data(MessagesModel::ROLE_TO_ADDRESS).toString() //Receiver is not us
    ){
    return _index.data(MessagesModel::ROLE_TO_ADDRESS).toString();
  }
  return QString();
}

void MainWindow::saveContact(const QModelIndex& _index) {
  QString _address = extractContact(_index);
  m_ui->m_addressBookFrame->addClicked(_address);
}

void MainWindow::replyTo(const QModelIndex& _index) {
  QString _address;
  if (
    _index.data(MessagesModel::ROLE_FROM_ADDRESS).toString().size() //Sender address is known
    &&
    WalletAdapter::instance().getAddress() != _index.data(MessagesModel::ROLE_FROM_ADDRESS).toString() //Sender is not us
    ){
    _address = _index.data(MessagesModel::ROLE_FROM_ADDRESS).toString();
  }
  if (
    _index.data(MessagesModel::ROLE_TO_ADDRESS).toString().size() //Receiver address is known
    &&
    WalletAdapter::instance().getAddress() != _index.data(MessagesModel::ROLE_TO_ADDRESS).toString() //Receiver is not us
    ){
    _address = _index.data(MessagesModel::ROLE_TO_ADDRESS).toString();
  }
  m_ui->m_sendMessageFrame->setAddress(_address);
  m_ui->m_sendMessageAction->trigger();
}

void MainWindow::sendTo(const QModelIndex& _index) {
  if (_index.data(AddressBookModel::ROLE_ADDRESS).isValid()){
    m_ui->m_sendMessageFrame->setAddress(_index.data(AddressBookModel::ROLE_ADDRESS).toString());
  }
  m_ui->m_sendMessageAction->trigger();
}

void MainWindow::payTo(const QModelIndex& _index) {
  if (_index.data(AddressBookModel::ROLE_ADDRESS).isValid()){
    QString addressbookName = _index.data(AddressBookModel::ROLE_LABEL).toString();
    QString addressbookAddress = _index.data(AddressBookModel::ROLE_ADDRESS).toString();
    m_ui->m_sendFrame->setAddress(QString("%1 <%2>").arg(addressbookName).arg(addressbookAddress));
  }
  m_ui->m_sendAction->trigger();
}

#ifdef Q_OS_WIN
void MainWindow::trayActivated(QSystemTrayIcon::ActivationReason _reason) {
  showNormal();
  activateWindow();
  m_trayIcon->hide();
}
void MainWindow::messageClicked() {
  showNormal();
  activateWindow();
  m_trayIcon->hide();
}
#endif

}
