// INTEL CONFIDENTIAL
//
// Copyright 2011-2018 Intel Corporation.
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

#include "OpenCLCPUBackendRunner.h"
#include "BackendOptions.h"
#include "BinaryDataReader.h"
#include "BinaryDataWriter.h"
#include "BlockLiteral.h"
#include "Buffer.h"
#include "BufferContainerList.h"
#include "DataVersion.h"
#include "ExplicitArgument.h"
#include "ExplicitBlockLiteralArgument.h"
#include "ExplicitGlobalMemArgument.h"
#include "OpenCLArgsBuffer.h"
#include "OpenCLBackendWrapper.h"
#include "OpenCLProgramConfiguration.h"
#include "OpenCLRunConfiguration.h"
#include "Performance.h"
#include "SATestException.h"
#include "XMLDataReader.h"
#include "XMLDataWriter.h"
#include "cpu_dev_limits.h"
#include "mem_utils.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include <algorithm>
#include <cassert>
#include <cstdio>
#include <string.h>
#include <vector>

// debug macro
#define DEBUG_TYPE "OpenCLCPUBackendRunner"

using namespace llvm;
using namespace Intel::OpenCL::DeviceBackend;

namespace Validation {

static int numberOfSpace = 4;

namespace Utils {
std::string GetDataFilePath(const std::string &fileName,
                            const std::string &baseDirectory);
}

extern void GenINT3();

class OpenCLExecutionContext {
public:
  OpenCLExecutionContext(const ICLDevBackendKernel_ *pKernel,
                         const BERunOptions *config,
                         cl_work_description_type workInfo,
                         uint8_t *pArgsBuffer, size_t argsBufferSize)
      : // allocate buffer for kernel's arguments
        m_pArgumentBuffer(
            (char *)align_malloc(pKernel->GetExplicitArgumentBufferSize() +
                                     sizeof(UniformKernelArgs),
                                 CPU_DEV_MAXIMUM_ALIGN)),
        m_uiVectorWidth(
            pKernel->GetKernelProporties()->GetMinGroupSizeFactorial()),
        m_pBlockLiteral(NULL) // pointer to BlockLiteral
  {
    // init kernel parameters
    InitParams(pKernel, (char *)pArgsBuffer, workInfo);
    m_tExecState.MXCSRstate = 0;
  }

  ~OpenCLExecutionContext() {
    if (m_pBlockLiteral)
      BlockLiteral::FreeMem(m_pBlockLiteral);
  }

