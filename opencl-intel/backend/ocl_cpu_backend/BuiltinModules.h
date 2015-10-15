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

File Name:  BuiltinModules.h

\*****************************************************************************/
#pragma once // <--- TODO: Add proper INCLUDE_GUARD

#include "llvm/ADT/SmallVector.h"
#include "llvm/Support/Errno.h"
#include "llvm/Support/MemoryBuffer.h"
#include <assert.h>
#include <string>
#include <memory>
#include "cl_dev_backend_api.h"
#include "CPUDetect.h"
#include "IDynamicFunctionsResolver.h"

namespace llvm
{
class Module;
class LLVMContext;
}

namespace Intel { namespace OpenCL { namespace DeviceBackend {

class BuiltinModules
{
public:
    BuiltinModules(llvm::SmallVector<llvm::Module*, 2> builtinsModules);
    ~BuiltinModules();

    llvm::SmallVector<llvm::Module*, 2> GetBuiltinModuleList() { return m_BuiltinsModules; }

private:
    int m_cpuId;

    llvm::SmallVector<llvm::Module*, 2> m_BuiltinsModules;
};

class BuiltinLibrary : public IDynamicFunctionsResolver
{
public:
    BuiltinLibrary(const Intel::CPUId&);
    virtual ~BuiltinLibrary();

    std::unique_ptr<llvm::MemoryBuffer> GetRtlBuffer() const { return std::unique_ptr<llvm::MemoryBuffer>(m_pRtlBuffer); }
    std::unique_ptr<llvm::MemoryBuffer> GetRtlBufferSvmlShared() const { return std::unique_ptr<llvm::MemoryBuffer>(m_pRtlBufferSvmlShared); }

    ECPU GetCPU() const { return m_cpuId.GetCPU();}

    virtual void SetContext(const void* pContext)
    {
        assert(false && "Set Builtin Library Context Not Implemented");
    }
    virtual unsigned long long int GetFunctionAddress(const std::string& functionName) const
    {
        assert(false && "Get Function Address Not Implemented");
        return 0;
    }

    virtual void Load() = 0;

protected:
    const Intel::CPUId   m_cpuId;
    llvm::MemoryBuffer* m_pRtlBuffer;
    llvm::MemoryBuffer* m_pRtlBufferSvmlShared;
};


}}}
