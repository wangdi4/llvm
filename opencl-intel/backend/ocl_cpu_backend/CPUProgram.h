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

// NOTICE: THIS CLASS WILL BE SERIALIZED TO THE DEVICE, IF YOU MAKE ANY CHANGE
//  OF THE CLASS FIELDS YOU SHOULD UPDATE THE SERILIZE METHODS
#pragma once

#include "Program.h"
#include "ObjectCodeCache.h"
#include <memory>

namespace llvm {
  class ExecutionEngine;
  class Function;
}

namespace Intel { namespace OpenCL { namespace DeviceBackend {

class CPUProgram : public Program
{
public:
    CPUProgram() : m_pExecutionEngine(nullptr) {}
    virtual ~CPUProgram();

    virtual void SetBuiltinModule(llvm::SmallVector<llvm::Module*, 2> bltnFuncList) { m_bltnFuncList = bltnFuncList; }

    virtual void SetExecutionEngine(void* ee) override {
        m_pExecutionEngine = (llvm::ExecutionEngine*)ee;
    }

    llvm::ExecutionEngine* GetExecutionEngine() { return m_pExecutionEngine; }

    void SetLLJIT(std::unique_ptr<LLJIT2> LLJIT) override {
        m_LLJIT = std::move(LLJIT);
    }

    LLJIT2* GetLLJIT() override { return m_LLJIT.get(); }

    void ReleaseExecutionEngine();

    void* GetPointerToFunction(llvm::Function* F);

    /**
     * Serialization methods for the class (used by the serialization service)
     */
    virtual void Deserialize(IInputStream& ist, SerializationStatus* stats);

    void SetObjectCache(ObjectCodeCache *);

    cl_ulong GetFunctionPointerFor(const char *) const override;

    // Get a map from global variable name to its property (size/pointer).
    void GetGlobalVariablePointers(cl_prog_gv_map &GVs) const override;

private:
    llvm::ExecutionEngine*  m_pExecutionEngine;
    std::unique_ptr<LLJIT2> m_LLJIT;
    llvm::SmallVector<llvm::Module*, 2> m_bltnFuncList;
    std::auto_ptr<ObjectCodeCache> m_ObjectCodeCache;

private:
    // Disable copy ctor and assignment operator
    CPUProgram( const CPUProgram& );
    bool operator = (const CPUProgram& );

};

}}} // namespace
