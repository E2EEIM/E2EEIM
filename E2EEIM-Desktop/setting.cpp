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

#include "setting.h"
#include "ui_setting.h"


QStringList Setting::readContact(QString Filename){
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
        if(Filename=="userData/"+ActiveUser+"/groupList.txt"){
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



Setting::Setting(QString activeUser, QString currentMenu, QString currentConversation, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Setting)
{
    ui->setupUi(this);
    ActiveUser=activeUser;
    ui->label->setText(activeUser);

    this->currentConversation=currentConversation;
    currentConversationStatus="notDelete";

    ui->pushButton_Delete->setEnabled(false);
    ui->pushButton_Leave->setEnabled(false);
    ui->pushButton_DeleteConversation->setEnabled(false);

    QString Filename = "userData/"+ActiveUser+"/contactList.txt";
    QStringList contactList=readContact(Filename);
    foreach(QString CONTACT, contactList){
        if(CONTACT != ""){
            QListWidgetItem *contact = new QListWidgetItem;
            contact->setIcon(QIcon(":/img/icons/person.png"));
            contact->setText(CONTACT);
            ui->listWidget_ContactList->addItem(contact);


        }
    }

    QString GroupFilename = "userData/"+ActiveUser+"/groupList.txt";
    QStringList groupList=readContact(GroupFilename);
    foreach(QString GROUP, groupList){
        QListWidgetItem *group = new QListWidgetItem;
        group->setIcon(QIcon(":/img/icons/person.png"));
        group->setText("GROUP~~"+GROUP);
        ui->listWidget_GroupList->addItem(group);
    }


    /* Load conversation list*/
    QDir conversationFile("./userData/"+ActiveUser+"/conversation");
    QStringList conversationList;
    foreach(QFileInfo item, conversationFile.entryInfoList()){
        if(item.isFile()){

            conversationList.append(item.fileName());
        }
    }

    /*Add conversation to list.*/
    foreach(QString conversation, conversationList){
        QListWidgetItem *item = new QListWidgetItem;
        item->setIcon(QIcon(":/img/icons/person.png"));
        item->setText(conversation);
        ui->listWidget_ConversationList->addItem(item);
    }



    /*Connect WidgetListItem on click event*/
    connect(ui->listWidget_ConversationList, SIGNAL(itemClicked(QListWidgetItem*)),
                this, SLOT(listWidget_Contact_ItemClicked(QListWidgetItem*)));
    connect(ui->listWidget_ContactList, SIGNAL(itemClicked(QListWidgetItem*)),
                this, SLOT(listWidget_Contact_ItemClicked(QListWidgetItem*)));
    connect(ui->listWidget_GroupList, SIGNAL(itemClicked(QListWidgetItem*)),
                this, SLOT(listWidget_Contact_ItemClicked(QListWidgetItem*)));
    connect(ui->tabWidget, SIGNAL(currentChanged(int)), this, SLOT(tabSelected()));

    currentTab="Account";

    if(currentMenu=="conversation"){
        ui->tabWidget->setCurrentIndex(1);
    }
    else if(currentMenu=="contact"){
        ui->tabWidget->setCurrentIndex(2);
    }
    else if(currentMenu=="group"){
        ui->tabWidget->setCurrentIndex(3);
    }

}
void Setting::tabSelected(){
    if(ui->tabWidget->currentIndex()==0){
        currentTab="Account";

    }
    else if(ui->tabWidget->currentIndex()==1){
        currentTab="Conversation";

    }
    else if(ui->tabWidget->currentIndex()==2){
        currentTab="Contact";

    }
    else if(ui->tabWidget->currentIndex()==3){
        currentTab="Group";

    }
}

void Setting::listWidget_Contact_ItemClicked(QListWidgetItem* item){

    QString selectedItem = item->text();

    QStringList selectedConversation;
    QStringList selectedContact;
    QStringList selectedGroup;

    QList <QListWidgetItem*> SELECTED_CONVERSATION = ui->listWidget_ConversationList->selectedItems();
    QList <QListWidgetItem*> SELECTED_CONTACT = ui->listWidget_ContactList->selectedItems();
    QList <QListWidgetItem*> SELECTED_GROUP = ui->listWidget_GroupList->selectedItems();

    if(SELECTED_CONVERSATION.isEmpty()==true){
        ui->pushButton_DeleteConversation->setEnabled(false);
    }
    else{
        ui->pushButton_DeleteConversation->setEnabled(true);
    }

    if(SELECTED_CONTACT.isEmpty()==true){
        ui->pushButton_Delete->setEnabled(false);
    }
    else{
        ui->pushButton_Delete->setEnabled(true);

    }

    if(SELECTED_GROUP.isEmpty()==true){
        ui->pushButton_Leave->setEnabled(false);
    }
    else{
        ui->pushButton_Leave->setEnabled(true);

    }

    if(currentTab=="Conversation"){
        selectedContact.empty();
        selectedGroup.empty();
        foreach( QListWidgetItem *conversation, SELECTED_CONVERSATION){
            selectedConversation.append(conversation->text());
        }
        deleteList=selectedConversation;
        deleteInFile="userData/"+ActiveUser+"/conversation";
    }
    else if(currentTab=="Contact"){
        selectedGroup.empty();
        selectedConversation.empty();
        foreach( QListWidgetItem *contact, SELECTED_CONTACT){
            selectedContact.append(contact->text());
        }
        deleteList=selectedContact;
        deleteInFile="userData/"+ActiveUser+"/contactList.txt";
    }
    else if(currentTab=="Group"){
        selectedContact.empty();
        selectedConversation.empty();
        foreach( QListWidgetItem *group, SELECTED_GROUP){
            selectedGroup.append(group->text());
        }
        deleteList=selectedGroup;
        deleteInFile="userData/"+ActiveUser+"/groupList.txt";

    }
}

Setting::~Setting()
{
    delete ui;
}

void Setting::on_pushButton_DeleteConversation_clicked()
{
    qDebug() << deleteList.isEmpty();
    if(deleteList.isEmpty()==false)
        deleteItem(deleteList, deleteInFile);
    ui->pushButton_DeleteConversation->setEnabled(false);
}

void Setting::on_pushButton_Delete_clicked()
{
    if(deleteList.isEmpty()==false)
        deleteItem(deleteList, deleteInFile);
    ui->pushButton_Delete->setEnabled(false);
}

void Setting::on_pushButton_Leave_clicked()
{
    if(deleteList.isEmpty()==false)
        deleteItem(deleteList, deleteInFile);
    ui->pushButton_Leave->setEnabled(false);
}

QString Setting::getCurrentConversationStatus(){

    QString status=currentConversationStatus;
    return status;
}

void Setting::deleteItem(QStringList deleteList, QString fileName){
    QStringList remain;
    QFile File(fileName);

    if(fileName=="userData/"+ActiveUser+"/conversation"){
        foreach(QString item, deleteList){
            qDebug() << item;
            QFile file(fileName+"/"+item);
            file.remove();

            if(item==currentConversation){
                currentConversationStatus="deleted";

            }
        }
        ui->listWidget_ConversationList->clear();
        /* Load conversation list*/
        QDir conversationFile("./userData/"+ActiveUser+"/conversation");
        QStringList conversationList;
        foreach(QFileInfo item, conversationFile.entryInfoList()){
            if(item.isFile()){

                conversationList.append(item.fileName());
            }
        }

        /*Add conversation to list.*/
        foreach(QString conversation, conversationList){
            QListWidgetItem *item = new QListWidgetItem;
            item->setIcon(QIcon(":/img/icons/person.png"));
            item->setText(conversation);
            ui->listWidget_ConversationList->addItem(item);
        }
    }
    else{
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
        bool foundTargetGroup=false;
        while( !in.atEnd())
        {
             if(fileName=="userData/"+ActiveUser+"/groupList.txt"){
                 line=in.readLine();
                 if(line.left(2)!="~~"){
                    if(foundTargetGroup==false){
                         remain.append(line);
                        continue;
                     }
                 }
                 else{
                     foreach(QString item, deleteList){
                         if(line.split("~~").at(1) == item.split("~~").last()){
                             foundTargetGroup=true;
                         }
                         else{
                             foundTargetGroup=false;
                             remain.append(line);
                         }
                     }
                 }
            }
             else{ // Delete from contactList.txt

                 line=in.readLine();
                 remain.append(line);

                 /*
                 line=in.readLine();
                 foreach(QString item, deleteList){
                     if(line!=item){
                         remain.append(line);
                     }
                 }
                 */
             }
        }
        File.flush();
        File.close();

        if(fileName=="userData/"+ActiveUser+"/contactList.txt"){
            foreach (QString item, deleteList) {
               int idx=remain.indexOf(item);
               remain.removeAt(idx);
            }
        }

        if(remain.isEmpty()==true){
            remain.append("\n");
        }

        if(!File.open(QFile::WriteOnly | QFile::Text)){
            qDebug() << "cound not open file for writing";
            abort();
        }

        QTextStream out(&File);
        foreach(QString item, remain) {
            //qDebug() << "item=" << item;
            out << item+"\n";
        }

        File.flush();
        File.close();


        if(fileName=="userData/"+ActiveUser+"/groupList.txt"){
            ui->listWidget_GroupList->clear();
            QString GroupFilename = "userData/"+ActiveUser+"/groupList.txt";
            QStringList groupList=readContact(GroupFilename);
           foreach(QString GROUP, groupList){
                QListWidgetItem *group = new QListWidgetItem;
                group->setIcon(QIcon(":/img/icons/person.png"));
                group->setText("GROUP~~"+GROUP);
                ui->listWidget_GroupList->addItem(group);
            }

        }
        else{
            ui->listWidget_ContactList->clear();
            QString Filename = "userData/"+ActiveUser+"/contactList.txt";
            QStringList contactList=readContact(Filename);
            foreach(QString CONTACT, contactList){
                if(CONTACT != ""){
                    QListWidgetItem *contact = new QListWidgetItem;
                    contact->setIcon(QIcon(":/img/icons/person.png"));
                    contact->setText(CONTACT);
                    ui->listWidget_ContactList->addItem(contact);
                }
            }
        }
    }
}




void Setting::on_pushButton_clicked()
{
    Setting::accept();
}
