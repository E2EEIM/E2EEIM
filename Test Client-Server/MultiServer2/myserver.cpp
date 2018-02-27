#include "myserver.h"

MyServer::MyServer(QQueue<QByteArray> &msg, QObject *parent) :
    QTcpServer(parent)
{
    queue=&msg;
}

void MyServer::startServer(){

    if(!this->listen(QHostAddress::Any, 2222)){
        qDebug() << "Could not start server";
    }
    else{
        qDebug() << "Listening...";
    }

}

void MyServer::incomingConnection(qintptr socketDescriptor){

    qDebug() << "server: " << queue;


    qDebug() << socketDescriptor << "Connecting...";
    MyThread *thread = new MyThread(queue, socketDescriptor, this);
    qDebug() << "FINISH CREATE THREAD";


    connect(thread, SIGNAL(newMessage(QByteArray)), this, SLOT(receiveMessage(QByteArray)));
    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
    qDebug() << "FINISH First connect";

    thread->start();
    qDebug() << "Thread Started";
}

void MyServer::receiveMessage(QByteArray pkg){
    queue->enqueue(pkg);
    qDebug() << "NEW RECEVIE MESSAGE" << pkg;
    qDebug() << "First in line:" << queue->first();
    qDebug() << "Queue length:" << queue->length();
    qDebug() << "Queue last item:" << queue->last();

}
