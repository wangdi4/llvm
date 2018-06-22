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

#ifndef KERNELINFOTAB_H
#define KERNELINFOTAB_H
#include <QWidget>
#include "Tab.h"
#include "Kernel.h"
#include "Entry.h"
#include <QFormLayout>
#include <QLabel>
#include <QVBoxLayout>
#include <QListWidget>
#include <QListWidgetItem>
#include <QHash>

namespace Validation
{
namespace GUI
{
/**
 * @brief The KernelInfoTab class
 * @detailed Tab, which show information about kernel - names of buffer files, list of buffers, etc
 * @TODO add list widgets to ref and neat buffers and information from kernelInfo struct.
 */
class KernelInfoTab : public Tab
{
        Q_OBJECT
public:
    KernelInfoTab(Kernel* kernel,int kernelId, QWidget *parent=0);
signals:
    /**
     * @brief viewBuffer emmits when we clicked to selected buffer in list of buffers
     */
    void viewBuffer(Entry);
private:
    int kernelId;
    QFormLayout* layout;
    QListWidget *inBuffers, *outBuffers, *refBuffers;
    QHash<QListWidgetItem*, int> inBuffersEntries;
private slots:
    void bufferSelected(QListWidgetItem*);
};

}
}
#endif // KERNELINFOTAB_H
