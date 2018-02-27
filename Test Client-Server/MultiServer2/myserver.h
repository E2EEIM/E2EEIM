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
    explicit MyServer(QQueue<QByteArray> &msg, QObject *parent = 0);
    void startServer();
signals:

public slots:
    void receiveMessage(QByteArray pkg);

private slots:
    //void receiveMessage(QByteArray package);

protected:
    void incomingConnection(qintptr socketDescriptor);
    QQueue<QByteArray> *queue;
};

#endif // MYSERVER_H
