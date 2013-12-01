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