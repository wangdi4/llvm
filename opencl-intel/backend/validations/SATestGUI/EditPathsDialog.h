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
