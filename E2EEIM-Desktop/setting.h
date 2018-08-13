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

#ifndef SETTING_H
#define SETTING_H

#include <QDialog>
#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QDebug>
#include <QTextStream>
#include <QStringList>
#include <QString>
#include <QList>
#include <QListWidgetItem>


namespace Ui {
class Setting;
}

class Setting : public QDialog
{
    Q_OBJECT

public:
    explicit Setting(QString activeUser, QString currentMenu, QString currentConversation, QWidget *parent = 0);
    ~Setting();

    QString getCurrentConversationStatus();

signals:

private slots:
    QStringList readContact(QString Filename);

    void listWidget_Conversation_ItemClicked(QListWidgetItem* item);
    void listWidget_Contact_ItemClicked(QListWidgetItem* item);
    void listWidget_Group_ItemClicked(QListWidgetItem* item);
    void deleteItem(QStringList deleteList, QString fileName);
    void on_pushButton_Delete_clicked();
    void tabSelected();
    void on_pushButton_Leave_clicked();

    void on_pushButton_DeleteConversation_clicked();

    void on_pushButton_clicked();

private:
    Ui::Setting *ui;
    QString ActiveUser;
    QStringList deleteList;
    QString deleteInFile;
    QString currentTab;
    QString currentConversation;
    QString currentConversationStatus;
};

#endif // SETTING_H
