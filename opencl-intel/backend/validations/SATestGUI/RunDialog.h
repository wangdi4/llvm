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

#ifndef RUNDIALOG_H
#define RUNDIALOG_H

#include "ConfigManager.h"
#include <QDialog>
#include <QListWidgetItem>

namespace Ui {
class RunDialog;
}

namespace Validation {
namespace GUI {

/**
 * @brief The RunDialog class
 * @detailed Dialog which shows when we click to "Run SATest" button. Shows all
 * variants of set of run options.
 */
class RunDialog : public QDialog {
  Q_OBJECT

public:
  /**
   * @brief RunDialog constructor
   * @param variants - vector of sets of run options
   */
  explicit RunDialog(QVector<RunVariant> variants, QWidget *parent = 0);
  ~RunDialog();
  /**
   * @brief getSelectedOption
   * @return index in run variants vector which selected by user
   */
  int getSelectedOption() { return index; }

private:
  Ui::RunDialog *ui;
  int index;
private slots:
  void selectedItem(int);
};

} // namespace GUI
} // namespace Validation

#endif // RUNDIALOG_H
