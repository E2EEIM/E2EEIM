#include <QCoreApplication>
#include "myserver.h"
#include <QByteArray>
#include <QQueue>
int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QQueue<QByteArray> msg;

    MyServer Server(msg);
    Server.startServer();

    return a.exec();
}
