// Copyright (c) 2011-2016 The Cryptonote developers
// Copyright (c) 2015-2016 XDN developers
// Copyright (c) 2018-2019 xDrop developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef TABLEMODEL_H
#define TABLEMODEL_H

#include <QAbstractTableModel>
#include <QList>

namespace WalletGui {

struct Contact
{
    QString name;
    QString address;

    bool operator==(const Contact &other) const
    {
        return name == other.name && address == other.address;
    }
};

inline QDataStream &operator<<(QDataStream &stream, const Contact &contact)
{
    return stream << contact.name << contact.address;
}

inline QDataStream &operator>>(QDataStream &stream, Contact &contact)
{
    return stream >> contact.name >> contact.address;
}

class AddressBookModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    AddressBookModel(QObject *parent = 0);
    AddressBookModel(QList<Contact> contacts, QObject *parent = 0);

    enum Columns {COLUMN_LABEL, COLUMN_ADDRESS};
    enum Roles { ROLE_LABEL = Qt::UserRole, ROLE_ADDRESS };    

    static AddressBookModel& instance();

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    void addEntry(QString name, QString address);
    void removeEntry();
    bool setData(const QModelIndex &index, const QVariant &value);
    bool insertRows(int position, int rows, const QModelIndex &index = QModelIndex()) override;
    bool removeRows(int position, int rows, const QModelIndex &index = QModelIndex()) override;
    QList<Contact> getContacts() const;
    QVariant getName(QString address);

private:
    QList<Contact> contacts;
    void saveAddressBook();
    void loadAddressBook();

};

}

#endif // TABLEMODEL_H