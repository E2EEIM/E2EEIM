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
#include "clienttask.h"
#include "encryption.h"
#include <QFile>
#include <QTime>

ClientTask::ClientTask(qintptr socketDescriptor, QQueue<QByteArray> *queue, QList<QString> *usernameList,
                       QList<QString> *userKeyList, QList<QString> *loginUser,
                       QList<QString> *loginRanNum, QList<QString> *waitingTaskUser,
                       QList<QString> *waitingTaskWork, QList<QString> *addFriendRequestList, QString keyFpr, QObject *parent) : QObject(parent)
{
    TRACE=false;

#ifdef QT_DEBUG
    TRACE=true;
#endif

    //Init pointer to share data with every thread.
    this->socketDescriptor=socketDescriptor;
    queuePtr=queue;
    this->usernameList=usernameList;
    this->userKeyList=userKeyList;
    this->loginUser=loginUser;
    this->loginRanNum=loginRanNum;
    this->waitingTaskUser=waitingTaskUser;
    this->waitingTaskWork=waitingTaskWork;
    this->addFriendRequestList=addFriendRequestList;
    this->serverKeyFpr=keyFpr;

    //Begin setup of GPGME
    gpgme_check_version (NULL);
    setlocale (LC_ALL, "");
    gpgme_set_locale (NULL, LC_CTYPE, setlocale (LC_CTYPE, NULL));
//#ifndef HAVE_W32_SYSTEM
//    gpgme_set_locale (NULL, LC_MESSAGES, setlocale (LC_MESSAGES, NULL));
//#endif
    //err = gpgme_engine_check_version (GPGME_PROTOCOL_OpenPGP);
    //detectError(err);
    //End setup of GPGME

    // Create the GPGME Context
    err = gpgme_new (&ctx);
    // Error handling
    detectError(err);

    // Set the context to textmode
    gpgme_set_textmode (ctx, 1);
    // Enable ASCII armor on the context
    gpgme_set_armor (ctx, 1);


    //Gey server key;
    ServerKey=getKey(serverKeyFpr);

    if(ServerKey){ //Found server key;

        if(TRACE) qDebug() << "SERVER KEY IS...";
        if(TRACE) printKeys(ServerKey);

        const char *serverPubKeyFile="serverPubKey.key";

        //Export server's public key.
        exportKey(ctx, ServerKey, err, serverPubKeyFile);
        if(TRACE) qDebug() << "'serverPubKey.key' Saved in current directory\n\n";
    }

    //Read exported server's key from text file.
    QFile f("serverPubKey.key");
    f.open(QFile::ReadOnly | QFile::Text);

    QTextStream in(&f);
    QString infile;
    infile+=in.readAll();
    f.flush();
    f.close();

    const QString tmp=infile;
    serverPubKey.append(tmp);
}

//Init data when thread start.
void ClientTask::initClient(){

    //Create socket for the client.
    socket=new QTcpSocket();

    //Create timer for get thread idle state.
    timer=new QTimer();

    //Set socket to comunicate with the client.
    if(!socket->setSocketDescriptor(socketDescriptor)){
        emit error(socket->error());
        return;
    }

    //Connect readyRead() event.
    //Creat a [signal/slot] mechanism, when receive data from the client
    //to call readyRead() function.
    connect(socket, SIGNAL(readyRead()), this, SLOT(readyRead()));

    //Connect disconnect() event.
    //Creat a [signal/slot] mechanism, when the client disconnect.
    //to call disconnect() Slot.
    connect(socket,SIGNAL(disconnected()), this, SLOT(disconnected()));

    //Connect idle event.
    //Creat a [signal/slot] mechanism, when thread idle.
    //to call task() function.
    connect(timer, SIGNAL(timeout()), this, SLOT(task()));

    splitPacket=false;

}

//Start timer when thread idle.
void ClientTask::timerStart(){
    timer->start(1000);
}

//Send data to the client.
void ClientTask::send(QByteArray data){

    if(TRACE) qDebug() << "---------------------Send data to: " << activeUser;

    //Write data to socket.
    socket->write(data);

    //Tranmit data.
    socket->flush();

    //Wait for write data finish.
    socket->waitForBytesWritten(1000);
}

