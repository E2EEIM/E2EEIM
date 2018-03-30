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
#ifndef CREATEGROUP_H
#define CREATEGROUP_H

#include <QDialog>
#include <QFile>
#include <QDebug>
#include <QTextStream>
#include <QStringList>
#include <QString>
#include <QList>

namespace Ui {
class CreateGroup;
}

class CreateGroup : public QDialog
{
    Q_OBJECT

public:
    explicit CreateGroup(QWidget *parent = 0, QString activeUser="");
    ~CreateGroup();

private slots:
    void on_pushButton_clicked();

private:
    Ui::CreateGroup *ui;
};

#endif // CREATEGROUP_H