  void InitParams(const ICLDevBackendKernel_ *pKernel, char *pArgsBuffer,
                  cl_work_description_type workInfo) {
    char *pArgValueDest = m_pArgumentBuffer.get();

    const int kernelParamCnt = pKernel->GetKernelParamsCount();

    for (int i = 0; i < kernelParamCnt; i++) {
      const KernelArgument arg = pKernel->GetKernelParams()[i];

      std::unique_ptr<FunctionArgument> pArg;

      switch (arg.Ty) {
      case KRNL_ARG_PTR_GLOBAL:
      case KRNL_ARG_PTR_CONST:
      case KRNL_ARG_PTR_IMG_2D:
      case KRNL_ARG_PTR_IMG_2D_DEPTH:
      case KRNL_ARG_PTR_IMG_3D:
      case KRNL_ARG_PTR_IMG_2D_ARR:
      case KRNL_ARG_PTR_IMG_2D_ARR_DEPTH:
      case KRNL_ARG_PTR_IMG_1D:
      case KRNL_ARG_PTR_IMG_1D_ARR:
      case KRNL_ARG_PTR_IMG_1D_BUF:
        pArg.reset(new ExplicitGlobalMemArgument(pArgValueDest, arg));
        break;
      case KRNL_ARG_PTR_BLOCK_LITERAL: {

        assert(i == 0 && "Block literal is not 0th argument in kernel");
        // pArgValueSrc - offset value
        // offset value is offset in bytes from the beginning of pArgsBuffer
        // arguments buffer to memory location where BlockLiteral is stored
        // offset value is of unsigned(32bit) type

        const size_t offs = (size_t) * (unsigned *)(pArgsBuffer);
        // deserialize in source memory BlockLiteral.
        // Actually it updates BlockDesc ptr field in BlockLiteral
        BlockLiteral *pBL =
            BlockLiteral::DeserializeInBuffer(pArgsBuffer + offs);

        // clone BlockLiteral to this Binary object. assume ownership
        assert(m_pBlockLiteral == NULL && "m_pBlockLiteral should be NULL");
        m_pBlockLiteral = BlockLiteral::Clone(pBL);

        // create special type of argument for passing BlockLiteral
        pArg.reset(new ExplicitBlockLiteralArgument(pArgValueDest, arg,
                                                    m_pBlockLiteral));
        break;
      }
      default:
        switch (arg.Ty) {
        case KRNL_ARG_PTR_LOCAL:
        case KRNL_ARG_INT:
        case KRNL_ARG_UINT:
        case KRNL_ARG_FLOAT:
        case KRNL_ARG_DOUBLE:
        case KRNL_ARG_VECTOR:
        case KRNL_ARG_VECTOR_BY_REF:
        case KRNL_ARG_SAMPLER:
        case KRNL_ARG_COMPOSITE:
          break;
        default:
          assert(false && "Unknown kind of argument");
        }
        pArg.reset(new ExplicitArgument(pArgValueDest, arg));
      }

      pArg->setValue(pArgsBuffer);

      // Advance the src buffer according to argument's size (the sec buffer is
      // packed)
      pArgsBuffer += pArg->getSize();
      // Advance the dest buffer according to argument's size and alignment
      pArgValueDest += pArg->getAlignedSize();
    }

    UniformKernelArgs *pKernelArgs = reinterpret_cast<UniformKernelArgs *>(
        m_pArgumentBuffer.get() + pKernel->GetExplicitArgumentBufferSize());

    size_t sizetMaxWorkDim = sizeof(size_t) * MAX_WORK_DIM;

    memcpy(pKernelArgs->GlobalOffset, workInfo.globalWorkOffset,
           sizetMaxWorkDim); // Filled by the runtime

    memcpy(pKernelArgs->UserGlobalSize, workInfo.globalWorkSize,
           sizetMaxWorkDim); // Filled by the runtime

    for (size_t dim = 0; dim < workInfo.workDimension; ++dim) {
      pKernelArgs->UserLocalSize[UNIFORM_WG_SIZE_INDEX][dim] =
          workInfo.localWorkSize[dim];
      // local size may be 0
      size_t nonUniWGSize =
          workInfo.localWorkSize[dim] == 0
              ? 0
              : workInfo.globalWorkSize[dim] % workInfo.localWorkSize[dim];
      // if the remainder is 0 set non-unifrom size to uniform value
      pKernelArgs->UserLocalSize[NONUNIFORM_WG_SIZE_INDEX][dim] =
          nonUniWGSize == 0 ? workInfo.localWorkSize[dim] : nonUniWGSize;
    }

    pKernelArgs->MinWorkGroupNum =
        size_t(workInfo.minWorkGroupNum); // Filled by the runtime, Required by
                                          // the heuristic

    pKernelArgs->UniformJITEntryPoint = NULL;    // Filled by the BE
    pKernelArgs->NonUniformJITEntryPoint = NULL; // Filled by the BE

    memset(pKernelArgs->UserWGCount, 0,
           sizetMaxWorkDim); // Updated by the BE, based on GLOBAL/LOCAL

    pKernelArgs->WorkDim = workInfo.workDimension; // Filled by the runtime

    m_pKernelRunner = pKernel->GetKernelRunner();

    // the second and third parameters are not used.
    m_pKernelRunner->PrepareKernelArguments(
        reinterpret_cast<void *>(m_pArgumentBuffer.get()), 0, 0, 1,
        /*calculateWGSize*/ true);

    // local group size calculated by PrepareKernelArguments (using heuristic)
    memcpy(m_LocalSize, pKernelArgs->UserLocalSize[UNIFORM_WG_SIZE_INDEX],
           sizetMaxWorkDim);
  }

  // Returns the actual number of Work Items handled by each executable instance
  const size_t *GetWorkGroupSize() const { return m_LocalSize; }

  unsigned int GetVectorSize() { return m_uiVectorWidth; }

