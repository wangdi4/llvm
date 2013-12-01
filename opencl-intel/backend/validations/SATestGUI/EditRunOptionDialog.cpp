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