//Read data when reaceive from the client.
void ClientTask::readyRead(){

    if(TRACE) qDebug() << "---------Receive data from: " << activeUser;

    //Read data from socket.
    QByteArray data(socket->readAll());

    if(TRACE) qDebug() << "readyRead()" << data;

    //Get data size from data.
    unsigned int dataSize;
    QDataStream ds(data.mid(0,4));
    ds >> dataSize;

    //Get operation protocol form data.
    unsigned int sizeOfPayloadAndOp=data.mid(4).size();

    //Only data with no loss will process.
    if(sizeOfPayloadAndOp!=dataSize){

        if(TRACE) qDebug() << "LOSS!!!!!!!!!!!!!!!!!!!!!!!";
        splitPacket=true;
        receiveBuffer.append(data);

        unsigned int dataSize;
        QDataStream ds(receiveBuffer.mid(0,4));
        ds >> dataSize;

        unsigned int sizeOfPayloadAndOp=receiveBuffer.mid(4).size();

        if(sizeOfPayloadAndOp==dataSize){

            if(TRACE) qDebug() << "+++++++++++BUFFER HAVE IT ALL, START TO PROCESS DATA+++++++++++++";

            QByteArray allData=receiveBuffer;

            splitPacket=false;
            receiveBuffer.clear();

            dataFilter(allData);
        }
        else if(sizeOfPayloadAndOp > dataSize){

            QString msg=QString::fromStdString(receiveBuffer.toStdString());

            if(msg.split("END PGP MESSAGE-----\n").count() > 1){
                for(int i=0; i < msg.split("END PGP MESSAGE-----\n").count(); i++){
                    QString item=msg.split("END PGP MESSAGE-----\n").at(i);
                    if(item.right(5)=="-----"){

                        if(TRACE) qDebug() << "=============== Read multiple packet from socket case !!";

                        item=item+"END PGP MESSAGE-----\n";

                        QByteArray data;
                        data.append(item.toLatin1());

                        splitPacket=false;
                        receiveBuffer.clear();
                        dataFilter(data);
                    }
                    else if(item.right(5)==""){
                        if(TRACE) qDebug() << "RIGHT 5 :" << item.right(5);
                        if(TRACE) qDebug() << "+++++++++++BUFFER HAVE IT ALL, THIS PART ARE NOTHING+++++++++++++";
                    }
                    else{
                        if(TRACE) qDebug() << "RIGHT 5 :" << item.right(5);
                        if(TRACE) qDebug() << "-----------------BUFFER STILL NOT GET ALL DTATA-----------------x";
                        splitPacket=true;
                        receiveBuffer.clear();
                        receiveBuffer.append(item);

                        unsigned int dataSize;
                        QDataStream ds(receiveBuffer.mid(0,4));
                        ds >> dataSize;

                        //Get operation protocol form data.
                        unsigned int sizeOfPayloadAndOp=receiveBuffer.mid(4).size();


                        if(sizeOfPayloadAndOp==dataSize){

                            if(TRACE) qDebug() << "+++++++++++BUFFER HAVE IT ALL, START TO PROCESS DATA+++++++++++++";

                            QByteArray allData=receiveBuffer;

                            splitPacket=false;
                            receiveBuffer.clear();

                            dataFilter(allData);
                        }
                    }
                }
            }
            else{
                if(TRACE) qDebug() << "-----------------BUFFER STILL NOT GET ALL DTATA-----------------";

            }

        }
        else{
            if(TRACE) qDebug() << "-----------------BUFFER STILL NOT GET ALL DTATA-----------------";
        }

    }
    else{
        splitPacket=false;
        receiveBuffer.clear();
        dataFilter(data);

    }

}

