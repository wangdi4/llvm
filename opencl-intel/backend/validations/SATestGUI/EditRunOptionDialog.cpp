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

#include "EditRunOptionDialog.h"
#include "Ui_EditRunOptionDialog.h"

namespace Validation
{
namespace GUI
{
EditRunOptionDialog::EditRunOptionDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::EditRunOptionDialog)
{
    ui->setupUi(this);
}

EditRunOptionDialog::EditRunOptionDialog(RunVariant variant,QWidget *parent):
    QDialog(parent),
    ui(new Ui::EditRunOptionDialog)
{

    ui->setupUi(this);
    ui->nameLE->setText(variant.name);
    ui->argsLE->setText(variant.args.join(" "));
}

RunVariant EditRunOptionDialog::getVariant()
{
    RunVariant variant;
    variant.args=ui->argsLE->text().split(" ");
    variant.name= ui->nameLE->text();
    return variant;
}

EditRunOptionDialog::~EditRunOptionDialog()
{
    delete ui;
}

}
}
