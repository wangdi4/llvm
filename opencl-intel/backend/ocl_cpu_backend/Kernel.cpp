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

File Name:  Kernel.cpp

\*****************************************************************************/

#include "Kernel.h"
#include "KernelProperties.h"
#include "ImplicitArgsUtils.h"
#include "TypeAlignment.h"
#include "exceptions.h"
#include "cpu_dev_limits.h"

#include <cstring>
#include <cmath>
#include <iostream>
#include <sstream>
#include <stddef.h>
#include <stdio.h>

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
      m_memArgs(memArgs), m_pProps(pProps) {
  if (!m_explicitArgs.empty()) {
    // calculates the whole explicit arguments buffer size
    // offset of the last argument in the buffer + argumentSize
    // and adjust alignment
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
  m_RequiredUniformKernelArgsAlignment = MinRequiredKernelArgAlignment;
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

void Kernel::CreateWorkDescription(
    cl_uniform_kernel_args *UniformImplicitArgs) const {
  // assumption: LocalWorkSize GlobalSize and minWorkGroup already initialized

  bool UseAutoGroupSize = true;
  for (unsigned int i = 0; i < UniformImplicitArgs->WorkDim; ++i) {
    UseAutoGroupSize = UseAutoGroupSize && ((UniformImplicitArgs->LocalSize[i]) == 0);
  }

  if (UseAutoGroupSize) {
    // Try hint size, find GCD for each dimension
    if (m_pProps->GetHintWGSize()[0] != 0) {
      for (unsigned int i = 0; i < UniformImplicitArgs->WorkDim; ++i) {
        UniformImplicitArgs->LocalSize[i] = GCD(
            UniformImplicitArgs->GlobalSize[i], m_pProps->GetHintWGSize()[i]);
      }
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
      unsigned int globalWorkSizeYZ = 1;
      for (unsigned int i = 1; i < UniformImplicitArgs->WorkDim; ++i) {
        // Calculate global group size on dimensions Y & Z
        globalWorkSizeYZ *= UniformImplicitArgs->GlobalSize[i];
        // Set local group size on dimensions Y & Z to 1
        UniformImplicitArgs->LocalSize[i] = 1;
      }

      const unsigned int kernelPrivateMemSize =
          (unsigned int)m_pProps->GetPrivateMemorySize();
      unsigned int globalWorkSizeX = UniformImplicitArgs->GlobalSize[0];
      unsigned int localSizeUpperLimit = min(
          min(CPU_MAX_WORK_GROUP_SIZE, globalWorkSizeX), // localSizeMaxLimit_1
          CPU_DEV_MAX_WG_PRIVATE_SIZE / (kernelPrivateMemSize > 0
                                             ? kernelPrivateMemSize
                                             : 1)); // localSizeMaxLimit_2

      unsigned int minMultiplyFactor = m_pProps->GetMinGroupSizeFactorial();
      assert(minMultiplyFactor &&
             (minMultiplyFactor & (minMultiplyFactor - 1)) == 0 &&
             "minMultiplyFactor assumed to be power of 2 that is not zero!");
      assert(
          (!m_pProps->GetJitCreateWIids() ||
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
        } else {
          //In this case we have few work-items, try satisfy as much as possible of work threads.
          const unsigned int workGroupNumMinLimit = (workThreadUtils + (globalWorkSizeYZ-1)) / globalWorkSizeYZ;
          localSizeMaxLimit = max(1, localSizeMaxLimit / workGroupNumMinLimit);
        }
        assert(localSizeMaxLimit <= globalWorkSizeX && "global size in dim X must be upper bound for local size");
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
          assert((workGroups << simdUtilsLog) >= (newHeuristic * workThreadUtils) && "Wrong balance factor calculation!");
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
      UniformImplicitArgs->LocalSize[0] = newHeuristic;
      assert(UniformImplicitArgs->LocalSize[0] > 0 &&
             "local size must be positive number");
    }
  }
}

unsigned long long int Kernel::GetKernelID() const { return 0; }

const char *Kernel::GetKernelName() const { return m_name.c_str(); }

int Kernel::GetKernelParamsCount() const { return m_explicitArgs.size(); }

const cl_kernel_argument *Kernel::GetKernelParams() const {
  return m_explicitArgs.empty() ? NULL : &m_explicitArgs[0];
}

size_t Kernel::GetArgumentBufferRequiredAlignment() const {
  return m_RequiredUniformKernelArgsAlignment;
}

const cl_kernel_argument_info *Kernel::GetKernelArgInfo() const { return NULL; }

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
  return m_memArgs.empty() ? NULL : &m_memArgs[0];
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

  // need to fill the WI ids buffer
  if (pKernelUniformImplicitArgs->LocalIDIndicesRequiredSize > 0) {
    ImplicitArgsUtils::initWILocalIds(
        pKernelUniformImplicitArgs->WorkDim,
        pKernelUniformImplicitArgs->LocalSize,
        pKernelUniformImplicitArgs->VectorWidth,
        pKernelUniformImplicitArgs->pLocalIDIndices);
  }
  const void *pEntryMark = pKernelUniformImplicitArgs->pJITEntryPoint;
  pKernelUniformImplicitArgs->pJITEntryPoint =
      ResolveEntryPointHandle(pEntryMark);

  if (false) {
    std::stringstream ss;
    ss << "In Initrunner:\n";
    ss << "pKernelUniformArgs = " << pKernelUniformArgs << "\n";
    DebugPrintUniformKernelArgs(
        pKernelUniformImplicitArgs,
        m_explicitArgsSizeInBytes,
        ss);
    std::cout << ss.str() << std::endl;
  }
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
                               unsigned int devMemObjArrayLength) const {
  assert(pKernelUniformArgs && "Uniform Arguments Pointer is null");
  void *pKernelUniformImplicitArgsPosition =
      (char *)pKernelUniformArgs + m_explicitArgsSizeInBytes;
  cl_uniform_kernel_args *pKernelUniformImplicitArgs =
      static_cast<cl_uniform_kernel_args *>(pKernelUniformImplicitArgsPosition);

  CreateWorkDescription(pKernelUniformImplicitArgs);

  // local cannot be zero at this point
  assert(pKernelUniformImplicitArgs->LocalSize[0] != 0 &&
         "LocalSize must be properly initialized");

  // In case of (pKernelUniformImplicitArgs.WorkDim < MAX_WORK_DIM)
  //  need to set local and global size to 1 for OOB dimensions
  for (unsigned int i = pKernelUniformImplicitArgs->WorkDim; i < MAX_WORK_DIM;
       ++i) {
    pKernelUniformImplicitArgs->GlobalSize[i] = 1;
    pKernelUniformImplicitArgs->LocalSize[i] = 1;
    pKernelUniformImplicitArgs->GlobalOffset[i] = 0;
    // no need to set GlobalOffset as it is used later only according
    // to dimension
  }

  // Calculate number of work groups and WG size
  size_t WGSize = 1;
  for (unsigned int i = 0; i < pKernelUniformImplicitArgs->WorkDim; ++i) {
    assert(pKernelUniformImplicitArgs->GlobalSize[i] % pKernelUniformImplicitArgs->LocalSize[i] == 0);
    pKernelUniformImplicitArgs->WGCount[i] =
        pKernelUniformImplicitArgs->GlobalSize[i] /
        pKernelUniformImplicitArgs->LocalSize[i];
    WGSize *= pKernelUniformImplicitArgs->LocalSize[i];
  }

  // need to decide which entrypoint to run
  const IKernelJITContainer *pScalarJIT = GetKernelJIT(0);
  if (m_pProps->GetJitCreateWIids()) {
    // vectorized kernel is inlined into scalar kernel,
    // so we have exactly one JIT function.
    pKernelUniformImplicitArgs->pJITEntryPoint =
        CreateEntryPointHandle(pScalarJIT->GetJITCode());
    pKernelUniformImplicitArgs->VectorWidth =
        pScalarJIT->GetProps()->GetVectorSize();
  } else {
    const IKernelJITContainer *pVectorJIT =
        GetKernelJITCount() > 1 ? GetKernelJIT(1) : NULL;
    // vectorized and scalar kernels could both be present
    if (NULL != pVectorJIT) {
      pKernelUniformImplicitArgs->pJITEntryPoint =
          CreateEntryPointHandle(pVectorJIT->GetJITCode());
      pKernelUniformImplicitArgs->VectorWidth =
          pVectorJIT->GetProps()->GetVectorSize();
    } else {
      pKernelUniformImplicitArgs->pJITEntryPoint =
          CreateEntryPointHandle(pScalarJIT->GetJITCode());
      pKernelUniformImplicitArgs->VectorWidth =
          pScalarJIT->GetProps()->GetVectorSize();
    }

    if (pKernelUniformImplicitArgs->LocalSize[0] %
        pKernelUniformImplicitArgs->VectorWidth) {
      // Disable vectorization for workgroup sizes that are not
      // a multiple of the vector width (Guy)
      pKernelUniformImplicitArgs->pJITEntryPoint =
          CreateEntryPointHandle(pScalarJIT->GetJITCode());
      pKernelUniformImplicitArgs->VectorWidth =
          pScalarJIT->GetProps()->GetVectorSize();
    }
    assert(!(1 != pKernelUniformImplicitArgs->VectorWidth && 1 == WGSize) &&
           "vectorized with WGsize = 1!");
  }

  pKernelUniformImplicitArgs->LocalIDIndicesRequiredSize = 0;
  // if the JIT creates the WI ids buffer then no need to ask it from the RT
  if (!m_pProps->GetJitCreateWIids()) {
    WGSize = WGSize / pKernelUniformImplicitArgs->VectorWidth;
    //TODO: usage of sizeof(size_t) breaks cross-compilation
    pKernelUniformImplicitArgs->LocalIDIndicesRequiredSize =
        ADJUST_SIZE_TO_MAXIMUM_ALIGN(WGSize * sizeof(size_t) *
                                     MAX_WI_DIM_POW_OF_2);

    pKernelUniformImplicitArgs->WGLoopIterCount = WGSize - 1;
  }

  if (true ) { // ocl20 
    pKernelUniformImplicitArgs->Block2KernelMapper = m_RuntimeService->GetBlockToKernelMapper();
  }

#ifdef OCL_DEV_BACKEND_PLUGINS
// TODO: Notify the plugin manager
// m_pluginManager.OnCreateBinary( pKernel,
//                                pWorkDescription,
//                                contextSize,
//                                pContext);
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
     << O + offsetof(cl_uniform_kernel_args, LocalSize) << ": size_t LocalSize[MAX_WORK_DIM]" << PRINT3(A->LocalSize) << "\n"
     << O + offsetof(cl_uniform_kernel_args, WGCount) << ": size_t WGCount[MAX_WORK_DIM]" << PRINT3(A->WGCount) << "\n"
     << O + offsetof(cl_uniform_kernel_args, WGLoopIterCount) << ": size_t WGLoopIterCount= " << A->WGLoopIterCount << "\n"
     << O + offsetof(cl_uniform_kernel_args, pLocalIDIndices) << ": size_t* pLocalIDIndices = " << A->pLocalIDIndices;
  if (A->LocalIDIndicesRequiredSize) {
    for (unsigned I = 0;
         I < std::min(A->LocalIDIndicesRequiredSize / sizeof(size_t),
                      size_t(32 * 4));
         I += 4) {
      unsigned J = 0;
      for (; J < A->WorkDim; ++J)
        ss << ", " << A->pLocalIDIndices[I];
      for (; J < 3; ++J)
        ss << ".";
    }
  }
  ss << "\n" 
     << O + offsetof(cl_uniform_kernel_args, RuntimeInterface)
     << ": void* RuntimeInterface= " << A->RuntimeInterface << "\n" 
     << O + offsetof(cl_uniform_kernel_args, minWorkGroupNum) << ": size_t minWorkGroupNum= " << A->minWorkGroupNum << "\n"
      // ss << O+offsetof(cl_uniform_kernel_args, RuntimeCallBacks) << ": void*
      // RuntimeCallBacks = " << A->pRuntimeContext << "\n";
     << O + offsetof(cl_uniform_kernel_args, LocalIDIndicesRequiredSize) << ": size_t LocalIDIndicesRequiredSize = " << A->LocalIDIndicesRequiredSize << "\n"
     << O + offsetof(cl_uniform_kernel_args, pJITEntryPoint) << ": void* pJITEntryPoint = " << A->pJITEntryPoint << "\n"
     << O + offsetof(cl_uniform_kernel_args, VectorWidth) << ": unsigned VectorWidth = " << A->VectorWidth << "\n";
#undef PRINT3
}

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

#if defined(ENABLE_SDE)
  BeforeExecution();
#endif

  assert(pKernelUniformImplicitArgs->WorkDim < 4);
  assert(pKernelUniformImplicitArgs->WorkDim > 0);
  static bool guard = true;
  if (false && guard) {
    guard = false; //Print only first group in NDRange to avoid huge dumps
    std::stringstream ss;
    ss << "GroupID: " << pGroupID[0] << ", " << pGroupID[1] << ", " << pGroupID[2] << "\n"
       << "pKernelUniformArgs = " << pKernelUniformArgs << "\n";
    DebugPrintUniformKernelArgs(pKernelUniformImplicitArgs,
                                m_explicitArgsSizeInBytes, ss);
    std::cout << ss.str() << std::endl;
  }

  IKernelJITContainer::JIT_PTR *kernel =
      (IKernelJITContainer::JIT_PTR *)(size_t)
      pKernelUniformImplicitArgs->pJITEntryPoint;

  // running the kernel with the specified args and (groupID, runtimeHandle)
  kernel(pKernelUniformArgs, pGroupID, pRuntimeHandle);

#if defined(ENABLE_SDE)
  AfterExecution();
#endif
  return CL_DEV_SUCCESS;
}

cl_dev_err_code Kernel::RestoreThreadState(ICLDevExecutionState &state) const {
  _mm_setcsr(state.MXCSRstate);
  return CL_DEV_SUCCESS;
}

KernelSet::~KernelSet() {
  for (std::vector<Kernel *>::const_iterator i = m_kernels.begin(),
                                             e = m_kernels.end();
       i != e; ++i) {
    delete *i;
  }
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

