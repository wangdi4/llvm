// INTEL CONFIDENTIAL
//
// Copyright 2010-2018 Intel Corporation.
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
