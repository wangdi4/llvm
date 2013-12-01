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
