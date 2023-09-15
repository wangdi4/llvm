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

///////////////////////////////////////////////////////////
//  mkl_kernels.cpp
///////////////////////////////////////////////////////////

#include "mkl_kernels.h"
#include "cl_dynamic_lib.h"
#include "export/mkl_builtins.h"
#include "gemm.h"

#ifdef __OMP2TBB__
extern "C" void __kmpc_begin(void *loc, int flags);
extern "C" void __kmpc_end(void *loc);
#endif

using namespace Intel::OpenCL::MKLKernels;

/////////////////////////////////////////////////////////////////////////////
// Create registration classes for MKL functions
#define REGISTER_MKL_FUNCTION(MKL_FUNCTION_NAME, MKL_CLASS_TYPE, DATA_TYPE)    \
  template <>                                                                  \
  const MKL_##MKL_CLASS_TYPE##_Parameters<DATA_TYPE>                           \
      MKL_##MKL_CLASS_TYPE##_Parameters<DATA_TYPE>::s_Params =                 \
          MKL_##MKL_CLASS_TYPE##_Parameters<DATA_TYPE>();

#include "mkl_kernels.inc"

#undef REGISTER_MKL_FUNCTION

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////

#ifndef __MIC__
// Intel::OpenCL::Utils::OclDynamicLib g_mklRT;
#endif

namespace Intel {
namespace OpenCL {
namespace MKLKernels {

template <bool useFunctions> bool InitLibrary();

#if defined(_WIN32)
#define GET_MKL_FUNCTION_PTR(NAME) ((ptrdiff_t)NAME)
#elif defined(__MIC__)
#define GET_MKL_FUNCTION_PTR(NAME) ((ptrdiff_t)NAME)
#else
#include <dlfcn.h>
// #define GET_MKL_FUNCTION_PTR(NAME)
//  Intel::OpenCL::Utils::OclDynamicLib::GetFuntionPtrByNameFromHandle(RTLD_DEFAULT,
// #NAME)
#define GET_MKL_FUNCTION_PTR(NAME) ((ptrdiff_t)NAME)
// #define GET_MKL_FUNCTION_PTR(NAME) g_mklRT.GetFunctionPtrByName(#NAME)
#endif

#define REGISTER_MKL_FUNCTION(MKL_FUNCTION_NAME, MKL_CLASS_TYPE, DATA_TYPE)    \
  struct MKL_FUNCTION_NAME##CreatorClass {                                     \
    static cl_dev_err_code MKL_FUNCTION_NAME##Creator(                         \
        Intel::OpenCL::BuiltInKernels::IBuiltInKernel **ppBIKernel) {          \
      ptrdiff_t pFunc = GET_MKL_FUNCTION_PTR(MKL_FUNCTION_NAME);               \
      if (nullptr == (void *)pFunc)                                            \
        return CL_DEV_NOT_SUPPORTED;                                           \
      *ppBIKernel = new MKLKernel<MKL_##MKL_CLASS_TYPE##_Executor<DATA_TYPE>>( \
          #MKL_FUNCTION_NAME, pFunc);                                          \
      return CL_DEV_SUCCESS;                                                   \
    }                                                                          \
  };                                                                           \
  REGISTER_BUILTIN_KERNEL(                                                     \
      MKL_FUNCTION_NAME,                                                       \
      MKL_FUNCTION_NAME##CreatorClass::MKL_FUNCTION_NAME##Creator)

template <> bool InitLibrary<true>() {
#ifdef __OMP2TBB__
  // Required to initialize OpenMP layer. In original OpenMP environment this
  // code is added by linker
  __kmpc_begin(nullptr, 0);
#endif

#if 0
    // Check if MKL library in the system path
#ifdef _WIN32
    if (g_mklRT.Load("mkl_intel_ilp64.dll") != 0)
#else
    if (g_mklRT.Load("libmkl_intel_ilp64.so") != 0 )
#endif
    {
        return false;
    }

    g_mklRT.Close();
#endif

// Import set of exposed MKL functions
#include "../mkl_kernels.inc"
  return true;
}
#undef REGISTER_MKL_FUNCTION

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
// Here we want to register only proxy's to remote device such as MIC
#define REGISTER_MKL_FUNCTION(MKL_FUNCTION_NAME, MKL_CLASS_TYPE, DATA_TYPE)    \
  struct MKL_FUNCTION_NAME##CreatorClassProxy {                                \
    static cl_dev_err_code MKL_FUNCTION_NAME##Creator(                         \
        Intel::OpenCL::BuiltInKernels::IBuiltInKernel **ppBIKernel) {          \
      *ppBIKernel =                                                            \
          new MKLKernel<MKL_##MKL_CLASS_TYPE##_Executor_Proxy<DATA_TYPE>>(     \
              #MKL_FUNCTION_NAME, nullptr);                                    \
      return CL_DEV_SUCCESS;                                                   \
    }                                                                          \
  };                                                                           \
  REGISTER_BUILTIN_KERNEL(                                                     \
      MKL_FUNCTION_NAME,                                                       \
      MKL_FUNCTION_NAME##CreatorClassProxy::MKL_FUNCTION_NAME##Creator)

template <> bool InitLibrary<false>() {
// Import set of exposed MKL functions
#include "../mkl_kernels.inc"

  return true;
}

#undef REGISTER_MKL_FUNCTION

template <> void ReleaseLibrary<true>() {
#ifdef __OMP2TBB__
  // Required to shutdown OpenMP layer. In original OpenMP environment this code
  // is added by linker
  __kmpc_end(nullptr);
#endif
}

template <> void ReleaseLibrary<false>() {}

} // namespace MKLKernels
} // namespace OpenCL
} // namespace Intel