//Send waiting data for the client when client connect to server.
void ClientTask::task(){

    QString task;
    QString taskProtocol;

    while(waitingTaskUser->indexOf(activeUser)!=(-1)){ //Looking for data the need to send to the client.

        //Slow thread to let client process data from server.
        emit slowDown();

        //Get message that need to send to the client.
        task=waitingTaskWork->at(waitingTaskUser->indexOf(activeUser));
        taskProtocol=task.mid(0,2);

        if(taskProtocol=="13"){ //In case the message for the client is add friend request.
            if(TRACE) qDebug() << "*13*13*13*13*13*13*13*13*13*13*13*13*13*13*13*13*13";

            //Get sender data.
            QString sender=task.mid(2);
            gpgme_key_t senderKey=getKey(sender);
            QString senderUsername=QString(senderKey->uids->name);

            //Create data package.
            QByteArray data;
            data.append(senderUsername);

            //Encrypt paylaod.
            QByteArray payload=encryptToClient(data, activeUser, "addFriendCon.cipher");

            //Clear data package.
            data.clear();

            //Append encrypted payload to data package.
            data.append(payload);

            //In insert add freind requst protocol number.
            data.insert(0, (char)13);


            //Insert size of(operation + payload) in front of byte array (data[0]).
            unsigned int dataSize=data.size();
            QByteArray dataSizeByte;
            QDataStream ds(&dataSizeByte, QIODevice::WriteOnly);
            ds << dataSize;
            data.insert(0, dataSizeByte);

            //Send data to the client.
            send(data);

            //Remove the waiting message for waiting list.
            waitingTaskWork->removeAt(waitingTaskUser->indexOf(activeUser));
            waitingTaskUser->removeAt(waitingTaskUser->indexOf(activeUser));
        }
        if(taskProtocol=="15"){ //In case the message for the client is public key.

            QString payload=task.mid(2);

            //Create data package.
            QByteArray data;

            //Append encrypted payload to data package.
            data.append(payload);

            //Insert public key exchange protocol to data package.
            data.insert(0, (char)15);

            //Insert size of(operation + payload) in front of byte array (data[0]).
            int dataSize=data.size();
            QByteArray dataSizeByte;
            QDataStream ds(&dataSizeByte, QIODevice::WriteOnly);
            ds << dataSize;
            data.insert(0, dataSizeByte);

            //Send encrypted public key to the client.
            send(data);

            //Remove the waing task from waiting list.
            waitingTaskWork->removeAt(waitingTaskUser->indexOf(activeUser));
            waitingTaskUser->removeAt(waitingTaskUser->indexOf(activeUser));

        }
        if(taskProtocol=="18"){ //In case the message for the client is send post message.

            QString cipher=task.mid(2);

            //Create data package.
            QByteArray data;

            //Append encrypted message to data package.
            data.append(cipher);

            //Encrypt message to the client.
            QByteArray payload=encryptToClient(data, activeUser, "addFriendCon.cipher");

            //Empty data package.
            data.clear();

            //Append encrypted cipher to data package.
            data=payload;

            //Inset send post message potocol to data package.
            data.insert(0, (char)18);

            //Insert size of(operation + payload) in front of byte array (data[0]).
            int dataSize=data.size();
            QByteArray dataSizeByte;
            QDataStream ds(&dataSizeByte, QIODevice::WriteOnly);
            ds << dataSize;
            data.insert(0, dataSizeByte);

            //Send encrypted cipher to the client.
            send(data);

            //Remove the waiting data for the client from waiting list.
            waitingTaskWork->removeAt(waitingTaskUser->indexOf(activeUser));
            waitingTaskUser->removeAt(waitingTaskUser->indexOf(activeUser));

        }

    }


}

