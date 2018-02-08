#ifndef CONNECTION_H
#define CONNECTION_H

#include <QObject>
#include <QTcpSocket>
#include <QDebug>

class Connection : public QObject
{
    Q_OBJECT
public:
    explicit Connection(QObject *parent = nullptr);
    void connect();
    void send(QString msg);
    void letDisconnect();

signals:

public slots:

private:
    QTcpSocket *socket;
};

#endif // CONNECTION_H
