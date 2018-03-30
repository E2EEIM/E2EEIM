#ifndef CONNECTION_H
#define CONNECTION_H

#include <QObject>
#include <QDebug>
#include <gpgme.h>
#include <QTcpSocket>

class Connection : public QObject
{
    Q_OBJECT
public:
    explicit Connection(QObject *parent = nullptr);
    void letConnect();
    void send(QByteArray data);
    void letDisconnect();
    QByteArray getActiveUserPubKey();
    void updateServPubKey(QByteArray data);
    QString getActiveUserName();
    QByteArray encryptToServ(QByteArray data, const char *outFileName);

signals:
    void newDataArrived(QByteArray data);

public slots:
    void readyRead();
    void dataFilter(QByteArray data);

private:
    QTcpSocket *socket;

    gpgme_ctx_t ctx;  // the context
    gpgme_error_t err; // errors
    gpgme_key_t key= nullptr; // temp key
    gpgme_key_t senderKey;
    gpgme_key_t recipientKey;
    gpgme_key_t ServerKey = nullptr;

    gpgme_key_t activeUserKey=nullptr;

    QByteArray activeUserPubKey;
};

#endif // CONNECTION_H
