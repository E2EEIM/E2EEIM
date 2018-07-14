#include "clientthread.h"

ClientThread::ClientThread()
{

}
void ClientThread::run(){

    emit newLoop();

    exec();


}

void ClientThread::slowThread(){
    sleep(1);
}
void ClientThread::endThread(){
    this->quit();
}
