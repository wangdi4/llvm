// INTEL CONFIDENTIAL
//
// Copyright 2010-2021 Intel Corporation.
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

#pragma once

#include "Compiler.h"
#include "ICompilerConfig.h"

#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Target/TargetMachine.h"

#include <string>

namespace llvm {
    class ExecutionEngine;
    class Module;
    class JITEventListener;
}

namespace Intel { namespace OpenCL { namespace DeviceBackend {

class BuiltinLibrary;
class BuiltinModules;

//*****************************************************************************************
// Provides the module optimization and code generation functionality.
//
class CPUCompiler: public Compiler
{
public:
    CPUCompiler(const ICompilerConfig& pConfig);
    virtual ~CPUCompiler();

    // Disable the copy ctor and assignment operator
    CPUCompiler( const CPUCompiler& ) = delete;
    bool operator = ( const CPUCompiler& ) = delete;

    // Create execution engine for the given module
    // Execution engine depends on module configuration
    void CreateExecutionEngine( llvm::Module* pModule ) override;

    // Get execution engine
    std::unique_ptr<llvm::ExecutionEngine> GetOwningExecutionEngine() override;

    // Create LLJIT instance
    std::unique_ptr<llvm::orc::LLJIT> CreateLLJIT(
        llvm::Module *M, std::unique_ptr<llvm::TargetMachine> TM,
        ObjectCodeCache *ObjCache) override;

    // Compile a module into a MemoryBuffer using llvm::orc::SimpleCompiler
    std::unique_ptr<llvm::MemoryBuffer> SimpleCompile(
        llvm::Module *module, ObjectCodeCache *objCache);

    void SetObjectCache(ObjectCodeCache* pCache) override;

    void SetBuiltinModules(const std::string& cpuName, const std::string& cpuFeatures) override;

    bool useLLDJITForExecution(llvm::Module* pModule) const override;
    bool isObjectFromLLDJIT(llvm::StringRef ObjBuf) const override;

    BuiltinModules* GetOrLoadBuiltinModules(bool ForceLoad = false);

protected:
    // Returns a list of pointers to the RTL library modules
    llvm::SmallVector<llvm::Module*, 2> &GetBuiltinModuleList() override;

private:
    void SelectCpu( const std::string& cpuName, const std::string& cpuFeatures );
    void CreateCPUExecutionEngine(llvm::Module* pModule);

private:
    std::unordered_map<std::thread::id, std::unique_ptr<BuiltinModules>>
        m_builtinModules;
    llvm::sys::Mutex        m_builtinModuleMutex;

    std::unique_ptr<llvm::ExecutionEngine> m_pExecEngine;

    std::unique_ptr<llvm::JITEventListener> m_pVTuneListener;
};

}}}
