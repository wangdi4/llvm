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

#define NOMINMAX

#include "cl_types.h"
#include "cpu_dev_limits.h"
#include "Compiler.h"
#include "Optimizer.h"
#include "VecConfig.h"
#include "BuiltinModules.h"
#include "exceptions.h"
#include "BuiltinModuleManager.h"
#include "CompilationUtils.h"
#include "MetadataAPI.h"

#include "llvm/Bitcode/BitcodeReader.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/PrettyStackTrace.h"
#include "llvm/Support/TargetRegistry.h"

#ifdef _WIN32
#include "lld/Common/TargetOptionsCommandFlags.h"
#else
#include "llvm/CodeGen/CommandFlags.inc"
#endif

#include <sstream>
#include <string>
#include <vector>

namespace Intel { namespace OpenCL { namespace DeviceBackend {

TargetOptions ExternInitTargetOptionsFromCodeGenFlags() {
#ifdef _WIN32
    // We only link to LLD on Windows
    return lld::initTargetOptionsFromCodeGenFlags();
#else
    return InitTargetOptionsFromCodeGenFlags();
#endif
}

// Supported target triples.
const char *PC_LIN64 = "x86_64-pc-linux";          // Used for RH64/SLES64.
const char *PC_WIN32 = "i686-pc-win32-msvc-elf";   // Win 32 bit.
const char *PC_WIN64 = "x86_64-pc-win32-msvc-elf"; // Win 64 bit.

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

/**
 * Generates the log record (to the given stream) enumerating function names
   with unresolved pipe accesses.
 */
void LogHasFpgaPipeDynamicAccess( llvm::raw_ostream& logs,
                                  const std::vector<std::string>& functions)
{
    logs << "Error: dynamic pipe or channel access in function(s):\n";

    for(const auto& fun : functions)
    {
        logs << fun << "\n";
    }
}

/**
 * Generates the log record (to the given stream) enumerating global names
   whose depth attribute is ignored.
   Note: currently only FPGA channels is expected to be such globals.
 */
void LogHasFPGAChannelsWithDepthIgnored(
    llvm::raw_ostream &logs, const std::vector<std::string> &globals)
{
    logs << "Warning: The default channel depths in the emulation flow will be "
         << "different from the hardware flow depth (0) to speed up emulation. "
         << "The following channels are affected:\n";

    for (const auto &global : globals)
    {
        logs << " - " <<  global << "\n";
    }

    logs << "\n";
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

CompilerBuildOptions::CompilerBuildOptions(const char* pBuildOpts):
    m_debugInfo(false),
    m_useNativeDebugger(false),
    m_profiling(false),
    m_disableOpt(false),
    m_relaxedMath(false),
    m_denormalsZero(false),
    m_uniformWGSize(false),
    m_APFLevel(0)
{
    llvm::StringRef buildOptions(pBuildOpts);

    if (buildOptions.empty())
      return;

     llvm::SmallVector<llvm::StringRef, 8> splittedOptions;
     buildOptions.split(splittedOptions, " ");
     for (const auto opt : splittedOptions) {
       std::string opts = opt.str();
       if (opt.equals("-profiling"))
         m_profiling = true;
       else if (opt.equals("-g"))
         m_debugInfo = true;
       else if (opt.equals("-gnative")) {
         m_debugInfo = true;
         m_useNativeDebugger = true;
       }
       else if (opt.equals("-cl-fast-relaxed-math"))
         m_relaxedMath = true;
       else if (opt.equals("-cl-opt-disable"))
         m_disableOpt = true;
       else if (opt.equals("-cl-denorms-are-zero"))
         m_denormalsZero = true;
       else if (opt.equals("-cl-uniform-work-group-size"))
         m_uniformWGSize = true;
       else if (opt.compare("-auto-prefetch-level") == 1)
         opt.substr(opt.find("=") + 1).getAsInteger(10, m_APFLevel);
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
#ifndef NDEBUG
    if (getenv("VOLCANO_COMPILER_DEBUG")) {
      args.push_back("-print-after-all");
      args.push_back("-print-before-all");
      args.push_back("-debug");
    }
#endif

    if (!getenv("DISABLE_INFER_AS")) {
      args.push_back("-override-flat-addr-space=4");
      args.push_back("-infer-as-rewrite-opencl-bis");
    }

    // Loops #pragma unroll are unrolled up to -pragma-unroll-threshold, which
    // is set by default to a huge value, and it basically allows to fully
    // unroll any loop. Needless to say, this can by suboptimal for x86 when
    // applied to big loops. FPGA device, in opposite, benefits from fully
    // unrolled loops, so most of FPGA workloads unroll every single loop.
    // Threshold heuristic set so as not to affect the performance negatively
    if (FPGA_EMU_DEVICE == config.TargetDevice())
    {
        args.push_back("-pragma-unroll-threshold=3072");
    }

    if( config.EnableTiming() && false == config.InfoOutputFile().empty())
    {
        std::stringstream ss;
        ss << "-info-output-file=" << config.InfoOutputFile().c_str();

        args.push_back("--time-passes");
        args.push_back(ss.str());
    }

    // Split the options by space and push back to args
    std::istringstream llvmOptsStream(config.LLVMOptions());
    std::copy(std::istream_iterator<std::string>(llvmOptsStream),
              std::istream_iterator<std::string>(),
              std::back_inserter(args));

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
    m_bIsFPGAEmulator(FPGA_EMU_DEVICE == config.TargetDevice()),
    m_bIsEyeQEmulator(EYEQ_EMU_DEVICE == config.TargetDevice()),
    m_pLLVMContext( new llvm::LLVMContext ),
    m_transposeSize(config.GetTransposeSize()),
    m_rtLoopUnrollFactor(config.GetRTLoopUnrollFactor()),
    m_IRDumpAfter(config.GetIRDumpOptionsAfter()),
    m_IRDumpBefore(config.GetIRDumpOptionsBefore()),
    m_IRDumpDir(config.GetDumpIRDir()),
    m_dumpHeuristicIR(config.GetDumpHeuristicIRFlag()),
    m_debug(false),
    m_useNativeDebugger(false)
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

static void materializeSpirTriple(llvm::Module *M) {
  assert((llvm::StringRef(M->getTargetTriple())).startswith("spir")
    && "Triple is not spir!");

  llvm::StringRef Triple =
#if defined(_M_X64)
      PC_WIN64;
#elif defined(__LP64__)
      PC_LIN64;
#elif defined(_WIN32)
      PC_WIN32;
#else
#error "Unsupported host platform"
#endif
  M->setTargetTriple(Triple);
}

llvm::TargetMachine* Compiler::GetTargetMachine(
                           llvm::Module* pModule) const {
  std::string ErrorString;

  // Leaving MArch blank implies using auto-detect
  llvm::StringRef MArch = "";
  llvm::StringRef MCPU  = m_CpuId.GetCPUName();

  llvm::Triple ModuleTriple(pModule->getTargetTriple());

  // FP_CONTRACT defined in module
  // Exclude FMA instructions when FP_CONTRACT is disabled
  llvm::TargetOptions TargetOpts = ExternInitTargetOptionsFromCodeGenFlags();
  if (pModule->getNamedMetadata("opencl.enable.FP_CONTRACT")) {
    TargetOpts.AllowFPOpFusion = llvm::FPOpFusion::Fast;
  } else {
    TargetOpts.AllowFPOpFusion = llvm::FPOpFusion::Standard;
  }

  llvm::CodeGenOpt::Level CGOptLevel =
    m_debug ? llvm::CodeGenOpt::None : llvm::CodeGenOpt::Default;

  llvm::EngineBuilder Builder;

  Builder.setErrorStr(&ErrorString);
  Builder.setOptLevel(CGOptLevel);
  Builder.setTargetOptions(TargetOpts);

  auto *TargetMachine =
    Builder.selectTarget(ModuleTriple, MArch, MCPU, m_forcedCpuFeatures);

  if (nullptr == TargetMachine) {
    throw Exceptions::CompilerException(
      "Failed to create TargetMachine: " + ErrorString);
  }

  return TargetMachine;
}

llvm::Module* Compiler::BuildProgram(llvm::Module* pModule,
                                     const char* pBuildOptions,
                                     ProgramBuildResult* pResult)
{
    assert(pModule && "pModule parameter must not be nullptr");
    assert(pResult && "Build results pointer must not be nullptr");

    validateVectorizerMode(pResult->LogS());

    // Check if given program is valid for the target.
    if (!isProgramValid(pModule, pResult))
    {
        throw Exceptions::CompilerException(
          "Program is not valid for this target", CL_DEV_INVALID_BINARY);
    }

    CompilerBuildOptions buildOptions(pBuildOptions);
    if (m_bIsEyeQEmulator)
    {
        buildOptions.SetRelaxedMath(false);
        buildOptions.SetDenormalsZero(true);
    }

    // Record the debug control flags.
    m_debug = buildOptions.GetDebugInfoFlag();
    m_useNativeDebugger = buildOptions.GetUseNativeDebuggerFlag();

    materializeSpirTriple(pModule);

    std::unique_ptr<TargetMachine> targetMachine(GetTargetMachine(pModule));

    pModule->setDataLayout(targetMachine->createDataLayout());

    //
    // Apply IR=>IR optimizations
    //

    intel::OptimizerConfig optimizerConfig( m_CpuId,
                                            m_transposeSize,
                                            m_IRDumpAfter,
                                            m_IRDumpBefore,
                                            m_IRDumpDir,
                                            targetMachine.get(),
                                            m_debug,
                                            m_useNativeDebugger,
                                            buildOptions.GetProfilingFlag(),
                                            buildOptions.GetDisableOpt(),
                                            buildOptions.GetRelaxedMath(),
                                            buildOptions.GetUniformWGSize(),
                                            m_bIsFPGAEmulator,
                                            m_bIsEyeQEmulator,
                                            m_dumpHeuristicIR,
                                            buildOptions.GetAPFLevel(),
                                            m_rtLoopUnrollFactor);
    Optimizer optimizer(pModule, GetBuiltinModuleList(), &optimizerConfig);
    optimizer.Optimize();

    if(optimizer.hasUndefinedExternals())
    {
        Utils::LogUndefinedExternals( pResult->LogS(), optimizer.GetUndefinedExternals());
        throw Exceptions::CompilerException( "Failed to parse IR", CL_DEV_INVALID_BINARY);
    }

    if( optimizer.hasRecursion())
    {
      Utils::LogHasRecursion(
          pResult->LogS(), optimizer.GetInvalidFunctions(
              Optimizer::InvalidFunctionType::RECURSION));

      throw Exceptions::CompilerException( "Recursive call detected.",
                                            CL_DEV_INVALID_BINARY);
    }

    if( optimizer.hasFpgaPipeDynamicAccess())
    {
      Utils::LogHasFpgaPipeDynamicAccess(
          pResult->LogS(), optimizer.GetInvalidFunctions(
              Optimizer::InvalidFunctionType::FPGA_PIPE_DYNAMIC_ACCESS));

      throw Exceptions::CompilerException(
          "Dynamic access to FPGA pipe or channel detected.",
          CL_DEV_INVALID_BINARY);
    }

    if (optimizer.hasFPGAChannelsWithDepthIgnored())
    {
        // In this case build is not failed, we just need to show diagnostics
        Utils::LogHasFPGAChannelsWithDepthIgnored(pResult->LogS(),
            optimizer.GetInvalidGlobals(
                Optimizer::InvalidGVType::FPGA_DEPTH_IS_IGNORED));
    }

    //
    // Populate the build results
    //
    pResult->SetBuildResult( CL_DEV_SUCCESS );

    // Execution Engine depends on module configuration and should
    // be created per module.
    // The object that owns module, should own the execution engine
    // as well and be responsible for release, of course.

    // Compiler creates execution engine but only keeps a pointer to the latest
    CreateExecutionEngine(pModule);
    return pModule;
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
void Compiler::LoadBuiltinModules(BuiltinLibrary* pLibrary,
                   llvm::SmallVector<llvm::Module*, 2>& builtinsModules) const
{
    llvm::SmallVector<std::unique_ptr<llvm::MemoryBuffer>, 4>
        rtlBuffersForEyeQEmulationMode = pLibrary->GetRtlBuffersForEyeQEmulationMode();
    // This is an empty loop unless in EyeQ emulation mode
    for (std::unique_ptr<llvm::MemoryBuffer> &rtlBufferForEyeQEmulationMode :
         rtlBuffersForEyeQEmulationMode) {
        assert(rtlBufferForEyeQEmulationMode &&
               "rtlBufferForEyeQEmulationMode is NULL pointer");
        llvm::ErrorOr<std::unique_ptr<llvm::Module>> spModuleOrErr =
            expectedToErrorOrAndEmitErrors(
                *m_pLLVMContext,
                llvm::getOwningLazyBitcodeModule(
                    std::move(rtlBufferForEyeQEmulationMode), *m_pLLVMContext));

        if (!spModuleOrErr) {
            throw Exceptions::CompilerException(
                "Failed to allocate/parse buitin module");
        }
        auto *pModule = spModuleOrErr.get().release();
        builtinsModules.push_back(pModule);
    }

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

bool Compiler::isProgramValid(llvm::Module* pModule, ProgramBuildResult* pResult) const
{
    // Check for the limitation: "Images are not supported on Xeon Phi".
    if (m_CpuId.GetCPU() == CPU_KNL &&
        CompilationUtils::isImagesUsed(*pModule)) {
      pResult->LogS() << "Images are not supported on given device.\n";
      return false;
    }

    return true;
}

void Compiler::validateVectorizerMode(llvm::raw_ostream& log) const
{
    // Validate if the vectorized mode valid and supported by the target arch.
    // If not then issue an error and interrupt the build.

    switch (m_CpuId.isTransposeSizeSupported(m_transposeSize)) {
    case SUPPORTED:
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
