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
#ifndef SATESTWRAPPER_H
#define SATESTWRAPPER_H
#include <QObject>
#include <QProcess>
#include <QProcessEnvironment>
#include <QStringList>
#include <QDateTime>
#include <QString>
#include "AppSettings.h"
#include "EnvVar.h"
#include <QList>
#include "ConfigManager.h"
#include <QTextEdit>

namespace Validation
{
namespace GUI
{
/**
 * @brief The SATestWrapper class
 * @detailed class ralize work with SATest.exe - run with defined by user run options, get stdout and stderr streams,
 *  set specific Enviroment Variables, etc
 */
class SATestWrapper: public QObject
{
    Q_OBJECT
public:
    /**
     * @brief SATestWrapper constructor
     */
    SATestWrapper();
    ~SATestWrapper();
    /**
     * @brief stdOut
     * @return all output from SATest.exe to stdout
     */
    QString stdOut();
    /**
     * @brief stdErr
     * @return all output from SATest.exe to stderr
     */
    QString stdErr();
    /**
     * @brief setConfig
     * @detailed config manager using to get specific enviroment variables and run options
     * @param cfg - pointer to ConfigManager object
     */
    void setConfig(ConfigManager* cfg);
    /**
     * @brief setLogger
     * @detailed QTextEdit using to write messages and error. This widget shows in Main Window
     * @param te - pointer to QTextEdit
     */
    void setLogger(QTextEdit* te);
signals:
    /**
     * @brief finished signal emmits when SATest.exe finished
     */
    void finished(int,QProcess::ExitStatus);
public slots:
    /**
     * @brief start SATest.exe
     * @param option - run options selected by user in RunDialog
     */
    void start(RunVariant option);
private:
    QTextEdit* log;
    QProcess* satest;
    QStringList args;
    QString path;
    QProcessEnvironment env;
    QString stdOutText;
    QString stdErrText;
    void prepareEnviroment();
    ConfigManager* config;
private slots:
    void readStdErr();
    void readStdOut();


};

}
}
#endif // SATESTWRAPPER_H
