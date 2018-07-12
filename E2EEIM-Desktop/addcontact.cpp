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

#include "addcontact.h"
#include "ui_addcontact.h"
#include <QDebug>
#include <QFile>
#include <QTextStream>


AddContact::AddContact(Connection &conn, Encryption &encryption, QStringList &addFriendRequestList, QWidget *parent, QString activeUser) :
    QDialog(parent),
    ui(new Ui::AddContact)
{
    ui->setupUi(this);

    this->addFriendRequestList=&addFriendRequestList;

    this->conn=&conn;
    this->encryption=&encryption;

    this->servKey=encryption.getServerPubKey();
    this->userPriKey=encryption.getUserPriKey();
    this->userPubKey=encryption.getUserPubKey();

    ui->pushButton_search->setEnabled(false);

    ACTIVE_USER=activeUser;

    ui->frame_account->hide();
    ui->label_searchResult->hide();
}

AddContact::~AddContact()
{
    delete ui;
}

void AddContact::on_lineEdit_search_textChanged(const QString &arg1)
{
    if(arg1==""){
        ui->pushButton_search->setEnabled(false);
    }
    else{
        ui->pushButton_search->setEnabled(true);
    }
}

void AddContact::on_pushButton_search_clicked()
{
    QString keyword=ui->lineEdit_search->text();
    QByteArray payload;
    payload.append(keyword);

    //Encrypt Payload
    QFile File_Payload("searchContact.keyword");
    if(!File_Payload.open(QFile::WriteOnly | QFile::Text)){
        qDebug() << "Cound not open file searchContact.keyword for writing";
        exit(1);
    }
    QTextStream out(&File_Payload);
    out << payload;

    File_Payload.flush();
    File_Payload.close();

    encryption->encryptSign(userPriKey, servKey, "searchContact.keyword", "searchContact.cipher");

    QFile File_EncryptedPayload("searchContact.cipher");
    if(!File_EncryptedPayload.open(QFile::ReadOnly | QFile::Text)){
        qDebug() << "Cound not open file for Read";
        abort();
    }
    QTextStream in(&File_EncryptedPayload);
    QString cipher;
    cipher=in.readAll();
    File_EncryptedPayload.close();

    payload.clear();

    QByteArray data;

    data.append(cipher);

    // Insert operation in front of byte array (data[0]).
    data.insert(0, (char)9);

    //Insert size of(operation + payload) in front of byte array (data[0]).
    int dataSize=data.size();
    QByteArray dataSizeByte;
    QDataStream ds2(&dataSizeByte, QIODevice::WriteOnly);
    ds2 << dataSize;
    data.insert(0, dataSizeByte);

    conn->send(data);
    data.clear();
    data=conn->getRecentReceivedMsg();

    //Decrypt Payload
    QFile File_Result("searchUserResult.cipher");
    if(!File_Result.open(QFile::WriteOnly | QFile::Text)){
        qDebug() << "Cound not open file earchUserResult.cipher for writing";
        abort();
    }
    QTextStream ts(&File_Result);
    ts << data.mid(5);

    File_Result.flush();
    File_Result.close();

    qDebug() << "DEEP_DEBUG_I";

    QString decryptResult=encryption->decryptVerify("searchUserResult.cipher", "searchUserResult.txt");

    if(decryptResult.mid(0,1)=="0"){
        ui->label_searchResult->setStyleSheet("qproperty-alignment: AlignCenter; color:#FF6666");
        ui->label_searchResult->setText("ERROR: Server signature not fully valid");
    }
    else{
        QFile File_result("searchUserResult.txt");
        if(!File_result.open(QFile::ReadOnly | QFile::Text)){
            qDebug() << "Cound not open file for Read";
            abort();
        }
        QTextStream in(&File_result);
        QString qs;
        qs=in.readAll();
        File_result.close();

        QString result=qs;


        QString Filename = "./userData/"+ACTIVE_USER+"/contactList.txt";
        QStringList contactList;
        QFile File(Filename);
        if(!File.exists()){
            if(!File.open(QFile::WriteOnly | QFile::Text)){
                qDebug() << "Cound not open file for writing";
                exit(1);
            }
            QTextStream out(&File);
            out << "";

            File.flush();
            File.close();
        }
        if(!File.open(QFile::ReadOnly | QFile::Text)){
            qDebug() << "Cound not open file for Read";
            exit(1);
        }

        QTextStream in2(&File);
        QString line;
        while( !in2.atEnd())
        {
            line=in2.readLine();
            contactList.append(line);
        }


        bool isFriend=false;
        foreach(QString contact, contactList){
            if(contact==keyword){
                isFriend=true;
                break;
            }
            if(contact=="!"+keyword){
                isFriend=true;
                break;
            }
            if(contact=="@"+keyword){
                isFriend=true;
                break;
            }

        }

        if(result=="0"){
            qDebug() << "Username:" << keyword << "not found!";
            ui->label_searchResult->setText("Username: "+keyword+", not found in this server!");
            ui->label_searchResult->setStyleSheet("QLabel { qproperty-alignment: AlignCenter; color:#555555;}");
            ui->label_searchResult->show();
            ui->frame_account->hide();

        }
        else if(keyword==ACTIVE_USER){
            QString status=result.split("\n").first();
            QString username=result.split("\n").at(1);
            QString key=result.split("\n").last();

            ui->label_account_username->setText(username);
            ui->label_account_key->setText(key);

            ui->label_searchResult->hide();
            ui->frame_account->show();
            ui->pushButton_sendAddFriendRequest->setEnabled(false);
            ui->pushButton_sendAddFriendRequest->setText("It's you!");
        }
        else if(isFriend==true){
            QString status=result.split("\n").first();
            QString username=result.split("\n").at(1);
            QString key=result.split("\n").last();

            ui->label_account_username->setText(username);
            ui->label_account_key->setText(key);

            ui->label_searchResult->hide();
            ui->frame_account->show();
            ui->pushButton_sendAddFriendRequest->setEnabled(false);
            ui->pushButton_sendAddFriendRequest->setText("Friend");
        }
        else{
            QString status=result.split("\n").first();
            QString username=result.split("\n").at(1);
            QString key=result.split("\n").last();

            ui->label_account_username->setText(username);
            ui->label_account_key->setText(key);

            ui->label_searchResult->hide();
            ui->frame_account->show();
            ui->pushButton_sendAddFriendRequest->setEnabled(true);
            ui->pushButton_sendAddFriendRequest->setText("Add Friend");

            foundUser=username;
        }
    }




}

