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

#include "signin.h"
#include "ui_signin.h"

SignIn::SignIn(Connection &conn, Encryption &encryption, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SignIn)
{
    ui->setupUi(this);

    this->conn=&conn;
    this->encryption=&encryption;

    ui->tabWidget_signIn->setTabEnabled(1, false);

    ui->tabWidget_signUp->setTabEnabled(1, false);
    ui->tabWidget_signUp->setTabEnabled(2, false);

    ui->frame_signIn_serverForm->hide();;
    ui->frame_SignUpServerEnterNew->hide();

    ui->pushButton_signIn_serverConnect->setEnabled(false);
    ui->pushButton_SignUpServerConnect->setEnabled(false);

    ui->label_signIn_serverErr->hide();
    ui->label_signUpConnectError->hide();

    int connectionStatus=conn.getConnectionStatus();

    qDebug() << "Connection status:" << connectionStatus;

    if(connectionStatus==0){
        ui->tabWidget_mainTab->setCurrentIndex(1);
        on_tabWidget_mainTab_currentChanged(1);
    }
    else if(connectionStatus==1){
        ui->tabWidget_mainTab->setCurrentIndex(0);
        on_tabWidget_mainTab_currentChanged(0);
    }

}

SignIn::~SignIn()
{
    delete ui;
}

