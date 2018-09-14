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
