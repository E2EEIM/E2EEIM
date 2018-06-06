/****************************************************************************
** Copyright (C) 2018  Krittitam Juksukit
** This file is part of E2EEIM.
**
** E2EEIM is free software: you can redistribute it and/or modify
** it under the terms of the GNU Lesser General Public License
** version 3 as published by the Free Software Foundation.
**
** E2EEIM is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU Lesser General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with E2EEIM.  If not, see <http://www.gnu.org/licenses/>.
**
*******************************************************************************/

#ifndef ADDCONTACT_H
#define ADDCONTACT_H

#include <QDialog>
#include "connection.h"
#include "encryption.h"

namespace Ui {
class AddContact;
}

class AddContact : public QDialog
{
    Q_OBJECT

public:
    explicit AddContact(Connection &conn, Encryption &encryption, QStringList &addFriendRequestList, QWidget *parent = 0, QString activeUser="");
    ~AddContact();

private slots:

    void on_lineEdit_search_textChanged(const QString &arg1);

    void on_pushButton_search_clicked();

    void on_pushButton_sendAddFriendRequest_clicked();

private:
    Ui::AddContact *ui;

    Connection *conn;
    Encryption *encryption;
    gpgme_key_t servKey;
    gpgme_key_t userPriKey;
    gpgme_key_t userPubKey;

    QString ACTIVE_USER;
    QString foundUser;
    QStringList *addFriendRequestList;

};

#endif // ADDCONTACT_H
