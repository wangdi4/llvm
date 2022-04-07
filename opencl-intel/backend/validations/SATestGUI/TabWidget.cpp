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

#include "tabwidget.h"

namespace Validation
{
namespace GUI
{

TabWidget::TabWidget(QWidget *parent) :
    QTabWidget(parent)
{
    connect(this, SIGNAL(tabCloseRequested(int)), this, SLOT(closeTab(int)));


}

void TabWidget::addTab(QString path, Tab::TabType type)
{
}

void TabWidget::setSaveAction(QAction *action)
{
    this->saveAction = action;
    connect(this->saveAction, SIGNAL(triggered()), this, SLOT(catchSaveAction()));
}

void TabWidget::showBuffer(Entry entry)
{
    if(!checkTab(entry))
    {
        qDebug()<<"creating new buffertab";
        qDebug()<<entry.kernelId;
        qDebug()<<entry.bufferNum;
        Kernel* kernel = cfg->getKernels()->at(entry.kernelId);
        BufferConverter* buf;
        QString caption;
        if(entry.buftype == Entry::inBuffer)
        {
            caption = "In buffer "+QString::number(entry.bufferNum);;
            buf = kernel->getInBuffers()->at(entry.bufferNum);
        }
        else if(entry.buftype == Entry::refBuffer)
        {
            caption = "Ref buffer "+QString::number(entry.bufferNum);
            buf = kernel->getInBuffers()->at(entry.bufferNum);
        }
        else
        {
            caption = "Neat buffer "+QString::number(entry.bufferNum);
            buf = kernel->getInBuffers()->at(entry.bufferNum);
        }
        BufferTab* tab = new BufferTab(buf);
        QTabWidget::addTab(tab,caption);
        setCurrentWidget(tab);
        QPair<Entry,Tab*> pair(entry,tab);
        opennedTabs.append(pair);
    }
}

void TabWidget::showRawFile(Entry entry)
{
    if(!checkTab(entry))
    {
        EditTab* tab = new EditTab(cfg,entry.path);
        QTabWidget::addTab(tab,QFileInfo(entry.path).fileName());
        setCurrentWidget(tab);
        QPair<Entry,Tab*> pair(entry,tab);
        opennedTabs.append(pair);
    }
}

void TabWidget::showKernel(Entry entry)
{
    if(!checkTab(entry))
    {
        Kernel* kernel = cfg->getKernels()->at(entry.kernelId);
        KernelInfoTab* tab = new KernelInfoTab(kernel, entry.kernelId);
        QTabWidget::addTab(tab,"kernel");
        connect(tab, SIGNAL(viewBuffer(Entry)), this, SLOT(showBuffer(Entry)));
        setCurrentWidget(tab);
        QPair<Entry,Tab*> pair(entry,tab);
        opennedTabs.append(pair);
    }
}

void TabWidget::closeTab(int index)
{
    QTabWidget::removeTab(index);

    opennedTabs.removeAt(index);
}

void TabWidget::showRunVariants(Entry entry)
{
    if(!checkTab(entry))
    {
        CommandArgumentsTab* tab = new CommandArgumentsTab(cfg, this);
        QTabWidget::addTab(tab, "run options");
        setCurrentWidget(tab);
        QPair<Entry,Tab*> pair(entry,tab);
        opennedTabs.append(pair);
    }
}

void TabWidget::showConfig(Entry entry)
{
    if(!checkTab(entry))
    {
        ConfigTab* tab = new ConfigTab(cfg);
        QTabWidget::addTab(tab,cfg->getCfgFileName());
        setCurrentWidget(tab);
        QPair<Entry,Tab*> pair(entry,tab);
        opennedTabs.append(pair);
    }
}

void TabWidget::catchSaveAction()
{
    Tab* currentTab = (Tab*)this->currentWidget();
    Entry currentEntry;
    QPair<Entry,Tab*> pair;
    foreach (pair, opennedTabs) {
        if(pair.second == currentTab)
        {
            currentEntry = pair.first;
            break;
        }
    }
    if(currentTab->type == Tab::Buf)
    {
        Kernel* kernel = cfg->getKernels()->at(currentEntry.kernelId);
        switch (currentEntry.buftype) {
        case Entry::inBuffer:
        {
            kernel->saveInBuffer();
        }
            break;
        case Entry::refBuffer:
        {
            kernel->saveRefBuffer();
        }
            break;
        case Entry::neatBuffer:
        {
            kernel->saveNeatBuffer();
        }
            break;
        default:
            break;
        }
    }
    else if(currentTab->type==Tab::Editor)
    {
        EditTab *tab = (EditTab*)(currentTab);
        if(tab)
        {
            tab->save();
        }
    }
}

bool TabWidget::checkTab(Entry entry)
{   QPair<Entry,Tab*> pair;
    foreach (pair, opennedTabs) {
        if(pair.first == entry)
        {
            this->setCurrentWidget(pair.second);
            return true;
        }
    }
    return false;
}

}
}
