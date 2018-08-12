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

    //Connect disconnect from server event.
    //Creat a [signal/slot] mechanism, when client application disconnect from server.
    //to call disconnectFromServer() function.
    connect(this->conn, SIGNAL(disconnectFromServer()), this, SLOT(disconnectFromServer()), Qt::QueuedConnection);

    //Connect receiveSignInVerify event.
    //Creat a [signal/slot] mechanism, when active-user
    //receive sign in verifiation request from server
    //to call receiveSignInVerfiy() function.
    connect(this->conn, SIGNAL(receiveSignInVerify(QByteArray)), this, SLOT(receiveSignInVerify(QByteArray)), Qt::QueuedConnection);

    //Connect receiveSignInResult event.
    //Creat a [signal/slot] mechanism, when active-user
    //receive sign-in request from server
    //to call receiveSignInResult() function.
    connect(this->conn, SIGNAL(receiveSignInResult(QByteArray)), this, SLOT(receiveSignInResult(QByteArray)), Qt::QueuedConnection);

    //Connect receiveSignInResult event.
    //Creat a [signal/slot] mechanism, when active-user
    //receive sign-in request from server
    //to call receiveSignInResult() function.
    connect(this->conn, SIGNAL(receiveSignUpResult(QByteArray)), this, SLOT(receiveSignUpResult(QByteArray)), Qt::QueuedConnection);

    //Connect generateKeyPair event.
    //Creat a [signal/slot] mechanism, when user
    //filled sign-up form and click Sign Up.
    //to call generateKeyPair() function.
    connect(this, SIGNAL(generateKeyPairSig(QStringList)), this, SLOT(generateKeyPair(QStringList)), Qt::QueuedConnection);


    //Set available tab when signIn window start to show.
    ui->tabWidget_signIn->setTabEnabled(1, false);
    ui->tabWidget_signUp->setTabEnabled(1, false);
    ui->tabWidget_signUp->setTabEnabled(2, false);

    //Hide server and port line edit.
    ui->frame_signIn_serverForm->hide();;
    ui->frame_SignUpServerEnterNew->hide();

    //Disable "Connect" button when signIn window start to show.
    ui->pushButton_signIn_serverConnect->setEnabled(false);
    ui->pushButton_SignUpServerConnect->setEnabled(false);

    //Hide error message when sigIn window start to show.
    ui->label_signIn_serverErr->hide();
    ui->label_signUpConnectError->hide();

    //Get current connection status.
    int connectionStatus=conn.getConnectionStatus();

    qDebug() << "Connection status:" << connectionStatus;

    //Set enable tab for each connection status.
    if(connectionStatus==0){
        ui->tabWidget_mainTab->setCurrentIndex(1);
        on_tabWidget_mainTab_currentChanged(1);
    }
    else if(connectionStatus==1){
        ui->tabWidget_mainTab->setCurrentIndex(0);
        on_tabWidget_mainTab_currentChanged(0);
    }
    else if(connectionStatus==-2){
        ui->tabWidget_signIn->setTabEnabled(1, false);
        ui->tabWidget_signUp->setTabEnabled(1, false);
        ui->tabWidget_signUp->setTabEnabled(2, false);

        ui->tabWidget_signIn->setCurrentIndex(0);
        on_tabWidget_mainTab_currentChanged(0);


    }

}

SignIn::~SignIn()
{
    delete ui;
}

