// Copyright (c) 2006-2014 Intel Corporation
// All rights reserved.
//
// WARRANTY DISCLAIMER
//
// THESE MATERIALS ARE PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL INTEL OR ITS
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THESE
// MATERIALS, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Intel Corporation is the author of the Materials, and requests that all
// problem reports or change requests be submitted to it directly

///////////////////////////////////////////////////////////
//  mkl_kernels.cpp
///////////////////////////////////////////////////////////


#include "export/mkl_builtins.h"
#include <cl_dynamic_lib.h>

#include "mkl_kernels.h"
#include "gemm.h"

#ifdef __OMP2TBB__
extern "C" void __kmpc_begin(void* loc, int flags);
extern "C" void __kmpc_end(void* loc);
#endif

using namespace Intel::OpenCL::MKLKernels;

/////////////////////////////////////////////////////////////////////////////
// Create registration classes for MKL functions
#define REGISTER_MKL_FUNCTION(MKL_FUNCTION_NAME,MKL_CLASS_TYPE,DATA_TYPE) \
    template<> const MKL_##MKL_CLASS_TYPE##_Parameters<DATA_TYPE> MKL_##MKL_CLASS_TYPE##_Parameters<DATA_TYPE>::s_Params = MKL_##MKL_CLASS_TYPE##_Parameters<DATA_TYPE>();

#include "mkl_kernels.inc"

#undef REGISTER_MKL_FUNCTION

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////

#ifndef __MIC__
//Intel::OpenCL::Utils::OclDynamicLib g_mklRT;
#endif

namespace Intel { namespace OpenCL { namespace MKLKernels {

template< bool useFunctions > bool InitLibrary();

#if defined(_WIN32)
#define GET_MKL_FUNCTION_PTR(NAME) ((ptrdiff_t)NAME)
#elif defined(__MIC__)
#define GET_MKL_FUNCTION_PTR(NAME) ((ptrdiff_t)NAME)
#else
#include <dlfcn.h>
//#define GET_MKL_FUNCTION_PTR(NAME) Intel::OpenCL::Utils::OclDynamicLib::GetFuntionPtrByNameFromHandle(RTLD_DEFAULT, #NAME)
#define GET_MKL_FUNCTION_PTR(NAME) ((ptrdiff_t)NAME)
//#define GET_MKL_FUNCTION_PTR(NAME) g_mklRT.GetFunctionPtrByName(#NAME)
#endif

#define REGISTER_MKL_FUNCTION(MKL_FUNCTION_NAME,MKL_CLASS_TYPE,DATA_TYPE) \
    struct MKL_FUNCTION_NAME##CreatorClass\
    {\
        static cl_dev_err_code MKL_FUNCTION_NAME##Creator(Intel::OpenCL::BuiltInKernels::IBuiltInKernel* *ppBIKernel)\
        {\
            ptrdiff_t pFunc = GET_MKL_FUNCTION_PTR(MKL_FUNCTION_NAME);\
            if ( nullptr == (void*)pFunc ) return CL_DEV_NOT_SUPPORTED;\
            *ppBIKernel = new MKLKernel< MKL_##MKL_CLASS_TYPE##_Executor<DATA_TYPE > >(#MKL_FUNCTION_NAME, pFunc);\
            return CL_DEV_SUCCESS;\
        }\
    };\
    REGISTER_BUILTIN_KERNEL(MKL_FUNCTION_NAME, MKL_FUNCTION_NAME##CreatorClass::MKL_FUNCTION_NAME##Creator)

template<> bool InitLibrary<true>()
{
#ifdef __OMP2TBB__
    // Required to initialize OpenMP layer. In original OpenMP environment this code is added by linker
    __kmpc_begin(nullptr,0);
#endif

#if 0
    // Check if MKL library in the system path
#ifdef WIN32
    if ( !g_mklRT.Load("mkl_intel_ilp64.dll") )
#else
    if ( !g_mklRT.Load("libmkl_intel_ilp64.so") )
#endif
    {
        return false;
    }

    g_mklRT.Close();
#endif

    // Import set of exposed MKL functions
    #include"../mkl_kernels.inc"
    return true;
}
#undef REGISTER_MKL_FUNCTION

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
// Here we want to register only proxy's to remote device such as MIC
#define REGISTER_MKL_FUNCTION(MKL_FUNCTION_NAME,MKL_CLASS_TYPE,DATA_TYPE) \
    struct MKL_FUNCTION_NAME##CreatorClassProxy\
    {\
        static cl_dev_err_code MKL_FUNCTION_NAME##Creator(Intel::OpenCL::BuiltInKernels::IBuiltInKernel* *ppBIKernel)\
        {\
            *ppBIKernel = new MKLKernel< MKL_##MKL_CLASS_TYPE##_Executor_Proxy<DATA_TYPE > >(#MKL_FUNCTION_NAME, nullptr);\
            return CL_DEV_SUCCESS;\
        }\
    };\
    REGISTER_BUILTIN_KERNEL(MKL_FUNCTION_NAME, MKL_FUNCTION_NAME##CreatorClassProxy::MKL_FUNCTION_NAME##Creator)

template<> bool InitLibrary<false>()
{
    // Import set of exposed MKL functions
    #include"../mkl_kernels.inc"

    return true;
}

#undef REGISTER_MKL_FUNCTION

template<> void ReleaseLibrary<true>()
{
#ifdef __OMP2TBB__
    // Required to shutdown OpenMP layer. In original OpenMP environment this code is added by linker
    __kmpc_end(nullptr);
#endif
}

template<> void ReleaseLibrary<false>()
{
}

}}}
