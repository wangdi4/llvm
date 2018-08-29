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

#ifndef TABWIDGET_H
#define TABWIDGET_H

#include <QTabWidget>
#include <QList>
#include <QFileInfo>
#include <QDebug>
#include "EditTab.h"
#include "Tab.h"
#include "Entry.h"
#include "ConfigManager.h"
#include "Buffer.h"
#include "BufferTab.h"
#include "Kernel.h"
#include "KernelInfoTab.h"
#include "CommandArgumentsTab.h"
#include "ConfigTab.h"
#include <QPair>
#include <QAction>

namespace Validation
{
namespace GUI
{

/**
 * @brief The TabWidget class
 * @detailed Widget wich show tabs - childrens of Tab class
 */
class TabWidget : public QTabWidget
{
    Q_OBJECT
public:
    /**
     * @brief TabWidget constructor
     * @param parent
     */
    explicit TabWidget(QWidget *parent = 0);
    /**
     * @brief addTab - TODO remove this function if it's dont need
     */
    void addTab(QString path, Tab::TabType type);
    /**
     * @brief setConfig - save pointer to ConfigManager object
     * @param cfg
     */
    void setConfig(ConfigManager* cfg) {this->cfg = cfg;}
    /**
     * @brief setSaveAction - set action from "Save" button from ToolBar from MainWindow
     */
    void setSaveAction(QAction*);

signals:

public slots:
    /**
     * @brief showBuffer - show BufferTab with information from buffer
     * @param entry
     */
    void showBuffer(Entry entry);
    /**
     * @brief showRawFile - show file as a text in EditTab
     * @param entry
     */
    void showRawFile(Entry entry);
    /**
     * @brief showKernel - show information about kernel in KernelTab
     */
    void showKernel(Entry);
    /**
     * @brief showRunVariants - show set of run options (parsed from *.tst or *.ini file) in CommandArgumentsTab
     */
    void showRunVariants(Entry);
    /**
     * @brief showConfig - show options of config (parsed from *.cfg) in ConfigTab
     */
    void showConfig(Entry);
    /**
     * @brief catchSaveAction - cath action from "Save" button and do some actions what need depends on current Tab
     */
    void catchSaveAction();
private:
    /**
     * @brief checkTab - check if tab is currently oppenned. if openned - show its, else - create tab and set focus on its
     * @return true or false
     */
    bool checkTab(Entry);
    QList<Tab*> tabs;
    QList<QPair<Entry,Tab*>> opennedTabs;
    ConfigManager* cfg;
    QAction* saveAction;

private slots:
    void closeTab(int);


};

}
}
#endif // TABWIDGET_H
