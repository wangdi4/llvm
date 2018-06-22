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
