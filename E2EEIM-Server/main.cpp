#include <QCoreApplication>
#include "server.h"

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
    QList<QString> addFriendRequestList;

    Server server(msg, usernameList, userKeyList, loginUser, loginRanNum, waitingTaskUser, waitingTaskWork, addFriendRequestList);
    server.startServer();

    // https://forum.qt.io/topic/86025/qt-get-external-ip-address-using-qnetworkreply/2

    return a.exec();
}
