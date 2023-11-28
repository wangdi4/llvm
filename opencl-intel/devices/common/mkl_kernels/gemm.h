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

#include "mkl_kernels.h"

namespace Intel {
namespace OpenCL {
namespace MKLKernels {
/*
 * ===========================================================================
 * Prototypes for level 3 BLAS
 * ===========================================================================
 */
template <typename datatype>
class MKL_GEMM_Parameters : public MKLParamDescriptor {
public:
  MKL_GEMM_Parameters()
      : order("order", type_string<CBLAS_ORDER>::type().c_str(),
              CL_KERNEL_ARG_ACCESS_NONE, CL_KERNEL_ARG_TYPE_CONST, KRNL_ARG_INT,
              this),
        transA("transA", type_string<CBLAS_TRANSPOSE>::type().c_str(),
               CL_KERNEL_ARG_ACCESS_NONE, CL_KERNEL_ARG_TYPE_CONST,
               KRNL_ARG_INT, this),
        pABuffer("pA", type_string<datatype>::const_ptr().c_str(),
                 CL_KERNEL_ARG_ACCESS_NONE, CL_KERNEL_ARG_TYPE_CONST,
                 KRNL_ARG_PTR_GLOBAL, this),
        transB("transB", type_string<CBLAS_TRANSPOSE>::type().c_str(),
               CL_KERNEL_ARG_ACCESS_NONE, CL_KERNEL_ARG_TYPE_CONST,
               KRNL_ARG_INT, this),
        pBBuffer("pB", type_string<datatype>::const_ptr().c_str(),
                 CL_KERNEL_ARG_ACCESS_NONE, CL_KERNEL_ARG_TYPE_CONST,
                 KRNL_ARG_PTR_GLOBAL, this),
        pCBuffer("pC", type_string<datatype>::ptr().c_str(),
                 CL_KERNEL_ARG_ACCESS_NONE, CL_KERNEL_ARG_TYPE_NONE,
                 KRNL_ARG_PTR_GLOBAL, this),
        alpha("alpha", type_string<datatype>::type().c_str(),
              CL_KERNEL_ARG_ACCESS_NONE, CL_KERNEL_ARG_TYPE_CONST,
              cl_kernel_arg_type_of<datatype>::get_type(), this),
        beta("beta", type_string<datatype>::type().c_str(),
             CL_KERNEL_ARG_ACCESS_NONE, CL_KERNEL_ARG_TYPE_CONST,
             cl_kernel_arg_type_of<datatype>::get_type(), this),
        pArgDescriptor(nullptr), pArgInfoDescriptor(nullptr) {
    size_t paramCount = GetParamCount();
    pArgDescriptor = new KernelArgument[paramCount];
    pArgInfoDescriptor = new cl_kernel_argument_info[paramCount];
    for (size_t i = 0; i < paramCount; ++i) {
      MEMCPY_S(&pArgDescriptor[i], sizeof(KernelArgument),
               &(m_lstParams.at(i)->GetArgumentDescriptor()),
               sizeof(KernelArgument));
      MEMCPY_S(&pArgInfoDescriptor[i], sizeof(cl_kernel_argument_info),
               &(m_lstParams.at(i)->GetArgumentInfoDescriptor()),
               sizeof(cl_kernel_argument_info));
    }
  }

  ~MKL_GEMM_Parameters() {
    if (nullptr != pArgDescriptor) {
      delete[] pArgDescriptor;
    }
    if (nullptr != pArgInfoDescriptor) {
      delete[] pArgInfoDescriptor;
    }
  }

  // Static information info
  static size_t GetParamCount() { return s_Params.m_lstParams.size(); }
  static size_t GetParamSize() { return s_Params.m_Offset; }
  static const KernelArgument *GetKernelParams() {
    return s_Params.pArgDescriptor;
  }
  static const cl_kernel_argument_info *GetKernelArgInfo() {
    return s_Params.pArgInfoDescriptor;
  }

  static unsigned int GetMemoryObjectArgumentCount() {
    return s_Params.m_lstMemArgs.size();
  }
  static const unsigned int *GetMemoryObjectArgumentIndexes() {
    return s_Params.m_lstMemArgs.size() > 0 ? &s_Params.m_lstMemArgs[0]
                                            : nullptr;
  }

  static const MKLParam<CBLAS_ORDER> &GetOrder() { return s_Params.order; }
  static const MKLParam<CBLAS_TRANSPOSE> &GetTransA() {
    return s_Params.transA;
  }
  static const MKLParam<const datatype *> &GetParamA() {
    return s_Params.pABuffer;
  }
  static const MKLParam<CBLAS_TRANSPOSE> &GetTransB() {
    return s_Params.transB;
  }
  static const MKLParam<const datatype *> &GetParamB() {
    return s_Params.pBBuffer;
  }
  static const MKLParam<datatype *> &GetParamC() { return s_Params.pCBuffer; }
  static const MKLParam<datatype> &GetAlpha() { return s_Params.alpha; }
  static const MKLParam<datatype> &GetBeta() { return s_Params.beta; }

protected:
  MKLParam<CBLAS_ORDER> order;
  MKLParam<CBLAS_TRANSPOSE> transA;
  MKLParam<const datatype *> pABuffer;
  MKLParam<CBLAS_TRANSPOSE> transB;
  MKLParam<const datatype *> pBBuffer;
  MKLParam<datatype *> pCBuffer;
  MKLParam<datatype> alpha;
  MKLParam<datatype> beta;

