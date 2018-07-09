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

#include "creategroup.h"
#include "ui_creategroup.h"


QStringList ReadContactList(QString Filename){
    QFile File(Filename);
    QStringList List;
    if(!File.exists()){
        if(!File.open(QFile::WriteOnly | QFile::Text)){
            qDebug() << "cound not open file for writing";
            abort();
        }
        QTextStream out(&File);
        out << "";

        File.flush();
        File.close();
    }

    if(!File.open(QFile::ReadOnly | QFile::Text)){
        qDebug() << "cound not open file for Read";
        abort();
    }

    QTextStream in(&File);
   QString line;
    while( !in.atEnd())
    {
         line= in.readLine();
         List.append(line);
    }

    File.flush();
    File.close();
    return List;
}

CreateGroup::CreateGroup(Connection &conn, Encryption &encryption, QString activeUser, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CreateGroup)
{
    ui->setupUi(this);

    this->conn=&conn;
    this->encryption=&encryption;
    ACTIVE_USER = activeUser;

    ui->listWidget->clear();                                //clear current display list
    QString Filename = "userData/"+ACTIVE_USER+"/contactList.txt";
    QStringList contactList=ReadContactList(Filename);                 //Read user data from file.
    foreach(QString CONTACT, contactList){
        if(CONTACT != ""){
            QListWidgetItem *contact = new QListWidgetItem;
            contact->setIcon(QIcon(":/img/person.png"));
            contact->setText(CONTACT);
            ui->listWidget->addItem(contact);
        }
    }
}

CreateGroup::~CreateGroup()
{
    delete ui;
}

