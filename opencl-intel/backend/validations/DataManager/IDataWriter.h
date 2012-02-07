/*****************************************************************************\

Copyright (c) Intel Corporation (2011).

    INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
    LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
    ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
    PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
    DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
    PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
    including liability for infringement of any proprietary rights, relating to
    use of the code. No license, express or implied, by estoppels or otherwise,
    to any intellectual property rights is granted herein.

File Name:  IDataWriter.h

\*****************************************************************************/
#ifndef __I_DATA_WRITER_H__
#define __I_DATA_WRITER_H__

#include "IContainer.h"

namespace Validation
{
    /// @brief Interface to a data writer. It saves object IContainer to storage
    class IDataWriter
    {
    public:
        /// @brief Write IContainer object to file
        /// @param [in] pContainer pointer to object with IContainer interface
        virtual void Write(const IContainer *pContainer) = 0;
    };

} // End of Validation namespace
#endif // __I_DATA_WRITER_H__
