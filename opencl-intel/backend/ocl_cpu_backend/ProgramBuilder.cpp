/*****************************************************************************\

Copyright (c) Intel Corporation (2010-2017).

    INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
    LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
    ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
    PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
    DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
    PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
    including liability for infringement of any proprietary rights, relating to
    use of the code. No license, express or implied, by estoppels or otherwise,
    to any intellectual property rights is granted herein.

File Name:  ProgramBuilder.cpp

\*****************************************************************************/
#define NOMINMAX

#include "cl_types.h"
#include "cpu_dev_limits.h"
#include "Compiler.h"
#include "ProgramBuilder.h"
#include "Optimizer.h"
#include "VecConfig.h"
#include "Program.h"
#include "Kernel.h"
#include "KernelProperties.h"
#include "CPUDetect.h"
#include "BuiltinModules.h"
#include "exceptions.h"
#include "BuiltinModuleManager.h"
#include "MetadataAPI.h"
#include "BitCodeContainer.h"
#include "CompilationUtils.h"
#include "cache_binary_handler.h"
#include "ObjectCodeContainer.h"
#include "ObjectCodeCache.h"
#include "OclTune.h"

#define DEBUG_TYPE "ProgramBuilder"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/IR/Argument.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/UnifyFunctionExitNodes.h"

#if defined (WIN32)
#include <windows.h>
#include <shellapi.h>
#include <codecvt>
#endif // WIN32

#include <algorithm>
#include <atomic>
#include <sstream>
#include <string>
#include <vector>

using std::string;
using namespace Intel::MetadataAPI;

