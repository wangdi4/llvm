// INTEL CONFIDENTIAL
//
// Copyright 2006 Intel Corporation.
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

#pragma once

#include "builtin_kernels.h"
#include "cl_dynamic_lib.h"
#include "cl_types.h"

#include <mkl_cblas.h>
#include <mkl_types.h>

#if defined(__OMP2TBB__)
extern "C" void __omp2tbb_set_thread_max_concurency(int max_concurency);
#endif

namespace Intel {
namespace OpenCL {
namespace MKLKernels {

template <typename> struct type_string {
  static std::string type() { return "undefined"; }
  static std::string ptr() { return "undefined"; }
  static std::string const_ptr() { return "undefined"; }
};

#define DEFINE_TYPE_STRING(T)                                                  \
  template <> struct type_string<T> {                                          \
    static std::string type() { return #T; }                                   \
    static std::string ptr() { return type() + "*"; }                          \
    static std::string const_ptr() { return string("const ") + type() + "*"; } \
  };

DEFINE_TYPE_STRING(float)
DEFINE_TYPE_STRING(double)
DEFINE_TYPE_STRING(CBLAS_ORDER)
DEFINE_TYPE_STRING(CBLAS_TRANSPOSE)

// C type to OpenCL kernel argment type conversion
template <typename> struct KernelArgumentType_of {
  static KernelArgumentType get_type();
};
template <> struct KernelArgumentType_of<float> {
  static KernelArgumentType get_type() { return KRNL_ARG_FLOAT; }
};
template <> struct KernelArgumentType_of<double> {
  static KernelArgumentType get_type() { return KRNL_ARG_DOUBLE; }
};

class MKLParamDescriptor {
public:
  MKLParamDescriptor() : m_Offset(0) {}

  class MKLParamBase {
  public:
    MKLParamBase(const char *szArgName, const char *szArgTypeName,
                 cl_kernel_arg_address_qualifier addrQual,
                 cl_kernel_arg_access_qualifier accsQual,
                 cl_kernel_arg_type_qualifier typeQual)
        : m_szArgName(szArgName), m_szArgTypeName(szArgTypeName) {
      m_ArgInfoDesc.name = m_szArgName.c_str();
      m_ArgInfoDesc.typeName = m_szArgTypeName.c_str();
      m_ArgInfoDesc.addressQualifier = addrQual;
      m_ArgInfoDesc.accessQualifier = accsQual;
      m_ArgInfoDesc.typeQualifier = typeQual;
    }

    cl_kernel_argument_info &GetArgumentInfoDescriptor() {
      return m_ArgInfoDesc;
    }
    KernelArgument &GetArgumentDescriptor() { return m_ArgDesc; }

  protected:
    KernelArgument m_ArgDesc;
    cl_kernel_argument_info m_ArgInfoDesc;

    std::string m_szArgName;
    std::string m_szArgTypeName;
  };

  template <typename paramType> class MKLParam : public MKLParamBase {
  public:
    MKLParam(const char *szArgName, const char *szArgTypeName,
             cl_kernel_arg_access_qualifier accsQual,
             cl_kernel_arg_type_qualifier typeQual, KernelArgumentType argType,
             MKLParamDescriptor *parent)
        : MKLParamBase(szArgName, szArgTypeName,
                       OpenCL::BuiltInKernels::ArgType2AddrQual(argType),
                       accsQual, typeQual),
          m_pParent(parent) {
      m_ArgDesc.OffsetInBytes = parent->m_Offset;
      parent->m_Offset += sizeof(paramType);
      m_ArgDesc.SizeInBytes = sizeof(paramType);
      m_ArgDesc.type = argType;
      parent->m_lstParams.push_back(this);
      if ((argType >= KRNL_ARG_PTR_GLOBAL) && (argType <= KRNL_ARG_PTR_CONST)) {
        // Currently only GLOBAL and CONSTANT buffers are supported
        unsigned int pramInx = parent->m_lstParams.size();
        parent->m_lstMemArgs.push_back(pramInx - 1);
      }
    }

    paramType GetValue(const void *paramBuffer) const {
      return *((paramType *)(((char *)paramBuffer) + m_ArgDesc.OffsetInBytes));
    }

    MKLParamDescriptor *m_pParent;
  };

  size_t GetParamCount() const { return m_lstParams.size(); }

  size_t GetParamSize() const { return m_Offset; }

  unsigned int GetMemoryObjectArgumentCount() const {
    return m_lstMemArgs.size();
  }
  const unsigned int *GetMemoryObjectArgumentIndexes() const {
    return m_lstMemArgs.size() > 0 ? &m_lstMemArgs[0] : nullptr;
  }

protected:
  size_t m_Offset; // Offset inside argument buffer

  std::vector<unsigned int>
      m_lstMemArgs; // List of indexes for memory arguments
  std::vector<MKLParamBase *> m_lstParams; // List of kernel arguments
};

template <class MKL_EXECUTOR_CLASS>
class MKLKernel : public Intel::OpenCL::BuiltInKernels::IBuiltInKernel {
public:
  MKLKernel(const char *szName, ptrdiff_t pFuncPtr)
      : m_szFuncName(szName), m_pMKLFuncPtr(pFuncPtr) {}

#ifndef __OMP2TBB__
  cl_dev_err_code
  Execute(const void *pParamBuffer,
          Intel::OpenCL::BuiltInKernels::OMPExecutorThread *pThread) const {
    MKL_EXECUTOR_CLASS executor(m_pMKLFuncPtr, pParamBuffer);
    return pThread->Execute(executor);
  }
#else
  cl_dev_err_code Execute(const Intel::OpenCL::TaskExecutor::ITaskList *pList,
                          const void *pParamBuffer) const {
    // Set concurrency for the library, this is a workaround until TBB will
    // return a pointer to current arena Currently we assuming that
    // master/application thread is joining.
    // TODO: need to recalculate number of active workers

    int iDeviceConcurency = pList->GetDeviceConcurency();
    bool bMasterJoinedExecution = pList->IsMasterJoined();
    bool bCanMasterJoin = pList->CanMasterJoin();

    // If master can join, but currently it doesn't we need execute on less
    // threads
    if (bCanMasterJoin && !bMasterJoinedExecution) {
      --iDeviceConcurency;
    }

    __omp2tbb_set_thread_max_concurency(iDeviceConcurency);

    MKL_EXECUTOR_CLASS executor(m_pMKLFuncPtr, pParamBuffer);
    return executor.Execute();
  }
#endif

  // ICLDevBackendKernel
  unsigned long long int GetKernelID() const {
    return (unsigned long long int)this;
  }
  const char *GetKernelName() const { return m_szFuncName.c_str(); }
  int GetKernelParamsCount() const {
    return (
        int)MKL_EXECUTOR_CLASS::MKL_GEMM_EXECUTOR_PAREMERTERS::GetParamCount();
  }
  const KernelArgument *GetKernelParams() const {
    return MKL_EXECUTOR_CLASS::MKL_GEMM_EXECUTOR_PAREMERTERS::GetKernelParams();
  }
  const cl_kernel_argument_info *GetKernelArgInfo() const {
    return MKL_EXECUTOR_CLASS::MKL_GEMM_EXECUTOR_PAREMERTERS::
        GetKernelArgInfo();
  }
  size_t GetExplicitArgumentBufferSize(void) const {
    return MKL_EXECUTOR_CLASS::MKL_GEMM_EXECUTOR_PAREMERTERS::GetParamSize();
  }
  size_t GetArgumentBufferRequiredAlignment(void) const {
    return MKL_EXECUTOR_CLASS::MKL_GEMM_EXECUTOR_PAREMERTERS::GetKernelParams()
        [0]
            .SizeInBytes;
  }
  const Intel::OpenCL::DeviceBackend::ICLDevBackendKernelRunner *
  GetKernelRunner(void) const {
    return nullptr;
  }

  int GetLineNumber(void *pointer) const { return -1; }

  size_t GetArgumentBufferSize() const {
    return MKL_EXECUTOR_CLASS::MKL_GEMM_EXECUTOR_PAREMERTERS::GetParamSize();
  }
  unsigned int GetMemoryObjectArgumentCount() const {
    return MKL_EXECUTOR_CLASS::MKL_GEMM_EXECUTOR_PAREMERTERS::
        GetMemoryObjectArgumentCount();
  }
  const unsigned int *GetMemoryObjectArgumentIndexes() const {
    return MKL_EXECUTOR_CLASS::MKL_GEMM_EXECUTOR_PAREMERTERS::
        GetMemoryObjectArgumentIndexes();
  }

protected:
  std::string m_szFuncName;
  ptrdiff_t m_pMKLFuncPtr;
};

} // namespace MKLKernels
} // namespace OpenCL
} // namespace Intel
