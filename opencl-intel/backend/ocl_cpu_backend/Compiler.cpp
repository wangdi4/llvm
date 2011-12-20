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

File Name:  Compiler.cpp

\*****************************************************************************/
#define NOMINMAX
#include <memory>
#include <vector>
#include <string>
#include "cl_types.h"
#include "cpu_dev_limits.h"
#include "Compiler.h"
#include "Optimizer.h"
#include "VecConfig.h"
#include "CPUDetect.h"
#include "BuiltinModule.h"
#include "exceptions.h"
#include "BuiltinModuleManager.h"
#include "plugin_manager.h"
#include "CompilationUtils.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetSelect.h"
#include "llvm/Target/TargetData.h"
#include "llvm/ExecutionEngine/JIT.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/DerivedTypes.h"
#include "llvm/Bitcode/ReaderWriter.h"
#include "llvm/Transforms/Utils/UnifyFunctionExitNodes.h"
#include "llvm/Module.h"
#include "llvm/Function.h"
#include "llvm/Argument.h"
#include "llvm/Type.h"
#include "llvm/BasicBlock.h"
#include "llvm/Instructions.h"
#include "llvm/Instruction.h"
#include "llvm/LLVMContext.h"
#include "VTune/JITProfiling.h"
using std::string;

#include <fstream>
#include <iostream>
#include <sstream>

