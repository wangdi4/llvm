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
