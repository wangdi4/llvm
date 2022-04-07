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

#include "SATestWrapper.h"

namespace Validation
{
namespace GUI
{

SATestWrapper::SATestWrapper()
{
    satest = new QProcess();
    connect(satest,SIGNAL(readyReadStandardOutput()), this, SLOT(readStdOut()));
    connect(satest,SIGNAL(readyReadStandardError()), this, SLOT(readStdErr()));
    connect(satest, SIGNAL(finished(int,QProcess::ExitStatus)),this,SIGNAL(finished(int,QProcess::ExitStatus)) );

}

SATestWrapper::~SATestWrapper()
{
    delete(satest);
}

void SATestWrapper::readStdErr()
{
    stdErrText = satest->readAllStandardError();
}

void SATestWrapper::readStdOut()
{
    stdOutText = satest->readAllStandardOutput();
}


void SATestWrapper::start(RunVariant option)
{
    prepareEnviroment();
    QString stamp = QDateTime::currentDateTime().toString();
    QString startmessage = stamp + ": starting SATEst.";
    log->append(startmessage);
    log->append("with arguments:");
    log->append(option.toStr());
    satest->setProcessEnvironment(this->env);
    QString path = AppSettings::instance()->getSATestPath();
    if(path!=""){
    QStringList args; //TODO add args!
    args = config->getArgs();
    args<<"-config="+config->getCfgFilePath();
    satest->setWorkingDirectory(config->workingDirectory());
    QString startString;
    startString+=path+" ";
    for(int i=0; i<args.size(); i++)
        startString+=args[i]+" ";
    log->append(startString);
    satest->start(path,args);
    }
    else
    {
        log->append("Can't start SATest. Set path to satest.exe!");
    }
}

void SATestWrapper::prepareEnviroment()
{
    this->env = QProcessEnvironment::systemEnvironment();
    QList<EnvVar> vars = AppSettings::instance()->getEnvVars();
    for(int i = 0; i< vars.size(); i++)
    {
        this->env.insert(vars[i].name, vars[i].value);
    }
}


QString SATestWrapper::stdErr()
{
    return stdErrText;
}

void SATestWrapper::setConfig(ConfigManager *cfg)
{
    this->config = cfg;
}

void SATestWrapper::setLogger(QTextEdit *edit)
{
    this->log = edit;
}

QString SATestWrapper::stdOut()
{
    return stdOutText;
}

}
}