  void ExecuteWorkGroup(size_t x, size_t y, size_t z) {

    size_t groupId[MAX_WORK_DIM] = {x, y, z};

    cl_dev_err_code rc = m_pKernelRunner->PrepareThreadState(m_tExecState);
    if (CL_DEV_FAILED(rc)) {
      throw Exception::TestRunnerException("PrepareThread failed\n");
    }

    m_sample.Start();

    LLVM_DEBUG(dbgs() << "Starting execution of the " << x << ", " << y << ", "
                      << z << " group.\n");

    cl_dev_err_code ret = m_pKernelRunner->RunGroup(
        reinterpret_cast<const void *>(m_pArgumentBuffer.get()), groupId,
        (void *)1 /* doesn't have RT handle here, workaround for an assert */);

    if (CL_DEV_FAILED(ret)) {
      throw Exception::TestRunnerException("Execution failed.\n");
    }

    LLVM_DEBUG(dbgs() << "Finished execution of the " << x << ", " << y << ", "
                      << z << " group.\n");

    m_sample.Stop();

    rc = m_pKernelRunner->RestoreThreadState(m_tExecState);
    if (CL_DEV_FAILED(rc)) {
      throw Exception::TestRunnerException("RestoreThreadState failed\n");
    }
  }

  void ResetSampling() { m_sample = Sample(); }

  Sample GetSampling() { return m_sample; }

private:
  const ICLDevBackendKernelRunner *m_pKernelRunner;
  ICLDevBackendKernelRunner::ICLDevExecutionState m_tExecState;

  auto_ptr_aligned m_pArgumentBuffer;
  Sample m_sample;
  unsigned int m_uiVectorWidth;
  BlockLiteral *m_pBlockLiteral;

