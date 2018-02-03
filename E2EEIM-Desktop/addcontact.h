#ifndef ADDCONTACT_H
#define ADDCONTACT_H

#include <QDialog>

namespace Ui {
class AddContact;
}

class AddContact : public QDialog
{
    Q_OBJECT

public:
    explicit AddContact(QWidget *parent = 0, QString activeUser="");
    ~AddContact();

private slots:
    void on_pushButton_clicked();

private:
    Ui::AddContact *ui;
};

#endif // ADDCONTACT_H
