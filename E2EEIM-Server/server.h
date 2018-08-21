#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QTcpServer>

class Server : public QTcpServer
{
    Q_OBJECT
public:
    Server(QQueue<QByteArray> &msg, QList<QString> &usernameList,
           QList<QString> &userKeyList, QList<QString> &loginUser,
           QList<QString> &loginRanNum, QList<QString> &waitingTaskUser,
           QList<QString> &waitingTaskWork, QList<QString> &addFriendRequestList,
           QString keyFpr, int port);
    void startServer();

protected:
    QQueue<QByteArray> *queue;
    QList<QString> *usernameList;
    QList<QString> *userKeyList;
    QList<QString> *loginUser;
    QList<QString> *loginRanNum;
    QList<QString> *waitingTaskUser;
    QList<QString> *waitingTaskWork;
    QList<QString> *addFriendRequestList;

    int port;
    QString keyFpr;

    void incomingConnection(qintptr socketDescriptor);
};

#endif // SERVER_H
