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
#ifndef COMMANDARGUMENTSTAB_H
#define COMMANDARGUMENTSTAB_H
#include "Tab.h"
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidget>
#include <QPushButton>
#include <QVector>
#include "ConfigManager.h"
#include "EditRunOptionDialog.h"

namespace Validation
{
namespace GUI
{

/**
 * @brief The CommandArgumentsTab class
 * @detailed Tab where shows set of run options ( arguments) for SATest.exe
 */
class CommandArgumentsTab : public Tab
{
    Q_OBJECT
public:
    /**
     * @brief CommandArgumentsTab - constructor
     * @param cfg - pointer to ConfigManager object from which getting run options
     * @param parent
     */
    explicit CommandArgumentsTab(ConfigManager* cfg,QWidget *parent = 0);

signals:

public slots:
private slots:
    /**
     * @brief addNewVariant - action for click "New" button,
     * shows EditRunOptionDialog
     */
    void addNewVariant(void);
    /**
     * @brief removeVariant - remove selected run option
     */
    void removeVariant(void);
    /**
     * @brief editVariant - action for click "Edit" button,
     * shows EditRunOptionDialog
     */
    void editVariant(void);
private:
    QVBoxLayout* layout;
    QHBoxLayout* buttonLayout;
    QListWidget* runVariantsList;
    QPushButton *newBtn, *editBtn, *delBtn;
    ConfigManager* cfg;
    void updateListWidget();

};


}
}

#endif // COMMANDARGUMENTSTAB_H
