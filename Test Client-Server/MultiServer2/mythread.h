#ifndef MYTHREAD_H
#define MYTHREAD_H

#include <QObject>
#include <QThread>
#include <QTcpSocket>
#include <QDebug>
#include <QQueue>

class MyThread : public QThread
{
    Q_OBJECT
public:
    explicit MyThread(QQueue<QByteArray> *queue, int ID, QObject *parent = 0);
    void run();

    QTcpSocket *socket;

signals:
    void error(QTcpSocket::SocketError socketError);



public slots:
    void readyRead();
    void disconnected();
    void send();

private:
    int socketDescriptor;

    QQueue<QByteArray> *queuePtr;
};

#endif // MYTHREAD_H
