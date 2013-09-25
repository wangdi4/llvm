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
#ifndef EDITTAB_H
#define EDITTAB_H

#include <QWidget>
#include <QString>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QFile>
#include "OCLHighlighter.h"
#include "Tab.h"
#include "ConfigManager.h"

namespace Validation
{
namespace GUI
{
/**
 * @brief The EditTab class
 * @detailed This Tab shows plain text data from selected file
 */
class EditTab : public Tab
{
public:
    EditTab(ConfigManager* cfg, QString path);
    void save();
private:
    QTextEdit* textEdit;
    OCLHighlighter* highlighter;
    QString path;
    ConfigManager* cfg;
};

}
}
#endif // EDITTAB_H
