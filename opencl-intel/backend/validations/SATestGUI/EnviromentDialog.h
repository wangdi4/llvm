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

#ifndef ENVIROMENTDIALOG_H
#define ENVIROMENTDIALOG_H

#include <QDialog>
#include <QList>
#include <QTableWidgetItem>
#include "AppSettings.h"
#include "EditVariableDialog.h"
#include "EnvVar.h"

namespace Ui {
class EnviromentDialog;
}

namespace Validation
{
namespace GUI
{

/**
 * @brief The EnviromentDialog class
 * @detailed Shows dialog wich show a table with user specific enviroment variables
 * and buttons to add|del|edit this vars.
 */
class EnviromentDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EnviromentDialog(QWidget *parent = 0);
    ~EnviromentDialog();

private:
    QList<EnvVar> vars;
    Ui::EnviromentDialog *ui;
    void updateTable();
    void loadEnvVars();

private slots:
    void addNewVar();
    void editVar();
    void delVar();
    void saveEnvVars();
};

}
}

#endif // ENVIROMENTDIALOG_H
