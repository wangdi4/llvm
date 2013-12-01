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
#ifndef BUFFERTAB_H
#define BUFFERTAB_H
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include "BufferConverter.h"
#include "Tab.h"
#include <QTableWidget>
#include <QWidget>
#include <QLabel>
#include <QTableView>
#include "BufferModel.h"
#include <QModelIndex>
#include <QHeaderView>

namespace Validation
{
namespace GUI
{

/**
 * @brief The BufferTab class
 * @detailed This Tab show buffer as a table with value-line, where user can modify value from selected cell
 */
class BufferTab : public Tab
{
    Q_OBJECT
public:
    /**
      * @brief BufferTab - default constructor
      * @param buffer - Buffer object from which getting data execute
      */
     BufferTab(BufferConverter *buffer);

signals:

public slots:
private:
     BufferConverter* buffer;
     QVBoxLayout* layout;
     QHBoxLayout* typeInfoLine;
     QHBoxLayout* editValueLine;
     QPushButton* editBtn;
     QLineEdit* valueLE;
     QComboBox *hexTableCB, *hexValueCB;
     QTableWidget* table;
     QTableView* tableView;
     BufferModel* model;

private slots:
     void changeTableView();
     void changeValueView();
     void showValue(QModelIndex);
     void changeValue();

};

}
}
#endif // BUFFERTAB_H
