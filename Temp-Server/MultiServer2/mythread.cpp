#include "mythread.h"
#include "encryption.h"
#include <QFile>
#include <QString>
#include <QHostAddress>

MyThread::MyThread(QQueue<QByteArray> *queue, QList<QString> *usernameList,
                   QList<QString> *userKeyList, QList<QString> *loginUser,
                   QList<QString> *loginRanNum, QList<QString> *waitingTaskUser,
                   QList<QString> *waitingTaskWork, QList<QString> *addFriendRequestList, int ID, QObject *parent) :
    QThread(parent)
{

    queuePtr=queue;
    this->usernameList=usernameList;
    this->userKeyList=userKeyList;
    this->loginUser=loginUser;
    this->loginRanNum=loginRanNum;
    this->waitingTaskUser=waitingTaskUser;
    this->waitingTaskWork=waitingTaskWork;
    this->addFriendRequestList=addFriendRequestList;

    this->socketDescriptor = ID;

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


    const char *servKey= "E2EEIM SERVER";

    err = gpgme_op_keylist_start(ctx,servKey, 1);
    detectError(err);

    int nKeysFound=0;

    while (!(err = gpgme_op_keylist_next(ctx, &key))) { // loop through the keys in the keyring
        ServerKey=key;
        nKeysFound++;
    }

    if(nKeysFound == 0){
        const char *parms = "<GnupgKeyParms format=\"internal\">\n"
            "Key-Type: RSA\n"
            "Key-Length: 4096\n"
            "Subkey-Type: RSA\n"
            "Subkey-Length: 4096\n"
            "Name-Real: E2EEIM SERVER\n"
            "Name-Comment: (Generated by E2EEIM Chat, Passphrase:abcdefgh)\n"
            "Name-Email: server@e2eeim.chat\n"
            "Expire-Date: 1d\n"
            "Passphrase: abcdefgh\n"
            "</GnupgKeyParms>\n";

        gpgme_genkey_result_t GenKeyresult;
        GenKeyresult = genKey(ctx, err, parms);
        qDebug() << "\n--------Finished Key Generation----------";
        err = gpgme_get_key(ctx,GenKeyresult->fpr,&key,1);
        detectError(err);
        ServerKey = key;

        qDebug() << "SERVER KEY IS...";
        printKeys(ServerKey);

        const char *serverPubKeyFile="serverPubKey.key";

        exportKey(ctx, ServerKey, err, serverPubKeyFile);
        qDebug() << "'serverPubKey.key' Saved in current directory\n\n";
    }
    else{
        qDebug() << "\n-------FOUND USEABLE SERVER KEY----------";
        qDebug() << "SERVER KEY IS...";
        printKeys(ServerKey);

        const char *serverPubKeyFile="serverPubKey.key";

        exportKey(ctx, ServerKey, err, serverPubKeyFile);
        qDebug() << "'serverPubKey.key' Saved in current directory\n\n";
    }

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

void MyThread::run(){

    //Thread starts here.
    qDebug() <<"sockfd No." << socketDescriptor << " Starting thread";

    socket = new QTcpSocket();
    if(!socket->setSocketDescriptor(this->socketDescriptor)){

        emit error(socket->error());
        return;
    }

    qDebug() << "Client peerAddress:"<< socket->peerAddress().toString();
    qDebug() << "Client peerPort:"<< socket->peerPort();


    connect(socket,SIGNAL(readyRead()), this, SLOT(readyRead()),Qt::DirectConnection);
    connect(socket,SIGNAL(disconnected()), this, SLOT(disconnected()),Qt::DirectConnection);

    qDebug() << "Client connected to sockfd No." << socketDescriptor;

    exec();
}

void MyThread::task(){

    QString task;
    QString taskProtocol;
    while(waitingTaskUser->indexOf(activeUser)!=(-1)){
        if(waitingTaskUser->indexOf(activeUser)!=(-1)){
            int idx=waitingTaskUser->indexOf(activeUser);
            qDebug() << "for User:"<< activeUser;
            qDebug() << "Task:" << waitingTaskWork->at(idx);

            task=waitingTaskWork->at(idx);
            taskProtocol=task.mid(0,2);

            if(taskProtocol=="13"){
                qDebug() << "*13*13*13*13*13*13*13*13*13*13*13*13*13*13*13*13*13";

                QString sender=task.mid(2);
                gpgme_key_t senderKey=getKey(sender);
                QString senderUsername=QString(senderKey->uids->name);

                QByteArray data;
                data.append(senderUsername);

                QByteArray payload=encryptToClient(data, activeUser, "addFriendCon.cipher");

                data.clear();
                data.append(payload);

                data.insert(0, (char)13);

                qDebug() << "data:" << data; // ////////////////////////////////

                //Insert size of(operation + payload) in front of byte array (data[0]).
                int dataSize=data.size();
                QByteArray dataSizeByte;
                QDataStream ds(&dataSizeByte, QIODevice::WriteOnly);
                ds << dataSize;
                data.insert(0, dataSizeByte);

                send(data);

                waitingTaskUser->removeAt(idx);
                waitingTaskWork->removeAt(idx);

            }
            if(taskProtocol=="15"){
                qDebug() << "*15*15*15*15*15*15*15*15*15*15*15*15*15*15*15*15*15";

                QString payload=task.mid(2);

                QByteArray data;
                data.append(payload);

                data.insert(0, (char)15);

                //Insert size of(operation + payload) in front of byte array (data[0]).
                int dataSize=data.size();
                QByteArray dataSizeByte;
                QDataStream ds(&dataSizeByte, QIODevice::WriteOnly);
                ds << dataSize;
                data.insert(0, dataSizeByte);

                send(data);

                waitingTaskUser->removeAt(idx);
                waitingTaskWork->removeAt(idx);


            }

        }

    }

}

void MyThread::dataFilter(QByteArray data){

    //QString dataQString(data);
    bool ok;
    int intOp=QString(data.mid(4,1)).data()->unicode();
    qDebug() << "intOp_f:"<< intOp;


    if(intOp==1){
        qDebug() << "*1*1*1*1*1*1*1*1*1*1*1*1*1*1*1*1*1*1*1*1*1*1*1*1*1*1*1*1*1*1*1*1*1*1";
        qDebug() << "*1-> RECEIVED Client require connection";
        printf("+----------------------+----------------------+-----------------+\n");
        printf("|  data Size (4 byte)  |  Operation (1 byte)  |Payload (0 byte) |\n");
        printf("+----------------------+----------------------+-----------------+\n");

        printDataDetail(data);
        qDebug() << "\n\n\n\n\n";

        //Create <-*2 package for reply
        data.clear();

          //-Append server's public key to Byte Array
        data.append(serverPubKey);

          //-Insert operation to index 0
        data.insert(0,(char)2);

          //-Insert size of (operation + payload) to index 0
        int payloadAndOpSize=serverPubKey.size()+1;
        QByteArray dataSize;
        QDataStream ds(&dataSize, QIODevice::WriteOnly);
        ds << payloadAndOpSize;
        data.insert(0, dataSize);

        send(data);

    }
    if(intOp==3){
        qDebug() << "*3*3*3*3*3*3*3*3*3*3*3*3*3*3*3*3*3*3*3*3*3*3*3*3*3*3*3*3*3*3*3*3*3*3";
        qDebug() << "*3-> RECEIVED Client sign up request\n";
        printf("                 1 byte            4 byte           <255      ~4000\n");
        printf("          +------------------+------------------+----------+-----------+\n");
        printf("payload = |  Username Length | UsrPubKey Length | Username | UsrPubKey |\n");
        printf("          +------------------+------------------+----------+-----------+\n");
        printf("\n");
        printf("+---------------------+-------------+----------------------------------+\n");
        printf("|  data Size (4 byte) | OP (1 byte) |          encrypted(payload)      |\n");
        printf("+---------------------+-------------+----------------------------------+\n");

        printDataDetail(data);

        //decrypt data to get payload
        QByteArray payload=decryptData(data, "signUp.pgp");

        qDebug() << "\n-DECRYPTED PAYLOAD-";
        qDebug() << "HEAD:" << QString(payload).mid(0, 20);
        int usernameLength=QString(payload).mid(0,1).data()->unicode();

        QString username=QString(payload).mid(5, usernameLength);

        /*
        int userPublicKeyLength;
        QDataStream ds(payload.mid(1,4));
        ds >> userPublicKeyLength;
        */
        int userPublicKeyLength;
        userPublicKeyLength = QString(payload).mid(1,4).toUInt(&ok, 16);

        qDebug() << "Username length:"<< usernameLength;
        qDebug() << "Username:" << username;
        qDebug() << "User's public key Length:" << userPublicKeyLength;
        qDebug() << "User's public key:"<< "[remove next line's //(comment) to see user's public key]";
        //qDebug() << "User's public key:"<< payload.mid(5+usernameLength);


        QString result;

        if(usernameList->indexOf(username)==(-1)){

            qDebug() << "\n-ADD NEW USER TO LIST-";
            addNewUser(payload);
            qDebug() << "Username:"<< username;
            qDebug() << "User's KeyID:" << userKeyList->at(usernameList->indexOf(username));

            qDebug() << "\n\n\n\n\n";

            result="Sign up success, "+username+" ready for sign in!";

        }
        else{

            result= "Username: "+username+" not available!";

        }



        //Send sign up result

        //Create <-*4 sign up result
        data.clear();
        QByteArray signUpResult;
        signUpResult.append(result);
        QString userSubkey=userKeyList->at(usernameList->indexOf(username));

        qDebug() << "-SIGN UP RESULT ENCRYPTING-";
        QByteArray ciper=encryptToClient(signUpResult, userSubkey, "signUpResult.ciper");

        //-Append server's encrypted sign up result message to Byte Array
        data.append(ciper);

        //-Insert operation to index 0
        data.insert(0,(char)4);

        //-Insert size of (operation + payload) to index 0
        int payloadAndOpSize=serverPubKey.size()+1;
        QByteArray dataSize;
        QDataStream ds2(&dataSize, QIODevice::WriteOnly);
        ds2 << payloadAndOpSize;
        data.insert(0, dataSize);

        send(data);

    }

    if(intOp==5){
        qDebug() << "*5*5*5*5*5*5*5*5*5*5*5*5*5*5*5*5*5*5*5*5*5*5*5*5*5*5*5*5*5*5*5*5*5*5";
        qDebug() << "*5-> RECEIVED Client sign in request\n";
        printf("                 1 byte            4 byte           <255      ~4000\n");
        printf("          +------------------+------------------+----------+-----------+\n");
        printf("payload = |  Username Length | UsrPubKey Length | Username | UsrPubKey |\n");
        printf("          +------------------+------------------+----------+-----------+\n");
        printf("\n");
        printf("+---------------------+-------------+----------------------------------+\n");
        printf("|  data Size (4 byte) | OP (1 byte) |          encrypted(payload)      |\n");
        printf("+---------------------+-------------+----------------------------------+\n");

        printDataDetail(data);

        //decrypt data to get payload
        QByteArray payload=decryptData(data, "signIn.pgp");

        qDebug() << "\n-DECRYPTED PAYLOAD-";
        int usernameLength=QString(payload).mid(0,1).data()->unicode();

        QString username(payload.mid(5, usernameLength));

        int userPublicKeyLength;
        userPublicKeyLength = QString(payload).mid(1,4).toUInt(&ok, 16);

        qDebug() << "Username length:"<< usernameLength;
        qDebug() << "Username:" << username;
        qDebug() << "User's public key Length:" << userPublicKeyLength;
        qDebug() << "User's public key:"<< "[remove next line's //(comment) to see user's public key]";
        //qDebug() << "User's public key:"<< payload.mid(5+usernameLength);

        if(usernameList->indexOf(username)!=(-1)){

            qDebug() << "USER INDEX: " << usernameList->indexOf(username);
            gpgme_key_t userKey=getKey(userKeyList->at(usernameList->indexOf(username)));

            printKeys(userKey);

            int rNum = qrand();
            QByteArray qb;
            qb.setNum(rNum);


            qDebug() << "int rNum:" << rNum;
            qDebug() << "QBy rNum:" << qb;

            if(loginUser->indexOf(username)==(-1)){

                loginUser->append(username);
                loginRanNum->append(QString(qb));
            }
            else{
                int idx=loginUser->indexOf(username);
                loginRanNum->replace(idx, QString(qb));
            }

            qDebug() << "loginUser:" << username;
            qDebug() << "loginRanNum:" << loginRanNum->at(
                            loginUser->indexOf(username));


            QByteArray cipher=encryptToClient(qb, username, "rannum.cipher");

            data.clear();

            data.append(cipher);

            // Insert operation in front of byte array (data[0]).
            data.insert(0, (char)6);

            //Insert size of(operation + payload) in front of byte array (data[0]).
            int dataSize=data.size();
            QByteArray dataSizeByte;
            QDataStream ds(&dataSizeByte, QIODevice::WriteOnly);
            ds << dataSize;
            data.insert(0, dataSizeByte);

            send(data);


        }
        else{
            qDebug() << "USER NOT FOUND IN THIS SERVER!!!!";

            QByteArray qb="USER NOT FOUND IN THIS SERVER!";

            qDebug() << "qb:" << qb; // ////////////////////////////////

            //QByteArray cipher=encryptToClient(qb, username, "rannum.cipher");
            data.clear();
            data.append(qb);

            //qDebug() << "cipher:" << cipher; // ////////////////////////////////

            // Insert operation in front of byte array (data[0]).
            data.insert(0, (char)6);

            qDebug() << "data:" << data; // ////////////////////////////////

            //Insert size of(operation + payload) in front of byte array (data[0]).
            int dataSize=data.size();
            QByteArray dataSizeByte;
            QDataStream ds(&dataSizeByte, QIODevice::WriteOnly);
            ds << dataSize;
            data.insert(0, dataSizeByte);

            qDebug() << "data:" << data; // ////////////////////////////////

            send(data);


        }

    }
    if(intOp==7){
        qDebug() << "*7*7*7*7*7*7*7*7*7*7*7*7*7*7*7*7*7*7*7*7*7*7*7*7*7*7*7*7*7*7*7*7*7*7";
        qDebug() << "*7-> RECEIVED Sign in verification";
        printf("+----------------------+----------------------+--------------------+\n");
        printf("|  data Size (4 byte)  | Operation (1 byte)   | Encrypted(payload) |\n");
        printf("+----------------------+----------------------+--------------------+\n");

        printDataDetail(data);
        QByteArray payload=decryptData(data, "signInVerify.pgp");

        qDebug() << payload;

        QString dataQS=QString(payload);

        QString verifyNum=dataQS.split("@@").first();
        QString keyID=dataQS.split("@@").last();

        qDebug() << "verifyNum:" << verifyNum;
        qDebug() << "keyID:" << keyID;

        gpgme_key_t userKey=getKey(keyID);

        QString username=(userKey->uids->name);
        qDebug() << "username:" << username;

        int userIndex=loginUser->indexOf(username);

        QByteArray qb;

        if(loginRanNum->at(userIndex)==verifyNum){

            qb = "verify success!!!";
            activeUser=username;
        }
        else{
            qb = "Server couldn't verify this account's private key belongs to you!";
        }

        qDebug() << qb;

        QByteArray cipher=encryptToClient(qb, username, "signInResult.cipher");

        data.clear();

        data.append(cipher);

        // Insert operation in front of byte array (data[0]).
        data.insert(0, (char)8);

        //Insert size of(operation + payload) in front of byte array (data[0]).
        int dataSize=data.size();
        QByteArray dataSizeByte;
        QDataStream ds(&dataSizeByte, QIODevice::WriteOnly);
        ds << dataSize;
        data.insert(0, dataSizeByte);

        send(data);

        if(qb=="verify success!!!"){
            task();
        }

    }
    if(intOp==9){
        qDebug() << "*9*9*9*9*9*9*9*9*9*9*9*9*9*9*9*9*9*9*9*9*9*9*9*9*9*9*9*9*9*9*9*9*9*9";
        qDebug() << "*9-> RECEIVED Search username request";
        printf("+----------------------+----------------------+--------------------+\n");
        printf("|  data Size (4 byte)  | Operation (1 byte)   | Encrypted(payload) |\n");
        printf("+----------------------+----------------------+--------------------+\n");

        printDataDetail(data);
        QByteArray payload=decryptData(data, "username.keyword");

        QString dataQS=QString(payload);

        QString keyword=dataQS.split("@@").first();
        QString sender=dataQS.split("@@").last();

        qDebug() << "keyword:" << keyword;
        qDebug() << "sender:" << sender;

        if(usernameList->indexOf(keyword)==(-1)){
            payload.clear();
            payload.append("0");
        }
        else{
            int idx=usernameList->indexOf(keyword);
            QString userKey=userKeyList->at(idx);

            payload.clear();
            payload.append("1\n");
            payload.append(keyword+"\n");
            payload.append(userKey);
        }

        QByteArray cipher=encryptToClient(payload, sender, "searchUser.cipher");

        data.clear();
        data.append(cipher);

        // Insert operation in front of byte array (data[0]).
        data.insert(0, (char)10);

        //Insert size of(operation + payload) in front of byte array (data[0]).
        int dataSize=data.size();
        QByteArray dataSizeByte;
        QDataStream ds(&dataSizeByte, QIODevice::WriteOnly);
        ds << dataSize;
        data.insert(0, dataSizeByte);

        send(data);

    }
    if(intOp==11){
        qDebug() << "*11*11*11*11*11*11*11*11*11*11*11*11*11*11*11*11*11*11*11*11*11*11*11";
        qDebug() << "*11-> RECEIVED add friend request";
        printf("+----------------------+----------------------+--------------------+\n");
        printf("|  data Size (4 byte)  | Operation (1 byte)   | Encrypted(payload) |\n");
        printf("+----------------------+----------------------+--------------------+\n");

        printDataDetail(data);
        QByteArray payload=decryptData(data, "username.keyword");

        QString dataQS=QString(payload);

        QString keyword=dataQS.split("@@").first();
        QString sender=dataQS.split("@@").last();

        qDebug() << "keyword:" << keyword;
        qDebug() << "sender:" << sender;

        if(usernameList->indexOf(keyword)==(-1)){
            payload.clear();
            payload.append("0");
        }
        else{

            payload.clear();
            payload.append("1");

            QString taskUser=keyword;
            QString taskWork="13"+sender;

            waitingTaskUser->append(taskUser);
            waitingTaskWork->append(taskWork);

            gpgme_key_t senderkey = getKey(sender);

            sender=QString(senderkey->uids->name);

            addFriendRequestList->append(sender+"@@"+keyword);

            qDebug() << sender+"@@"+keyword;
        }

        QByteArray cipher=encryptToClient(payload, sender, "searchUser.cipher");

        data.clear();
        data.append(cipher);

        // Insert operation in front of byte array (data[0]).
        data.insert(0, (char)12);

        //Insert size of(operation + payload) in front of byte array (data[0]).
        int dataSize=data.size();
        QByteArray dataSizeByte;
        QDataStream ds(&dataSizeByte, QIODevice::WriteOnly);
        ds << dataSize;
        data.insert(0, dataSizeByte);

        send(data);
    }
    if(intOp==14){
        qDebug() << "*14*14*14*14*14*14*14*14*14*14*14*14*14*14*14*14*14*14*14*14*14*14*14";
        qDebug() << "*14-> RECEIVED add friend confirmation result";
        printf("+----------------------+----------------------+--------------------+\n");
        printf("|  data Size (4 byte)  | Operation (1 byte)   | Encrypted(payload) |\n");
        printf("+----------------------+----------------------+--------------------+\n");

        printDataDetail(data);
        QByteArray payload=decryptData(data, "username.keyword");

        QString dataQS=QString(payload);

        QString keyword=dataQS.split("@@").first();
        QString sender=dataQS.split("@@").last();

        qDebug() << "keyword:" << keyword;
        qDebug() << "sender:" << sender;

        gpgme_key_t username_sendRequest = getKey(keyword);
        gpgme_key_t username_sendConfirm = getKey(sender);

        sender=QString(username_sendConfirm->uids->name);

        QString addFriendRequestID=keyword+"@@"+sender;

        if(addFriendRequestList->indexOf(addFriendRequestID)!=-1){

            int idx=addFriendRequestID.indexOf(addFriendRequestID);

            qDebug() << "username_sendRequest_key";
            printKeys(username_sendRequest);
            qDebug() << "username_sendConfirm_key";
            printKeys(username_sendConfirm);

           exportKey(ctx, username_sendRequest, err, "temp.key");

           QFile outFile("temp.key");
           if(!outFile.open(QFile::ReadOnly | QFile::Text)){
               qDebug() << "cound not open file for writing";
               abort();
           }
           QTextStream in(&outFile);
           QString dataStream;
           dataStream=in.readAll();
           outFile.flush();
           outFile.close();

           QByteArray userReqKey;
           userReqKey.append(dataStream);

           QString usernameOfRequest=QString(username_sendRequest->uids->name);
           userReqKey.insert(0, usernameOfRequest);

           char usernameSize=char(usernameOfRequest.size());
           userReqKey.insert(0, usernameSize);

           encryptToClient(userReqKey, sender, "key.cipher");
           QFile outFile3("key.cipher");
           if(!outFile3.open(QFile::ReadOnly | QFile::Text)){
               qDebug() << "cound not open file for writing";
               abort();
           }
           QTextStream in3(&outFile3);
           QString dataStream3;
           dataStream3=in3.readAll();
           outFile3.flush();
           outFile3.close();

           QByteArray qb;
           qb.append(dataStream3);

           waitingTaskUser->append(sender);
           waitingTaskWork->append("15"+qb);

           exportKey(ctx, username_sendConfirm, err, "temp.key");

           QFile outFile2("temp.key");
           if(!outFile2.open(QFile::ReadOnly | QFile::Text)){
               qDebug() << "cound not open file for writing";
               abort();
           }
           QTextStream in2(&outFile2);
           QString dataStream2;
           dataStream2=in2.readAll();
           outFile2.flush();
           outFile2.close();

           QByteArray userConKey;
           userConKey.append(dataStream2);

           QString usernameOfConfirm=QString(username_sendConfirm->uids->name);
           userConKey.insert(0, usernameOfConfirm);

           usernameSize=char(usernameOfConfirm.size());
           userConKey.insert(0, usernameSize);

           encryptToClient(userConKey, keyword, "key.cipher");
           QFile outFile4("key.cipher");
           if(!outFile4.open(QFile::ReadOnly | QFile::Text)){
               qDebug() << "cound not open file for writing";
               abort();
           }
           QTextStream in4(&outFile4);
           QString dataStream4;
           dataStream4=in4.readAll();
           outFile4.flush();
           outFile4.close();

           QByteArray qb2;
           qb2.append(dataStream4);

           waitingTaskUser->append(keyword);
           waitingTaskWork->append("15"+qb2);

           addFriendRequestList->removeAt(idx);

           task();

        }
    }

}


void MyThread::printDataDetail(QByteArray data){

    QDataStream ds(data.mid(0,4));
    int dataSize;
    ds >> dataSize;

    qDebug() << "       Byte Array:" << "[remove next line's //(comment) to see byte array value]";
    //qDebug() << "     Byte Array:" << data;
    qDebug() << "  Total data size:" << data.size();
    qDebug() << "data size in byte:"<< data.mid(0,4);
    qDebug() << " data size in int:"<< dataSize;
    qDebug() << "operation in byte:"<< data.mid(4,1);
    qDebug() << " operation in int:"<< QString(data.mid(4,1)).data()->unicode();
    qDebug() << "     payload size:"<< data.mid(5).size();
    qDebug() << "    payload value:"<< "[Remove next line's //(comment) to see payload value]";
    //qDebug() << "    payload value:"<< dataQString.mid(5);

}

QByteArray MyThread::decryptData(QByteArray data, const char* outputFileName){

    QByteArray decrypted;
    QByteArray cipher=data.mid(5);
    int intOp=QString(data.mid(4,1)).data()->unicode();

    QFile File("temp.cipher");
    if(!File.open(QFile::WriteOnly | QFile::Text)){
        qDebug() << "cound not open file for writing";
        abort();
    }
    QTextStream out(&File);
    out << cipher;
    File.flush();
    File.close();


    QString verifyResult;

    qDebug() << "decryptV";

    qDebug()<< "\n\n\n\n intOp:" << intOp;

    if(intOp==3){
        decrypt(ctx, err, "temp.cipher", outputFileName);
    }
    else{
        verifyResult=decryptVerify(ctx, err, "temp.cipher", outputFileName);
    }

    qDebug() << "decryptVx";


    QFile outFile(outputFileName);

    if(!outFile.open(QFile::ReadOnly | QFile::Text)){
        qDebug() << "cound not open file for writing";
        abort();
    }
    QTextStream in(&outFile);
    QString dataStream;
    dataStream=in.readAll();
    outFile.flush();
    outFile.close();

    decrypted.append(dataStream);

    if(intOp==7 || intOp==9 || intOp==11 || intOp==14){
        decrypted.append("@@");
        decrypted.append(verifyResult.mid(1));

        return decrypted;
    }
    else{
        return decrypted;
    }


}


void MyThread::addNewUser(QByteArray payload){

    int usernameLength=QString(payload).mid(0,1).data()->unicode();
    QString username=QString(payload.mid(5, usernameLength));

    // Import user's public key
    QByteArray publicKey=payload.mid(5+usernameLength);

    QFile File("temp.data");
    if(!File.open(QFile::WriteOnly | QFile::Text)){
        qDebug() << "cound not open file for writing";
        abort();
    }
    QTextStream out(&File);
    out << publicKey;
    File.flush();
    File.close();

    gpgme_import_result_t importKeyResult = importKey(ctx, err, "temp.data");
    qDebug() <<"Import key, condered:"<< importKeyResult->considered;
    qDebug() <<"Import key, imported:"<< importKeyResult->imported;
    qDebug() <<"Import key, unchaged:"<< importKeyResult->unchanged;

    //Get user's keyID
    gpgme_key_t userKey=getKey(username);
    QString userKeyID(userKey->subkeys->keyid);

    usernameList->append(username);
    userKeyList->append(userKeyID);
}

gpgme_key_t MyThread::getKey(QString pattern){

    gpgme_key_t targetKey;
    QByteArray ba = pattern.toLatin1();
    const char *patt=ba.data();
    err = gpgme_op_keylist_start(ctx, patt, 0);
    detectError(err);
    int nKeysFound=0;
    while (!(err = gpgme_op_keylist_next(ctx, &key))) { // loop through the keys in the keyring
        targetKey=key;
        nKeysFound++;
    }

    if(nKeysFound==0){
        qDebug() << "nKeyFound:" << nKeysFound;
        abort();
    }
    return targetKey;
}

QByteArray MyThread::encryptToClient(QByteArray data, QString recipient, const char *outFileName){

    gpgme_key_t recipientKey=getKey(recipient);
    QByteArray encrypted;
    QFile File("temp.data");
    if(!File.open(QFile::WriteOnly | QFile::Text)){
        qDebug() << "cound not open file for writing";
        abort();
    }
    QTextStream out(&File);
    out << data;
    File.flush();
    File.close();

    qDebug() << "Signer's key...";
    printKeys(ServerKey);
    qDebug() << "Recipient's key...";
    printKeys(recipientKey);

    gpgme_signers_add(ctx, ServerKey);
    encryptSign(ctx, err, recipientKey, "temp.data", outFileName);
    gpgme_key_unref (recipientKey);

    QFile outFile(outFileName);

    if(!outFile.open(QFile::ReadOnly | QFile::Text)){
        qDebug() << "cound not open file for writing";
        abort();
    }
    QTextStream in(&outFile);
    QString ciper;
    ciper=in.readAll();
    outFile.flush();
    outFile.close();

    encrypted.append(ciper);

    return encrypted;

}


void MyThread::send(QByteArray data){

    socket->write(data);
    socket->flush();
    socket->waitForBytesWritten(1000);

    int intOp=QString(data.mid(4,1)).data()->unicode();

    if(intOp==2){
        qDebug() << "*2*2*2*2*2*2*2*2*2*2*2*2*2*2*2*2*2*2*2*2*2*2*2*2*2*2*2*2*2*2*2*2*2*2";
        qDebug() << "<-*2 SEND Server's public key";
        printf("+--------------------+-------------+----------------------------------+\n");
        printf("| data size (4 byte) | op (1 byte) |    payload (server public key)   |\n");
        printf("+--------------------+-------------+----------------------------------+\n");

        printDataDetail(data);
        qDebug() << "\n\n\n\n\n";
    }
    else if(intOp==4){
        qDebug() << "*4*4*4*4*4*4*4*4*4*4*4*4*4*4*4*4*4*4*4*4*4*4*4*4*4*4*4*4*4*4*4*4*4*4";
        qDebug() << "<-*4 SEND Sign up result";
        printf("+--------------------+-------------+----------------------------------+\n");
        printf("| data size (4 byte) | op (1 byte) |    encrypted(result message)     |\n");
        printf("+--------------------+-------------+----------------------------------+\n");

        printDataDetail(data);
        qDebug() << "\n\n\n\n\n";
    }




}

void MyThread::readyRead(){
    qDebug() << "\nNew data arrived!";

    QByteArray data = socket->readAll();

    unsigned int dataSize;
    QDataStream ds(data.mid(0,4));
    ds >> dataSize;

    unsigned int sizeOfPayloadAndOp=data.mid(4).size();

    if(sizeOfPayloadAndOp==dataSize){
       qDebug() <<"(data SIZE == PAYLOAD + OP) -> NO LOSS!";
       dataFilter(data);
    }


}

void MyThread::disconnected(){

    qDebug() << socketDescriptor << " Disconnected: ";

    socket->deleteLater();
    exit(0);
}
