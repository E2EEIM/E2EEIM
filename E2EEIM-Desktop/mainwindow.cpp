//
// Copyright (C) 2018  Krittitam Juksukit
// This file is part of E2EEIM.
//
// E2EEIM is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License
// version 3 as published by the Free Software Foundation.
//
// E2EEIM is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with E2EEIM.  If not, see <http://www.gnu.org/licenses/>.
//
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "addcontact.h"
#include "creategroup.h"
#include <qlayout.h>
#include <algorithm>
#include <string>
#include <QDesktopWidget>
#include <QMainWindow>
#include <QWidget>
#include <QFile>
#include <QString>
#include <QDebug>
#include <QTextStream>
#include <QStringList>
#include <QDir>
#include <QStringRef>
#include <QFileInfo>
#include <QPainter>

using namespace std;

QString currentMenu;
QString ACTIVE_USR;
QString conversationWith;

QStringList getGroupMember(QString GroupName){
    QFile File("./userData/"+ACTIVE_USR+"/groupList.txt");
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
    short flagLoad=0;
    while(!in.atEnd())
    {
            line=in.readLine();
            if(line.left(2)=="~~"){
                if((line.split("~~").at(1)) == GroupName){
                    flagLoad=1;
                    continue;
                }
                else if(flagLoad==1)
                    break;
            }
            if(flagLoad!=0){
                List.append(line);

            }
    }
    return List;
}

// ///////////////////////////////////////READ CONTACT LIST
QStringList Read(QString Filename){
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
        if(Filename=="userData/"+ACTIVE_USR+"/groupList.txt"){
            line=in.readLine();
            if(line.left(2)!="~~")
                continue;
            List.append(line.split("~~").at(1));
        }
        else{
            line= in.readLine();
            List.append(line);
        }
    }

    File.flush();
    File.close();
    return List;
}


QStringList ReadConversation(QString Filename){

    int ACTIVE_USR_length;
    int conversationWith_length;

    ACTIVE_USR_length=ACTIVE_USR.length();
    conversationWith_length=conversationWith.length();

    QFile File(Filename);
    QStringList List;
    if(!File.exists()){
        if(!File.open(QFile::WriteOnly | QFile::Text)){
            qDebug() << "Cound not open file for writing";
            abort();
        }
        QTextStream out(&File);
        out << "";

        File.flush();
        File.close();
    }


    if(!File.open(QFile::ReadOnly | QFile::Text)){
        qDebug() << "Cound not open file for Read";
        abort();
    }

    QTextStream in(&File);
    QString line;
    QString tmp="[-STaRT-]";

    if(Filename.split("~~").first()=="./userData/"+ACTIVE_USR+"/conversation/GROUP"){ //In case group conversation;
        QStringList groupMember=getGroupMember(Filename.split("~~").last());
        while( !in.atEnd())
        {
            line=in.readLine();
            int flagFistLine=1;
            if(line.left(ACTIVE_USR_length+1)!=(ACTIVE_USR+":")){
                flagFistLine=0;
                foreach(QString memberName, groupMember){
                    if(line.left(memberName.length()+1)==(memberName+":")){
                        flagFistLine=1;
                        break;
                    }
                }
            }
            if(flagFistLine==0){
                tmp=tmp+"\n"+line;
                if(in.atEnd())
                    List.append(tmp);
                continue;
            }
            if(tmp!="[-STaRT-]"){
                List.append(tmp);
            }
            if(in.atEnd()){
                List.append(line);
            }
            tmp=line;
        }

    }
    else{
        while( !in.atEnd())
        {
            line= in.readLine();
            if((line.left(ACTIVE_USR_length+1)!=(ACTIVE_USR+":")) &&
                    (line.left(conversationWith_length+1)!=(conversationWith+":"))){
                tmp=tmp+"\n"+line;
                if(in.atEnd())
                    List.append(tmp);
                continue;
            }
            if(tmp!="[-STaRT-]"){
                List.append(tmp);
            }
            if(in.atEnd()){
                List.append(line);
            }
             tmp=line;
        }
    }

    File.flush();
    File.close();
    return List;
}


