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

#include "cl_dev_backend_api.h"
#include "cl_cpu_detect.h"
#include "ICompilerConfig.h"

#include "llvm/ADT/SmallVector.h"
#include "llvm/ExecutionEngine/Orc/LLJIT.h"
#include "llvm/Support/raw_ostream.h"

#include <map>
#include <string>
#include <thread>
#include <unordered_map>

namespace llvm {
    class ExecutionEngine;
    class Function;
    class LLVMContext;
    class MemoryBuffer;
    class Module;
    class TargetMachine;
    class Type;
}

extern bool s_ignore_termination;

namespace Intel { namespace OpenCL { namespace DeviceBackend {
class BuiltinLibrary;
class BuiltinModules;
class ProgramBuildResult;
class ObjectCodeCache;

namespace Utils
{
/**
 * Class used to block the destruction of Compiler internal data
 * after backend dll unload
 *
 * Problem:
 * OCL SDK permit declaration of static OCL object (like Context)
 * that may hold the pointer to backend object (CompilerService for example)
 * The problem with such approach is that upon application termination the
 * backend dll could be unloaded prior to the above static object destruction.
 *
 * Solution:
 * We define the special static object ( that would be initilized after
 * any other static object in OCL SDK ). Upon backend dll unload this static object's
 * destructor will be called first and will set the special flag. Using this flag,
 * other backend objects (Compiler, CompilerService) may decide to abort their
 * desctuction in order to avoid crashes.
 *
 */
class TerminationBlocker
{
public:
    ~TerminationBlocker() { s_released = true; s_ignore_termination = true;}
    static bool IsReleased() { return s_released; }
private:
    static bool s_released;
};
}

//*****************************************************************************************
// Represent the build specific options passed to the compiler upon BuildProgram call
//
class CompilerBuildOptions
{
public:
    CompilerBuildOptions(const char* pBuildOpts);

    bool GetDisableOpt()    const { return m_disableOpt; }
    bool GetDebugInfoFlag() const { return m_debugInfo; }
    bool GetUseNativeDebuggerFlag() const { return m_useNativeDebugger; }
    bool GetProfilingFlag() const { return m_profiling; }
    bool GetRelaxedMath()   const { return m_relaxedMath; }
    bool GetUniformWGSize() const { return m_uniformWGSize; }
    int  GetAPFLevel()      const { return m_APFLevel; }
    bool GetDenormalsZero() const { return m_denormalsZero;}

    void SetDisableOpt(bool disableOpt)       { m_disableOpt = disableOpt; }
    void SetDebugInfoFlag(bool debugInfo)     { m_debugInfo = debugInfo; }
    void SetUseNativeDebuggerFlag(bool V)     { m_useNativeDebugger = V; };
    void SetProfilingFlag(bool profiling)     { m_profiling = profiling; }
    void SetRelaxedMath(bool relaxedMath)     { m_relaxedMath = relaxedMath; }
    void SetUniformWGSize(bool uniformWGSize) { m_uniformWGSize = uniformWGSize; }
    void SetAPFLevel(int APFLevel)            { m_APFLevel = APFLevel; }
    void SetDenormalsZero(bool denormalsZero) { m_denormalsZero = denormalsZero;}

private:
    bool m_debugInfo;
    bool m_useNativeDebugger;
    bool m_profiling;
    bool m_disableOpt;
    bool m_relaxedMath;
    bool m_denormalsZero;
    bool m_uniformWGSize;
    int  m_APFLevel;
};

//*****************************************************************************************
// Represents the result of program build Holds the build result code and build log
//
class ProgramBuildResult
{
public:
    ProgramBuildResult();

    bool Succeeded() const;

    bool Failed() const;

    std::string GetBuildLog() const;

    llvm::raw_ostream& LogS();

    void SetBuildResult( cl_dev_err_code code );

    cl_dev_err_code GetBuildResult() const;

private:
    cl_dev_err_code m_result;
    std::string m_buildLog;
    mutable llvm::raw_string_ostream m_logStream;
};

//*****************************************************************************************
// Provides the module optimization and code generation functionality.
//
class Compiler
{
public:
    /**
     * Ctor
     */
    Compiler(const ICompilerConfig& config);
    virtual ~Compiler();

    /**
     * Initializes the LLVM environment.
     * Must be called from single threaded environment, before any
     * instance of Compiler class are created
     */
    static void Init();

    /**
     * Initialize the global options
     */
    static void InitGlobalState( const IGlobalCompilerConfig& config );

