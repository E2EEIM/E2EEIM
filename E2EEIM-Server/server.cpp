#include "server.h"
#include "clienttask.h"
#include "clientthread.h"

Server::Server(QQueue<QByteArray> &msg, QList<QString> &usernameList,
               QList<QString> &userKeyList, QList<QString> &loginUser,
               QList<QString> &loginRanNum, QList<QString> &waitingTaskUser,
               QList<QString> &waitingTaskWork, QList<QString> &addFriendRequestList,
               QString keyFpr, int port)
{
    queue=&msg;
    this->usernameList=&usernameList;
    this->userKeyList=&userKeyList;
    this->loginUser=&loginUser;
    this->loginRanNum=&loginRanNum;
    this->waitingTaskUser=&waitingTaskUser;
    this->waitingTaskWork=&waitingTaskWork;
    this->addFriendRequestList=&addFriendRequestList;
    this->port=port;
    this->keyFpr=keyFpr;

}

void Server::startServer(){

    //Listen on port 2222.
    if(!this->listen(QHostAddress::Any, this->port)){
        qDebug() << "Could not start server";
    }
    else{
        qDebug() << "Listening... on port " << port;
    }
}

//When client connecting to server.
void Server::incomingConnection(qintptr socketDescriptor){

    qDebug() << "sockfd No."<< socketDescriptor << "Connecting...";
    qDebug() << "Create thread for sockfd No." << socketDescriptor;

    //Create thread for the client.
    ClientThread *clientThread=new ClientThread();

    //Cleate client taskWork for the client.
    ClientTask *clientTask=new ClientTask(socketDescriptor, queue,
                                          usernameList, userKeyList,
                                          loginUser, loginRanNum,
                                          waitingTaskUser, waitingTaskWork,
                                          addFriendRequestList, keyFpr);

    //Stop thread when user disconnect.
    connect(clientThread, SIGNAL(finished()), clientThread, SLOT(deleteLater()));

    //Init user data when thread start.
    connect(clientThread, SIGNAL(started()), clientTask, SLOT(initClient()));

    //Start timer then thread return to event loop.
    connect(clientThread, SIGNAL(newLoop()), clientTask, SLOT(timerStart()));

    //Slow thread when client task emit slowDown().
    connect(clientTask, SIGNAL(slowDown()), clientThread, SLOT(slowThread()));

    //Stop thread when user disconnect.
    connect(clientTask, SIGNAL(clientDisconnect()), clientThread, SLOT(endThread()));

    //Move client work to the client thread.
    clientTask->moveToThread(clientThread);

    //Start thread.
    clientThread->start();
}
