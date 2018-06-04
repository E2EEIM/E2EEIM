/****************************************************************************
** Copyright (C) 2018  Krittitam Juksukit
** This file is part of E2EEIM.
**
** E2EEIM is free software: you can redistribute it and/or modify
** it under the terms of the GNU Lesser General Public License
** version 3 as published by the Free Software Foundation.
**
** E2EEIM is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU Lesser General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with E2EEIM.  If not, see <http://www.gnu.org/licenses/>.
**
*******************************************************************************/

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "signin.h"

using namespace std;



MainWindow::MainWindow(Connection &conn, Encryption &encryption, QWidget *parent) : // /////////////////////////////////////////////////////////////
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    this->conn=&conn;
    this->encryption=&encryption;

    connect(this->conn, SIGNAL(receiveAddFriendrequest(QByteArray)), this, SLOT(receiveAddFriendRequest(QByteArray)), Qt::QueuedConnection);
    connect(this->conn, SIGNAL(receiveNewPublicKey(QByteArray)), this, SLOT(receiveNewPublicKey(QByteArray)), Qt::QueuedConnection);

    signOut();

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::initUserDataPath(){

    qDebug() << "ACTIVE_USR" << ACTIVE_USR;
    QString userDataPath("./userData");
    QDir userData;
    userData.mkdir(userDataPath);

    QString activeUserDataPath("./userData/"+ACTIVE_USR);
    QDir activeUserData;
    activeUserData.mkdir(activeUserDataPath);

    QString userConversationDir("./userData/"+ACTIVE_USR+"/conversation");
    QDir userConversation;
    userConversation.mkdir(userConversationDir);

    /* Load conversation list*/
    QDir conversationFile("./userData/"+ACTIVE_USR+"/conversation");
    QStringList conversationList;
    foreach(QFileInfo item, conversationFile.entryInfoList()){
        if(item.isFile()){

            conversationList.append(item.fileName());
        }
    }

    /*Add conversation to list.*/
    foreach(QString conversation, conversationList){
        QListWidgetItem *item = new QListWidgetItem;
        item->setIcon(QIcon(":/img/icons/person.png"));
        item->setText(conversation);
        ui->listWidget_Contact->addItem(item);
    }


    /*Connect WidgetListItem on click event*/
    connect(ui->listWidget_Contact, SIGNAL(itemClicked(QListWidgetItem*)),
                this, SLOT(listWidget_Contact_ItemClicked(QListWidgetItem*)));

    /*Connect combobox on click event*/
    connect(ui->comboBox, SIGNAL(currentTextChanged(QString)),
                this, SLOT(textMenuChange()));

    /*Filter Enter key press when user typing*/
    ui->plainTextEdit->installEventFilter(this);

    /*Set icon size in listWidget*/
    ui->listWidget_Contact->setIconSize(QSize(50, 50));
    ui->listWidget_Conversation->setIconSize(QSize(50, 50));
    ui->listWidget_Conversation->scrollToBottom();

    currentMenu = "conversation";

    ui->frame_2->hide();
    ui->frame_addFriend_confirm->hide();
    ui->frame_3->hide();

    finishInitUserDataStatus=true;

}

QStringList MainWindow::getGroupMember(QString GroupName){
    QFile File("./userData/"+ACTIVE_USR+"/groupList.txt");
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
    short flagLoad=0;
    while(!in.atEnd())
    {
            line=in.readLine();
            if(line.left(2)=="~~"){
                if((line.split("~~").at(1)) == GroupName){
                    flagLoad=1;
                    continue;
                }
                else if(flagLoad==1)
                    break;
            }
            if(flagLoad!=0){
                List.append(line);

            }
    }
    return List;
}

// ///////////////////////////////////////READ CONTACT LIST
QStringList MainWindow::readTextLine(QString Filename){
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
        if(Filename=="./userData/"+ACTIVE_USR+"/groupList.txt"){
            line=in.readLine();
            if(line.left(2)!="~~")
                continue;
            List.append(line.split("~~").at(1));
        }
        else{
            line= in.readLine();
            List.append(line);
        }
    }

    File.flush();
    File.close();
    return List;
}