    /**
     * Terminate the LLVM environment.
     * Must be called from a single threaded environment, after all
     * the Compiler class instanced are destroyed
     */
    static void Terminate();

    /**
     * Build the given program using the supplied build options
     */
    llvm::Module* BuildProgram(
        llvm::Module*, const char* pBuildOpts, ProgramBuildResult* pResult,
        std::unique_ptr<llvm::TargetMachine> &targetMachine);

    Intel::OpenCL::Utils::CPUDetect *GetCpuId() const { return m_CpuId; }

    // Create execution engine
    virtual void CreateExecutionEngine(llvm::Module* m)  = 0;

    // Get the latest execution engine
    virtual std::unique_ptr<llvm::ExecutionEngine>
    GetOwningExecutionEngine() = 0;

    // Create LLJIT instance
    virtual std::unique_ptr<llvm::orc::LLJIT> CreateLLJIT(
        llvm::Module *M, std::unique_ptr<llvm::TargetMachine> TM,
        ObjectCodeCache *ObjCache) = 0;

    // Get Function Address Resolver
    virtual void *GetFunctionAddressResolver() { return NULL; }

    // Returns a list of pointers to the RTL libraries
    virtual llvm::SmallVector<llvm::Module*, 2> &GetBuiltinModuleList() = 0;

    std::unique_ptr<llvm::Module> ParseModuleIR(llvm::MemoryBuffer* pIRBuffer);

    const std::string GetBitcodeTargetTriple(const void* pBinary, size_t uiBinarySize) const;

    virtual void SetObjectCache(ObjectCodeCache* pCache) = 0;

    virtual void SetBuiltinModules(const std::string & /*cpuName*/,
                                   const std::string & /*cpuFeatures*/ = "") {}

    virtual std::string& getBuiltinInitLog() {
        return m_builtinInitLog;
    }

    void materializeSpirTriple(llvm::Module *M);

    virtual bool useLLDJITForExecution(llvm::Module* pModule) const = 0;
    virtual bool isObjectFromLLDJIT(llvm::StringRef ObjBuf) const = 0;

    // Reset and create a new LLVMContext for current thread.
    // Returns the new LLVMContext.
    llvm::LLVMContext * resetLLVMContextForCurrentThread();

protected:
    SmallVector<std::unique_ptr<Module>, 2> LoadBuiltinModules(BuiltinLibrary *
                                                               pLibrary);

    // Create TargetMachine for X86.
    llvm::TargetMachine* GetTargetMachine(llvm::Module* pModule) const;

    virtual void setBuiltinInitLog(std::string& log) {
       m_builtinInitLog = log;
    }
protected:
    bool                     m_bIsFPGAEmulator;
    // Each host thread should have its own LLVMContext, because it is not
    // thread-safe for multiple threads to access LLVM resources within a
    // single LLVMContext.
    std::unordered_map<std::thread::id, std::unique_ptr<llvm::LLVMContext>>
        m_LLVMContexts;
    // LLVMContext is reset when CL_CONFIG_REPLACE_IR_BEFORE_OPTIMIZER is set,
    // This vector stored the old LLVMContext, which must be valid until its
    // related resources are released.
    std::vector<std::unique_ptr<llvm::LLVMContext>> m_depletedLLVMContexts;
    llvm::sys::Mutex m_LLVMContextMutex;
    Intel::OpenCL::Utils::CPUDetect *m_CpuId;
    llvm::SmallVector<std::string, 8>
                             m_forcedCpuFeatures;
    ETransposeSize           m_transposeSize;
    int                      m_rtLoopUnrollFactor;
    std::string              m_builtinInitLog;
    bool                     m_dumpHeuristicIR;
    bool                     m_debug;
    bool                     m_disableOptimization;
    bool                     m_useNativeDebugger;
    bool                     m_streamingAlways;
    unsigned m_expensiveMemOpts;
    PassManagerType m_passManagerType;

    static bool s_globalStateInitialized;

private:
    // Disable copy ctor and assignment operator
    Compiler( const Compiler& );
    bool operator = (const Compiler& );
    // Validate if the vectorized mode is supported by a target arch.
    // If not then issue an error and interrupt the compilation.
    void validateVectorizerMode(llvm::raw_ostream& log) const;

    // Get or generate LLVMContext for current thread.
    llvm::LLVMContext& getLLVMContext();
};
} // namespace DeviceBackend
} // namespace OpenCL
} // namespace Intel
