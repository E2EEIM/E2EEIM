#ifndef CLIENTTHREAD_H
#define CLIENTTHREAD_H

#include <QObject>
#include <QThread>

class ClientThread : public QThread
{
    Q_OBJECT
public:
    ClientThread();
    void run();

signals:
    void newLoop();

public slots:
    void slowThread();
    void endThread();
};

#endif // CLIENTTHREAD_H
