#include "signin.h"
#include "ui_signin.h"
#include <QDebug>

SignIn::SignIn(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SignIn)
{
    ui->setupUi(this);
}

SignIn::~SignIn()
{
    delete ui;
}

void SignIn::on_pushButton_clicked()
{
    QString activeUser=ui->lineEdit->text();
    //qDebug() << activeUser;
    if(activeUser!=""){
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