  KernelArgument *pArgDescriptor;
  cl_kernel_argument_info *pArgInfoDescriptor;

  static const MKL_GEMM_Parameters<datatype> s_Params;
};

template <typename datatype>
class MKL_GEMM_Executor
    : public Intel::OpenCL::BuiltInKernels::IBuiltInKernelExecutor {
public:
  typedef void (*MKL_FuncType)(const CBLAS_ORDER Order,
                               const CBLAS_TRANSPOSE TransA,
                               const CBLAS_TRANSPOSE TransB, const MKL_INT M,
                               const MKL_INT N, const MKL_INT K,
                               const datatype alpha, const datatype *A,
                               const MKL_INT lda, const datatype *B,
                               const MKL_INT ldb, const datatype beta,
                               datatype *C, const MKL_INT ldc);

  typedef MKL_GEMM_Parameters<datatype> MKL_GEMM_EXECUTOR_PAREMERTERS;

  MKL_GEMM_Executor(ptrdiff_t func_ptr, const void *params)
      : m_FuncPtr((MKL_FuncType)func_ptr), m_pParamBuffer(params) {
    const UniformKernelArgs *pKernelArgs =
        (const UniformKernelArgs
             *)((const char *)params +
                MKL_GEMM_EXECUTOR_PAREMERTERS::GetParamSize());

    m_iRowsA_M = (int)pKernelArgs->GlobalSize[0];
    m_iColsB_N = (int)pKernelArgs->GlobalSize[1];
    m_iK = (int)pKernelArgs->GlobalSize[2];
  }

  cl_dev_err_code Execute() const {
    CBLAS_ORDER order =
        (CBLAS_ORDER)MKL_GEMM_EXECUTOR_PAREMERTERS::GetOrder().GetValue(
            m_pParamBuffer);
    CBLAS_TRANSPOSE transA =
        (CBLAS_TRANSPOSE)MKL_GEMM_EXECUTOR_PAREMERTERS::GetTransA().GetValue(
            m_pParamBuffer);
    const datatype *pA =
        (const datatype *)MKL_GEMM_EXECUTOR_PAREMERTERS::GetParamA().GetValue(
            m_pParamBuffer);
    CBLAS_TRANSPOSE transB =
        (CBLAS_TRANSPOSE)MKL_GEMM_EXECUTOR_PAREMERTERS::GetTransB().GetValue(
            m_pParamBuffer);
    const datatype *pB =
        (const datatype *)MKL_GEMM_EXECUTOR_PAREMERTERS::GetParamB().GetValue(
            m_pParamBuffer);
    datatype *pC =
        (datatype *)MKL_GEMM_EXECUTOR_PAREMERTERS::GetParamC().GetValue(
            m_pParamBuffer);
    datatype alpha =
        (datatype)MKL_GEMM_EXECUTOR_PAREMERTERS::GetAlpha().GetValue(
            m_pParamBuffer);
    datatype beta = (datatype)MKL_GEMM_EXECUTOR_PAREMERTERS::GetBeta().GetValue(
        m_pParamBuffer);

    // Execute parallel(OpenMP) MKL function
    m_FuncPtr(order, transA, transB, m_iRowsA_M, m_iColsB_N, m_iK, alpha, pA,
              m_iK, pB, m_iColsB_N, beta, pC, m_iColsB_N);

    return CL_DEV_SUCCESS;
  }

  cl_dev_err_code GetLastError() const { return m_lastError; }

protected:
  int m_iRowsA_M;
  int m_iColsB_N;
  int m_iK;

  MKL_FuncType m_FuncPtr;
  const void *m_pParamBuffer;

  cl_dev_err_code m_lastError;
};

// This class is used to expose MKL function on MIC device
template <typename datatype>
class MKL_GEMM_Executor_Proxy
    : public Intel::OpenCL::BuiltInKernels::IBuiltInKernelExecutor {
public:
  MKL_GEMM_Executor_Proxy(ptrdiff_t func_ptr, const void *params) {}

  typedef MKL_GEMM_Parameters<datatype> MKL_GEMM_EXECUTOR_PAREMERTERS;

  cl_dev_err_code Execute() const { return CL_DEV_SUCCESS; }
};

} // namespace MKLKernels
} // namespace OpenCL
} // namespace Intel
