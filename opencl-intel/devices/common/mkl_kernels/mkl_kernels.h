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

#include <builtin_kernels.h>
#include <cl_dynamic_lib.h>
#include <cl_types.h>

#include <mkl_types.h>
#include <mkl_cblas.h>

#if defined(__OMP2TBB__)
extern "C" void __omp2tbb_set_thread_max_concurency(int max_concurency);
#endif

namespace Intel { namespace OpenCL { namespace MKLKernels {

template<typename>
    struct type_string
    {
        static std::string type() { return "undefined"; }
        static std::string ptr() { return "undefined";}
        static std::string const_ptr() {return "undefined";}
    };

#define DEFINE_TYPE_STRING(T) \
    template<> struct type_string<T> { \
        static std::string type() { return #T; } \
        static std::string ptr() { return type()+"*";}\
        static std::string const_ptr() {return string("const ")+type()+"*";}\
    };

DEFINE_TYPE_STRING(float)
DEFINE_TYPE_STRING(double)

class MKLParamDescriptor
{
public:
    MKLParamDescriptor() : m_Offset(0) {}

    class MKLParamBase
    {
    public:
        MKLParamBase(const char* szArgName, const char* szArgTypeName,
            cl_kernel_arg_address_qualifier addrQual,
            cl_kernel_arg_access_qualifier    accsQual,
            cl_kernel_arg_type_qualifier    typeQual) :
        m_szArgName(szArgName), m_szArgTypeName(szArgTypeName)
        {
            m_ArgInfoDesc.name = m_szArgName.c_str();
            m_ArgInfoDesc.typeName = m_szArgTypeName.c_str();
            m_ArgInfoDesc.adressQualifier = addrQual;
            m_ArgInfoDesc.accessQualifier = accsQual;
            m_ArgInfoDesc.typeQualifier = typeQual;
        }

        cl_kernel_argument_info& GetArgumentInfoDescriptor() {return m_ArgInfoDesc;}
        cl_kernel_argument& GetArgumentDescriptor() {return m_ArgDesc;}

    protected:
        cl_kernel_argument      m_ArgDesc;
        cl_kernel_argument_info m_ArgInfoDesc;

        std::string             m_szArgName;
        std::string             m_szArgTypeName;

    };

    template<typename paramType > class MKLParam : public MKLParamBase
    {
    public:
        MKLParam(const char* szArgName, const char* szArgTypeName,
            cl_kernel_arg_access_qualifier    accsQual,
            cl_kernel_arg_type_qualifier    typeQual,
            cl_kernel_arg_type argType,
            MKLParamDescriptor* parent) :
            MKLParamBase(szArgName, szArgTypeName, OpenCL::BuiltInKernels::ArgType2AddrQual(argType), accsQual, typeQual), m_pParent(parent)
        {
            m_ArgDesc.offset_in_bytes = parent->m_Offset;
            parent->m_Offset += sizeof(paramType);
            m_ArgDesc.size_in_bytes = sizeof(paramType);
            m_ArgDesc.type = argType;
            parent->m_lstParams.push_back(this);
            if ( (argType >= CL_KRNL_ARG_PTR_GLOBAL) && (argType <= CL_KRNL_ARG_PTR_CONST) )
            {
                // Currently only GLOBAL and CONSTANT buffers are supported
                unsigned int pramInx = parent->m_lstParams.size();
                parent->m_lstMemArgs.push_back(pramInx - 1);
            }
        }

        paramType GetValue(const void* paramBuffer) const
        {
            return *((paramType*)(((char*)paramBuffer)+m_ArgDesc.offset_in_bytes));
        }

        MKLParamDescriptor*    m_pParent;
    };

    size_t GetParamCount() const
    {
        return m_lstParams.size();
    }

    size_t GetParamSize() const
    {
        return m_Offset;
    }

    unsigned int        GetMemoryObjectArgumentCount() const { return m_lstMemArgs.size(); }
    const unsigned int* GetMemoryObjectArgumentIndexes() const { return m_lstMemArgs.size()>0 ? &m_lstMemArgs[0] : NULL;}

protected:
    size_t                            m_Offset;
    
    std::vector<unsigned int>       m_lstMemArgs;       // List of indexes for memory arguments
    std::vector<MKLParamBase*>        m_lstParams;        // List of kernel arguments
};

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
        pABuffer("pA", type_string<datatype>::const_ptr().c_str(), CL_KERNEL_ARG_ACCESS_NONE, CL_KERNEL_ARG_TYPE_CONST, CL_KRNL_ARG_PTR_GLOBAL, this),
        pBBuffer("pB", type_string<datatype>::const_ptr().c_str(), CL_KERNEL_ARG_ACCESS_NONE, CL_KERNEL_ARG_TYPE_CONST, CL_KRNL_ARG_PTR_GLOBAL, this),
        pCBuffer("pC", type_string<datatype>::ptr().c_str(), CL_KERNEL_ARG_ACCESS_NONE, CL_KERNEL_ARG_TYPE_NONE, CL_KRNL_ARG_PTR_GLOBAL,this),
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

    static const MKLParam<const datatype*>& GetParamA() { return s_Params.pABuffer; }
    static const MKLParam<const datatype*>& GetParamB() { return s_Params.pBBuffer; }
    static const MKLParam<datatype*>& GetParamC() { return s_Params.pCBuffer; }

protected:
    MKLParam<const datatype*>    pABuffer;
    MKLParam<const datatype*>    pBBuffer;
    MKLParam<datatype*>          pCBuffer;

    cl_kernel_argument*          pArgDescriptor;
    cl_kernel_argument_info*     pArgInfoDescriptor;


    static const MKL_GEMM_Parameters<datatype>    s_Params;
};

template<typename datatype > class MKL_GEMM_Executor : public Intel::OpenCL::BuiltInKernels::IBuiltInKernelExecutor
{
public:
    typedef void (*MKL_FuncType)(const  CBLAS_ORDER Order, const  CBLAS_TRANSPOSE TransA,
                 const  CBLAS_TRANSPOSE TransB, const MKL_INT M, const MKL_INT N,
                 const MKL_INT K, const datatype alpha, const datatype *A,
                 const MKL_INT lda, const datatype *B, const MKL_INT ldb,
                 const datatype beta, datatype *C, const MKL_INT ldc);

    typedef MKL_GEMM_Parameters< datatype > MKL_GEMM_EXECUTOR_PAREMERTERS;

    MKL_GEMM_Executor(Intel::OpenCL::Utils::OclDynamicLib::func_t func_ptr, const void* params)
        : m_FuncPtr((MKL_FuncType) func_ptr), m_pParamBuffer(params)
        {
            const cl_uniform_kernel_args* pKernelArgs = (const cl_uniform_kernel_args*)((const char*)params+ MKL_GEMM_EXECUTOR_PAREMERTERS::GetParamSize());

            m_iRowsA_M = (int)pKernelArgs->GlobalSize[0];
            m_iColsB_N = (int)pKernelArgs->GlobalSize[1];
            m_iK = (int)pKernelArgs->GlobalSize[2];
        }

    cl_dev_err_code    Execute() const
    {
        const datatype* pA = (const datatype*)MKL_GEMM_EXECUTOR_PAREMERTERS::GetParamA().GetValue(m_pParamBuffer);
        const datatype* pB = (const datatype*)MKL_GEMM_EXECUTOR_PAREMERTERS::GetParamB().GetValue(m_pParamBuffer);
        datatype* pC = (datatype*)MKL_GEMM_EXECUTOR_PAREMERTERS::GetParamC().GetValue(m_pParamBuffer);

        // Execute parallel(OpenMP) MKL function
        m_FuncPtr(CblasRowMajor, CblasNoTrans, CblasNoTrans, m_iRowsA_M, m_iColsB_N, m_iK, (datatype)1.0, pA, m_iK, pB, m_iColsB_N, (datatype)0.0, pC, m_iColsB_N);

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
    MKL_GEMM_Executor_Proxy(Intel::OpenCL::Utils::OclDynamicLib::func_t func_ptr, const void* params) {}

    typedef MKL_GEMM_Parameters< datatype > MKL_GEMM_EXECUTOR_PAREMERTERS;

    cl_dev_err_code    Execute() const
    {
        return CL_DEV_SUCCESS;
    }
/*
    static size_t GetParamCount() {return m_sParams.GetParamCount();}
    static size_t GetParamSize() {return m_sParams.GetParamSize();}
    static const cl_kernel_argument* GetKernelParams() {return m_sParams.pArgDescriptor;}
    static const cl_kernel_argument_info* GetKernelArgInfo() {return m_sParams.pArgInfoDescriptor;}

    static unsigned int GetMemoryObjectArgumentCount() { return m_sParams.GetMemoryObjectArgumentCount();}
    static const unsigned int* GetMemoryObjectArgumentIndexes() {return m_sParams.GetMemoryObjectArgumentIndexes();}
*/
};

template<class MKL_EXECUTOR_CLASS > class MKLKernel : public Intel::OpenCL::BuiltInKernels::IBuiltInKernel
{
public:
    MKLKernel(const char* szName, Intel::OpenCL::Utils::OclDynamicLib::func_t pFuncPtr) :
        m_szFuncName(szName), m_pMKLFuncPtr(pFuncPtr)
    {
    }

#ifndef __OMP2TBB__
    cl_dev_err_code    Execute(const void* pParamBuffer, Intel::OpenCL::BuiltInKernels::OMPExecutorThread* pThread) const
    {
        MKL_EXECUTOR_CLASS executor(m_pMKLFuncPtr, pParamBuffer);
        return pThread->Execute(executor);
    }
#else
    cl_dev_err_code    Execute(const Intel::OpenCL::TaskExecutor::ITaskList* pList, const void* pParamBuffer) const
    {
        // Set concurrency for the library, this is a workaround until TBB will return a pointer to current arena
        // Currently we assuming that master/application thread is joining.
        // TODO: need to recalculate number of active workers

        int iDeviceConcurency = pList->GetDeviceConcurency();
        bool bMasterJoinedExecution = pList->IsMasterJoined();
        bool bCanMasterJoin = pList->CanMasterJoin();

        // If master can join, but currently it doesn't we need execute on less threads
        if ( bCanMasterJoin && !bMasterJoinedExecution )
        {
            --iDeviceConcurency;
        }

        __omp2tbb_set_thread_max_concurency(iDeviceConcurency);

        MKL_EXECUTOR_CLASS executor(m_pMKLFuncPtr, pParamBuffer);
        return executor.Execute();
    }
#endif

    //ICLDevBackendKernel
    unsigned long long int GetKernelID() const { return (unsigned long long int)this;}
    const char*    GetKernelName() const {return m_szFuncName.c_str();}
    int GetKernelParamsCount() const {return (int)MKL_EXECUTOR_CLASS::MKL_GEMM_EXECUTOR_PAREMERTERS::GetParamCount();}
    const cl_kernel_argument* GetKernelParams() const { return MKL_EXECUTOR_CLASS::MKL_GEMM_EXECUTOR_PAREMERTERS::GetKernelParams();}
    const cl_kernel_argument_info* GetKernelArgInfo() const { return MKL_EXECUTOR_CLASS::MKL_GEMM_EXECUTOR_PAREMERTERS::GetKernelArgInfo();}
    size_t GetExplicitArgumentBufferSize(void) const { return MKL_EXECUTOR_CLASS::MKL_GEMM_EXECUTOR_PAREMERTERS::GetParamSize(); }
    size_t GetArgumentBufferRequiredAlignment(void) const { return MKL_EXECUTOR_CLASS::MKL_GEMM_EXECUTOR_PAREMERTERS::GetKernelParams()[0].size_in_bytes;}
    const Intel::OpenCL::DeviceBackend::ICLDevBackendKernelRunner* GetKernelRunner(void) const { return NULL;}

    int GetLineNumber(void* pointer) const { return -1;}

    size_t GetArgumentBufferSize() const { return MKL_EXECUTOR_CLASS::MKL_GEMM_EXECUTOR_PAREMERTERS::GetParamSize();}
    unsigned int GetMemoryObjectArgumentCount() const {return MKL_EXECUTOR_CLASS::MKL_GEMM_EXECUTOR_PAREMERTERS::GetMemoryObjectArgumentCount();}
    const unsigned int* GetMemoryObjectArgumentIndexes() const {return MKL_EXECUTOR_CLASS::MKL_GEMM_EXECUTOR_PAREMERTERS::GetMemoryObjectArgumentIndexes();}

    const Intel::OpenCL::DeviceBackend::ICLDevBackendKernelProporties* GetKernelProporties() const {return &m_mklProperties;}

protected:
    class MKLKernelProperties : public Intel::OpenCL::DeviceBackend::ICLDevBackendKernelProporties
    {
        unsigned int GetKernelPackCount() const {return 1;}
        const size_t* GetRequiredWorkGroupSize() const {return NULL;}
        size_t GetPrivateMemorySize() const {return 1;}
        size_t GetImplicitLocalMemoryBufferSize() const {return 0;}
        size_t GetKernelExecutionLength() const {return -1;}
        bool HasPrintOperation() const {return false;}
        bool HasBarrierOperation() const {return false;}
        bool HasKernelCallOperation() const {return false;}
        unsigned int GetMinGroupSizeFactorial() const { return 0;}
        bool IsBlock() const { return false;}
        const char* GetKernelAttributes() const { return NULL; }
    };

    std::string                                 m_szFuncName;
    Intel::OpenCL::Utils::OclDynamicLib::func_t m_pMKLFuncPtr;
    MKLKernelProperties                         m_mklProperties;
};

}}}
