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
#include "EditVariableDialog.h"
#include "Ui_EditVariableDialog.h"

namespace Validation
{
namespace GUI
{
EditVariableDialog::EditVariableDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::EditVariableDialog)
{
    ui->setupUi(this);
    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(updateFields()));
}

EditVariableDialog::EditVariableDialog(QString name, QString value, QWidget *parent):
    QDialog(parent),
    ui(new Ui::EditVariableDialog)
{
    ui->setupUi(this);
    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(updateFields()));
    ui->nameTE->setText(name);
    m_name = name;
    ui->valueTE->setText(value);
    m_value = value;

}

EditVariableDialog::~EditVariableDialog()
{
    delete ui;
}

void EditVariableDialog::updateFields()
{
    m_name = ui->nameTE->text();
    m_value = ui->valueTE->text();
}

}
}