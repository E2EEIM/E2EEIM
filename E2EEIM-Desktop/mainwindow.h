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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidget>
#include "signin.h"
#include "connection.h"
#include "encryption.h"
#include "addcontact.h"
#include "creategroup.h"
#include "setting.h"
#include <qlayout.h>
#include <algorithm>
#include <string>
#include <QDesktopWidget>
#include <QMainWindow>
#include <QWidget>
#include <QFile>
#include <QString>
#include <QDebug>
#include <QTextStream>
#include <QStringList>
#include <QDir>
#include <QStringRef>
#include <QFileInfo>
#include <QPainter>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(Connection &conn, Encryption &encryption, QWidget *parent = 0);
    ~MainWindow();

signals:
    //void closeWindow();

public slots:
    void receiveAddFriendRequest(QByteArray data);
    void receiveNewPublicKey(QByteArray data);
    void disconnectFromServer();

private slots:
    QStringList ReadConversation(QString Filename);
    QStringList getGroupMember(QString GroupName);
    void removeFromListFile(QString filename, QString item);

    void initUserDataPath();

    void on_pushButton_Contact_clicked();

    void on_pushButton_Conversation_clicked();

    void on_pushButton_Group_clicked();

    void on_pushButton_AddList_clicked();

    void listWidget_Contact_ItemClicked(QListWidgetItem* item);

    void on_pushButton_SEND_clicked();

    void textMenuChange();

    void signOut();

    void cleanClose();

    bool eventFilter(QObject *watched, QEvent *event);

    QStringList readTextLine(QString Filename);


    void on_pushButton_addFriend_accept_clicked();

    void on_pushButton_addFriend_decline_clicked();

private:
    Ui::MainWindow *ui;
    Connection *conn;
    Encryption *encryption;

    QString currentMenu;
    QString ACTIVE_USR;
    QString conversationWith;
    QStringList addFriendRequestList;
    QStringList addFriendConfirmList;

    bool finishInitUserDataStatus;
    bool anyNewContact;

    gpgme_key_t servKey;
    gpgme_key_t userPriKey;
    gpgme_key_t userPubKey;
};

#endif // MAINWINDOW_H
