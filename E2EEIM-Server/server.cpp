#include "server.h"
#include "clienttask.h"
#include "clientthread.h"

Server::Server(QQueue<QByteArray> &msg, QList<QString> &usernameList,
               QList<QString> &userKeyList, QList<QString> &loginUser,
               QList<QString> &loginRanNum, QList<QString> &waitingTaskUser,
               QList<QString> &waitingTaskWork, QList<QString> &addFriendRequestList)
{
    queue=&msg;
    this->usernameList=&usernameList;
    this->userKeyList=&userKeyList;
    this->loginUser=&loginUser;
    this->loginRanNum=&loginRanNum;
    this->waitingTaskUser=&waitingTaskUser;
    this->waitingTaskWork=&waitingTaskWork;
    this->addFriendRequestList=&addFriendRequestList;

}

void Server::startServer(){

    if(!this->listen(QHostAddress::Any, 2222)){
        qDebug() << "Could not start server";
    }
    else{
        qDebug() << "Listening... on port 2222";
    }
}

void Server::incomingConnection(qintptr socketDescriptor){

    qDebug() << "sockfd No."<< socketDescriptor << "Connecting...";
    qDebug() << "Create thread for sockfd No." << socketDescriptor;

    ClientThread *clientThread=new ClientThread();
    ClientTask *clientTask=new ClientTask(socketDescriptor, queue, usernameList, userKeyList,
                                          loginUser, loginRanNum, waitingTaskUser, waitingTaskWork, addFriendRequestList);

    connect(clientThread, SIGNAL(finished()), clientThread, SLOT(deleteLater()));
    connect(clientThread, SIGNAL(started()), clientTask, SLOT(initClient()));
    connect(clientThread, SIGNAL(newLoop()), clientTask, SLOT(timerStart()));
    connect(clientTask, SIGNAL(slowDown()), clientThread, SLOT(slowThread()));
    connect(clientTask, SIGNAL(clientDisconnect()), clientThread, SLOT(endThread()));

    clientTask->moveToThread(clientThread);


    clientThread->start();
}
