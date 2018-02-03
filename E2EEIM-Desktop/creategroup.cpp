#include "creategroup.h"
#include "ui_creategroup.h"
#include <QFile>
#include <QDebug>
#include <QTextStream>
#include <QStringList>
#include <QString>
#include <QList>

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

CreateGroup::CreateGroup(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CreateGroup)
{
    ui->setupUi(this);

    ui->listWidget->clear();                                //clear current display list
    QString Filename = "userData/contactList.txt";
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

    if(groupName==""){
        ui->label->setText("Please enter group name.");
    }
    else{
        foreach(QString item, GROUP) {
            QString Filename = "userData/groupList.txt";
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
        CreateGroup::close();
    }

}
