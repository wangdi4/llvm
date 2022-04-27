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
