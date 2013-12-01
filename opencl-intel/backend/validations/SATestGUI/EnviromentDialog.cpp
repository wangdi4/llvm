/*****************************************************************************\

Copyright (c) Intel Corporation (2013).

    INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
    LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
    ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
    PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
    DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
    PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
    including liability for infringement of any proprietary rights, relating to
    use of the code. No license, express or implied, by estoppels or otherwise,
    to any intellectual property rights is granted herein.
\*****************************************************************************/
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