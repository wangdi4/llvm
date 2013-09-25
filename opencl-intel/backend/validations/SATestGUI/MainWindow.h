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