//User click Sign In button in (Sign In-account) tab
void SignIn::on_pushButton_signIn_AccountSignIn_clicked()
{
    //Get selected account.
    selectedAccount=ui->comboBox_signIn_SelectAccount->currentText();
    if(selectedAccount!=""){

        //get index of selected account in account list.
        int keyIndex=accountNameList.indexOf(selectedAccount);
        QString accountKey;


        //Get account key's fingerprint.
        for(int i=0; i<accountKeyList.length(); i++){
            if(i==keyIndex){
                 accountKey=accountKeyList.at(i);
            }
        }
        qDebug() << "selectedAccount: "<< selectedAccount;
        qDebug() << "accountKey: "<< accountKey;

        //Convert key fingerprint to const char* variable type
        //that required in get key process.
        QByteArray ba=accountKey.toLatin1();
        const char *patt=ba.data();

        //Get private key of the figerprint.
        gpgme_key_t privateKey = encryption->getKey(patt, 1);
        encryption->setUserPriKey(privateKey);

        //Get public key of the fingerprint.
        gpgme_key_t publicKey = encryption->getKey(patt, 0);
        encryption->setUserPubKey(publicKey);

        encryption->printKeys(publicKey);

        // Export selected account public key
        encryption->exportKey(publicKey, "userPublicKey.key");

        //Read exported public key to pubKey variable.
        QFile File("userPublicKey.key");
        if(!File.open(QFile::ReadOnly | QFile::Text)){
            qDebug() << "Cound not open file for Read";
            abort();
        }
        QTextStream in(&File);
        QString pubKey;
        pubKey=in.readAll();
        File.close();

        //Create payload to send sign in request to server.
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

          //Save payload to text file
          //the required in encryption process.
        QFile File_Payload("signIn.payload");
        if(!File_Payload.open(QFile::WriteOnly | QFile::Text)){
            qDebug() << "Cound not open file for writing";
            abort();
        }
        QTextStream out(&File_Payload);
        out << data;

        File_Payload.flush();
        File_Payload.close();

        //Get server public key.
        gpgme_key_t servPubKey=encryption->getServerPubKey();

        //Encrypt payload.
        encryption->encryptSign(privateKey, servPubKey, "signIn.payload", "signIn.epkg");

        //Read encrypted payload and store in payload variable.
        QFile File_EncryptedPayload("signIn.epkg");
        if(!File_EncryptedPayload.open(QFile::ReadOnly | QFile::Text)){
            qDebug() << "Cound not open file for Read";
            abort();
        }
        QTextStream in2(&File_EncryptedPayload);
        QString payload;
        payload=in2.readAll();
        File_EncryptedPayload.close();

        //Clear data package variable
        //and add enctyped payload to data package.
        data.clear();
        data.append(payload);

        // Insert sign in requst operation number in front of byte array (data[0]).
        data.insert(0, (char)5);

        //Insert size of(operation + payload) in front of byte array (data[0]).
        int dataSize=data.size();
        QByteArray dataSizeByte;
        QDataStream ds2(&dataSizeByte, QIODevice::WriteOnly);
        ds2 << dataSize;
        data.insert(0, dataSizeByte);

        //Send sign in request to srever.
        conn->send(data);

        //Clear data variable.
        data.clear();

        //Display sign in status
        ui->label_signIn_keyFpr->setStyleSheet("color:#333333");
        ui->label_signIn_keyFpr->setText("Status: Waiting for response from server...");

    }
    else{ // In caes user not select any account.
        ui->label_signIn_keyFpr->setText("\nUsername can not be empty!");
    }
}

void SignIn::receiveSignInVerify(QByteArray data){


    //In case sign in not success.
    if(data.mid(5)=="USER NOT FOUND IN THIS SERVER!"){
        ui->label_signIn_keyFpr->setStyleSheet("color:#AA6666");
        ui->label_signIn_keyFpr->setText("Error: This account not found in this server!");

    }
    else{ //In case sign in success.
        //Save cipher recieved from server to text file
        //that required in decryption process.
        QFile File_Result("signInRan.cipher");
        if(!File_Result.open(QFile::WriteOnly | QFile::Text)){
            qDebug() << "Cound not open file for writing";
            abort();
        }
        QTextStream ts(&File_Result);
        ts << data.mid(5);

        File_Result.flush();
        File_Result.close();


        //Decrypt recieved cipher.
        QString decryptResult=encryption->decryptVerify("signInRan.cipher", "signInRan.txt");

        if(decryptResult.mid(0,1)=="0"){ //In case bad signature from server.
            ui->label_signIn_keyFpr->setStyleSheet("color:#AA6666");
            ui->label_signIn_keyFpr ->setText("ERROR: Server signature not fully valid");
        }
        else{
            //Read decrypted message, random text stuff from server.
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

            //Encrypt random text stuff that get from server to send back to server.
            gpgme_key_t privateKey=encryption->getUserPriKey();
            gpgme_key_t servPubKey=encryption->getServerPubKey();
            encryption->encryptSign(privateKey, servPubKey, "signInRan.txt", "signIn.epkg");

            //Read cipher, encrypted message to server.
            QFile File_EncryptedPayload("signIn.epkg");
            if(!File_EncryptedPayload.open(QFile::ReadOnly | QFile::Text)){
                qDebug() << "Cound not open file for Read";
                abort();
            }
            QTextStream in(&File_EncryptedPayload);
            QString payload;
            payload=in.readAll();
            File_EncryptedPayload.close();

            //Clear data package variavle
            //and add cipher to data package.
            data.clear();
            data.append(payload);

            // Insert sign in verification operation number in front of byte array (data[0]).
            data.insert(0, (char)7);

            //Insert size of(operation + payload) in front of byte array (data[0]).
            int dataSize=data.size();
            QByteArray dataSizeByte;
            QDataStream ds2(&dataSizeByte, QIODevice::WriteOnly);
            ds2 << dataSize;
            data.insert(0, dataSizeByte);

            //Send data package to server.
            conn->send(data);

            //Clear data package variable.
            data.clear();

            //Display sign in status
            ui->label_signIn_keyFpr->setStyleSheet("color:#333333");
            ui->label_signIn_keyFpr->setText("Status: Verifying account...");

            //Set Sign-in flag to in form connection it's still under sign-in process.
            conn->signInFlag=true;

        }
    }

}