MainWindow::MainWindow(QString activeUser, QWidget *parent) : // /////////////////////////////////////////////////////////////
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ACTIVE_USR = activeUser;

    if(activeUser=="")
        QCoreApplication::quit();

    QString userDataPath("./userData");
    QDir userData;
    userData.mkdir(userDataPath);

    QString activeUserDataPath("./userData/"+ACTIVE_USR);
    QDir activeUserData;
    activeUserData.mkdir(activeUserDataPath);

    QString userConversationDir("./userData/"+ACTIVE_USR+"/conversation");
    QDir userConversation;
    userConversation.mkdir(userConversationDir);


    /* Load conversation list*/
    QDir conversationFile("./userData/"+ACTIVE_USR+"/conversation");
    QStringList conversationList;
    foreach(QFileInfo item, conversationFile.entryInfoList()){
        if(item.isFile()){

            conversationList.append(item.fileName());
        }
    }

    /*Add conversation to list.*/
    foreach(QString conversation, conversationList){
        QListWidgetItem *item = new QListWidgetItem;
        item->setIcon(QIcon(":/img/person.png"));
        item->setText(conversation);
        ui->listWidget_Contact->addItem(item);
    }


    /*Connect WidgetListItem on click event*/
    connect(ui->listWidget_Contact, SIGNAL(itemClicked(QListWidgetItem*)),
                this, SLOT(listWidget_Contact_ItemClicked(QListWidgetItem*)));

    /*Filter Enter key press when user typing*/
    ui->plainTextEdit->installEventFilter(this);

    /*Set icon size in listWidget*/
    ui->listWidget_Contact->setIconSize(QSize(50, 50));
    ui->listWidget_Conversation->setIconSize(QSize(50, 50));
    ui->listWidget_Conversation->scrollToBottom();

    currentMenu = "conversation";

    ui->frame_2->hide();

    /*
    ui->listWidget_Conversation->setStyleSheet(
      "QListWidget::item {"
      "border: 2px solid #eeeeee;"
      "padding: 5px;"
      "border-radius: 25px;"
      "background-color: #eeeeee;"
      "}"
      "QListWidget::item:selected {"
         "background-color: red;"
      "}");
       */

}

MainWindow::~MainWindow()
{
    delete ui;
}

//click conversation icon
void MainWindow::on_pushButton_Conversation_clicked()
{
    currentMenu = "conversation";

    ui->frame_2->hide();

    //clear current display list
    ui->listWidget_Contact->clear();

    /* Load conversation list*/
    QDir conversationFile("./userData/"+ACTIVE_USR+"/conversation");
    QStringList conversationList;
    foreach(QFileInfo item, conversationFile.entryInfoList()){
        if(item.isFile()){

            conversationList.append(item.fileName());
        }
    }

    /*Add conversation to list.*/
    foreach(QString conversation, conversationList){
        QListWidgetItem *item = new QListWidgetItem;
        item->setIcon(QIcon(":/img/person.png"));
        item->setText(conversation);
        ui->listWidget_Contact->addItem(item);
    }

    //In case user move splitter hide contacts list and don't know how to show it again.
    //Code below will show contact list when user click this chat buttom.
    QList<int> currentSizes = ui->splitter->sizes();
    if(currentSizes[0]==0){
        currentSizes[0]=1;
        currentSizes[1]=1;
        ui->splitter->setSizes(currentSizes);
    }

    //show add new conversation icon
    ui->pushButton_AddList->setIcon((QIcon(":img/add_conversation.png")));

    //show current clicked icon
    ui->pushButton_Conversation->setIcon(QIcon(":/img/menu_chat_clicked.png"));
    ui->pushButton_Contact->setIcon(QIcon(":/img/menu_contact.png"));
    ui->pushButton_Group->setIcon(QIcon(":/img/menu_group.png"));
    ui->pushButton_Conversation->setIconSize(QSize(70,70));
    ui->pushButton_Contact->setIconSize(QSize(70,70));
    ui->pushButton_Group->setIconSize(QSize(70,70));
}

//click concatact icon
void MainWindow::on_pushButton_Contact_clicked()
{

    currentMenu = "contact";

    ui->frame_2->show();

    //clear current display list
    ui->listWidget_Contact->clear();


    /*Add contact to contact list.*/
    QString Filename = "userData/"+ACTIVE_USR+"/contactList.txt";
    QStringList contactList=Read(Filename);                 //Read user data from file.
    foreach(QString CONTACT, contactList){
        if(CONTACT != ""){
            QListWidgetItem *contact = new QListWidgetItem;
            contact->setIcon(QIcon(":/img/person.png"));
            contact->setText(CONTACT);
            ui->listWidget_Contact->addItem(contact);


        }
    }

    //In case user move splitter hide contacts list and don't know how to show it again.
    //Code below will show contact list when user click this concact buttom.
    QList<int> currentSizes = ui->splitter->sizes();
    if(currentSizes[0]==0){
        currentSizes[0]=1;
        currentSizes[1]=1;
        ui->splitter->setSizes(currentSizes);
    }


    //show add new contact icon
    ui->pushButton_AddList->setIcon((QIcon(":img/add_contact.png")));


    //show current clicked icon
    ui->pushButton_Conversation->setIcon(QIcon(":/img/menu_chat.png"));
    ui->pushButton_Contact->setIcon(QIcon(":/img/menu_contact_clicked.png"));
    ui->pushButton_Group->setIcon(QIcon(":/img/menu_group.png"));
    ui->pushButton_Conversation->setIconSize(QSize(70,70));
    ui->pushButton_Contact->setIconSize(QSize(70,70));
    ui->pushButton_Group->setIconSize(QSize(70,70));
}

