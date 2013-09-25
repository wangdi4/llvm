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