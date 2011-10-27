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

File Name:  IContainer.h

\*****************************************************************************/
#ifndef __ICONTAINER_H__
#define __ICONTAINER_H__

namespace Validation
{
    class IContainerVisitor;
    /// @brief IContainer marker interface
    /// Intended to mark all data container objects managed by DataManager
    class IContainer
    {
    public:
        /// @brief virtual destructor stub
        virtual ~IContainer(){}
        /// @brief visitor accept method
        virtual void Accept( IContainerVisitor& visitor) const = 0;
    };
}

#endif // __ICONTAINER_H__
