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

#ifndef CONNECTION_H
#define CONNECTION_H

#include <QObject>
#include <QTcpSocket>
#include <QDataStream>
#include <QFile>
#include "encryption.h"

class Connection : public QObject
{
    Q_OBJECT
public:
    explicit Connection(Encryption &encryption, QObject *parent = nullptr);
    void connected(QString host, QString port);
    void send(QByteArray data);
    void letDisconnect();
    int getConnectionStatus();
    QByteArray getRecentReceivedMsg();
    QString getServerAddr();
    QString getServerPort();

signals:
    void receiveAddFriendrequest(QByteArray data);

public slots:
    void readyRead();

private:
    void writeToFile(QByteArray data, QString filename);
    QTcpSocket *socket;
    Encryption *encryption;
    int connectStatus;
    QByteArray recentReceivedMsg;
    QString serverAddr;
    QString serverPort;

};

#endif // CONNECTION_H