//Process received data.
void ClientTask::dataFilter(QByteArray data){

    //bool ok;

    //Get process protocol of the data package.
    int intOp=QString(data.mid(4,1)).data()->unicode();
    if(TRACE) qDebug() << "intOp:" << intOp;

    if(intOp==1){ //In case protocol is connection request.

        //printDataDetail(data);

        //Create <-*2 package for reply
        data.clear();

          //-Append server's public key to Byte Array
        data.append(serverPubKey);

          //-Insert operation to index 0
        data.insert(0,(char)2);

          //-Insert size of (operation + payload) to index 0
        unsigned int payloadAndOpSize=serverPubKey.size()+1;
        QByteArray dataSize;
        QDataStream ds(&dataSize, QIODevice::WriteOnly);
        ds << payloadAndOpSize;
        data.insert(0, dataSize);

        send(data);

    }
    if(intOp==3){ //In case protocol is sign up request.

        //printDataDetail(data);

        //decrypt data to get payload
        QByteArray payload=decryptData(data, "signUp.pgp");

        //Get username length from data package.
        int usernameLength=QString(payload).mid(0,1).data()->unicode();

        //Get uername from data package.
        QString username=QString(payload).mid(5, usernameLength);

        //Get user data key length.
        //int userPublicKeyLength;
        //userPublicKeyLength = QString(payload).mid(1,4).toUInt(&ok, 16);


        QString result;

        //Username validation and available.
        if(usernameList->indexOf(username)==(-1)){

            //Sign Up for the client.
            addNewUser(payload);

            result="Sign up success, "+username+" ready for sign in!";

            //Create <-*4 sign up result
            data.clear();
            QByteArray signUpResult;
            signUpResult.append(result);
            QString userSubkey=userKeyList->at(usernameList->indexOf(username));

            QByteArray ciper=encryptToClient(signUpResult, userSubkey, "signUpResult.ciper");

            //-Append server's encrypted sign up result message to Byte Array
            data.append(ciper);

        }
        else{

            result= "Username: "+username+" not available!";

            //Create <-*4 sign up result
            data.clear();
            data.append(result);

        }



        //Send sign up result


        //-Insert operation to index 0
        data.insert(0,(char)4);

        //-Insert size of (operation + payload) to index 0
        unsigned int payloadAndOpSize=data.size();
        QByteArray dataSize;
        QDataStream ds2(&dataSize, QIODevice::WriteOnly);
        ds2 << payloadAndOpSize;
        data.insert(0, dataSize);

        if(TRACE) qDebug() << "====payloadAndOpSize:" << payloadAndOpSize;

        if(true){
            //Get data size from data.
                unsigned int dataSize;
                QDataStream ds(data.mid(0,4));
                ds >> dataSize;

                //Get operation protocol form data.
                unsigned int sizeOfPayloadAndOp=data.mid(4).size();


                if(TRACE) qDebug() << "readyRead:" << data;

                if(TRACE) qDebug() << sizeOfPayloadAndOp << "-" << dataSize;
        }

        //Send sign up result.
        send(data);

    }

    if(intOp==5){ //In case protocol is sign in request.

        //printDataDetail(data);

        //decrypt data to get payload
        QByteArray payload=decryptData(data, "signIn.pgp");

        //Get username length from data package.
        int usernameLength=QString(payload).mid(0,1).data()->unicode();

        //Get username from data package.
        QString username(payload.mid(5, usernameLength));

        //Get user's public key length.
        //int userPublicKeyLength;
        //userPublicKeyLength = QString(payload).mid(1,4).toUInt(&ok, 16);

        //Search for the username in server.
        if(usernameList->indexOf(username)!=(-1)){
            gpgme_key_t userKey=getKey(userKeyList->at(usernameList->indexOf(username)));

            if(TRACE) printKeys(userKey);

            //random number.
            qsrand(QTime::currentTime().msec());
            int rNum = qrand();
            QByteArray qb;
            qb.setNum(rNum);

            //Save sign in ID.
            if(loginUser->indexOf(username)==(-1)){

                loginUser->append(username);
                loginRanNum->append(QString(qb));
            }
            else{
                int idx=loginUser->indexOf(username);
                loginRanNum->replace(idx, QString(qb));
            }


            //Encrypt sign in verifiaction.
            QByteArray cipher=encryptToClient(qb, username, "rannum.cipher");

            //Empty data package variable.
            data.clear();

            //Append encrypted sign in verification to data package.
            data.append(cipher);

            // Insert sign in verification protocol number in front of byte array (data[0]).
            data.insert(0, (char)6);

            //Insert size of(operation + payload) in front of byte array (data[0]).
            unsigned int dataSize=data.size();
            QByteArray dataSizeByte;
            QDataStream ds(&dataSizeByte, QIODevice::WriteOnly);
            ds << dataSize;
            data.insert(0, dataSizeByte);

            //Send encrypted sign in verification to client.
            send(data);


        }
        else{ //In case the username not found in this server.

            //Create sign in result.
            QByteArray qb="USER NOT FOUND IN THIS SERVER!";

            //Empty data package variable.
            data.clear();

            //Append sign in result to data package.
            data.append(qb);

            // Insert sign in reuslt protocol number in front of byte array (data[0]).
            data.insert(0, (char)6);

            //Insert size of(procols number + payload) in front of byte array (data[0]).
            unsigned int dataSize=data.size();
            QByteArray dataSizeByte;
            QDataStream ds(&dataSizeByte, QIODevice::WriteOnly);
            ds << dataSize;
            data.insert(0, dataSizeByte);

            //Send sign in result to the client.
            send(data);


        }

    }
    if(intOp==7){ //In case protocol is sign in verification.

        //printDataDetail(data);

        //Decrypt payload
        QByteArray payload=decryptData(data, "signInVerify.pgp");

        QString dataQS=QString(payload);

        //Get verfiy number.
        QString verifyNum=dataQS.split("@@").first();

        //Get fingerprint of the key that decrypt payload.
        QString keyID=dataQS.split("@@").last();

        //Get key for fingerprint.
        gpgme_key_t userKey=getKey(keyID);

        //Get user of key.
        QString username=(userKey->uids->name);

        //Get index of sign in ID.
        int userIndex=loginUser->indexOf(username);

        QByteArray qb;

        //Verify login.
        if(loginRanNum->at(userIndex)==verifyNum){

            //Create sign in result.
            qb = "verify success!!!";
            activeUser=username;
        }
        else{

            //Create sign in result.
            qb = "Server couldn't verify this account's private key belongs to you!";
        }

        //Encrypt sign in result to the client.
        QByteArray cipher=encryptToClient(qb, username, "signInResult.cipher");

        //Empty data package variable.
        data.clear();

        //Append encrypted sign in result to data package.
        data.append(cipher);

        // Insert operation(sign in result protocol) in front of byte array (data[0]).
        data.insert(0, (char)8);

        //Insert size of(operation + payload) in front of byte array (data[0]).
        unsigned int dataSize=data.size();
        QByteArray dataSizeByte;
        QDataStream ds(&dataSizeByte, QIODevice::WriteOnly);
        ds << dataSize;
        data.insert(0, dataSizeByte);

        //Send encrypted sign in reuslt to the client.
        send(data);

        if(qb=="verify success!!!"){
            task();
        }

    }
    if(intOp==9){ //In case protocol is search for user.

        //printDataDetail(data);
        //Decrypt payload.
        QByteArray payload=decryptData(data, "username.keyword");

        QString dataQS=QString(payload);

        //Get keyword and sender.
        QString keyword=dataQS.split("@@").first();
        QString sender=dataQS.split("@@").last();

        //Search for keywork.
        if(usernameList->indexOf(keyword)==(-1)){// In case not found.

            //Empty data package variable.
            payload.clear();

            //Append search result to data package.
            payload.append("0");
        }
        else{ //In case found the user.

            //Get found user key fingerprint.
            int idx=usernameList->indexOf(keyword);
            QString userKey=userKeyList->at(idx);

            //Clear data package variable.
            payload.clear();

            //Append search result to data package.
            payload.append("1\n");
            payload.append(keyword+"\n");
            payload.append(userKey);
        }

        //Encrypt search result.
        QByteArray cipher=encryptToClient(payload, sender, "searchUser.cipher");

        //Empty data package variable.
        data.clear();

        //Append encrypted search result.
        data.append(cipher);

        // Insert operation(search result protocol) in front of byte array (data[0]).
        data.insert(0, (char)10);

        //Insert size of(operation + payload) in front of byte array (data[0]).
        unsigned int dataSize=data.size();
        QByteArray dataSizeByte;
        QDataStream ds(&dataSizeByte, QIODevice::WriteOnly);
        ds << dataSize;
        data.insert(0, dataSizeByte);

        //Send encrypted search result to the client.
        send(data);

    }
    if(intOp==11){ //In case protocol is add friend request.

        //printDataDetail(data);

        //Decrypt paylaod.
        QByteArray payload=decryptData(data, "username.keyword");

        QString dataQS=QString(payload);

        //Get sender and recipeint.
        QString keyword=dataQS.split("@@").first();
        QString sender=dataQS.split("@@").last();

        //Search for recpient.
        if(usernameList->indexOf(keyword)==(-1)){ //In case not found recipeint.
            //Empty paylaod
            payload.clear();
            //Append add friend requst result.
            payload.append("0");
        }
        else{ //In case found recipeint.

            //Empty payload.
            payload.clear();

            //Append add friend request result.
            payload.append("1");

            //Create waiting message for recipeint.
            QString taskUser=keyword;
            QString taskWork="13"+sender;

            //Add waiting message to waiting list.
            waitingTaskUser->append(taskUser);
            waitingTaskWork->append(taskWork);

            //Get sender key.
            gpgme_key_t senderkey = getKey(sender);

            //Get sender username.
            sender=QString(senderkey->uids->name);

            //Add the requst to server's add friend request list for verify.
            addFriendRequestList->append(sender+"@@"+keyword);

        }

        //Encrypt request result for sender.
        QByteArray cipher=encryptToClient(payload, sender, "searchUser.cipher");

        //Empty data package variable.
        data.clear();

        //Append encrypted result to data package.
        data.append(cipher);

        // Insert operation(add friend request result protocol) in front of byte array (data[0]).
        data.insert(0, (char)12);

        //Insert size of(operation + payload) in front of byte array (data[0]).
        unsigned int dataSize=data.size();
        QByteArray dataSizeByte;
        QDataStream ds(&dataSizeByte, QIODevice::WriteOnly);
        ds << dataSize;
        data.insert(0, dataSizeByte);

        //Send request result to sender.
        send(data);
    }
    if(intOp==14){ //In case protocols is accept add firend request.

        //printDataDetail(data);

        //Decrypt payload.
        QByteArray payload=decryptData(data, "username.keyword");

        QString dataQS=QString(payload);

        //Get request sender username and confirm sender username.
        QString keyword=dataQS.split("@@").first();
        QString sender=dataQS.split("@@").last();

        //Get request sernder public key and confirm sender public key.
        gpgme_key_t username_sendRequest = getKey(keyword);
        gpgme_key_t username_sendConfirm = getKey(sender);


        //Verify confirm and request.
        sender=QString(username_sendConfirm->uids->name);

        QString addFriendRequestID=keyword+"@@"+sender;

        if(addFriendRequestList->indexOf(addFriendRequestID)!=-1){ //In case found the request of the confirm.

            //Export request sender public key to text file.
           exportKey(ctx, username_sendRequest, err, "temp.key");

           //Read request sender public eky form text file.
           QFile outFile("temp.key");
           if(!outFile.open(QFile::ReadOnly | QFile::Text)){
               if(TRACE) qDebug() << "cound not open file for writing";
               abort();
           }
           QTextStream in(&outFile);
           QString dataStream;
           dataStream=in.readAll();
           outFile.flush();
           outFile.close();

           //Create data package variable.
           QByteArray userReqKey;

           //Append request sender public key to data package.
           userReqKey.append(dataStream);

           //Insert request sender username to data package.
           QString usernameOfRequest=QString(username_sendRequest->uids->name);
           userReqKey.insert(0, usernameOfRequest);

           //Insert request sender username length to data package.
           char usernameSize=char(usernameOfRequest.size());
           userReqKey.insert(0, usernameSize);

           //Encrypt data package for confirm sender.
           encryptToClient(userReqKey, sender, "key.cipher");

           //Read encrypted data package for text file.
           QFile outFile3("key.cipher");
           if(!outFile3.open(QFile::ReadOnly | QFile::Text)){
               if(TRACE) qDebug() << "cound not open file for writing";
               abort();
           }
           QTextStream in3(&outFile3);
           QString dataStream3;
           dataStream3=in3.readAll();
           outFile3.flush();
           outFile3.close();

           //Create data pacakge for confirm sender.
           QByteArray qb;

           //Append request sender public key to data package for confirm sender.
           qb.append(dataStream3);

           //Add data for confirm sender to data waiting list.
           waitingTaskUser->append(sender);
           waitingTaskWork->append("15"+qb);

           //Export confirm sender public key to a text file.
           exportKey(ctx, username_sendConfirm, err, "temp.key");

           //Read confirm sender public key from text file.
           QFile outFile2("temp.key");
           if(!outFile2.open(QFile::ReadOnly | QFile::Text)){
               if(TRACE) qDebug() << "cound not open file for writing";
               abort();
           }
           QTextStream in2(&outFile2);
           QString dataStream2;
           dataStream2=in2.readAll();
           outFile2.flush();
           outFile2.close();

           //Create data package for request sender.
           QByteArray userConKey;

           //Append confirm sender public key to data package for request sender.
           userConKey.append(dataStream2);

           //Insert confirm sender username to data package for request sender.
           QString usernameOfConfirm=QString(username_sendConfirm->uids->name);
           userConKey.insert(0, usernameOfConfirm);

           //Insert confirm sender username length to data package for request sender.
           usernameSize=char(usernameOfConfirm.size());
           userConKey.insert(0, usernameSize);

           //Encrypt data for request sender.
           encryptToClient(userConKey, keyword, "key.cipher");
           QFile outFile4("key.cipher");
           if(!outFile4.open(QFile::ReadOnly | QFile::Text)){
               if(TRACE) qDebug() << "cound not open file for writing";
               abort();
           }
           QTextStream in4(&outFile4);
           QString dataStream4;
           dataStream4=in4.readAll();
           outFile4.flush();
           outFile4.close();

           //Create data waiting item.
           QByteArray qb2;

           //Append encrypted confirm sender to data waiting item.
           qb2.append(dataStream4);

           //Add data waiting item for request sender to data waiting list.
           waitingTaskUser->append(keyword);
           waitingTaskWork->append("15"+qb2);

           //Remove the add Friend request from list.
           addFriendRequestList->removeAt(addFriendRequestList->indexOf(addFriendRequestID));

           //Go to idle state.
           task();

        }
        else{

            if(TRACE) qDebug() << "LOOKING FOR:" << addFriendRequestID;

            for(int i=0; i < addFriendRequestList->length(); i++){
                if(TRACE) qDebug() <<"addFriendRequestList" << addFriendRequestList->at(i);
            }

        }
    }
    if(intOp==17){ //In case the protocol is post message.

        //printDataDetail(data);

        //Decrypt payload.
        QByteArray payload=decryptData(data, "msg.cipher");

        //Verify key signature.
       if(payload.mid(0,1)=="1"){ //In case good signature.
           //Get reciepint username from payload.
           int recipientUsernameSize=QString(payload.mid(1,1)).data()->unicode();
           QString recipientUsername=QString(payload.mid(2, recipientUsernameSize));

           //Search for recipeint username inserver.
           if(usernameList->indexOf(recipientUsername)!=(-1)){ //Found recipient.

               //Add the message for recipent to data waiting list.
               waitingTaskUser->append(recipientUsername);
               waitingTaskWork->append("18"+QString(payload.mid(recipientUsernameSize+2)));
           }
       }
    }

}

