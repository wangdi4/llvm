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

#include "MainWindow.h"
#include "Ui_MainWindow.h"
#include <string>
#include <list>
#include "XMLDataReader.h"
#include "BufferContainerList.h"
#include "BufferContainer.h"
#include "Buffer.h"
#include <QByteArray>

namespace Validation
{
namespace GUI
{

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    createWidgets();
    createActions();
    example();

    //
    QString config = QFileDialog::getOpenFileName(this, "Select config file", QDir::currentPath(),"*.cfg");
    this->setWindowTitle(config);
    cfg = new ConfigManager();
    cfg->setConfigFile(config);
    //
    filetree->setConfig(cfg);
    satest = new SATestWrapper();
    satest->setConfig(cfg);
    tabs->setConfig(cfg);
    tabs->setSaveAction(saveAction);
    satest->setLogger(this->outputtext);

    connect(satest, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(updatetext(int,QProcess::ExitStatus)));
    connect(filetree, SIGNAL(showRawFile(Entry)), tabs, SLOT(showRawFile(Entry)));
    connect(filetree, SIGNAL(showBuffer(Entry)), tabs, SLOT(showBuffer(Entry)));
    connect(filetree, SIGNAL(showKernelInfo(Entry)), tabs, SLOT(showKernel(Entry)));
    connect(filetree, SIGNAL(showConfig(Entry)), tabs, SLOT(showConfig(Entry)));
    connect(filetree, SIGNAL(showRunVariants(Entry)), tabs, SLOT(showRunVariants(Entry)));
}

MainWindow::~MainWindow()
{
    AppSettings::instance()->Release();
    delete ui;
}

void MainWindow::createWidgets()
{
    filetreedock = new QDockWidget(this);
    filetreedock->setWindowTitle("File tree");
    this->addDockWidget(Qt::LeftDockWidgetArea, filetreedock);
    filetreedock->setFeatures(QDockWidget::NoDockWidgetFeatures);

    stoutputdock = new QDockWidget(this);
    stoutputdock->setWindowTitle("Output");
    this->addDockWidget(Qt::BottomDockWidgetArea,stoutputdock);
    stoutputdock->setFeatures(QDockWidget::NoDockWidgetFeatures);

    filetree = new FileTree(filetreedock);
    filetreedock->setWidget(filetree);
    tabs = new TabWidget(this);
    tabs->setTabsClosable(true);
    this->setCentralWidget(tabs);
    outputtext = new QTextEdit(stoutputdock);
    stoutputdock->setWidget(outputtext);


}

void MainWindow::createActions()
{
    openfileaction = new QAction(QString("Open"),this);
    closefileaction = new QAction(QString("Close file"),this);
    quitappaction = new QAction(QString("Quit"),this);
    runsatestaction = new QAction(QIcon(":/icons/Run.png"),QString("Run SATest"),this);
    filemenu = menuBar()->addMenu(QString("File"));
    filemenu->addAction(openfileaction);
    filemenu->addAction(closefileaction);
    filemenu->addSeparator();
    filemenu->addAction(runsatestaction);
    filemenu->addSeparator();
    filemenu->addAction(quitappaction);

    saveAction = new QAction(QIcon(":/icons/Save.png"),QString("Save"),this);
    ui->mainToolBar->addAction(saveAction);

    ui->mainToolBar->addAction(runsatestaction);
    connect(runsatestaction, SIGNAL(triggered()), this,SLOT(runsatest())) ;
    showvardialogaction = new QAction(QIcon(":/icons/Env.png"),QString("Edit Enviroment Variables"), this);
    ui->mainToolBar->addAction(showvardialogaction);
    connect(showvardialogaction, SIGNAL(triggered()), this, SLOT(showEnvDialog()));
    editpathsaction = new QAction(QIcon(":/icons/Path.png"),QString("Edit paths"), this);
    ui->mainToolBar->addAction(editpathsaction);
    connect(editpathsaction, SIGNAL(triggered()), this, SLOT(showEditPathDialog()));
}

void MainWindow::example()
{
    filetree->setContextMenuPolicy(Qt::ActionsContextMenu);
    filetree->addAction(closefileaction);
}

void MainWindow::updateOutputFromOut()
{
}

void MainWindow::updateOutputFromErr()
{
}

void MainWindow::runsatest()
{
    RunDialog dlg(cfg->getRunVariants());
    if(dlg.exec()==QDialog::Accepted)
    {
        qDebug()<<"run option"<<dlg.getSelectedOption();
        satest->start(cfg->getRunVariants().at(dlg.getSelectedOption()));
    }


}

void MainWindow::updatetext(int exitCode, QProcess::ExitStatus exitStatus)
{
    qDebug()<<"Status: "<<exitStatus;
    qDebug()<<"Code: "<<exitCode;
    QString str;
    str+="Exit Code: "+QString::number(exitCode)+"\n";
    str+="Exit Status: "+QString::number(exitStatus)+"\n";
    str+="stdout:\n";
    str+=satest->stdOut();
    str+="stderr:\n";
    str+=satest->stdErr();
    this->outputtext->append(str);
}

void MainWindow::showEnvDialog()
{
    qDebug()<<"dialog!";
    EnviromentDialog dlg(this);
    dlg.exec();
}

void MainWindow::showEditPathDialog()
{
    EditPathsDialog dlg(this);
    dlg.exec();
}

void MainWindow::addEditTab(QString path)
{
    tabs->addTab(path, Tab::Editor);
}

}
}
