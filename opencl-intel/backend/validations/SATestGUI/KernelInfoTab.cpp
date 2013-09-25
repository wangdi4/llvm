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
#include "KernelInfoTab.h"

namespace Validation
{
namespace GUI
{

KernelInfoTab::KernelInfoTab(Kernel* kernel, int kernelId, QWidget *parent):Tab(parent)
{
    this->kernelId = kernelId;
    layout = new QFormLayout(this);
    layout->addRow("Kernel: ", new QLabel(kernel->name()));
    layout->addRow("Input file: ", new QLabel(kernel->inFileName()));

    inBuffers = new QListWidget(this);
    for(int i = 0; i<kernel->getInBuffers()->size(); i++)
    {
        QListWidgetItem* item = new QListWidgetItem(QString(QString("Buffer ")+QString::number(i)));
        inBuffers->addItem(item);
        inBuffersEntries.insert(item, i);
    }

    QVBoxLayout* vlayout = new QVBoxLayout(this);
    vlayout->addWidget(inBuffers);
    layout->addRow("Buffers", vlayout);
    ///
    layout->addRow("Neat file: ", new QLabel(kernel->neatFileName()));

    layout->addRow("Ref file: ", new QLabel(kernel->refFileName()));
    this->setLayout(layout);

    connect(inBuffers, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(bufferSelected(QListWidgetItem*)));
}


void KernelInfoTab::bufferSelected(QListWidgetItem *item)
{
    int bufferIndex = inBuffersEntries.value(item);
    Entry entry;
    qDebug()<<"Buffer Index "<<bufferIndex;
    entry.bufferNum = bufferIndex;
    entry.buftype = Entry::inBuffer;
    entry.kernelId = kernelId;
    emit viewBuffer(entry);
}

}
}