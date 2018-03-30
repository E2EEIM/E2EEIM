#ifndef MYTHREAD_H
#define MYTHREAD_H

#include <QObject>
#include <QThread>
#include <QTcpSocket>
#include <QDebug>
#include <QQueue>
#include <gpgme.h>

class MyThread : public QThread
{
    Q_OBJECT
public:
    explicit MyThread(QQueue<QByteArray> *queue, QList<QString> *usernameList,
                      QList<QString> *userKeyList, int ID, QObject *parent = 0);
    void run();

    QTcpSocket *socket;

signals:
    void error(QTcpSocket::SocketError socketError);
    void newdata(QByteArray data);



public slots:
    void readyRead();
    void disconnected();
    void send(QByteArray data);
    void dataFilter(QByteArray data);
    void printDataDetail(QByteArray data);
    QByteArray decryptData(QByteArray data, const char* outPutFileName);
    void addNewUser(QByteArray payload);
    QByteArray encryptToClient(QByteArray data,QString recipient, const char *outFileName);

    gpgme_key_t getKey(QString pattern);

private:
    int socketDescriptor;

    QQueue<QByteArray> *queuePtr;
    QList<QString> *usernameList;
    QList<QString> *userKeyList;

    gpgme_ctx_t ctx;  // the context
    gpgme_error_t err; // errors
    gpgme_key_t key= nullptr; // temp key
    gpgme_key_t senderKey;
    gpgme_key_t recipientKey;
    gpgme_key_t ServerKey = nullptr;

    QByteArray serverPubKey;
};

#endif // MYTHREAD_H
