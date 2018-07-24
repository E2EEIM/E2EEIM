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


    //Connect receiveAddFriendRequest event.
    //Creat a [signal/slot] mechanism, when active-user receive add friend request
    //to call receiveAddFriendRequest() function.
    connect(this->conn, SIGNAL(receiveAddFriendrequest(QByteArray)), this, SLOT(receiveAddFriendRequest(QByteArray)), Qt::QueuedConnection);

    //Connect receiveNewPublicKey event.
    //Creat a [signal/slot] mechanism, when active-user receive new public key
    //to call receiveAddNewPublicKey() function.
    connect(this->conn, SIGNAL(receiveNewPublicKey(QByteArray)), this, SLOT(receiveNewPublicKey(QByteArray)), Qt::QueuedConnection);

    //Connect receiveNewMessage event.
    //Creat a [signal/slot] mechanism, when active-user receive new message
    //to call receiveNewMessage() function.
    connect(this->conn, SIGNAL(receiveNewMessage(QByteArray)), this, SLOT(receiveNewMessage(QByteArray)), Qt::QueuedConnection);

    //Connect disconnect from server event.
    //Creat a [signal/slot] mechanism, when client application disconnect from server.
    //to call disconnectFromServer() function.
    connect(this->conn, SIGNAL(disconnectFromServer()), this, SLOT(disconnectFromServer()), Qt::QueuedConnection);

    // call signOut() to display signIn window before user can use mainWindow.
    signOut();

}

MainWindow::~MainWindow()
{
    delete ui;
}

// Init user data after login.
void MainWindow::initUserDataPath(){

    anyNewContact=false;

    // Create users data directory
    QString userDataPath("./userData");
    QDir userData;
    userData.mkdir(userDataPath);

    QString activeUserDataPath("./userData/"+ACTIVE_USR);
    QDir activeUserData;
    activeUserData.mkdir(activeUserDataPath);

    // Create coversation log directory
    QString userConversationDir("./userData/"+ACTIVE_USR+"/conversation");
    QDir userConversation;
    userConversation.mkdir(userConversationDir);

    // Create newMessage file for new message notification list,
    // this file store list of unread conversation or new message
    // received from server when user sign in. Contents in this file
    // will load to QList<QString> newMessage variable.
    QFile FileNewMessage("./userData/"+ACTIVE_USR+"/newMessage");
    if(!FileNewMessage.exists()){
        if(!FileNewMessage.open(QFile::WriteOnly | QFile::Text)){
            qDebug() << "could not open AU/newMessage file for writing";
            abort();
        }
        QTextStream out(&FileNewMessage);
        out.setCodec("UTF-8");
        out << "";

        FileNewMessage.flush();
        FileNewMessage.close();
    }

    // Create addFriendRequest file for save value in
    // QList<QString> addFriendRequestList, list of username
    //that user have sent add friend request to.
    QFile File("./userData/"+ACTIVE_USR+"/addFriendRequestList.txt");
    if(!File.exists()){
        if(!File.open(QFile::WriteOnly | QFile::Text)){
            qDebug() << "could not open addFriendRequestList.txt file for writing";
            abort();
        }
        QTextStream out(&File);
        out.setCodec("UTF-8");
        out << "";

        File.flush();
        File.close();
    }

    // Get list of username that active-user have sent
    // add friend request for addFriendRequest.txt file.
    if(!File.open(QFile::ReadOnly | QFile::Text)){
        qDebug() << "could not open addFriendRequestList.txt file for Read";
        abort();
    }

    QTextStream in(&File);
    in.setCodec("UTF-8");
    QString line;
    while(!in.atEnd())
    {
            line=in.readLine();
            addFriendRequestList.append(line);
    }



    // Load conversation list from conversation directory.
    QDir conversationFile("./userData/"+ACTIVE_USR+"/conversation");
    QStringList conversationList;
    foreach(QFileInfo item, conversationFile.entryInfoList()){
        if(item.isFile()){

            conversationList.append(item.fileName());
        }
    }

    // Add conversation list to listWidget_Contact that
    // show list of of conversation in GUI.
    foreach(QString conversation, conversationList){
        QListWidgetItem *item = new QListWidgetItem;
        item->setIcon(QIcon(":/img/icons/person.png"));
        item->setText(conversation);
        ui->listWidget_Contact->addItem(item);
    }


    /*
     Connect WidgetListItem on click event.
     Creat a [signal/slot] mechanism, when active-user click
     an item on listWidget_Contat the listWidget emit itemClick() signal
     to call listWidget_Contact_ItemClicked() function.
     This part are for identify what item active-user clicked to
     let listWidget_Contact_itemClicked() know and control what to do
     when the item have clicked. listWidget_Contact is for show list
     of items that depend on what recent menu user clicked, it can be
     list of conversation, list of contact(friend) or list of group.
     */
    connect(ui->listWidget_Contact, SIGNAL(itemClicked(QListWidgetItem*)),
                this, SLOT(listWidget_Contact_ItemClicked(QListWidgetItem*)), Qt::QueuedConnection);

    /*Connect combobox on click event.
     * Create a [signal/slot] mechanism, when active-user select
     * an item in textMenu(three line icon) to call
     * textMenuChange() function.
     */
    connect(ui->comboBox, SIGNAL(currentTextChanged(QString)),
                this, SLOT(textMenuChange()));

    /*Filter Enter/Return key press when user typing*/
    // Filter enter key is for send message that user typed
    // in plainTextEdit by enter key.
    ui->plainTextEdit->installEventFilter(this);

    /*Set icon size in listWidget*/
    ui->listWidget_Contact->setIconSize(QSize(50, 50));
    ui->listWidget_Conversation->setIconSize(QSize(50, 50));

    // Scoll conversation space to bottom to show recent
    // message of the selected conversation.
    ui->listWidget_Conversation->scrollToBottom();

    // Hide accept/decline add friend requset part and
    // group accept/decline invite part in conversation space.
    ui->frame_2->hide();
    ui->frame_addFriend_confirm->hide();
    ui->frame_3->hide();

    // Init default value for notification flag.
    anyNewMessage=false;
    anyNewContact=false;
    anyNewGroup=false;

    removeNotiFlag=false;

    // Set this(this function) initialization status.
    finishInitUserDataStatus=true;

    // call on_pushButton_Converation_clicked(),
    // usuall the function will call when active-user click
    // Coveration Menu to show or refresh conversation list
    // in QListWidget_Contact
    on_pushButton_Conversation_clicked();
    currentMenu = "conversation";

    // init default split bar size.
    QList<int> currentSizes = ui->splitter->sizes();
    currentSizes[0]=216;
    currentSizes[1]=507;
    ui->splitter->setSizes(currentSizes);

}

// read group members from groupList.txt file.
QStringList MainWindow::getGroupMember(QString GroupName){ // read group member from groupList.txt file.
    QFile File("./userData/"+ACTIVE_USR+"/groupList.txt");
    QStringList List;
    if(!File.exists()){
        if(!File.open(QFile::WriteOnly | QFile::Text)){
            qDebug() << "Could not open groupList.txt file for writing";
            abort();
        }
        QTextStream out(&File);
        out.setCodec("UTF-8");
        out << "";

        File.flush();
        File.close();
    }

    if(!File.open(QFile::ReadOnly | QFile::Text)){
        qDebug() << "Could not open file for Read";
        abort();
    }


    QTextStream in(&File);
    in.setCodec("UTF-8");
    QString line;
    short flagLoad=0;
    while(!in.atEnd())
    {
            line=in.readLine();
            if(line.left(2)=="~~"){
                if((line.split("~~").at(1)) == GroupName){ // Find selected group.
                    flagLoad=1;
                    continue;
                }
                else if(flagLoad==1)
                    break;
            }
            if(flagLoad!=0){
                List.append(line); // Get group members.

            }
    }
    return List;
}

