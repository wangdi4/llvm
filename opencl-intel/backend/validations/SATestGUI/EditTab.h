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

#ifndef EDITTAB_H
#define EDITTAB_H

#include <QWidget>
#include <QString>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QFile>
#include "OCLHighlighter.h"
#include "Tab.h"
#include "ConfigManager.h"

namespace Validation
{
namespace GUI
{
/**
 * @brief The EditTab class
 * @detailed This Tab shows plain text data from selected file
 */
class EditTab : public Tab
{
public:
    EditTab(ConfigManager* cfg, QString path);
    void save();
private:
    QTextEdit* textEdit;
    OCLHighlighter* highlighter;
    QString path;
    ConfigManager* cfg;
};

}
}
#endif // EDITTAB_H
