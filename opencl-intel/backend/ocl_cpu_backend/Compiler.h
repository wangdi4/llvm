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

#pragma once

#include "cl_dev_backend_api.h"
#include "CPUDetect.h"
#include "ICompilerConfig.h"

#include "llvm/ADT/SmallVector.h"
#include "llvm/Support/raw_ostream.h"

#include <string>

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
    llvm::Module* BuildProgram(llvm::Module*, const char* pBuildOpts,
                               ProgramBuildResult* pResult);

    const CPUId &GetCpuId() const { return m_CpuId; }

    // Create execution engine
    virtual void CreateExecutionEngine(llvm::Module* m)  = 0;

    // Get the latest execution engine
    virtual void *GetExecutionEngine() = 0;

    // Get Function Address Resolver
    virtual void *GetFunctionAddressResolver() { return NULL; }

    // Returns a list of pointers to the RTL libraries
    virtual llvm::SmallVector<llvm::Module*, 2> GetBuiltinModuleList() const = 0;

    llvm::Module* ParseModuleIR(llvm::MemoryBuffer* pIRBuffer);

    const std::string GetBitcodeTargetTriple(const void* pBinary, size_t uiBinarySize) const;

    virtual void SetObjectCache(ObjectCodeCache* pCache) = 0;

    virtual void SetBuiltinModules(const std::string& cpuName, const std::string& cpuFeatures=""){}
protected:
    void LoadBuiltinModules(BuiltinLibrary* pLibrary,
      llvm::SmallVector<llvm::Module*, 2>& builtinsModules) const;

    // Create TargetMachine for X86.
    llvm::TargetMachine* GetTargetMachine(llvm::Module* pModule) const;

protected:
    bool                     m_bIsFPGAEmulator;
    bool                     m_bIsEyeQEmulator;
    llvm::LLVMContext*       m_pLLVMContext;
    Intel::CPUId             m_CpuId;
    llvm::SmallVector<std::string, 8>
                             m_forcedCpuFeatures;
    ETransposeSize           m_transposeSize;
    int                      m_rtLoopUnrollFactor;
    std::vector<int>         m_IRDumpAfter;
    std::vector<int>         m_IRDumpBefore;
    std::string              m_IRDumpDir;
    bool                     m_dumpHeuristicIR;
    bool                     m_debug;
    bool                     m_useNativeDebugger;

    static bool s_globalStateInitialized;

private:
    // Disable copy ctor and assignment operator
    Compiler( const Compiler& );
    bool operator = (const Compiler& );
    // Check if given program is valid for the target.
    bool isProgramValid(llvm::Module*, ProgramBuildResult*) const;
    // Validate if the vectorized mode is supported by a target arch.
    // If not then issue an error and interrupt the compilation.
    void validateVectorizerMode(llvm::raw_ostream& log) const;
};

}}}
