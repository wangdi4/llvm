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
