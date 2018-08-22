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

signals:
    void receiveAddFriendrequest(QByteArray data);
    void generateKeyPairSig(QStringList account);


public slots:
    void disconnectFromServer();
    void receiveSignInVerify(QByteArray data);
    void receiveSignInResult(QByteArray data);
    void receiveSignUpResult(QByteArray data);
    void generateKeyPair(QStringList account);


private slots:

    void on_lineEdit_SignUpServerIP_textChanged(const QString &arg1);

    void on_lineEdit_SignUpServerPort_textChanged(const QString &arg1);

    void on_pushButton_SignUpServerConnect_clicked();

    void on_tabWidget_signUp_currentChanged(int index);

    void on_pushButton_signUpAccountSignUp_clicked();

    void on_tabWidget_mainTab_currentChanged(int index);

    void on_comboBox_signIn_SelectAccount_currentIndexChanged(const QString &arg1);

    void on_comboBox_signIn_selectServer_currentIndexChanged(const QString &arg1);

    void on_lineEdit_signIn_serverAddress_textChanged(const QString &arg1);

    void on_lineEdit_signIn_serverPort_textChanged(const QString &arg1);

    void on_pushButton_signIn_serverConnect_clicked();

    void on_pushButton_signIn_AccountSignIn_clicked();

    void on_comboBox_signUp_selectServer_currentTextChanged(const QString &arg1);

    void on_tabWidget_signIn_currentChanged(int index);

    void selectKeyToSignUp(int index);
    void signUpFormValidation();
    void signUpByKey();

private:
    Ui::SignIn *ui;
    Connection *conn;
    Encryption *encryption;
    gpgme_key_t newUsersPrivateKey;
    gpgme_key_t newUsersPublicKey;
    QString selectedAccount;
    QStringList accountNameList;
    QStringList accountKeyList;

};

#endif // SIGNIN_H
