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
