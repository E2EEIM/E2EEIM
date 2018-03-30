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

    MyServer Server(msg, usernameList, userKeyList);
    Server.startServer();

    return a.exec();
}
