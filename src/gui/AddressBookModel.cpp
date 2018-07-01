// Copyright (c) 2011-2016 The Cryptonote developers
// Copyright (c) 2015-2016 XDN developers
// Copyright (c) 2018-2019 xDrop developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <QtWidgets>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>

#include "WalletAdapter.h"
#include "AddressBookModel.h"
#include "Settings.h"


namespace WalletGui {

AddressBookModel& AddressBookModel::instance() {
  static AddressBookModel inst;
  return inst;
}

AddressBookModel::AddressBookModel(QObject *parent)
    : QAbstractTableModel(parent)
{
    connect(&WalletAdapter::instance(), &WalletAdapter::walletInitCompletedSignal, this, &AddressBookModel::loadAddressBook, Qt::QueuedConnection);
}

AddressBookModel::AddressBookModel(QList<Contact> contacts, QObject *parent)
    : QAbstractTableModel(parent)
    , contacts(contacts)
{
    connect(&WalletAdapter::instance(), &WalletAdapter::walletInitCompletedSignal, this, &AddressBookModel::loadAddressBook, Qt::QueuedConnection);
}

int AddressBookModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return contacts.size();
}

int AddressBookModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 2;
}

QVariant AddressBookModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.row() >= contacts.size() || index.row() < 0)
        return QVariant();

    const auto &contact = contacts.at(index.row());

    switch (role) {
    case Qt::DisplayRole:{
      switch (index.column()) {
      case COLUMN_LABEL:
        return index.data(ROLE_LABEL);
      case COLUMN_ADDRESS:
        return index.data(ROLE_ADDRESS);
      default:
        return QVariant();
      }
    }
    case ROLE_LABEL:
      return contact.name;
    case ROLE_ADDRESS:
      return contact.address;
    default:
      return QVariant();
    }

    return QVariant();
}

QVariant AddressBookModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal) {
        switch (section) {
            case 0:
                return tr("Name");

            case 1:
                return tr("Address");

            default:
                return QVariant();
        }
    }
    return QVariant();
}

bool AddressBookModel::insertRows(int position, int rows, const QModelIndex &index)
{
    Q_UNUSED(index);
    beginInsertRows(QModelIndex(), position, position + rows - 1);

    for (int row = 0; row < rows; ++row)
        contacts.insert(position, { QString(), QString() });

    endInsertRows();
    return true;
}

QVariant AddressBookModel::getName(QString address)
{
  QList<Contact> contacts = getContacts();
  for (const auto &contact: qAsConst(contacts)){
    if (contact.address == address)
      return contact.name;
  }
  return QVariant();  
}


void AddressBookModel::addEntry(QString name, QString address)
{
    if (!contacts.contains({ name, address })) {
        insertRows(0, 1, QModelIndex());

        QModelIndex index = this->index(0, 0, QModelIndex());
        this->setData(index, name);
        index = this->index(0, 1, QModelIndex());
        this->setData(index, address);
    }
    saveAddressBook();
}

bool AddressBookModel::removeRows(int position, int rows, const QModelIndex &index)
{
    Q_UNUSED(index);
    beginRemoveRows(QModelIndex(), position, position + rows - 1);

    for (int row = 0; row < rows; ++row)
        contacts.removeAt(position);

    endRemoveRows();
    saveAddressBook();

    return true;
}

void AddressBookModel::removeEntry()
{
    QTableView *temp = new QTableView;
    QSortFilterProxyModel *proxy = static_cast<QSortFilterProxyModel*>(temp->model());
    QItemSelectionModel *selectionModel = temp->selectionModel();

    QModelIndexList indexes = selectionModel->selectedRows();

    foreach (QModelIndex index, indexes) {
        int row = proxy->mapToSource(index).row();
        this->removeRows(row, 1, QModelIndex());
    }
}

bool AddressBookModel::setData(const QModelIndex &index, const QVariant &value)
{
    if (index.isValid()) {
        int row = index.row();

        auto contact = contacts.value(row);

        if (index.column() == 0)
            contact.name = value.toString();
        else if (index.column() == 1)
            contact.address = value.toString();
        else
            return false;

        contacts.replace(row, contact);
        emit(dataChanged(index, index));

        return true;
    }

    return false;
}

Qt::ItemFlags AddressBookModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::ItemIsEnabled;

    return QAbstractTableModel::flags(index);
}

QList<Contact> AddressBookModel::getContacts() const
{
    return contacts;
}


void AddressBookModel::saveAddressBook() {
  QFile addressBookFile(Settings::instance().getAddressBookFile());
  if (addressBookFile.open(QIODevice::ReadWrite | QIODevice::Truncate)) {
    QDataStream out(&addressBookFile);
    out << contacts;
  }
}

void AddressBookModel::loadAddressBook() {
  QFile addressBookFile(Settings::instance().getAddressBookFile());
  if (addressBookFile.open(QIODevice::ReadWrite)) {
    QList<Contact> contacts;
    QDataStream in(&addressBookFile);
    in >> contacts;

    if (!contacts.isEmpty()) {
      for (const auto &contact: qAsConst(contacts)){
        addEntry(contact.name, contact.address);
      }
    }else{
      addEntry("Me",WalletAdapter::instance().getAddress());
      addEntry("xDrop Dev","DFk1UuPZFjZEXBCkbGJz6CUh2f6Zx671RAz1JJG24pekFdUdPq4TxojVmW89GXqrNMMaC4xuf6GmQDQeCEcJ344k3LVrXn4");
    }
  }
}

}