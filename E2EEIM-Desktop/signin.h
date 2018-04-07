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

#ifndef SIGNIN_H
#define SIGNIN_H

#include <QDialog>
#include "connection.h"
#include "encryption.h"

namespace Ui {
class SignIn;
}

class SignIn : public QDialog
{
    Q_OBJECT

public:
    explicit SignIn(Connection &conn, Encryption &encryption, QWidget *parent = 0);
    QString getActiveUser();
    ~SignIn();

private slots:
    void on_pushButton_clicked();

    void on_comboBox_selectServer_currentIndexChanged(const QString &arg1);

    void on_lineEdit_SignUpServerIP_textChanged(const QString &arg1);

    void on_lineEdit_SignUpServerPort_textChanged(const QString &arg1);

    void on_pushButton_SignUpServerConnect_clicked();

    void on_tabWidget_signUp_currentChanged(int index);

    void on_pushButton_signUpAccountSignUp_clicked();

private:
    Ui::SignIn *ui;
    Connection *conn;
    Encryption *encryption;
    gpgme_key_t newUsersPrivateKey;
    gpgme_key_t newUsersPublicKey;

};

#endif // SIGNIN_H
