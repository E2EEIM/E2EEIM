#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);    // High-DPI Scaling support
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);       // High-DPI Icons support

    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
