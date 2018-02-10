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
#include "creategroup.h"
#include "ui_creategroup.h"


QString ACTIVE_USER;

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

CreateGroup::CreateGroup(QWidget *parent, QString activeUser) :
    QDialog(parent),
    ui(new Ui::CreateGroup)
{
    ui->setupUi(this);

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
        CreateGroup::close();
    }

}
