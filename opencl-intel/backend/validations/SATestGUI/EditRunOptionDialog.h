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
#ifndef EDITRUNOPTIONDIALOG_H
#define EDITRUNOPTIONDIALOG_H

#include <QDialog>
#include "ConfigManager.h"

namespace Ui {
class EditRunOptionDialog;
}

namespace Validation
{
namespace GUI
{

/**
 * @brief The EditRunOptionDialog class
 * @detailed shows dialog for add|edit run options
 */
class EditRunOptionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EditRunOptionDialog(QWidget *parent = 0);
    EditRunOptionDialog(RunVariant variant,QWidget *parent=0);

    RunVariant getVariant();
    ~EditRunOptionDialog();

private:
    Ui::EditRunOptionDialog *ui;

};

}
}
#endif // EDITRUNOPTIONDIALOG_H
