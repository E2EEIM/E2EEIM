#include "mainwindow.h"
#include "signin.h"
#include <QApplication>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);    // High-DPI Scaling support
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);       // High-DPI Icons support

    QApplication a(argc, argv);

    QString activeUser="";
    SignIn signIn;
    signIn.setModal(true);

    if(signIn.exec() == QDialog::Accepted)
    {
        activeUser  = signIn.getActiveUser();
    }

    MainWindow w(activeUser);
    w.setWindowTitle("E2EEIM-"+activeUser);
    w.show();

    if(activeUser==""){
      return 0;
    }

    return a.exec();

}
