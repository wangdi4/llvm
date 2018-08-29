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

#ifndef EDITVARIABLEDIALOG_H
#define EDITVARIABLEDIALOG_H

#include <QDialog>
#include <QString>
#include <QDebug>

namespace Ui {
class EditVariableDialog;
}

namespace Validation
{
namespace GUI
{

/**
 * @brief The EditVariableDialog class
 * @detailed This dialog can add|update user specific enviroment variable
 */
class EditVariableDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EditVariableDialog(QWidget *parent = 0);
    EditVariableDialog(QString name, QString value, QWidget *parent = 0);
    ~EditVariableDialog();
    QString name() {return m_name;}
    QString value() {return m_value;}

private:
    Ui::EditVariableDialog *ui;
    QString m_name, m_value;
private slots:
    void updateFields();
};

}
}

#endif // EDITVARIABLEDIALOG_H
