#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidget>
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
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
};

#endif // MAINWINDOW_H