QStringList MainWindow::ReadConversation(QString Filename){

    int ACTIVE_USR_length;
    int conversationWith_length;

    ACTIVE_USR_length=ACTIVE_USR.length();
    conversationWith_length=conversationWith.length();

    QFile File(Filename);
    QStringList List;
    if(!File.exists()){
        if(!File.open(QFile::WriteOnly | QFile::Text)){
            qDebug() << "Cound not open file for writing";
            abort();
        }
        QTextStream out(&File);
        out << "";

        File.flush();
        File.close();
    }


    if(!File.open(QFile::ReadOnly | QFile::Text)){
        qDebug() << "Cound not open file for Read";
        abort();
    }

    QTextStream in(&File);
    QString line;
    QString tmp="[-STaRT-]";

    if(Filename.split("~~").first()=="./userData/"+ACTIVE_USR+"/conversation/GROUP"){ //In case group conversation;
        QStringList groupMember=getGroupMember(Filename.split("~~").last());
        while( !in.atEnd())
        {
            line=in.readLine();
            int flagFistLine=1;
            if(line.left(ACTIVE_USR_length+1)!=(ACTIVE_USR+":")){
                flagFistLine=0;
                foreach(QString memberName, groupMember){
                    if(line.left(memberName.length()+1)==(memberName+":")){
                        flagFistLine=1;
                        break;
                    }
                }
            }
            if(flagFistLine==0){
                tmp=tmp+"\n"+line;
                if(in.atEnd())
                    List.append(tmp);
                continue;
            }
            if(tmp!="[-STaRT-]"){
                List.append(tmp);
            }
            if(in.atEnd()){
                List.append(line);
            }
            tmp=line;
        }

    }
    else{
        while( !in.atEnd())
        {
            line= in.readLine();
            if((line.left(ACTIVE_USR_length+1)!=(ACTIVE_USR+":")) &&
                    (line.left(conversationWith_length+1)!=(conversationWith+":"))){
                tmp=tmp+"\n"+line;
                if(in.atEnd())
                    List.append(tmp);
                continue;
            }
            if(tmp!="[-STaRT-]"){
                List.append(tmp);
            }
            if(in.atEnd()){
                List.append(line);
            }
             tmp=line;
        }
    }

    File.flush();
    File.close();
    return List;
}


