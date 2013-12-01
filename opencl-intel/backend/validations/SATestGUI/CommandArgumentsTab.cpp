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
#include "CommandArgumentsTab.h"

namespace Validation
{
namespace GUI
{

CommandArgumentsTab::CommandArgumentsTab(ConfigManager *cfg, QWidget *parent) :
    Tab(parent)
{
    this->cfg = cfg;
    layout = new QVBoxLayout();
    runVariantsList = new QListWidget(this);
    layout->addWidget(runVariantsList);
    buttonLayout = new QHBoxLayout();
    newBtn = new QPushButton("New");
    editBtn = new QPushButton("Edit");
    delBtn = new QPushButton("Remove");
    buttonLayout->addWidget(newBtn);
    buttonLayout->addWidget(editBtn);
    buttonLayout->addWidget(delBtn);
    layout->addLayout(buttonLayout);
    setLayout(layout);
    updateListWidget();
    connect(newBtn, SIGNAL(clicked()), this, SLOT(addNewVariant()));
    connect(editBtn, SIGNAL(clicked()), this, SLOT(editVariant()));
    connect(delBtn, SIGNAL(clicked()), this, SLOT(removeVariant()));
}

void CommandArgumentsTab::addNewVariant()
{
    EditRunOptionDialog dlg;
    if(dlg.exec()==QDialog::Accepted)
    {
        RunVariant var = dlg.getVariant();
        cfg->addRunVariant(var);
        updateListWidget();
    }
}

void CommandArgumentsTab::removeVariant()
{
    int pos = runVariantsList->currentRow();
    cfg->removeVariant(pos);
    updateListWidget();
}

void CommandArgumentsTab::editVariant()
{
     RunVariant var = this->cfg->getRunVariants().at(runVariantsList->currentRow());
    EditRunOptionDialog dlg(var);
    if(dlg.exec()==QDialog::Accepted)
    {
        RunVariant newvar = dlg.getVariant();
        cfg->updateVariant(runVariantsList->currentRow(), newvar);
        updateListWidget();
    }
}


void CommandArgumentsTab::updateListWidget()
{
    runVariantsList->clear();
    for(int i =0; i<this->cfg->getRunVariants().size(); i++)
    {
        RunVariant var = this->cfg->getRunVariants().at(i);
        runVariantsList->addItem(var.toStr());
    }

    runVariantsList->setCurrentRow(0);
}

}
}