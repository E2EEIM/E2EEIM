#include "clientthread.h"

ClientThread::ClientThread()
{

}
void ClientThread::run(){

    emit newLoop();

    exec();


}
//Slow thread for client to read data.
void ClientThread::slowThread(){
    sleep(1);
}

//Stop thread.
void ClientThread::endThread(){
    this->quit();
}
