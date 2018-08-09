// INTEL CONFIDENTIAL
//
// Copyright 2013-2018 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

#include "EnviromentDialog.h"
#include "Ui_EnviromentDialog.h"

namespace Validation
{
namespace GUI
{
EnviromentDialog::EnviromentDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::EnviromentDialog)
{
    ui->setupUi(this);
    ui->varTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    loadEnvVars();
    updateTable();
    connect(ui->newBtn, SIGNAL(clicked()), this, SLOT(addNewVar()));
    connect(ui->editBtn, SIGNAL(clicked()), this, SLOT(editVar()));
    connect(ui->deleteBtn, SIGNAL(clicked()), this, SLOT(delVar()));
    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(saveEnvVars()));
}

EnviromentDialog::~EnviromentDialog()
{
    delete ui;
}

void EnviromentDialog::updateTable()
{
    ui->varTable->clear();
    QStringList headers;
    headers<<"Name"<<"Value";
    ui->varTable->setHorizontalHeaderLabels(headers);

    ui->varTable->setRowCount(vars.size());
    for(int i=0; i<vars.size(); i++)
    {
        QTableWidgetItem* nameitm = new QTableWidgetItem(vars[i].name);
        QTableWidgetItem* valitm = new QTableWidgetItem(vars[i].value);
        ui->varTable->setItem(i,0,nameitm);
        ui->varTable->setItem(i,1,valitm);
    }
}

void EnviromentDialog::loadEnvVars()
{
    vars = AppSettings::instance()->getEnvVars();
}

void EnviromentDialog::saveEnvVars()
{
    AppSettings::instance()->saveEnvVars(vars);
}
void EnviromentDialog::addNewVar()
{
    EditVariableDialog dlg;
    if(dlg.exec() == QDialog::Accepted)
    {
        EnvVar var;
        var.name = dlg.name();
        var.value = dlg.value();
        vars.append(var);
        updateTable();
    }
}

void EnviromentDialog::editVar()
{
    int row = ui->varTable->currentRow();
    QString name = ui->varTable->item(row,0)->text();
    QString value = ui->varTable->item(row,1)->text();
    EditVariableDialog dlg(name, value);
    if(dlg.exec() == QDialog::Accepted)
    {
        EnvVar var;
        var.name = dlg.name();
        var.value = dlg.value();
        qDebug()<<var.name;
        qDebug()<<var.value;
        vars[row]=var;
        updateTable();
    }
}

void EnviromentDialog::delVar()
{
    int row = ui->varTable->currentRow();
    vars.removeAt(row);
    updateTable();
}

}
}
