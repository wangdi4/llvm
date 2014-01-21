// Copyright (c) 2006-2012 Intel Corporation
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
#pragma once

#include "mkl_kernels.h"

namespace Intel { namespace OpenCL { namespace MKLKernels {
/*
 * ===========================================================================
 * Prototypes for level 3 BLAS
 * ===========================================================================
 */
 template<typename datatype>
class MKL_GEMM_Parameters : public MKLParamDescriptor
{
public:
    MKL_GEMM_Parameters():
        order("order", type_string<CBLAS_ORDER>::type().c_str(), CL_KERNEL_ARG_ACCESS_NONE, CL_KERNEL_ARG_TYPE_CONST, CL_KRNL_ARG_INT, this),
        transA("transA", type_string<CBLAS_TRANSPOSE>::type().c_str(), CL_KERNEL_ARG_ACCESS_NONE, CL_KERNEL_ARG_TYPE_CONST, CL_KRNL_ARG_INT, this),
        pABuffer("pA", type_string<datatype>::const_ptr().c_str(), CL_KERNEL_ARG_ACCESS_NONE, CL_KERNEL_ARG_TYPE_CONST, CL_KRNL_ARG_PTR_GLOBAL, this),
        transB("transB", type_string<CBLAS_TRANSPOSE>::type().c_str(), CL_KERNEL_ARG_ACCESS_NONE, CL_KERNEL_ARG_TYPE_CONST, CL_KRNL_ARG_INT, this),
        pBBuffer("pB", type_string<datatype>::const_ptr().c_str(), CL_KERNEL_ARG_ACCESS_NONE, CL_KERNEL_ARG_TYPE_CONST, CL_KRNL_ARG_PTR_GLOBAL, this),
        pCBuffer("pC", type_string<datatype>::ptr().c_str(), CL_KERNEL_ARG_ACCESS_NONE, CL_KERNEL_ARG_TYPE_NONE, CL_KRNL_ARG_PTR_GLOBAL,this),
        alpha("alpha", type_string<datatype>::type().c_str(), CL_KERNEL_ARG_ACCESS_NONE, CL_KERNEL_ARG_TYPE_CONST, cl_kernel_arg_type_of<datatype>::get_type(), this),
        beta("beta", type_string<datatype>::type().c_str(), CL_KERNEL_ARG_ACCESS_NONE, CL_KERNEL_ARG_TYPE_CONST, cl_kernel_arg_type_of<datatype>::get_type(), this),
        pArgDescriptor(NULL), pArgInfoDescriptor(NULL)
        {
            size_t paramCount = GetParamCount();
            pArgDescriptor = new cl_kernel_argument[paramCount];
            if ( NULL != pArgDescriptor )
            {
                for(size_t i=0; i<paramCount; ++i)
                {
                    MEMCPY_S(&pArgDescriptor[i], sizeof(cl_kernel_argument),
                            &(m_lstParams.at(i)->GetArgumentDescriptor()), sizeof(cl_kernel_argument));
                }
            }
            pArgInfoDescriptor = new cl_kernel_argument_info[paramCount];
            if ( NULL != pArgInfoDescriptor )
            {
                for(size_t i=0; i<paramCount; ++i)
                {
                    MEMCPY_S(&pArgInfoDescriptor[i], sizeof(cl_kernel_argument_info),
                        &(m_lstParams.at(i)->GetArgumentInfoDescriptor()), sizeof(cl_kernel_argument_info));
                }
            }
        }

    ~MKL_GEMM_Parameters()
    {
        if ( NULL != pArgDescriptor )
        {
            delete[] pArgDescriptor;
        }
        if ( NULL != pArgInfoDescriptor )
        {
            delete[] pArgInfoDescriptor;
        }
    }

    // Static information info
    static size_t GetParamCount() {return s_Params.m_lstParams.size();}
    static size_t GetParamSize() {return s_Params.m_Offset;}
    static const cl_kernel_argument* GetKernelParams() {return s_Params.pArgDescriptor;}
    static const cl_kernel_argument_info* GetKernelArgInfo() {return s_Params.pArgInfoDescriptor;}

    static unsigned int GetMemoryObjectArgumentCount() { return s_Params.m_lstMemArgs.size();}
    static const unsigned int* GetMemoryObjectArgumentIndexes() {return s_Params.m_lstMemArgs.size() > 0 ? &s_Params.m_lstMemArgs[0] : NULL;}

