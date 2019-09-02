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

#include "CompilerConfig.h"
#include "Kernel.h"
#include "KernelProperties.h"
#include "ImplicitArgsUtils.h"
#include "TypeAlignment.h"
#include "exceptions.h"
#include "Serializer.h"

#include "cpu_dev_limits.h"

#include <cstring>
#include <cmath>
#include <iostream>
#include <sstream>
#include <stddef.h>
#include <stdio.h>

#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#define DEBUG_TYPE "ocl-rt-kernel"

static size_t GCD(size_t a, size_t b) {
  while (1) {
    a = a % b;
    if (a == 0)
      return b;
    b = b % a;
    if (b == 0)
      return a;
  }
}

#define BITS_IN_BYTE (8)
static unsigned int LOG(unsigned int a) {
  assert((a != 0) && ((a & (a - 1)) == 0) && "assume a is a power of 2");
  for (unsigned int i = 0; i < (unsigned int)sizeof(a) * BITS_IN_BYTE; ++i) {
    if (a & 0x1)
      return i;
    a = a >> 1;
  }
  return 0;
}

unsigned int min(unsigned int a, unsigned int b) { return a < b ? a : b; }

unsigned int max(unsigned int a, unsigned int b) { return a > b ? a : b; }

#if defined(ENABLE_SDE)
// These functions are used as marks for the debug trace of the JIT execution
extern "C" {
void BeforeExecution() {}
void AfterExecution() {}
}
#endif // ENABLE_SDE