/*
void SignIn::on_pushButton_clicked()
{
    QString activeUser=selectedAccount;
    //qDebug() << activeUser;
    if(activeUser!=""){
        conn->send("abcdefgh");
        SignIn::accept();
    }
    else{
        ui->label_signIn_keyFpr->setText("\nUsername can not be empty!");
    }
}
*/
void SignIn::on_pushButton_signIn_AccountSignIn_clicked()
{
    selectedAccount=ui->comboBox_signIn_SelectAccount->currentText();
    if(selectedAccount!=""){

        int keyIndex=accountNameList.indexOf(selectedAccount);
        QString accountKey;


        for(int i=0; i<accountKeyList.length(); i++){
            if(i==keyIndex){
                 accountKey=accountKeyList.at(i);
            }
        }
        qDebug() << "selectedAccount: "<< selectedAccount;
        qDebug() << "accountKey: "<< accountKey;

        QByteArray ba=accountKey.toLatin1();
        const char *patt=ba.data();

        gpgme_key_t privateKey = encryption->getKey(patt, 1);
        encryption->setUserPriKey(privateKey);

        gpgme_key_t publicKey = encryption->getKey(patt, 0);
        encryption->setUserPriKey(publicKey);

        encryption->printKeys(publicKey);

        // Export selected account public key
        encryption->exportKey(publicKey, "userPublicKey.key");

        QFile File("userPublicKey.key");
        if(!File.open(QFile::ReadOnly | QFile::Text)){
            qDebug() << "Cound not open file for Read";
            abort();
        }
        QTextStream in(&File);
        QString pubKey;
        pubKey=in.readAll();
        File.close();

        // Create 5*-> send sign in require.
        QByteArray data;
        data.clear();

        // Create payload

          //Add user public key to byte array.
        data.append(pubKey);

          //Insert unsername in front of byte array (data[0]).
        data.insert(0, selectedAccount);


          //Insert user's public key size in front of byte array (data[0]).
        int publicKeySize=pubKey.size();
        QByteArray publicKeySizeHex;
        publicKeySizeHex.setNum(publicKeySize, 16);

        while(publicKeySizeHex.length() < 4){
            publicKeySizeHex.insert(0,"0");
        }

        data.insert(0, publicKeySizeHex);

          //Insert user username size in front of byte array (data[0]).
        char usernameSize=char(selectedAccount.size());
        data.insert(0, usernameSize);

        //Encrypt Payload
        QFile File_Payload("signIn.payload");
        if(!File_Payload.open(QFile::WriteOnly | QFile::Text)){
            qDebug() << "Cound not open file for writing";
            abort();
        }
        QTextStream out(&File_Payload);
        out << data;

        File_Payload.flush();
        File_Payload.close();

        gpgme_key_t servPubKey=encryption->getServerPubKey();

        encryption->encryptSign(privateKey, servPubKey, "signIn.payload", "signIn.epkg");

        QFile File_EncryptedPayload("signIn.epkg");
        if(!File_EncryptedPayload.open(QFile::ReadOnly | QFile::Text)){
            qDebug() << "Cound not open file for Read";
            abort();
        }
        QTextStream in2(&File_EncryptedPayload);
        QString payload;
        payload=in2.readAll();
        File_EncryptedPayload.close();

        data.clear();
        data.append(payload);

        // Insert operation in front of byte array (data[0]).
        data.insert(0, (char)5);

        //Insert size of(operation + payload) in front of byte array (data[0]).
        int dataSize=data.size();
        QByteArray dataSizeByte;
        QDataStream ds2(&dataSizeByte, QIODevice::WriteOnly);
        ds2 << dataSize;
        data.insert(0, dataSizeByte);

        conn->send(data);

        data.clear();
        data=conn->getRecentReceivedMsg();

        //qDebug() << data;

        if(data.mid(5)=="USER NOT FOUND IN THIS SERVER!"){
            ui->label_signIn_keyFpr->setStyleSheet("color:#FF6666");
            ui->label_signIn_keyFpr->setText("Error: This account not found in this server!");

        }
        else{
            QFile File_Result("signInRan.cipher");
            if(!File_Result.open(QFile::WriteOnly | QFile::Text)){
                qDebug() << "Cound not open file for writing";
                abort();
            }
            QTextStream ts(&File_Result);
            ts << data.mid(5);

            File_Result.flush();
            File_Result.close();


            bool isValid=encryption->decryptVerify("signInRan.cipher", "signInRan.txt");

            if(isValid==false){
                ui->label_signIn_keyFpr->setStyleSheet("color:#FF6666");
                ui->label_signIn_keyFpr ->setText("ERROR: Server signature not fully valid");
            }
            else{
                QFile File_result("signInRan.txt");
                if(!File_result.open(QFile::ReadOnly | QFile::Text)){
                    qDebug() << "Cound not open file for Read";
                    abort();
                }
                QTextStream in2(&File_result);
                QString signInRan;
                signInRan=in2.readAll();
                File_result.close();

                qDebug() << signInRan;

                encryption->encryptSign(privateKey, servPubKey, "signInRan.txt", "signIn.epkg");

                QFile File_EncryptedPayload("signIn.epkg");
                if(!File_EncryptedPayload.open(QFile::ReadOnly | QFile::Text)){
                    qDebug() << "Cound not open file for Read";
                    abort();
                }
                QTextStream in(&File_EncryptedPayload);
                QString payload;
                payload=in.readAll();
                File_EncryptedPayload.close();

                data.clear();
                data.append(payload);

                // Insert operation in front of byte array (data[0]).
                data.insert(0, (char)7);

                //Insert size of(operation + payload) in front of byte array (data[0]).
                int dataSize=data.size();
                QByteArray dataSizeByte;
                QDataStream ds2(&dataSizeByte, QIODevice::WriteOnly);
                ds2 << dataSize;
                data.insert(0, dataSizeByte);

                conn->send(data); // //////////////////////////

                data.clear();
                data=conn->getRecentReceivedMsg();

                //Decrypt Payload
                QFile File_Result("signInResult.cipher");
                if(!File_Result.open(QFile::WriteOnly | QFile::Text)){
                    qDebug() << "Cound not open file for writing";
                    abort();
                }
                QTextStream ts(&File_Result);
                ts << data.mid(5);

                File_Result.flush();
                File_Result.close();

                bool isValid=encryption->decryptVerify("signInResult.cipher", "signInResult.txt");

                if(isValid==false){
                    ui->label_signIn_keyFpr->setStyleSheet("color:#FF6666");
                    ui->label_signIn_keyFpr ->setText("ERROR: Server signature not fully valid");
                }
                else{
                    QFile File_result("signInResult.txt");
                    if(!File_result.open(QFile::ReadOnly | QFile::Text)){
                        qDebug() << "Cound not open file for Read";
                        abort();
                    }
                    QTextStream in(&File_result);
                    QString signInResult;
                    signInResult=in.readAll();
                    File_result.close();

                    qDebug() << signInResult;

                    if(signInResult=="verify success!!!"){

                        selectedAccount=ui->comboBox_signIn_SelectAccount->currentText();
                        QString accountKey;


                        int keyIndex=accountNameList.indexOf(selectedAccount);


                        for(int i=0; i<accountKeyList.length(); i++){
                            if(i==keyIndex){
                                 accountKey=accountKeyList.at(i);
                            }
                        }

                        ui->label_signIn_keyFpr->setText(accountKey);
                        ui->label_signIn_keyFpr->setStyleSheet("color:#999999");

                        QByteArray ba=accountKey.toLatin1();
                        const char *patt=ba.data();

                        gpgme_key_t privateKey = encryption->getKey(patt, 1);
                        encryption->setUserPriKey(privateKey);

                        gpgme_key_t publicKey = encryption->getKey(patt, 0);
                        encryption->setUserPubKey(publicKey);

                        SignIn::accept();
                    }
                    else{
                        ui->label_signIn_keyFpr->setText(signInResult);
                    }
                }

            }
        }






        //SignIn::accept();
    }
    else{
        ui->label_signIn_keyFpr->setText("\nUsername can not be empty!");
    }
}

