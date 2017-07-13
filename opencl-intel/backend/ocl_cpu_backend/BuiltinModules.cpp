/*****************************************************************************\

Copyright (c) Intel Corporation (2010).

    INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
    LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
    ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
    PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
    DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
    PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
    including liability for infringement of any proprietary rights, relating to
    use of the code. No license, express or implied, by estoppels or otherwise,
    to any intellectual property rights is granted herein.

File Name:  BuiltinModules.cpp

\*****************************************************************************/

#include "BuiltinModules.h"
#include "llvm/Support/MemoryBuffer.h"

namespace Intel { namespace OpenCL { namespace DeviceBackend {

BuiltinModules::BuiltinModules(llvm::SmallVector<llvm::Module*, 2> builtinsModules):
    m_BuiltinsModules(builtinsModules)
{
}

BuiltinModules::~BuiltinModules() { }

BuiltinLibrary::BuiltinLibrary(const Intel::CPUId &cpuId):
    m_cpuId(cpuId),
    m_pRtlBuffer(nullptr),
    m_pRtlBufferSvmlShared(nullptr)
{
}

BuiltinLibrary::~BuiltinLibrary()
{ }

}}}
