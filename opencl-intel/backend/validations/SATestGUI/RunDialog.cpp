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