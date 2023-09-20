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

#include "ProgramBuilder.h"
#include "BackendConfiguration.h"
#include "BitCodeContainer.h"
#include "BuiltinModuleManager.h"
#include "BuiltinModules.h"
#include "CPUCompiler.h"
#include "Kernel.h"
#include "KernelProperties.h"
#include "ObjectCodeCache.h"
#include "ObjectCodeContainer.h"
#include "Optimizer.h"
#include "Program.h"
#include "SystemInfo.h"
#include "VectorizerUtils.h"
#include "cl_cpu_detect.h"
#include "cl_sys_info.h"
#include "cl_types.h"
#include "cpu_dev_limits.h"
#include "exceptions.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/IR/Argument.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/SYCLTransforms/Utils/CompilationUtils.h"
#include "llvm/Transforms/SYCLTransforms/Utils/MetadataAPI.h"
#include "llvm/Transforms/SYCLTransforms/Utils/SYCLStatistic.h"
#include "llvm/Transforms/Utils/UnifyFunctionExitNodes.h"

#include <algorithm>
#include <atomic>
#include <sstream>
#include <string>
#include <vector>

#define DEBUG_TYPE "ProgramBuilder"

using namespace llvm;
using namespace SYCLKernelMetadataAPI;
using namespace intel;

