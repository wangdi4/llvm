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