namespace Intel { namespace OpenCL { namespace DeviceBackend {
void dumpModule(llvm::Module& m){
#if !defined(__NDEBUG__)
  static unsigned counter=0;
  std::string buffer;
  llvm::raw_string_ostream stream(buffer);
  std::stringstream fileName;
  fileName << "kernel" << counter++ << ".ll";
  std::ofstream outf(fileName.str().c_str());
  std::cout << "before" << std::endl;
  stream << m;
  std::cout << "after" << std::endl;
  stream.flush();
  outf << buffer;
#endif
}


/*
 * Utility methods
 */
namespace Utils 
{
/**
 * Generates the log record (to the given stream) enumerating the given external function names
 */
void LogUndefinedExternals( llvm::raw_ostream& logs, const std::vector<std::string>& externals)
{
    logs << "Error: unimplemented function(s) used:\n";

    for( std::vector<std::string>::const_iterator i = externals.begin(), e = externals.end(); i != e; ++i)
    {
        logs << *i << "\n";
    }
    //LLVMBackend::GetInstance()->m_logger->Log(Logger::ERROR_LEVEL, L"implemented function(s) used:\n<%s>", m_strLastError.c_str());
}


} //namespace Utils 


ProgramBuildResult::ProgramBuildResult():
    m_result(CL_DEV_SUCCESS),
    m_logStream(m_buildLog) { }

bool ProgramBuildResult::Succeeded() const 
{ 
    return CL_DEV_SUCCEEDED(m_result); 
}

bool ProgramBuildResult::Failed() const 
{ 
    return CL_DEV_FAILED(m_result); 
}

std::string ProgramBuildResult::GetBuildLog() const 
{ 
    m_logStream.flush();
    return m_buildLog; 
}

llvm::raw_ostream& ProgramBuildResult::LogS() 
{ 
    return m_logStream; 
}

void ProgramBuildResult::SetBuildResult( cl_dev_err_code code ) 
{
    m_result = code; 
}

cl_dev_err_code ProgramBuildResult::GetBuildResult() 
{ 
    return m_result; 
}

void ProgramBuildResult::SetFunctionsWidths( FunctionWidthVector* pv)
{
    m_pFunctionWidths = pv;
}

const FunctionWidthVector& ProgramBuildResult::GetFunctionsWidths() const
{
    assert(m_pFunctionWidths);
    return *m_pFunctionWidths;
}

void ProgramBuildResult::SetKernelsInfo( KernelsInfoMap* pKernelsInfo)
{
    m_pKernelsInfo = pKernelsInfo;
}

KernelsInfoMap& ProgramBuildResult::GetKernelsInfo()
{
    assert(m_pKernelsInfo);
    return *m_pKernelsInfo;
}

void ProgramBuildResult::SetPrivateMemorySize( size_t size)
{
    m_privateMemorySize = size;
}

size_t ProgramBuildResult::GetPrivateMemorySize() const
{
    return m_privateMemorySize;    
}

/*
 * This is a static method which must be called from the 
 * single threaded environment before any instance of Compiler
 * class is created
 */
void Compiler::Init()
{
    //TODO: Add handling of failure of MT init
    llvm::llvm_start_multithreaded();
    // OpecnCL assumes stack is aligned
    // We are forccing LLVM to align the stack
    std::vector<char *> args;
    char arg1[] = "OclBackend";
    args.push_back(arg1);
    char arg2[] = "-force-align-stack";
    args.push_back(arg2);
    // SSE requirest maximum alignment of parameters of 16 bytes
    // AVX265 requirest maximum alignment of parameters of 32 bytes
    char arg3[] = "-stack-alignment=32";
    args.push_back(arg3);
    llvm::cl::ParseCommandLineOptions(args.size(), &args[0]);

    llvm::InitializeAllTargets();
    llvm::InitializeAllAsmPrinters();
}

/*
 * This is a static method which must be called from the 
 * single threaded environment after all instanced of the
 * Compiler class are deleted
 */
void Compiler::Terminate()
{
    llvm::llvm_shutdown();
}

Compiler::Compiler(const CompilerConfig& config):
    m_config(config),
    m_pLLVMContext( new llvm::LLVMContext )
{
   if (!m_config.GetTimePasses().empty())
   {
        std::vector<char *> args;
        char arg1[] = "timepasses";
        args.push_back(arg1);
        char arg2[] = "-time-passes";
        args.push_back(arg2);
        std::stringstream fileName;
        fileName << "-info-output-file=" << m_config.GetTimePasses().c_str() << std::ends;
        char* arg3 = new char[fileName.str().size()];
        for(unsigned int i=0; i< fileName.str().size(); i++)
            arg3[i] = (fileName.str().c_str())[i];
        args.push_back(arg3);
        llvm::cl::ParseCommandLineOptions(args.size(), &args[0]);
   }
}

Compiler::~Compiler()
{
    delete m_pLLVMContext;
}

llvm::Module* Compiler::BuildProgram(llvm::MemoryBuffer* pIRBuffer, 
                                           const CompilerBuildOptions* pOptions,
                                           ProgramBuildResult* pResult)
{
    assert(pIRBuffer && "pIRBuffer parameter must not be NULL");
    assert(pResult && "Build results pointer must not be NULL");
    assert(pOptions && "Build options pointer must not be NULL");

    //TODO: Add log
    std::auto_ptr<llvm::Module> spModule(ParseModuleIR(pIRBuffer)); 
    assert(spModule.get() && "Cannot Created llvm Module from the Program Bit Code");

    //
    // Apply IR=>IR optimizations
    //
    intel::OptimizerConfig optimizerConfig( m_selectedCpuId, 
                                            m_config.GetTransposeSize(), 
                                            m_selectedCpuFeatures,
                                            m_config.GetIRDumpOptionsAfter(),
                                            m_config.GetIRDumpOptionsBefore(),
                                            m_config.GetDumpIRDir(),
                                            pOptions->GetDebugInfoFlag(),
                                            pOptions->GetDisableOpt(),
                                            pOptions->GetRelaxedMath(),
                                            pOptions->GetlibraryModule());
    Optimizer optimizer( spModule.get(), GetRtlModule(), &optimizerConfig);
    optimizer.Optimize();
    
    if( optimizer.hasUndefinedExternals() && !pOptions->GetlibraryModule())
    {
        Utils::LogUndefinedExternals( pResult->LogS(), optimizer.GetUndefinedExternals());
        throw Exceptions::CompilerException( "Failed to parse IR", CL_DEV_INVALID_BINARY);
    }

    //
    // Populate the build results
    //
    std::auto_ptr<FunctionWidthVector> vectorizedFunctions( new FunctionWidthVector() );
    std::auto_ptr<KernelsInfoMap> kernelsMap( new KernelsInfoMap());


    optimizer.GetVectorizedFunctions( *vectorizedFunctions.get());
    //dumpModule(*(spModule.get()));

    optimizer.GetKernelsInfo( *kernelsMap.get());

    if (!pOptions->GetlibraryModule()){  //the build results don't apply to a library module
      pResult->SetPrivateMemorySize(optimizer.getPrivateMemorySize());
      pResult->SetFunctionsWidths( vectorizedFunctions.release() );
      pResult->SetKernelsInfo( kernelsMap.release());
    }
    pResult->SetBuildResult( CL_DEV_SUCCESS );

    // !!!WORKAROUND!!! if module wasn't built previously than we use
    // optimizer output to create execution engine in compiler
    // spModule is now owned by the execution engine
    if(!m_config.GetLoadBuiltins())
        this->CreateExecutionEngine(spModule.get());
    return spModule.release();
}

llvm::Module* Compiler::ParseModuleIR(llvm::MemoryBuffer* pIRBuffer)
{
    //
    // Parse the module IR 
    //
    std::string strErr;
    llvm::Module* pModule = llvm::ParseBitcodeFile( pIRBuffer, *m_pLLVMContext, &strErr);
    if ( NULL == pModule || !strErr.empty())
    {
        throw Exceptions::CompilerException(std::string("Failed to parse IR: ") + strErr, CL_DEV_INVALID_BINARY);
    }
    return pModule;
}

llvm::Module* Compiler::CreateRTLModule(BuiltinLibrary* pLibrary)
{
    llvm::MemoryBuffer* pRtlBuffer = pLibrary->GetRtlBuffer();
    std::auto_ptr<llvm::Module> spModule(llvm::ParseBitcodeFile(pRtlBuffer, *m_pLLVMContext));
 
    if ( NULL == spModule.get())
    {
        // Failed to load runtime library
        spModule.reset( new llvm::Module("dummy", *m_pLLVMContext));
        if ( NULL == spModule.get() ) 
        {
            throw Exceptions::CompilerException("Failed to allocate/parse buitin module");
        }
    } 
    else 
    {
        spModule->setModuleIdentifier("RTLibrary");
    }

    // Setting the target triple as a tripple the backend was compiled under
#if defined(_M_X64)
    spModule->setTargetTriple( "x86_64-pc-win32");
#elif defined(__LP64__)
    spModule->setTargetTriple( "amd64");
#else
    spModule->setTargetTriple( "i686-pc-win32");
#endif 
    return spModule.release();
}

}}}