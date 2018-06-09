/****************************************************************************
** Copyright (C) 2018  Krittitam Juksukit
** This file is part of E2EEIM.
**
** E2EEIM is free software: you can redistribute it and/or modify
** it under the terms of the GNU Lesser General Public License
** version 3 as published by the Free Software Foundation.
**
** E2EEIM is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU Lesser General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with E2EEIM.  If not, see <http://www.gnu.org/licenses/>.
**
*******************************************************************************/

#include "connection.h"
#include "encryption.h"

Connection::Connection(Encryption &encryption, QObject *parent) : QObject(parent){

    this->encryption=&encryption;
    connectStatus=0;


}

void Connection::connected(QString host, QString port){

    //Create data package
    QByteArray data;

    //Add payload
    QByteArray payload=NULL;

    data.append(payload);

    //Insert operation in front of byte array (data[0]).
    QByteArray operation;
    char op_Char=(char)1;
    operation.append(op_Char);

    data.insert(0, operation);

    //Insert Size of (payload + operation) in front of byte array (data[0]).
    QByteArray dataSize;
    unsigned int dataSize_int=operation.size() + payload.size();
    QDataStream ds(&dataSize, QIODevice::WriteOnly);
    ds << dataSize_int;
    data.insert(0, dataSize);

    //Send require connection
    connectStatus=0;
    socket = new QTcpSocket(this);

    connect(socket, SIGNAL(readyRead()), this, SLOT(readyRead()));
    connect(socket, SIGNAL(disconnected()), this, SLOT(letDisconnect()));

    socket->connectToHost(host, port.toInt());
    qDebug() << "Waiting for connection";
    if(socket->waitForConnected(3000)){
        qDebug() << "Connected!";
        send(data);
        socket->waitForReadyRead(20000);

        serverAddr=host;
        serverPort=port;
    }
    else{
        qDebug() << "Not Connected!";
        connectStatus=-1; //not connect
    }
}

void Connection::writeToFile(QByteArray data, QString filename){
    QFile File(filename);
    if(!File.exists()){
        if(!File.open(QFile::WriteOnly | QFile::Text)){
            qDebug() << "cound not open file for writing";
            abort();
        }
        QTextStream out(&File);
        out << data;

        File.flush();
        File.close();
    }
    else{
        if(!File.open(QFile::WriteOnly | QFile::Text)){
            qDebug() << "cound not open file for writing";
            abort();
        }
        QTextStream out(&File);
        out << data;

        File.flush();
        File.close();
    }
}

int Connection::getConnectionStatus(){
    return connectStatus;
}

QString Connection::getServerAddr(){
    return serverAddr;
}
QString Connection::getServerPort(){
    return serverPort;
}

void Connection::send(QByteArray data){
    QByteArray package=data;
    socket->write(package);
    socket->flush();
    socket->waitForBytesWritten((1000));

    int op=QString(data.mid(4,1)).data()->unicode();

    if(op == 9){
       qDebug() << " OP 9 SENDED";
    }

    if(op==3 || op==5 || op==7 || op==9 || op==11){
        socket->waitForReadyRead();
    }

}
void Connection::readyRead(){

    QByteArray data=socket->readAll();
    int op=QString(data.mid(4,1)).data()->unicode();

    qDebug() << "INT OPr:" << op;

    if(op==2){

        if(data.size() < 5){
            connectStatus=2; //2 This server or this port not for E2EEIM.
        }
        else{
            int operation=QString(data.mid(4,1)).data()->unicode();

            if(operation==2){
                //import server public key
                writeToFile(data.mid(5), "servPubKey.key");

                gpgme_import_result_t keyImportResult=encryption->importKey("servPubKey.key");

                //Init server public key
                QString fpr=QString(keyImportResult->imports->fpr);
                QByteArray ba=fpr.toLatin1();
                const char *patt=ba.data();

                gpgme_key_t serverKey;
                serverKey = encryption->getKey(patt, 0);
                encryption->setServerKey(serverKey);

                connectStatus=1; //connected;

            }
            else{
                connectStatus=2; //2 This server or this port not for E2EEIM.
            }
        }
    }

    if(op==4 || op==6 || op==8 || op==10 || op==12){
        recentReceivedMsg=data;
    }

    if(op==13){
        qDebug() << "It should emit";
        emit receiveAddFriendrequest(data);
    }
    if(op==15){
        qDebug() << "Receive new public key!!!";
        emit receiveNewPublicKey(data);
    }
    else{
        qDebug() << "New data arrived!!!!!!!!!!";
    }

}
QByteArray Connection::getRecentReceivedMsg(){
    return recentReceivedMsg;
}

void Connection::letDisconnect(){
    socket->close();
    connectStatus=-2;
    emit disconnectFromServer();
}
