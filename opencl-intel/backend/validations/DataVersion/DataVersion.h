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

File Name:  DataVersion.h

\*****************************************************************************/
#ifndef __DATAVERSION_H__
#define __DATAVERSION_H__

#include <iomanip>

#include "IBufferContainerList.h"
#include "llvm/Metadata.h"


namespace Validation
{
    struct DataVersion
    {
        public:
            static void ConvertData (IBufferContainerList* pContainerList, llvm::NamedMDNode* metadata, std::string kernelName);

            static std::string GetDataVersionSignature() {
                return std::string("DataVersion ");
            }

            static uint32_t GetCurrentDataVersion() {
                return currentVersion;
            }

            // data version can be in the range from 00001 to 99999
            static uint32_t GetNumOfDigits() {
                static const uint32_t len = 5;
                return len;
            }

            static std::string GetCurrentDataVersionString() {
                std::stringstream ss;
                ss << std::setfill('0') << std::setw(GetNumOfDigits());
                ss << currentVersion;
                return ss.str();
            }
        private:
            /// current data version
            static const uint32_t currentVersion = 1;
    };

} // End of Validation namespace

#endif // __DATAVERSION_H__
