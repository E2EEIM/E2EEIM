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
