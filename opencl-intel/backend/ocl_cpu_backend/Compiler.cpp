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
#include "cl_types.h"
#include "cpu_dev_limits.h"
#include "Compiler.h"
#include "Optimizer.h"
#include "VecConfig.h"
#include "CPUDetect.h"
#include "BuiltinModules.h"
#include "exceptions.h"
#include "BuiltinModuleManager.h"
#include "CompilationUtils.h"
#include "MetaDataApi.h"
#include "common_clang.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/PrettyStackTrace.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/Bitcode/BitcodeReader.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Argument.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/CodeGen/CommandFlags.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Linker/Linker.h"

#include <memory>
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
using std::string;

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

TargetOptions ExternInitTargetOptionsFromCodeGenFlags() {
  return InitTargetOptionsFromCodeGenFlags();
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

CompilerBuildOptions::CompilerBuildOptions( llvm::Module* pModule):
    m_debugInfo(false),
    m_profiling(false),
    m_disableOpt(false),
    m_relaxedMath(false),
    m_denormalsZero(false),
    m_libraryModule(false),
    m_fpgaEmulator(false),
    m_APFLevel(0)
{
    assert(pModule);

    NamedMDNode* metadata = pModule->getNamedMetadata("opencl.compiler.options");

    llvm::Triple triple(pModule->getTargetTriple());
    if(triple.isINTELFPGAEnvironment())
    {
        m_fpgaEmulator = true;
    }

    if(NULL == metadata)
    {
        return;
    }

    if(metadata->getNumOperands() == 0)
    {
        return;
    }

    MDNode* flag = metadata->getOperand(0);
    for(uint32_t i =0; flag && (i < flag->getNumOperands()); ++i)
    {
        MDString* flagName = dyn_cast<MDString>(flag->getOperand(i));

        assert(flagName &&
            "opencl.compiler.options is expected to have a node inside!");

        if(flagName->getString() == "-g")
            m_debugInfo = true;
        if(flagName->getString() == "-profiling")
            m_profiling = true;
        if(flagName->getString() == "-cl-opt-disable")
            m_disableOpt = true;
        if(flagName->getString() == "-cl-fast-relaxed-math")
            m_relaxedMath = true;
        if(flagName->getString() == "-create-library")
            m_libraryModule = true;
        if(flagName->getString() == "-cl-denorms-are-zero")
            m_denormalsZero = true;
        if(flagName->getString() == "-auto-prefetch-level=0")
            m_APFLevel = 0;
        if(flagName->getString() == "-auto-prefetch-level=1")
            m_APFLevel = 1;
        if(flagName->getString() == "-auto-prefetch-level=2")
            m_APFLevel = 2;
        if(flagName->getString() == "-auto-prefetch-level=3")
            m_APFLevel = 3;
    }
}

bool Compiler::s_globalStateInitialized = false;

/*
 * This is a static method which must be called from the
 * single threaded environment before any instance of Compiler
 * class is created
 */
void Compiler::Init()
{
    // [LLVM 3.6 UPGRADE] llvm_start_multithreaded was removed from LLVM.
    // http://reviews.llvm.org/rL211285 states this function is no op since
    // r211277.
    // http://reviews.llvm.org/rL211277
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

    if (!config.DisableStackDump())
    {
        llvm::EnablePrettyStackTrace();
    }

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
    m_rtLoopUnrollFactor(config.GetRTLoopUnrollFactor()),
    m_IRDumpAfter(config.GetIRDumpOptionsAfter()),
    m_IRDumpBefore(config.GetIRDumpOptionsBefore()),
    m_IRDumpDir(config.GetDumpIRDir()),
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

llvm::Module* Compiler::BuildProgram(llvm::Module* pModule,
                                     ProgramBuildResult* pResult)
{
    assert(pModule && "pModule parameter must not be NULL");
    assert(pResult && "Build results pointer must not be NULL");

    validateVectorizerMode(pResult->LogS());

    // Check if given program is valid for the target.
    if (!isProgramValid(pModule, pResult))
    {
        throw Exceptions::CompilerException("Program is not valid for this target", CL_DEV_INVALID_BINARY);
    }

    CompilerBuildOptions buildOptions(pModule);

    //
    // Apply IR=>IR optimizations
    //
    intel::OptimizerConfig optimizerConfig( m_CpuId,
                                            m_transposeSize,
                                            m_IRDumpAfter,
                                            m_IRDumpBefore,
                                            m_IRDumpDir,
                                            buildOptions.GetDebugInfoFlag(),
                                            buildOptions.GetProfilingFlag(),
                                            buildOptions.GetDisableOpt(),
                                            buildOptions.GetRelaxedMath(),
                                            buildOptions.GetlibraryModule(),
                                            buildOptions.IsFpgaEmulator(),
                                            m_dumpHeuristicIR,
                                            buildOptions.GetAPFLevel(),
                                            m_rtLoopUnrollFactor);
    Optimizer optimizer(pModule, GetBuiltinModuleList(), &optimizerConfig);
    optimizer.Optimize();

    if( optimizer.hasUndefinedExternals() && !buildOptions.GetlibraryModule())
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
    m_debug = buildOptions.GetDebugInfoFlag();

    //dumpModule(*(pModule));

    pResult->SetBuildResult( CL_DEV_SUCCESS );

    // Execution Engine depends on module configuration and should
    // be created per module.
    // The object that owns module, should own the execution engine
    // as well and be responsible for release, of course.

    // Compiler creates execution engine but only keeps a pointer to the latest
    CreateExecutionEngine(pModule);
    return pModule;
}

bool Compiler::FindFunctionBodyInModules(std::string &FName,
                               llvm::Module const *bifModule,
                               llvm::Function* &pFunction) const
{
    pFunction = bifModule->getFunction(FName);

    // Note that due to the lazy Module parsing of built-in modules the function
    // body might not be materialized yet and is reported as declaration in that case.
    // (This behaiour was fixed in LLVM 3.6)
    if (pFunction != NULL &&
        (pFunction->isMaterializable() || !pFunction->isDeclaration())) {
      return true;
    }

    pFunction = NULL;
    return false;
}

llvm::Module* Compiler::ParseModuleIR(llvm::MemoryBuffer* pIRBuffer)
{
    // Parse the module IR
    llvm::ErrorOr<std::unique_ptr<llvm::Module>> pModuleOrErr =
      expectedToErrorOrAndEmitErrors(*m_pLLVMContext, llvm::parseBitcodeFile(
                               pIRBuffer->getMemBufferRef(), *m_pLLVMContext));
    if ( !pModuleOrErr )
    {
        throw Exceptions::CompilerException(std::string("Failed to parse IR: ")
              + pModuleOrErr.getError().message(), CL_DEV_INVALID_BINARY);
    }
    return pModuleOrErr.get().release();
}

// RTL builtin modules consist of two libraries.
// The first is shared across all HW architectures and the second one
// is optimized for a specific HW architecture.
// NOTE: There is no shared library for KNC so it has the optimized one only.
void Compiler::LoadBuiltinModules(BuiltinLibrary* pLibrary,
                   llvm::SmallVector<llvm::Module*, 2>& builtinsModules) const
{
    std::unique_ptr<llvm::MemoryBuffer> rtlBuffer(std::move(
                                        pLibrary->GetRtlBuffer()));
    assert(rtlBuffer && "pRtlBuffer is NULL pointer");
    llvm::ErrorOr<std::unique_ptr<llvm::Module>> spModuleOrErr =
      expectedToErrorOrAndEmitErrors(*m_pLLVMContext,
               llvm::getOwningLazyBitcodeModule(std::move(rtlBuffer), *m_pLLVMContext));

    if ( !spModuleOrErr )
    {
        // Failed to load runtime library
        spModuleOrErr = llvm::ErrorOr<std::unique_ptr<llvm::Module>>(
                std::unique_ptr<llvm::Module>(
                  new llvm::Module("dummy", *m_pLLVMContext)));
        if ( !spModuleOrErr )
        {
            throw Exceptions::CompilerException(
              "Failed to allocate/parse buitin module");
        }
    }
    else
    {
        spModuleOrErr.get().get()->setModuleIdentifier("RTLibrary");
    }

    auto* pModule = spModuleOrErr.get().release();
    builtinsModules.push_back(pModule);

    // on KNC we don't have shared (common) library, so skip loading
    if (pLibrary->GetCPU() != MIC_KNC) {
        // the shared RTL is loaded here
        std::unique_ptr<llvm::MemoryBuffer> RtlBufferSvmlShared(std::move(
                                          pLibrary->GetRtlBufferSvmlShared()));
        llvm::Expected<std::unique_ptr<llvm::Module>> spModuleSvmlSharedOrErr(
            llvm::getOwningLazyBitcodeModule(std::move(RtlBufferSvmlShared),
                *m_pLLVMContext));

        if ( !spModuleSvmlSharedOrErr ) {
            throw Exceptions::CompilerException(
              "Failed to allocate/parse buitin module");
        }

        llvm::Module* pModuleSvmlShared = spModuleSvmlSharedOrErr.get().release();
        // on both 64-bit and 32-bit platform the same shared RTL contatinig platform independent byte code is used,
        // so set triple and data layout for shared RTL from particular RTL in order to avoid warnings from linker.
        pModuleSvmlShared->setTargetTriple(pModule->getTargetTriple());
        pModuleSvmlShared->setDataLayout(pModule->getDataLayout());

        builtinsModules.push_back(pModuleSvmlShared);
    }
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
                auto *featureString = dyn_cast<MDString>(mdNode->getOperand(i).get());
                assert(featureString && "MDString is expected");
                if (featureString->getString() == "cl_images")
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
          validity = UNSUPPORTED;
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

const std::string Compiler::GetBitcodeTargetTriple( const void* pBinary,
                                                    size_t uiBinarySize ) const
{

    std::unique_ptr<MemoryBuffer> spIRBuffer(
      MemoryBuffer::getMemBuffer(StringRef(static_cast<const char*>(pBinary),
                                           uiBinarySize), "", false));
    llvm::Expected<std::string> strTargetTriple =
                   llvm::getBitcodeTargetTriple(spIRBuffer->getMemBufferRef());
    if (!strTargetTriple || *strTargetTriple == "") {
      throw Exceptions::CompilerException(
                     std::string("Failed to get target triple from bitcode!"),
                     CL_DEV_INVALID_BINARY);
    }

    return *strTargetTriple;
}

}}}
