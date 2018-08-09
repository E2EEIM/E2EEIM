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
#include <QDir>
#include <QQueue>
#include <QByteArray>
#include <QTimer>
#include "encryption.h"

class Connection : public QObject
{
    Q_OBJECT
public:
    explicit Connection(Encryption &encryption, QObject *parent = nullptr);
    void connected(QString host, QString port);
    void send(QByteArray data);
    int getConnectionStatus();
    QByteArray getRecentReceivedMsg();
    QString getServerAddr();
    QString getServerPort();
    bool signInFlag;

signals:
    void receiveAddFriendrequest(QByteArray data);
    void receiveNewPublicKey(QByteArray data);
    void receiveNewMessage(QByteArray data);
    void disconnectFromServer();
    void dataWaiting();


public slots:
    void readyRead();
    void letDisconnect();
    void processReceivedData();
    void noMoreData();

private:
    void writeToFile(QByteArray data, QString filename);
    QTcpSocket *socket;
    Encryption *encryption;
    int connectStatus;
    QByteArray recentReceivedMsg;
    QString serverAddr;
    QString serverPort;
    QQueue<QByteArray> receivedData;
    QTimer *waitForRecive;

    bool splitPacket;
    QByteArray receiveBuffer;

};

#endif // CONNECTION_H
