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
#include "CompilationUtils.h"

#include "llvm/Support/Casting.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/Bitcode/ReaderWriter.h"
#include "llvm/Transforms/Utils/UnifyFunctionExitNodes.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Argument.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/CodeGen/CommandFlags.h"
#include "llvm/Linker.h"
using std::string;

#include <fstream>
#include <iostream>
#include <sstream>


namespace llvm
{
  extern bool DisablePrettyStackTrace;
}

namespace Intel { namespace OpenCL { namespace DeviceBackend {

void dumpModule(llvm::Module& m){
#ifndef NDEBUG
  static unsigned counter=0;
  std::string buffer;
  llvm::raw_string_ostream stream(buffer);
  std::stringstream fileName;
  fileName << "kernel" << counter << ".ll";
  std::ofstream outf(fileName.str().c_str());
  stream << m;
  std::cout << "dumped kernel" << counter << std::endl;
  counter++;
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

/**
 * Generates the log record (to the given stream) enumerating function names
   with unresolved function pointer calls
 */
void LogFuncPtrCalls( llvm::raw_ostream& logs, const std::vector<std::string>& externals)
{
    logs << "Error: unresolved pointer calls in function(s):\n";

    for( std::vector<std::string>::const_iterator i = externals.begin(), e = externals.end(); i != e; ++i)
    {
        logs << *i << "\n";
    }
}

/**
 * Generates the log record (to the given stream) enumerating function names
   with recursive calls
 */
void LogHasRecursion( llvm::raw_ostream& logs, const std::vector<std::string>& externals)
{
    logs << "Error: recursive call in function(s):\n";

    for( std::vector<std::string>::const_iterator i = externals.begin(), e = externals.end(); i != e; ++i)
    {
        logs << *i << "\n";
    }
}

bool TerminationBlocker::s_released = false;

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

cl_dev_err_code ProgramBuildResult::GetBuildResult() const
{
    return m_result;
}

bool Compiler::s_globalStateInitialized = false;

/*
 * This is a static method which must be called from the
 * single threaded environment before any instance of Compiler
 * class is created
 */
void Compiler::Init()
{
    //TODO: Add handling of failure of MT init
    llvm::llvm_start_multithreaded();
    // Initialize LLVM for X86 target.
    LLVMInitializeX86TargetInfo();
    LLVMInitializeX86Target();
    LLVMInitializeX86AsmPrinter();
    LLVMInitializeX86TargetMC();
#if defined(INCLUDE_MIC_DEVICE)
    LLVMInitializeY86TargetInfo();
    LLVMInitializeY86Target();
    LLVMInitializeY86AsmPrinter();
    LLVMInitializeY86TargetMC();
#endif
}

/**
 * Initialize the global options
 */
void Compiler::InitGlobalState( const IGlobalCompilerConfig& config )
{
    if( s_globalStateInitialized )
    {
        return;
    }

    std::vector<std::string> args;

    args.push_back("OclBackend");
    // OpenCL assumes stack is aligned
    // We are forcing LLVM to align the stack
    args.push_back("-force-align-stack");
    // SSE requires maximum alignment of parameters of 16 bytes
    // AVX requires maximum alignment of parameters of 32 bytes
    args.push_back("-stack-alignment=32");

    if( config.EnableTiming() && false == config.InfoOutputFile().empty())
    {
        std::stringstream ss;
        ss << "-info-output-file=" << config.InfoOutputFile().c_str();

        args.push_back("--time-passes");
        args.push_back(ss.str());
    }

    // Generate the argc/argv parameters for the llvm::ParsecommandLineOptions
    std::vector<char*> argv;

    std::vector<std::string>::iterator i = args.begin();
    std::vector<std::string>::iterator e = args.end();

    for(; i != e; ++i )
    {
        //be careful here. The pointer returned by c_str() is only guaranteed to remain unchanged
        //until the next call to a non-constant member function of the string object.
        argv.push_back( const_cast<char*>(i->c_str()) );
    }

    llvm::cl::ParseCommandLineOptions(argv.size(), &argv[0]);

    llvm::DisablePrettyStackTrace = config.DisableStackDump();

    s_globalStateInitialized = true;
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

Compiler::Compiler(const ICompilerConfig& config):
    m_pLLVMContext( new llvm::LLVMContext ),
    m_transposeSize(config.GetTransposeSize()),
    m_IRDumpAfter(config.GetIRDumpOptionsAfter()),
    m_IRDumpBefore(config.GetIRDumpOptionsBefore()),
    m_IRDumpDir(config.GetDumpIRDir()),
    m_needLoadBuiltins(config.GetLoadBuiltins()),
    m_dumpHeuristicIR(config.GetDumpHeuristicIRFlag()),
    m_debug(false)
{
    // WORKAROUND!!! See the notes in TerminationBlocker description
   static Utils::TerminationBlocker blocker;
}

Compiler::~Compiler()
{
    // WORKAROUND!!! See the notes in TerminationBlocker description
    if( Utils::TerminationBlocker::IsReleased() )
        return;

    delete m_pLLVMContext;
}

llvm::Module* Compiler::BuildProgram(llvm::MemoryBuffer* pIRBuffer,
                                     const CompilerBuildOptions* pOptions,
                                     ProgramBuildResult* pResult)
{
    assert(pIRBuffer && "pIRBuffer parameter must not be NULL");
    assert(pResult && "Build results pointer must not be NULL");
    assert(pOptions && "Build options pointer must not be NULL");

    validateVectorizerMode(pResult->LogS());

    //TODO: Add log
    std::auto_ptr<llvm::Module> spModule(ParseModuleIR(pIRBuffer));
    assert(spModule.get() && "Cannot Created llvm Module from the Program Bit Code");

    // Check if given program is valid for the target.
    if (!isProgramValid(spModule.get(), pResult))
    {
        throw Exceptions::CompilerException("Program is not valid for this target", CL_DEV_INVALID_BINARY);
    }
    dumpModule(*(spModule.get()));
    //
    // Apply IR=>IR optimizations
    //
    intel::OptimizerConfig optimizerConfig( m_CpuId,
                                            m_transposeSize,
                                            m_IRDumpAfter,
                                            m_IRDumpBefore,
                                            m_IRDumpDir,
                                            pOptions->GetDebugInfoFlag(),
                                            pOptions->GetProfilingFlag(),
                                            pOptions->GetDisableOpt(),
                                            pOptions->GetRelaxedMath(),
                                            pOptions->GetlibraryModule(),
                                            m_dumpHeuristicIR,
                                            pOptions->GetAPFLevel());
    Optimizer optimizer( spModule.get(), GetRtlModule(), &optimizerConfig);
    optimizer.Optimize();

    if( optimizer.hasUndefinedExternals() && !pOptions->GetlibraryModule())
    {
        Utils::LogUndefinedExternals( pResult->LogS(), optimizer.GetUndefinedExternals());
        throw Exceptions::CompilerException( "Failed to parse IR", CL_DEV_INVALID_BINARY);
    }

    if( optimizer.hasFunctionPtrCalls())
    {
      Utils::LogFuncPtrCalls( pResult->LogS(), optimizer.GetFuncNames(true));
      throw Exceptions::CompilerException( "Dynamic block variable call detected.",
                                            CL_DEV_INVALID_BINARY);
    }

    if( optimizer.hasRecursion())
    {
      Utils::LogHasRecursion( pResult->LogS(), optimizer.GetFuncNames(false));
      throw Exceptions::CompilerException( "Recursive call detected.",
                                            CL_DEV_INVALID_BINARY);
    }
    //
    // Populate the build results
    //
    m_debug = pOptions->GetDebugInfoFlag();

    //dumpModule(*(spModule.get()));

    pResult->SetBuildResult( CL_DEV_SUCCESS );

    // Execution Engine depends on module configuration and should
    // be created per module.
    // The object that owns module, should own the execution engine
    // as well and be responsible for release, of course.

    // Compiler creates execution engine but only keeps a pointer to the latest
    CreateExecutionEngine(spModule.get());
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

// for CPU implementation RTL module consists of two libraries: shared (common for all CPU architectures)
// and particular (optimized for one architecture), they should be linked,
// for KNC we have the particular RTL only
llvm::Module* Compiler::CreateRTLModule(BuiltinLibrary* pLibrary) const
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

    // on KNC we don't have shared (common) library, so skip loading
    if (pLibrary->GetCPU() != MIC_KNC) {
        // the shared RTL is loaded here
        llvm::MemoryBuffer* pRtlBufferSvmlShared = pLibrary->GetRtlBufferSvmlShared();

        std::auto_ptr<llvm::Module> spModuleSvmlShared(llvm::ParseBitcodeFile(pRtlBufferSvmlShared, *m_pLLVMContext));

        if ( NULL == spModuleSvmlShared.get()) {
            throw Exceptions::CompilerException("Failed to allocate/parse buitin module");
        }

        std::string ErrorMessage;
        // we need to link particular and shared RTLs together
        if (llvm::Linker::LinkModules(spModule.get(), spModuleSvmlShared.get(), Linker::DestroySource, &ErrorMessage) ) {
            std::string ErrStr("Failed to link shared builtins module.");
            if ( !ErrorMessage.empty() ) {
                ErrStr.append(ErrorMessage);
            }
            throw Exceptions::CompilerException(ErrStr);
        }
    }

    UpdateTargetTriple(spModule.get());
    return spModule.release();
}

bool Compiler::isProgramValid(llvm::Module* pModule, ProgramBuildResult* pResult) const
{
    // Check for the limitation: "Images are not supported on Xeon Phi".
    if(!m_CpuId.HasGatherScatter()) return true;
    if (llvm::NamedMDNode* pNode = pModule->getNamedMetadata("opencl.used.optional.core.features"))
    {
        // Usually "opencl.used.optional.core.features" metadata has only one node.
        // It can have multiple nodes if clCompileProgram/clLinkProgram API is used.
        // In that case each node represent the features used in its original
        // binary before linking it into the final module.
        // We should respect all of them and abort compilation if any of the binaries has images.
        for (unsigned mdNodeId = 0; mdNodeId < pNode->getNumOperands(); ++mdNodeId)
        {
            MDNode *mdNode = pNode->getOperand(mdNodeId);
            for (unsigned i = 0; i < mdNode->getNumOperands(); ++i)
            {
                if (mdNode->getOperand(i)->getName() == "cl_images")
                {
                    pResult->LogS() << "Images are not supported on given device.\n";
                    return false;
                }
            }
        }
    }
    return true;
}

void Compiler::validateVectorizerMode(llvm::raw_ostream& log) const
{
    // Validate if the vectorized mode valid and supported by the target arch.
    // If not then issue an error and interrupt the build.
    enum {
      VALID,
      INVALID,
      UNSUPPORTED
    } validity = VALID;

    switch(m_transposeSize) {
      default:
        validity = INVALID;
        break;

      case TRANSPOSE_SIZE_AUTO:
      case TRANSPOSE_SIZE_1:
        validity = VALID;
        break;

      case TRANSPOSE_SIZE_4:
        if(!m_CpuId.HasSSE41())
          validity = UNSUPPORTED;
        break;

      case TRANSPOSE_SIZE_8:
        if(!m_CpuId.HasAVX1())
          validity = UNSUPPORTED;
        break;

      case TRANSPOSE_SIZE_16:
        if(!m_CpuId.HasGatherScatter())
          validity = INVALID;
          // TODO: uncomment when MIC/AVX512 support became relevant
          //validity = UNSUPPORTED;
        break;
    }

    switch(validity) {
      case VALID:
        return;

      case INVALID:
        log << "The specified vectorizer mode (" << m_transposeSize
             << ") is invalid.\n";
        break;

      case UNSUPPORTED:
        log << "The specified vectorizer mode (" << m_transposeSize
             << ") is not supported by the target architecture.\n";
        break;
    }
    throw Exceptions::CompilerException("Failed to apply the vectorizer mode.",
                                        CL_DEV_INVALID_BUILD_OPTIONS);
}

void UpdateTargetTriple(llvm::Module *pModule)
{
  std::string triple = pModule->getTargetTriple();

  //Force ELF codegen on Windows (MCJIT does not support COFF format)
  if ((triple.find("win32") != std::string::npos)
        && triple.find("-elf") == std::string::npos) {
    pModule->setTargetTriple(triple + "-elf");    // transforms:
                                                  // x86_64-pc-win32
                                                  // i686-pc-win32
                                                  // to:
                                                  // x86_64-pc-win32-elf
                                                  // i686-pc-win32-elf
  }
}

}}}