QString SignIn::getActiveUser(){
    QString ACTIVE_USER=selectedAccount;
    return ACTIVE_USER;
}


void SignIn::on_lineEdit_SignUpServerIP_textChanged(const QString &arg1)
{
    QString port=ui->lineEdit_SignUpServerPort->text();
    if(arg1!="" && port!=""){
        ui->pushButton_SignUpServerConnect->setEnabled(true);
    }
    else{
        ui->pushButton_SignUpServerConnect->setEnabled(false);
    }
}

void SignIn::on_lineEdit_SignUpServerPort_textChanged(const QString &arg1)
{
    QString ip=ui->lineEdit_SignUpServerIP->text();
    if(ip!="" && arg1!=""){
        ui->pushButton_SignUpServerConnect->setEnabled(true);
    }
    else{
        ui->pushButton_SignUpServerConnect->setEnabled(false);
    }
}

void SignIn::on_pushButton_SignUpServerConnect_clicked()
{
    if(conn->getConnectionStatus()!=1){
        ui->label_signUpConnectError->setText("");
        ui->label_signUpConnectError->hide();

        QString newIP=ui->lineEdit_SignUpServerIP->text();
        QString newPort=ui->lineEdit_SignUpServerPort->text();
        QString selectedServer=ui->comboBox_signUp_selectServer->currentText();

        if(selectedServer!="Select server..." && selectedServer!="*New Server"){
            qDebug() << selectedServer;
        }
        else{
            if(selectedServer=="*New Server"){
                qDebug()<<newIP<<", Port "<<newPort;
                ui->label_signUpConnectError->setText("Waiting for connection");
                ui->label_signUpConnectError->setStyleSheet("color:#333333");
                ui->label_signUpConnectError->show();

                QString loading[3]={"Waiting for connection.",
                                    "Waiting for connection..",
                                    "Waiting for connection..."};

                for(int i=1; i<3; i++){
                    ui->label_signUpConnectError->setText(loading[i]);
                }

                conn->connect(newIP, newPort);

                if(conn->getConnectionStatus()==-1){
                    ui->label_signUpConnectError->show();
                    ui->label_signUpConnectError->setStyleSheet("color:#FF6666");
                    ui->label_signUpConnectError->setText("ERROR: Can not connect to server!");
                }
                if(conn->getConnectionStatus()==1){

                    QString servAddr=conn->getServerAddr();
                    QString servPort=conn->getServerPort();

                    qDebug() << conn->getConnectionStatus();
                    ui->tabWidget_signIn->setTabEnabled(1, true);
                    ui->tabWidget_signIn->setCurrentIndex(1);
                    ui->label_signIn_serverErr->show();
                    ui->label_signIn_serverErr->setText("Server:" +servAddr+":"+servPort+" connected");
                    ui->label_signIn_serverErr->setStyleSheet("QLabel { qproperty-alignment: AlignCenter; color:#66AA66;}");


                    ui->tabWidget_signUp->setTabEnabled(1, true);
                    ui->tabWidget_signUp->setCurrentIndex(1);
                    ui->label_signUpConnectError->show();
                    ui->label_signUpConnectError->setText("Server:" +servAddr+":"+servPort+" connected");
                    ui->label_signUpConnectError->setStyleSheet("QLabel { qproperty-alignment: AlignCenter; color:#66AA66;}");

                    ui->frame_signIn_serverForm->hide();
                    ui->frame_SignUpServerEnterNew->hide();

                    ui->pushButton_signIn_serverConnect->setText("Disconnect");
                    ui->pushButton_SignUpServerConnect->setText("Disconnect");

                    ui->pushButton_signIn_serverConnect->setEnabled(true);
                    ui->pushButton_SignUpServerConnect->setEnabled(true);

                    ui->comboBox_signIn_selectServer->setEnabled(false);
                    ui->comboBox_signUp_selectServer->setEnabled(false);

                    ui->tabWidget_signUp->setCurrentIndex(1);
                }
                if(conn->getConnectionStatus()==0 || conn->getConnectionStatus()==2){
                    ui->label_signUpConnectError->show();
                    ui->label_signUpConnectError->setStyleSheet("color:#FF6666");
                    ui->label_signUpConnectError->setText("This server or this port not for E2EEIM");
                }

            }
        }

        ui->label_signUpConnectError->hide();
        ui->label_signUpConnectError->show();
        //ui->label_signUpConnectError->setStyleSheet("color:#FF6666");

    }
    else{
        conn->letDisconnect();

        int connectStatus=conn->getConnectionStatus();
        if(connectStatus!=1){
            ui->comboBox_signIn_selectServer->setEnabled(true);
            ui->comboBox_signUp_selectServer->setEnabled(true);

            ui->label_signIn_serverErr->setText("");
            ui->label_signUpConnectError->setText("");

            ui->pushButton_signIn_serverConnect->setText("Connect");
            ui->pushButton_SignUpServerConnect->setText("Connect");

            ui->pushButton_signIn_serverConnect->setEnabled(false);
            ui->pushButton_SignUpServerConnect->setEnabled(false);

            ui->tabWidget_signIn->setTabEnabled(1,0);
            ui->tabWidget_signUp->setTabEnabled(1,0);
            ui->tabWidget_signUp->setTabEnabled(2,0);

            ui->tabWidget_signIn->setCurrentIndex(0);
            ui->tabWidget_signUp->setCurrentIndex(0);

            ui->comboBox_signIn_selectServer->setCurrentIndex(0);
            ui->comboBox_signUp_selectServer->setCurrentIndex(0);
        }
        qDebug() << "Connection status:" << connectStatus;

    }

}