void SignIn::receiveSignInResult(QByteArray data){

    //Save recieved sign in result cipher to text file
    //that required in decryption process.
    QFile File_Result("signInResult.cipher");
    if(!File_Result.open(QFile::WriteOnly | QFile::Text)){
        qDebug() << "Cound not open file for writing";
        abort();
    }
    QTextStream ts(&File_Result);
    ts << data.mid(5);

    File_Result.flush();
    File_Result.close();

    //Decrypt sign in result cipher.
    QString decryptResult=encryption->decryptVerify("signInResult.cipher", "signInResult.txt");

    if(decryptResult.mid(0,1)=="0"){ //In case bad signature from server.
        ui->label_signIn_keyFpr->setStyleSheet("color:#AA6666");
        ui->label_signIn_keyFpr ->setText("ERROR: Server signature not fully valid");
    }
    else{
        //Read decrypted sign in result.
        QFile File_result("signInResult.txt");
        if(!File_result.open(QFile::ReadOnly | QFile::Text)){
            qDebug() << "Cound not open file for Read";
            abort();
        }
        QTextStream in(&File_result);
        QString signInResult;
        signInResult=in.readAll();
        File_result.close();


        if(signInResult=="verify success!!!"){ //In case sign in success.

            //Get selected account username.
            selectedAccount=ui->comboBox_signIn_SelectAccount->currentText();
            QString accountKey;

            //Get index of selected account in account list.
            int keyIndex=accountNameList.indexOf(selectedAccount);

            //Get the account key fingerprint.
            for(int i=0; i<accountKeyList.length(); i++){
                if(i==keyIndex){
                     accountKey=accountKeyList.at(i);
                }
            }

            //Display key fingerprint.
            ui->label_signIn_keyFpr->setText(accountKey);
            ui->label_signIn_keyFpr->setStyleSheet("color:#999999");

            //Convert key fingerprint into const char* variable type
            //that required in get key process.
            QByteArray ba=accountKey.toLatin1();
            const char *patt=ba.data();

            //Get the account private key.
            gpgme_key_t privateKey = encryption->getKey(patt, 1);

            //Set account private key as active-user's private key in encryption class.
            encryption->setUserPriKey(privateKey);

            //Get the account public key.
            gpgme_key_t publicKey = encryption->getKey(patt, 0);

            //Set account public key as active-user's public key in encryption class.
            encryption->setUserPubKey(publicKey);

            //Close signIn window to show mainWindow.
            SignIn::accept();
        }
        else{ //In case sign in not success.
            //Show sign in result message from server.
            ui->label_signIn_keyFpr->setText(signInResult);
        }
    }

}

//Let other class get selected account username.
QString SignIn::getActiveUser(){
    QString ACTIVE_USER=selectedAccount;
    return ACTIVE_USER;
}

//User typing in [sign up - server ip] line edit.
void SignIn::on_lineEdit_SignUpServerIP_textChanged(const QString &arg1)
{
    //Get server port number from lineEdit
    QString port=ui->lineEdit_SignUpServerPort->text();

    if(arg1!="" && port!=""){ //When port number and ip address are not empty.

        //Enable "Connect" button
        ui->pushButton_SignUpServerConnect->setEnabled(true);

        //Set "Connect" button as default button for enter key.
        ui->pushButton_signIn_AccountSignIn->setDefault(false);
        ui->pushButton_signIn_serverConnect->setDefault(false);
        ui->pushButton_signUpAccountSignUp->setDefault(false);
        ui->pushButton_SignUpServerConnect->setDefault(true);
    }
    else{ //In case server port number or server address empty.

        //Disable "Connect" button
        ui->pushButton_SignUpServerConnect->setEnabled(false);
    }
}

//User typing in [sign up - server port number] line edit.
void SignIn::on_lineEdit_SignUpServerPort_textChanged(const QString &arg1)
{
    //Get server ip address from lineEdit
    QString ip=ui->lineEdit_SignUpServerIP->text();

    if(ip!="" && arg1!=""){//When port number and ip address are not empty.

        //Enable "Connect" button
        ui->pushButton_SignUpServerConnect->setEnabled(true);

        //Set "Connect" button as default button for enter key.
        ui->pushButton_signIn_AccountSignIn->setDefault(false);
        ui->pushButton_signIn_serverConnect->setDefault(false);
        ui->pushButton_signUpAccountSignUp->setDefault(false);
        ui->pushButton_SignUpServerConnect->setDefault(true);
    }
    else{//In case server port number or server address empty.
        //Disable "Connect" button.
        ui->pushButton_SignUpServerConnect->setEnabled(false);
    }
}

