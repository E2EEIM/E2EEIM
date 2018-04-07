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

    ui->tabWidget_signUp->setTabEnabled(1, false);
    ui->tabWidget_signUp->setTabEnabled(2, false);

    ui->frame_SignUpServerEnterNew->hide();
    ui->pushButton_SignUpServerConnect->setEnabled(false);

    ui->label_signUpConnectError->hide();

}

SignIn::~SignIn()
{
    delete ui;
}

void SignIn::on_pushButton_clicked()
{
    QString activeUser=ui->lineEdit->text();
    //qDebug() << activeUser;
    if(activeUser!=""){
        SignIn::accept();
    }
    else{
        ui->label->setText("\nUsername can not be empty!");
    }
}
QString SignIn::getActiveUser(){
    QString ACTIVE_USER { ui->lineEdit->text() };
    return ACTIVE_USER;
}

void SignIn::on_comboBox_selectServer_currentIndexChanged(const QString &arg1)
{
    if(arg1=="Select server..."){
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
    ui->label_signUpConnectError->setText("");
    ui->label_signUpConnectError->hide();

    QString newIP=ui->lineEdit_SignUpServerIP->text();
    QString newPort=ui->lineEdit_SignUpServerPort->text();
    QString selectedServer=ui->comboBox_selectServer->currentText();

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
                ui->label_signUpConnectError->show();
                ui->label_signUpConnectError->setStyleSheet("color:#66AA66");
                ui->label_signUpConnectError->setText("Connected!");

                ui->tabWidget_signUp->setCurrentIndex(1);
            }
            if(conn->getConnectionStatus()==2){
                ui->label_signUpConnectError->show();
                ui->label_signUpConnectError->setStyleSheet("color:#FF6666");
                ui->label_signUpConnectError->setText("This server or this port not for E2EEIM");
            }

            qDebug() << conn->getConnectionStatus();
        }
    }

    ui->label_signUpConnectError->hide();
    ui->label_signUpConnectError->show();
    //ui->label_signUpConnectError->setStyleSheet("color:#FF6666");
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

        newUsersPrivateKey = encryption->getKey(GenKeyresult->fpr, 1);
        newUsersPublicKey = encryption->getKey(GenKeyresult->fpr, 0);



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
        int publicKeySize=pubKey.size();
        QByteArray publicKeySizeByte;
        QDataStream ds(&publicKeySizeByte, QIODevice::WriteOnly);
        ds << publicKeySize;
        data.insert(0, publicKeySizeByte);

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

        encryption->encryptSign(newUsersPrivateKey, encryption->serverKey, "2.payload", "2payload.encrypted");

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

        //Encrypt Payload
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

            qDebug() << signUpReslut;

            ui->tabWidget_signUp->setTabEnabled(0, true);
            ui->tabWidget_signUp->setTabEnabled(1, false);
            ui->tabWidget_signUp->setTabEnabled(2, true);
        }

    }
}
