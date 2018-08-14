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

    signInFlag=false;
    waitForRecive = new QTimer();
    connect(waitForRecive, SIGNAL(timeout()), this, SLOT(noMoreData()));
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

    connect(socket, SIGNAL(readyRead()), this, SLOT(readyRead()), Qt::DirectConnection);
    connect(this, SIGNAL(dataWaiting()), this, SLOT(processReceivedData()), Qt::DirectConnection);
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

    if( op==9 || op==11){
        socket->waitForReadyRead();
    }

}
void Connection::readyRead(){
    QByteArray data(socket->readAll());


    //Get data size from data.
    unsigned int dataSize;
    QDataStream ds(data.mid(0,4));
    ds >> dataSize;

    //Get operation protocol form data.
    unsigned int sizeOfPayloadAndOp=data.mid(4).size();

    int op=QString(data.mid(4,1)).data()->unicode();

    qDebug() << "intOp:" << op;


    qDebug() << "readyRead:" << data;

    //Only data with no loss will process.
    if(sizeOfPayloadAndOp!=dataSize){

        qDebug() << "LOSS!!!!!!!!!!!!!!!!!!!!!!!";
        splitPacket=true;
        receiveBuffer.append(data);

        unsigned int dataSize;
        QDataStream ds(receiveBuffer.mid(0,4));
        ds >> dataSize;

        unsigned int sizeOfPayloadAndOp=receiveBuffer.mid(4).size();

        if(sizeOfPayloadAndOp==dataSize){

            qDebug() << "+++++++++++BUFFER HAVE IT ALL, START TO PROCESS DATA+++++++++++++";

            QByteArray allData=receiveBuffer;

            splitPacket=false;
            receiveBuffer.clear();

            if(signInFlag==true){
                waitForRecive->start(5000);
                if(QString(allData.mid(4,1)).data()->unicode() < 15){
                    receivedData.enqueue(allData);
            qDebug() << "******************** ADD TO QUEUE **";
                    emit dataWaiting();
                }
                else{
            qDebug() << "******************** ADD TO QUEUE **";
                    receivedData.enqueue(allData);
                }
            }
            else{
                receivedData.enqueue(allData);
        qDebug() << "******************** ADD TO QUEUE **";
                emit dataWaiting();
            }

        }
        else if(sizeOfPayloadAndOp > dataSize){

            QString msg=QString::fromStdString(data.toStdString());

            if(msg.split("END PGP MESSAGE-----\n").count() > 1){
                for(int i=0; i < msg.split("END PGP MESSAGE-----\n").count(); i++){
                    QString item=msg.split("END PGP MESSAGE-----\n").at(i);
                    if(item.right(5)=="-----"){

                        qDebug() << "=============== Read multiple packet from socket case !!";

                        item=item+"END PGP MESSAGE-----\n";

                        QByteArray data;
                        data.append(item.toLatin1());

                        splitPacket=false;
                        receiveBuffer.clear();

                        if(signInFlag==true){
                            waitForRecive->start(5000);
                            if(QString(data.mid(4,1)).data()->unicode() < 15){
                                receivedData.enqueue(data);
                            qDebug() << "******************** ADD TO QUEUE **";
                                emit dataWaiting();
                            }
                            else{
                                receivedData.enqueue(data);
                            qDebug() << "******************** ADD TO QUEUE **";
                            }
                        }
                        else{
                            receivedData.enqueue(data);
                        qDebug() << "******************** ADD TO QUEUE **";
                            emit dataWaiting();
                        }

                    }
                    else if(item.right(5)==""){
                        qDebug() << "RIGHT 5 :" << item.right(5);
                        qDebug() << "+++++++++++BUFFER HAVE IT ALL, THIS PART ARE NOTHING+++++++++++++";
                    }
                    else{
                        qDebug() << "RIGHT 5 :" << item.right(5);
                        qDebug() << "-----------------BUFFER STILL NOT GET ALL DTATA------------------x";
                        splitPacket=true;
                        receiveBuffer.clear();
                        receiveBuffer.append(item);

                        unsigned int dataSize;
                        QDataStream ds(receiveBuffer.mid(0,4));
                        ds >> dataSize;

                        //Get operation protocol form data.
                        unsigned int sizeOfPayloadAndOp=receiveBuffer.mid(4).size();

                        if(sizeOfPayloadAndOp==dataSize){

                            qDebug() << "+++++++++++BUFFER HAVE IT ALL, START TO PROCESS DATA+++++++++++++";

                            QByteArray allData=receiveBuffer;

                            splitPacket=false;
                            receiveBuffer.clear();

                            if(signInFlag==true){
                                waitForRecive->start(5000);
                                if(QString(allData.mid(4,1)).data()->unicode() < 15){
                                    receivedData.enqueue(allData);
                            qDebug() << "******************** ADD TO QUEUE **";
                                    emit dataWaiting();
                                }
                                else{
                            qDebug() << "******************** ADD TO QUEUE **";
                                    receivedData.enqueue(allData);
                                }
                            }
                            else{
                                receivedData.enqueue(allData);
                        qDebug() << "******************** ADD TO QUEUE **";
                                emit dataWaiting();
                            }

                        }
                    }
                }
            }
            else{
                qDebug() << "-----------------BUFFER STILL NOT GET ALL DTATA-----------------";

            }

        }
        else{
            qDebug() << "-----------------BUFFER STILL NOT GET ALL DTATA-----------------";
        }

    }
    else{
        splitPacket=false;
        receiveBuffer.clear();


        if(signInFlag==true){
            waitForRecive->start(5000);
            if(QString(data.mid(4,1)).data()->unicode() < 15){
                receivedData.enqueue(data);
            qDebug() << "******************** ADD TO QUEUE **";
                emit dataWaiting();
            }
            else{
                receivedData.enqueue(data);
            qDebug() << "******************** ADD TO QUEUE **";
            }
        }
        else{
            receivedData.enqueue(data);
        qDebug() << "******************** ADD TO QUEUE **";
            emit dataWaiting();
        }

    }

}

void Connection::noMoreData(){
    waitForRecive->stop();
    signInFlag=false;
    processReceivedData();
}

void Connection::processReceivedData(){

    if(!receivedData.isEmpty()){

        QByteArray firstInQueue=receivedData.first();
        int firstOp = QString(firstInQueue.mid(4,1)).data()->unicode();

        qDebug() << "firstOp:" << firstOp;
        qDebug() << "signInFlag:" << signInFlag;

        // Do not process Op=15 and Op=18 when sign-in still not finish.
        if(firstOp >= 15 && signInFlag==true){
            qDebug() << "RETURN 15 true";
            return;
        }


        while(!receivedData.isEmpty()){
            QByteArray data=receivedData.dequeue();
            int op=QString(data.mid(4,1)).data()->unicode();

            //qDebug() << "intOp:" << op;

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
            else if(op==4){
                emit receiveSignUpResult(data);
            }
            else if(op==6){
                emit receiveSignInVerify(data);
            }
            else if(op==8){
                emit receiveSignInResult(data);
            }
            else if(op==10 || op==12){
                recentReceivedMsg=data;
            }
            else if(op==13){
                emit receiveAddFriendrequest(data);
            }
            else if(op==15){
                emit receiveNewPublicKey(data);
            }
            else if(op==18){
                emit receiveNewMessage(data);
            }
            else{
                qDebug() << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!";
                qDebug() << "!!!!!!!!!! RECEIVE UNKNOWN OPERATION !!!!!!!!!!!";
                qDebug() << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!";
                qDebug() << "UNKNOWN DATA:" << data;

            }
        }
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