void SignIn::on_tabWidget_signUp_currentChanged(int index)
{
    if(index==0){
    }
    else if(index==1){
        ui->tabWidget_signUp->setTabEnabled(1, true);
        ui->label_signUpAccountErrMsg->setStyleSheet("color:#ff6666");
        ui->label_signUpAccountErrMsg->setText("");
    }
}

void SignIn::on_pushButton_signUpAccountSignUp_clicked()
{
    QString errMsg;
    QString username=ui->lineEdit_signUpAccountUsername->text();
    QString passphrase=ui->lineEdit_signUpAccountPassphrase->text();
    QString confirmPassphrase=ui->lineEdit_signUpAccountConfirmPassphrase->text();

    if(username==""){
        errMsg+="\n-Username can not be empty!";
    }
    if(passphrase==""){
        errMsg+="\n-Passphrase can not be empty!";
    }
    if(passphrase!=confirmPassphrase){
        errMsg+="\n-Passphrase and confirm-passphrase does not match!";
    }
    if(errMsg!=""){
        ui->label_signUpAccountErrMsg->setText(errMsg);
    }
    else{
        ui->tabWidget_signUp->setCurrentIndex(2);
        ui->tabWidget_signUp->setTabEnabled(0, false);
        ui->tabWidget_signUp->setTabEnabled(1, false);
        ui->tabWidget_signUp->setTabEnabled(2, true);

        ui->label_signUpFinishg->setText("Generating Key Pair...");


        for(int i=10000000; i>0; i--){

        }

        // Generate a new key pair
        QByteArray data;
        data.append(username);

        QString keyParms="<GnupgKeyParms format=\"internal\">\n"
                         "Key-Type: RSA\n"
                         "Key-Length: 4096\n"
                         "Subkey-Type: RSA\n"
                         "Subkey-Length: 4096\n"
                         "Name-Real: "+username+"\n"
                         "Name-Comment: Generated by E2EEIM Chat, Passphrase:"+passphrase+"\n"
                         "Name-Email: client@e2eeim.chat\n"
                         "Expire-Date: 1d\n"
                         "Passphrase: "+passphrase+"\n"
                         "</GnupgKeyParms>\n";


        QByteArray byteParms = keyParms.toLatin1();
        const char *parms = byteParms.data();

        gpgme_genkey_result_t GenKeyresult;
        GenKeyresult = encryption->genKey(parms);

        QString accountKey=QString(GenKeyresult->fpr);
        QByteArray ba=accountKey.toLatin1();
        const char *patt=ba.data();

        newUsersPrivateKey = encryption->getKey(patt, 1);
        newUsersPublicKey = encryption->getKey(patt, 0);

        // Export new user's public key
        encryption->exportKey(newUsersPublicKey, "userPublicKey.key");

        QFile File("userPublicKey.key");
        if(!File.open(QFile::ReadOnly | QFile::Text)){
            qDebug() << "Cound not open file for Read";
            abort();
        }
        QTextStream in(&File);
        QString pubKey;
        pubKey=in.readAll();
        File.close();



        // Create *2-> send sign up require.
        data.clear();

        // Create payload

          //Add user public key to byte array.
        data.append(pubKey);

          //Insert unsername in front of byte array (data[0]).
        data.insert(0, username);

          //Insert user public key size in front of byte array (data[0]).
        /*
        int publicKeySize=pubKey.size();
        QByteArray publicKeySizeByte;
        QDataStream ds(&publicKeySizeByte, QIODevice::WriteOnly);
        ds << publicKeySize;
        data.insert(0, publicKeySizeByte);
        */

        int publicKeySize=pubKey.size();
        QByteArray publicKeySizeHex;
        publicKeySizeHex.setNum(publicKeySize, 16);

        while(publicKeySizeHex.length() < 4){
            publicKeySizeHex.insert(0,"0");
        }

        data.insert(0, publicKeySizeHex);

          //Insert user username size in front of byte array (data[0]).
        char usernameSize=char(username.size());
        data.insert(0, usernameSize);

        //Encrypt Payload
        QFile File_Payload("2.payload");
        if(!File_Payload.open(QFile::WriteOnly | QFile::Text)){
            qDebug() << "Cound not open file for writing";
            abort();
        }
        QTextStream out(&File_Payload);
        out << data;

        File_Payload.flush();
        File_Payload.close();

        gpgme_key_t servPubKey=encryption->getServerPubKey();

        encryption->encryptSign(newUsersPrivateKey, servPubKey, "2.payload", "2payload.encrypted");

        QFile File_EncryptedPayload("2payload.encrypted");
        if(!File_EncryptedPayload.open(QFile::ReadOnly | QFile::Text)){
            qDebug() << "Cound not open file for Read";
            abort();
        }
        QTextStream in2(&File_EncryptedPayload);
        QString payload;
        payload=in2.readAll();
        File_EncryptedPayload.close();

        data.clear();
        data.append(payload);

        // Insert operation in front of byte array (data[0]).
        data.insert(0, (char)3);

        //Insert size of(operation + payload) in front of byte array (data[0]).
        int dataSize=data.size();
        QByteArray dataSizeByte;
        QDataStream ds2(&dataSizeByte, QIODevice::WriteOnly);
        ds2 << dataSize;
        data.insert(0, dataSizeByte);

        ui->label_signUpFinishg->setText("Sending sign up request...");

        conn->send(data);

        // Get sign up result message
        data.clear();
        data=conn->getRecentReceivedMsg();

        //Decrypt Payload
        QFile File_Result("signUpResult.cipher");
        if(!File_Result.open(QFile::WriteOnly | QFile::Text)){
            qDebug() << "Cound not open file for writing";
            abort();
        }
        QTextStream ts(&File_Result);
        ts << data.mid(5);

        File_Result.flush();
        File_Result.close();


        bool isValid=encryption->decryptVerify("signUpResult.cipher", "signUpResult.txt");

        if(isValid==false){
            ui->label_signUpFinishg->setText("ERROR: Server signature not fully valid");
        }
        else{
            QFile File_result("signUpResult.txt");
            if(!File_result.open(QFile::ReadOnly | QFile::Text)){
                qDebug() << "Cound not open file for Read";
                abort();
            }
            QTextStream in2(&File_result);
            QString signUpReslut;
            signUpReslut=in2.readAll();
            File_result.close();

            ui->label_signUpFinishg->setText(signUpReslut);

            /*
            if(signUpReslut.mid(0,9)=="Username:"){

                encryption->deletePrivateKey(newUsersPrivateKey);

            }
            */

            qDebug() << signUpReslut;

            ui->tabWidget_signUp->setTabEnabled(0, true);
            ui->tabWidget_signUp->setTabEnabled(1, true);
            ui->tabWidget_signUp->setTabEnabled(2, true);

            ui->lineEdit_signUpAccountUsername->clear();
            ui->lineEdit_signUpAccountPassphrase->clear();
            ui->lineEdit_signUpAccountConfirmPassphrase->clear();

        }

    }
}

