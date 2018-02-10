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
    explicit Setting(QString activeUser, QWidget *parent = 0);
    ~Setting();

private slots:
    QStringList readContact(QString Filename);
    void listWidget_Contact_ItemClicked(QListWidgetItem* item);
    void deleteItem(QStringList deleteList, QString fileName);
    void on_pushButton_Delete_clicked();
    void tabSelected();
    void on_pushButton_Leave_clicked();

    void on_pushButton_DeleteConversation_clicked();

private:
    Ui::Setting *ui;
    QString ActiveUser;
    QStringList deleteList;
    QString deleteInFile;
    QString currentTab;
};

#endif // SETTING_H