namespace Intel {
namespace OpenCL {
namespace DeviceBackend {

static void BEFatalErrorHandler(void * /*user_data*/, const char *reason,
                                bool /*gen_crash_diag*/) {
  errs()
      << "**Internal compiler error** " << reason << "\n"
      << "Please report the issue on Intel OpenCL forum \n"
      << "https://software.intel.com/en-us/forums/opencl for assistance. \n ";
  abort();
}

static unsigned getEqualizerDumpFileId() {
  static std::atomic<unsigned> fileId(0);
  fileId++;

  return fileId.load(std::memory_order_relaxed);
}

static unsigned getVolcanoDumpFileId() {
  static std::atomic<unsigned> fileId(0);
  fileId++;

  return fileId.load(std::memory_order_relaxed);
}

static unsigned getReplaceIRBeforeOptimizerFileId() {
  static std::atomic<unsigned> FileId(0);
  unsigned Id = FileId.load(std::memory_order_relaxed);
  FileId++;
  return Id;
}

static unsigned getReplaceIRAfterOptimizerFileId() {
  static std::atomic<unsigned> FileId(0);
  unsigned Id = FileId.load(std::memory_order_relaxed);
  FileId++;
  return Id;
}

// Returns the memory buffer of the Program object bytecode
static MemoryBuffer *GetProgramMemoryBuffer(Program *pProgram) {
  const BitCodeContainer *pCodeContainer =
      static_cast<const BitCodeContainer *>(
          pProgram->GetProgramIRCodeContainer());
  return pCodeContainer->GetMemoryBuffer();
}

namespace Utils {
/// @brief helper funtion to set RuntimeService in Kernel objects from KernelSet
void UpdateKernelsWithRuntimeService(const RuntimeServiceSharedPtr &rs,
                                     KernelSet *pKernels) {
  for (unsigned cnt = 0; cnt < pKernels->GetCount(); ++cnt) {
    Kernel *pK = pKernels->GetKernel(cnt);
    pK->SetRuntimeService(rs);
  }
}

/// Apply runtime config to kernels.
void UpdateKernelsWithRuntimeConfig(const ICompilerConfig *Config,
                                    KernelSet *Kernels) {
  for (size_t I = 0, E = Kernels->GetCount(); I != E; ++I)
    Kernels->GetKernel(I)->SetRuntimeConfig(Config);
}

} // namespace Utils

ProgramBuilder::ProgramBuilder(IAbstractBackendFactory *pBackendFactory,
                               std::unique_ptr<ICompilerConfig> config)
    : m_pBackendFactory(pBackendFactory), m_config(std::move(config)),
      m_targetDevice(m_config->TargetDevice()),
      m_dumpFilenamePrefix(m_config->GetDumpFilenamePrefix()) {
  // prepare default base file name for stat file in the following cases:
  // stats are enabled but the user didn't set up the base file name
  // the user set up as base file name only a directory name, i.e. it ends
  // with \ or /
  // the default file name is the running executable name
  if (m_dumpFilenamePrefix.empty() ||
      sys::path::is_separator(*m_dumpFilenamePrefix.rbegin())) {
    std::string name = Utils::SystemInfo::GetExecutableFilename();
    // if still no meaningful name just use "Program" as module name
    if (name.empty())
      name = "Program";
    m_dumpFilenamePrefix += name;
    if (SYCLStatistic::isEnabled())
      m_statWkldName = name;
  }
}

ProgramBuilder::~ProgramBuilder() {}

std::string
ProgramBuilder::generateDumpFilename(const std::string &hash, unsigned fileId,
                                     const std::string &suffix) const {
  return m_dumpFilenamePrefix + "_" + std::to_string(fileId) + "_" + hash +
         suffix;
}

void ProgramBuilder::DumpModuleStats(Program *program, Module *pModule,
                                     bool isEqualizerStats) {
  if (!SYCLStatistic::isEnabled())
    return;

  // use sequential number to distinguish dumped files
  unsigned fileId =
      isEqualizerStats ? getEqualizerDumpFileId() : getVolcanoDumpFileId();
  std::string suffix = isEqualizerStats ? "_eq.ll" : ".ll";
  std::string fileName =
      generateDumpFilename(program->GenerateHash(), fileId, suffix);

  // if stats are enabled dump module info
  if (SYCLStatistic::isEnabled()) {
    SYCLStatistic::setModuleStatInfo(
        pModule, VERSIONSTRING,
        m_statWkldName.c_str(),                           // workload name
        (m_statWkldName + std::to_string(fileId)).c_str() // module name
    );
  }
  // dump IR with stats
  std::error_code ec;
  raw_fd_ostream IRFD(fileName.c_str(), ec, sys::fs::FA_Write);
  if (!ec)
    pModule->print(IRFD, 0);
  else
    throw Exceptions::CompilerException(ec.message());
}

static Module *replaceModule(Compiler *Cmplr, Program *Prog,
                             ProgramBuildResult &BuildResult,
                             bool BeforeOptimizer) {
  std::string EnvName = BeforeOptimizer
                            ? "CL_CONFIG_REPLACE_IR_BEFORE_OPTIMIZER"
                            : "CL_CONFIG_REPLACE_IR_AFTER_OPTIMIZER";
  std::string Env;
  if (!Intel::OpenCL::Utils::getEnvVar(Env, EnvName) || Env.empty())
    return Prog->GetModule();

  SmallVector<StringRef, 4> FileNames;
  SplitString(Env, FileNames, ",");
  StringRef FileName =
      FileNames[BeforeOptimizer ? getReplaceIRBeforeOptimizerFileId()
                                : getReplaceIRAfterOptimizerFileId()];

  std::string WarningMsg =
      (Twine("WARNING: replace module IR ") +
       Twine(BeforeOptimizer ? "before" : "after") + Twine(" optimizer : ") +
       Twine(FileName) + Twine("\n"))
          .str();
  dbgs() << WarningMsg;
  BuildResult.LogS() << WarningMsg;
  // Create new LLVMContext instead of reusing pModule's LLVMContext, in
  // order to avoid type renaming in textual IR dump.
  LLVMContext *ReplaceModuleCtx = Cmplr->resetLLVMContextForCurrentThread();
  // Reload builtin modules since context is changed.
  static_cast<CPUCompiler *>(Cmplr)->GetOrLoadBuiltinModules(
      /*ForceLoad*/ true);
  assert(ReplaceModuleCtx && "invalid replace context");
  SMDiagnostic Err;
  std::unique_ptr<Module> ReplaceModule =
      parseIRFile(FileName, Err, *ReplaceModuleCtx);
  if (!ReplaceModule) {
    Err.print("", errs());
    throw Exceptions::DeviceBackendExceptionBase(
        std::string("Failed to load module IR to replace"));
  }
  Prog->GetModuleOwner().reset(nullptr);
  Prog->SetModule(std::move(ReplaceModule));
  return Prog->GetModule();
}

void ProgramBuilder::ParseProgram(Program *pProgram) {
  try {
    assert(!pProgram->HasCachedExecutable() &&
           "Program must not be loaded from cache");
    pProgram->SetModule(
        GetCompiler()->ParseModuleIR(GetProgramMemoryBuffer(pProgram)));
  } catch (Exceptions::CompilerException &e) {
    throw Exceptions::DeviceBackendExceptionBase(e.what());
  }
}

cl_dev_err_code
ProgramBuilder::BuildProgram(Program *pProgram,
                             const ICLDevBackendOptions *pOptions,
                             const char *pBuildOpts) {
  assert(pProgram && "Program parameter must not be nullptr");
  ProgramBuildResult buildResult;

  try {
    if (pProgram->HasCachedExecutable()) {
      if (ReloadProgramFromCachedExecutable(pProgram)) {
        std::string log = "Reload Program Binary Object.";
        pProgram->SetBuildLog(log);
        return CL_DEV_SUCCESS;
      }
    }
    Compiler *pCompiler = GetCompiler();
    Module *pModule = pProgram->GetModule();

    if (!pModule) {
      ParseProgram(pProgram);
      pModule = pProgram->GetModule();
    }
    assert(pModule && "Module parsing has failed without exception. Strange");

    pModule = replaceModule(pCompiler, pProgram, buildResult,
                            /*BeforeOptimizer*/ true);

    // If environment variable CL_CONFIG_DUMP_IR_BEFORE_OPTIMIZER is set to
    // true, then we dump IR before optimization.
    std::string Env;
    if (Intel::OpenCL::Utils::getEnvVar(Env,
                                        "CL_CONFIG_DUMP_IR_BEFORE_OPTIMIZER") &&
        Intel::OpenCL::Utils::ConfigFile::ConvertStringToType<bool>(Env))
      DumpModuleStats(pProgram, pModule, /*isEqualizerStats = */ true);

    // Handle LLVM ERROR which can occured during build programm
    // Need to do it to eliminate RT hanging when clBuildProgramm failed
    ScopedFatalErrorHandler FatalErrorHandler(BEFatalErrorHandler, nullptr);

    std::unique_ptr<TargetMachine> targetMachine;
    pCompiler->BuildProgram(pModule, pBuildOpts, &buildResult, targetMachine);

    pProgram->SetBuiltinModule(pCompiler->GetBuiltinModuleList());

    // init refcounted runtime service shared storage between program
    // and kernels
    RuntimeServiceSharedPtr lRuntimeService =
        RuntimeServiceSharedPtr(new RuntimeServiceImpl);
    // set runtime service for the program
    pProgram->SetRuntimeService(lRuntimeService);

    pModule = replaceModule(pCompiler, pProgram, buildResult,
                            /*BeforeOptimizer*/ false);

    // Dump module IR after optimizer if requested
    if (Intel::OpenCL::Utils::getEnvVar(Env,
                                        "CL_CONFIG_DUMP_IR_AFTER_OPTIMIZER") &&
        Intel::OpenCL::Utils::ConfigFile::ConvertStringToType<bool>(Env))
      DumpModuleStats(pProgram, pModule, /*isEqualizerStats = */ false);

    PostOptimizationProcessing(pProgram);

    // ObjectCodeCache structure will be filled by a callback after JIT
    // happens.
    std::unique_ptr<ObjectCodeCache> objCache(
        new ObjectCodeCache(nullptr, nullptr, 0));

    if (!(pOptions && pOptions->GetBooleanValue(
                          CL_DEV_BACKEND_OPTION_STOP_BEFORE_JIT, false))) {
      JitProcessing(pProgram, pOptions, std::move(targetMachine),
                    objCache.get(), buildResult);

      // LLVMBackend::GetInstance()->m_logger->Log(Logger::DEBUG_LEVEL,
      // L"Start iterating over kernels");
      std::unique_ptr<KernelSet> pKernels =
          CreateKernels(pProgram, pBuildOpts, buildResult);
      // update kernels with RuntimeService
      Utils::UpdateKernelsWithRuntimeService(lRuntimeService, pKernels.get());

      pProgram->SetKernelSet(std::move(pKernels));
    }

    BuildProgramCachedExecutable(objCache.get(), pProgram);
  } catch (Exceptions::DeviceBackendExceptionBase &e) {
    // if an exception is caught, the LLVM error handler should be removed
    // safely on windows, the LLVM error handler will not be removed
    // automatically and will cause assertion failure in debug mode
    remove_fatal_error_handler();

    buildResult.LogS() << e.what() << "\n";
    buildResult.SetBuildResult(e.GetErrorCode());
    pProgram->SetBuildLog(buildResult.GetBuildLog());
    throw e;
  }

  pProgram->SetBuildLog(buildResult.GetBuildLog());
  return buildResult.GetBuildResult();
}

KernelJITProperties *
ProgramBuilder::CreateKernelJITProperties(unsigned int vectorSize) const {
  KernelJITProperties *pProps = m_pBackendFactory->CreateKernelJITProperties();
  pProps->SetUseVTune(m_config->GetUseVTune());
  pProps->SetVectorSize(vectorSize);
  return pProps;
}

KernelProperties *ProgramBuilder::CreateKernelProperties(
    const Program * /*pProgram*/, Function *func,
    const CompilerBuildOptions &buildOptions,
    const ProgramBuildResult & /*buildResult*/) const {
  Module *pModule = func->getParent();

  auto kmd = KernelMetadataAPI(func);

  std::stringstream kernelAttributes;

  unsigned int kernelForcedVecLength = 0;

  // WG size is set based on attributes passed via metadata.
  unsigned int optWGSize = CPU_DEFAULT_WG_SIZE; // TODO: to be checked
  size_t hintWGSize[MAX_WORK_DIM] = {0, 0, 0};
  if (kmd.WorkGroupSizeHint.hasValue()) {
    // TODO: SExt <=> ZExt
    hintWGSize[0] = kmd.WorkGroupSizeHint.getXDim();
    size_t numDims = kmd.WorkGroupSizeHint.getNumOfDims();
    hintWGSize[1] = numDims > 1 ? kmd.WorkGroupSizeHint.getYDim() : 1;
    hintWGSize[2] = numDims > 2 ? kmd.WorkGroupSizeHint.getZDim() : 1;

    if (hintWGSize[0]) {
      optWGSize = hintWGSize[0];
      for (size_t i = 1; i < MAX_WORK_DIM; ++i) {
        if (hintWGSize[i]) {
          optWGSize *= hintWGSize[i];
        }
      }
    }

    kernelAttributes << "work_group_size_hint(" << hintWGSize[0] << ","
                     << hintWGSize[1] << "," << hintWGSize[2] << ") ";
  }

  size_t reqdWGSize[MAX_WORK_DIM] = {0, 0, 0};
  // TODO: SExt <=> ZExt
  if (kmd.ReqdWorkGroupSize.hasValue()) {
    reqdWGSize[0] = kmd.ReqdWorkGroupSize.getXDim();
    reqdWGSize[1] = kmd.ReqdWorkGroupSize.getYDim();
    reqdWGSize[2] = kmd.ReqdWorkGroupSize.getZDim();

    if (reqdWGSize[0]) {
      optWGSize = reqdWGSize[0];
      for (size_t i = 1; i < MAX_WORK_DIM; ++i) {
        if (reqdWGSize[i]) {
          optWGSize *= reqdWGSize[i];
        }
      }
    }

    kernelAttributes << "reqd_work_group_size(" << reqdWGSize[0] << ","
                     << reqdWGSize[1] << "," << reqdWGSize[2] << ") ";
  }

  size_t reqdNumSG = 0;
  if (kmd.ReqdNumSubGroups.hasValue()) {
    reqdNumSG = kmd.ReqdNumSubGroups.get();
    kernelAttributes << "required_num_sub_groups(" << reqdNumSG << ") ";
  }

  bool isAutorun = false;
  if (kmd.Autorun.hasValue()) {
    isAutorun = true;
    kernelAttributes << "autorun ";
  }

  bool isTask = false;
  if (kmd.MaxGlobalWorkDim.hasValue()) {
    int MaxGlobalWorkDim = kmd.MaxGlobalWorkDim.get();
    kernelAttributes << "max_global_work_dim(" << MaxGlobalWorkDim << ") ";
    if (0 == MaxGlobalWorkDim) {
      isTask = true;
    }
  }

  bool canUseGlobalWorkOffset = true;
  if (kmd.CanUseGlobalWorkOffset.hasValue()) {
    canUseGlobalWorkOffset = kmd.CanUseGlobalWorkOffset.get();
    kernelAttributes << "uses_global_Work_offset(" << canUseGlobalWorkOffset
                     << ") ";
  }

  if (kmd.hasVecLength()) {
    kernelForcedVecLength = kmd.getVecLength();
    kernelAttributes << "intel_vec_len_hint(" << kernelForcedVecLength << ") ";
  }

  if (kmd.VecTypeHint.hasValue()) {
    Type *VTHTy = kmd.VecTypeHint.getType();

    int vecSize = 1;

    if (VTHTy->isVectorTy()) {
      vecSize = cast<FixedVectorType>(VTHTy)->getNumElements();
      VTHTy = cast<VectorType>(VTHTy)->getElementType();
    }

    kernelAttributes << "vec_type_hint(";

    if (VTHTy->isIntegerTy() && kmd.VecTypeHint.getSign() == 0)
      kernelAttributes << "u";

    if (VTHTy->isFloatTy()) {
      kernelAttributes << "float";
    } else if (VTHTy->isDoubleTy()) {
      kernelAttributes << "double";
    } else if (VTHTy->isIntegerTy(8)) {
      kernelAttributes << "char";
    } else if (VTHTy->isIntegerTy(16)) {
      kernelAttributes << "short";
    } else if (VTHTy->isIntegerTy(32)) {
      kernelAttributes << "int";
    } else if (VTHTy->isIntegerTy(64)) {
      kernelAttributes << "long";
    } else if (VTHTy->isHalfTy()) {
      kernelAttributes << "half";
    }

    if (vecSize > 1)
      kernelAttributes << vecSize;
    kernelAttributes << ") ";
  }

  auto skimd = KernelInternalMetadataAPI(func);

  // Since work-group autorun kernels should be launched with
  // global size = (2^32, 2^32, 2^32) and local size = reqd_work_group_size, we
  // need to serialize work-groups even if there is no fpga pipes/channels to
  // avoid grabbing all of available threads by work-groups of autorun kernels
  //
  // Work-groups are serialized in the following cases:
  //   1. CL_CONFIG_CPU_TBB_NUM_WORKERS is 1. This is implemented in
  //      UpdateKernelsWithRuntimeConfig.
  //   2. Kernel has FPGA pipe/channel
  //   3. Kernel is FPGA autorun.
  bool needSerializeWGs =
      (skimd.UseFPGAPipes.hasValue() && skimd.UseFPGAPipes.get()) || isAutorun;

  const bool HasNoBarrierPath = skimd.NoBarrierPath.get();
  const bool HasMatrixCall =
      skimd.HasMatrixCall.hasValue() && skimd.HasMatrixCall.get();
  const unsigned int localBufferSize =
      skimd.LocalBufferSize.hasValue() ? skimd.LocalBufferSize.get() : 0;
  const bool hasGlobalSync =
      skimd.KernelHasGlobalSync.hasValue() && skimd.KernelHasGlobalSync.get();
  const size_t scalarExecutionLength = skimd.KernelExecutionLength.hasValue()
                                           ? skimd.KernelExecutionLength.get()
                                           : 0;
  const unsigned int scalarBufferStride =
      skimd.BarrierBufferSize.hasValue() ? skimd.BarrierBufferSize.get() : 0;
  unsigned int privateMemorySize =
      skimd.PrivateMemorySize.hasValue() ? skimd.PrivateMemorySize.get() : 0;
  size_t VF =
      skimd.VectorizedWidth.hasValue() ? skimd.VectorizedWidth.get() : 1;

  size_t vectorExecutionLength = 0;
  unsigned int vectorBufferStride = 0;
  // Need to check if Vectorized Kernel Value exists, it is not guaranteed that
  // Vectorized is running in all scenarios.
  if (skimd.VectorizedKernel.hasValue() &&
      skimd.VectorizedKernel.get() != nullptr) {
    auto vkimd = KernelInternalMetadataAPI(skimd.VectorizedKernel.get());
    vectorExecutionLength = vkimd.KernelExecutionLength.get();
    vectorBufferStride = vkimd.BarrierBufferSize.get();
    privateMemorySize = std::max<unsigned int>(privateMemorySize,
                                               vkimd.PrivateMemorySize.get());
    VF = vkimd.VectorizedWidth.get();
  }

  // Execution length contains the max size between
  // the length of scalar and the length of vectorized versions.
  const size_t executionLength =
      std::max(scalarExecutionLength, vectorExecutionLength);
  // Private memory size contains the max size between
  // the needed size for scalar and needed size for vectorized versions.
  unsigned int barrierBufferSize =
      std::max<unsigned int>(scalarBufferStride, vectorBufferStride);
  // Aligh barrier buffer and private memory size
  barrierBufferSize = ADJUST_SIZE_TO_MAXIMUM_ALIGN(barrierBufferSize);
  privateMemorySize = ADJUST_SIZE_TO_MAXIMUM_ALIGN(privateMemorySize);

  KernelProperties *pProps = new KernelProperties();

  // Kernel should keep size of pointer specified inside module
  // to allow cross-platform compilation
  unsigned int ptrSizeInBytes = pModule->getDataLayout().getPointerSize(0);
  pProps->SetPointerSize(ptrSizeInBytes);

  pProps->SetHasDebugInfo(buildOptions.GetDebugInfoFlag());
  pProps->SetOptWGSize(optWGSize);
  pProps->SetReqdWGSize(reqdWGSize);
  pProps->SetHintWGSize(hintWGSize);
  pProps->SetReqdNumSG(reqdNumSG);
  pProps->SetTotalImplSize(localBufferSize);
  pProps->SetHasNoBarrierPath(HasNoBarrierPath);
  pProps->SetHasMatrixCall(HasMatrixCall);
  pProps->SetHasGlobalSync(hasGlobalSync);
  pProps->SetKernelExecutionLength(executionLength);
  pProps->SetVectorizationWidth(VF);
  pProps->SetIsAutorun(isAutorun);
  pProps->SetNeedSerializeWGs(needSerializeWGs);
  pProps->SetIsTask(isTask);
  pProps->SetCanUseGlobalWorkOffset(canUseGlobalWorkOffset);
  pProps->SetRequiredSubGroupSize(kernelForcedVecLength);
  auto kernelAttributesStr = kernelAttributes.str();
  // Remove space at the end
  if (!kernelAttributesStr.empty())
    kernelAttributesStr.pop_back();
  pProps->SetKernelAttributes(kernelAttributesStr);
  pProps->SetDAZ(buildOptions.GetDenormalsZero());
  pProps->SetCpuId(
      GetCompiler()->GetCpuId()->GetCPUIdForKernelPropertiesSerialize());
  if (HasNoBarrierPath && VF > 1)
    pProps->EnableVectorizedWithTail();

  pProps->SetBarrierBufferSize(barrierBufferSize);
  // workaround:
  //   GetPrivateMemorySize returns the min. required private memory
  //   size per work-item even if there are no work-group level built-ins.
  pProps->SetPrivateMemorySize(privateMemorySize);

  // set isBlock property
  pProps->SetIsBlock(CompilationUtils::isBlockInvocationKernel(func));

  //
  pProps->SetTargetDevice(m_targetDevice);

  pProps->SetDeviceMaxWGSize(m_config->GetDeviceMaxWGSize());

  pProps->SetIsNonUniformWGSizeSupported(
      !func->getFnAttribute("uniform-work-group-size").getValueAsBool());

  // set can unite WG and vectorization dimention
  pProps->SetCanUniteWG(skimd.CanUniteWorkgroups.hasValue() &&
                        skimd.CanUniteWorkgroups.get());
  pProps->SetVerctorizeOnDimention(skimd.VectorizationDimension.hasValue()
                                       ? skimd.VectorizationDimension.get()
                                       : 0);

  if (skimd.SubGroupConstructionMode.hasValue())
    pProps->SetSubGroupConstructionMode(skimd.SubGroupConstructionMode.get());

  return pProps;
}

cl_dev_err_code ProgramBuilder::FinalizeProgram(Program *Prog) {
  // Apply runtime config.
  Utils::UpdateKernelsWithRuntimeConfig(m_config.get(), Prog->GetKernelSet());

  return Prog->Finalize();
}

void ProgramBuilder::BuildLibraryProgram(Program *Prog,
                                         std::string &KernelNames) {
  assert(Prog && "Program parameter must not be nullptr");
  ProgramBuildResult buildResult;
  try {
    Compiler *Cmplr = GetCompiler();

    char ModuleName[MAX_PATH];
    Intel::OpenCL::Utils::GetModuleDirectory(ModuleName, MAX_PATH);
    std::string ModuleDir(ModuleName);
    std::string CPUPrefix = Cmplr->GetCpuId()->GetCPUPrefix();

#ifdef INTEL_CUSTOMIZATION
    // The code below may cause ip leak, so we exclude it in our product release
    // build.
#ifndef INTEL_PRODUCT_RELEASE
    // Make sure that unit test binary can correctly find the ocl builtin libs.
    SmallString<128> TempPah(ModuleDir);
    sys::path::append(TempPah, std::string("clbltfn") + CPUPrefix + ".rtl");
    if (!sys::fs::exists(TempPah))
      ModuleDir = DEFAULT_OCL_LIBRARY_DIR;
#endif // INTEL_PRODUCT_RELEASE
#endif // INTEL_CUSTOMIZATION

    SmallString<128> BaseName(ModuleDir);
    sys::path::append(BaseName, OCL_LIBRARY_TARGET_NAME);
    std::string RtlFilePath = std::string(BaseName) + OCL_OUTPUT_EXTENSION;
    std::string ObjectFilePath =
        std::string(BaseName) + CPUPrefix + OCL_PRECOMPILED_OUTPUT_EXTENSION;
    assert(sys::fs::exists(RtlFilePath) && "Library rtl file is not found");
    assert(sys::fs::exists(ObjectFilePath) &&
           "Library object file is not found");

    // Load module file.
    ErrorOr<std::unique_ptr<MemoryBuffer>> rtlBufferOrErr =
        MemoryBuffer::getFile(RtlFilePath);
    if (!rtlBufferOrErr)
      throw Exceptions::DeviceBackendExceptionBase(
          std::string("Failed to load the library kernel rtl file"));
    // Load object file.
    ErrorOr<std::unique_ptr<MemoryBuffer>> objBufferOrErr =
        MemoryBuffer::getFile(ObjectFilePath);
    if (!objBufferOrErr)
      throw Exceptions::DeviceBackendExceptionBase(
          std::string("Failed to load the library kernel object file"));

    // Create JIT and add object buffer to JIT.
    std::unique_ptr<Module> M =
        Cmplr->ParseModuleIR(rtlBufferOrErr.get().get());
    auto LLJIT = Cmplr->CreateLLJIT(M.get(), nullptr, nullptr);
    if (auto Err = LLJIT->addObjectFile(std::move(objBufferOrErr.get()))) {
      logAllUnhandledErrors(std::move(Err), errs());
      throw Exceptions::CompilerException("Failed to addObjectFile");
    }
    Prog->SetLLJIT(std::move(LLJIT));

    // Set IR.
    BitCodeContainer *BCC =
        new BitCodeContainer(std::move(rtlBufferOrErr.get()));
    Prog->SetBitCodeContainer(BCC);
    Prog->SetModule(std::move(M));

    // Init runtime service.
    RuntimeServiceSharedPtr RTService =
        RuntimeServiceSharedPtr(new RuntimeServiceImpl);
    Prog->SetRuntimeService(RTService);

    std::unique_ptr<KernelSet> Kernels =
        CreateKernels(Prog, nullptr, buildResult);

    // Get kernel names.
    size_t NumKernels = Kernels->GetCount();
    assert(NumKernels == 3 && "Invalid number of library kernels");
    std::ostringstream O;
    for (size_t i = 0; i < NumKernels; ++i) {
      O << Kernels->GetKernel(i)->GetKernelName();
      if (i < (NumKernels - 1))
        O << ";";
    }
    KernelNames = O.str();

    // Update kernels with RuntimeService.
    Utils::UpdateKernelsWithRuntimeService(RTService, Kernels.get());

    Prog->SetKernelSet(std::move(Kernels));

    Prog->SetBuildLog(buildResult.GetBuildLog());

  } catch (Exceptions::DeviceBackendExceptionBase &e) {
    // if an exception is caught, the LLVM error handler should be removed
    // safely on windows, the LLVM error handler will not be removed
    // automatically and will cause assertion failure in debug mode
    remove_fatal_error_handler();

    buildResult.LogS() << e.what() << "\n";
    throw Exceptions::DeviceBackendExceptionBase{buildResult.GetBuildLog(),
                                                 e.GetErrorCode()};
  }
}
} // namespace DeviceBackend
} // namespace OpenCL
} // namespace Intel
