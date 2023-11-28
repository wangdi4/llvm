// Copyright 2010 Intel Corporation.
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

#include "Compiler.h"
#include "BackendUtils.h"
#include "BuiltinModuleManager.h"
#include "BuiltinModules.h"
#include "CompilerConfig.h"
#include "OptimizerLTO.h"
#include "OptimizerOCL.h"
#include "cl_config.h"
#include "cl_env.h"
#include "cl_types.h"
#include "cpu_dev_limits.h"
#include "exceptions.h"

#include "llvm/ADT/StringSet.h"
#include "llvm/Bitcode/BitcodeReader.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/IR/Module.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/PrettyStackTrace.h"
#include "llvm/Transforms/SYCLTransforms/Utils/CompilationUtils.h"
#include "llvm/Transforms/SYCLTransforms/Utils/MetadataAPI.h"

#ifdef _WIN32
#include "lld/Common/TargetOptionsCommandFlags.h"
#endif
#include "llvm/CodeGen/CommandFlags.h"

#include <sstream>
#include <string>
#include <vector>

using namespace llvm;

namespace Intel {
namespace OpenCL {
namespace DeviceBackend {

TargetOptions
ExternInitTargetOptionsFromCodeGenFlags(llvm::Triple ModuleTriple) {
#ifdef _WIN32
  // We only link to LLD on Windows
  return lld::initTargetOptionsFromCodeGenFlags();
#else
  return codegen::InitTargetOptionsFromCodeGenFlags(ModuleTriple);
#endif
}

static codegen::RegisterCodeGenFlags CGF;

// Supported target triples.
const char *PC_LIN64 = "x86_64-pc-linux";          // Used for RH64/SLES64.
const char *PC_WIN32 = "i686-pc-win32-msvc-elf";   // Win 32 bit.
const char *PC_WIN64 = "x86_64-pc-win32-msvc-elf"; // Win 64 bit.

/*
 * Utility methods
 */
namespace Utils {
/**
 * Generates the log record (to the given stream) enumerating function names
   with recursive calls
 */
static void LogHasRecursion(llvm::raw_ostream &logs,
                            const std::vector<std::string> &externals) {
  logs << "Error: recursive call in function(s):\n";

  for (std::vector<std::string>::const_iterator i = externals.begin(),
                                                e = externals.end();
       i != e; ++i) {
    logs << *i << "\n";
  }
}
} // namespace Utils

ProgramBuildResult::ProgramBuildResult()
    : m_result(CL_DEV_SUCCESS), m_logStream(m_buildLog) {}

bool ProgramBuildResult::Succeeded() const {
  return CL_DEV_SUCCEEDED(m_result);
}

bool ProgramBuildResult::Failed() const { return CL_DEV_FAILED(m_result); }

std::string ProgramBuildResult::GetBuildLog() const {
  m_logStream.flush();
  return m_buildLog;
}

llvm::raw_ostream &ProgramBuildResult::LogS() { return m_logStream; }

void ProgramBuildResult::SetBuildResult(cl_dev_err_code code) {
  m_result = code;
}

cl_dev_err_code ProgramBuildResult::GetBuildResult() const { return m_result; }

CompilerBuildOptions::CompilerBuildOptions(const char *pBuildOpts) {
  if (!pBuildOpts)
    return;
  llvm::StringRef buildOptions(pBuildOpts);
  if (buildOptions.empty())
    return;

  llvm::SmallVector<llvm::StringRef, 8> splittedOptions;
  buildOptions.split(splittedOptions, " ");
  for (const auto &opt : splittedOptions) {
    std::string opts = opt.str();
    if (opt.equals("-profiling"))
      m_profiling = true;
    else if (opt.equals("-g"))
      m_debugInfo = true;
    else if (opt.equals("-coverage"))
      m_coverage = true;
    else if (opt.equals("-cl-fast-relaxed-math"))
      m_relaxedMath = true;
    else if (opt.equals("-cl-opt-disable"))
      m_disableOpt = true;
    else if (opt.equals("-cl-denorms-are-zero"))
      m_denormalsZero = true;
  }
}

bool Compiler::s_globalStateInitialized = false;

/*
 * This is a static method which must be called from the
 * single threaded environment before any instance of Compiler
 * class is created
 */
void Compiler::Init() {
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

void Compiler::InitGlobalState(const IGlobalCompilerConfig &config) {
  if (s_globalStateInitialized)
    return;

  SmallVector<const char *, 32> Args;

  Args.push_back("OclBackend");
  std::string Env;
#ifndef NDEBUG
  if (Intel::OpenCL::Utils::getEnvVar(Env, "CL_CONFIG_COMPILER_DEBUG")) {
    Args.push_back("-print-after-all");
    Args.push_back("-print-before-all");
    Args.push_back("-debug");
  }
#endif

#if INTEL_CUSTOMIZATION
  if (!Intel::OpenCL::Utils::getEnvVar(Env, "DISABLE_INFER_AS"))
    Args.push_back("-infer-as-rewrite-opencl-bis");
#endif // INTEL_CUSTOMIZATION

  // Loops #pragma unroll are unrolled up to -pragma-unroll-threshold, which
  // is set by default to a huge value, and it basically allows to fully
  // unroll any loop. Needless to say, this can by suboptimal for x86 when
  // applied to big loops. FPGA device, in opposite, benefits from fully
  // unrolled loops, so most of FPGA workloads unroll every single loop.
  // Threshold heuristic set so as not to affect the performance negatively
  if (FPGA_EMU_DEVICE == config.TargetDevice())
    Args.push_back("-pragma-unroll-threshold=3072");

  std::string TimePasses;
  if (config.EnableTiming() && false == config.InfoOutputFile().empty()) {
    TimePasses = "-info-output-file=" + config.InfoOutputFile();
    Args.push_back("--time-passes");
    Args.push_back(TimePasses.c_str());
  }

  for (const std::string &Option : config.LLVMOptions())
    Args.push_back(Option.c_str());

  Optimizer::initOptimizerOptions();

  // Disable unrolling with runtime trip count. It is harmful for
  // sycl_benchmarks/dnnbench-pooling.
  Args.push_back("-unroll-runtime=false");

  // Threshold is tuned based on sycl_benchmarks/mandelbrot-dpd,dnnbench-norm.
  // For dnnbench-norm, we need to disable vectorization on single-task kernels
  // to observe the impact of the threshold.
  Args.push_back("-unroll-partial-threshold=30");

  // inline threshold is not exposed by standard new pass manager
  // pipeline, so we have to set threshold globally here.
  Args.push_back("-inline-threshold=16384");

  // Handle CL_CONFIG_LLVM_OPTIONS at the end so that it can pass an option to
  // overturn a previously added option.
  std::vector<std::string> LastOptions;
  if (Intel::OpenCL::Utils::getEnvVar(Env, "CL_CONFIG_LLVM_OPTIONS")) {
    LastOptions = std::move(SplitString(Env, ' '));
    for (const auto &Option : LastOptions)
      Args.push_back(Option.c_str());
  }

  Args.push_back(nullptr);

  cl::ParseCommandLineOptions(Args.size() - 1, Args.data());

  if (!config.DisableStackDump())
    llvm::EnablePrettyStackTrace();

  s_globalStateInitialized = true;
}

Compiler::Compiler(const ICompilerConfig &config)
    : m_bIsFPGAEmulator(FPGA_EMU_DEVICE == config.TargetDevice()),
      m_transposeSize(config.GetTransposeSize()),
      m_rtLoopUnrollFactor(config.GetRTLoopUnrollFactor()),
      m_dumpHeuristicIR(config.GetDumpHeuristicIRFlag()),
      m_streamingAlways(config.GetStreamingAlways()),
      m_expensiveMemOpts(config.GetExpensiveMemOpts()),
      m_passManagerType(config.GetPassManagerType()),
      m_subGroupConstructionMode(config.GetSubGroupConstructionMode()) {}

Compiler::~Compiler() {}

void Compiler::materializeSpirTriple(llvm::Module *M) {
  assert(Triple(M->getTargetTriple()).isSPIR() && "Triple is not spir!");

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

llvm::TargetMachine *Compiler::GetTargetMachine(llvm::Module *pModule) const {
  std::string ErrorString;

  // Leaving MArch blank implies using auto-detect
  llvm::StringRef MArch = "";
  llvm::StringRef MCPU = m_CpuId->GetCPUName();

  llvm::Triple ModuleTriple(pModule->getTargetTriple());

  llvm::TargetOptions TargetOpts =
      ExternInitTargetOptionsFromCodeGenFlags(ModuleTriple);

#if INTEL_CUSTOMIZATION
  // When -cl-fast-relaxed-math is enabled, Codegen's fast fp-model is too
  // aggressive for OpenCL, leading to "fdiv fast" precision loss (violates
  // the OpenCL Spec).
  // Disabling Codegen's -do-x86-global-fma optimization in this situation
  // could improve the precision (Only apply this for OpenCL program).
  if (!CompilationUtils::isGeneratedFromOCLCPP(*pModule) &&
      CompilationUtils::hasFDivWithFastFlag(pModule))
    TargetOpts.DoFMAOpt = false;
#endif // INTEL_CUSTOMIZATION

  OptimizationLevel OptLevel =
      BackendUtils::getOptLevel(m_buildOptions.GetDisableOpt(), *pModule);
  // Align OpenCL with DPC++ that the default opt-level is O2.
  bool IsOCL = !CompilationUtils::isGeneratedFromOCLCPP(*pModule);
  llvm::CodeGenOptLevel CGOptLevel =
      (OptLevel == OptimizationLevel::O0)   ? llvm::CodeGenOptLevel::None
      : (OptLevel == OptimizationLevel::O1) ? llvm::CodeGenOptLevel::Less
      : (OptLevel == OptimizationLevel::O2 || IsOCL)
          ? llvm::CodeGenOptLevel::Default
          : llvm::CodeGenOptLevel::Aggressive;

  llvm::EngineBuilder Builder;

  Builder.setErrorStr(&ErrorString);
  Builder.setOptLevel(CGOptLevel);
  Builder.setTargetOptions(TargetOpts);

  auto *TargetMachine =
      Builder.selectTarget(ModuleTriple, MArch, MCPU, m_forcedCpuFeatures);

  if (nullptr == TargetMachine) {
    throw Exceptions::CompilerException("Failed to create TargetMachine: " +
                                        ErrorString);
  }

  return TargetMachine;
}

llvm::Module *
Compiler::BuildProgram(llvm::Module *pModule, const char *pBuildOptions,
                       ProgramBuildResult *pResult,
                       std::unique_ptr<llvm::TargetMachine> &targetMachine) {
  assert(pModule && "pModule parameter must not be nullptr");
  assert(pResult && "Build results pointer must not be nullptr");

  validateVectorizerMode(pResult->LogS());

  m_buildOptions = CompilerBuildOptions(pBuildOptions);

  materializeSpirTriple(pModule);

  targetMachine.reset(GetTargetMachine(pModule));

  pModule->setDataLayout(targetMachine->createDataLayout());

  //
  // Apply IR=>IR optimizations
  //

  intel::OptimizerConfig optimizerConfig(
      m_CpuId, m_transposeSize, targetMachine.get(),
      m_buildOptions.GetProfilingFlag(), m_buildOptions.GetDisableOpt(),
      m_buildOptions.GetRelaxedMath(), m_buildOptions.GetCoverage(),
      m_bIsFPGAEmulator, m_dumpHeuristicIR, m_rtLoopUnrollFactor,
      m_streamingAlways, m_expensiveMemOpts, m_subGroupConstructionMode);
  auto &BIModules = GetBuiltinModuleList();
  std::unique_ptr<Optimizer> optimizer;
  switch (m_passManagerType) {
  case PM_OCL:
    optimizer =
        std::make_unique<OptimizerOCL>(*pModule, BIModules, optimizerConfig);
    break;
  default:
    optimizer =
        std::make_unique<OptimizerLTO>(*pModule, BIModules, optimizerConfig);
    break;
  };

  optimizer->Optimize(pResult->LogS());

  if (const std::string &Msg = optimizer->getExceptionMsg(); !Msg.empty())
    throw Exceptions::CompilerException(Msg, CL_DEV_INVALID_BINARY);

  //
  // Populate the build results
  //
  pResult->SetBuildResult(CL_DEV_SUCCESS);

  if (useLLDJITForExecution(pModule)) {
    // Execution Engine depends on module configuration and should
    // be created per module.
    // The object that owns module, should own the execution engine
    // as well and be responsible for release, of course.

    // Compiler creates execution engine but only keeps a pointer to the
    // latest
    CreateExecutionEngine(pModule);
  }
  return pModule;
}

std::unique_ptr<llvm::Module>
Compiler::ParseModuleIR(llvm::MemoryBuffer *pIRBuffer) {
  LLVMContext &Ctx = getLLVMContext();
  // Parse the module IR
  llvm::ErrorOr<std::unique_ptr<llvm::Module>> pModuleOrErr =
      expectedToErrorOrAndEmitErrors(
          Ctx, llvm::parseBitcodeFile(pIRBuffer->getMemBufferRef(), Ctx));
  if (!pModuleOrErr) {
    throw Exceptions::CompilerException(std::string("Failed to parse IR: ") +
                                            pModuleOrErr.getError().message(),
                                        CL_DEV_INVALID_BINARY);
  }
  return std::move(pModuleOrErr.get());
}

// RTL builtin modules consist of two libraries.
// The first is shared across all HW architectures and the second one
// is optimized for a specific HW architecture.
SmallVector<std::unique_ptr<Module>, 2>
Compiler::LoadBuiltinModules(BuiltinLibrary *pLibrary) {
  SmallVector<std::unique_ptr<Module>, 2> builtinsModules;
  LLVMContext &Ctx = getLLVMContext();

  std::unique_ptr<llvm::MemoryBuffer> rtlBuffer(pLibrary->GetRtlBuffer());
  assert(rtlBuffer && "pRtlBuffer is NULL pointer");
  llvm::ErrorOr<std::unique_ptr<llvm::Module>> spModuleOrErr =
      expectedToErrorOrAndEmitErrors(
          Ctx, llvm::getOwningLazyBitcodeModule(std::move(rtlBuffer), Ctx));

  if (!spModuleOrErr) {
    // Failed to load runtime library
    spModuleOrErr = llvm::ErrorOr<std::unique_ptr<llvm::Module>>(
        std::unique_ptr<llvm::Module>(new llvm::Module("dummy", Ctx)));
    if (!spModuleOrErr) {
      throw Exceptions::CompilerException(
          "Failed to allocate/parse buitin module");
    }
  } else {
    spModuleOrErr.get().get()->setModuleIdentifier("RTLibrary");
  }

  auto &spModule = spModuleOrErr.get();
  auto *pModule = spModule.get();
  // Note: the order of builtinsModules matters, BuiltinImport pass will
  // try to import functions in a precedent module first.
  // We explicitly insert the target-specific module first, so that the
  // function definitions in the target-specific RTL can override those of
  // shared RTL.
  builtinsModules.push_back(std::move(spModule));

  // the shared RTL is loaded here
  std::unique_ptr<llvm::MemoryBuffer> RtlBufferSvmlShared(
      pLibrary->GetRtlBufferSvmlShared());
  llvm::Expected<std::unique_ptr<llvm::Module>> spModuleSvmlSharedOrErr(
      llvm::getOwningLazyBitcodeModule(std::move(RtlBufferSvmlShared), Ctx));

  if (!spModuleSvmlSharedOrErr) {
    throw Exceptions::CompilerException(
        "Failed to allocate/parse buitin module");
  }

  auto &pModuleSvmlShared = spModuleSvmlSharedOrErr.get();
  // on both 64-bit and 32-bit platform the same shared RTL contatinig platform
  // independent byte code is used, so set triple and data layout for shared RTL
  // from particular RTL in order to avoid warnings from linker.
  pModuleSvmlShared->setTargetTriple(pModule->getTargetTriple());
  pModuleSvmlShared->setDataLayout(pModule->getDataLayout());

  // Make sure the shared RTL module is inserted after the target-specific
  // module.
  builtinsModules.push_back(std::move(pModuleSvmlShared));
  return builtinsModules;
}

void Compiler::validateVectorizerMode(llvm::raw_ostream &log) const {
  // Validate if the vectorized mode valid and supported by the target arch.
  // If not then issue an error and interrupt the build.

  switch (m_CpuId->isTransposeSizeSupported(m_transposeSize)) {
  case Intel::OpenCL::Utils::SUPPORTED:
    return;

  case Intel::OpenCL::Utils::INVALID:
    log << "The specified vectorizer mode is invalid.\n";
    break;

  case Intel::OpenCL::Utils::UNSUPPORTED:
    log << "The specified vectorizer mode (" << m_transposeSize
        << ") is not supported by the target architecture.\n";
    break;
  }
  throw Exceptions::CompilerException("Failed to apply the vectorizer mode.",
                                      CL_DEV_INVALID_BUILD_OPTIONS);
}

const std::string Compiler::GetBitcodeTargetTriple(const void *pBinary,
                                                   size_t uiBinarySize) const {

  std::unique_ptr<MemoryBuffer> spIRBuffer(MemoryBuffer::getMemBuffer(
      StringRef(static_cast<const char *>(pBinary), uiBinarySize), "", false));
  llvm::Expected<std::string> strTargetTriple =
      llvm::getBitcodeTargetTriple(spIRBuffer->getMemBufferRef());
  if (!strTargetTriple || *strTargetTriple == "") {
    throw Exceptions::CompilerException(
        std::string("Failed to get target triple from bitcode!"),
        CL_DEV_INVALID_BINARY);
  }

  return *strTargetTriple;
}

llvm::LLVMContext *Compiler::resetLLVMContextForCurrentThread() {
  std::lock_guard<llvm::sys::Mutex> Locked(m_LLVMContextMutex);
  auto NewCtx = std::make_unique<LLVMContext>();
  auto NewCtxPtr = NewCtx.get();
  auto It = m_LLVMContexts.find(std::this_thread::get_id());
  assert(It != m_LLVMContexts.end() && "LLVMContext should already exist");
  It->second.swap(NewCtx);
  m_depletedLLVMContexts.push_back(std::move(NewCtx));
  return NewCtxPtr;
}

llvm::LLVMContext &Compiler::getLLVMContext() {
  std::lock_guard<llvm::sys::Mutex> Locked(m_LLVMContextMutex);
  auto TID = std::this_thread::get_id();
  auto It = m_LLVMContexts.find(TID);
  if (It == m_LLVMContexts.end()) {
    It = m_LLVMContexts.emplace(TID, std::make_unique<LLVMContext>()).first;
  }
  return *It->second;
}

} // namespace DeviceBackend
} // namespace OpenCL
} // namespace Intel