void SignIn::on_tabWidget_mainTab_currentChanged(int index)
{
    if(conn->getConnectionStatus()==1){

        QString servAddr=conn->getServerAddr();
        QString servPort=conn->getServerPort();

        qDebug() << conn->getConnectionStatus();
        ui->tabWidget_signIn->setTabEnabled(1, true);
        ui->tabWidget_signIn->setCurrentIndex(1);
        ui->label_signIn_serverErr->show();
        ui->label_signIn_serverErr->setText("Server:" +servAddr+":"+servPort+" connected");
        ui->label_signIn_serverErr->setStyleSheet("QLabel { qproperty-alignment: AlignCenter; color:#66AA66;}");


        ui->tabWidget_signUp->setTabEnabled(1, true);
        ui->tabWidget_signUp->setCurrentIndex(1);
        ui->label_signUpConnectError->show();
        ui->label_signUpConnectError->setText("Server:" +servAddr+":"+servPort+" connected");
        ui->label_signUpConnectError->setStyleSheet("QLabel { qproperty-alignment: AlignCenter; color:#66AA66;}");

        ui->frame_signIn_serverForm->hide();
        ui->frame_SignUpServerEnterNew->hide();

        ui->pushButton_signIn_serverConnect->setText("Disconnect");
        ui->pushButton_SignUpServerConnect->setText("Disconnect");

        ui->pushButton_signIn_serverConnect->setEnabled(true);
        ui->pushButton_SignUpServerConnect->setEnabled(true);

        ui->comboBox_signIn_selectServer->setEnabled(false);
        ui->comboBox_signUp_selectServer->setEnabled(false);
    }

    if(index == 0){
        if(ui->tabWidget_signIn->currentIndex()==0 ||
                ui->tabWidget_signIn->currentIndex()==1){
            QStringList allAccounts=encryption->getE2eeimAccounts();

            if(!allAccounts.isEmpty()){
                ui->comboBox_signIn_SelectAccount->clear();
                accountNameList.clear();
                accountKeyList.clear();

                int i=0;
                foreach (QString account, allAccounts) {
                    if(i%2==0){
                        accountNameList.append(account);
                        ui->comboBox_signIn_SelectAccount->addItem(account);
                    }
                    else{
                        accountKeyList.append(account);
                    }

                    i++;
                }

                selectedAccount=ui->comboBox_signIn_SelectAccount->currentText();
                on_comboBox_signIn_SelectAccount_currentIndexChanged(selectedAccount);
            }
        }
    }
}

