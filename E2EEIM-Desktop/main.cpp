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
#include "signin.h"
#include "connection.h"
#include "encryption.h"
#include <QApplication>
#include <QDebug>

void closeMainWindow(){

    //window.close();

    qDebug() << "closeMainWindowxxxx";
    Encryption encryption;
    Connection connection(encryption);

    QString activeUser="";
    SignIn signIn(connection, encryption);
    signIn.setModal(false);

    if(signIn.exec() == QDialog::Accepted)
    {
        activeUser  = signIn.getActiveUser();
    }

    MainWindow w(activeUser);
    w.setWindowTitle("E2EEIM-"+activeUser);
    w.show();
    qDebug() << "closeMainWindowyyyy";
}

int main(int argc, char *argv[])
{
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);    // High-DPI Scaling support
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);       // High-DPI Icons support

    QApplication a(argc, argv);

    Encryption encryption;
    Connection connection(encryption);

    QString activeUser="";
    SignIn signIn(connection, encryption);
    signIn.setModal(false);

    if(signIn.exec() == QDialog::Accepted)
    {
        activeUser  = signIn.getActiveUser();
    }

    MainWindow w(activeUser);
    w.setWindowTitle("E2EEIM-"+activeUser);
    w.show();

    QObject::connect(&w, &MainWindow::closeWindow, closeMainWindow);




    if(activeUser==""){
      return 0;
    }
    //w.deleteLater();
    return a.exec();

}
