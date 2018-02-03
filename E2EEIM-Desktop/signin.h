#ifndef SIGNIN_H
#define SIGNIN_H

#include <QDialog>

namespace Ui {
class SignIn;
}

class SignIn : public QDialog
{
    Q_OBJECT

public:
    explicit SignIn(QWidget *parent = 0);
    QString getActiveUser();
    ~SignIn();

private slots:
    void on_pushButton_clicked();

private:
    Ui::SignIn *ui;
};

#endif // SIGNIN_H
