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
#include "signin.h"
#include "ui_signin.h"
#include <QDebug>

SignIn::SignIn(Connection &conn, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SignIn)
{
    ui->setupUi(this);

    connection = &conn;
}

SignIn::~SignIn()
{
    delete ui;
}

void SignIn::on_pushButton_clicked()
{
    QString activeUser=ui->lineEdit->text();
    if(activeUser!=""){

        connection->connect();
        connection->send(activeUser);
        //connection->letDisconnect();


        SignIn::accept();
    }
    else{
        ui->label->setText("\nUsername can not be empty!");
    }
}
QString SignIn::getActiveUser(){
    QString ACTIVE_USER { ui->lineEdit->text() };
    return ACTIVE_USER;
}