namespace Intel { namespace OpenCL { namespace DeviceBackend {

static void BEFatalErrorHandler(void *user_data, const std::string& reason,
                                bool gen_crash_diag)
{
    errs() << "**Internal compiler error** " << reason << "\n" <<
              "Please report the issue on Intel OpenCL forum \n" <<
              "https://software.intel.com/en-us/forums/opencl for assistance. \n ";
    abort();
}

/*
 * Utility methods
 */
namespace Utils
{
static int getFileCount() {
  static std::atomic<unsigned> fileCount(0);
  fileCount++;

  return fileCount.load(std::memory_order_relaxed);
}

// Returns the memory buffer of the Program object bytecode
static llvm::MemoryBuffer *GetProgramMemoryBuffer(Program *pProgram) {
  const BitCodeContainer *pCodeContainer =
      static_cast<const BitCodeContainer *>(
          pProgram->GetProgramIRCodeContainer());
  return (llvm::MemoryBuffer *)pCodeContainer->GetMemoryBuffer();
}

/// @brief helper funtion to set RuntimeService in Kernel objects from KernelSet
void UpdateKernelsWithRuntimeService( const RuntimeServiceSharedPtr& rs, KernelSet * pKernels )
{
  for(unsigned cnt = 0; cnt < pKernels->GetCount(); ++cnt){
    Kernel * pK = pKernels->GetKernel(cnt);
    pK->SetRuntimeService(rs);
  }
}
} //namespace Utils

using namespace Intel::OpenCL::ELFUtils;

// checks if the given program has an object binary to be loaded from
static bool checkIfProgramHasCachedExecutable(Program *pProgram) {
  assert(pProgram && "pProgram is null");
  if (!pProgram->GetObjectCodeContainer())
    return false;

  const char *pObject =
      (const char *)pProgram->GetObjectCodeContainer()->GetCode();
  size_t objectSize = pProgram->GetObjectCodeContainer()->GetCodeSize();
  CacheBinaryReader reader(pObject, objectSize);
  return reader.IsCachedObject();
}

// Update the size of the variables in global adress space used by the program.
static void updateGlobalVariableTotalSize(Program *pProgram, Module *pModule) {
  auto globalSizeMetadata = ModuleInternalMetadataAPI(pModule).GlobalVariableTotalSize;
  // The info is missing only when we build image built-ins and we don't
  // care about the size of global variables in the program.
  if (!globalSizeMetadata.hasValue())
    return;
  pProgram->SetGlobalVariableTotalSize(globalSizeMetadata.get());
}

ProgramBuilder::ProgramBuilder(IAbstractBackendFactory* pBackendFactory, const ICompilerConfig& config):
    m_pBackendFactory(pBackendFactory),
    m_useVTune(config.GetUseVTune()),
    m_forcedPrivateMemorySize(config.GetForcedPrivateMemorySize()),
    m_statFileBaseName(config.GetStatFileBaseName())
{
    // prepare default base file name for stat file in the following cases:
    // stats are enabled but the user didn't set up the base file name
    // the user set up as base file name only a directory name, i.e. it ends
    // with \ or /
    // the default file name is the running executable name
    if ((m_statFileBaseName.empty() && intel::Statistic::isEnabled()) ||
        (!m_statFileBaseName.empty() &&
         llvm::sys::path::is_separator(*m_statFileBaseName.rbegin())))
    {
#if defined (WIN32)
        LPWSTR *cl;
        int numArgs;
        string nameStr, nameStr2;
        const char *name = nullptr;

        cl = CommandLineToArgvW(L"", &numArgs);
        if (nullptr != cl) {
            std::wstring wstr(*cl);
            nameStr = std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(wstr);
            name = nameStr.c_str();
        }

        // Free memory allocated for CommandLineToArgvW arguments.
        LocalFree(cl);

        // find the base name by searching for the last '/' in the name
        // Remove .exe from the name. To actually create a file name without
        // the extension, need to create it in a string and take it's c_str()
        if (name != nullptr) {
            nameStr2 = llvm::sys::path::stem(StringRef(name)).str();
            name = nameStr2.c_str();
        }

#else // WIN32
         const char *name = getenv("_");
        // find the base name by searching for the last '/' in the name
        if (name != nullptr)
            name = llvm::sys::path::filename(StringRef(name)).data();
#endif // WIN32
        // if still no meaningful name just use "Program" as module name
        if (name == nullptr || *name == 0)
            name = "Program";

        m_statFileBaseName += name;

        if (intel::Statistic::isEnabled())
          m_statWkldName = name;
    }
}

ProgramBuilder::~ProgramBuilder()
{
}

void ProgramBuilder::DumpModuleStats(llvm::Module* pModule)
{
    if (intel::Statistic::isEnabled() || !m_statFileBaseName.empty())
    {
        // use sequential number to distinguish dumped files
        std::stringstream fileNameBuilder;
        fileNameBuilder << (Utils::getFileCount());

        std::string fileName(m_statFileBaseName);
        fileName += fileNameBuilder.str();
        fileName += ".ll";

        // if stats are enabled dump module info
        if (intel::Statistic::isEnabled())
        {
          intel::Statistic::setModuleStatInfo(pModule,
              m_statWkldName.c_str(), // workload name
              (m_statWkldName + fileNameBuilder.str()).c_str() // module name
              );
        }
        // dump IR with stats
        std::error_code ec;
        raw_fd_ostream IRFD(fileName.c_str(), ec, llvm::sys::fs::F_RW);
        if (!ec)
          pModule->print(IRFD, 0);
        else
          throw Exceptions::CompilerException(ec.message());
    }
}

void ProgramBuilder::ParseProgram(Program* pProgram)
{
    try
    {
        assert(!checkIfProgramHasCachedExecutable(pProgram) && "Program must not be loaded from cache");
        pProgram->SetModule( GetCompiler()->ParseModuleIR( Utils::GetProgramMemoryBuffer(pProgram)));
    }
    catch(Exceptions::CompilerException& e)
    {
        throw Exceptions::DeviceBackendExceptionBase(e.what());
    }
}

cl_dev_err_code ProgramBuilder::BuildProgram(Program* pProgram, const ICLDevBackendOptions* pOptions)
{
    assert(pProgram && "Program parameter must not be nullptr");
    ProgramBuildResult buildResult;

    try
    {
        if(checkIfProgramHasCachedExecutable(pProgram))
        {
             std::string log = "Reload Program Binary Object.";
             ReloadProgramFromCachedExecutable(pProgram);
             pProgram->SetBuildLog(log);
             return CL_DEV_SUCCESS;
        }
        Compiler* pCompiler = GetCompiler();
        llvm::Module* pModule = (llvm::Module*)pProgram->GetModule();

        if(!pModule)
        {
            ParseProgram(pProgram);
            pModule = (llvm::Module*)pProgram->GetModule();
        }
        assert(pModule && "Module parsing has failed without exception. Strage");

        // Handle LLVM ERROR which can occured during build programm
        // Need to do it to eliminate RT hanging when clBuildProgramm failed
        llvm::ScopedFatalErrorHandler FatalErrorHandler(BEFatalErrorHandler, nullptr);

        pCompiler->BuildProgram( pModule, &buildResult);
        // ObjectCodeCache structure will be filled by a callback after JIT happens.
        std::unique_ptr<ObjectCodeCache> pObjectCodeCache(new ObjectCodeCache(nullptr, nullptr, 0));
        pCompiler->SetObjectCache(pObjectCodeCache.get());

        pProgram->SetExecutionEngine(pCompiler->GetExecutionEngine());
        pProgram->SetBuiltinModule(pCompiler->GetBuiltinModuleList());

        // init refcounted runtime service shared storage between program and kernels
        RuntimeServiceSharedPtr lRuntimeService =
                          RuntimeServiceSharedPtr(new RuntimeServiceImpl);
        // set runtime service for the program
        pProgram->SetRuntimeService(lRuntimeService);

        // Dump module stats just before lowering if requested
        DumpModuleStats(pModule);

        PostOptimizationProcessing(pProgram, pModule, pOptions);
        if (!(pOptions && pOptions->GetBooleanValue(CL_DEV_BACKEND_OPTION_STOP_BEFORE_JIT, false)))
        {
            //LLVMBackend::GetInstance()->m_logger->Log(Logger::DEBUG_LEVEL, L"Start iterating over kernels");
            KernelSet* pKernels = CreateKernels( pProgram,
                                                 pModule,
                                                 buildResult);
            // update kernels with RuntimeService
            Utils::UpdateKernelsWithRuntimeService( lRuntimeService, pKernels );

            pProgram->SetKernelSet( pKernels );
        }

        // call post build method
        PostBuildProgramStep( pProgram, pModule, pOptions );
        updateGlobalVariableTotalSize(pProgram, pModule);

        BuildProgramCachedExecutable(pObjectCodeCache.get(), pProgram);
    }
    catch( Exceptions::DeviceBackendExceptionBase& e )
    {
        buildResult.LogS() << e.what() << "\n";
        buildResult.SetBuildResult( e.GetErrorCode());
        pProgram->SetBuildLog( buildResult.GetBuildLog() );
        throw e;
    }

    pProgram->SetBuildLog( buildResult.GetBuildLog());
    return buildResult.GetBuildResult();
}

KernelJITProperties* ProgramBuilder::CreateKernelJITProperties( unsigned int vectorSize) const
{
    KernelJITProperties* pProps = m_pBackendFactory->CreateKernelJITProperties();
    pProps->SetUseVTune(m_useVTune);
    pProps->SetVectorSize(vectorSize);
    pProps->SetMaxPrivateMemorySize(m_forcedPrivateMemorySize);
    return pProps;
}

KernelProperties *ProgramBuilder::CreateKernelProperties(
    const Program *pProgram, Function *func,
    const ProgramBuildResult &buildResult) const {
  Module *pModule = func->getParent();

  auto kmd = KernelMetadataAPI(func);

  std::stringstream kernelAttributes;

  // WG size is set based on attributes passed via metadata.
  unsigned int optWGSize = 128; // TODO: to be checked
  size_t hintWGSize[MAX_WORK_DIM] = {0, 0, 0};
  if (kmd.WorkGroupSizeHint.hasValue()) {
    // TODO: SExt <=> ZExt
    hintWGSize[0] = kmd.WorkGroupSizeHint.getXDim();
    hintWGSize[1] = kmd.WorkGroupSizeHint.getYDim();
    hintWGSize[2] = kmd.WorkGroupSizeHint.getZDim();

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
    kernelAttributes << "autorun";
  }

  if (kmd.VecLenHint.hasValue()) {
    int32_t VecLen = kmd.VecLenHint.get();
    kernelAttributes << "intel_vec_len_hint(" << VecLen << ") ";
  }

  if (kmd.VecTypeHint.hasValue()) {
    Type *VTHTy = kmd.VecTypeHint.getType();

    int vecSize = 1;

    if (VTHTy->isVectorTy()) {
      vecSize = VTHTy->getVectorNumElements();
      VTHTy = VTHTy->getVectorElementType();
    }

    kernelAttributes << "vec_type_hint(";

    if (kmd.VecTypeHint.getSign() == 0)
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
    }

    if (vecSize > 1)
      kernelAttributes << vecSize;
    kernelAttributes << ") ";
  }

