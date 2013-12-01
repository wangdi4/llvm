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
#ifndef RUNDIALOG_H
#define RUNDIALOG_H

#include <QDialog>
#include <QListWidgetItem>
#include "ConfigManager.h"

namespace Ui {
class RunDialog;
}

namespace Validation
{
namespace GUI
{

/**
 * @brief The RunDialog class
 * @detailed Dialog which shows when we click to "Run SATest" button. Shows all variants of set of run options.
 */
class RunDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief RunDialog constructor
     * @param variants - vector of sets of run options
     */
    explicit RunDialog(QVector<RunVariant> variants,QWidget *parent = 0);
    ~RunDialog();
    /**
     * @brief getSelectedOption
     * @return index in run variants vector which selected by user
     */
    int getSelectedOption(){return index;}

private:
    Ui::RunDialog *ui;
    int index;
private slots:
    void selectedItem(int);
};

}
}

#endif // RUNDIALOG_H
