#include <QCoreApplication>
#include <QDebug>
#include "connection.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    Connection connection;
    connection.letConnect();

    // *1->  Connection request /////////////////////////////////////////////////////////////////
    QChar operation(1);
    QByteArray data="00000001";
    data.append(operation.toLatin1());
    connection.send(data);



    // *3-> Send sign up protocal decrypt([username|userPublicKey]) //////////////////////////////
    QByteArray ActiveUserPubKey = connection.getActiveUserPubKey();
    QString ActiveUserName=connection.getActiveUserName();
    data.clear();
    data.append(ActiveUserName);            //Add active-user's username to QByteArray
    data.append(ActiveUserPubKey);          //Add active-user's public key to after username;
    int aupk_size=ActiveUserPubKey.size();  //Get active-user's public size;
    int auNameSize=ActiveUserName.size();   //Get active-user's username length;


    //Insert active-user's public key size to front of QByteArray
    QByteArray temp;
    temp.setNum(aupk_size, 16);
    while(temp.size()<8){
        temp.insert(0,"0");
    }
    data.insert(0, temp);

    //Insert active-user's username length to front of QByteArray
    QChar usernameSize(auNameSize);
    data.insert(0, usernameSize.toLatin1());
    temp.clear();
    temp=connection.encryptToServ(data, "signUp.pgp");
    data.clear();
    data=temp;
    //create payload section finished

    //create main *3->(sign up) protocal
    temp.clear();
    temp.setNum((data.size()+1), 16); // Add payload size

    while(temp.size()<8){
        temp.insert(0,"0");
    }
    temp.append((char)3);             // Add operation

    temp.append(data);
    data.clear();
    data=temp;

    connection.send(data);














    //return 0;
    return a.exec();
}
