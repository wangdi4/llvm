/*****************************************************************************\

Copyright (c) Intel Corporation (2011-2012).

    INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
    LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
    ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
    PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
    DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
    PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
    including liability for infringement of any proprietary rights, relating to
    use of the code. No license, express or implied, by estoppels or otherwise,
    to any intellectual property rights is granted herein.

File Name:  RefALUVER.h


\*****************************************************************************/
#ifndef __RefALUVER_H__
#define __RefALUVER_H__

#include "llvm/Support/DataTypes.h"

namespace Validation
{
    class RefALUVersion {
    public:
        static uint64_t GetVersion() {
            return refALUVersion;
        }
    private:
        /// NEAT version to be incremented after every change in NEAT
        static const uint64_t refALUVersion = 1;
};
}
#endif // __RefALUVER_H__

