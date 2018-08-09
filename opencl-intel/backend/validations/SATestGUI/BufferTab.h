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