//Decrypt data from client.
QByteArray ClientTask::decryptData(QByteArray data, const char* outputFileName){

    QByteArray decrypted; //For decrypted message
    QByteArray cipher=data.mid(5); //Get cipher from data package.
    int intOp=QString(data.mid(4,1)).data()->unicode(); //Get operation number(Protocol)

    //Write cipher to a text file.
    QFile File("temp.cipher");
    if(!File.open(QFile::WriteOnly | QFile::Text)){
        if(TRACE) qDebug() << "cound not open file for writing";
        abort();
    }
    QTextStream out(&File);
    out << cipher;
    File.flush();
    File.close();


    QString verifyResult; //For verify signature result.


    if(intOp==3){ //In case operation number(Protocol number) is 3.

        //Use decrypt() function.
        decrypt(ctx, err, "temp.cipher", outputFileName);
    }
    else{ //In case operation number is not 3.

        //Use decryptVerify() function.
        verifyResult=decryptVerify(ctx, err, "temp.cipher", outputFileName);
    }


    //Read decrypted message from output file.
    QFile outFile(outputFileName);

    if(!outFile.open(QFile::ReadOnly | QFile::Text)){
        if(TRACE) qDebug() << "cound not open file for writing";
        abort();
    }
    QTextStream in(&outFile);
    QString dataStream;
    dataStream=in.readAll();
    outFile.flush();
    outFile.close();


    //Return decrypted message and verify result.
    if(intOp==7 || intOp==9 || intOp==11 || intOp==14){
        decrypted.append(dataStream);
        decrypted.append("@@");
        decrypted.append(verifyResult.mid(1));

        return decrypted;
    }
    else if(intOp==17){

        decrypted.append(verifyResult.mid(0,1));
        decrypted.append(dataStream);

        return decrypted;

    }
    else{
        decrypted.append(dataStream);
        return decrypted;
    }


}

