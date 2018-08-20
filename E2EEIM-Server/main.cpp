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

void serverKey(int argc){
    bool validUse=false;

    while(validUse==false){

        if(argc==1){

            gpgme_ctx_t ctx;  // the context
            gpgme_error_t err; // errors
            gpgme_key_t key= nullptr; // the key

            //Begin setup of GPGME
                gpgme_check_version (NULL);
                setlocale (LC_ALL, "");
                gpgme_set_locale (NULL, LC_CTYPE, setlocale (LC_CTYPE, NULL));
            //#ifndef HAVE_W32_SYSTEM
            //    gpgme_set_locale (NULL, LC_MESSAGES, setlocale (LC_MESSAGES, NULL));
            //#endif
                //err = gpgme_engine_check_version (GPGME_PROTOCOL_OpenPGP);
                //detectError(err);
                //End setup of GPGME

                // Create the GPGME Context
                err = gpgme_new (&ctx);
                // Error handling
                detectError(err);

                // Set the context to textmode
                gpgme_set_textmode (ctx, 1);
                // Enable ASCII armor on the context
                gpgme_set_armor (ctx, 1);


            qDebug() << "select 1 or 2";

            QTextStream qtin(stdin);
            //QString line = qtin.readLine();

            QString word;
            qtin >> word;

            if(word=="2"){

                printf("Enter key fingerprint or key UID:");

                QTextStream qtin(stdin);
                //QString line = qtin.readLine();

                QString word;
                qtin >> word;

                QByteArray ba = word.toLatin1();
                const char *patt=ba.data();

                gpgme_get_key(ctx, patt, &key, 1);

                if(key){
                    qDebug() << "FOUND";
                }
                else{
                    qDebug() << "NOT FOUND";
                }


            }
        }
    }
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

    serverKey(argc);

    /*

    //Create server.
    Server server(msg, usernameList, userKeyList, loginUser, loginRanNum, waitingTaskUser, waitingTaskWork, addFriendRequestList);

    //Start server.
    server.startServer();

    */


    // https://forum.qt.io/topic/86025/qt-get-external-ip-address-using-qnetworkreply/2

    return a.exec();
}
