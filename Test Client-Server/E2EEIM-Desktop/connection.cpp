#include "connection.h"

Connection::Connection(QObject *parent) : QObject(parent){

}

void Connection::connect(){

    socket = new QTcpSocket(this);

    socket->connectToHost("127.0.0.1",2222);
    qDebug() << "Waiting for connection";
    if(socket->waitForConnected(3000)){
        qDebug() << "Connected!";
        //socket->close();
    }
    else{
        qDebug() << "Not Connected!";
    }
}

void Connection::send(QString msg){
    QByteArray package=msg.toUtf8();
    socket->write(package);
    socket->flush();
    socket->waitForBytesWritten((1000));

    socket->waitForReadyRead(3000);
    qDebug() << "Reading:" << socket->bytesAvailable();

    QByteArray returnFromServ= socket->readAll();
    QString servRe=QString::fromUtf8(returnFromServ);
    qDebug() << servRe;


}

void Connection::letDisconnect(){
    socket->close();
}
