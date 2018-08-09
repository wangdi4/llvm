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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDockWidget>
#include <QTextEdit>
#include <QTreeWidget>
#include <QTabWidget>
#include <QWidget>
#include <QAction>
#include <QString>
#include <QMenu>
#include <QTreeWidgetItem>
#include <QProcess>
#include <QByteArray>
#include <QDebug>
#include "SATestWrapper.h"
#include "EnviromentDialog.h"
#include "EditPathsDialog.h"
#include "RunDialog.h"
#include "TabWidget.h"
#include "FileTree.h"
#include "ConfigManager.h"
#include "OpenCLProgramConfiguration.h"
#include <QDateTime>

namespace Ui {
class MainWindow;
}

namespace Validation
{
namespace GUI
{
/**
 * @brief The MainWindow class
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

    QDockWidget* filetreedock;
    QDockWidget* stoutputdock;
    ConfigManager* cfg;
    FileTree* filetree;
    TabWidget* tabs;
    QTextEdit* outputtext;

    QAction* openfileaction;
    QAction* closefileaction;
    QAction* quitappaction;
    QAction* runsatestaction;
    QAction* showvardialogaction;
    QAction* editpathsaction;
    QAction* saveAction;
    QMenu* filemenu;

    SATestWrapper* satest;
    /**
     * @brief createWidgets
     */
    void createWidgets();
    /**
     * @brief createActions
     */
    void createActions();
    void example();
private slots:
    void updateOutputFromOut();
    void updateOutputFromErr();
    void runsatest();
    void updatetext(int,QProcess::ExitStatus);
    void showEnvDialog();
    void showEditPathDialog();
    void addEditTab(QString);
};


}
}
#endif // MAINWINDOW_H
