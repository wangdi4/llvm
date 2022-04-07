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

#ifndef APPSETTINGS_H
#define APPSETTINGS_H
#include <QSettings>
#include <QString>
#include <QCoreApplication>
#include <QList>
#include "envvar.h"

namespace Validation
{
namespace GUI
{

/**
 * @brief The AppSettings class
 * @detailed - SingleTon for application settings
 * @TODO - add functionality for save|load run aptions (set of arguments for SATest.exe)
 */
class AppSettings
{
public:
    static AppSettings * instance()
       {
           if(!p_instance)
               p_instance = new AppSettings();
           return p_instance;
       }

    void Release() {
        this->~AppSettings();
    }
    /**
     * @brief getEnvVars
     * @return specific enviroment variables created by user
     */
    const QList<EnvVar> getEnvVars();
    /**
     * @brief saveEnvVars - save specific enviroment variables created by user created in EditVariableDialog
     * @param vars
     */
    void saveEnvVars(const QList<EnvVar> vars);
    /**
     * @brief getWorkDir
     * @return working directory for SATest.exe selected by user in EditPathDialog
     */
    const QString getWorkDir();
    /**
     * @brief setWorkDir - set working directory for SATest.exe selected by user in EditPathDialog
     * @param path
     */
    void setWorkDir(const QString path);
    /**
     * @brief getSATestPath
     * @return path to SATest.exe selected by user in EditPathDialog
     */
    const QString getSATestPath();
    /**
     * @brief setSATestPath - set path to SATest.exe selected by user in EditPathDialog
     * @param path
     */
    void setSATestPath(const QString path);
private:
    AppSettings();
    ~AppSettings();
    const AppSettings& operator=( const AppSettings& );
    AppSettings(const AppSettings& root);
    static AppSettings * p_instance;
    QSettings *m_settings;
};

}
}

#endif // APPSETTINGS_H
