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

File Name:  Compiler.h

\*****************************************************************************/
#pragma once

#include "cl_dev_backend_api.h"
#include "CPUDetect.h"
#include "ICompilerConfig.h"
#include "llvm/Support/raw_ostream.h"

#include <string>

namespace llvm {
    class ExecutionEngine;
    class Function;
    class LLVMContext;
    class MemoryBuffer;
    class Module;
    class Type;
}

extern bool s_ignore_termination;

namespace Intel { namespace OpenCL { namespace DeviceBackend {
class BuiltinLibrary;
class BuiltinModule;
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
    CompilerBuildOptions( llvm::Module* pModule);

    bool GetDisableOpt()    const { return m_disableOpt; }
    bool GetDebugInfoFlag() const { return m_debugInfo; }
    bool GetProfilingFlag() const { return m_profiling; }
    bool GetRelaxedMath()   const { return m_relaxedMath; }
    bool GetlibraryModule() const { return m_libraryModule; }
    int  GetAPFLevel()      const { return m_APFLevel; }
    bool GetDenormalsZero() const { return m_denormalsZero;}

private:
    bool m_debugInfo;
    bool m_profiling;
    bool m_disableOpt;
    bool m_relaxedMath;
    bool m_denormalsZero;
    // Sets whether optimized code is library module or a set of kernels
    // If this options is set to true then some optimization passes will be skipped
    bool m_libraryModule;
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
    llvm::Module* BuildProgram(llvm::Module*,
                               ProgramBuildResult* pResult);

    const CPUId &GetCpuId() const
    {
        return m_CpuId;
    }

    // Create execution engine
    virtual void CreateExecutionEngine(llvm::Module* m)  = 0;

    // Get the latest execution engine
    virtual void *GetExecutionEngine() = 0;

    // Get Function Address Resolver
    virtual void *GetFunctionAddressResolver() { return NULL; }

    /**
     * Returns pointer to the RTL library module
     */
    virtual llvm::Module* GetRtlModule() const = 0;

    llvm::Module* ParseModuleIR(llvm::MemoryBuffer* pIRBuffer);

    virtual void SetObjectCache(ObjectCodeCache* pCache) = 0;

protected:

    llvm::Module* CreateRTLModule(BuiltinLibrary* pLibrary) const;

protected:
    llvm::LLVMContext*       m_pLLVMContext;
    Intel::CPUId             m_CpuId;
    std::vector<std::string> m_forcedCpuFeatures;
    ETransposeSize           m_transposeSize;
    int                      m_rtLoopUnrollFactor;
    std::vector<int>         m_IRDumpAfter;
    std::vector<int>         m_IRDumpBefore;
    std::string              m_IRDumpDir;
    bool                     m_dumpHeuristicIR;
    bool                     m_debug;

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

void UpdateTargetTriple(llvm::Module *pModule);
}}}
