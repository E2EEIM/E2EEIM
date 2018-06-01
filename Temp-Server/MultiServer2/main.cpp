#include <QCoreApplication>
#include "myserver.h"
#include <QByteArray>
#include <QQueue>
int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QQueue<QByteArray> msg;
    QList<QString> usernameList;
    QList<QString> userKeyList;
    QList<QString> loginUser;
    QList<QString> loginRanNum;
    QList<QString> waitingTaskUser;
    QList<QString> waitingTaskWork;

    MyServer Server(msg, usernameList, userKeyList, loginUser, loginRanNum, waitingTaskUser, waitingTaskWork);
    Server.startServer();

    return a.exec();
}
