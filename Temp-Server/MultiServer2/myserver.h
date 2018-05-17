#ifndef MYSERVER_H
#define MYSERVER_H

#include <QObject>
#include <QTcpServer>
#include <QDebug>
#include "mythread.h"
#include <QList>

class MyServer : public QTcpServer
{
    Q_OBJECT
public:
    explicit MyServer(QQueue<QByteArray> &msg, QList<QString> &usernameList, QList<QString> &userKeyList, QList<QString> &loginUser, QList<QString> &loginRanNum, QObject *parent = nullptr);
    void startServer();
signals:

public slots:

private slots:

protected:
    void incomingConnection(qintptr socketDescriptor);
    QQueue<QByteArray> *queue;
    QList<QString> *usernameList;
    QList<QString> *userKeyList;
    QList<QString> *loginUser;
    QList<QString> *loginRanNum;
};

#endif // MYSERVER_H