void SignIn::on_comboBox_signIn_SelectAccount_currentIndexChanged(const QString &arg1)
{


    selectedAccount=arg1;
    selectedAccount=ui->comboBox_signIn_SelectAccount->currentText();
    QString accountKey;


    int keyIndex=accountNameList.indexOf(selectedAccount);


    for(int i=0; i<accountKeyList.length(); i++){
        if(i==keyIndex){
             accountKey=accountKeyList.at(i);
        }
    }

    ui->label_signIn_keyFpr->setText(accountKey);
    ui->label_signIn_keyFpr->setStyleSheet("color:#999999");

    QByteArray ba=accountKey.toLatin1();
    const char *patt=ba.data();

    gpgme_key_t privateKey = encryption->getKey(patt, 1);
    encryption->setUserPriKey(privateKey);

    gpgme_key_t publicKey = encryption->getKey(patt, 0);
    encryption->setUserPubKey(publicKey);

}

void SignIn::on_comboBox_signIn_selectServer_currentIndexChanged(const QString &arg1)
{
    if(arg1=="Select Server..."){
        ui->pushButton_signIn_serverConnect ->setEnabled(false);
        ui->frame_signIn_serverForm->hide();

    }
    if(arg1=="*New Server"){
        QString ip=ui->lineEdit_signIn_serverAddress->text();
        QString port=ui->lineEdit_signIn_serverPort->text();

        if(ip=="" || port==""){
            ui->pushButton_signIn_serverConnect->setEnabled(false);
        }
        else{
            ui->pushButton_signIn_serverConnect->setEnabled(true);
        }
    }
    if(arg1!="*New Server"){
        ui->frame_signIn_serverForm->hide();
    }
    else{
        ui->frame_signIn_serverForm->show();
    }

}

