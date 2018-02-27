#include "mythread.h"

MyThread::MyThread(QQueue<QByteArray> *queue, int ID, QObject *parent) :
    QThread(parent)
{
    this->socketDescriptor = ID;
    queuePtr=queue;


}

void MyThread::run(){

    //Thread starts here.
    qDebug() << socketDescriptor << " Starting thread";
    socket = new QTcpSocket();
    if(!socket->setSocketDescriptor(this->socketDescriptor)){

        emit error(socket->error());
        return;
    }

    connect(socket,SIGNAL(readyRead()), this, SLOT(readyRead()),Qt::DirectConnection);
    connect(socket,SIGNAL(disconnected()), this, SLOT(disconnected()),Qt::DirectConnection);

    qDebug() << socketDescriptor << " Client connected";


    exec();
}

void MyThread::send(){
    qDebug() << "SEND";
}

void MyThread::readyRead(){

    QByteArray Data = socket->readAll();

    qDebug() << socketDescriptor << " Data in: " << Data;

    emit this->newMessage(Data);

    Data="Server Recevied your message as:" + Data;

    socket->write(Data);

    qDebug() << socketDescriptor << " Data out: " << Data;

}

void MyThread::disconnected(){

    qDebug() << socketDescriptor << " Disconnected: ";

    socket->deleteLater();
    exit(0);
}
