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

#include "RunDialog.h"
#include "Ui_RunDialog.h"

namespace Validation
{
namespace GUI
{

RunDialog::RunDialog(QVector<RunVariant> variants, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RunDialog)
{
    ui->setupUi(this);
    for(int i=0; i<variants.size(); i++)
    {
        QListWidgetItem* item = new QListWidgetItem(variants[i].toStr());
        ui->listWidget->insertItem(i,item);
    }
    ui->listWidget->setCurrentRow(0);
        index = 0;
    connect(ui->listWidget, SIGNAL(currentRowChanged(int)), this, SLOT(selectedItem(int)));
}

RunDialog::~RunDialog()
{
    delete ui;
}

void RunDialog::selectedItem(int index)
{
    this->index = index;
}

}
}