  // work group size
  size_t m_LocalSize[MAX_WORK_DIM];
};

OpenCLCPUBackendRunner::OpenCLCPUBackendRunner(const BERunOptions &runConfig)
    : OpenCLBackendRunner(runConfig) {}

OpenCLCPUBackendRunner::~OpenCLCPUBackendRunner() {}

void PrintImage(const ImageDesc *pImageDesc, int i) {
  std::string indent(numberOfSpace, ' ');
  ImageTypeValWrapper type_val = pImageDesc->GetImageTypeDesc();
  ImageSizeDesc sizeDesc = pImageDesc->GetSizesDesc();
  std::size_t dimention = type_val.GetDimentionCount();
  std::cout << "Argument " << i << " (Image)" << std::endl;
  std::cout << indent << "Image type      : " << type_val.ToString()
            << std::endl;
  std::cout << indent << "Image size(B)   : " << pImageDesc->GetSizeInBytes()
            << std::endl;
  std::cout << indent << "Element size(B) : " << pImageDesc->GetElementSize()
            << std::endl;
  std::cout << indent << "Channel type    : " << pImageDesc->DataTypeToString()
            << std::endl;
  std::cout << indent << "Channel order   : " << pImageDesc->OrderToString()
            << std::endl;
  std::cout << indent << "Width           : " << sizeDesc.width << std::endl;
  if (1 < dimention) {
    std::cout << indent << "Height          : " << sizeDesc.height << std::endl;
  }
  if (2 < dimention) {
    std::cout << indent << "Depth           : " << sizeDesc.depth << std::endl;
  }
  std::cout << indent << "Row             : " << sizeDesc.row << std::endl;
  std::cout << indent << "Slice           : " << sizeDesc.slice << std::endl;
}

void PrintVector(TypeDesc &desc, std::size_t depth) {
  std::string indent(depth * numberOfSpace, ' ');
  TypeDesc sub_desc = desc.GetSubTypeDesc(0);
  std::cout << indent << "    VectorType      : " << sub_desc.TypeToString()
            << " x " << desc.GetSizeInBytes() / sub_desc.GetSizeInBytes()
            << std::endl;
}

void PrintStruct(TypeDesc &desc, std::size_t depth) {
  depth++;
  std::string indent(depth * numberOfSpace, ' ');
  std::size_t num_of_sub_types = desc.GetNumOfSubTypes();
  std::cout << indent << "Number of subtypes : " << num_of_sub_types
            << std::endl;
  std::cout << indent << "{" << std::endl;
  for (std::size_t i = 0; i < num_of_sub_types; ++i) {
    TypeDesc sub_desc = desc.GetSubTypeDesc(i);
    std::cout << indent << "    Type            : " << sub_desc.TypeToString()
              << std::endl;
    std::cout << indent << "    Element size(B) : " << sub_desc.GetSizeInBytes()
              << std::endl;
    std::cout << indent
              << "    Offset          : " << sub_desc.GetOffsetInStruct()
              << std::endl;
    if (sub_desc.IsStruct()) {
      PrintStruct(sub_desc, depth);
    } else if (sub_desc.IsVector()) {
      PrintVector(sub_desc, depth);
    }
    std::cout << std::endl;
  }
  std::cout << indent << "}" << std::endl;
}

void PrintBuffer(const BufferDesc *pBufferDesc, std::size_t i) {
  std::string indent(numberOfSpace, ' ');
  TypeDesc type_desc = pBufferDesc->GetElementDescription();
  std::cout << "Argument " << i << " (Buffer)" << std::endl;
  std::cout << indent << "Length          : " << pBufferDesc->NumOfElements()
            << std::endl;
  std::cout << indent << "Type            : " << type_desc.TypeToString()
            << std::endl;
  std::cout << indent << "Element size(B) : " << type_desc.GetSizeInBytes()
            << std::endl;
  if (type_desc.IsStruct()) {
    PrintStruct(type_desc, 0);
  } else if (type_desc.IsVector()) {
    PrintVector(type_desc, 0);
  }
}

void PrintMeta(BufferContainerList &input) {
  std::size_t containerCount = input.GetBufferContainerCount();
  for (std::size_t j = 0; j < containerCount; ++j) {
    IBufferContainer *container = input.GetBufferContainer(j);
    std::size_t objectCount = container->GetMemoryObjectCount();
    std::cout << "Number of arguments: " << objectCount << std::endl
              << std::endl;

    for (std::size_t i = 0; i < objectCount; ++i) {
      IMemoryObject *object = container->GetMemoryObject(i);
      const IMemoryObjectDesc *desc = object->GetMemoryObjectDesc();
      if (BufferDesc::GetBufferDescName() == desc->GetName()) // Buffer
      {
        const BufferDesc *buffer = reinterpret_cast<const BufferDesc *>(desc);
        PrintBuffer(buffer, i);
      } else if (ImageDesc::GetImageDescName() == desc->GetName()) // Image
      {
        const ImageDesc *pImageDesc = reinterpret_cast<const ImageDesc *>(desc);
        PrintImage(pImageDesc, i);
      } else // Anything else
        llvm_unreachable("Argument type don't known.\n");

      std::cout << std::endl;
    }
  }
}

void OpenCLCPUBackendRunner::Run(IRunResult *runResult, const IProgram *program,
                                 const IProgramConfiguration *programConfig,
                                 const IRunComponentConfiguration *runConfig) {
  assert((program != NULL) && "Program is not initialized\n");
  assert((programConfig != NULL) &&
         "Program Configuration is not initialized\n");
  assert((runConfig != NULL) && "Run Configuration is not initialized\n");
  assert((runResult != NULL) && "Run Result is not initialized\n");

  const OpenCLProgramConfiguration *pOCLProgramConfig =
      static_cast<const OpenCLProgramConfiguration *>(programConfig);
  const BERunOptions *pOCLRunConfig =
      static_cast<const BERunOptions *>(runConfig);
  const OpenCLProgram *pOCLProgram =
      static_cast<const OpenCLProgram *>(program);

  std::unique_ptr<CPUBackendOptions> options;
  if (pOCLRunConfig->GetValue<bool>(RC_BR_USE_SDE, false)) {
    options.reset(new SDEBackendOptions());
  } else {
    options.reset(new CPUBackendOptions());
  }

  options->InitFromRunConfiguration(*pOCLRunConfig);

  ICLDevBackendExecutionServicePtr spExecutionService(NULL);
  ICLDevBackendCompileServicePtr spCompileService(NULL);
  ICLDevBackendImageServicePtr spImageService(NULL);

  LLVM_DEBUG(dbgs() << "Get execution service started.\n");
  cl_dev_err_code ret = m_pServiceFactory->GetExecutionService(
      options.get(), spExecutionService.getOutPtr());
  if (CL_DEV_FAILED(ret)) {
    if (CL_DEV_NOT_SUPPORTED == ret)
      throw Exception::TestRunnerException(
          "Can't create execution service for current device. Try build "
          "back-end with SDE support.\n");
    throw Exception::TestRunnerException("Create execution service failed\n");
  }
  LLVM_DEBUG(dbgs() << "Get execution service finished.\n");

  options->InitTargetDescriptionSession(spExecutionService.get());

  LLVM_DEBUG(dbgs() << "Get compilation service started.\n");
  ret = m_pServiceFactory->GetCompilationService(options.get(),
                                                 spCompileService.getOutPtr());
  if (CL_DEV_FAILED(ret)) {
    if (ret == CL_DEV_INVALID_OPERATION_MODE) {
      throw Exception::TestRunnerException("Invalid CPU architecture was set.",
                                           VALIDATION_INVALID_OPERATION_MODE);
    }
    throw Exception::TestRunnerException("Create compilation service failed\n");
  }
  LLVM_DEBUG(dbgs() << "Get compilation service finished.\n");

  LLVM_DEBUG(dbgs() << "Get image service started.\n");
  ret = m_pServiceFactory->GetImageService(options.get(),
                                           spImageService.getOutPtr());
  if (CL_DEV_FAILED(ret)) {
    throw Exception::TestRunnerException("Create image service failed\n");
  }
  LLVM_DEBUG(dbgs() << "Get image service finished.\n");

  {
    //
    // Program need to be released before the compilation service - thus inner
    // scope is necessary
    //
    ProgramHolder programHolder(spCompileService.get());

    for (uint32_t i = 0;
         i < pOCLRunConfig->GetValue<uint32_t>(RC_BR_BUILD_ITERATIONS_COUNT, 1);
         ++i) {
      programHolder.setProgram(
          CreateProgram(pOCLProgram, spCompileService.get()));
      PriorityBooster booster(
          !pOCLRunConfig->GetValue<bool>(RC_BR_MEASURE_PERFORMANCE, false));

      BuildProgram(programHolder.getProgram(), spCompileService.get(),
                   runResult, pOCLRunConfig, pOCLProgramConfig);
    }

    ICLDevBackendProgram_ *program = programHolder.getProgram();
    if (!pOCLRunConfig->GetValue<std::string>(RC_BR_DUMP_OPTIMIZED_LLVM_IR, "")
             .empty()) {
      // currently dumping to the file is temporary unsupported
      const ICLDevBackendCodeContainer *pCodeContainer =
          program->GetProgramIRCodeContainer();
      ProgramDumpConfig dumpOptions(pOCLRunConfig);
      spCompileService->DumpCodeContainer(pCodeContainer, &dumpOptions);
    }

    if (!pOCLRunConfig->GetValue<std::string>(RC_BR_DUMP_JIT, "").empty()) {
      std::string filename = Utils::GetDataFilePath(
          pOCLRunConfig->GetValue<std::string>(RC_BR_DUMP_JIT, ""),
          pOCLProgramConfig->GetBaseDirectory());
      ProgramJitDumpConfig dumpOptions(filename);
      spCompileService->DumpJITCodeContainer(program, &dumpOptions);
    }

    if (pOCLRunConfig->GetValue<bool>(RC_BR_DUMP_KERNEL_PROPERTY, false)) {
      for (auto it = pOCLProgramConfig->beginKernels(),
                e = pOCLProgramConfig->endKernels();
           it != e; ++it)
        DumpKernelProperties(program, *it);
    }

    if (pOCLRunConfig->GetValue<bool>(RC_BR_BUILD_ONLY, false)) {
      return;
    }

    for (OpenCLProgramConfiguration::KernelConfigList::const_iterator it =
             pOCLProgramConfig->beginKernels();
         it != pOCLProgramConfig->endKernels(); ++it) {
      BufferContainerList input;
      LoadInputBuffer(*it, &input);

      if (pOCLRunConfig->GetValue<bool>(RC_BR_VERBOSE, false)) {
        PrintMeta(input);
      }

      PriorityBooster booster(
          !pOCLRunConfig->GetValue<bool>(RC_BR_MEASURE_PERFORMANCE, false));
      for (uint32_t i = 0; i < pOCLRunConfig->GetValue<uint32_t>(
                                   RC_BR_EXECUTE_ITERATIONS_COUNT, 1);
           ++i) {
        ExecuteKernel(input, runResult, program, spImageService.get(), *it,
                      pOCLRunConfig);
      }
    }
  } // ProgramHolder scope end
}

void OpenCLCPUBackendRunner::DumpKernelProperties(
    ICLDevBackendProgram_ *program, OpenCLKernelConfiguration *kernelConfig) {
  assert(program && kernelConfig && "invalid program or kernelConfig");
  std::string kernelName = kernelConfig->GetKernelName();
  const ICLDevBackendKernel_ *kernel = nullptr;
  cl_dev_err_code err = program->GetKernelByName(kernelName.c_str(), &kernel);
  if (err != CL_DEV_SUCCESS)
    throw Exception::TestRunnerException("GetKernelByName " + kernelName +
                                         " failed with error code " +
                                         std::to_string(err));
  assert(kernel && "invalid kernel");
  auto *kernelProperties = kernel->GetKernelProporties();
  assert(kernelProperties && "invalid kernel property");
  kernelProperties->Print();
}

static void initWorkInfo(cl_work_description_type *workInfo,
                         const OpenCLKernelConfiguration *pKernelConfig,
                         const BERunOptions *pRunConfig) {

  std::copy(pKernelConfig->GetLocalWorkSize(),
            pKernelConfig->GetLocalWorkSize() + MAX_WORK_DIM,
            workInfo->localWorkSize);

  std::copy(pKernelConfig->GetGlobalWorkOffset(),
            pKernelConfig->GetGlobalWorkOffset() + MAX_WORK_DIM,
            workInfo->globalWorkOffset);

  std::copy(pKernelConfig->GetGlobalWorkSize(),
            pKernelConfig->GetGlobalWorkSize() + MAX_WORK_DIM,
            workInfo->globalWorkSize);

  // TODO: this number should be similar to how the runtime set it,
  //       i.e. number-of-working-threads
  workInfo->minWorkGroupNum =
      1; // Intel::OpenCL::Utils::GetNumberOfProcessors();
  workInfo->workDimension = pKernelConfig->GetWorkDimension();
  // adjust the local work group sized in case we are running
  // in validation mode. Adjusting is mainly selecting the appropriate
  // local work group size if one was not selected by user. We need
  // to adjust to be able to use the same value as a reference runner.
  // In Performance mode no adjustment is needed and we let the backend
  // to select one
  if (!pRunConfig->GetValue<bool>(RC_BR_MEASURE_PERFORMANCE, false)) {
    for (size_t i = 0; i < workInfo->workDimension; ++i) {
      if (workInfo->localWorkSize[i] == 0) {
        workInfo->localWorkSize[i] = std::min<uint32_t>(
            static_cast<uint32_t>(pKernelConfig->GetGlobalWorkSize()[i]),
            pRunConfig->GetValue<uint32_t>(RC_COMMON_DEFAULT_LOCAL_WG_SIZE, 0));
        // the values specified in globalWorkSize[0], ..,
        // globalWorkSize[work_dim - 1] must be evenly divisible by
        // the corresponding values specified in
        // localWorkSize[0], .., localWorkSize[work_dim - 1]
        if (static_cast<uint32_t>(pKernelConfig->GetGlobalWorkSize()[i]) %
                workInfo->localWorkSize[i] !=
            0) {
          workInfo->localWorkSize[i] = 1;
        }
      }
    }
  }
}

void OpenCLCPUBackendRunner::ExecuteKernel(
    IBufferContainerList &input, IRunResult *runResult,
    ICLDevBackendProgram_ *pProgram, ICLDevBackendImageService *pImageService,
    OpenCLKernelConfiguration *pKernelConfig, const BERunOptions *pRunConfig) {

  assert(NULL != pProgram);
  assert(pImageService);

  // Get kernel to run
  std::string kernelName = pKernelConfig->GetKernelName();
  const ICLDevBackendKernel_ *pKernel = NULL;

  cl_dev_err_code errCode =
      pProgram->GetKernelByName(kernelName.c_str(), &pKernel);

  // currently pProgram->GetKernelByName returns CL_DEV_INVALID_KERNEL_NAME or
  // CL_DEV_SUCCESS only
  switch (errCode) {
  case CL_DEV_SUCCESS:
    break;
  case CL_DEV_INVALID_KERNEL_NAME:
    throw Exception::TestRunnerException(
        std::string("kernel name ") + kernelName +
        std::string(" was not found in the source code\n"));
    break;
  default:
    throw Exception::TestRunnerException(
        "Unexpected error code from ICLDevBackendProgram_::GetKernelByName "
        "method");
  }

  // Get kernel arguments
  int kernelNumArgs = pKernel->GetKernelParamsCount();
  const KernelArgument *pKernelArgs = pKernel->GetKernelParams();
  std::vector<bool> ignoreList;
  FillIgnoreList(ignoreList, pKernelArgs, kernelNumArgs);
  runResult->SetComparatorIgnoreList(kernelName.c_str(), ignoreList);

  auto *KernelF = m_pModule->getFunction(pKernelConfig->GetKernelName());
  assert(KernelF && KernelF->getCallingConv() == CallingConv::SPIR_KERNEL &&
         "No valid kernel found");

  DataVersion::ConvertData(&input, KernelF);

  // Create the argument buffer
  OpenCLArgsBuffer argsBuffer(
      pKernelArgs, kernelNumArgs, &input, pImageService,
      !pRunConfig->GetValue<bool>(RC_BR_MEASURE_PERFORMANCE, false));

  cl_work_description_type workInfo;

  initWorkInfo(&workInfo, pKernelConfig, pRunConfig);

  OpenCLExecutionContext spContext(pKernel, pRunConfig, workInfo,
                                   argsBuffer.GetArgsBuffer(),
                                   argsBuffer.GetArgsBufferSize());

  size_t regions[MAX_WORK_DIM];
  size_t dim = workInfo.workDimension;
  // init the work group regions
  for (size_t i = 0; i < dim; ++i) {
    regions[i] = (pKernelConfig->GetGlobalWorkSize()[i] /
                  spContext.GetWorkGroupSize()[i]) +
                 (pKernelConfig->GetGlobalWorkSize()[i] %
                      spContext.GetWorkGroupSize()[i] !=
                  0);
  }

  // Note:
  //     Think of refactoring this code for more elegant solution:
  //     Probably using the task executor model used in OCL SDK

  if (pRunConfig->GetValue<bool>(RC_BR_MEASURE_PERFORMANCE, false)) {
    spContext.ResetSampling();
  }

  if (pRunConfig->GetValue<bool>(RC_BR_USE_PIN_TRACE_MARKS, false)) {
    GenINT3();
  }

  if (pRunConfig->GetValue<bool>(RC_COMMON_RUN_SINGLE_WG, false)) {
    spContext.ExecuteWorkGroup(0, 0, 0);
  } else {
    switch (dim) {
    case 1:
      for (size_t x = 0; x < regions[0]; ++x) {
        spContext.ExecuteWorkGroup(x, 0, 0);
      }
      break;
    case 2:
      for (size_t y = 0; y < regions[1]; ++y) {
        for (size_t x = 0; x < regions[0]; ++x) {
          spContext.ExecuteWorkGroup(x, y, 0);
        }
      }
      break;
    case 3:
      for (size_t z = 0; z < regions[2]; ++z) {
        for (size_t y = 0; y < regions[1]; ++y) {
          for (size_t x = 0; x < regions[0]; ++x) {
            spContext.ExecuteWorkGroup(x, y, z);
          }
        }
      }
      break;
    default:
      throw Exception::TestRunnerException(
          "Wrong number of dimensions while running the kernel\n");
    }
  }

  if (pRunConfig->GetValue<bool>(RC_BR_USE_PIN_TRACE_MARKS, false)) {
    GenINT3();
  }

  if (pRunConfig->GetValue<bool>(RC_BR_MEASURE_PERFORMANCE, false)) {
    Performance &perfResults = (Performance &)runResult->GetPerformance();
    perfResults.SetExecutionTime(kernelName, spContext.GetVectorSize(),
                                 spContext.GetSampling());
  } else // Do not save output in PERF mode.
  {
    // Copy output into runResult output buffer container list
    IBufferContainerList &output = runResult->GetOutput(kernelName.c_str());

    argsBuffer.CopyOutput(output, &input);
  }
}

} // namespace Validation
