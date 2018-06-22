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
