#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "addcontact.h"
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
using namespace std;

QString currentMenu;

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
         line= in.readLine();
         List.append(line);
    }

    File.flush();
    File.close();
    return List;
}




MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QString userDataPath("./userData");
    QDir userData;
    userData.mkdir(userDataPath);


    /*Add conversation to list.*/
    QString Filename = "userData/conversationList.txt";
    QStringList contactList=Read(Filename);                 //Read user data from file.
    foreach(QString CONTACT, contactList){
        QListWidgetItem *item = new QListWidgetItem;
        item->setIcon(QIcon(":/img/person.png"));
        item->setText("Chat with " + CONTACT);
        ui->listWidget_Contact->addItem(item);
    }


    for(int i=0; i<=40; i++){
        QListWidgetItem *newItemx = new QListWidgetItem;
        newItemx->setIcon(QIcon(":/img/person.png"));
        newItemx->setText("Can you hear me? "+QString::number(i));
        ui->listWidget_Conversation->addItem(newItemx);
    }


    QListWidgetItem *newItemx = new QListWidgetItem;
    //newItemx->setIcon(QIcon(":/img/person.png"));
    newItemx->setText("Yes, I can hear you but I don't know who you are");
    newItemx->setTextAlignment(2);
    ui->listWidget_Conversation->addItem(newItemx);

    /*Set icon size in listWidget*/
    ui->listWidget_Contact->setIconSize(QSize(50, 50));
    ui->listWidget_Conversation->setIconSize(QSize(50, 50));
    ui->listWidget_Conversation->scrollToBottom();

    currentMenu = "conversation";


    /*Connect List items event on click*/
    connect(ui->listWidget_Contact, SIGNAL(itemClicked(QListWidgetItem*)),
                this, SLOT(listWidget_Contact_ItemClicked(QListWidgetItem*)));

}

MainWindow::~MainWindow()
{
    delete ui;
}

//click conversation icon
void MainWindow::on_pushButton_Conversation_clicked()
{
    currentMenu = "conversation";

    //clear current display list
    ui->listWidget_Contact->clear();


    /*Add conversation to list.*/
    QString Filename = "userData/conversationList.txt";
    QStringList contactList=Read(Filename);                 //Read user data from file.
    foreach(QString CONTACT, contactList){
        QListWidgetItem *item = new QListWidgetItem;
        item->setIcon(QIcon(":/img/person.png"));
        item->setText("Chat with " + CONTACT);
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

    //clear current display list
    ui->listWidget_Contact->clear();


    /*Add contact to contact list.*/
    QString Filename = "userData/contactList.txt";
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
    //clear current display list
    ui->listWidget_Contact->clear();

    /*Add group to list.*/
    QString Filename = "userData/groupList.txt";
    QStringList contactList=Read(Filename);                 //Read user data from file.
    foreach(QString GROUP, contactList){
        QListWidgetItem *group = new QListWidgetItem;
        group->setIcon(QIcon(":/img/person.png"));
        group->setText(GROUP);
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
        qDebug() << "add new conversation";
    }
    else if(currentMenu=="contact"){
        qDebug() << "add new contact";
        AddContact addcontact;
        addcontact.setModal(false);
        addcontact.exec();

        /* Reload contact list*/

        ui->listWidget_Contact->clear();                        //clear current display list
        QString Filename = "userData/contactList.txt";
        QStringList contactList=Read(Filename);                 //Read user data from file.
        foreach(QString CONTACT, contactList){
            if(CONTACT != ""){
                QListWidgetItem *contact = new QListWidgetItem;
                contact->setIcon(QIcon(":/img/person.png"));
                contact->setText(CONTACT);
                ui->listWidget_Contact->addItem(contact);
            }
        }

    }
    else if(currentMenu=="group"){
        qDebug() << "add new group";
    }
}

void MainWindow::listWidget_Contact_ItemClicked(QListWidgetItem* item){
    qDebug() << item->text();
    ui->label_ConversationWith->setText(item->text());
    ui->listWidget_Conversation->clear();

    ui->listWidget_Conversation->scrollToBottom();
}