void SignIn::on_lineEdit_signIn_serverAddress_textChanged(const QString &arg1)
{
    QString port=ui->lineEdit_signIn_serverPort->text();
    if(arg1!="" && port!=""){
        ui->pushButton_signIn_serverConnect->setEnabled(true);
    }
    else{
        ui->pushButton_signIn_serverConnect->setEnabled(false);
    }
}

void SignIn::on_lineEdit_signIn_serverPort_textChanged(const QString &arg1)
{
    QString ip=ui->lineEdit_signIn_serverAddress->text();
    if(ip!="" && arg1!=""){
        ui->pushButton_signIn_serverConnect->setEnabled(true);
    }
    else{
        ui->pushButton_signIn_serverConnect->setEnabled(false);
    }
}

void SignIn::on_pushButton_signIn_serverConnect_clicked()
{
    if(conn->getConnectionStatus()!=1){
        ui->label_signIn_serverErr->setText("");
        ui->label_signIn_serverErr->hide();

        QString newIP=ui->lineEdit_signIn_serverAddress->text();
        QString newPort=ui->lineEdit_signIn_serverPort->text();
        QString selectedServer=ui->comboBox_signIn_selectServer->currentText();

        if(selectedServer!="Select Server..." && selectedServer!="*New Server"){
            qDebug() << selectedServer;
        }
        else{
            if(selectedServer=="*New Server"){
                qDebug()<<newIP<<", Port "<<newPort;
                ui->label_signIn_serverErr->setText("Waiting for connection");
                ui->label_signIn_serverErr->setStyleSheet("color:#333333");
                ui->label_signIn_serverErr->show();

                QString loading[3]={"Waiting for connection.",
                                    "Waiting for connection..",
                                    "Waiting for connection..."};

                for(int i=1; i<3; i++){
                    ui->label_signIn_serverErr->setText(loading[i]);
                }

                conn->connect(newIP, newPort);

                if(conn->getConnectionStatus()==-1){
                    ui->label_signIn_serverErr->show();
                    ui->label_signIn_serverErr->setStyleSheet("color:#FF6666");
                    ui->label_signIn_serverErr->setText("ERROR: Can not connect to server!");
                }
                if(conn->getConnectionStatus()==1){

                    QString servAddr=conn->getServerAddr();
                    QString servPort=conn->getServerPort();

                    qDebug() << conn->getConnectionStatus();
                    ui->tabWidget_signIn->setTabEnabled(1, true);
                    ui->tabWidget_signIn->setCurrentIndex(1);
                    ui->label_signIn_serverErr->show();
                    ui->label_signIn_serverErr->setText("Server:" +servAddr+":"+servPort+" connected");
                    ui->label_signIn_serverErr->setStyleSheet("QLabel { qproperty-alignment: AlignCenter; color:#66AA66;}");


                    ui->tabWidget_signUp->setTabEnabled(1, true);
                    ui->tabWidget_signUp->setCurrentIndex(1);
                    ui->label_signUpConnectError->show();
                    ui->label_signUpConnectError->setText("Server:" +servAddr+":"+servPort+" connected");
                    ui->label_signUpConnectError->setStyleSheet("QLabel { qproperty-alignment: AlignCenter; color:#66AA66;}");

                    ui->frame_signIn_serverForm->hide();
                    ui->frame_SignUpServerEnterNew->hide();

                    ui->pushButton_signIn_serverConnect->setText("Disconnect");
                    ui->pushButton_SignUpServerConnect->setText("Disconnect");

                    ui->pushButton_signIn_serverConnect->setEnabled(true);
                    ui->pushButton_SignUpServerConnect->setEnabled(true);

                    ui->comboBox_signIn_selectServer->setEnabled(false);
                    ui->comboBox_signUp_selectServer->setEnabled(false);

                }
                if(conn->getConnectionStatus()==0 || conn->getConnectionStatus()==2){
                    ui->label_signIn_serverErr->show();
                    ui->label_signIn_serverErr->setStyleSheet("color:#FF6666");
                    ui->label_signIn_serverErr->setText("This server or this port not for E2EEIM");
                }

            }
        }

        ui->label_signIn_serverErr->hide();
        ui->label_signIn_serverErr->show();
        ui->label_signUpConnectError->setStyleSheet("color:#FF6666");
    }
    else{ // User click disconnect
        conn->letDisconnect();

        int connectStatus=conn->getConnectionStatus();
        if(connectStatus!=1){
            ui->comboBox_signIn_selectServer->setEnabled(true);
            ui->comboBox_signUp_selectServer->setEnabled(true);

            ui->label_signIn_serverErr->setText("");
            ui->label_signUpConnectError->setText("");

            ui->pushButton_signIn_serverConnect->setText("Connect");
            ui->pushButton_SignUpServerConnect->setText("Connect");

            ui->pushButton_signIn_serverConnect->setEnabled(false);
            ui->pushButton_SignUpServerConnect->setEnabled(false);

            ui->tabWidget_signIn->setTabEnabled(1,0);
            ui->tabWidget_signUp->setTabEnabled(1,0);
            ui->tabWidget_signUp->setTabEnabled(2,0);

            ui->tabWidget_signIn->setCurrentIndex(0);
            ui->tabWidget_signUp->setCurrentIndex(0);

            ui->comboBox_signIn_selectServer->setCurrentIndex(0);
            ui->comboBox_signUp_selectServer->setCurrentIndex(0);
        }
        qDebug() << "Connection status:" << connectStatus;

    }


}



void SignIn::on_comboBox_signUp_selectServer_currentTextChanged(const QString &arg1)
{
    qDebug() << arg1;
    if(arg1=="Select Server..."){
        ui->pushButton_SignUpServerConnect->setEnabled(false);
        ui->frame_SignUpServerEnterNew->hide();

    }
    if(arg1=="*New Server"){
        QString ip=ui->lineEdit_SignUpServerIP->text();
        QString port=ui->lineEdit_SignUpServerPort->text();

        if(ip=="" || port==""){
            ui->pushButton_SignUpServerConnect->setEnabled(false);
        }
        else{
            ui->pushButton_SignUpServerConnect->setEnabled(true);
        }
    }
    if(arg1!="*New Server"){
        ui->frame_SignUpServerEnterNew->hide();
    }
    else{
        ui->frame_SignUpServerEnterNew->show();
    }
}
