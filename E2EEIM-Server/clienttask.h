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
#ifndef CLIENTTASK_H
#define CLIENTTASK_H

#include <QObject>
#include <QTcpSocket>
#include <QQueue>
#include <QTimer>
#include <gpgme.h>
#include <QDataStream>

class ClientTask : public QObject
{
    Q_OBJECT
public:
    explicit ClientTask(qintptr socketDescriptor, QQueue<QByteArray> *queue,
                        QList<QString> *usernameList, QList<QString> *userKeyList,
                        QList<QString> *loginUser, QList<QString> *loginRanNum,
                        QList<QString> *waitingTaskUser, QList<QString> *waitingTaskWork,
                        QList<QString> *addFriendRequestList, QString keyFpr,
                        QObject *parent = nullptr);

    QTimer *timer;

signals:
    void error(QTcpSocket::SocketError socketError);
    void newdata(QByteArray data);
    void slowDown();
    void clientDisconnect();

public slots:
    void initClient();
    void timerStart();
    void readyRead();
    void send(QByteArray data);
    void task();
    void disconnected();
    void dataFilter(QByteArray data);
    //void printDataDetail(QByteArray data);
    QByteArray decryptData(QByteArray data, const char* outPutFileName);
    void addNewUser(QByteArray payload);
    QByteArray encryptToClient(QByteArray data,QString recipient, const char *outFileName);
    gpgme_key_t getKey(QString pattern);


private:
    QTcpSocket *socket;
    qintptr socketDescriptor;

    QByteArray serverPubKey;
    QString activeUser;
    QString serverKeyFpr;

    bool splitPacket;
    QByteArray receiveBuffer;

    QQueue<QByteArray> *queuePtr;
    QList<QString> *usernameList;
    QList<QString> *userKeyList;
    QList<QString> *loginUser;
    QList<QString> *loginRanNum;
    QList<QString> *waitingTaskUser;
    QList<QString> *waitingTaskWork;
    QList<QString> *addFriendRequestList;

    gpgme_ctx_t ctx;  // the context
    gpgme_error_t err; // errors
    gpgme_key_t key= nullptr; // temp key
    gpgme_key_t senderKey;
    gpgme_key_t recipientKey;
    gpgme_key_t ServerKey = nullptr;
    gpgme_key_t activeUserKey;
};

#endif // CLIENTTASK_H