void AddContact::on_pushButton_sendAddFriendRequest_clicked()
{
    QByteArray payload;

    payload.append(foundUser);

    //Encrypt Payload
    QFile File_Payload("searchContact.keyword");
    if(!File_Payload.open(QFile::WriteOnly | QFile::Text)){
        qDebug() << "Cound not open file searchContact.keyword for writing";
        exit(1);
    }
    QTextStream out(&File_Payload);
    out << payload;

    File_Payload.flush();
    File_Payload.close();

    encryption->encryptSign(userPriKey, servKey, "searchContact.keyword", "searchContact.cipher");

    QFile File_EncryptedPayload("searchContact.cipher");
    if(!File_EncryptedPayload.open(QFile::ReadOnly | QFile::Text)){
        qDebug() << "Cound not open file for Read";
        abort();
    }
    QTextStream in(&File_EncryptedPayload);
    QString cipher;
    cipher=in.readAll();
    File_EncryptedPayload.close();

    payload.clear();

    QByteArray data;

    data.append(cipher);

    // Insert operation in front of byte array (data[0]).
    data.insert(0, (char)11);

    //Insert size of(operation + payload) in front of byte array (data[0]).
    int dataSize=data.size();
    QByteArray dataSizeByte;
    QDataStream ds2(&dataSizeByte, QIODevice::WriteOnly);
    ds2 << dataSize;
    data.insert(0, dataSizeByte);

    conn->send(data);

    data.clear();
    data=conn->getRecentReceivedMsg();

    //Decrypt Payload
    QFile File_Result("searchUserResult.cipher");
    if(!File_Result.open(QFile::WriteOnly | QFile::Text)){
        qDebug() << "Cound not open file searchUserResult.cipher for writing";
        abort();
    }
    QTextStream ts(&File_Result);
    ts << data.mid(5);

    File_Result.flush();
    File_Result.close();

    QString decryptResult=encryption->decryptVerify("searchUserResult.cipher", "searchUserResult.txt");

    if(decryptResult.mid(0,1)=="0"){
        ui->label_searchResult->setStyleSheet("qproperty-alignment: AlignCenter; color:#FF6666");
        ui->label_searchResult->setText("ERROR: Server signature not fully valid");
    }
    else{
        QFile File_result("searchUserResult.txt");
        if(!File_result.open(QFile::ReadOnly | QFile::Text)){
            qDebug() << "Cound not open file for Read";
            abort();
        }
        QTextStream in(&File_result);
        QString qs;
        qs=in.readAll();
        File_result.close();

        QString result=qs;

        if(result=="1"){
            this->addFriendRequestList->append(foundUser);

            QFile File("./userData/"+ACTIVE_USER+"/addFriendRequestList.txt");
            if(!File.exists()){
                if(!File.open(QFile::WriteOnly | QFile::Text)){
                    qDebug() << "could not open addFriendRequestList.txt file for writing";
                    abort();
                }
                QTextStream out(&File);
                out.setCodec("UTF-8");
                out << "";

                File.flush();
                File.close();
            }
            if(File.exists()){
                if(!File.open(QFile::Append | QFile::Text)){
                    qDebug() << "could not open addFriendRequestList.txt file for writing";
                    abort();
                }
                QTextStream out(&File);
                out.setCodec("UTF-8");
                out << foundUser+"\n";

                File.flush();
                File.close();
            }

            ui->pushButton_sendAddFriendRequest->setText("Add friend request sended");
            ui->pushButton_sendAddFriendRequest->setEnabled(false);

        }
    }
}
