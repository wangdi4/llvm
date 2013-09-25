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
