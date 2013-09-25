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
#include "FileTree.h"
#include <QDebug>

namespace Validation
{
namespace GUI
{

static QString getFileName(QString path)
{
    QFileInfo info(path);
    return info.fileName();
}
static QString getFileName(std::string path)
{
    return getFileName(QString::fromStdString(path));
}
static QTreeWidgetItem* treeWidgetFactory(QTreeWidgetItem* parent, QString text)
{
    QTreeWidgetItem *item = new QTreeWidgetItem(parent);
    item->setText(0,text);
    return item;
}
static QTreeWidgetItem* treeWidgetFactory(QTreeWidget* parent, QString text)
{
    QTreeWidgetItem *item = new QTreeWidgetItem(parent);
    item->setText(0,text);
    return item;
}
FileTree::FileTree(QWidget *parent) :
    QTreeWidget(parent)
{
    rawAction =  new QAction("View raw data",this);
    this->setContextMenuPolicy(Qt::ActionsContextMenu);
    this->addAction(rawAction);
    connect(this, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)), this, SLOT(catchDoubleClicked(QTreeWidgetItem*,int)));
    connect(rawAction, SIGNAL(triggered()), this, SLOT(editRawFileRequest()));

}

void FileTree::setConfig(ConfigManager *cfg)
{
    this->cfg = cfg;
    this->setHeaderLabel(QString::fromStdString(cfg->getReader()->GetProgramName()));
    ///
    addEntry( treeWidgetFactory(this, cfg->getCfgFileName()),
              Entry::configuration,
              cfg->getCfgFilePath());
    addEntry( treeWidgetFactory(this, cfg->getTstFileName()),
              Entry::tst,
              cfg->getTstFilePath());
    addEntry( treeWidgetFactory(this, getFileName(cfg->getReader()->GetProgramFilePath())),
              Entry::cl,
              QString::fromStdString(cfg->getReader()->GetProgramFilePath()));

    QString kernelsStr = "Kernels ("+QString::number(cfg->getReader()->GetNumberOfKernelConfigurations())+")";
    QTreeWidgetItem* kernelRoot = treeWidgetFactory(this, kernelsStr);

    QVector<Kernel*>* kernels = cfg->getKernels();
    for(int i = 0; i< kernels->size(); i++)
    {
        Kernel* kernel = kernels->at(i);
        QTreeWidgetItem* kernelItem = treeWidgetFactory(kernelRoot, kernel->name());
        addEntry(kernelItem, Entry::kernel,i);
        QTreeWidgetItem* in = treeWidgetFactory(kernelItem,kernel->inFileName());
        addEntry(in,
                 Entry::in,
                 kernel->inFilePath());
        QVector<BufferConverter*>* inbuffers = kernel->getInBuffers();
        for(int j = 0; j< inbuffers->size(); j++)
        {
            addEntry(treeWidgetFactory(in, QString("Buffer")+QString::number(j)),
                     Entry::buffer,
                     Entry::inBuffer,
                     i,
                     j);
        }

        QTreeWidgetItem* ref = treeWidgetFactory(kernelItem, kernel->refFileName());
        addEntry(ref,
                 Entry::ref,
                 kernel->refFileName());
        QVector<BufferConverter*>* refbuffers = kernel->getRefBuffers();
        for(int j = 0; j< refbuffers->size(); j++)
        {
            addEntry(treeWidgetFactory(ref, QString("Buffer")+QString::number(j)),
                     Entry::buffer,
                     Entry::refBuffer,
                     i,
                     j);
        }

        QTreeWidgetItem* neat = treeWidgetFactory(kernelItem, kernel->neatFileName());
        addEntry(neat,
                 Entry::neat,
                 kernel->neatFilePath());
        QVector<BufferConverter*>* neatbuffers = kernel->getNeatBuffers();
        for(int j = 0; j< neatbuffers->size(); j++)
        {
            addEntry(treeWidgetFactory(neat, QString("Buffer")+QString::number(j)),
                     Entry::buffer,
                     Entry::neatBuffer,
                     i,
                     j);
        }
    }
}

Entry FileTree::getEntry(QTreeWidgetItem *itm)
{
    return entries[itm];
}

void FileTree::addEntry(QTreeWidgetItem *itm, Entry::Type type, QString path)
{
    Entry entry;
    entry.type = type;
    entry.path = path;
    entries.insert(itm,entry);
}

void FileTree::addEntry(QTreeWidgetItem *itm, Entry::Type type, Entry::BufferType buftype, int kernelId, int bufferNum)
{
    Entry entry;
    entry.type = type;
    entry.buftype = buftype;
    entry.kernelId = kernelId;
    entry.bufferNum = bufferNum;
    entries.insert(itm,entry);
}

void FileTree::addEntry(QTreeWidgetItem *itm, Entry::Type type, int kernelId)
{
    Entry entry;
    entry.type = type;
    entry.kernelId = kernelId;
    entries.insert(itm,entry);
}

void FileTree::catchDoubleClicked(QTreeWidgetItem *itm, int idx)
{
    if(entries.contains(itm))
    {
        Entry entry = getEntry(itm);
        if(entry.type == Entry::buffer)
        {
            qDebug()<<"open buffer";
            qDebug()<<entry.kernelId;
            qDebug()<<entry.bufferNum;
            emit showBuffer(entry);
        }
        else if(entry.type == Entry::kernel)
        {
            qDebug()<<"open kernel info";
            qDebug()<<"kernel # "<<QString::number(entry.kernelId);
            emit showKernelInfo(entry);
        }
        else if(entry.type == Entry::configuration)
        {
            emit showConfig(entry);
        }
        else if(entry.type == Entry::tst)
        {
            qDebug()<<"show run variants";
            emit showRunVariants(entry);
        }
        else if (entry.type == Entry::cl)
        {
            emit showRawFile(entry);
        }
        else
        {
            qDebug()<<"view entry";
            qDebug()<<entry.type;
            //emit editRawFile(entry->path);
        }
    }
}

void FileTree::editRawFileRequest()
{
    QTreeWidgetItem* item = this->currentItem();
    Entry entry = getEntry(item);
    if(entry.type == Entry::configuration || entry.type == Entry::cl)
    {
        emit showRawFile(entry);
    }
}


}
}