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

#ifndef CONFIGTAB_H
#define CONFIGTAB_H
#include "Tab.h"
#include "ConfigManager.h"
#include <QLabel>
#include <QCheckBox>
#include <QTextEdit>
#include <QLineEdit>
#include <QLabel>
#include <QFormLayout>

namespace Validation
{
namespace GUI
{

/**
 * @brief The ConfigTab class
 * @detailed Shows options from *.cfg file
 * @TODO add functionality for saving updated params
 */
class ConfigTab : public Tab
{
    Q_OBJECT
public:
    explicit ConfigTab(ConfigManager*);

signals:

public slots:
private:
    QFormLayout* layout;
    QCheckBox* vectorizer;
    QTextEdit* dirs;
    QCheckBox* vtune;
    QLineEdit* compFlags;
    ConfigManager* cfg;

};

}
}

#endif // CONFIGTAB_H