//User sign up.
void ClientTask::addNewUser(QByteArray payload){

    //Get username.
    int usernameLength=QString(payload).mid(0,1).data()->unicode();
    QString username=QString(payload.mid(5, usernameLength));

    // Import user's public key
    QByteArray publicKey=payload.mid(5+usernameLength);

    //Save user public key to a file.
    QFile File("temp.data");
    if(!File.open(QFile::WriteOnly | QFile::Text)){
        if(TRACE) qDebug() << "cound not open file for writing";
        abort();
    }
    QTextStream out(&File);
    out << publicKey;
    File.flush();
    File.close();

    //Import user's public key.
    importKey(ctx, err, "temp.data");

    //Get user's keyID
    gpgme_key_t userKey=getKey(username);
    QString userKeyID(userKey->subkeys->keyid);

    //Add user to register list.
    usernameList->append(username);
    userKeyList->append(userKeyID);
}

//Encrypt message before send to client.
QByteArray ClientTask::encryptToClient(QByteArray data, QString recipient, const char *outFileName){

    //Get recipient public key.
    gpgme_key_t recipientKey=getKey(recipient);

    QByteArray encrypted; //For encrypted messsage.

    //Save raw data to a text file.
    QFile File("temp.data");
    if(!File.open(QFile::WriteOnly | QFile::Text)){
        abort();
    }
    QTextStream out(&File);
    out << data;
    File.flush();
    File.close();

    //Set signer key.
    gpgme_signers_add(ctx, ServerKey);

    //Encrypt and sign message.
    encryptSign(ctx, err, recipientKey, "temp.data", outFileName);

    //Release key in recipientKey variable.
    gpgme_key_unref (recipientKey);


    //Read encrypted message from encrypt output file.
    QFile outFile(outFileName);
    if(!outFile.open(QFile::ReadOnly | QFile::Text)){
        if(TRACE) qDebug() << "cound not open file for writing";
        abort();
    }
    QTextStream in(&outFile);
    QString ciper;
    ciper=in.readAll();
    outFile.flush();
    outFile.close();

    encrypted.append(ciper);

    //return encryped message.
    return encrypted;

}

//Get key.
gpgme_key_t ClientTask::getKey(QString pattern){

    //Convert pattern into const char*
    //that required in search key process.
    gpgme_key_t targetKey;
    QByteArray ba = pattern.toLatin1();
    const char *patt=ba.data();

    //Find key.
    err = gpgme_op_keylist_start(ctx, patt, 0);
    detectError(err);
    int nKeysFound=0;
    while (!(err = gpgme_op_keylist_next(ctx, &key))) { // loop through the keys in the keyring
        targetKey=key;
        nKeysFound++;
    }

    if(nKeysFound==0){
        return targetKey;
    }
    return targetKey;
}

//Client disconnect from server.
void ClientTask::disconnected(){
    qDebug() << socketDescriptor << " Disconnected: ";
    //Delete socket.
    socket->deleteLater();

    //Emit signal to delete thread.
    emit clientDisconnect();
}
