//
// Copyright (C) 2018  Krittitam Juksukit
// This file is part of E2EEIM.
//
// E2EEIM is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License
// version 3 as published by the Free Software Foundation.
//
// E2EEIM is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with E2EEIM.  If not, see <http://www.gnu.org/licenses/>.
//
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidget>
#include "connection.h"
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(Connection &conn, QString activeUser,QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_pushButton_Contact_clicked();

    void on_pushButton_Conversation_clicked();

    void on_pushButton_Group_clicked();

    void on_pushButton_AddList_clicked();

    void listWidget_Contact_ItemClicked(QListWidgetItem* item);

    void on_pushButton_SEND_clicked();

    bool eventFilter(QObject *watched, QEvent *event);

private:
    Ui::MainWindow *ui;
    Connection *connection;
};

#endif // MAINWINDOW_H