// Read contact(friend) or group list in list file.
QStringList MainWindow::readTextLine(QString Filename){
    QFile File(Filename);
    QStringList List;
    if(!File.exists()){
        if(!File.open(QFile::WriteOnly | QFile::Text)){
            qDebug() << "Could not open "+Filename+" file for writing";
            abort();
        }
        QTextStream out(&File);
        out.setCodec("UTF-8");
        out << "";

        File.flush();
        File.close();
    }

    if(!File.open(QFile::ReadOnly | QFile::Text)){
        qDebug() << "Could not open file for Read";
        abort();
    }

    QTextStream in(&File);
    in.setCodec("UTF-8");
   QString line;
    while( !in.atEnd())
    {
        if(Filename=="./userData/"+ACTIVE_USR+"/groupList.txt"){
            line=in.readLine();
            if(line.left(2)!="~~" && line.left(2)!="~+")
                continue;
            List.append(line);
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

// read a conversation history from conversation file.
QStringList MainWindow::ReadConversation(QString Filename){

    int ACTIVE_USR_length;
    int conversationWith_length;

    ACTIVE_USR_length=ACTIVE_USR.length();
    conversationWith_length=conversationWith.length();

    QFile File(Filename);
    QStringList List;
    if(!File.exists()){
        if(!File.open(QFile::WriteOnly | QFile::Text)){
            qDebug() << "Could not open"+Filename+" file for writing";
            abort();
        }
        QTextStream out(&File);
        out.setCodec("UTF-8");
        out << "";

        File.flush();
        File.close();
    }


    if(!File.open(QFile::ReadOnly | QFile::Text)){
        qDebug() << "Could not open " +Filename+ " file for Read";
        abort();
    }

    QTextStream in(&File);
    in.setCodec("UTF-8");
    QString line;
    QString tmp="[-STaRT-]"; // Start message mark, to support multi-line message.

    QString onlyFileName=Filename.split("/").last();

    bool isGroupConversation=false;
    QStringList allGroupName=readTextLine("./userData/"+ACTIVE_USR+"/groupList.txt");
    foreach(QString item, allGroupName){
        if(item=="~~"+onlyFileName+"~~"){
            isGroupConversation=true;
        }
    }


    if(isGroupConversation==true){ //In case group conversation;
        QStringList groupMember=getGroupMember(onlyFileName);
        while( !in.atEnd())
        {
            line=in.readLine();
            int flagFistLine=1;
            if(line.left(ACTIVE_USR_length+1)!=(ACTIVE_USR+":")){ // In case the message is not active-user's message.
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
    else{ // in case not group conversation.
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


//click conversation icon (conversation menu) event
void MainWindow::on_pushButton_Conversation_clicked()
{
    currentMenu = "conversation"; // set current menu

    ui->frame_2->hide(); // hide add contact icon.


    //clear current display list in listWdget_Contact
    ui->listWidget_Contact->clear();

    anyNewMessage=false; //set default new message notification flag.

    if(ACTIVE_USR!=""){

        //
        QString Filename = "./userData/"+ACTIVE_USR+"/newMessage";
        newMessageList=readTextLine(Filename);                 //Read user data from file.

        QStringList notiItemList;
        QStringList normalItemList;

        /* Load conversation list*/
        QDir conversationFile("./userData/"+ACTIVE_USR+"/conversation");
        QStringList conversationList;
        foreach(QFileInfo item, conversationFile.entryInfoList()){
            if(item.isFile()){

                conversationList.append(item.fileName());
                bool newMsgInThis=false;

                // Checking if the conversation have new message to show notification.
                foreach (QString newMessage, newMessageList) {
                    if(item.fileName()==newMessage){
                        notiItemList.append(item.fileName());
                        newMsgInThis=true;
                        anyNewMessage=true;
                    }
                }
                if(newMsgInThis==false){
                    normalItemList.append(item.fileName());
                }
            }
        }

        /*Add conversation to display in listWidget_Contact.*/
        foreach(QString conversation, notiItemList){ // notification conversation.
            QListWidgetItem *item = new QListWidgetItem;
            item->setIcon(QIcon(":/img/icons/newPerson.png"));
            item->setText(conversation);
            ui->listWidget_Contact->addItem(item);
            anyNewMessage=true;

        }
        foreach(QString conversation, normalItemList){ // normal conversation.
            QListWidgetItem *item = new QListWidgetItem;
            item->setIcon(QIcon(":/img/icons/person.png"));
            item->setText(conversation);
            ui->listWidget_Contact->addItem(item);
        }

        //In case user move splitter hide contacts list and don't know how to show it again.
        //Code below will show contact list when user click this chat buttom.
        QList<int> currentSizes = ui->splitter->sizes();
        if(currentSizes[0]==0){
            currentSizes[0]=216;
            //currentSizes[1]=507;
            ui->splitter->setSizes(currentSizes);
        }

    }



    //show add new conversation icon
    ui->pushButton_AddList->setIcon((QIcon(":img/icons/add_conversation.png")));

    //show current clicked icon
    if(anyNewMessage==true){ // In case conversation menu need notification icon.
        ui->pushButton_Conversation->setIcon(QIcon(":/img/icons/menu_conversation_noti_clicked.png"));
    }
    else{
        ui->pushButton_Conversation->setIcon(QIcon(":/img/icons/menu_chat_clicked.png"));
    }

    if(anyNewContact==true){ // In case contact(friend) menu need notification icon.
        ui->pushButton_Contact->setIcon(QIcon(":/img/icons/menu_contact_normal_noti.png"));
    }
    else{
        ui->pushButton_Contact->setIcon(QIcon(":/img/icons/menu_contact.png"));
    }
    if(anyNewGroup==true){ // In case group menu need notification icon.
        ui->pushButton_Group->setIcon(QIcon(":/img/icons/menu_group_noti_normal.png"));
    }
    else{
        ui->pushButton_Group->setIcon(QIcon(":/img/icons/menu_group.png"));
    }

    // Set icon size
    ui->pushButton_Conversation->setIconSize(QSize(70,70));
    ui->pushButton_Contact->setIconSize(QSize(70,70));
    ui->pushButton_Group->setIconSize(QSize(70,70));
}

//click contact icon (contact menu) event
void MainWindow::on_pushButton_Contact_clicked()
{

    currentMenu = "contact"; // Set current menu

    ui->frame_2->show(); // Show add contact icon

    //clear current display list
    ui->listWidget_Contact->clear();


    /*Add contact to contact list.*/
    QString Filename = "./userData/"+ACTIVE_USR+"/contactList.txt";
    QStringList contactList=readTextLine(Filename);                 //Read user data from file.

    anyNewContact=false; // Set default new friend notification status flag.

    foreach(QString CONTACT, contactList){
        if(CONTACT.at(0) == "!"){ // In case the contact is add friend request.
            QListWidgetItem *contact = new QListWidgetItem;
            contact->setIcon(QIcon(":/img/icons/newPerson.png"));
            QString theContact=CONTACT;
            contact->setText(theContact);
            ui->listWidget_Contact->addItem(contact);
            anyNewContact=true;

        }
        else if(CONTACT.at(0) == "@"){ // In case the contact is new friend
            QListWidgetItem *contact = new QListWidgetItem;
            contact->setIcon(QIcon(":/img/icons/newPerson.png"));
            QString theContact=CONTACT;
            contact->setText(theContact);
            ui->listWidget_Contact->addItem(contact);
            anyNewContact=true;

        }
        else{
            QListWidgetItem *contact = new QListWidgetItem;
            contact->setIcon(QIcon(":/img/icons/person.png"));
            contact->setText(CONTACT);
            ui->listWidget_Contact->addItem(contact);
        }
    }

    //In case user move splitter hide contacts list and don't know how to show it again.
    //Code below will show contact list when user click this concact buttom.
    QList<int> currentSizes = ui->splitter->sizes();
    if(currentSizes[0]==0){
        currentSizes[0]=216;
        currentSizes[1]=507;
        ui->splitter->setSizes(currentSizes);
    }


    //show add new contact icon
    ui->pushButton_AddList->setIcon((QIcon(":img/icons/add_contact.png")));


    //show current clicked menu
    if(anyNewMessage==true){ // In case conversation menu need notification icon.
        ui->pushButton_Conversation->setIcon(QIcon(":/img/icons/menu_conversation_noti_normal.png"));
    }
    else{
        ui->pushButton_Conversation->setIcon(QIcon(":/img/icons/menu_chat.png"));
    }
    if(anyNewContact==true){// In case contact(friends) menu need notification icon.
        ui->pushButton_Contact->setIcon(QIcon(":/img/icons/menu_contact_clicked_noti.png"));
    }
    else{
        ui->pushButton_Contact->setIcon(QIcon(":/img/icons/menu_contact_clicked.png"));
    }
    if(anyNewGroup==true){ // In case group menu need notification icon.
        ui->pushButton_Group->setIcon(QIcon(":/img/icons/menu_group_noti_normal.png"));
    }
    else{
        ui->pushButton_Group->setIcon(QIcon(":/img/icons/menu_group.png"));
    }

    // set menu icon size
    ui->pushButton_Conversation->setIconSize(QSize(70,70));
    ui->pushButton_Contact->setIconSize(QSize(70,70));
    ui->pushButton_Group->setIconSize(QSize(70,70));
}

//Group menu click event
void MainWindow::on_pushButton_Group_clicked()
{

    currentMenu="group"; // Set current menu value

    ui->frame_2->show(); // Show create new group icon

    //clear current display list in listWidget_Contact
    ui->listWidget_Contact->clear();

    anyNewGroup=false; //set default need notifiation flag.

    /*Add group to list.*/
    QString Filename = "./userData/"+ACTIVE_USR+"/groupList.txt";
    QStringList contactList=readTextLine(Filename);                 //Read user data from file.
    foreach(QString item, contactList){
        QListWidgetItem *group = new QListWidgetItem;
        QString groupName;
        if(item.mid(0,2)=="~+"){ //In case the group is invitation.
            groupName=item.split("~+").at(1);
            group->setIcon(QIcon(":/img/icons/groupItem_noti.png"));
            group->setText("+"+groupName);
            anyNewGroup=true;
        }
        else{
            groupName=item.split("~~").at(1);
            group->setIcon(QIcon(":/img/icons/groupItem.png"));
            group->setText(groupName);
        }
        ui->listWidget_Contact->addItem(group);
    }

    //In case user move splitter hide contacts list and don't know how to show it again.
    //Code below will show contact list when user click this group buttom.
    QList<int> currentSizes = ui->splitter->sizes();
    if(currentSizes[0]==0){
        currentSizes[0]=216;
        currentSizes[1]=507;
        ui->splitter->setSizes(currentSizes);
    }

    //show add new group icon
    ui->pushButton_AddList->setIcon((QIcon(":img/icons/add_group.png")));

    //show current clicked icon
    if(anyNewMessage==true){ // In case conversation menu need notification icon.
        ui->pushButton_Conversation->setIcon(QIcon(":/img/icons/menu_conversation_noti_normal.png"));
    }
    else{
        ui->pushButton_Conversation->setIcon(QIcon(":/img/icons/menu_chat.png"));
    }
    if(anyNewContact==true){ // In case contact menu need notification icon.
        ui->pushButton_Contact->setIcon(QIcon(":/img/icons/menu_contact_normal_noti.png"));
    }
    else{
        ui->pushButton_Contact->setIcon(QIcon(":/img/icons/menu_contact.png"));
    }
    if(anyNewGroup==true){ // In case group menu need notification icon.
        ui->pushButton_Group->setIcon(QIcon(":/img/icons/menu_group_noti_clicked.png"));
    }
    else{
        ui->pushButton_Group->setIcon(QIcon(":/img/icons/menu_group_clicked.png"));
    }

    //Set icons size.
    ui->pushButton_Conversation->setIconSize(QSize(70,70));
    ui->pushButton_Contact->setIconSize(QSize(70,70));
    ui->pushButton_Group->setIconSize(QSize(70,70));
}

// Add new contact(friend) or create new group click event
void MainWindow::on_pushButton_AddList_clicked()
{
    if(currentMenu=="contact"){ // In case user click add new contact(friend) icon

        //Create and show addContact window
        AddContact addcontact(*conn, *encryption, addFriendRequestList, this, ACTIVE_USR);
        addcontact.setModal(false);
        addcontact.exec();

        /* Reload contact list*/
        on_pushButton_Contact_clicked();


    }
    else if(currentMenu=="group"){ //In case user click create new icon

        CreateGroup createGroup(*conn, *encryption, ACTIVE_USR, this);
        createGroup.setModal(false);
        createGroup.exec();

        /* Reload contact list*/
        on_pushButton_Group_clicked();
    }
}

/*When user click item on contact list(listWidget_Contact).*/
void MainWindow::listWidget_Contact_ItemClicked(QListWidgetItem* item){

    if(removeNotiFlag==false){  //In case user click notification item.
        conversationWith=item->text(); //Set conversationWith variable as text of the item.
    }
    else{
        removeNotiFlag=false;
    }

    // display conversation name as text of item.
    ui->label_ConversationWith->setText(conversationWith);
    ui->listWidget_Conversation->clear();

    if(conversationWith.at(0)=="!"){ // In case the item is add friend request.

        //Hide conversation space.
        ui->listWidget_Conversation->hide();
        ui->frame->hide();

        //Show [accept/decline] part in conversation space.
        ui->pushButton_newFriend_picture->setIcon(QIcon(":/img/icons/newPerson.png"));
        ui->pushButton_newFriend_picture->show();
        ui->frame_addFriend_confirm->show();
        ui->pushButton_addFriend_accept->setText("Accept");
        ui->pushButton_addFriend_decline->show();
        ui->frame_3->show();
        ui->label_ConversationWith->setText(conversationWith.mid(1) +
                                            " wants to be your friend.");
    }
    else if(conversationWith.at(0)=="+"){ //In case the item is group invite.

        QString msg="Someone invite you to "+conversationWith.mid(1)+" group.";

        //Hide conversation space.
        ui->listWidget_Conversation->hide();
        ui->frame->hide();

        //Show [Join group/decline] part in conversation sapce.
        ui->pushButton_newFriend_picture->setIcon(QIcon(":/img/icons/groupItem_noti.png"));
        ui->pushButton_newFriend_picture->show();
        ui->frame_addFriend_confirm->show();
        ui->pushButton_addFriend_accept->setText("Join");
        ui->pushButton_addFriend_decline->show();
        ui->frame_3->show();
        ui->label_ConversationWith->setText(msg);

    }
    else if(conversationWith.at(0)=="@"){  //In case the item is new firend.

        //Hide conversation space.
        ui->listWidget_Conversation->hide();
        ui->frame->hide();

        //Show new firend notification.
        ui->pushButton_newFriend_picture->show();
        ui->frame_addFriend_confirm->show();
        ui->pushButton_addFriend_accept->setText("OK");
        ui->pushButton_addFriend_decline->hide();
        ui->frame_3->show();
        ui->label_ConversationWith->setText("You and "+ conversationWith.mid(1) +
                                            " are friends now!");
    }
    else{

        ui->pushButton_newFriend_picture->hide();
        ui->frame_addFriend_confirm->hide();
        ui->frame_3->hide();
        ui->listWidget_Conversation->show();
        ui->frame->show();

        /*Load conversation*/
        QString filename="./userData/"+ACTIVE_USR+"/conversation/"+conversationWith;
        QStringList conversation=ReadConversation(filename);


        /*Show conversation*/
        foreach(QString msg, conversation){
            QListWidgetItem *item = new QListWidgetItem;
            if(msg.left(ACTIVE_USR.length()+1) == ACTIVE_USR+":"){ //Active-user's message
                msg = msg.remove(ACTIVE_USR+": ");
                item->setText(msg);
                item->setTextAlignment(2);
            }
            else{ //Not active-user's message
                msg = msg.remove(conversationWith+": ");
                item->setIcon(QIcon(":/img/icons/person.png"));
                item->setText(msg);
            }

            ui->listWidget_Conversation->addItem(item);
        }


        // show recent message.
        ui->listWidget_Conversation->scrollToBottom();
    }


    //Clear notification when users click notification item.
    QString Filename = "./userData/"+ACTIVE_USR+"/newMessage";
    newMessageList=readTextLine(Filename);

    bool messageFormUnreadUser=false; //The conversation is notifacation item flag.

    foreach (QString newMessage, newMessageList) {
        if(newMessage==conversationWith){ //In case the conversaton is notification item.
            messageFormUnreadUser=true;
            break;
        }
    }

    //Remove notifiaction if user click notification item
    if(messageFormUnreadUser==true && currentMenu=="conversation"){
        removeFromListFile(Filename, conversationWith);
        removeNotiFlag=true;
        on_pushButton_Conversation_clicked();
    }

    QList<int> currentSizes = ui->splitter->sizes();
    if(currentSizes[1]<1){
        currentSizes[0]=216;
        currentSizes[1]=507;
        ui->splitter->setSizes(currentSizes);
    }
}

/*When user click SEND message button*/
void MainWindow::on_pushButton_SEND_clicked()
{
    QString conversationWith = ui->label_ConversationWith->text(); //Get recipient username
    QString msg = ui->plainTextEdit->toPlainText(); //Get message

    //Checking, if recipent is a group.
    QStringList groupList=readTextLine("./userData/"+ACTIVE_USR+"/groupList.txt");
    foreach(QString item, groupList){
        if("~~"+conversationWith+"~~"==item){
            sendTo="group";
            break;
        }
    }

    //Checking, if recipient is a person(friend).
    QStringList contactList=readTextLine("./userData/"+ACTIVE_USR+"/contactList.txt");
    foreach(QString item, contactList){
        if(conversationWith==item){
            sendTo="person";
            break;
        }
    }


    // Save new message to conversation
    QString Filename="./userData/"+ACTIVE_USR+"/conversation/"+conversationWith;
    if((conversationWith != "") && (msg != "") ){
        ui->listWidget_Conversation->clear();
        QFile File(Filename);
        if(!File.exists()){
            if(!File.open(QFile::WriteOnly | QFile::Text)){
                qDebug() << "could not open "+ Filename +" file for writing";
                abort();
            }
            QTextStream out(&File);
            out.setCodec("UTF-8");
            out << "";

            File.flush();
            File.close();
        }
        if(File.exists()){
            if(!File.open(QFile::Append | QFile::Text)){
                qDebug() << "could not open"+ Filename+" file for writing";
                abort();
            }


            msg=ACTIVE_USR+": "+msg+"\n";


            QTextStream out(&File);
            out.setCodec("UTF-8");
            out << msg;

            File.flush();
            File.close();
        }

        if(sendTo=="group"){ // In case send message as group conversation.
            msg="~~"+conversationWith+"~~"+msg; // append group conversation protocol.
        }

        // Send message
        if(sendTo=="person"){ // In case send message to a person(friend).
            sendToPerson(conversationWith, msg); //Call sendToPerson() to send message.
        }
        else if(sendTo=="group"){ // In case send message as group conversation.

            QStringList groupMember=getGroupMember(conversationWith); //Get group member
            foreach(QString item, groupMember){ //Send message to every group member
                if(item==ACTIVE_USR){ //Don's send to active-user.
                    continue;
                }
                sendToPerson(item, msg); //Send message to each group member.
            }
        }

        /*Load conversation*/
        QString filename="./userData/"+ACTIVE_USR+"/conversation/"+conversationWith;
        QStringList conversation=ReadConversation(filename);

        /*Show conversation*/
        foreach(QString msg, conversation){
            QListWidgetItem *item = new QListWidgetItem;
            if(msg.left(ACTIVE_USR.length()+1) == ACTIVE_USR+":"){ // In the message is active-user's message.
                msg = msg.remove(ACTIVE_USR+": "); // Don't show active-user's username in conversation.
                item->setText(msg);
                item->setTextAlignment(2); // Show active-user's message at right align.
            }
            else{
                msg = msg.remove(conversationWith+": "); //In case the message is not active-user's message.
                item->setIcon(QIcon(":/img/icons/person.png")); // Show person icon when it's not active-user's message.
                item->setText(msg);
            }

            ui->listWidget_Conversation->addItem(item); //Add the message to conversation to display.
        }

        ui->listWidget_Conversation->scrollToBottom(); //Scroll to bottom to show recent message.
    }

    ui->plainTextEdit->clear(); //Clear typing space.
}

//Send post message to server.
void MainWindow::sendToPerson(QString recipientName, QString message){

    //Creat text file for encrypt message.
    QString Filename="./userData/"+ACTIVE_USR+"/msgToEncrypt.txt";
    QFile rawFile(Filename);
    if(!rawFile.exists()){
        if(!rawFile.open(QFile::WriteOnly | QFile::Text)){
            qDebug() << "could not open"+Filename+" file for writing";
            abort();
        }
        QTextStream out(&rawFile);
        out.setCodec("UTF-8");
        out << "";

        rawFile.flush();
        rawFile.close();
    }

    //Write the message to text file to encrypt.
    if(rawFile.exists()){
        if(!rawFile.open(QFile::WriteOnly | QFile::Text)){
            qDebug() << "could not open file"+ Filename +" for writing";
            abort();
        }

        QTextStream out(&rawFile);
        out.setCodec("UTF-8");
        out << message;

        rawFile.flush();
        rawFile.close();
    }

    //Convert recipient username into const char* variable type
    //that required in get public key process.
    QByteArray qbRecipient = recipientName.toLatin1();
    const char *recipient = qbRecipient.data();

    //Get recipent public's key
    gpgme_key_t recipientKey=encryption->getKey(recipient, 0);

    //Convert the message file path into const char* variable type
    //that required in encryption process.
    QByteArray qbIn=Filename.toLatin1();
    const char *inputFileName=qbIn.data();

    //Convert output file path into constant char* variable type
    //that required in encryption process.
    QString outputFilePath="./userData/"+ACTIVE_USR+"/msgToEncrypt.cipher";
    QByteArray qbOut=outputFilePath.toLatin1();
    const char *outPutFileName=qbOut.data();

    //Create output file.
    QFile cipherFile(outputFilePath);
    if(!cipherFile.exists()){
        if(!cipherFile.open(QFile::WriteOnly | QFile::Text)){
            qDebug() << "could not open"+outputFilePath+" file for writing";
            abort();
        }
        QTextStream out(&cipherFile);
        out.setCodec("UTF-8");
        out << "";

        cipherFile.flush();
        cipherFile.close();
    }

    //Encrypt the message for recipient.
    encryption->encryptSign(userPriKey, recipientKey, inputFileName, outPutFileName);

    //Read encrypted message for recipient from output file.
    QFile File_cipher("./userData/"+ACTIVE_USR+"/msgToEncrypt.cipher");
    if(!File_cipher.open(QFile::ReadOnly | QFile::Text)){
        qDebug() << "could not open file for Read";
        abort();
    }
    QTextStream in(&File_cipher);
    in.setCodec("UTF-8");
    QString cipher;
    cipher=in.readAll();
    File_cipher.close();

    //get recipient username size.
    char recvUsernameSize=char(recipientName.size());

    //Creat post message protocol for server.
    QByteArray msgToServer;
    msgToServer.append(cipher);
    msgToServer.insert(0, recipientName);
    msgToServer.insert(0, recvUsernameSize);

    //Save post message protocol for server to text file to encrypt.
    Filename="./userData/"+ACTIVE_USR+"/msgToEncrypt.txt";
    if(rawFile.exists()){
        if(!rawFile.open(QFile::WriteOnly | QFile::Text)){
            qDebug() << "could not open "+Filename+" file for writing";
            abort();
        }

        QTextStream out(&rawFile);
        out.setCodec("UTF-8");
        out << msgToServer;

        rawFile.flush();
        rawFile.close();

    }

    //Convert the post message file path into constant char* variable type
    //that required in encryption process.
    Filename="./userData/"+ACTIVE_USR+"/msgToEncrypt.txt";
    QByteArray qb2=Filename.toLatin1();
    inputFileName=qb2.data();

    //Convert output file path into constant char* variable type
    //that required in encryption process.
    Filename="./userData/"+ACTIVE_USR+"/msgToEncrypt.cipher";
    QByteArray qb3=Filename.toLatin1();
    outPutFileName=qb3.data();

    //Encrypt the message for server.
    encryption->encryptSign(userPriKey, servKey, inputFileName, outPutFileName);

    //Read encrypted message for server from output file.
    QFile qf("./userData/"+ACTIVE_USR+"/msgToEncrypt.cipher");
    if(!qf.open(QFile::ReadOnly | QFile::Text)){
        qDebug() << "could not open msgToEncrypt.cipher file for Read";
        abort();
    }
    QTextStream pl(&qf);
    QString qs;
    qs=pl.readAll();
    qf.close();

    QString payload=qs;

    QByteArray data;
    data.append(payload);

    // Insert operation in front of byte array (data[0]).
    data.insert(0, (char)17);

    //Insert size of(operation + payload) in front of byte array (data[0]).
    int dataSize=data.size();
    QByteArray dataSizeByte;
    QDataStream ds(&dataSizeByte, QIODevice::WriteOnly);
    ds << dataSize;
    data.insert(0, dataSizeByte);

    //Send post message to server.
    conn->send(data);

}

//User select text menu event.
void MainWindow::textMenuChange(){
    QString selected = ui->comboBox->currentText(); //Get the selected menu.
    //ui->comboBox->setCurrentIndex(0);
    if(selected=="Quit"){ //In case user selecte "Quit" in text menu.
        cleanClose(); //Call cleanClose() function to quit client application in proper way.
    }
    if(selected=="Setting"){ //In case user selecte "Setting" in text menu.
        Setting settingWindow(ACTIVE_USR); //Create setting window.
        settingWindow.setWindowTitle("Setting"); //Set the window title.
        settingWindow.setModal(false); //No need modal.
        settingWindow.exec(); //Open the setting window.
        on_pushButton_Conversation_clicked(); //Refresh client appliation.
    }
    if(selected=="Sign Out"){ //In case user select "Sign Out" in text menu.
        disconnectFromServer(); //Call disconnectFormServer() to disconnect from server and not receive
                                //any data from server when no active-user using cliet application.

    }
    ui->comboBox->setCurrentIndex(0); //Set selected selected item in text menu to default value.
}

// Sign out function.
void MainWindow::signOut(){

    qDebug() << "signOut";

    on_pushButton_Conversation_clicked(); //Set mainWindow menu to default menu(conversation).
    ui->label_ConversationWith->clear(); //Clear showing conversationWith.
    ui->listWidget_Conversation->clear(); //Clear conversation space.
    ui->listWidget_Contact->clear(); //Clear item in listWidget_Contact.

    //Set menu icons to default.
    ui->pushButton_Conversation->setIcon(QIcon(":/img/icons/menu_chat_clicked.png"));
    ui->pushButton_Contact->setIcon(QIcon(":/img/icons/menu_contact.png"));
    ui->pushButton_Group->setIcon(QIcon(":/img/icons/menu_group.png"));

    //Set mainWindow title to default.
    this->setWindowTitle("E2EEIM");

    //Disable mainWindow.
    this->hide();

    //Set active-user to default.
    ACTIVE_USR = "";

    //Set init user data status to default.
    finishInitUserDataStatus=false;

    //Create sign in/sign up window.
    SignIn signIn(*conn, *encryption);
    signIn.setModal(false);

    //Connect user select text menu event.
    //Creat a [signal/slot] mechanism, when active-user select item in text menu.
    //to call textMenuChage() function.
    connect(ui->comboBox, SIGNAL(currentTextChanged(QString)),
                this, SLOT(textMenuChange()));

    //User close signIn/signUp window conditions.
    if(signIn.exec() == QDialog::Accepted)
    {
        ACTIVE_USR = signIn.getActiveUser(); //get account username user selected to sign in.
    }

    if(ACTIVE_USR != ""){ //In cast user select some account username.

        this->setEnabled(true); //Enable mainWindow.

        this->servKey=encryption->getServerPubKey(); //Get server public key from encryption class.
        this->userPriKey=encryption->getUserPriKey(); //Get active-user's public key from encryption class.
        this->userPubKey=encryption->getUserPubKey(); //Get active-user's public key from encryption class.

        initUserDataPath(); //Init user data path.

        this->setWindowTitle("E2EEIM-"+ACTIVE_USR); // Show active-user's username in window title.
        this->show();

    }
    else{ //In case user not select any account to sign in.
        this->setWindowTitle("E2EEIM"); //Set mainWindow title to default.
        cleanClose(); //Close client application in proper way.
    }

}

//Close client application.
void MainWindow::cleanClose(){
    qDebug() << "CLEAN CLOSE";
    if(conn->getConnectionStatus()==1){ //In case client application still connect to server.
        conn->letDisconnect(); //Disconnect from erver.
    }
    exit(0); //Close client application.
}

/*Filter Enter/Return key when user typing
 * to send message when user press enter/return key.
 */
bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if(watched == ui->plainTextEdit){ //watch on typing space.
        if(event->type() == QKeyEvent::KeyPress ||
                event->type() == QKeyEvent::KeyRelease){
            QKeyEvent * KEY = static_cast<QKeyEvent*>(event); //Condition when user press or release a key.

            if((KEY->key()==Qt::Key_Return ||
                KEY->key()==Qt::Key_Enter) &&
                    KEY->modifiers()==Qt::ShiftModifier){ //When user press shift+[enter/return] keys

                if(event->type()==QKeyEvent::KeyRelease){ //When user release shift+[enter/return] keys
                    return true; //Do noting to let user get new line.
                }
            }
            else if(KEY->key()==Qt::Key_Return ||
                    KEY->key()==Qt::Key_Enter){ //In case user only press enter/return key.

                        on_pushButton_SEND_clicked(); //Call on_pushButton_SEND_clicked() to send message.
                        return true;
           }

        }
        return false; //Do noting when not typing in typing space.
    }
    return false; //Don noting when user have any action on other object.

}

//Client application recive add friend request.
void MainWindow::receiveAddFriendRequest(QByteArray data){

    QString username;

    //After login client application may receive add friend requst immediately
    //but client application not finish init user data.
    if(finishInitUserDataStatus==false){
        initUserDataPath(); //Init user data path before read/write any file.
    }

    //Decrypt Payload from server.

    //Save received data package to text file.
    QFile File_Result("temp.cipher");
    if(!File_Result.open(QFile::WriteOnly | QFile::Text)){
        qDebug() << "could not open temp.cipher file for writing";
        abort();
    }
    QTextStream ts(&File_Result);
    ts << data.mid(5);

    File_Result.flush();
    File_Result.close();

    //Decrypt received data package from text file.
    QString decryptResult=encryption->decryptVerify("temp.cipher", "temp.txt");

    if(decryptResult.mid(0,1)=="0"){ //In case decrypt result is bad signature from signer.
        //Do noting to ignore the request.
    }
    else{

        //Read decrypt message from text file.
        QFile File_result("temp.txt");
        if(!File_result.open(QFile::ReadOnly | QFile::Text)){
            qDebug() << "Could not open file for Read";
            abort();
        }
        QTextStream in(&File_result);
        in.setCodec("UTF-8");
        QString qs;
        qs=in.readAll();
        File_result.close();
        username=qs;

        username="!"+username; //Append "!" to sender's username to notify it's  a add friend request.

        QString ContactName=username;

        bool isDuplicate=false; //Duplicate flag.


        //Dupliate checking.
        QString Filename = "./userData/"+ACTIVE_USR+"/contactList.txt";
        QStringList contactList=readTextLine(Filename);
        foreach(QString CONTACT, contactList){
            if(ContactName==CONTACT){
                isDuplicate=true;
            }
        }

        if(isDuplicate==false){ //In case not duplicate.

            QString Filename = "./userData/"+ACTIVE_USR+"/contactList.txt";

            if(ContactName.mid(1) != ""){ //When sender's username is not empty.

                //Create contact list file if not exists.
                QFile File(Filename);
                if(!File.exists()){
                    if(!File.open(QFile::WriteOnly | QFile::Text)){
                        qDebug() << "could not open"+Filename+" file for writing";
                        exit(1);
                    }
                    QTextStream out(&File);
                    out.setCodec("UTF-8");
                    out << "";

                 File.flush();
                    File.close();
                }

                //Append the request to contact list.
                if(File.exists()){

                    if(!File.open(QFile::Append | QFile::Text)){
                          qDebug() << "could not open"+ Filename+ " file for writing";
                          exit(1);
                    }
                      QTextStream out(&File);
                      out.setCodec("UTF-8");
                      out << ContactName+"\n";

                      File.flush();
                      File.close();
                   }

            }
            anyNewContact=true; //Set new contact(friend) notification flag to true.

            //Set contact menu icon to show notification.
            ui->pushButton_Contact->setIcon(QIcon(":/img/icons/menu_contact_normal_noti.png"));

        }
    }
}

// Client application receive a public key from server.
void MainWindow::receiveNewPublicKey(QByteArray data){

    //After login client application may receive a public key immediately
    //but client application not finish init user data.
    if(finishInitUserDataStatus==false){
        initUserDataPath(); //Init user data path before read/write any file.
    }

    //Decrypt Payload

    //Save received data package to text file to decrypt.
    QFile File_Result("temp.cipher");
    if(!File_Result.open(QFile::WriteOnly | QFile::Text)){
        qDebug() << "could not open temp.cipher file for writing";
        abort();
    }
    QTextStream ts(&File_Result);
    ts << data.mid(5);

    File_Result.flush();
    File_Result.close();

    //Decrypt data package from text file.
    QString decryptResult=encryption->decryptVerify("temp.cipher", "temp.txt");

    if(decryptResult.mid(0,1)=="1"){ //In case decrypt result is good signature from server.

        //Read decrypted data package from decryption output file.
        QFile File("temp.txt");
        if(!File.open(QFile::ReadOnly | QFile::Text)){
            qDebug() << "Could not open file for Read";
            abort();
        }
        QTextStream in(&File);
        in.setCodec("UTF-8");
        QString qs;
        qs=in.readAll();
        File.close();

        QString payload;
        payload=qs;

        //Get data from decrypted payload.
        int usernameLength=payload.mid(0,1).data()->unicode();
        QString username=payload.mid(1, usernameLength);
        QString pubKey=payload.mid(usernameLength+1);

        //If name of public key is the public key that active-user waiting for.
        if(addFriendRequestList.indexOf(username)!=(-1)||
                addFriendConfirmList.indexOf(username)!=(-1)){

            //Save the public key to text file.
            QFile File("temp.txt");
            if(!File.open(QFile::WriteOnly | QFile::Text)){
                qDebug() << "Could not open file temp.txt for writing";
                exit(1);
            }
            QTextStream out(&File);
            out.setCodec("UTF-8");
            out << pubKey;

            File.flush();
            File.close();

            //Import public key from the text file.
            gpgme_import_result_t importResult=encryption->importKey("temp.txt");

            //When import public key success.
            if(importResult->imported==1 || importResult->unchanged==1){
                QString fpr=QString(importResult->imports->fpr);

                //Get the public from key ring.
                gpgme_key_t key=encryption->getKey(fpr.toLatin1().data(), 0);

                //Get uername of public key.
                QString ContactName=QString(key->uids->name);

                //Remove the received add friend request from contact list
                //if the public key is the public key of add friend request sender.
                this->removeFromListFile("./userData/"+ACTIVE_USR+"/contactList.txt", "!"+ContactName);


                if(ContactName != ""){ //When name of public key is not empty.
                    //Creat contact list file if it doesn't exist.
                    QString Filename = "./userData/"+ACTIVE_USR+"/contactList.txt";
                    QFile File(Filename);
                    if(!File.exists()){
                        if(!File.open(QFile::WriteOnly | QFile::Text)){
                            qDebug() << "Could not open" +Filename+ " file for writing";
                            exit(1);
                        }
                        QTextStream out(&File);
                        out.setCodec("UTF-8");
                        out << "";

                     File.flush();
                        File.close();
                    }

                    //Append name of public key as new contact(friend) to contact list.
                    //Append new friend's username to contact list file.
                    if(File.exists()){

                        if(!File.open(QFile::Append | QFile::Text)){
                              qDebug() << "Could not open"+Filename+" file for writing";
                              exit(1);
                        }
                          QTextStream out(&File);
                          out.setCodec("UTF-8");
                          out << "@"+ContactName+"\n";

                          File.flush();
                          File.close();
                       }

                }

                //Change display information from "Waiting for new friens's public key from server"
                //to "You and new friend's are friends now!"
                if(ui->label_ConversationWith->text().mid(0, 12) == "Waiting for "){
                    ui->label_ConversationWith->setText("You and "+username+" are friends now!");
                    ui->frame_addFriend_confirm->show();
                    ui->pushButton_addFriend_decline->hide();
                    ui->pushButton_addFriend_accept->setText("OK");
                    on_pushButton_Contact_clicked();

                }

                anyNewContact=true; //Set new friend notification flag.

                //Refrest contact(friend) menu icon to show notification.
                if(currentMenu=="conversation"){
                    on_pushButton_Contact_clicked();
                    on_pushButton_Conversation_clicked();
                }
                else if(currentMenu=="group"){
                    on_pushButton_Contact_clicked();
                    on_pushButton_Group_clicked();
                }
                else{
                    on_pushButton_Contact_clicked();
                }
            }

        }

    }


}

//Client receive new message.
void MainWindow::receiveNewMessage(QByteArray data){

    qDebug() << "RECEVIE NEW MESSAGE!!!!!!!!!!!!!";

    //After login client application may receive a new immediately
    //but client application not finish init user data.
    if(finishInitUserDataStatus==false){
        initUserDataPath(); //Init user data path file before read/write any file.
    }

    //Decrypt Payload

    //Save received data package to text file.
    QFile FileCipher("./userData/"+ACTIVE_USR+"/msg.cipher");

    //Create a text file to save received data package.
    if(!FileCipher.exists()){
        if(!FileCipher.open(QFile::WriteOnly | QFile::Text)){
            qDebug() << "Could not open msg.cipher file for writing";
            abort();
        }
        QTextStream out(&FileCipher);
        out.setCodec("UTF-8");
        out << "";

        FileCipher.flush();
        FileCipher.close();
    }

    //Create decryption outfile.
    QFile FileMsg("./userData/"+ACTIVE_USR+"/msg");
    if(!FileMsg.exists()){
        if(!FileMsg.open(QFile::WriteOnly | QFile::Text)){
            qDebug() << "Could not open msg file for writing";
            abort();
        }
        QTextStream out(&FileMsg);
        out.setCodec("UTF-8");
        out << "";

        FileMsg.flush();
        FileMsg.close();
    }


    //Save received data package to text file.
    if(!FileCipher.open(QFile::WriteOnly | QFile::Text)){
        qDebug() << "Could not open msg.cipher file for writing";
        abort();
    }
    QTextStream ts(&FileCipher);
    ts << data.mid(5);

    FileCipher.flush();
    FileCipher.close();

    //Convert the saved data package file path into constant char* variable type
    //that required in decrypt process.
    QString Filename;
    Filename="./userData/"+ACTIVE_USR+"/msg.cipher";
    QByteArray qb2=Filename.toLatin1();
    const char *inputFileName=qb2.data();

    //Convert the decryption output file path into constant char* variable type
    //that required in decrypt process.
    Filename="./userData/"+ACTIVE_USR+"/msg";
    QByteArray qb3=Filename.toLatin1();
    const char *outPutFileName=qb3.data();

    //Decrypt received message.
    QString decryptResult=encryption->decryptVerify(inputFileName, outPutFileName);

    //When decrypt result is good signature from server.
    if(decryptResult.mid(0,1)=="1"){

        //Read decrypt message from server to get encrypted message from sender.
        QFile File(Filename);
        if(!File.open(QFile::ReadOnly | QFile::Text)){
            qDebug() << "Could not open file for Read";
            abort();
        }
        QTextStream in(&File);
        in.setCodec("UTF-8");
        QString qs;
        qs=in.readAll();
        File.close();

        QString payload;
        payload=qs;

        //Save encrypted message from sender to text file.
        if(!FileCipher.open((QFile::WriteOnly | QFile::Text))){
            qDebug() << "Could not open file for Read";
            abort();
        }

        QTextStream out(&FileCipher);
        out.setCodec("UTF-8");
        out << payload;

        FileCipher.flush();
        FileCipher.close();

        //Convert the cipher(encrypted message) file path into constant char* variable type
        //that required in decrypt process.
        Filename="./userData/"+ACTIVE_USR+"/msg.cipher";
        QByteArray qb2=Filename.toLatin1();
        const char *inputFileName=qb2.data();


        //Convert the output file path into constant char* variable type
        //that required in decrypt process.
        Filename="./userData/"+ACTIVE_USR+"/msg";
        QByteArray qb3=Filename.toLatin1();
        const char *outPutFileName=qb3.data();


        //Decrypt cipher from sender.
        QString decryptResult=encryption->decryptVerify(inputFileName, outPutFileName);

        //When decrypt result is good signature from sender.
        if(decryptResult.mid(0,1)=="1"){

            //Read decrypted message from decryption output file.
            QFile File(Filename);
            if(!File.open(QFile::ReadOnly | QFile::Text)){
                qDebug() << "Could not open file for Read";
                abort();
            }
            QTextStream in(&File);
            in.setCodec("UTF-8");
            QString qs;
            qs=in.readAll();
            File.close();

            QString payload;
            payload=qs;

            qDebug() << "decrypResult:" << decryptResult;
            //qDebug() << "payload:" << payload;

            //Get the message.
            QString msg=payload;

            //Get fingerprint of public key that decrypt the message.
            QByteArray qb=decryptResult.mid(1).toLatin1();
            const char *fpr=qb.data();
            gpgme_key_t senderKey=encryption->getKey(fpr, 0);

            //Get name of public key.
            QString senderUsername=QString(senderKey->uids->name);


            //In case normal message, not group message.

            //This condition are for verify name of public key is the same as
            //owner message that marked in message and for identify
            //the message is not group message.
            if(msg.split(":").first()==senderUsername){

                //Get every contact(friend) in contact list.
                QString Filename = "./userData/"+ACTIVE_USR+"/contactList.txt";
                QStringList contactList=readTextLine(Filename);

                //The loop or this condition is for verify the sender is in contact list.
                foreach (QString item, contactList) {
                    if(senderUsername==item || "@"+senderUsername==item){

                        //Appen the mesage into conversation history.
                        QFile File("./userData/"+ACTIVE_USR+"/conversation/"+senderUsername);
                        if(!File.exists()){
                            if(!File.open(QFile::WriteOnly | QFile::Text)){
                                qDebug() << "could not open conversation/+senderUsername file for writing";
                                abort();
                            }
                            QTextStream out(&File);
                            out.setCodec("UTF-8");
                            out << msg;

                            File.flush();
                            File.close();
                        }
                        else{
                            if(!File.open(QFile::Append | QFile::Text)){
                                  qDebug() << "could not open conversation/+senderUsername file for writing";
                                  exit(1);
                            }
                            QTextStream out(&File);
                            out.setCodec("UTF-8");
                            out << msg;

                            File.flush();
                            File.close();

                        }


                        //Get all contact(friend) username that need to show
                        //as notification icon to notify active-user.
                        QString Filename = "./userData/"+ACTIVE_USR+"/newMessage";
                        newMessageList=readTextLine(Filename);


                        //Checking, Is the sender already show as notification item.
                        bool messageFormUnreadUser=false;
                        foreach (QString newMessage, newMessageList) {
                            if(newMessage==senderUsername){
                                messageFormUnreadUser=true;
                                break;
                            }
                        }

                        //If the sender not show as notification item then set
                        //notification to inform active-user, client-application
                        //receive new message from this sender.
                        if(messageFormUnreadUser==false){

                            //Append sender's uesrname to notification list (newMessage file)
                            QFile FileNewMessage("./userData/"+ACTIVE_USR+"/newMessage");
                            if(!FileNewMessage.exists()){
                                if(!FileNewMessage.open(QFile::WriteOnly | QFile::Text)){
                                    qDebug() << "could not open AU/newMessage file for writing";
                                    exit(1);
                                }
                                QTextStream out(&FileNewMessage);
                                out.setCodec("UTF-8");
                                out << senderUsername+"\n";

                                FileNewMessage.flush();
                                FileNewMessage.close();

                                anyNewMessage=true;
                                break;
                            }
                            else{
                                if(!FileNewMessage.open(QFile::Append | QFile::Text)){
                                      qDebug() << "could not open AU/newMessage file for writing";
                                      exit(1);
                                }
                                QTextStream out(&FileNewMessage);
                                out.setCodec("UTF-8");
                                out << senderUsername+"\n";

                                FileNewMessage.flush();
                                FileNewMessage.close();

                                anyNewMessage=true;
                                break;

                            }
                        }
                    }
                }

                //Refresh menu bar to show notification icon.
                if(anyNewMessage==true){

                    if(currentMenu=="contact"){

                        on_pushButton_Conversation_clicked();
                        on_pushButton_Contact_clicked();
                    }
                    else if(currentMenu=="group"){

                        on_pushButton_Conversation_clicked();
                        on_pushButton_Group_clicked();
                    }
                    else{

                        on_pushButton_Conversation_clicked();
                    }

                    //In case mainWindow showing conversation
                    //with sender then refresh conversation
                    //and show recent message.
                    if(conversationWith==senderUsername){


                        /*Load conversation*/
                        QString filename="./userData/"+ACTIVE_USR+"/conversation/"+senderUsername;
                        QStringList conversation=ReadConversation(filename);

                        /*Show conversation*/
                        ui->listWidget_Conversation->clear(); //Clear conversation space.

                        //Refresh conversation space.
                        foreach(QString msg, conversation){
                            QListWidgetItem *item = new QListWidgetItem;
                            if(msg.left(ACTIVE_USR.length()+1) == ACTIVE_USR+":"){ //In case message is active-user's message
                                msg = msg.remove(ACTIVE_USR+": "); //Do not show active-user's username.
                                item->setText(msg);
                                item->setTextAlignment(2); //Show active-username's message at right align.
                            }
                            else{ //In cast the message is not active-user's message.
                                msg = msg.remove(conversationWith+": "); //Show sender username.
                                item->setIcon(QIcon(":/img/icons/person.png")); //Show person icon.
                                item->setText(msg);
                            }

                            ui->listWidget_Conversation->addItem(item); //Add the message to conversation space.
                        }

                        ui->listWidget_Conversation->scrollToBottom(); //Scoll conversation sapce to bottom to show recent message.
                    }
                }

            }

            else if(msg.mid(0,2)=="~+"){ //In case the message is a group invitation.

                //Save invitation to text file.
                QFile groupInviteFile("./userData/"+ACTIVE_USR+"/groupInvite");
                if(!groupInviteFile.exists()){
                    if(!groupInviteFile.open(QFile::WriteOnly | QFile::Text)){
                        qDebug() << "could not open"+Filename+" file for writing";
                        abort();
                    }
                    QTextStream out(&groupInviteFile);
                    out.setCodec("UTF-8");
                    out << "";

                    groupInviteFile.flush();
                    groupInviteFile.close();
                }
                if(groupInviteFile.exists()){
                    if(!groupInviteFile.open(QFile::WriteOnly | QFile::Text)){
                        qDebug() << "could not open"+Filename+" file for writing";
                        abort();
                    }
                    QTextStream out(&groupInviteFile);
                    out.setCodec("UTF-8");
                    out << msg;

                    groupInviteFile.flush();
                    groupInviteFile.close();
                }

                QString groupName;
                QStringList member;
                QString groupPubKey;

                //Read group invitation from text file.
                QStringList inviteMsg=readTextLine("./userData/"+ACTIVE_USR+"/groupInvite");
                int lineBeginPubKey=0; //Line number that it's the first line of group member public key.
                foreach (QString line, inviteMsg) {
                    if(line=="-----BEGIN PGP PUBLIC KEY BLOCK-----"){ //Stop loop when found the first list of public key.
                        break;
                    }
                    if(line.mid(0,2)=="~+"){ //Get group name.
                        groupName=line.mid(2);
                    }
                    else if(line!="~~~~" && line!=""){ //Get member username.
                        member.append(line);
                    }
                    lineBeginPubKey++;

                }

                //Get group public key.
                QString pubKey=msg.split("-----BEGIN PGP PUBLIC KEY BLOCK-----").last();
                groupPubKey="-----BEGIN PGP PUBLIC KEY BLOCK-----"+pubKey;

                //Append group name group list file.
                QFile groupListFile("./userData/"+ACTIVE_USR+"/groupList.txt");
                if(!groupListFile.exists()){
                    if(!groupListFile.open(QFile::WriteOnly | QFile::Text)){
                        qDebug() << "could not open"+Filename+" file for writing";
                        abort();
                    }
                    QTextStream out(&groupListFile);
                    out.setCodec("UTF-8");
                    out << "";

                    groupListFile.flush();
                    groupListFile.close();
                }
                if(groupListFile.exists()){
                    if(!groupListFile.open(QFile::Append | QFile::Text)){
                        qDebug() << "could not open"+Filename+" file for writing";
                        abort();
                    }
                    QTextStream out(&groupListFile);
                    out.setCodec("UTF-8");
                    out << "~+"+groupName+"~+\n";
                    foreach (QString item, member) {
                       out << item+"\n";
                    }

                    groupListFile.flush();
                    groupListFile.close();
                }

                //Save group member public key to a file.
                QFile groupKeyFile("./userData/"+ACTIVE_USR+"/groupPubKey_"+groupName);
                if(!groupKeyFile.exists()){
                    if(!groupKeyFile.open(QFile::WriteOnly | QFile::Text)){
                        qDebug() << "could not open"+Filename+" file for writing";
                        abort();
                    }
                    QTextStream out(&groupKeyFile);
                    out.setCodec("UTF-8");
                    out << "";

                    groupKeyFile.flush();
                    groupKeyFile.close();
                }
                if(groupKeyFile.exists()){
                    if(!groupKeyFile.open(QFile::WriteOnly | QFile::Text)){
                        qDebug() << "could not open"+Filename+" file for writing";
                        abort();
                    }
                    QTextStream out(&groupKeyFile);
                    out.setCodec("UTF-8");
                    out << groupPubKey;

                    groupKeyFile.flush();
                    groupKeyFile.close();
                }

                //Refresh menu bar to show notification icon in group menu.
                if(currentMenu=="conversation"){
                    on_pushButton_Group_clicked();
                    on_pushButton_Conversation_clicked();
                }
                else if(currentMenu=="contacts"){
                    on_pushButton_Group_clicked();
                    on_pushButton_Contact_clicked();
                }
                else{
                    on_pushButton_Group_clicked();
                }

            }
            else if(msg.mid(0,2)=="~~"){ //In case the received message is message for group conversation.

                QString groupName=msg.split("~~").at(1); //Get group name.

                //Get group conversation file path.
                QString conversationFilePath= "./userData/"+ACTIVE_USR+"/conversation/"+groupName;

                //Get group member.
                QStringList groupMember=getGroupMember(groupName);


                foreach (QString item, groupMember) {
                    if(senderUsername==item){ //If the sender is a group member.

                        //Appent the message to the group conversation.
                        QFile File("./userData/"+ACTIVE_USR+"/conversation/"+groupName);
                        if(!File.exists()){
                            if(!File.open(QFile::WriteOnly | QFile::Text)){
                                qDebug() << "could not open conversation/+senderUsername file for writing";
                                abort();
                            }
                            QTextStream out(&File);
                            out.setCodec("UTF-8");
                            out << msg.split("~~").at(2);

                            File.flush();
                            File.close();
                        }
                        else{
                            if(!File.open(QFile::Append | QFile::Text)){
                                  qDebug() << "could not open conversation/+senderUsername file for writing";
                                  exit(1);
                            }
                            QTextStream out(&File);
                            out.setCodec("UTF-8");
                            out << msg.split("~~").at(2);

                            File.flush();
                            File.close();

                        }


                        //Get all notififation list.
                        QString Filename = "./userData/"+ACTIVE_USR+"/newMessage";
                        newMessageList=readTextLine(Filename);


                        //Looking for group name notifiaction.
                        bool messageFormUnreadGroup=false;
                        foreach (QString newMessage, newMessageList) {
                            if(newMessage==groupName){
                                messageFormUnreadGroup=true;
                                break;
                            }
                        }

                        //If not found the group in notification list
                        //then create notification for the group
                        //to inform active-user that client application
                        //receive new message from a group member.
                        if(messageFormUnreadGroup==false){

                            //Append group name to notification list.
                            QFile FileNewMessage("./userData/"+ACTIVE_USR+"/newMessage");
                            if(!FileNewMessage.exists()){
                                if(!FileNewMessage.open(QFile::WriteOnly | QFile::Text)){
                                    qDebug() << "could not open AU/newMessage file for writing";
                                    exit(1);
                                }
                                QTextStream out(&FileNewMessage);
                                out.setCodec("UTF-8");
                                out << groupName+"\n";

                                FileNewMessage.flush();
                                FileNewMessage.close();

                                anyNewMessage=true;
                                break;
                            }
                            else{
                                if(!FileNewMessage.open(QFile::Append | QFile::Text)){
                                      qDebug() << "could not open AU/newMessage file for writing";
                                      exit(1);
                                }
                                QTextStream out(&FileNewMessage);
                                out.setCodec("UTF-8");
                                out << groupName+"\n";

                                FileNewMessage.flush();
                                FileNewMessage.close();

                                anyNewMessage=true;
                                break;

                            }
                        }
                    }
                }


                //Refresh menu bar to show notification icon.
                if(anyNewMessage==true){

                    if(currentMenu=="contact"){
                        on_pushButton_Conversation_clicked();
                        on_pushButton_Contact_clicked();
                    }
                    else if(currentMenu=="group"){
                        on_pushButton_Conversation_clicked();
                        on_pushButton_Group_clicked();
                    }
                    else{
                        on_pushButton_Conversation_clicked();
                    }

                    //In cast client applicaiton showing the group conversation
                    //then refresh conversation space.
                    if(conversationWith==groupName){

                        //Read conversation from file.
                        QString filename="./userData/"+ACTIVE_USR+"/conversation/"+groupName;
                        QStringList conversation=ReadConversation(filename);

                        /*Show conversation*/
                        ui->listWidget_Conversation->clear(); //Clear conversation space.
                        foreach(QString msg, conversation){
                            QListWidgetItem *item = new QListWidgetItem;
                            if(msg.left(ACTIVE_USR.length()+1) == ACTIVE_USR+":"){ //In case the message is active-user's message.
                                msg = msg.remove(ACTIVE_USR+": "); //Do not show active-user username.
                                item->setText(msg);
                                item->setTextAlignment(2); //Show active-user's message at right align.
                            }
                            else{ //In case the message is not active-user's message.
                                msg = msg.remove(conversationWith+": "); //Remove groupname from the message.
                                item->setIcon(QIcon(":/img/icons/person.png")); //Show person icon.
                                item->setText(msg);
                            }

                            ui->listWidget_Conversation->addItem(item); //Show the message in conversation space.
                        }

                        ui->listWidget_Conversation->scrollToBottom(); //Scoll to bottom to show recent message.
                    }
                }




            }
        }

    }

}


//active-user accept new friend or join group.
void MainWindow::on_pushButton_addFriend_accept_clicked()
{
    //In case active-user click "OK" to acknowledge
    //that client application already receive new firend's public key.
    if(ui->pushButton_addFriend_accept->text()=="OK"){

        //Hide accept and decline button.
        ui->frame_addFriend_confirm->hide();
        ui->pushButton_addFriend_accept->setText("Accept");
        ui->pushButton_addFriend_decline->show();
        ui->frame_addFriend_confirm->hide();
        ui->frame_3->hide();

        //Show conversation space.
        conversationWith=conversationWith.mid(1);
        ui->label_ConversationWith->setText(conversationWith);
        ui->listWidget_Conversation->show();
        ui->frame->show();

        //Remove new request and new friend mark in contact list.
        QString Filename = "./userData/"+ACTIVE_USR+"/contactList.txt";
        this->removeFromListFile(Filename, "!"+conversationWith);
        this->removeFromListFile(Filename, "@"+conversationWith);

        //Append new friend username to contact list.
        QFile File(Filename);
        if(!File.open(QFile::Append | QFile::Text)){
              qDebug() << "Could not open file " +Filename+ " for writing";
              exit(1);
        }
        QTextStream out2(&File);
        out2.setCodec("UTF-8");
        out2 << conversationWith+"\n";

        File.flush();
        File.close();

        //Refresh contact list.
        on_pushButton_Contact_clicked();

        //In case active-user have sent add friend request to this new friend.
        //When someone accpet the request and get active-username from server.
        //they might send message to active-user immediatly before active-user
        //click "OK" to acknowledge that the new friend already accept the request,
        //client application need to refresh conversation to show the message.
        Filename="./userData/"+ACTIVE_USR+"/conversation/"+conversationWith;
        QFile file_newFriend(Filename);
        if(file_newFriend.exists()){

            //Read conversation from file.
            QString filename="./userData/"+ACTIVE_USR+"/conversation/"+conversationWith;
            QStringList conversation=ReadConversation(filename);

            /*Show conversation*/
            ui->listWidget_Conversation->clear(); //Clear conversation space.
            foreach(QString msg, conversation){
                QListWidgetItem *item = new QListWidgetItem;
                if(msg.left(ACTIVE_USR.length()+1) == ACTIVE_USR+":"){ //In case the message is active's user mesage
                    msg = msg.remove(ACTIVE_USR+": "); //Do not show active-user's username in message.
                    item->setText(msg);
                    item->setTextAlignment(2); //Show active-user's message at right align.
                }
                else{
                    msg = msg.remove(conversationWith+": "); //Do not show sender's username in message.
                    item->setIcon(QIcon(":/img/icons/person.png")); //Show person icon.
                    item->setText(msg);
                }

                ui->listWidget_Conversation->addItem(item); //Show message in conversation space.
            }

            ui->listWidget_Conversation->scrollToBottom(); //Scroll down to bottom to show recent message.

        }

    }
    else if(ui->pushButton_addFriend_accept->text()=="Accept") //In case active-user accept add friend request.
    {
        QString username=conversationWith.mid(1); //Get sender username.

        //Create payload to send to server.
        QByteArray payload;
        payload.append(username); //Append sender's username to payload.

        //Encrypt Payload

        //Save payload to text file for encryption.
        QFile File_Payload("addFriendConfirm.keyword");
        if(!File_Payload.open(QFile::WriteOnly | QFile::Text)){
            qDebug() << "Could not open file searchContact.keyword for writing";
            exit(1);
        }
        QTextStream out(&File_Payload);
        out.setCodec("UTF-8");
        out << payload;

        File_Payload.flush();
        File_Payload.close();

        encryption->printKeys(userPriKey);

        //Encrypt payload before send to server.
        encryption->encryptSign(userPriKey, servKey, "addFriendConfirm.keyword", "addFriendConfirm.cipher");

        //Read cipher, the encrypted payload.
        QFile File_EncryptedPayload("addFriendConfirm.cipher");
        if(!File_EncryptedPayload.open(QFile::ReadOnly | QFile::Text)){
            qDebug() << "Could not open file for Read";
            abort();
        }
        QTextStream in(&File_EncryptedPayload);
        in.setCodec("UTF-8");
        QString cipher;
        cipher=in.readAll();
        File_EncryptedPayload.close();

        payload.clear();

        //Create data package, accept add friend request protocol.
        QByteArray data;
        data.append(cipher); //Append cipher to data package.

        // Insert accept add friend request operation in front of byte array (data[0]).
        data.insert(0, (char)14);

        //Insert size of(operation + payload) in front of byte array (data[0]).
        int dataSize=data.size();
        QByteArray dataSizeByte;
        QDataStream ds2(&dataSizeByte, QIODevice::WriteOnly);
        ds2 << dataSize;
        data.insert(0, dataSizeByte);

        //Send data package to server.
        conn->send(data);

        //Add the accepted/confirmed sender username to addFriendConfirmList for
        //verify when receive the request sender public key.
        addFriendConfirmList.append(username);

        //Inform accept/confirm status to active-user.
        ui->label_ConversationWith->setText("Waiting for "+username+"'s public key from server...");

        //Hide accept and decline button.
        ui->frame_addFriend_confirm->hide();

    }
    else if(ui->pushButton_addFriend_accept->text()=="Join"){ //In case active-user join group invite.

        //Inform active-user, key importing
        //and hide join/decline button.
        ui->label_ConversationWith->setText("Please wait, importing group member public keys...");
        ui->frame_addFriend_confirm->hide();

        //Convert group member public key file path into const char* variable type
        //that required in import public key process.
        QString grouPubKeyFile="./userData/"+ACTIVE_USR+"/groupPubKey_"+conversationWith.mid(1);
        QByteArray qb=grouPubKeyFile.toLatin1();
        const char *inputFileName=qb.data();

        //Import public key process.
        gpgme_import_result_t importResult=encryption->importKey(inputFileName);


        //Edit group list file.

        //Read group list file into QStringList groupList variable.
        QString groupListFile="./userData/"+ACTIVE_USR+"/groupList.txt";
        QStringList groupList;

        QFile FileGroupList(groupListFile);
        if(!FileGroupList.open(QFile::ReadOnly | QFile::Text)){
            qDebug() << "could not open groupList.txt file for Read";
            abort();
        }

        QTextStream in(&FileGroupList);
        in.setCodec("UTF-8");
        QString line;
        while(!in.atEnd())
        {
                line=in.readLine();
                groupList.append(line);
        }
        FileGroupList.close();


        //Remove the group inviteation from group list.
        QString newList;
        foreach (QString item, groupList) {
            if(item!="~+"+conversationWith.mid(1)+"~+"){
                newList=newList+(item+"\n");
            }
            else{
                item="~~"+conversationWith.mid(1)+"~~";
                newList=newList+(item+"\n");
            }
        }

        //Save new list to group list file.
        QFile File(groupListFile);
        if(!File.open(QFile::WriteOnly | QFile::Text)){
            qDebug() << "could not open "+groupListFile+ "file for writing";
            exit(1);
        }
        QTextStream out(&File);
        out.setCodec("UTF-8");
        out << newList;
        File.flush();
        File.close();

        //Refresh group list to remove group invite notifiaction icon.
        on_pushButton_Group_clicked();

        //Remove notification in conversation space.
        conversationWith=conversationWith.mid(1);

        if(importResult->imported!=0 || importResult->unchanged!=0){ //Import group member public key success.

            //Hide accept/decline space from conversation space.
            ui->frame_addFriend_confirm->hide();
            ui->pushButton_addFriend_accept->setText("Accept");
            ui->pushButton_addFriend_decline->show();
            ui->frame_addFriend_confirm->hide();
            ui->frame_3->hide();

            //Show conversation space.
            ui->label_ConversationWith->setText(conversationWith);
            ui->listWidget_Conversation->show();
            ui->frame->show();
        }

        //Show conversation history.
        QString Filename="./userData/"+ACTIVE_USR+"/conversation/"+conversationWith;
        QFile file_newGroup(Filename);
        if(file_newGroup.exists()){

            /*Load conversation*/
            QString filename="./userData/"+ACTIVE_USR+"/conversation/"+conversationWith;
            QStringList conversation=ReadConversation(filename);

            /*Show conversation*/
            ui->listWidget_Conversation->clear();
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

    }
}

//When active-user decline to add friend request or
//decline group invitation.
void MainWindow::on_pushButton_addFriend_decline_clicked()
{
    //Remove add friend request from contact list.
    QString Filename = "./userData/"+ACTIVE_USR+"/contactList.txt";
    this->removeFromListFile(Filename, conversationWith);

    //Refresh showing contact list.
    on_pushButton_Contact_clicked();
    ui->frame_addFriend_confirm->hide();
    ui->frame_3->hide();
    ui->label_ConversationWith->setText("");

}

//Remove an item from file.
void MainWindow::removeFromListFile(QString filename, QString item){

    //Load file content into QStringList list variable.
    QStringList list=readTextLine(filename);


    //Edit list.
    QStringList newList;
    foreach (QString listItem, list) {
        if(listItem != item){
            newList.append(listItem);
        }
    }


    //Save new list to file.
    QFile File(filename);
    if(!File.exists()){
        if(!File.open(QFile::WriteOnly | QFile::Text)){
            qDebug() << "could not open "+filename+ "file for writing";
            exit(1);
        }
        QTextStream out(&File);
        out.setCodec("UTF-8");
        out << "";

     File.flush();
     File.close();
    }
    if(File.exists()){

        if(!File.open(QFile::WriteOnly | QFile::Text)){
            qDebug() << "Could not open "+filename+" file for writing";
            exit(1);
        }
        QTextStream out(&File);
        out.setCodec("UTF-8");
        out << "";
        File.flush();
        File.close();

        if(!File.open(QFile::Append | QFile::Text)){
              qDebug() << "Could not open"+filename+"file for writing";
              exit(1);
        }
        QTextStream out2(&File);
        out2.setCodec("UTF-8");
        foreach(QString listItem, newList){
            out2 << listItem+"\n";
        }

        File.flush();
        File.close();
    }

}

//Disconnect client application from server.
void MainWindow::disconnectFromServer(){
    qDebug() << "DISCONNECT FROM SERVER";

    if(conn->getConnectionStatus()==1){ //In case client application still connect to server.
        conn->letDisconnect(); //force disconnect.
    }

    if(this->windowTitle()!="E2EEIM"){ //Set mainWindow title to default.
        this->setWindowTitle("E2EEIM DISCONNECT FROM SERVER!!"); //Inform user disconnect status.
        this->setDisabled(true); //Disable mainWindow.
        signOut(); //Show signIn/signUp window.
    }
}