//When user click "Connect" button in [ Sign Up - server] tab.
void SignIn::on_pushButton_SignUpServerConnect_clicked()
{

    if(conn->getConnectionStatus()!=1){// In case connection status is not "connected".

        //Hide error message
        ui->label_signUpConnectError->setText("");
        ui->label_signUpConnectError->hide();

        if(conn->getConnectionStatus()==-2){// In case connection status is disconnected
            ui->comboBox_signIn_selectServer->setCurrentText("*New Server");
            ui->comboBox_signUp_selectServer->setCurrentText("*New Server");
        }

        //Get value from [Sign up - Server] form
        QString newIP=ui->lineEdit_SignUpServerIP->text();
        QString newPort=ui->lineEdit_SignUpServerPort->text();
        QString selectedServer=ui->comboBox_signUp_selectServer->currentText();

        //In case user select server from dropdown combobox
        if(selectedServer!="Select server..." && selectedServer!="*New Server"){
            qDebug() << selectedServer;
        }
        else{//In case user not select server from drop down combobox.
            if(selectedServer=="*New Server"){//In case user select *New Server.

                //Show connect status.
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

                //Connect to new server.
                conn->connected(newIP, newPort);

                //In case client applcation can't connect to server.
                if(conn->getConnectionStatus()==-1){
                    ui->label_signUpConnectError->show();
                    ui->label_signUpConnectError->setStyleSheet("color:#AA6666");
                    ui->label_signUpConnectError->setText("ERROR: Can not connect to server!");
                }

                //In case client application can connect to server.
                if(conn->getConnectionStatus()==1){

                    //Get server ip and port from Connection class
                    QString servAddr=conn->getServerAddr();
                    QString servPort=conn->getServerPort();

                    qDebug() << conn->getConnectionStatus();

                    //Enable [sign in - account] and [sign up - account] tab
                    //display connection status.
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

                    //Hide *new server from
                    ui->frame_signIn_serverForm->hide();
                    ui->frame_SignUpServerEnterNew->hide();

                    //Change "Connect" button to be "Disconnect" button
                    ui->pushButton_signIn_serverConnect->setText("Disconnect");
                    ui->pushButton_SignUpServerConnect->setText("Disconnect");

                    //Enable "Disconnect" button
                    ui->pushButton_signIn_serverConnect->setEnabled(true);
                    ui->pushButton_SignUpServerConnect->setEnabled(true);

                    //Disable drop down select server combobox
                    ui->comboBox_signIn_selectServer->setEnabled(false);
                    ui->comboBox_signUp_selectServer->setEnabled(false);

                    //Move to [Sign Up - Account] tab
                    ui->tabWidget_signUp->setCurrentIndex(1);
                }

                //In case server not return server's public key.
                if(conn->getConnectionStatus()==0 || conn->getConnectionStatus()==2){
                    ui->label_signUpConnectError->show();
                    ui->label_signUpConnectError->setStyleSheet("color:#AA6666");
                    ui->label_signUpConnectError->setText("This server or this port not for E2EEIM");
                }

            }
        }

        ui->label_signUpConnectError->hide();
        ui->label_signUpConnectError->show();
        //ui->label_signUpConnectError->setStyleSheet("color:#AA6666");

    }
    else{// In case client application connected to server.

        //Disconnect form current server.
        conn->letDisconnect();

        int connectStatus=conn->getConnectionStatus();

        //Set connection GUI as disconnected.
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

//When user change sub tab of in Sign Up tab.
void SignIn::on_tabWidget_signUp_currentChanged(int index)
{
    if(index==0){// User select [Sign Up - Server] tab.

        //Set "Connect" button as default button for Enter key.
        ui->pushButton_signIn_AccountSignIn->setDefault(false);
        ui->pushButton_signIn_serverConnect->setDefault(false);
        ui->pushButton_signUpAccountSignUp->setDefault(false);
        ui->pushButton_SignUpServerConnect->setDefault(true);
    }
    else if(index==1){//User select [Sign Up - Account] tab.

        //Set "Sign Up" button as defult button for Enter key.
        ui->tabWidget_signUp->setTabEnabled(1, true);
        ui->label_signUpAccountErrMsg->setStyleSheet("color:#AA6666");
        ui->label_signUpAccountErrMsg->setText("");

        ui->pushButton_signIn_AccountSignIn->setDefault(false);
        ui->pushButton_signIn_serverConnect->setDefault(false);
        ui->pushButton_signUpAccountSignUp->setDefault(true);
        ui->pushButton_SignUpServerConnect->setDefault(false);
    }
}

//When user click "Sign Up" button in [Sign Up - Account] tab.
void SignIn::on_pushButton_signUpAccountSignUp_clicked()
{

    //Get information from sign up form.
    QString errMsg;
    QString username=ui->lineEdit_signUpAccountUsername->text();
    QString passphrase=ui->lineEdit_signUpAccountPassphrase->text();
    QString confirmPassphrase=ui->lineEdit_signUpAccountConfirmPassphrase->text();

    //Username validation.
    if(username==""){
        errMsg+="\n-Username can not be empty!";
    }
    else{
        if(username.mid(0,1) == "." || username.mid(0,1) == "-" || username.mid(0,1) == "_"){
            errMsg+="\n-Username first character must be letters(a-z, A-Z) or numbers(0-9)!";
        }
        if(username.length() > 16){
            errMsg+="\n-Username can't be longer then 16 character!";
        }
        else if(username.length() < 4){
            errMsg+="\n-Username must be longer then 4 character!";
        }
        else{
            for(int i=0; i < username.length()-1; i++){
                int unicode=username.mid(i,1).data()->unicode();

                if(unicode<45){
                    errMsg+="\n-Username only letters(a-z, A-Z), numbers(0-9),\n  hyphen(-), underscore(_), periods(.) are allowed!";
                    break;
                }
                if(unicode>122){
                    errMsg+="\n-Username only letters(a-z, A-Z), numbers(0-9),\n  hyphen(-), underscore(_), periods(.) are allowed!";
                    break;
                }
                if(unicode==47||unicode==96){
                    errMsg+="\n-Username only letters(a-z, A-Z), numbers(0-9),\n  hyphen(-), underscore(_), periods(.) are allowed!";
                    break;
                }
                if(unicode > 57 && unicode < 65){
                    errMsg+="\n-Username only letters(a-z, A-Z), numbers(0-9),\n  hyphen(-), underscore(_), periods(.) are allowed!";
                    break;
                }
                if(unicode > 90 && unicode < 94){
                    errMsg+="\n-Username only letters(a-z, A-Z), numbers(0-9),\n  hyphen(-), underscore(_), periods(.) are allowed!";
                    break;
                }
            }

        }

    }

    //Passphrase validation.
    if(passphrase==""){
        errMsg+="\n-Passphrase can not be empty!";
    }
    if(passphrase!=confirmPassphrase){
        errMsg+="\n-Passphrase and confirm-passphrase does not match!";
    }
    if(errMsg!=""){
        ui->label_signUpAccountErrMsg->setText(errMsg);
    }
    else{// When Username and Passphrase valid.

        ui->pushButton_signUpAccountSignUp->setEnabled(false);

        //Set useable tab.
        ui->tabWidget_signUp->setCurrentIndex(2);
        ui->tabWidget_signUp->setTabEnabled(0, false);
        ui->tabWidget_signUp->setTabEnabled(1, false);
        ui->tabWidget_signUp->setTabEnabled(2, true);

        //Inform sign up status.
        ui->label_signUpFinishg->setText("Generating Key Pair...");
        ui->label_signUpAccountErrMsg->setText("Generating Key Pair, please wait...");
        ui->label_signUpAccountErrMsg->setStyleSheet("QLabel { qproperty-alignment: AlignCenter; color:#66AA66;}");
        ui->label_signUpAccountErrMsg->show();


        QStringList account;
        account.append(username);
        account.append(passphrase);

        QCoreApplication::processEvents();

        emit generateKeyPair(account);
    }
}

void SignIn::generateKeyPair(QStringList account){

    QString username=account.at(0);
    QString passphrase=account.at(1);
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

    QCoreApplication::processEvents();

    gpgme_genkey_result_t GenKeyresult;
    GenKeyresult = encryption->genKey(parms);


    QString accountKey=QString(GenKeyresult->fpr);
    QByteArray ba=accountKey.toLatin1();
    const char *patt=ba.data();


    //Get generated key pair..
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


    // //////// Send Sign Up Request to Server.

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

    //ui->label_signUpFinishg->setText("Sending sign up request...");

    if(conn->getConnectionStatus()==1){
        conn->send(data);

        ui->label_signUpAccountErrMsg->setText("Sending sign up request, please wait...");
        ui->label_signUpAccountErrMsg->setStyleSheet("QLabel { qproperty-alignment: AlignCenter; color:#66AA66;}");

        ui->label_signUpFinishg->setText("Sending sign up request, please wait...");

    }

    QCoreApplication::processEvents();

    data.clear();
}


void SignIn::receiveSignUpResult(QByteArray data){


    QString decryptResult="1";

    if(data.length() < 50){ //In case username not available.

        QFile File_Result2("signUpResult.txt");
        if(!File_Result2.open(QFile::WriteOnly | QFile::Text)){
            qDebug() << "Cound not open file for writing";
            abort();
        }
        QTextStream ts2(&File_Result2);
        ts2 << data.mid(5);

        File_Result2.flush();
        File_Result2.close();
    }
    else{ //In case username available

        //Decrypt Payload

        //Save encrypted result to a file.
        QFile File_Result("signUpResult.cipher");
        if(!File_Result.open(QFile::WriteOnly | QFile::Text)){
            qDebug() << "Cound not open file for writing";
            abort();
        }
        QTextStream ts(&File_Result);
        ts << data.mid(5);

        File_Result.flush();
        File_Result.close();

        //Create output file.
        QFile File_Result2("signUpResult.txt");
        if(!File_Result2.open(QFile::WriteOnly | QFile::Text)){
            qDebug() << "Cound not open file for writing";
            abort();
        }
        QTextStream ts2(&File_Result2);
        ts2 << "";

        File_Result2.flush();
        File_Result2.close();


        //Decrypt sign up result from server.
        decryptResult=encryption->decryptVerify("signUpResult.cipher", "signUpResult.txt");

    }



    if(decryptResult.mid(0,1)=="0"){
        ui->label_signUpFinishg->setText("ERROR: Server signature not fully valid");

        ui->pushButton_signUpAccountSignUp->setEnabled(true);
    }
    else{


        //Read dectyped sign up result from text file.
        QFile File_result("signUpResult.txt");
        if(!File_result.open(QFile::ReadOnly | QFile::Text)){
            qDebug() << "Cound not open file for Read";
            abort();
        }
        QTextStream in2(&File_result);
        QString signUpReslut;
        signUpReslut=in2.readAll();
        File_result.close();

        //Display sign up result.
        ui->label_signUpFinishg->setText(signUpReslut);

        /*
        if(signUpReslut.mid(0,9)=="Username:"){

            encryption->deletePrivateKey(newUsersPrivateKey);

        }
        */

        qDebug() << signUpReslut;

        //Enable use able tab after sign up.
        ui->tabWidget_signUp->setTabEnabled(0, true);
        ui->tabWidget_signUp->setTabEnabled(1, true);
        ui->tabWidget_signUp->setTabEnabled(2, true);

        //Clear sign up form.
        ui->lineEdit_signUpAccountUsername->clear();
        ui->lineEdit_signUpAccountPassphrase->clear();
        ui->lineEdit_signUpAccountConfirmPassphrase->clear();

        ui->pushButton_signUpAccountSignUp->setEnabled(true);

    }

}

//When user change main tab between SignIn/SignUp
void SignIn::on_tabWidget_mainTab_currentChanged(int index)
{
    //When connection status is "connected"
    if(conn->getConnectionStatus()==1){

        //Get connected server ip and port.
        QString servAddr=conn->getServerAddr();
        QString servPort=conn->getServerPort();

        qDebug() << conn->getConnectionStatus();

        //Display connection status.
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

        //Hide *New Server form.
        ui->frame_signIn_serverForm->hide();
        ui->frame_SignUpServerEnterNew->hide();

        //Change "connect" button to be "Disconnect" button.
        ui->pushButton_signIn_serverConnect->setText("Disconnect");
        ui->pushButton_SignUpServerConnect->setText("Disconnect");

        //Enable "Disconnect" button.
        ui->pushButton_signIn_serverConnect->setEnabled(true);
        ui->pushButton_SignUpServerConnect->setEnabled(true);

        //Disable drop down select server combobox.
        ui->comboBox_signIn_selectServer->setEnabled(false);
        ui->comboBox_signUp_selectServer->setEnabled(false);
    }

    if(conn->getConnectionStatus()==-2){ //In case disconnted.

        //Get recent connected server ip and port.
            QString servAddr=conn->getServerAddr();
            QString servPort=conn->getServerPort();

            qDebug() << conn->getConnectionStatus();


            //Disable [Sign In - Account] tab
            //and display connection status.
            ui->tabWidget_signIn->setTabEnabled(1, false);
            ui->tabWidget_signIn->setCurrentIndex(0);
            ui->label_signIn_serverErr->show();
            ui->label_signIn_serverErr->setText("Disconnect from server!! \nServer:" +servAddr+":"+servPort);
            ui->label_signIn_serverErr->setStyleSheet("QLabel { qproperty-alignment: AlignCenter; color:#AA6666;}");


            //Disable [Sign Up - Account] tab
            //and display connection status.
            ui->tabWidget_signUp->setTabEnabled(1, false);
            ui->tabWidget_signUp->setCurrentIndex(0);
            ui->label_signUpConnectError->show();
            ui->label_signUpConnectError->setText("Disconnect from server!! \nServer:" +servAddr+":"+servPort);
            ui->label_signUpConnectError->setStyleSheet("QLabel { qproperty-alignment: AlignCenter; color:#AA6666;}");

            //Display *New Server form.
            ui->frame_signIn_serverForm->show();
            ui->frame_SignUpServerEnterNew->show();

            //Placeholder as recent conneted server in *New Server form.
            ui->lineEdit_signIn_serverAddress->setText(servAddr);
            ui->lineEdit_signIn_serverPort->setText(servPort);

            ui->lineEdit_SignUpServerIP->setText(servAddr);
            ui->lineEdit_SignUpServerPort->setText(servPort);

            //Enable "Connect" button.
            ui->pushButton_signIn_serverConnect->setText("Connect");
            ui->pushButton_SignUpServerConnect->setText("Connect");

            ui->pushButton_signIn_serverConnect->setEnabled(true);
            ui->pushButton_SignUpServerConnect->setEnabled(true);

            ui->comboBox_signIn_selectServer->setEnabled(true);
            ui->comboBox_signUp_selectServer->setEnabled(true);
        }


    if(index == 0){ // When user select Sign In tab.
        if(ui->tabWidget_signIn->currentIndex()==0 ||
                ui->tabWidget_signIn->currentIndex()==1){ //When user select Server or Account tab.
            QStringList allAccounts=encryption->getE2eeimAccounts();

            //Reload useable account.
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

                //Get selected account.
                selectedAccount=ui->comboBox_signIn_SelectAccount->currentText();
                on_comboBox_signIn_SelectAccount_currentIndexChanged(selectedAccount);
            }

            //Set "Connect" button as default button for Enter key.
            if(ui->tabWidget_signIn->currentIndex()==0){
                ui->pushButton_signIn_AccountSignIn->setDefault(false);
                ui->pushButton_signIn_serverConnect->setDefault(true);
                ui->pushButton_signUpAccountSignUp->setDefault(false);
                ui->pushButton_SignUpServerConnect->setDefault(false);

            }
            //Set "Sign In" button as default button for Enter key.
            if(ui->tabWidget_signIn->currentIndex()==1){
                ui->pushButton_signIn_AccountSignIn->setDefault(true);
                ui->pushButton_signIn_serverConnect->setDefault(false);
                ui->pushButton_signUpAccountSignUp->setDefault(false);
                ui->pushButton_SignUpServerConnect->setDefault(false);
            }
        }
    }
}

//User select account in drop-down combobox.
void SignIn::on_comboBox_signIn_SelectAccount_currentIndexChanged(const QString &arg1)
{


    //Get selected account.
    selectedAccount=arg1;
    selectedAccount=ui->comboBox_signIn_SelectAccount->currentText();
    QString accountKey;

    //Get selected account index.
    //int keyIndex=accountNameList.indexOf(selectedAccount);
    int keyIndex=ui->comboBox_signIn_SelectAccount->currentIndex();


    //Get key fingerprint of selected account.
    for(int i=0; i<accountKeyList.length(); i++){
        if(i==keyIndex){
             accountKey=accountKeyList.at(i);
        }
    }

    //Show key fingerprint of selected account.
    ui->label_signIn_keyFpr->setText(accountKey);
    ui->label_signIn_keyFpr->setStyleSheet("color:#999999");

    QByteArray ba=accountKey.toLatin1();
    const char *patt=ba.data();

    //Set selected account key for Encryption class.
    gpgme_key_t privateKey = encryption->getKey(patt, 1);
    encryption->setUserPriKey(privateKey);

    gpgme_key_t publicKey = encryption->getKey(patt, 0);
    encryption->setUserPubKey(publicKey);

    //Set "Sign In" button as default button for enter key.
    ui->pushButton_signIn_AccountSignIn->setDefault(true);
    ui->pushButton_signIn_serverConnect->setDefault(false);
    ui->pushButton_signUpAccountSignUp->setDefault(false);
    ui->pushButton_SignUpServerConnect->setDefault(false);

}

//When user select server in drop-down combobox in [Sign In - server] tab.
void SignIn::on_comboBox_signIn_selectServer_currentIndexChanged(const QString &arg1)
{
    if(arg1=="Select Server..."){ // In case user not select server.
        //Disable "Connect" button.
        ui->pushButton_signIn_serverConnect ->setEnabled(false);
        ui->frame_signIn_serverForm->hide();

    }
    if(arg1=="*New Server"){ // In case user select *New Server.

        //Get ip and port of New Server.
        QString ip=ui->lineEdit_signIn_serverAddress->text();
        QString port=ui->lineEdit_signIn_serverPort->text();

        if(ip=="" || port==""){// Ip or Port is empty.
            //Disable "Connect" button.
            ui->pushButton_signIn_serverConnect->setEnabled(false);
        }
        else{//Ip and port is not empty.

            //Enable "Connect" button.
            ui->pushButton_signIn_serverConnect->setEnabled(true);

            //Set "Connect" button as default button for Enter key.
            ui->pushButton_signIn_AccountSignIn->setDefault(false);
            ui->pushButton_signIn_serverConnect->setDefault(true);
            ui->pushButton_signUpAccountSignUp->setDefault(false);
            ui->pushButton_SignUpServerConnect->setDefault(false);
        }
    }
    if(arg1!="*New Server"){// User not select "*New server"

        //Hide *New Server form.
        ui->frame_signIn_serverForm->hide();
    }
    else{
        //Show *New Server form.
        ui->frame_signIn_serverForm->show();
    }

}

//User typing server address in [Sign In - Server] tab.
void SignIn::on_lineEdit_signIn_serverAddress_textChanged(const QString &arg1)
{
    //Get server port.
    QString port=ui->lineEdit_signIn_serverPort->text();
    if(arg1!="" && port!=""){ //When sever ip and port are not empty.

        //Enable "Connect" button.
        ui->pushButton_signIn_serverConnect->setEnabled(true);

        //Set "Connect" button as default button for Enter key.
        ui->pushButton_signIn_AccountSignIn->setDefault(false);
        ui->pushButton_signIn_serverConnect->setDefault(true);
        ui->pushButton_signUpAccountSignUp->setDefault(false);
        ui->pushButton_SignUpServerConnect->setDefault(false);
    }
    else{
        //Disable "Connect" button.
        ui->pushButton_signIn_serverConnect->setEnabled(false);
    }
}

//User typing server port in [Sign In - Server] tab.
void SignIn::on_lineEdit_signIn_serverPort_textChanged(const QString &arg1)
{
    //Get server ip.
    QString ip=ui->lineEdit_signIn_serverAddress->text();
    if(ip!="" && arg1!=""){ //When server ip and port are not empty.

        //Enable "Connect" button.
        ui->pushButton_signIn_serverConnect->setEnabled(true);

        //Set "Connect" button as default button for enter key.
        ui->pushButton_signIn_AccountSignIn->setDefault(false);
        ui->pushButton_signIn_serverConnect->setDefault(true);
        ui->pushButton_signUpAccountSignUp->setDefault(false);
        ui->pushButton_SignUpServerConnect->setDefault(false);
    }
    else{

        //Disalbe "Connect" button.
        ui->pushButton_signIn_serverConnect->setEnabled(false);
    }
}

//User click "Connect" button in [Sign In - Server] tab.
void SignIn::on_pushButton_signIn_serverConnect_clicked()
{
    if(conn->getConnectionStatus()!=1){ //When client application not connect to any server.

        if(conn->getConnectionStatus()==-2){
            ui->comboBox_signIn_selectServer->setCurrentText("*New Server");
            ui->comboBox_signUp_selectServer->setCurrentText("*New Server");
        }
        //Hide error message.
        ui->label_signIn_serverErr->setText("");
        ui->label_signIn_serverErr->hide();

        //Get server information in server tab.
        QString newIP=ui->lineEdit_signIn_serverAddress->text();
        QString newPort=ui->lineEdit_signIn_serverPort->text();
        QString selectedServer=ui->comboBox_signIn_selectServer->currentText();

        if(selectedServer!="Select Server..." && selectedServer!="*New Server"){ //In case user select a server from drop-down combobox.
            qDebug() << selectedServer;
        }
        else{// In case user not select any server.

            //User select "*New Server"
            if(selectedServer=="*New Server"){
                //Display connection satatus.
                qDebug()<<newIP<<", Port "<<newPort;
                ui->label_signIn_serverErr->setText("Waiting for connection...");
                ui->label_signIn_serverErr->setStyleSheet("color:#333333");
                ui->label_signIn_serverErr->show();

                QCoreApplication::processEvents();

                //Send connection request to new server.
                conn->connected(newIP, newPort);

                QCoreApplication::processEvents();

                //In case cannot connect to new server.
                if(conn->getConnectionStatus()==-1){
                    ui->label_signIn_serverErr->show();
                    ui->label_signIn_serverErr->setStyleSheet("color:#AA6666");
                    ui->label_signIn_serverErr->setText("ERROR: Can not connect to server!");
                }

                //In caes can connect to New Server.
                if(conn->getConnectionStatus()==1){

                    //Get server information.
                    QString servAddr=conn->getServerAddr();
                    QString servPort=conn->getServerPort();

                    qDebug() << conn->getConnectionStatus();

                    //Display connection status and enable tab after connected to a server.
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

                    //Hide *New Server form.
                    ui->frame_signIn_serverForm->hide();
                    ui->frame_SignUpServerEnterNew->hide();

                    //Change "Connect" button to be "Disconnect" button.
                    ui->pushButton_signIn_serverConnect->setText("Disconnect");
                    ui->pushButton_SignUpServerConnect->setText("Disconnect");

                    //Enable "Disconnect" button.
                    ui->pushButton_signIn_serverConnect->setEnabled(true);
                    ui->pushButton_SignUpServerConnect->setEnabled(true);

                    //Disale select server combobox.
                    ui->comboBox_signIn_selectServer->setEnabled(false);
                    ui->comboBox_signUp_selectServer->setEnabled(false);

                }
                if(conn->getConnectionStatus()==0 || conn->getConnectionStatus()==2){
                    ui->label_signIn_serverErr->show();
                    ui->label_signIn_serverErr->setStyleSheet("color:#AA6666");
                    ui->label_signIn_serverErr->setText("This server or this port not for E2EEIM");
                }

            }
        }

        ui->label_signIn_serverErr->hide();
        ui->label_signIn_serverErr->show();
        ui->label_signUpConnectError->setStyleSheet("color:#AA6666");
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


//User select server in drop-down combobox.
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

            ui->pushButton_signIn_AccountSignIn->setDefault(false);
            ui->pushButton_signIn_serverConnect->setDefault(false);
            ui->pushButton_signUpAccountSignUp->setDefault(false);
            ui->pushButton_SignUpServerConnect->setDefault(true);
        }
    }
    if(arg1!="*New Server"){
        ui->frame_SignUpServerEnterNew->hide();
    }
    else{
        ui->frame_SignUpServerEnterNew->show();
    }
}

void SignIn::disconnectFromServer(){
    qDebug() << "DISCONNECT FROM SERVER";
    this->setWindowTitle("E2EEIM DISCONNECT FROM SERVER!!");

    ui->tabWidget_signIn->setTabEnabled(1, false);
    ui->tabWidget_signUp->setTabEnabled(1, false);
    ui->tabWidget_signUp->setTabEnabled(2, false);

    ui->tabWidget_signIn->setCurrentIndex(0);
    on_tabWidget_mainTab_currentChanged(0);
}

void SignIn::on_tabWidget_signIn_currentChanged(int index)
{
    if(index==1){

        QStringList allAccounts=encryption->getE2eeimAccounts();

        //Reload useable account.
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

            //Get selected account.
            selectedAccount=ui->comboBox_signIn_SelectAccount->currentText();
            on_comboBox_signIn_SelectAccount_currentIndexChanged(selectedAccount);
        }
    }
}
