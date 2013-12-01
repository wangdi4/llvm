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
#ifndef CONFIGTAB_H
#define CONFIGTAB_H
#include "Tab.h"
#include "ConfigManager.h"
#include <QLabel>
#include <QCheckBox>
#include <QTextEdit>
#include <QLineEdit>
#include <QLabel>
#include <QFormLayout>

namespace Validation
{
namespace GUI
{

/**
 * @brief The ConfigTab class
 * @detailed Shows options from *.cfg file
 * @TODO add functionality for saving updated params
 */
class ConfigTab : public Tab
{
    Q_OBJECT
public:
    explicit ConfigTab(ConfigManager*);

signals:

public slots:
private:
    QFormLayout* layout;
    QCheckBox* vectorizer;
    QTextEdit* dirs;
    QCheckBox* vtune;
    QLineEdit* compFlags;
    ConfigManager* cfg;

};

}
}

#endif // CONFIGTAB_H
