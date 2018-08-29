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

#ifndef COMMANDARGUMENTSTAB_H
#define COMMANDARGUMENTSTAB_H
#include "Tab.h"
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidget>
#include <QPushButton>
#include <QVector>
#include "ConfigManager.h"
#include "EditRunOptionDialog.h"

namespace Validation
{
namespace GUI
{

/**
 * @brief The CommandArgumentsTab class
 * @detailed Tab where shows set of run options ( arguments) for SATest.exe
 */
class CommandArgumentsTab : public Tab
{
    Q_OBJECT
public:
    /**
     * @brief CommandArgumentsTab - constructor
     * @param cfg - pointer to ConfigManager object from which getting run options
     * @param parent
     */
    explicit CommandArgumentsTab(ConfigManager* cfg,QWidget *parent = 0);

signals:

public slots:
private slots:
    /**
     * @brief addNewVariant - action for click "New" button,
     * shows EditRunOptionDialog
     */
    void addNewVariant(void);
    /**
     * @brief removeVariant - remove selected run option
     */
    void removeVariant(void);
    /**
     * @brief editVariant - action for click "Edit" button,
     * shows EditRunOptionDialog
     */
    void editVariant(void);
private:
    QVBoxLayout* layout;
    QHBoxLayout* buttonLayout;
    QListWidget* runVariantsList;
    QPushButton *newBtn, *editBtn, *delBtn;
    ConfigManager* cfg;
    void updateListWidget();

};


}
}

#endif // COMMANDARGUMENTSTAB_H
