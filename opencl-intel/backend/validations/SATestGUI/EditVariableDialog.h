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
