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

#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include "OpenCLProgramConfiguration.h"
#include "Kernel.h"
#include "KernelInfo.h"
#include <QString>
#include <QFileInfo>
#include <QFile>
#include <QTextStream>
#include <QStringList>
#include <QDebug>

#include <QVector>

namespace Validation
{
namespace GUI
{

/**
 * @brief The RunVariant struct
 * @detailed Contains name and options to SATest.exe (--exapmle)
 */
struct RunVariant
{
    QString name;
    QStringList args;
    const QString toStr()
    {
        QString str = name + " [";
        for(int i=0; i<args.size(); i++)
            str+=" "+args[i];
        return str+"]";
    }
};

/**
 * @brief The Config struct
 * @detailed Contains params, parsed from *.cfg file
 */
struct Config
{
    bool vectorizer;
    bool vtune;
    QStringList includeDirs;
    QString compFlags;
};

/**
 * @brief The ConfigManager class
 * @detailed Implements all work with configuration with SATest project
 * @TODO add function for saving *.cfg
 * f.e. void saveCfg(Config cfg);
 */
class ConfigManager
{
public:
    typedef OpenCLProgramConfiguration ConfReader;
    /**
     * @brief ConfigManager constructor
     */
    ConfigManager();
    ~ConfigManager();
    /**
     * @brief setConfigFile
     * @param path to config file
     */
    void setConfigFile(QString path);
    /**
     * @brief getReader
     * @return directly pointer to OpenCLProgramConfiguration object
     */
    ConfReader* getReader();
    /**
     * @brief getTstFileName
     * @return name of *.tst file
     */
    QString getTstFileName();
    /**
     * @brief getTstFilePath
     * @return full path to *.tst file
     */
    QString getTstFilePath();
    /**
     * @brief workingDirectory
     * @return path to working directory
     */
    QString workingDirectory();
    /**
     * @brief getCfgFilePath
     * @return full path to *.cfg file
     */
    QString getCfgFilePath();
    /**
     * @brief getCfgFileName
     * @return name of *.cfg file
     */
    QString getCfgFileName() {return info.fileName();}
    /**
     * @brief getProgramName
     * @return program name from cfg file
     */
    QString getProgramName();
    /**
     * @brief getConfig
     * @return pointer to Config struct
     */
    Config* getConfig() {return config;}
    /**
     * @brief getArgs
     * @return run arguments to SATest.exe parsed from *.tst
     */
    QStringList getArgs();
    /**
     * @brief getKernels
     * @return vector of Kernel objects
     */
    QVector<Kernel*>* getKernels(){return kernels;}
    /**
     * @brief getRunVariants
     * @return vector of all run variants
     */
    QVector<RunVariant> getRunVariants(){return runVariants;}
    /**
     * @brief addRunVariant
     * @param var - run variant from dialog of creating new run variant by user
     */
    void addRunVariant(RunVariant var){runVariants.append(var);}
    /**
     * @brief updateVariant
     * @param pos
     * @param var
     */
    void updateVariant(int pos, RunVariant var) {runVariants[pos]=var;}
    /**
     * @brief removeVariant
     * @param pos
     */
    void removeVariant(int pos) {runVariants.remove(pos);}
    /**
     * @brief getFilePlainText
     * @param path
     * @return plain text from path
     */
    QString getFilePlainText(QString path);
    /**
     * @brief saveFilePlainText - save plain text to path
     * @param path
     * @param Text
     */
    void saveFilePlainText(QString path, QString Text);
private:
    OpenCLProgramConfiguration *reader;
    QFileInfo info;
    QStringList params;
    void parseTst();
    void parseKernels();
    void parseCfg();
    QVector<RunVariant> runVariants;
    QVector<Kernel*>* kernels;
    Config* config;


};

}
}

#endif // CONFIGMANAGER_H
