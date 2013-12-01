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
#include "ConfigManager.h"

namespace Validation
{
namespace GUI
{

ConfigManager::ConfigManager()
{
    kernels = new QVector<Kernel*>;
}

ConfigManager::~ConfigManager()
{
    delete kernels;
}

void ConfigManager::setConfigFile(QString config)
{

    info.setFile(config);
    QString filepath = info.absoluteFilePath();
    QString dirpath = info.absolutePath();

    std::string path(filepath.toLocal8Bit().constData());
    std::string dir(dirpath.toLocal8Bit().constData());

    reader = new OpenCLProgramConfiguration(path, dir);

    parseTst();
    parseKernels();
    parseCfg();
}

ConfigManager::ConfReader *ConfigManager::getReader()
{
    return reader;
}

QString ConfigManager::getTstFileName()
{
    QString basename = info.baseName();
    QString _tstname = basename+".tst";
    return _tstname;
}

QString ConfigManager::getTstFilePath()
{
    QString path = info.absolutePath();
    return path+"/"+getTstFileName();
}

QString ConfigManager::workingDirectory()
{
    return info.absolutePath();
}

QString ConfigManager::getCfgFilePath()
{
    return info.absoluteFilePath();
}

QString ConfigManager::getProgramName()
{
    return QString::fromStdString(reader->GetProgramName());
}

QStringList ConfigManager::getArgs()
{
    return this->params;
}

QString ConfigManager::getFilePlainText(QString path)
{
    QFile file(path);
    QString text = "Can't open file";
    if(file.open(QIODevice::ReadWrite))
    {
        QTextStream stream ( &file );
        text = stream.readAll();
        file.close(); // when your done.
    }
    return text;
}

void ConfigManager::saveFilePlainText(QString path, QString text)
{
    QFile file(path);
    if(file.open(QIODevice::ReadWrite|QIODevice::Truncate))
    {
        QTextStream stream ( &file );
        stream<<text;
        file.close(); // when your done.
    }
}

void ConfigManager::parseTst()
{
    // open file
    QFile tstFile(getTstFilePath());
    if(!tstFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox msgBox;
        QString message = getTstFileName() + QString(" failed to open ") + tstFile.errorString();
        msgBox.setText(message);
        msgBox.exec();
        return;
    }

    //read file line by line
    QStringList lines;
    QTextStream in(&tstFile);
    while(!in.atEnd())
    {
        lines<<in.readLine();
    }
    // create "%s.cfg" and use it to find
    // required line
    qDebug()<<"parser";
    QString param = info.fileName();
    qDebug()<<param;
    param.replace(getTstFileName(), "%s");
    qDebug()<<param;

    // the parameter line may ends by '>' or "| FILECHECK"
    QString startword("SATest ");
    QString endword0(" |");
    QString endword1(" >");
    for(int i = 0; i<lines.size(); i++)
    {
        if(lines[i].contains(param))
        {
            // extract the parameters from the line
            int start = lines[i].indexOf(startword) + startword.size();
            int end = start;
            qDebug()<<"founded end word:";
            if(lines[i].contains(endword1))
            {
                end = lines[i].indexOf(endword1);
                qDebug()<<endword1;
            }
            else if(lines[i].contains(endword0))
            {
                end = lines[i].indexOf(endword0);
                qDebug()<<endword0;
            }
            qDebug()<<"start:";
            qDebug()<<start;
            qDebug()<<"end:";
            qDebug()<<end;
            qDebug()<<"len:";
            qDebug()<<end-start;
            // split the line to the separate parameters
            qDebug()<<"founded substring";
            QString arg = lines[i].mid(start, end-start);
            qDebug()<<arg;
            qDebug()<<"stringlist after split";
            QStringList arglist = arg.split(" ");
            qDebug()<<arglist;
            for(int j = 0; j<arglist.size(); j++)
            {
                if(arglist[j].contains(param))
                {
                    qDebug()<<"founded param";
                    qDebug()<<arglist[j];
                    arglist.removeAt(j);
                }
            }
            qDebug()<<"stringlist after remove -config= param";
            qDebug()<<arglist;
            RunVariant var;
            var.name = QString("From .tst file");
            var.args = arglist;
            runVariants.append(var);
            this->params = arglist;        }


    }
}

void ConfigManager::parseKernels()
{
    ConfigManager::ConfReader::KernelConfigList::const_iterator it;
    for(it=this->getReader()->beginKernels(); it!= this->getReader()->endKernels(); ++it)
    {
        KernelInfo kernel;
        kernel.name = QString::fromStdString((*it)->GetKernelName());
        kernel.infile = QString::fromStdString((*it)->GetInputFilePath());
        kernel.reffile = QString::fromStdString((*it)->GetReferenceFilePath());
        kernel.neatfile = QString::fromStdString((*it)->GetNeatFilePath());
        kernel.arrGlobalWorkOffset = (*it)->GetGlobalWorkOffset();
        kernel.arrGlobalWorkSize = (*it)->GetGlobalWorkSize();
        kernel.arrLocalWorkSize = (*it)->GetLocalWorkSize();
        kernel.workDimension = (*it)->GetWorkDimension();
        kernel.infileType = (*it)->GetInputFileType();
        kernel.reffileType =(*it)->GetReferenceFileType();
        kernel.neatfileType = (*it)->GetNeatFileType();
        Kernel* kern = new Kernel(kernel);
        kernels->append(kern);
    }
}

void ConfigManager::parseCfg()
{
    config = new Config();
    config->compFlags = QString::fromStdString(reader->GetCompilationFlags());
    OpenCLIncludeDirs* dirs = reader->GetIncludeDirs();
    if(dirs)
    {
        OpenCLIncludeDirs::IncludeDirsList::const_iterator it;
        for(it = dirs->beginIncldueDirs(); it!=dirs->endIncludeDirs(); ++it)
        {
            config->includeDirs<<QString::fromStdString((*it));
        }
    }
    config->vectorizer = reader->GetUseVectorizer();
}

}
}