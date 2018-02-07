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
#include "addcontact.h"
#include "ui_addcontact.h"
#include <QDebug>
#include <QFile>
#include <QTextStream>

QString ACTIVE_USER2;

AddContact::AddContact(QWidget *parent, QString activeUser) :
    QDialog(parent),
    ui(new Ui::AddContact)
{
    ui->setupUi(this);
    ACTIVE_USER2=activeUser;
}

AddContact::~AddContact()
{
    delete ui;
}

void AddContact::on_pushButton_clicked()
{
    QString ContactName=ui->lineEdit->text();
    QString Filename = "userData/"+ACTIVE_USER2+"/contactList.txt";

    if(ContactName != ""){
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
            out << ContactName+"\n";

            File.flush();
            File.close();

            AddContact::close();
     }
    }


}
