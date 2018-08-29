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
