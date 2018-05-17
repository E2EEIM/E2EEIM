#include "myserver.h"

MyServer::MyServer(QQueue<QByteArray> &msg,QList<QString> &usernameList,
                   QList<QString> &userKeyList,QList<QString> &loginUser,
                   QList<QString> &loginRanNum, QObject *parent) :
    QTcpServer(parent)
{
    queue=&msg;
    this->usernameList=&usernameList;
    this->userKeyList=&userKeyList;
    this->loginUser=&loginUser;
    this->loginRanNum=&loginRanNum;
}

void MyServer::startServer(){

    if(!this->listen(QHostAddress::Any, 2222)){
        qDebug() << "Could not start server";
    }
    else{
        qDebug() << "Listening... on port 2222";
    }

}

void MyServer::incomingConnection(qintptr socketDescriptor){

    //qDebug() << "server: " << queue;


    qDebug() << "sockfd No."<< socketDescriptor << "Connecting...";
    qDebug() << "Create thread for sockfd No." << socketDescriptor;
    MyThread *thread = new MyThread(queue, usernameList, userKeyList,
                                    loginUser, loginRanNum, socketDescriptor, this);
    //MyThread *thread = new MyThread(queue, usernameList, userKeyList, socketDescriptor, this);

    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));

    thread->start();
}