void CreateGroup::on_pushButton_clicked()
{
    QStringList GROUP;

    QString groupName=ui->lineEdit->text();
    groupName=groupName.remove("~~");
    GROUP.append("~~"+groupName+"~~");

    QList <QListWidgetItem*> groupMember = ui->listWidget->selectedItems();
    foreach( QListWidgetItem *member, groupMember){
        GROUP.append(member->text());
    }
    GROUP.append(ACTIVE_USER);

    if(groupName==""){
        ui->label->setText("Please enter group name.");
    }
    else if(GROUP.length() < 4){
        ui->label->setText("Please select at least 2 friends to create group!");
    }
    else{
        foreach(QString item, GROUP) {
            QString Filename = "userData/"+ACTIVE_USER+"/groupList.txt";
            QFile File(Filename);
            if(!File.exists()){
                if(!File.open(QFile::WriteOnly | QFile::Text)){
                    qDebug() << "cound not open file for writing";
                    abort();
                }
                QTextStream out(&File);
                out << "";

             File.flush();
                File.close();
            }
            if(File.exists()){
              if(!File.open(QFile::Append | QFile::Text)){
                    qDebug() << "cound not open file for writing";
                    abort();
             }
                QTextStream out(&File);
                out << item+"\n";

                File.flush();
                File.close();

            }

        }


        foreach(QString recipient, GROUP) {

            qDebug() <<"[recipient]:" <<recipient;
            if(recipient.mid(0,1)!="~" && recipient!=ACTIVE_USER){
                gpgme_key_t keys[GROUP.length()-2];
                qDebug() << "GROUP LENGTH:"<<GROUP.length();
                int i=0;
                foreach (QString item, GROUP) {

                    qDebug() << "GROUP MEMBER:"<< item;
                    if(item.mid(0,1)!="~" && item!=ACTIVE_USER && item!=recipient){

                        qDebug() << "-----GET KEY:" << item;

                        QByteArray qb=item.toLatin1();
                        const char *pattern=qb.data();
                        gpgme_key_t key=encryption->getKey(pattern, 0);
                        encryption->printKeys(key);
                        keys[i]=key;
                        i++;
                    }
                }
                keys[GROUP.length()-3]=NULL;

                QString Filename = "userData/"+ACTIVE_USER+"/groupPubKey.txt";
                QByteArray qb = Filename.toLatin1();
                const char *outputFilename = qb.data();

                encryption->exportMultiKey(keys, outputFilename);

                //Add to group message
                QString addGroupFile = "userData/"+ACTIVE_USER+"/addToGroup.txt";
                QFile FileAG(addGroupFile);
                if(!FileAG.exists()){
                    if(!FileAG.open(QFile::WriteOnly | QFile::Text)){
                        qDebug() << "cound not open file for writing";
                        abort();
                    }
                    QTextStream out(&FileAG);
                    out << "";
                    FileAG.flush();
                    FileAG.close();
                }
                if(FileAG.exists()){
                  if(!FileAG.open(QFile::WriteOnly | QFile::Text)){
                        qDebug() << "cound not open file for writing";
                        abort();
                 }
                    QTextStream out(&FileAG);
                    out << "~+"+groupName+"\n";
                    foreach (QString member, GROUP) {
                        if(member.mid(0,1)!="~"){
                            out << member+"\n";
                        }
                    }
                    out << "~~~~\n";

                    QFile groupPubKey_File("userData/"+ACTIVE_USER+"/groupPubKey.txt");
                    if(!groupPubKey_File.open(QFile::ReadOnly | QFile::Text)){
                        qDebug() << "Cound not open file for Read";
                        abort();
                    }
                    QTextStream in(&groupPubKey_File);
                    QString pubKey;
                    pubKey=in.readAll();
                    groupPubKey_File.close();

                    out << pubKey;

                    FileAG.flush();
                    FileAG.close();

                }



                //Encrypt payload
                QString inputFilename_QS = "userData/"+ACTIVE_USER+"/addToGroup.txt";
                QByteArray qb2 = inputFilename_QS.toLatin1();
                const char *inputFilename = qb2.data();

                QString outputFilename_QS = "userData/"+ACTIVE_USER+"/addToGroup.cipher";
                QByteArray qb3 = outputFilename_QS.toLatin1();
                const char *cipher = qb3.data();

                QByteArray qb4 = recipient.toLatin1();
                const char *recipientUsername = qb4.data();

                gpgme_key_t recipientKey=encryption->getKey(recipientUsername, 0);
                gpgme_key_t userPriKey=encryption->getUserPriKey();
                encryption->encryptSign(userPriKey, recipientKey, inputFilename, cipher);



                //Create payload send to server.

                QFile inviteFile("userData/"+ACTIVE_USER+"/addToGroup.cipher");
                if(!inviteFile.open(QFile::ReadOnly | QFile::Text)){
                    qDebug() << "Cound not open file for Read";
                    abort();
                }
                QTextStream in(&inviteFile);
                QString msg;
                msg=in.readAll();
                inviteFile.close();


                char recvUsernameSize=char(recipient.size());

                QByteArray data;
                data.append(msg);

                data.insert(0, recipient);
                data.insert(0, recvUsernameSize);

                Filename="./userData/"+ACTIVE_USER+"/msgToEncrypt.txt";
                QFile rawFile(Filename);
                if(!rawFile.exists()){
                    if(!rawFile.open(QFile::WriteOnly | QFile::Text)){
                        qDebug() << "could not open "+Filename+" file for writing";
                        abort();
                    }

                    QTextStream out(&rawFile);
                    out << data;

                    rawFile.flush();
                    rawFile.close();

                }
                if(rawFile.exists()){
                    if(!rawFile.open(QFile::WriteOnly | QFile::Text)){
                        qDebug() << "could not open "+Filename+" file for writing";
                        abort();
                    }

                    QTextStream out(&rawFile);
                    out << data;

                    rawFile.flush();
                    rawFile.close();

                }

                Filename="./userData/"+ACTIVE_USER+"/msgToEncrypt.txt";
                QByteArray qb2x=Filename.toLatin1();
                const char *inputFileName=qb2x.data();


                Filename="./userData/"+ACTIVE_USER+"/msgToEncrypt.cipher";
                QByteArray qb3x=Filename.toLatin1();
                const char *outPutFileName=qb3x.data();

                Filename="./userData/"+ACTIVE_USER+"/msgToEncrypt.cipher";
                QFile cipherFile(Filename);
                if(!cipherFile.exists()){
                    if(!cipherFile.open(QFile::WriteOnly | QFile::Text)){
                        qDebug() << "could not open"+Filename+" file for writing";
                        abort();
                    }
                    QTextStream out(&cipherFile);
                    out << "";

                    cipherFile.flush();
                    cipherFile.close();
                }

                gpgme_key_t servKey=encryption->getServerPubKey();
                encryption->printKeys(servKey);
                encryption->encryptSign(userPriKey, servKey, inputFileName, outPutFileName);

                QFile qf("./userData/"+ACTIVE_USER+"/msgToEncrypt.cipher");
                if(!qf.open(QFile::ReadOnly | QFile::Text)){
                    qDebug() << "could not open msgToEncrypt.cipher file for Read";
                    abort();
                }
                QTextStream pl(&qf);
                QString qs;
                qs=pl.readAll();
                qf.close();

                QString payload=qs;

                data.clear();
                data.append(payload);


                // Insert operation in front of byte array (data[0]).
                data.insert(0, (char)17);

                //Insert size of(operation + payload) in front of byte array (data[0]).
                int dataSize=data.size();
                QByteArray dataSizeByte;
                QDataStream ds(&dataSizeByte, QIODevice::WriteOnly);
                ds << dataSize;
                data.insert(0, dataSizeByte);

                conn->send(data);

                qDebug() << "-----------------------------------invite sended to " << recipient;


            }
        }



        CreateGroup::close();
    }

}