    static const MKLParam<CBLAS_ORDER>& GetOrder() { return s_Params.order; }
    static const MKLParam<CBLAS_TRANSPOSE>& GetTransA() { return s_Params.transA; }
    static const MKLParam<const datatype*>& GetParamA() { return s_Params.pABuffer; }
    static const MKLParam<CBLAS_TRANSPOSE>& GetTransB() { return s_Params.transB; }
    static const MKLParam<const datatype*>& GetParamB() { return s_Params.pBBuffer; }
    static const MKLParam<datatype*>& GetParamC() { return s_Params.pCBuffer; }
    static const MKLParam<datatype>& GetAlpha() { return s_Params.alpha; }
    static const MKLParam<datatype>& GetBeta() { return s_Params.beta; }

protected:
    MKLParam<CBLAS_ORDER>       order;
    MKLParam<CBLAS_TRANSPOSE>   transA;
    MKLParam<const datatype*>   pABuffer;
    MKLParam<CBLAS_TRANSPOSE>   transB;
    MKLParam<const datatype*>   pBBuffer;
    MKLParam<datatype*>         pCBuffer;
    MKLParam<datatype>          alpha;
    MKLParam<datatype>          beta;

    cl_kernel_argument*         pArgDescriptor;
    cl_kernel_argument_info*    pArgInfoDescriptor;

    static const MKL_GEMM_Parameters<datatype>    s_Params;
};

template<typename datatype > class MKL_GEMM_Executor : public Intel::OpenCL::BuiltInKernels::IBuiltInKernelExecutor
{
public:
    typedef void (*MKL_FuncType)(const CBLAS_ORDER Order, const  CBLAS_TRANSPOSE TransA,
                 const  CBLAS_TRANSPOSE TransB, const MKL_INT M, const MKL_INT N,
                 const MKL_INT K, const datatype alpha, const datatype *A,
                 const MKL_INT lda, const datatype *B, const MKL_INT ldb,
                 const datatype beta, datatype *C, const MKL_INT ldc);

    typedef MKL_GEMM_Parameters< datatype > MKL_GEMM_EXECUTOR_PAREMERTERS;

    MKL_GEMM_Executor(ptrdiff_t func_ptr, const void* params)
        : m_FuncPtr((MKL_FuncType) func_ptr), m_pParamBuffer(params)
        {
            const cl_uniform_kernel_args* pKernelArgs = (const cl_uniform_kernel_args*)((const char*)params+ MKL_GEMM_EXECUTOR_PAREMERTERS::GetParamSize());

            m_iRowsA_M = (int)pKernelArgs->GlobalSize[0];
            m_iColsB_N = (int)pKernelArgs->GlobalSize[1];
            m_iK = (int)pKernelArgs->GlobalSize[2];
        }

    cl_dev_err_code    Execute() const
    {
        CBLAS_ORDER order = (CBLAS_ORDER)MKL_GEMM_EXECUTOR_PAREMERTERS::GetOrder().GetValue(m_pParamBuffer);
        CBLAS_TRANSPOSE transA = (CBLAS_TRANSPOSE)MKL_GEMM_EXECUTOR_PAREMERTERS::GetTransA().GetValue(m_pParamBuffer);
        const datatype* pA = (const datatype*)MKL_GEMM_EXECUTOR_PAREMERTERS::GetParamA().GetValue(m_pParamBuffer);
        CBLAS_TRANSPOSE transB = (CBLAS_TRANSPOSE)MKL_GEMM_EXECUTOR_PAREMERTERS::GetTransB().GetValue(m_pParamBuffer);
        const datatype* pB = (const datatype*)MKL_GEMM_EXECUTOR_PAREMERTERS::GetParamB().GetValue(m_pParamBuffer);
        datatype* pC = (datatype*)MKL_GEMM_EXECUTOR_PAREMERTERS::GetParamC().GetValue(m_pParamBuffer);
        datatype alpha = (datatype)MKL_GEMM_EXECUTOR_PAREMERTERS::GetAlpha().GetValue(m_pParamBuffer);
        datatype beta = (datatype)MKL_GEMM_EXECUTOR_PAREMERTERS::GetBeta().GetValue(m_pParamBuffer);

        // Execute parallel(OpenMP) MKL function
        m_FuncPtr(order, transA, transB, m_iRowsA_M, m_iColsB_N, m_iK, alpha, pA, m_iK, pB, m_iColsB_N, beta, pC, m_iColsB_N);

        return CL_DEV_SUCCESS;
    }

    cl_dev_err_code GetLastError() const
    {
        return m_lastError;
    }

protected:
    int        m_iRowsA_M;
    int        m_iColsB_N;
    int        m_iK;

    MKL_FuncType    m_FuncPtr;
    const void*     m_pParamBuffer;

    cl_dev_err_code m_lastError;
};

// This class is used to expose MKL function on MIC device
template<typename datatype > class MKL_GEMM_Executor_Proxy : public Intel::OpenCL::BuiltInKernels::IBuiltInKernelExecutor
{
public:
    MKL_GEMM_Executor_Proxy(ptrdiff_t func_ptr, const void* params) {}

    typedef MKL_GEMM_Parameters< datatype > MKL_GEMM_EXECUTOR_PAREMERTERS;

    cl_dev_err_code    Execute() const
    {
        return CL_DEV_SUCCESS;
    }
};

}}}
