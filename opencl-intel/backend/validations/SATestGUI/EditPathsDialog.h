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

#ifndef EDITPATHSDIALOG_H
#define EDITPATHSDIALOG_H

#include <QDialog>
#include <QMessageBox>
#include <QFile>
#include <QDir>
#include <QFileDialog>
#include "AppSettings.h"

namespace Ui {
class EditPathsDialog;
}

namespace Validation
{
namespace GUI
{

/**
 * @brief The EditPathsDialog class
 * @detailed shows dialog for add|update|remove path to SATest.exe and its working directory
 */
class EditPathsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EditPathsDialog(QWidget *parent = 0);
    ~EditPathsDialog();

private:
    Ui::EditPathsDialog *ui;
private slots:
    void check();
    void selectSatest();
    void selectWorkDir();
};


}
}
#endif // EDITPATHSDIALOG_H
