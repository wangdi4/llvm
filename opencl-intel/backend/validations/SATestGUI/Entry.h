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
#ifndef ENTRY_H
#define ENTRY_H
#include <QString>
#include <QTreeWidgetItem>

namespace Validation
{
namespace GUI
{

/**
 * @brief The Entry struct
 * @detailed This entry is very important part of all program architecture.
 * It can describe items in FileTree widget and FileTree widget send it to TabWidget to create
 * new tab. If you want create new file in SATest architecture and want to show it in this program
 * You must add in thi struct new type or buffer type.
 */
struct Entry
{
    enum Type{
        configuration,
        cl,
        neat,
        ref,
        in,
        tst,
        buffer,
        kernel,
        root
    };
    enum BufferType
    {
        inBuffer,
        refBuffer,
        neatBuffer
    };

    Type type ;
    BufferType buftype ;
    QString path;
    int kernelId ;
    int bufferNum ;
    /** @TODO rewrite this operator
     *  At now monent we have a bug when we create new tab by clicking to buffer item in Filetree
     *  and if we click to this buffer in KernelTab we don't set focus to BufferTab with this buffer
     *  but create new BufferTab.
     */
    bool operator ==(Entry entry)
    {
        if(entry.type != this->type)
            return false;
        else
        {
            if(entry.type == buffer)
            {
                return (entry.bufferNum==this->bufferNum &&
                        entry.kernelId == this->kernelId &&
                        entry.buftype == this->buftype);
            }
            if(entry.type == kernel)
            {
                return entry.kernelId == this->kernelId;
            }
            else
            {
                return entry.path == this->path;
            }
        }
    }
};


}
}

#endif // ENTRY_H
