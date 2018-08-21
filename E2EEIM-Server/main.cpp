#include <QCoreApplication>
#include "server.h"
#include <gpgme.h>
#include <iostream>
#include <QQueue>

#define detectError(err)                                    \
    if (err) {                                              \
        fprintf (stderr, "%s:%d: %s: %s\n",                 \
                __FILE__, __LINE__, gpgme_strsource (err),  \
                gpgme_strerror (err));                      \
            exit (1);                                       \
        }

QString serverKey(int argc){

    bool validUse=false;
    QString fpr=NULL;

    //Select server key UI loop;
    while(validUse==false){

        if(argc==1){ //User not specify key pair;

            //Create GPGME context and setup GPGME
            gpgme_ctx_t ctx;  // the context
            gpgme_error_t err; // errors
            gpgme_key_t key= nullptr; // the key

            //Begin setup of GPGME
                gpgme_check_version (NULL);
                setlocale (LC_ALL, "");
                gpgme_set_locale (NULL, LC_CTYPE, setlocale (LC_CTYPE, NULL));

                // Create the GPGME Context
                err = gpgme_new (&ctx);
                // Error handling
                detectError(err);

                // Set the context to textmode
                gpgme_set_textmode (ctx, 1);
                // Enable ASCII armor on the context
                gpgme_set_armor (ctx, 1);


            qDebug() << "\n\n-------------------------";
            qDebug() << "Select option below:\n";
            qDebug() << "   G for generate new key pair for server.";
            qDebug() << "   E for use existing key pair in key ring.";
            qDebug() << "   Q for quit server application.";

            QTextStream qtin(stdin);
            //QString line = qtin.readLine();

            printf("\nYour selection:");
            QString word;
            qtin >> word;

            if(word=="E" || word=="e"){

                //Loop until user found useable key pair.
                while(true){

                    printf("Enter key fingerprint or key ID:");

                    QTextStream qtin(stdin);
                    //QString line = qtin.readLine();

                    QString word;
                    qtin >> word;

                    QByteArray ba = word.toLatin1();
                    const char *patt=ba.data();

                    err=gpgme_get_key(ctx, patt, &key, 1);

                    if(key){
                        err=gpgme_get_key(ctx, patt, &key, 0);

                        if(key){
                            fpr=QString(key->fpr);

                            qDebug() << "============================";
                            printf("Key ID: %s\n", key->subkeys->keyid);
                            if(key->uids && key->uids->name)
                                printf("Name: %s\n", key->uids->name);
                            if(key->uids && key->uids->email)
                                printf("E-Mail: %s\n", key->uids->email);
                            qDebug() << "============================";

                            validUse=true;
                            break;
                        }
                        else{
                            qDebug() << "\n\nPublic key not found!!\n\n";

                        }
                    }

                    else{
                        qDebug() << "\n\nPrivate key not found!!\n\n";
                    }
                }

            }
            if(word=="G" || word=="g"){

                QString keyname;
                QString passphrase;

                bool isNameValid=false;

                //User choose key name
                while(isNameValid==false){

                    QTextStream qtin(stdin);
                    //QString line = qtin.readLine();

                    printf("\nChoose name of the new key pair:");
                    QString name;
                    qtin >> name;

                    for(int i=0; i<name.length(); i++){ //key name validation;
                        int dec=name.mid(i,1).data()->unicode();

                        if((dec>64 && dec<91) ||
                                (dec>96 && dec<123)){
                            isNameValid=true;
                            continue;
                        }
                        else{
                            qDebug() << "\nNew key name only letters(a-z, A-Z) are allowed!\n";
                            isNameValid=false;
                            break;
                        }
                    }

                    if(isNameValid==true){
                        keyname=name;
                    }
                }

                //User enter passphrase
                bool isPassphraseMatch=false;
                while(isPassphraseMatch==false){

                    QTextStream qtin(stdin);
                    //QString line = qtin.readLine();

                    printf("\nChoose key pair passphrase (password):");
                    QString pass;
                    qtin >> pass;

                    QTextStream qtin2(stdin);
                    //QString line = qtin.readLine();

                    printf("\nConfirm key pair passphrase (password):");
                    QString conf;
                    qtin2 >> conf;

                    if(conf==pass){
                        passphrase=pass;
                        isPassphraseMatch=true;
                        break;
                    }
                    else{
                        qDebug() << "\nPassphrase does not match the confirm passphrase!\n";
                    }
                }


                QString keyParms="<GnupgKeyParms format=\"internal\">\n"
                                 "Key-Type: RSA\n"
                                 "Key-Length: 4096\n"
                                 "Subkey-Type: RSA\n"
                                 "Subkey-Length: 4096\n"
                                 "Name-Real: "+keyname+"\n"
                                 "Name-Comment: Generated by E2EEIM Chat, Passphrase:"+passphrase+"\n"
                                 "Name-Email: server@e2eeim.chat\n"
                                 "Expire-Date: 1y\n"
                                 "Passphrase: "+passphrase+"\n"
                                 "</GnupgKeyParms>\n";


                QByteArray byteParms = keyParms.toLatin1();
                const char *parms = byteParms.data();

                qDebug() << "\n\nKey pair generating...";

                gpgme_genkey_result_t result;

                err = gpgme_op_genkey (ctx, parms, NULL, NULL);
                detectError(err);

                result = gpgme_op_genkey_result (ctx);
                if (!result){
                    fprintf (stderr, "%s:%d: gpgme_op_genkey_result returns NULL\n",
                           __FILE__, __LINE__);
                    exit (1);
                }
                if (result->fpr && strlen (result->fpr) != 40){
                    fprintf (stderr, "%s:%d: generated key has unexpected fingerprint\n",
                           __FILE__, __LINE__);
                    exit (1);
                }
                if (!result->primary){
                    fprintf (stderr, "%s:%d: primary key was not generated\n",
                           __FILE__, __LINE__);
                    exit (1);
                }
                if (!result->sub)
                    {
                    fprintf (stderr, "%s:%d: sub key was not generated\n",
                           __FILE__, __LINE__);
                    exit (1);
                }

                err=gpgme_get_key(ctx, result->fpr, &key, 1);

                if(key){
                    err=gpgme_get_key(ctx, result->fpr, &key, 0);

                    if(key){
                        fpr=QString(key->fpr);

                        qDebug() << "Key pair generation success!";
                        qDebug() << "============================";
                        printf("Key ID: %s\n", key->subkeys->keyid);
                        if(key->uids && key->uids->name)
                            printf("Name: %s\n", key->uids->name);
                        if(key->uids && key->uids->email)
                            printf("E-Mail: %s\n", key->uids->email);
                        qDebug() << "============================";

                        validUse=true;
                    }
                    else{
                        qDebug() << "\n\nPublic key not found!!\n\n";

                    }
                }

                else{
                    qDebug() << "\n\nPrivate key not found!!\n\n";
                }

            }
            if(word=="Q" || word=="q"){
                exit(0);
            }
        }
    }

    return fpr;
}

int serverPort(){

    int port;

    //loop until user select valid port number.
    while(true){

        printf("\nSelect port number between 1024-65535 for the server:");

        QTextStream qtin(stdin);
        qtin >> port;

        if(port>1023 && port<65535){
            break;
        }
    }

    return port;

}


int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QQueue<QByteArray> msg;
    QList<QString> usernameList;
    QList<QString> userKeyList;
    QList<QString> loginUser;
    QList<QString> loginRanNum;
    QList<QString> waitingTaskUser;
    QList<QString> waitingTaskWork;
    QList<QString> addFriendRequestList;

    qDebug() << "-------------------------";

    QString keyFpr=serverKey(argc);
    int port=serverPort();

    //Create server.
    Server server(msg, usernameList, userKeyList,
                  loginUser, loginRanNum,
                  waitingTaskUser, waitingTaskWork,
                  addFriendRequestList, keyFpr, port);

    //Start server.
    server.startServer();

    // https://forum.qt.io/topic/86025/qt-get-external-ip-address-using-qnetworkreply/2

    return a.exec();
}
