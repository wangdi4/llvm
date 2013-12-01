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
#include "EditPathsDialog.h"
#include "Ui_EditPathsDialog.h"

namespace Validation
{
namespace GUI
{
EditPathsDialog::EditPathsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::EditPathsDialog)
{
    ui->setupUi(this);
    ui->ststLE->setText(AppSettings::instance()->getSATestPath());
    ui->wrkDirLE->setText(AppSettings::instance()->getWorkDir());
    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(check()));
    connect(ui->browseSTstPB, SIGNAL(clicked()), this, SLOT(selectSatest()));
    connect(ui->browseWrkDirPB, SIGNAL(clicked()), this, SLOT(selectWorkDir()));
}

EditPathsDialog::~EditPathsDialog()
{
    delete ui;
}

void EditPathsDialog::check()
{
    bool satest = QFile::exists(ui->ststLE->text());
    QDir dir(ui->wrkDirLE->text());
    bool wrkdir = dir.exists();
    QMessageBox message(this);
    if(!satest)
    {
        message.setText("SATest not foud!");
        message.exec();
    }
    if(!wrkdir)
    {
        message.setText("Working dir not found!");
        message.exec();
    }
    if(satest && wrkdir)
    {
        AppSettings::instance()->setSATestPath(ui->ststLE->text());
        AppSettings::instance()->setWorkDir(ui->wrkDirLE->text());
        this->accept();
    }

}

void EditPathsDialog::selectSatest()
{
    QString path = QFileDialog::getOpenFileName(this, "Select SATest file", QDir::currentPath());
    if(!path.isEmpty())
    {
        ui->ststLE->setText(path);
    }
}

void EditPathsDialog::selectWorkDir()
{
    QString directory = QFileDialog::getExistingDirectory(this,
                               "Select working directory", QDir::currentPath());
    if(!directory.isEmpty())
    {
        ui->wrkDirLE->setText(directory);
    }
}

}
}