namespace Intel {
namespace OpenCL {
namespace DeviceBackend {

Kernel::Kernel(const std::string &name,
               const std::vector<cl_kernel_argument> &args,
               const std::vector<unsigned int> &memArgs,
               KernelProperties *pProps)
    : m_name(name), m_CSRMask(0), m_CSRFlags(0), m_explicitArgs(args),
      m_RequiredUniformKernelArgsAlignment(MinRequiredKernelArgAlignment),
      m_memArgs(memArgs), m_pProps(pProps) {
  if (!m_explicitArgs.empty()) {
    // calculates the whole explicit arguments buffer size
    // offset of the last argument in the buffer + argumentSize
    // and adjust each argument at least to size_t alignment
    // because of the implicit arguments
    const cl_kernel_argument &lastArg = m_explicitArgs.back();
    m_explicitArgsSizeInBytes = ImplicitArgsUtils::getAdjustedAlignment(
        lastArg.offset_in_bytes + TypeAlignment::getSize(lastArg),
        sizeof(size_t));
  } else {
    m_explicitArgsSizeInBytes = 0;
  }

  // Set CSR flagsCreateWorkDescription
  m_CSRMask |= _MM_FLUSH_ZERO_MASK;
  m_CSRMask |= _MM_DENORMALS_ZERO_MASK;

  if (m_pProps->GetDAZ()) {
    m_CSRFlags |= _MM_FLUSH_ZERO_ON;     // OFF is default
    m_CSRFlags |= _MM_DENORMALS_ZERO_ON; // OFF is default
  }
  m_CSRMask |= _MM_ROUND_MASK;
  m_CSRFlags |= _MM_ROUND_NEAREST; // Default

  // calculating the required alignment for the arguments buffer
  for (unsigned int i = 0; i < m_explicitArgs.size(); ++i) {
    if (m_RequiredUniformKernelArgsAlignment < TypeAlignment::getAlignment(m_explicitArgs[i])) {
      m_RequiredUniformKernelArgsAlignment = TypeAlignment::getAlignment(m_explicitArgs[i]);
    }
  }
}

Kernel::~Kernel() {
  delete m_pProps;
  for (std::vector<IKernelJITContainer *>::iterator i = m_JITs.begin(),
                                                    e = m_JITs.end();
       i != e; ++i) {
    delete *i;
  }
}

void Kernel::AddKernelJIT(IKernelJITContainer *pJIT) { m_JITs.push_back(pJIT); }

void Kernel::FreeAllJITs() {
  for (unsigned i = 0; i < m_JITs.size(); ++i) {
    m_JITs[i]->FreeJITCode();
  }
}

void Kernel::CreateWorkDescription(cl_uniform_kernel_args *UniformImplicitArgs,
                                   size_t numOfComputeUnits) const {
  // assumption: LocalWorkSize GlobalSize and minWorkGroup already initialized

  size_t max_wg_private_size = m_pProps->GetMaxPrivateMemorySize()
                               ? m_pProps->GetMaxPrivateMemorySize()
                               : CPU_DEV_MAX_WG_PRIVATE_SIZE;

  size_t maxWorkGroupSize = (m_pProps->TargetDevice() == FPGA_EMU_DEVICE)
                            ? FPGA_MAX_WORK_GROUP_SIZE
                            : CPU_MAX_WORK_GROUP_SIZE;

  bool UseAutoGroupSize = true;
  for (unsigned int i = 0; i < UniformImplicitArgs->WorkDim; ++i) {
    UseAutoGroupSize = UseAutoGroupSize && ((UniformImplicitArgs->LocalSize[UNIFORM_WG_SIZE_INDEX][i]) == 0);
  }

  // TODO[MA]: recheck if the CanUniteWG flag is set only if the correctness analysis passed
  bool canUniteWG = m_pProps->GetCanUniteWG();
  unsigned int vectorizeOnDim = m_pProps->GetVectorizedDimention();
  //
  // When JIT transforms a simple loop to memcpy, the memcpy size should is bigger enough
  // to get a better performance, but the CPU_MAX_WORK_GROUP_SIZE is fixed to 8192,
  // we couldn't change it directly, because some CTS cases depends on it.
  // the forced size should meet below conditions,
  // it only for CPU device now, other device might need to tune a new factor.
  //
  // 1. forcedSize > factor(8000) and forcedSize % factor(8000) == 0
  // 2. globalWorkSize > forcedSize and globalWorkSize % forcedSize == 0
  //
  // if it doesn't be set, using the origial algorithm to calculate it
  bool UseForcedWGSize = false;
  unsigned int forcedSize = m_pProps->GetForcedWorkGroupSize();
  LLVM_DEBUG(llvm::dbgs() << "Forced Work Group Size: " << m_pProps->GetForcedWorkGroupSize() <<
             ", Can use it? " << m_pProps->GetCanUseForcedWorkGroupSize());
  if (UseAutoGroupSize && m_pProps->TargetDevice() == CPU_DEVICE &&
      m_pProps->GetForcedWorkGroupSize() > CPU_MAX_WORK_GROUP_SIZE &&
      m_pProps->GetCanUseForcedWorkGroupSize()) {
    size_t gwSizeX = UniformImplicitArgs->GlobalSize[vectorizeOnDim];
    unsigned int gwSizeYZ = 1;
    for (unsigned int i = 0; i < UniformImplicitArgs->WorkDim; ++i) {
      if (vectorizeOnDim == i) { //skip the dimension on which we vectorized
        continue;
      }
      // Calculate global group size on dimensions Y & Z
      gwSizeYZ *= UniformImplicitArgs->GlobalSize[i];
    }
    // TODO: User should follow same logic to set CL_CONFIG_CPU_FORCE_WORK_GROUP_SIZE
    unsigned int factor = (CPU_MAX_WORK_GROUP_SIZE == 8 * 1024 ? 8000 : 0);
    const unsigned int gwSize = gwSizeX * gwSizeYZ;
    if (factor > 0 && gwSizeYZ == 1 &&
        (forcedSize % factor == 0) && (forcedSize > factor) &&
        (gwSize > forcedSize) && (gwSize % forcedSize == 0)) {
      UseForcedWGSize = true;
    }
    if (UseForcedWGSize) {
      UniformImplicitArgs->LocalSize[UNIFORM_WG_SIZE_INDEX][vectorizeOnDim] =
      UniformImplicitArgs->LocalSize[NONUNIFORM_WG_SIZE_INDEX][vectorizeOnDim] = forcedSize;
    }
  }

  // In case we can merge WG's but we have local size given (not NULL)
  if (canUniteWG && !UseAutoGroupSize && !UseForcedWGSize) {
    // Need to merge WG in the dimension a kernel is vectorized for
    size_t localWorkSizeX = UniformImplicitArgs->LocalSize[UNIFORM_WG_SIZE_INDEX][vectorizeOnDim];
    size_t globalWorkSizeX = UniformImplicitArgs->GlobalSize[vectorizeOnDim];
    // Compute the maximum WG size given that there are N threads to run on.
    unsigned int localSizeUpperLimit = min(globalWorkSizeX / numOfComputeUnits,
             m_pProps->GetMaxWorkGroupSize(maxWorkGroupSize, max_wg_private_size));
    // Make WG size upper limit to be al least of vector size in case if local size set to 1
    unsigned int minMultiplyFactor = m_pProps->GetMinGroupSizeFactorial();
    size_t loopUpperLimit = min(minMultiplyFactor*localWorkSizeX, localSizeUpperLimit);
    // Some magic happens below. Unfortunately the reasoning why it works this way is lost for good.
    size_t baseVectorIterations = localWorkSizeX / minMultiplyFactor;
    size_t baseScalarIterations = localWorkSizeX - (baseVectorIterations * minMultiplyFactor);

    size_t vectorIterations = 0;
    size_t scalarIterations = 0;

    size_t maxUtil = 0;
    size_t bestLocalSize = localWorkSizeX;
    for(size_t currLocalSize = localWorkSizeX; currLocalSize < loopUpperLimit; currLocalSize += localWorkSizeX) {
      // update vector and scalar iterations counters
      vectorIterations += baseVectorIterations;
      scalarIterations += baseScalarIterations;
      if(scalarIterations >= minMultiplyFactor) {
        scalarIterations -= minMultiplyFactor;
        vectorIterations += 1;
      }

      // if the suggest WG size (currLocalSize) don't divide the global size move on
      if((globalWorkSizeX % currLocalSize) != 0) continue;

      size_t currentUtil = currLocalSize / (vectorIterations + scalarIterations);
      if(currentUtil > maxUtil) {
        maxUtil = currentUtil;
        bestLocalSize = currLocalSize;
      }
    }
    UniformImplicitArgs->LocalSize[UNIFORM_WG_SIZE_INDEX][vectorizeOnDim] = bestLocalSize;
  }

  else if (UseAutoGroupSize && !UseForcedWGSize) {
    unsigned int globalWorkSizeYZ = 1;
    for (unsigned int i = 0; i < UniformImplicitArgs->WorkDim; ++i) {
      if (vectorizeOnDim == i){ //skip the dimension on which we vectorized
        continue;
      }

      // Calculate global group size on dimensions Y & Z
      globalWorkSizeYZ *= UniformImplicitArgs->GlobalSize[i];
    }

    unsigned int globalWorkSizeX = UniformImplicitArgs->GlobalSize[vectorizeOnDim];
    unsigned int localSizeUpperLimit = min(globalWorkSizeX,
          m_pProps->GetMaxWorkGroupSize(maxWorkGroupSize, max_wg_private_size));
    assert(0 < localSizeUpperLimit &&
           "clEnqueueNDRangeKernel must fail with CL_OUT_OF_RESOURCES earlier.");

    unsigned int minMultiplyFactor = m_pProps->GetMinGroupSizeFactorial();
    assert(minMultiplyFactor &&
           (minMultiplyFactor & (minMultiplyFactor - 1)) == 0 &&
           "minMultiplyFactor assumed to be power of 2 that is not zero!");
    assert(
        (!m_pProps->IsVectorizedWithTail() ||
         GetKernelJIT(0)->GetProps()->GetVectorSize() == minMultiplyFactor) &&
        "GetMinGroupSizeFactorial is not equal to VectorSize!");
    unsigned int minMultiplyFactorLog = LOG(minMultiplyFactor);

    const unsigned int globalWorkSize = globalWorkSizeX * globalWorkSizeYZ;
    // These variables hold the max utility of SIMD and work threads
    const unsigned int workThreadUtils = UniformImplicitArgs->minWorkGroupNum;
    const unsigned int simdUtilsLog = minMultiplyFactorLog;
    // Try to assure (if possible) the local-size is a multiply of vector
    // width.
    if (((globalWorkSizeX & (minMultiplyFactor - 1)) == 0) &&
        (localSizeUpperLimit >= minMultiplyFactor)) {
      globalWorkSizeX = globalWorkSizeX >> minMultiplyFactorLog;
      localSizeUpperLimit = localSizeUpperLimit >> minMultiplyFactorLog;
    } else {
      // SIMD utility was not satisfied
      minMultiplyFactor = 1;
      minMultiplyFactorLog = 0;
    }
    unsigned int localSizeMaxLimit = localSizeUpperLimit;
    const bool isLargeGlobalWGsize =
        ((workThreadUtils << simdUtilsLog) < globalWorkSize);
    if (isLargeGlobalWGsize) {
      if (m_pProps->HasGlobalSyncOperation()) {
        localSizeMaxLimit = min(localSizeMaxLimit, globalWorkSize/(workThreadUtils << simdUtilsLog));
      } else {
        localSizeMaxLimit = min(
             localSizeMaxLimit,
             // Calculating lower bound for [sqrt(global/(WT*SIMD))*SIMD]
             // Starting the search from this number improves the chances to
             // find a local size that satisfies the "balance" factor.
             // Optimal balanced local size applies the following:
             // Let,
             //   X - local size
             //   Y - number of work groups = (global size / local size)
             // Then: (X * workThreadUtils) == (Y << simdUtilsLog)
             ((unsigned int)sqrt((float)(globalWorkSize/(workThreadUtils << simdUtilsLog)))) << (simdUtilsLog-minMultiplyFactorLog) );
      }
    } else {
      //In this case we have few work-items, try satisfy as much as possible of work threads.
      const unsigned int workGroupNumMinLimit = (workThreadUtils + (globalWorkSizeYZ-1)) / globalWorkSizeYZ;
      localSizeMaxLimit = max(1, localSizeMaxLimit / workGroupNumMinLimit);
    }
    assert(localSizeMaxLimit <= globalWorkSizeX && "global size in dim X must be upper bound for local size");

    // Try hint size, find GCD for each dimension
    if (m_pProps->GetHintWGSize()[0] != 0) {
      for (unsigned int i = 0; i < UniformImplicitArgs->WorkDim; ++i) {
        UniformImplicitArgs->LocalSize[UNIFORM_WG_SIZE_INDEX][i] =
        UniformImplicitArgs->LocalSize[NONUNIFORM_WG_SIZE_INDEX][i] = GCD(
            UniformImplicitArgs->GlobalSize[i], m_pProps->GetHintWGSize()[i]);
      }
    }
    // If we have only one thread for execution
    else if (1 == numOfComputeUnits && globalWorkSize <= localSizeMaxLimit) {
      // Make local size == global size
      auto GlobalSizeBegin = UniformImplicitArgs->GlobalSize;
      auto GlobalSizeEnd =
          UniformImplicitArgs->GlobalSize + UniformImplicitArgs->WorkDim;
      std::copy(GlobalSizeBegin, GlobalSizeEnd,
                UniformImplicitArgs->LocalSize[UNIFORM_WG_SIZE_INDEX]);
      std::copy(GlobalSizeBegin, GlobalSizeEnd,
                UniformImplicitArgs->LocalSize[NONUNIFORM_WG_SIZE_INDEX]);
    } else {
      // Old Heuristic
      // On the last use optimal size
      // Fill first dimension with WG size
      // outputWorkSizes.localWorkSize[0] =
      // GCD(pInputWorkSizes->globalWorkSize[0], m_pProps->GetOptWGSize());
      //
      // for(unsigned int i=1; i<outputWorkSizes.workDimension; ++i)
      //{
      //    outputWorkSizes.localWorkSize[i] = 1;
      //}

      // New Heuristic
      for (unsigned int i = 0; i < UniformImplicitArgs->WorkDim; ++i) {
        if (vectorizeOnDim == i){ //skip the dimension on which we vectorized
          continue;
        }

        // Set local group size on dimensions Y & Z to 1
        UniformImplicitArgs->LocalSize[UNIFORM_WG_SIZE_INDEX][i] =
        UniformImplicitArgs->LocalSize[NONUNIFORM_WG_SIZE_INDEX][i]  = 1;
      }
      //Search for max local size that satisfies the constraints
      unsigned int newHeuristic = localSizeMaxLimit;
      for (; newHeuristic>1; newHeuristic--) {
        if ( globalWorkSizeX % newHeuristic == 0 ) {
          break;
        }
      }
      newHeuristic = newHeuristic << minMultiplyFactorLog;
      //For small global WG size no need for checking the balance cost function
      if(isLargeGlobalWGsize) {
        //Cost function: check if we found a balanced local size compared to number of work groups
        #define BALANCE_FACTOR (2)
        const unsigned int workGroups = globalWorkSize / newHeuristic;
        assert((m_pProps->HasGlobalSyncOperation() || // do not check that work is balanced if we are trying to minimize the number of WGs
            (workGroups << simdUtilsLog) >= (newHeuristic * workThreadUtils)) && "Wrong balance factor calculation!");
        const int balanceFactor = (workGroups << simdUtilsLog) - BALANCE_FACTOR * (newHeuristic * workThreadUtils);
        if ( balanceFactor > 0 ) {
          localSizeUpperLimit = min(localSizeUpperLimit, (unsigned int)sqrt((float)globalWorkSize));
          assert(localSizeUpperLimit <= globalWorkSizeX && "global size in dim X must be upper bound for local size");
          //Try to search better local size
          unsigned int newHeuristicUp = localSizeMaxLimit+1;
          for (; newHeuristicUp<=localSizeUpperLimit; newHeuristicUp++) {
            if ( globalWorkSizeX % newHeuristicUp == 0 ) {
              newHeuristicUp = newHeuristicUp << minMultiplyFactorLog;
              //Check cost function and update heuristic if needed
              const unsigned int workGroupsUp = globalWorkSize / newHeuristicUp;
              const int balanceFactorUp = (newHeuristicUp * workThreadUtils) >= (workGroupsUp << simdUtilsLog) ?
                (newHeuristicUp * workThreadUtils) - BALANCE_FACTOR * (workGroupsUp << simdUtilsLog) :
                (workGroupsUp << simdUtilsLog) - BALANCE_FACTOR * (newHeuristicUp * workThreadUtils);
              if (balanceFactorUp < balanceFactor) {
                //Found better local size
                newHeuristic = newHeuristicUp;
              }
              break;
            }
          }
        }
      }
      UniformImplicitArgs->LocalSize[UNIFORM_WG_SIZE_INDEX][vectorizeOnDim] =
      UniformImplicitArgs->LocalSize[NONUNIFORM_WG_SIZE_INDEX][vectorizeOnDim] = newHeuristic;
      assert(UniformImplicitArgs->LocalSize[UNIFORM_WG_SIZE_INDEX][vectorizeOnDim] > 0 &&
             "local size must be positive number");
    }
  }
}

unsigned long long int Kernel::GetKernelID() const { return 0; }

const char *Kernel::GetKernelName() const { return m_name.c_str(); }

int Kernel::GetKernelParamsCount() const { return m_explicitArgs.size(); }

const cl_kernel_argument *Kernel::GetKernelParams() const {
  return m_explicitArgs.empty() ? nullptr : &m_explicitArgs[0];
}

size_t Kernel::GetArgumentBufferRequiredAlignment() const {
  return m_RequiredUniformKernelArgsAlignment;
}

const cl_kernel_argument_info *Kernel::GetKernelArgInfo() const { return nullptr; }

const ICLDevBackendKernelProporties *Kernel::GetKernelProporties() const {
  return m_pProps;
}

int Kernel::GetLineNumber(void *pointer) const {
  const unsigned int tNumJits = GetKernelJITCount();
  int lineNum = -1;
  for (unsigned int i = 0; (i < tNumJits) && (-1 == lineNum); i++) {
    lineNum = GetKernelJIT(i)->GetLineNumber(pointer);
  }
  return lineNum;
}

size_t Kernel::GetExplicitArgumentBufferSize() const {
  return m_explicitArgsSizeInBytes;
}

unsigned int Kernel::GetMemoryObjectArgumentCount() const {
  return m_memArgs.size();
}

const unsigned int *Kernel::GetMemoryObjectArgumentIndexes() const {
  return m_memArgs.empty() ? nullptr : &m_memArgs[0];
}

const std::vector<cl_kernel_argument> *Kernel::GetKernelParamsVector() const {
  return &m_explicitArgs;
}

const IKernelJITContainer *Kernel::GetKernelJIT(unsigned int index) const {
  assert(index < m_JITs.size());
  return m_JITs[index];
}

unsigned int Kernel::GetKernelJITCount() const { return m_JITs.size(); }

cl_dev_err_code Kernel::InitRunner(void *pKernelUniformArgs) const {
  assert(pKernelUniformArgs && "Uniform Arguments Pointer is null");
  void *pKernelUniformImplicitArgsPosition =
      ((char *)pKernelUniformArgs) + m_explicitArgsSizeInBytes;
  cl_uniform_kernel_args *pKernelUniformImplicitArgs =
      static_cast<cl_uniform_kernel_args *>(pKernelUniformImplicitArgsPosition);

  pKernelUniformImplicitArgs->pUniformJITEntryPoint =
      ResolveEntryPointHandle(pKernelUniformImplicitArgs->pUniformJITEntryPoint);
  pKernelUniformImplicitArgs->pNonUniformJITEntryPoint =
      ResolveEntryPointHandle(pKernelUniformImplicitArgs->pNonUniformJITEntryPoint);

  assert(pKernelUniformImplicitArgs->RuntimeInterface);
  return CL_DEV_SUCCESS;
}

cl_dev_err_code Kernel::PrepareThreadState(ICLDevExecutionState &state) const {
  state.MXCSRstate = _mm_getcsr();
  unsigned int uiNewFlags = (state.MXCSRstate & ~m_CSRMask) | m_CSRFlags;
  _mm_setcsr(uiNewFlags);
  return CL_DEV_SUCCESS;
}

cl_dev_err_code
Kernel::PrepareKernelArguments(void *pKernelUniformArgs,
                               const cl_mem_obj_descriptor **pDevMemObjArray,
                               unsigned int devMemObjArrayLength,
                               size_t numOfComputeUnits) const {
  assert(pKernelUniformArgs && "Uniform Arguments Pointer is null");
  void *pKernelUniformImplicitArgsPosition =
      (char *)pKernelUniformArgs + m_explicitArgsSizeInBytes;
  cl_uniform_kernel_args *pKernelUniformImplicitArgs =
      static_cast<cl_uniform_kernel_args *>(pKernelUniformImplicitArgsPosition);

  CreateWorkDescription(pKernelUniformImplicitArgs, numOfComputeUnits);

  // local cannot be zero at this point
  assert(pKernelUniformImplicitArgs->LocalSize[UNIFORM_WG_SIZE_INDEX][0] != 0 &&
         "LocalSize must be properly initialized");

  // In case of (pKernelUniformImplicitArgs.WorkDim < MAX_WORK_DIM)
  //  need to set local and global size to 1 for OOB dimensions
  for (unsigned int i = pKernelUniformImplicitArgs->WorkDim; i < MAX_WORK_DIM;
       ++i) {
    pKernelUniformImplicitArgs->GlobalSize[i] = 1;
    pKernelUniformImplicitArgs->LocalSize[UNIFORM_WG_SIZE_INDEX][i] =
    pKernelUniformImplicitArgs->LocalSize[NONUNIFORM_WG_SIZE_INDEX][i] = 1;
    pKernelUniformImplicitArgs->GlobalOffset[i] = 0;
    pKernelUniformImplicitArgs->WGCount[i] = 1;
    // no need to set GlobalOffset as it is used later only according
    // to dimension
  }

  // Calculate number of work groups and WG size
  for (unsigned int i = 0; i < pKernelUniformImplicitArgs->WorkDim; ++i) {
    assert((m_pProps->IsNonUniformWGSizeSupported() ||
            0 == pKernelUniformImplicitArgs->GlobalSize[i] %
                 pKernelUniformImplicitArgs->LocalSize[UNIFORM_WG_SIZE_INDEX][i]) &&
      "Kernel is built without support of non-uniform work-group size but it is non-uniform.");
    size_t GlbSize = pKernelUniformImplicitArgs->GlobalSize[i];
    size_t LclSize = pKernelUniformImplicitArgs->LocalSize[UNIFORM_WG_SIZE_INDEX][i];
    pKernelUniformImplicitArgs->WGCount[i] = GlbSize / LclSize;
    // In case of non-uniform work-group size there can be a reminder group
    // with size less than size of other groups.
    pKernelUniformImplicitArgs->WGCount[i] += static_cast<size_t>(0 != GlbSize % LclSize);
  }

  // need to decide which entrypoint to run
  const IKernelJITContainer *pScalarJIT = GetKernelJIT(0);
  if (m_pProps->IsVectorizedWithTail()) {
    // Vectorized kernel is inlined into scalar kernel,
    // so we have exactly one JIT function.
    // In this case pScalarJit contains this combination.
    pKernelUniformImplicitArgs->pUniformJITEntryPoint =
    pKernelUniformImplicitArgs->pNonUniformJITEntryPoint =
        CreateEntryPointHandle(pScalarJIT->GetJITCode());
  } else {
    const IKernelJITContainer *pVectorJIT =
        GetKernelJITCount() > 1 ? GetKernelJIT(1) : nullptr;
    if(pVectorJIT) {
      // The both vectorized and scalar JITs are present.
      // Rely on the fact that JIT is always vectorized by work group size from 0 d.
      //
      // Use vector JIT if get_local_size(0) is evenly divisible by the vector width
      // and scalar JIT otherwise.
      bool useVectorJit = pKernelUniformImplicitArgs->LocalSize[UNIFORM_WG_SIZE_INDEX][0] %
                          pVectorJIT->GetProps()->GetVectorSize() == 0;
      pKernelUniformImplicitArgs->pUniformJITEntryPoint = useVectorJit ?
                                                          CreateEntryPointHandle(pVectorJIT->GetJITCode()) :
                                                          CreateEntryPointHandle(pScalarJIT->GetJITCode());

      useVectorJit = pKernelUniformImplicitArgs->LocalSize[NONUNIFORM_WG_SIZE_INDEX][0] %
                     pVectorJIT->GetProps()->GetVectorSize() == 0;
      pKernelUniformImplicitArgs->pNonUniformJITEntryPoint = useVectorJit ?
                                                          CreateEntryPointHandle(pVectorJIT->GetJITCode()) :
                                                          CreateEntryPointHandle(pScalarJIT->GetJITCode());
    } else {
      // Only scalar JIT is present.
      pKernelUniformImplicitArgs->pUniformJITEntryPoint =
      pKernelUniformImplicitArgs->pNonUniformJITEntryPoint =
          CreateEntryPointHandle(pScalarJIT->GetJITCode());
    }
  }

  // FIXME: CSSD1000?????: add a check for OpenCL 2.0
  if (true ) { // ocl20
    pKernelUniformImplicitArgs->Block2KernelMapper = m_RuntimeService->GetBlockToKernelMapper();
  }

#ifdef OCL_DEV_BACKEND_PLUGINS

    cl_work_description_type workDesc;
    size_t sizetMaxWorkDim = sizeof(size_t)*MAX_WORK_DIM;
    workDesc.workDimension = pKernelUniformImplicitArgs->WorkDim;
    memcpy(workDesc.globalWorkOffset, pKernelUniformImplicitArgs->GlobalOffset, sizetMaxWorkDim);
    memcpy(workDesc.globalWorkSize, pKernelUniformImplicitArgs->GlobalSize, sizetMaxWorkDim);
    memcpy(workDesc.localWorkSize, pKernelUniformImplicitArgs->LocalSize, sizetMaxWorkDim);
    workDesc.minWorkGroupNum = pKernelUniformImplicitArgs->minWorkGroupNum;

    m_pluginManager.OnCreateBinary(this->GetKernel(), &workDesc, size_t(devMemObjArrayLength), pDevMemObjArray);

#endif



  if (false) {
    std::cout << "In Prepare Args:\n";
    std::cout << "pKernelUniformArgs = " << pKernelUniformArgs << "\n";
    DebugPrintUniformKernelArgs(
        pKernelUniformImplicitArgs,
        (const char *)(pKernelUniformImplicitArgsPosition) -
            (const char *)(pKernelUniformArgs),
        std::cout);
  }
  return CL_DEV_SUCCESS;
}

void Kernel::DebugPrintUniformKernelArgs(const cl_uniform_kernel_args *A,
                                         size_t offsetToImplicit,
                                         std::ostream &ss) const {
#define PRINT3(X) " = {" << (X)[0] << ", " << (X)[1] << ", " << (X)[2] << "}"
  size_t O = offsetToImplicit;
  ss << "KernelWrapper's argument:\n "
     << O + offsetof(cl_uniform_kernel_args, WorkDim) << ": size_t WorkDim = " << A->WorkDim << "\n"
     << O + offsetof(cl_uniform_kernel_args, GlobalOffset) << ": size_t GlobalOffset[MAX_WORK_DIM]" << PRINT3(A->GlobalOffset) << "\n"
     << O + offsetof(cl_uniform_kernel_args, GlobalSize) << ": size_t GlobalSize[MAX_WORK_DIM]" << PRINT3(A->GlobalSize) << "\n"
     << O + offsetof(cl_uniform_kernel_args, LocalSize[UNIFORM_WG_SIZE_INDEX]) <<
         ": size_t LocalSize[UNIFORM_WG_SIZE_INDEX][MAX_WORK_DIM]" << PRINT3(A->LocalSize[UNIFORM_WG_SIZE_INDEX]) << "\n"
     << O + offsetof(cl_uniform_kernel_args, LocalSize[NONUNIFORM_WG_SIZE_INDEX]) <<
        ": size_t LocalSize[NONUNIFORM_WG_SIZE_INDEX][MAX_WORK_DIM]" << PRINT3(A->LocalSize[NONUNIFORM_WG_SIZE_INDEX]) << "\n"
     << O + offsetof(cl_uniform_kernel_args, WGCount) << ": size_t WGCount[MAX_WORK_DIM]" << PRINT3(A->WGCount) << "\n"
     << O + offsetof(cl_uniform_kernel_args, RuntimeInterface)
     << ": void* RuntimeInterface= " << A->RuntimeInterface << "\n"
     << O + offsetof(cl_uniform_kernel_args, minWorkGroupNum) << ": size_t minWorkGroupNum= " << A->minWorkGroupNum << "\n"
     << O+offsetof(cl_uniform_kernel_args, RuntimeInterface) << ": void* RuntimeInterface = " << A->RuntimeInterface << "\n"
     << O + offsetof(cl_uniform_kernel_args, pUniformJITEntryPoint) << ": void* pUniformJITEntryPoint = " << A->pUniformJITEntryPoint << "\n"
     << O + offsetof(cl_uniform_kernel_args, pNonUniformJITEntryPoint) << ": void* pNonUniformJITEntryPoint = " << A->pNonUniformJITEntryPoint << "\n";
#undef PRINT3
}

#define SSC_MARK_1     \
        __asm__  ( "push %rbx              \n"\
                   "mov $0x1, %rbx         \n"\
                   ".byte 0x64, 0x67, 0x90 \n"\
                   "pop %rbx               \n")

#define SSC_MARK_2     \
        __asm__  ( "push %rbx              \n"\
                   "mov $0x2, %rbx         \n"\
                   ".byte 0x64, 0x67, 0x90 \n"\
                   "pop %rbx               \n")

cl_dev_err_code Kernel::RunGroup(const void *pKernelUniformArgs,
                                 const size_t *pGroupID,
                                 void *pRuntimeHandle) const {
  assert(pKernelUniformArgs && "kernelUniformArgs is null");
  assert(pGroupID && "pGroupID is null");
  assert(pRuntimeHandle && "pRuntimeHandle is null");

  const void *pKernelUniformImplicitArgsPosition =
      (const char *)pKernelUniformArgs + m_explicitArgsSizeInBytes;
  const cl_uniform_kernel_args *pKernelUniformImplicitArgs =
      static_cast<const cl_uniform_kernel_args *>(
          pKernelUniformImplicitArgsPosition);


  assert(pKernelUniformImplicitArgs->WorkDim <= MAX_WORK_DIM);
  static bool guard = true;
  if (false && guard) {
    guard = false; //Print only first group in NDRange to avoid huge dumps
    std::stringstream ss;
    ss << "GroupID: " << pGroupID[0] << ", " << pGroupID[1] << ", " << pGroupID[2] << "\n"
       << "pKernelUniformArgs = " << pKernelUniformArgs <<  "pRuntimeHandle = " << pRuntimeHandle << "\n";
    DebugPrintUniformKernelArgs(pKernelUniformImplicitArgs,
                                m_explicitArgsSizeInBytes, ss);
    std::cout << ss.str() << std::endl;
  }

  IKernelJITContainer::JIT_PTR *kernel =
      (IKernelJITContainer::JIT_PTR *)(size_t) ( pGroupID[0] != pKernelUniformImplicitArgs->WGCount[0] - 1 ?
                 pKernelUniformImplicitArgs->pUniformJITEntryPoint :
                 pKernelUniformImplicitArgs->pNonUniformJITEntryPoint);

  assert(kernel && "Kernel function is nullptr");

  // running the kernel with the specified args and (groupID, runtimeHandle)
#if defined (ENABLE_SDE)
  // do not forget to export BeforeExecution and AfterExecution symbols
  // in OclCpuBackEnd_linux_exports.txt
  BeforeExecution();

  SSC_MARK_1;
  kernel(pKernelUniformArgs, pGroupID, pRuntimeHandle);
  SSC_MARK_2;

  AfterExecution();
#else
  kernel(pKernelUniformArgs, pGroupID, pRuntimeHandle);
#endif  // ENABLE_SDE
  return CL_DEV_SUCCESS;
}

cl_dev_err_code Kernel::RestoreThreadState(ICLDevExecutionState &state) const {
  _mm_setcsr(state.MXCSRstate);
  return CL_DEV_SUCCESS;
}

void Kernel::Serialize(IOutputStream& ost, SerializationStatus* stats) const
{
  Serializer::SerialString(m_name, ost);

  // Serialize the CSRMask and CSRFlags
  Serializer::SerialPrimitive<unsigned int>(&m_CSRMask, ost);
  Serializer::SerialPrimitive<unsigned int>(&m_CSRFlags, ost);

  // Serialize the kernel arguments (one by one)
  unsigned int vectorSize = m_explicitArgs.size();
  Serializer::SerialPrimitive<unsigned int>(&vectorSize, ost);
  for (size_t i = 0; i < vectorSize; ++i) {
    Serializer::SerialPrimitive<cl_kernel_argument>(&m_explicitArgs[i], ost);
  }

  // Serialize explicit argument buffer size
  Serializer::SerialPrimitive<unsigned int>(&m_explicitArgsSizeInBytes, ost);
  // Serialize explicit argument buffer alignment
  Serializer::SerialPrimitive<unsigned int>(&m_RequiredUniformKernelArgsAlignment, ost);

  // Serial memory object information
  vectorSize = m_memArgs.size();
  Serializer::SerialPrimitive<unsigned int>(&vectorSize, ost);
  for (size_t i = 0; i < vectorSize; ++i) {
    Serializer::SerialPrimitive<unsigned int>(&m_memArgs[i], ost);
  }

  Serializer::SerialPointerHint((const void **)&m_pProps, ost);
  if (nullptr != m_pProps) {
    m_pProps->Serialize(ost, stats);
  }

  // Serial the kernel JIT's (one by one)
  vectorSize = m_JITs.size();
  Serializer::SerialPrimitive<unsigned int>(&vectorSize, ost);
  for (std::vector<IKernelJITContainer *>::const_iterator it = m_JITs.begin();
       it != m_JITs.end(); ++it) {
    IKernelJITContainer *currentArgument = (*it);
    Serializer::SerialPointerHint((const void **)&currentArgument, ost);
    if (nullptr != currentArgument) {
      currentArgument->Serialize(ost, stats);
    }
  }
}

void Kernel::Deserialize(IInputStream& ist, SerializationStatus* stats)
{
  Serializer::DeserialString(m_name, ist);

  // Deserialize the CSRMask and CSRFlags
  Serializer::DeserialPrimitive<unsigned int>(&m_CSRMask, ist);
  Serializer::DeserialPrimitive<unsigned int>(&m_CSRFlags, ist);

  // Deserial the kernel arguments (one by one)
  unsigned int vectorSize = 0;
  Serializer::DeserialPrimitive<unsigned int>(&vectorSize, ist);
  m_explicitArgs.resize(vectorSize);
  for (size_t i = 0; i < vectorSize; ++i) {
    Serializer::DeserialPrimitive<cl_kernel_argument>(&m_explicitArgs[i], ist);
  }

  // Deserial explicit argument buffer size
  Serializer::DeserialPrimitive<unsigned int>(&m_explicitArgsSizeInBytes, ist);
  // Deserial explicit argument buffer alignment
  Serializer::DeserialPrimitive<unsigned int>(&m_RequiredUniformKernelArgsAlignment, ist);

  // Deserial memory object information
  Serializer::DeserialPrimitive<unsigned int>(&vectorSize, ist);
  m_memArgs.resize(vectorSize);
  for (size_t i = 0; i < vectorSize; ++i) {
    Serializer::DeserialPrimitive<unsigned int>(&m_memArgs[i], ist);
  }

  Serializer::DeserialPointerHint((void **)&m_pProps, ist);
  if (nullptr != m_pProps) {
    m_pProps =
        stats->GetBackendFactory()->CreateKernelProperties();
    m_pProps->Deserialize(ist, stats);
  }

  Serializer::DeserialPrimitive<unsigned int>(&vectorSize, ist);
  for (unsigned int i = 0; i < vectorSize; ++i) {
    IKernelJITContainer *currentArgument = nullptr;
    Serializer::DeserialPointerHint((void **)&currentArgument, ist);
    if (nullptr != currentArgument) {
      currentArgument =
          stats->GetBackendFactory()->CreateKernelJITContainer();
      currentArgument->Deserialize(ist, stats);
    }
    m_JITs.push_back(currentArgument);
  }

}

KernelSet::KernelSet() : m_kernels(0), m_blockKernelsCount(0)
{}

KernelSet::~KernelSet() {
  for (std::vector<Kernel *>::const_iterator i = m_kernels.begin(),
                                             e = m_kernels.end();
       i != e; ++i) {
    delete *i;
  }
}

void KernelSet::AddKernel(Kernel *pKernel) {
  m_kernels.push_back(pKernel);
  m_blockKernelsCount += pKernel->GetKernelProporties()->IsBlock() ? 1 : 0;
}

Kernel *KernelSet::GetKernel(int index) const {
  if (index < 0 || index > (int)m_kernels.size()) {
    throw Exceptions::DeviceBackendExceptionBase(
        "Index OOB while accessing the kernel set");
  }
  return m_kernels[index];
}

Kernel *KernelSet::GetKernel(const char *name) const {
  for (std::vector<Kernel *>::const_iterator i = m_kernels.begin(),
                                             e = m_kernels.end();
       i != e; ++i) {
    if (!std::string((*i)->GetKernelName()).compare(name))
      return *i;
  }
  throw Exceptions::DeviceBackendExceptionBase(
      "No kernel found for given name");
}
}
}
} // namespace