void MainWindow::on_pushButton_Group_clicked()
{

    currentMenu = "group";

    ui->frame_2->show();

    //clear current display list
    ui->listWidget_Contact->clear();

    /*Add group to list.*/
    QString Filename = "userData/"+ACTIVE_USR+"/groupList.txt";
    QStringList contactList=Read(Filename);                 //Read user data from file.
    foreach(QString GROUP, contactList){
        QListWidgetItem *group = new QListWidgetItem;
        group->setIcon(QIcon(":/img/person.png"));
        group->setText("GROUP~~"+GROUP);
        ui->listWidget_Contact->addItem(group);
    }

    //In case user move splitter hide contacts list and don't know how to show it again.
    //Code below will show contact list when user click this group buttom.
    QList<int> currentSizes = ui->splitter->sizes();
    if(currentSizes[0]==0){
        currentSizes[0]=1;
        currentSizes[1]=1;
        ui->splitter->setSizes(currentSizes);
    }

    //show add new group icon
    ui->pushButton_AddList->setIcon((QIcon(":img/add_group.png")));

    //show current clicked icon
    ui->pushButton_Conversation->setIcon(QIcon(":/img/menu_chat.png"));
    ui->pushButton_Contact->setIcon(QIcon(":/img/menu_contact.png"));
    ui->pushButton_Group->setIcon(QIcon(":/img/menu_group_clicked.png"));
    ui->pushButton_Conversation->setIconSize(QSize(70,70));
    ui->pushButton_Contact->setIconSize(QSize(70,70));
    ui->pushButton_Group->setIconSize(QSize(70,70));
}

void MainWindow::on_pushButton_AddList_clicked()
{
    if(currentMenu=="conversation"){

    }
    else if(currentMenu=="contact"){

        AddContact addcontact(this, ACTIVE_USR);
        addcontact.setModal(false);
        addcontact.exec();

        /* Reload contact list*/
        on_pushButton_Contact_clicked();


    }
    else if(currentMenu=="group"){

        CreateGroup createGroup(this, ACTIVE_USR);
        createGroup.setModal(false);
        createGroup.exec();

        /* Reload contact list*/
        on_pushButton_Group_clicked();


    }
}

/*When user click item on contact list.*/
void MainWindow::listWidget_Contact_ItemClicked(QListWidgetItem* item){
    ui->label_ConversationWith->setText(item->text());
    ui->listWidget_Conversation->clear();
    conversationWith = ui->label_ConversationWith->text();

    /*Load conversation*/
    QString filename="./userData/"+ACTIVE_USR+"/conversation/"+item->text();
    QStringList conversation=ReadConversation(filename);

    /*Show conversation*/
    foreach(QString msg, conversation){
        QListWidgetItem *item = new QListWidgetItem;
        if(msg.left(ACTIVE_USR.length()+1) == ACTIVE_USR+":"){
            msg = msg.remove(ACTIVE_USR+": ");
            item->setText(msg);
            item->setTextAlignment(2);
        }
        else{
            msg = msg.remove(conversationWith+": ");
            item->setIcon(QIcon(":/img/person.png"));
            item->setText(msg);
        }

        ui->listWidget_Conversation->addItem(item);
    }

    ui->listWidget_Conversation->scrollToBottom();
}

/*When user click SEND button*/
void MainWindow::on_pushButton_SEND_clicked()
{


    QString conversationWith = ui->label_ConversationWith->text();
    QString msg = ui->plainTextEdit->toPlainText();
    QString Filename="./userData/"+ACTIVE_USR+"/conversation/"+conversationWith;

    if((conversationWith != "") && (msg != "") ){
        ui->listWidget_Conversation->clear();
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

            msg=ACTIVE_USR+": "+msg+"\n";
            QTextStream out(&File);
            out << msg;

            File.flush();
            File.close();
        }

        /*Load conversation*/
        QString filename="./userData/"+ACTIVE_USR+"/conversation/"+conversationWith;
        QStringList conversation=ReadConversation(filename);

        /*Show conversation*/
        foreach(QString msg, conversation){
            QListWidgetItem *item = new QListWidgetItem;
            if(msg.left(ACTIVE_USR.length()+1) == ACTIVE_USR+":"){
                msg = msg.remove(ACTIVE_USR+": ");
                item->setText(msg);
                item->setTextAlignment(2);
            }
            else{
                msg = msg.remove(conversationWith+": ");
                item->setIcon(QIcon(":/img/person.png"));
                item->setText(msg);
            }

            ui->listWidget_Conversation->addItem(item);
        }

        ui->listWidget_Conversation->scrollToBottom();
    }

    ui->plainTextEdit->clear();
}

/*Enter key filter when user typing*/
bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if(watched == ui->plainTextEdit){
        if(event->type() == QKeyEvent::KeyPress ||
                event->type() == QKeyEvent::KeyRelease){
            QKeyEvent * KEY = static_cast<QKeyEvent*>(event);

            if((KEY->key()==Qt::Key_Return ||
                KEY->key()==Qt::Key_Enter) &&
                    KEY->modifiers()==Qt::ShiftModifier){

                if(event->type()==QKeyEvent::KeyRelease){
                    return true;
                }
            }
            else if(KEY->key()==Qt::Key_Return ||
                    KEY->key()==Qt::Key_Enter){

                        on_pushButton_SEND_clicked();
                        return true;
           }

        }
        return false;
    }
    return false;

}