//click conversation icon
void MainWindow::on_pushButton_Conversation_clicked()
{
    currentMenu = "conversation";

    ui->frame_2->hide();


    //clear current display list
    ui->listWidget_Contact->clear();

    /* Load conversation list*/
    QDir conversationFile("./userData/"+ACTIVE_USR+"/conversation");
    QStringList conversationList;
    foreach(QFileInfo item, conversationFile.entryInfoList()){
        if(item.isFile()){

            conversationList.append(item.fileName());
        }
    }

    /*Add conversation to list.*/
    foreach(QString conversation, conversationList){
        QListWidgetItem *item = new QListWidgetItem;
        item->setIcon(QIcon(":/img/icons/person.png"));
        item->setText(conversation);
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
    ui->pushButton_AddList->setIcon((QIcon(":img/icons/add_conversation.png")));

    //show current clicked icon
    ui->pushButton_Conversation->setIcon(QIcon(":/img/icons/menu_chat_clicked.png"));
    if(anyNewContact==true){
        ui->pushButton_Contact->setIcon(QIcon(":/img/icons/menu_contact_normal_noti.png"));
    }
    else{
        ui->pushButton_Contact->setIcon(QIcon(":/img/icons/menu_contact.png"));
    }
    ui->pushButton_Group->setIcon(QIcon(":/img/icons/menu_group.png"));
    ui->pushButton_Conversation->setIconSize(QSize(70,70));
    ui->pushButton_Contact->setIconSize(QSize(70,70));
    ui->pushButton_Group->setIconSize(QSize(70,70));


}

//click concatact icon
void MainWindow::on_pushButton_Contact_clicked()
{

    currentMenu = "contact";

    ui->frame_2->show();

    //clear current display list
    ui->listWidget_Contact->clear();


    /*Add contact to contact list.*/
    QString Filename = "./userData/"+ACTIVE_USR+"/contactList.txt";
    QStringList contactList=readTextLine(Filename);                 //Read user data from file.

    anyNewContact=false;

    foreach(QString CONTACT, contactList){
        if(CONTACT.at(0) != "@"){
            QListWidgetItem *contact = new QListWidgetItem;
            contact->setIcon(QIcon(":/img/icons/person.png"));
            contact->setText(CONTACT);
            ui->listWidget_Contact->addItem(contact);
        }
        else{
            QListWidgetItem *contact = new QListWidgetItem;
            contact->setIcon(QIcon(":/img/icons/newPerson.png"));
            QString theContact="!"+CONTACT.mid(1);
            contact->setText(theContact);
            ui->listWidget_Contact->addItem(contact);
            anyNewContact=true;
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
    ui->pushButton_AddList->setIcon((QIcon(":img/icons/add_contact.png")));


    //show current clicked icon
    ui->pushButton_Conversation->setIcon(QIcon(":/img/icons/menu_chat.png"));
    if(anyNewContact==true){
        ui->pushButton_Contact->setIcon(QIcon(":/img/icons/menu_contact_clicked_noti.png"));
    }
    else{
        ui->pushButton_Contact->setIcon(QIcon(":/img/icons/menu_contact_clicked.png"));
    }
    ui->pushButton_Group->setIcon(QIcon(":/img/icons/menu_group.png"));
    ui->pushButton_Conversation->setIconSize(QSize(70,70));
    ui->pushButton_Contact->setIconSize(QSize(70,70));
    ui->pushButton_Group->setIconSize(QSize(70,70));
}

void MainWindow::on_pushButton_Group_clicked()
{

    currentMenu = "group";

    ui->frame_2->show();

    //clear current display list
    ui->listWidget_Contact->clear();

    /*Add group to list.*/
    QString Filename = "./userData/"+ACTIVE_USR+"/groupList.txt";
    QStringList contactList=readTextLine(Filename);                 //Read user data from file.
    foreach(QString GROUP, contactList){
        QListWidgetItem *group = new QListWidgetItem;
        group->setIcon(QIcon(":/img/icons/person.png"));
        group->setText("GROUP~~"+GROUP);
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
    ui->pushButton_AddList->setIcon((QIcon(":img/icons/add_group.png")));

    //show current clicked icon
    ui->pushButton_Conversation->setIcon(QIcon(":/img/icons/menu_chat.png"));
    if(anyNewContact==true){
        ui->pushButton_Contact->setIcon(QIcon(":/img/icons/menu_contact_normal_noti.png"));
    }
    else{
        ui->pushButton_Contact->setIcon(QIcon(":/img/icons/menu_contact.png"));
    }
    ui->pushButton_Group->setIcon(QIcon(":/img/icons/menu_group_clicked.png"));
    ui->pushButton_Conversation->setIconSize(QSize(70,70));
    ui->pushButton_Contact->setIconSize(QSize(70,70));
    ui->pushButton_Group->setIconSize(QSize(70,70));
}

void MainWindow::on_pushButton_AddList_clicked()
{
    if(currentMenu=="conversation"){

    }
    else if(currentMenu=="contact"){

        AddContact addcontact(*conn, *encryption, this, ACTIVE_USR);
        addcontact.setModal(false);
        addcontact.exec();

        /* Reload contact list*/
        on_pushButton_Contact_clicked();


    }
    else if(currentMenu=="group"){

        CreateGroup createGroup(this, ACTIVE_USR);
        createGroup.setModal(false);
        createGroup.exec();

        /* Reload contact list*/
        on_pushButton_Group_clicked();


    }
}

/*When user click item on contact list.*/
void MainWindow::listWidget_Contact_ItemClicked(QListWidgetItem* item){
    conversationWith=item->text();

    ui->label_ConversationWith->setText(conversationWith);
    ui->listWidget_Conversation->clear();

    if(conversationWith.at(0) != "!"){

        ui->pushButton_newFriend_picture->hide();
        ui->frame_addFriend_confirm->hide();
        ui->frame_3->hide();
        ui->listWidget_Conversation->show();
        ui->frame->show();


        /*Load conversation*/
        QString filename="./userData/"+ACTIVE_USR+"/conversation/"+item->text();
        QStringList conversation=ReadConversation(filename);

        /*Show conversation*/
        foreach(QString msg, conversation){
            QListWidgetItem *item = new QListWidgetItem;
            if(msg.left(ACTIVE_USR.length()+1) == ACTIVE_USR+":"){
                msg = msg.remove(ACTIVE_USR+": ");
                item->setText(msg);
                item->setTextAlignment(2);
            }
            else{
                msg = msg.remove(conversationWith+": ");
                item->setIcon(QIcon(":/img/icons/person.png"));
                item->setText(msg);
            }

            ui->listWidget_Conversation->addItem(item);
        }

        ui->listWidget_Conversation->scrollToBottom();
    }
    else{

        ui->listWidget_Conversation->hide();
        ui->frame->hide();
        ui->pushButton_newFriend_picture->show();
        ui->frame_addFriend_confirm->show();
        ui->frame_3->show();
        ui->label_ConversationWith->setText(conversationWith.mid(1) +
                                            " wants to be your friend.");
    }

}

/*When user click SEND button*/
void MainWindow::on_pushButton_SEND_clicked()
{


    QString conversationWith = ui->label_ConversationWith->text();
    QString msg = ui->plainTextEdit->toPlainText();
    QString Filename="./userData/"+ACTIVE_USR+"/conversation/"+conversationWith;

    if((conversationWith != "") && (msg != "") ){
        ui->listWidget_Conversation->clear();
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

            msg=ACTIVE_USR+": "+msg+"\n";
            QTextStream out(&File);
            out << msg;

            File.flush();
            File.close();
        }

        /*Load conversation*/
        QString filename="./userData/"+ACTIVE_USR+"/conversation/"+conversationWith;
        QStringList conversation=ReadConversation(filename);

        /*Show conversation*/
        foreach(QString msg, conversation){
            QListWidgetItem *item = new QListWidgetItem;
            if(msg.left(ACTIVE_USR.length()+1) == ACTIVE_USR+":"){
                msg = msg.remove(ACTIVE_USR+": ");
                item->setText(msg);
                item->setTextAlignment(2);
            }
            else{
                msg = msg.remove(conversationWith+": ");
                item->setIcon(QIcon(":/img/icons/person.png"));
                item->setText(msg);
            }

            ui->listWidget_Conversation->addItem(item);
        }

        ui->listWidget_Conversation->scrollToBottom();
    }

    ui->plainTextEdit->clear();
}

void MainWindow::textMenuChange(){
    QString selected = ui->comboBox->currentText();
    //ui->comboBox->setCurrentIndex(0);
    if(selected=="Quit"){
        cleanClose();
    }
    if(selected=="Setting"){
        Setting settingWindow(ACTIVE_USR);
        settingWindow.setWindowTitle("Setting");
        settingWindow.setModal(false);
        settingWindow.exec();
        on_pushButton_Conversation_clicked();
    }
    if(selected=="Sign Out"){
        signOut();
    }
    ui->comboBox->setCurrentIndex(0);
}

void MainWindow::signOut(){

    qDebug() << "signOut";

    on_pushButton_Conversation_clicked();
    ui->label_ConversationWith->clear();
    ui->listWidget_Conversation->clear();
    ui->listWidget_Contact->clear();

    ui->pushButton_Conversation->setIcon(QIcon(":/img/icons/menu_chat_clicked.png"));
    ui->pushButton_Contact->setIcon(QIcon(":/img/icons/menu_contact.png"));
    ui->pushButton_Group->setIcon(QIcon(":/img/icons/menu_group.png"));


    this->setWindowTitle("E2EEIM");
    this->hide();

    ACTIVE_USR = "";
    finishInitUserDataStatus=false;

    SignIn signIn(*conn, *encryption);
    signIn.setModal(false);

    connect(ui->comboBox, SIGNAL(currentTextChanged(QString)),
                this, SLOT(textMenuChange()));

    if(signIn.exec() == QDialog::Accepted)
    {
        ACTIVE_USR = signIn.getActiveUser();
        qDebug() <<"DEEP_DEBUG 0";

    }

    if(ACTIVE_USR != ""){

        qDebug() <<"DEEP_DEBUG 1";

        this->servKey=encryption->getServerPubKey();
        this->userPriKey=encryption->getUserPriKey();
        this->userPubKey=encryption->getUserPubKey();

        initUserDataPath();

        this->setWindowTitle("E2EEIM-"+ACTIVE_USR);
        this->show();

        qDebug() <<"DEEP_DEBUG 2";

    }
    else{
        this->setWindowTitle("E2EEIM");
        cleanClose();
    }

}

void MainWindow::cleanClose(){
    qDebug() << "CLEAN CLOSE";
    if(conn->getConnectionStatus()==1){
        conn->letDisconnect();
    }
    //QCoreApplication::quit();
    exit(0);
}

/*Enter key filter when user typing*/
bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if(watched == ui->plainTextEdit){
        if(event->type() == QKeyEvent::KeyPress ||
                event->type() == QKeyEvent::KeyRelease){
            QKeyEvent * KEY = static_cast<QKeyEvent*>(event);

            if((KEY->key()==Qt::Key_Return ||
                KEY->key()==Qt::Key_Enter) &&
                    KEY->modifiers()==Qt::ShiftModifier){

                if(event->type()==QKeyEvent::KeyRelease){
                    return true;
                }
            }
            else if(KEY->key()==Qt::Key_Return ||
                    KEY->key()==Qt::Key_Enter){

                        on_pushButton_SEND_clicked();
                        return true;
           }

        }
        return false;
    }
    return false;

}

void MainWindow::receiveAddFriendRequest(QByteArray data){

    qDebug() << "************************RECEIVE ADD FIREND REQUEST**********************";
    //qDebug() << data;

    qDebug() <<"DEEP_DEBUG 3";

    QString username;

    if(finishInitUserDataStatus==false){
        initUserDataPath();
    }

    //Decrypt Payload
    QFile File_Result("temp.cipher");
    if(!File_Result.open(QFile::WriteOnly | QFile::Text)){
        qDebug() << "Cound not open file for writing";
        abort();
    }
    QTextStream ts(&File_Result);
    ts << data.mid(5);

    File_Result.flush();
    File_Result.close();

    bool isValid=encryption->decryptVerify("temp.cipher", "temp.txt");

    qDebug() <<"DEEP_DEBUG 4";

    if(isValid==false){
        //ui->label_signIn_keyFpr->setStyleSheet("color:#FF6666");
        //ui->label_signIn_keyFpr ->setText("ERROR: Server signature not fully valid");
    }
    else{
        QFile File_result("temp.txt");
        if(!File_result.open(QFile::ReadOnly | QFile::Text)){
            qDebug() << "Cound not open file for Read";
            abort();
        }
        QTextStream in(&File_result);
        QString qs;
        qs=in.readAll();
        File_result.close();

        qDebug() << qs;
        username=qs;
    }

    username="@"+username;

    QString ContactName=username;

    bool isDuplicate=false;


    QString Filename = "./userData/"+ACTIVE_USR+"/contactList.txt";
    QStringList contactList=readTextLine(Filename);
    foreach(QString CONTACT, contactList){
        if(ContactName==CONTACT){
            isDuplicate=true;
        }
    }

    if(isDuplicate==false){
        QString Filename = "./userData/"+ACTIVE_USR+"/contactList.txt";

        if(ContactName != ""){
            QFile File(Filename);
            if(!File.exists()){
                if(!File.open(QFile::WriteOnly | QFile::Text)){
                    qDebug() << "cound not open file for writing";
                    exit(1);
                }
                QTextStream out(&File);
                out << "";

             File.flush();
                File.close();
            }
            if(File.exists()){

                if(!File.open(QFile::Append | QFile::Text)){
                      qDebug() << "cound not open file for writing";
                      exit(1);
                }
                  QTextStream out(&File);
                  out << ContactName+"\n";

                  File.flush();
                  File.close();
               }

        }
        anyNewContact=true;
        ui->pushButton_Contact->setIcon(QIcon(":/img/icons/menu_contact_normal_noti.png"));

    }
}

void MainWindow::receiveNewPublicKey(QByteArray data){

    qDebug() << "************************RECEIVE NEW PUBLIC KEY**********************";
    //qDebug() << data;

    qDebug() <<"DEEP_DEBUG A";

    QString pubKey;

    if(finishInitUserDataStatus==false){
        initUserDataPath();
    }

    //Decrypt Payload
    QFile File_Result("temp.cipher");
    if(!File_Result.open(QFile::WriteOnly | QFile::Text)){
        qDebug() << "Cound not open file for writing";
        abort();
    }
    QTextStream ts(&File_Result);
    ts << data.mid(5);

    File_Result.flush();
    File_Result.close();

    bool isValid=encryption->decryptVerify("temp.cipher", "temp.txt");

    qDebug() <<"DEEP_DEBUG B";

    if(isValid==true){
        QFile File_result("temp.txt");
        if(!File_result.open(QFile::ReadOnly | QFile::Text)){
            qDebug() << "Cound not open file for Read";
            abort();
        }
        QTextStream in(&File_result);
        QString qs;
        qs=in.readAll();
        File_result.close();

        pubKey=qs;

        gpgme_import_result_t importResult=encryption->importKey("temp.txt");

        if(importResult->imported==1 || importResult->unchanged==1){
            QString fpr=QString(importResult->imports->fpr);

            gpgme_key_t key=encryption->getKey(fpr.toLatin1().data(), 0);

            QString ContactName=QString(key->uids->name);

            this->removeFromListFile("./userData/"+ACTIVE_USR+"/contactList.txt", "@"+ContactName);

            QString Filename = "./userData/"+ACTIVE_USR+"/contactList.txt";

            if(ContactName != ""){
                QFile File(Filename);
                if(!File.exists()){
                    if(!File.open(QFile::WriteOnly | QFile::Text)){
                        qDebug() << "cound not open file for writing";
                        exit(1);
                    }
                    QTextStream out(&File);
                    out << "";

                 File.flush();
                    File.close();
                }
                if(File.exists()){

                    if(!File.open(QFile::Append | QFile::Text)){
                          qDebug() << "cound not open file for writing";
                          exit(1);
                    }
                      QTextStream out(&File);
                      out << ContactName+"\n";

                      File.flush();
                      File.close();
                   }

            }

            if(ui->label_ConversationWith->text().mid(0, 12) == "Waiting for "){
                ui->label_ConversationWith->setText("");
                ui->pushButton_newFriend_picture->hide();
                on_pushButton_Contact_clicked();

            }

        }
    }


}



void MainWindow::on_pushButton_addFriend_accept_clicked()
{
    QString username=conversationWith.mid(1);

    QByteArray payload;
    payload.append(username);

    //Encrypt Payload
    QFile File_Payload("addFriendConfirm.keyword");
    if(!File_Payload.open(QFile::WriteOnly | QFile::Text)){
        qDebug() << "Cound not open file searchContact.keyword for writing";
        exit(1);
    }
    QTextStream out(&File_Payload);
    out << payload;

    File_Payload.flush();
    File_Payload.close();

    encryption->printKeys(userPriKey);

    encryption->encryptSign(userPriKey, servKey, "addFriendConfirm.keyword", "addFriendConfirm.cipher");

    QFile File_EncryptedPayload("addFriendConfirm.cipher");
    if(!File_EncryptedPayload.open(QFile::ReadOnly | QFile::Text)){
        qDebug() << "Cound not open file for Read";
        abort();
    }
    QTextStream in(&File_EncryptedPayload);
    QString cipher;
    cipher=in.readAll();
    File_EncryptedPayload.close();

    payload.clear();

    QByteArray data;

    data.append(cipher);

    // Insert operation in front of byte array (data[0]).
    data.insert(0, (char)14);

    //Insert size of(operation + payload) in front of byte array (data[0]).
    int dataSize=data.size();
    QByteArray dataSizeByte;
    QDataStream ds2(&dataSizeByte, QIODevice::WriteOnly);
    ds2 << dataSize;
    data.insert(0, dataSizeByte);

    conn->send(data);

    ui->label_ConversationWith->setText("Waiting for "+username+"'s public key from server...");

    ui->frame_addFriend_confirm->hide();

}

void MainWindow::on_pushButton_addFriend_decline_clicked()
{
    QString Filename = "./userData/"+ACTIVE_USR+"/contactList.txt";
    QStringList contactList=readTextLine(Filename);
    QStringList newContactList;
    foreach(QString contact, contactList){
        if(contact!="@"+conversationWith.mid(1)){
            newContactList.append(contact);
        }
    }

    QFile File(Filename);
    if(!File.exists()){
        if(!File.open(QFile::WriteOnly | QFile::Text)){
            qDebug() << "cound not open file for writing";
            exit(1);
        }
        QTextStream out(&File);
        out << "";

     File.flush();
     File.close();
    }
    if(File.exists()){

        if(!File.open(QFile::WriteOnly | QFile::Text)){
            qDebug() << "cound not open file for writing";
            exit(1);
        }
        QTextStream out(&File);
        out << "";
        File.flush();
        File.close();

        if(!File.open(QFile::Append | QFile::Text)){
              qDebug() << "cound not open file for writing";
              exit(1);
        }
        QTextStream out2(&File);
        foreach(QString contact, newContactList){
            out2 << contact+"\n";
        }

        File.flush();
        File.close();
    }

    on_pushButton_Contact_clicked();
    ui->frame_addFriend_confirm->hide();
    ui->frame_3->hide();
    ui->label_ConversationWith->setText("");

}

void MainWindow::removeFromListFile(QString filename, QString item){
    QStringList list=readTextLine(filename);
    QStringList newList;
    foreach (QString listItem, list) {
        if(listItem != item){
            newList.append(listItem);
        }
    }


    QFile File(filename);
    if(!File.exists()){
        if(!File.open(QFile::WriteOnly | QFile::Text)){
            qDebug() << "cound not open file for writing";
            exit(1);
        }
        QTextStream out(&File);
        out << "";

     File.flush();
     File.close();
    }
    if(File.exists()){

        if(!File.open(QFile::WriteOnly | QFile::Text)){
            qDebug() << "cound not open file for writing";
            exit(1);
        }
        QTextStream out(&File);
        out << "";
        File.flush();
        File.close();

        if(!File.open(QFile::Append | QFile::Text)){
              qDebug() << "cound not open file for writing";
              exit(1);
        }
        QTextStream out2(&File);
        foreach(QString listItem, newList){
            out2 << listItem+"\n";
        }

        File.flush();
        File.close();
    }

}
