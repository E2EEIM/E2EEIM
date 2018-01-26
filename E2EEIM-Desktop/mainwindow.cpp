#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <qlayout.h>
#include <algorithm>
#include <string>
#include <QDesktopWidget>
#include <QMainWindow>
#include <QWidget>
using namespace std;
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    /*Add contact to contact list.*/
    string AtoZContacts[]={"3","4","2","1","5"};
    int ContactArrayLength=(sizeof(AtoZContacts)/(sizeof(AtoZContacts[0])));
    sort(AtoZContacts, AtoZContacts + (sizeof(AtoZContacts)/sizeof(string)));
    for(int i=0; i<ContactArrayLength; i++){
        QListWidgetItem *contact = new QListWidgetItem;
        contact->setIcon(QIcon(":/img/person.png"));
        contact->setText("Chat with Friend "+QString::fromStdString(AtoZContacts[i]));
        ui->listWidget_Contact->addItem(contact);
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

}

MainWindow::~MainWindow()
{
    delete ui;
}

//click conversation icon
void MainWindow::on_pushButton_Conversation_clicked()
{
    //clear current display list
    ui->listWidget_Contact->clear();


    /*Add contact to contact list.*/
    string AtoZContacts[]={"3","4"};
    int arrayLength=(sizeof(AtoZContacts)/(sizeof(AtoZContacts[0])));
    //sort(AtoZContacts, AtoZContacts + (sizeof(AtoZContacts)/sizeof(string)));
    for(int i=0; i<arrayLength; i++){
        QListWidgetItem *contact = new QListWidgetItem;
        contact->setIcon(QIcon(":/img/person.png"));
        contact->setText("Chat with Friend "+QString::fromStdString(AtoZContacts[i]));
        ui->listWidget_Contact->addItem(contact);
    }

    //In case user move splitter hide contacts list and don't know how to show it again.
    //Code below will show contact list when user click this chat buttom.
    QList<int> currentSizes = ui->splitter->sizes();
    if(currentSizes[0]==0){
        currentSizes[0]=1;
        currentSizes[1]=1;
        ui->splitter->setSizes(currentSizes);
    }

    //show current clicked icon
    ui->pushButton_Conversation->setIcon(QIcon(":/img/clicked.png"));
    ui->pushButton_Contact->setIcon(QIcon(":/img/person.png"));
    ui->pushButton_Group->setIcon(QIcon(":/img/user.svg"));
    ui->pushButton_Conversation->setIconSize(QSize(70,70));
    ui->pushButton_Contact->setIconSize(QSize(70,70));
    ui->pushButton_Group->setIconSize(QSize(70,70));
}

//click concatact icon
void MainWindow::on_pushButton_Contact_clicked()
{
    //clear current display list
    ui->listWidget_Contact->clear();


    /*Add contact to contact list.*/
    string AtoZContacts[]={"3","4","2","1","5"};
    int arrayLength=(sizeof(AtoZContacts)/(sizeof(AtoZContacts[0])));
    sort(AtoZContacts, AtoZContacts + (sizeof(AtoZContacts)/sizeof(string)));
    for(int i=0; i<arrayLength; i++){
        QListWidgetItem *contact = new QListWidgetItem;
        contact->setIcon(QIcon(":/img/person.png"));
        contact->setText("Friend "+QString::fromStdString(AtoZContacts[i]));
        ui->listWidget_Contact->addItem(contact);
    }

    //In case user move splitter hide contacts list and don't know how to show it again.
    //Code below will show contact list when user click this concact buttom.
    QList<int> currentSizes = ui->splitter->sizes();
    if(currentSizes[0]==0){
        currentSizes[0]=1;
        currentSizes[1]=1;
        ui->splitter->setSizes(currentSizes);
    }

    //show current clicked icon
    ui->pushButton_Conversation->setIcon(QIcon(":/img/conversation.png"));
    ui->pushButton_Contact->setIcon(QIcon(":/img/clicked.png"));
    ui->pushButton_Group->setIcon(QIcon(":/img/user.svg"));
    ui->pushButton_Conversation->setIconSize(QSize(70,70));
    ui->pushButton_Contact->setIconSize(QSize(70,70));
    ui->pushButton_Group->setIconSize(QSize(70,70));
}

void MainWindow::on_pushButton_Group_clicked()
{
    //clear current display list
    ui->listWidget_Contact->clear();


    /*Add group to contact list.*/
    string AtoZContacts[]={"3","4","2"};
    int arrayLength=(sizeof(AtoZContacts)/(sizeof(AtoZContacts[0])));
    sort(AtoZContacts, AtoZContacts + (sizeof(AtoZContacts)/sizeof(string)));
    for(int i=0; i<arrayLength; i++){
        QListWidgetItem *contact = new QListWidgetItem;
        contact->setIcon(QIcon(":/img/person.png"));
        contact->setText("Group "+QString::fromStdString(AtoZContacts[i]));
        ui->listWidget_Contact->addItem(contact);
    }

    //In case user move splitter hide contacts list and don't know how to show it again.
    //Code below will show contact list when user click this group buttom.
    QList<int> currentSizes = ui->splitter->sizes();
    if(currentSizes[0]==0){
        currentSizes[0]=1;
        currentSizes[1]=1;
        ui->splitter->setSizes(currentSizes);
    }


    //show current clicked icon
    ui->pushButton_Conversation->setIcon(QIcon(":/img/conversation.png"));
    ui->pushButton_Contact->setIcon(QIcon(":/img/person.png"));
    ui->pushButton_Group->setIcon(QIcon(":/img/clicked.png"));
    ui->pushButton_Conversation->setIconSize(QSize(70,70));
    ui->pushButton_Contact->setIconSize(QSize(70,70));
    ui->pushButton_Group->setIconSize(QSize(70,70));
}
