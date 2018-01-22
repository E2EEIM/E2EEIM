#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <algorithm>
#include <string>
#include <QDesktopWidget>
#include <QMainWindow>
using namespace std;
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

            int recentAct[]={3,4,2,1,5};
    for(int i=0; i<5; i++){
        QListWidgetItem *contact = new QListWidgetItem;
        contact->setIcon(QIcon(":/img/icon2.png"));
        contact->setText("Friend "+QString::number(recentAct[i]));
        ui->listWidget_Recent->addItem(contact);
    }


    string AtoZContacts[]={"3","4","2","1","5"};
    sort(AtoZContacts, AtoZContacts + (sizeof(AtoZContacts)/sizeof(string)));
    for(int i=0; i<5; i++){
        QListWidgetItem *contact = new QListWidgetItem;
        contact->setIcon(QIcon(":/img/icon2.png"));
        contact->setText("Friend "+QString::fromStdString(AtoZContacts[i]));
        ui->listWidget_SortedContact->addItem(contact);
    }

    int conversationList[]={3,4};
    for(int i=0; i<2; i++){
        QListWidgetItem *conversation = new QListWidgetItem;
        conversation->setIcon(QIcon(":/img/icon2.png"));
        conversation->setText("Conversation with friend "+QString::number(conversationList[i]));
        ui->listWidget_ChatList->addItem(conversation);
    }


    for(int i=0; i<=40; i++){
        QListWidgetItem *newItemx = new QListWidgetItem;
        newItemx->setIcon(QIcon(":/img/icon1.png"));
        newItemx->setText("Can you hear me? "+QString::number(i));
        ui->listWidget_Conversation->addItem(newItemx);
    }


    QListWidgetItem *newItemx = new QListWidgetItem;
    //newItemx->setIcon(QIcon(":/img/icon1.png"));
    newItemx->setText("Yes, I can hear you but I don't know who you are");
    newItemx->setTextAlignment(2);
    ui->listWidget_Conversation->addItem(newItemx);


    ui->listWidget_Recent->setIconSize(QSize(50, 50));
    ui->listWidget_SortedContact->setIconSize(QSize(50, 50));
    ui->listWidget_ChatList->setIconSize(QSize(50, 50));
    ui->listWidget_Conversation->setIconSize(QSize(40, 40));
    ui->listWidget_Conversation->scrollToBottom();

}

MainWindow::~MainWindow()
{
    delete ui;
}
