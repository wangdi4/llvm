// INTEL CONFIDENTIAL
//
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

#include "CPUCompiler.h"
#include "BuiltinModuleManager.h"
#include "BuiltinModules.h"
#include "LLDJITBuilder.h"
#include "ObjectCodeCache.h"
#include "VectorizerUtils.h"
#include "cl_cpu_detect.h"
#include "cl_types.h"
#include "cpu_dev_limits.h"
#include "exceptions.h"
// Reference a symbol in JIT.cpp and MCJIT.cpp so that static or global
// constructors are called
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/JITEventListener.h"
#include "llvm/ExecutionEngine/MCJIT.h"
#include "llvm/ExecutionEngine/Orc/RTDyldObjectLinkingLayer.h"
#include "llvm/ExecutionEngine/SectionMemoryManager.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/TargetParser/Triple.h"
#include "llvm/Transforms/SYCLTransforms/Utils/CompilationUtils.h"
#include <string>

using namespace llvm;
using CPUDetect = Intel::OpenCL::Utils::CPUDetect;

namespace Intel {
namespace OpenCL {
namespace DeviceBackend {

/*
 *  Constants
 */
const char *CPU_ARCH_AUTO = "auto";

TargetOptions ExternInitTargetOptionsFromCodeGenFlags();
JITEventListener *getGDBRegistrationListenerInstance();

/*
 * Utility methods
 */
namespace Utils {
/**
 * Return the CPU identifier (from CPUDetect enumeration) given the CPU name.
 * CPU Name may be equal to 'auto', in this case the CPU detection will be
 * performed
 */
Intel::OpenCL::Utils::ECPU GetOrDetectCpuId(const std::string &cpuArch) {
  Intel::OpenCL::Utils::ECPU cpuId = Intel::OpenCL::Utils::CPU_UNKNOWN;
  CPUDetect *pCpuDetect = CPUDetect::GetInstance();
  if (CPU_ARCH_AUTO == cpuArch) {
    cpuId = pCpuDetect->GetCPU();
  } else {
    cpuId = CPUDetect::IsValidCPUName(cpuArch.c_str())
                ? CPUDetect::GetCPUByName(cpuArch.c_str())
                : Intel::OpenCL::Utils::CPU_UNKNOWN;
  }

  if (Intel::OpenCL::Utils::CPU_UNKNOWN == cpuId) {
    throw Exceptions::CompilerException("Unsupported CPU Architecture");
  }

  return cpuId;
}

// Temporary solution for WeightedInstCountAnalysis pass to obtain correct ISA.
static void applyCpuIdLLVMOptions(const CPUDetect *CPUId) {
  SmallVector<const char *, 3> Args;
  Args.push_back("CPUCompiler");

  std::string ISA = "-sycl-vector-variant-isa-encoding-override=";
  switch (VectorizerUtils::getCPUIdISA(CPUId)) {
  case VFISAKind::AVX512:
    ISA += "AVX512Core";
    break;
  case VFISAKind::AVX2:
    ISA += "AVX2";
    break;
  case VFISAKind::AVX:
    ISA += "AVX1";
    break;
  default:
    ISA += "SSE42";
  }
  Args.push_back(ISA.c_str());

  Args.push_back(nullptr);
  cl::ParseCommandLineOptions(Args.size() - 1, Args.data());
}

/**
 * Splits the given string using the supplied delimiter
 * populates the given vector of strings
 */
void SplitString(const std::string &s, const char *d,
                 SmallVectorImpl<std::string> &v) {
  StringRef sr(s);
  SmallVector<StringRef, 2> sv;

  sr.split(sv, d, -1, false);
  std::transform(sv.begin(), sv.end(), std::back_inserter(v),
                 [](StringRef s) { return std::string(s); });
}

} // namespace Utils

BuiltinModules *CPUCompiler::GetOrLoadBuiltinModules(bool ForceLoad) {
  std::lock_guard<sys::Mutex> Locked(m_builtinModuleMutex);
  auto TID = std::this_thread::get_id();
  auto It = m_builtinModules.find(TID);
  if (ForceLoad && It != m_builtinModules.end())
    It->second.reset();
  if (ForceLoad || It == m_builtinModules.end()) {
    BuiltinModuleManager *Manager = BuiltinModuleManager::GetInstance();
    BuiltinLibrary *Library = m_bIsFPGAEmulator
                                  ? Manager->GetOrLoadFPGAEmuLibrary(m_CpuId)
                                  : Manager->GetOrLoadCPULibrary(m_CpuId);
    auto bltnModules = LoadBuiltinModules(Library);
    m_builtinModules[TID].reset(new BuiltinModules(std::move(bltnModules)));
    setBuiltinInitLog(Library->getLog());
  }
  return m_builtinModules[TID].get();
}

// If binary doesn't match current cpu arch but its maximum supported
// instruction can be supported by current cpu, load builtin modules again
// for backwards compatibility.
void CPUCompiler::SetBuiltinModules(const std::string &cpuName,
                                    const std::string &cpuFeatures = "") {
  // config.GetLoadBuiltins should be true
  SelectCpu(cpuName, cpuFeatures);
  Utils::applyCpuIdLLVMOptions(m_CpuId);
  GetOrLoadBuiltinModules(true);
}

CPUCompiler::CPUCompiler(const ICompilerConfig &config) : Compiler(config) {
  SelectCpu(config.GetCpuArch(), config.GetCpuFeatures());
  Utils::applyCpuIdLLVMOptions(m_CpuId);

  // Initialize the BuiltinModules
  if (config.GetLoadBuiltins()) {
    std::lock_guard<sys::Mutex> Locked(m_builtinModuleMutex);
    BuiltinModuleManager *Manager = BuiltinModuleManager::GetInstance();
    // Load libraries into current process so that they are visiable for jit
    // processing in function BuildLibraryProgram. BTW, builtin rtl files are
    // also loaded into buffers.
    std::ignore = m_bIsFPGAEmulator ? Manager->GetOrLoadFPGAEmuLibrary(m_CpuId)
                                    : Manager->GetOrLoadCPULibrary(m_CpuId);
  }

  // Create the listener that allows Amplifier to profile OpenCL kernels
  if (config.GetUseVTune()) {
    m_pVTuneListener.reset(JITEventListener::createIntelJITEventListener());
  }

  // Initialize asm parsers to support inline assembly
  InitializeAllAsmParsers();
}

CPUCompiler::~CPUCompiler() {}

void CPUCompiler::SelectCpu(const std::string &cpuName,
                            const std::string &cpuFeatures) {
  Intel::OpenCL::Utils::ECPU selectedCpuId = Utils::GetOrDetectCpuId(cpuName);
  Utils::SplitString(cpuFeatures, ",", m_forcedCpuFeatures);

  m_CpuId = CPUDetect::GetInstance();
  if (cpuName == CPU_ARCH_AUTO) {
    // CPU name and features are detected by llvm sys utils, and will be passed
    // to code generator. Setting disabled features, could avoid emitting the
    // illegal instruction which are disabled on underlying OS, CPU or virtual
    // machine.
    m_CpuId->GetDisabledCPUFeatures(m_forcedCpuFeatures);
    return;
  }

  if (selectedCpuId == Intel::OpenCL::Utils::CPU_SNB)
    m_forcedCpuFeatures.push_back("+avx");

  if (selectedCpuId == Intel::OpenCL::Utils::CPU_HSW)
    m_forcedCpuFeatures.push_back("+avx2");

  // Comment by Craig Topper: The F16C instructions are all encoded using the
  // VEX prefix which became available with AVX. That's why CPU_SANDYBRIDGE
  // appeared in condition.
  if ((selectedCpuId >= Intel::OpenCL::Utils::CPU_SNB) &&
      m_CpuId->IsFeatureSupported(Intel::OpenCL::Utils::CFS_F16C))
    m_forcedCpuFeatures.push_back("+f16c");

  if (selectedCpuId >= Intel::OpenCL::Utils::CPU_SKX) {
    m_forcedCpuFeatures.push_back("+avx512f");
    m_forcedCpuFeatures.push_back("+avx512cd");
    m_forcedCpuFeatures.push_back("+avx512bw");
    m_forcedCpuFeatures.push_back("+avx512dq");
    m_forcedCpuFeatures.push_back("+avx512vl");
    m_forcedCpuFeatures.push_back("+pku");
  }

  if (selectedCpuId == Intel::OpenCL::Utils::CPU_SKX)
    m_forcedCpuFeatures.push_back("+clwb");

  if (selectedCpuId >= Intel::OpenCL::Utils::CPU_CLX)
    m_forcedCpuFeatures.push_back("+avx512vnni");

  if (selectedCpuId >= Intel::OpenCL::Utils::CPU_ICL) {
    // CNL features
    m_forcedCpuFeatures.push_back("+avx512vbmi");
    m_forcedCpuFeatures.push_back("+avx512ifma");
    // ICL features
    m_forcedCpuFeatures.push_back("+avx512vbmi2");
    m_forcedCpuFeatures.push_back("+avx512bitalg");
    m_forcedCpuFeatures.push_back("+avx512vpopcntdq");
  }
  if (selectedCpuId == Intel::OpenCL::Utils::CPU_ICX) {
    // ICX features
    m_forcedCpuFeatures.push_back("+wbnoinvd");
    m_forcedCpuFeatures.push_back("+clwb");
    m_forcedCpuFeatures.push_back("+pconfig");
  }
  if (selectedCpuId >= Intel::OpenCL::Utils::CPU_SPR) {
    // SPR features
    m_forcedCpuFeatures.push_back("+amx-tile");
    m_forcedCpuFeatures.push_back("+amx-int8");
    m_forcedCpuFeatures.push_back("+amx-bf16");
    m_forcedCpuFeatures.push_back("+avx512fp16");
    m_forcedCpuFeatures.push_back("+avx512bf16");
  }
  if (selectedCpuId >= Intel::OpenCL::Utils::CPU_GNR) {
    m_forcedCpuFeatures.push_back("+amx-fp16");
    m_forcedCpuFeatures.push_back("+prefetchi");
  }

  // When CL_CONFIG_CPU_TARGET_ARCH env is set, we need to reset CPU according
  // to config
  m_CpuId->ResetCPU(selectedCpuId, m_forcedCpuFeatures);
}

void CPUCompiler::CreateExecutionEngine(Module *pModule) {
  // Compiler keeps a pointer to the execution engine object
  // and is not responsible for EE release
  CreateCPUExecutionEngine(pModule);
}

std::unique_ptr<ExecutionEngine> CPUCompiler::GetOwningExecutionEngine() {
  return std::move(m_pExecEngine);
}

std::unique_ptr<orc::LLJIT>
CPUCompiler::CreateLLJIT(Module *M, std::unique_ptr<TargetMachine> TM,
                         ObjectCodeCache *ObjCache) {
  // TargetMachine builder
  orc::JITTargetMachineBuilder JTMB((Triple(M->getTargetTriple())));
  if (!TM) {
    auto TMDefault = JTMB.createTargetMachine();
    if (!TMDefault)
      throw Exceptions::CompilerException("createTargetMachine failed");
    TM = std::move(*TMDefault);
  }

  // Create LLJIT instance
  Expected<std::unique_ptr<orc::LLJIT>> LLJITOrErr =
      orc::LLJITBuilder()
          .setJITTargetMachineBuilder(std::move(JTMB))
          .setCompileFunctionCreator(
              [&](orc::JITTargetMachineBuilder /*JTMB*/)
                  -> Expected<
                      std::unique_ptr<orc::IRCompileLayer::IRCompiler>> {
                return std::make_unique<orc::TMOwningSimpleCompiler>(
                    std::move(TM), ObjCache);
              })
          .setObjectLinkingLayerCreator(
              [&](orc::ExecutionSession &ES, const Triple &) {
                // TODO switch to default JITLink.
                auto GetMemMgr = []() {
                  return std::make_unique<SectionMemoryManager>();
                };
                return std::make_unique<orc::RTDyldObjectLinkingLayer>(
                    ES, std::move(GetMemMgr));
              })
          .create();
  if (!LLJITOrErr)
    throw Exceptions::CompilerException("Failed to create LLJIT");
  std::unique_ptr<orc::LLJIT> &LLJIT = LLJITOrErr.get();

  // Register JITEventListener
  orc::RTDyldObjectLinkingLayer &LL =
      static_cast<orc::RTDyldObjectLinkingLayer &>(LLJIT->getObjLinkingLayer());
  LL.registerJITEventListener(*getGDBRegistrationListenerInstance());
  if (m_pVTuneListener)
    LL.registerJITEventListener(*m_pVTuneListener);

  // Enable searching for symbols in the current process.
  const DataLayout &DL = LLJIT->getDataLayout();
  auto Generator = orc::DynamicLibrarySearchGenerator::GetForCurrentProcess(
      DL.getGlobalPrefix());
  if (!Generator)
    throw Exceptions::CompilerException(
        "Failed to create DynamicLibrarySearchGenerator");
  (void)LLJIT->getMainJITDylib().addGenerator(std::move(*Generator));

  // Add builtin function symbols
  if (auto Err =
          BuiltinModuleManager::GetInstance()->RegisterCPUBIFunctionsToLLJIT(
              m_bIsFPGAEmulator, LLJIT.get())) {
    logAllUnhandledErrors(std::move(Err), errs());
    throw Exceptions::CompilerException("Failed to add builtin symbols");
  }

  return std::move(LLJIT);
}

bool CPUCompiler::useLLDJITForExecution(Module *pModule) const {
#ifdef _WIN32
  if (CompilationUtils::isGeneratedFromOCLCPP(*pModule) &&
      !pModule->debug_compile_units().empty())
    return true;
  // Following line should be removed when amplifier is working with LLDJIT, see
  // CMPLRLLVM-50721.
  return m_buildOptions.GetDebugInfoFlag();
#else
  // The parameter is used in Windows code
  (void)pModule;
  return false;
#endif
}

bool CPUCompiler::isObjectFromLLDJIT(StringRef ObjBuf) const {
#ifdef _WIN32
  return LLDJIT::isObjectFromLLDJIT(ObjBuf);
#else
  return false;
#endif
}

void CPUCompiler::CreateCPUExecutionEngine(Module *pModule) {
#ifdef _WIN32
  LLDJITBuilder::prepareModuleForLLD(pModule);
  auto TargetMachine = GetTargetMachine(pModule);
  m_pExecEngine =
      std::move(LLDJITBuilder::CreateExecutionEngine(pModule, TargetMachine));
  if (!m_pExecEngine)
    throw Exceptions::CompilerException("Failed to create execution engine");

  if (m_pVTuneListener)
    m_pExecEngine->RegisterJITEventListener(m_pVTuneListener.get());
#else
  (void)pModule;
#endif
}

SmallVector<Module *, 2> &CPUCompiler::GetBuiltinModuleList() {
  BuiltinModules *BM = GetOrLoadBuiltinModules();
  assert(BM && "Invalid BuiltinModules");
  return BM->GetBuiltinModuleList();
}

std::unique_ptr<MemoryBuffer>
CPUCompiler::SimpleCompile(Module *module, ObjectCodeCache *objCache) {
  TargetMachine *targetMachine = GetTargetMachine(module);
  orc::SimpleCompiler simpleCompiler(*targetMachine, objCache);
  auto objBuffer = simpleCompiler(*module);
  if (!objBuffer)
    throw Exceptions::CompilerException(
        "Failed to compile module using SimpleCompiler");
  return std::move(*objBuffer);
}

void CPUCompiler::SetObjectCache(ObjectCodeCache *pCache) {
  assert(m_pExecEngine && "invalid ExecutionEngine");
  m_pExecEngine->setObjectCache(pCache);
}

} // namespace DeviceBackend
} // namespace OpenCL
} // namespace Intel
