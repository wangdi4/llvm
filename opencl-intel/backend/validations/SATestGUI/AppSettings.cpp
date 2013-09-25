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

#include "AppSettings.h"

namespace Validation
{
namespace GUI
{

AppSettings* AppSettings::p_instance = 0;


AppSettings::AppSettings()
{
    QString path = QCoreApplication::instance()->applicationDirPath()+"/settings.ini";
    m_settings = new QSettings(path, QSettings::IniFormat);
}

AppSettings::~AppSettings()
{
    if (p_instance)
        delete p_instance;

    delete m_settings;
}



const QList<EnvVar> AppSettings::getEnvVars()
{
    QList<EnvVar> vars;
    int size = m_settings->beginReadArray("Vars");
    for(int i = 0; i< size; i++)
    {
        m_settings->setArrayIndex(i);
        EnvVar var;
        var.name = m_settings->value("Name").toString();
        var.value = m_settings->value("Value").toString();
        vars.append(var);
    }
    m_settings->endArray();
    return vars;
}

void AppSettings::saveEnvVars(QList<EnvVar> vars)
{
    m_settings->beginWriteArray("Vars");
    for(int i=0; i< vars.size(); i++)
    {
        m_settings->setArrayIndex(i);
        m_settings->setValue("Name", vars[i].name);
        m_settings->setValue("Value", vars[i].value);
    }
    m_settings->endArray();
}

const QString AppSettings::getWorkDir()
{
    QString str = "";
    if(m_settings->contains("Work_dir"))
        str = m_settings->value("Work_dir").toString();
    return str;
}

void AppSettings::setWorkDir(QString path)
{
    m_settings->setValue("Work_dir", path);
}

const QString AppSettings::getSATestPath()
{
    QString str = "";
    if(m_settings->contains("SATest_path"))
        str = m_settings->value("SATest_path").toString();
    return str;
}

void AppSettings::setSATestPath(QString path)
{
    m_settings->setValue("SATest_path", path);
}

}

}