  auto skimd = KernelInternalMetadataAPI(func);

  bool needSerializeWGs =
      skimd.UseFPGAPipes.hasValue() && skimd.UseFPGAPipes.get();

  // Need to check if NoBarrierPath Value exists, it is not guaranteed that
  // KernelAnalysisPass is running in all scenarios.
  const bool HasNoBarrierPath =
      skimd.NoBarrierPath.hasValue() && skimd.NoBarrierPath.get();
  const unsigned int localBufferSize = skimd.LocalBufferSize.get();
  const bool hasBarrier = skimd.KernelHasBarrier.get();
  const bool hasGlobalSync = skimd.KernelHasGlobalSync.get();
  const size_t scalarExecutionLength = skimd.KernelExecutionLength.get();
  const unsigned int scalarBufferStride = skimd.BarrierBufferSize.get();
  unsigned int privateMemorySize = skimd.PrivateMemorySize.get();

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

  CompilerBuildOptions buildOptions(pModule);
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
  pProps->SetHasBarrier(hasBarrier);
  pProps->SetHasGlobalSync(hasGlobalSync);
  pProps->SetKernelExecutionLength(executionLength);
  pProps->SetIsAutorun(isAutorun);
  pProps->SetNeedSerializeWGs(needSerializeWGs);
  auto kernelAttributesStr = kernelAttributes.str();
  // Remove space at the end
  if (!kernelAttributesStr.empty())
    kernelAttributesStr.pop_back();
  pProps->SetKernelAttributes(kernelAttributesStr);
  pProps->SetDAZ(buildOptions.GetDenormalsZero());
  pProps->SetCpuId(GetCompiler()->GetCpuId());
  if (HasNoBarrierPath)
    pProps->EnableVectorizedWithTail();

  pProps->SetBarrierBufferSize(barrierBufferSize);
  // CSSD100016517 workaround:
  //   GetPrivateMemorySize returns the min. required private memory
  //   size per work-item even if there are no work-group level built-ins.
  pProps->SetPrivateMemorySize(privateMemorySize);
  pProps->SetMaxPrivateMemorySize(m_forcedPrivateMemorySize);

  // set isBlock property
  pProps->SetIsBlock(CompilationUtils::isBlockInvocationKernel(func));

  // OpenCL 2.0 related properties
  if (OclVersion::CL_VER_2_0 <=
          CompilationUtils::fetchCLVersionFromMetadata(*pModule) &&
      CompilationUtils::fetchCompilerOption(*pModule,
                                            "-cl-uniform-work-group-size")
          .empty()) {
    pProps->SetIsNonUniformWGSizeSupported(true);
  } else {
    pProps->SetIsNonUniformWGSizeSupported(false);
  }

  // set can unite WG and vectorization dimention
  pProps->SetCanUniteWG(skimd.CanUniteWorkgroups.get());
  pProps->SetVerctorizeOnDimention(skimd.VectorizationDimension.get());

  return pProps;
}
}}}
