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

//#include <map>
//#include <string>

#include <builtin_kernels.h>
#include <cl_dynamic_lib.h>

#include <mkl_types.h>
#include <mkl_cblas.h>

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
			cl_kernel_arg_access_qualifier	accsQual,
			cl_kernel_arg_type_qualifier	typeQual) :
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

		//std::string&		GetParamName() {return m_szParamName;}
	protected:
		cl_kernel_argument		m_ArgDesc;
		cl_kernel_argument_info	m_ArgInfoDesc;

		std::string				m_szArgName;
		std::string				m_szArgTypeName;

	};

	template<typename paramType > class MKLParam : public MKLParamBase
	{
	public:
		MKLParam(const char* szArgName, const char* szArgTypeName,
			cl_kernel_arg_access_qualifier	accsQual,
			cl_kernel_arg_type_qualifier	typeQual,
			cl_kernel_arg_type argType,
			MKLParamDescriptor* parent) :
			MKLParamBase(szArgName, szArgTypeName, OpenCL::BuiltInKernels::ArgType2AddrQual(argType), accsQual, typeQual), m_pParent(parent)
		{
			m_stLocalOffset = parent->m_Offset;
			parent->m_Offset += sizeof(paramType);
			m_ArgDesc.size_in_bytes = sizeof(paramType);
			m_ArgDesc.type = argType;
			parent->m_lstParams.push_back(this);
		}

		paramType GetValue(const void* paramBuffer) const
		{
			return *((paramType*)(((char*)paramBuffer)+m_stLocalOffset));
		}

		size_t				m_stLocalOffset;
		MKLParamDescriptor*	m_pParent;
	};

	size_t GetParamCount() const
	{
		return m_lstParams.size();
	}

	size_t GetParamSize() const
	{
		return m_Offset;
	}
protected:
	size_t							m_Offset;

	std::vector<MKLParamBase*>		m_lstParams;	
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

		MKLParam<const datatype*>	pABuffer;
		MKLParam<const datatype*>	pBBuffer;
		MKLParam<datatype*>			pCBuffer;

		cl_kernel_argument*			pArgDescriptor;
		cl_kernel_argument_info*	pArgInfoDescriptor;
	};

template<typename datatype > class MKL_GEMM_Executor : public Intel::OpenCL::BuiltInKernels::IBuiltInKernelExecutor
{
public:
	typedef void (*MKL_FuncType)(const  CBLAS_ORDER Order, const  CBLAS_TRANSPOSE TransA,
                 const  CBLAS_TRANSPOSE TransB, const MKL_INT M, const MKL_INT N,
                 const MKL_INT K, const datatype alpha, const datatype *A,
                 const MKL_INT lda, const datatype *B, const MKL_INT ldb,
                 const datatype beta, datatype *C, const MKL_INT ldc);

	MKL_GEMM_Executor(MKL_FuncType func_ptr, size_t* pWorkSize, const void* params)
		: m_FuncPtr(func_ptr), m_pParamBuffer(params)
		{
			m_iRowsA_M = (int)pWorkSize[0];
			m_iColsB_N = (int)pWorkSize[1];
			m_iK = (int)pWorkSize[2];
		}

	cl_dev_err_code	Execute() const
	{
		cl_mem_obj_descriptor* pObjA = (cl_mem_obj_descriptor*)m_sParams.pABuffer.GetValue(m_pParamBuffer);
		cl_mem_obj_descriptor* pObjB = (cl_mem_obj_descriptor*)m_sParams.pBBuffer.GetValue(m_pParamBuffer);
		cl_mem_obj_descriptor* pObjC = (cl_mem_obj_descriptor*)m_sParams.pCBuffer.GetValue(m_pParamBuffer);

		const datatype* pA = (const datatype*)pObjA->pData;
		const datatype* pB = (const datatype*)pObjB->pData;
		datatype* pC = (datatype*)pObjC->pData;

		// Execute parallel(OpenMP) MKL function
		m_FuncPtr(CblasRowMajor, CblasNoTrans, CblasNoTrans, m_iRowsA_M, m_iColsB_N, m_iK, 1.0,
			pA, m_iK, pB, m_iColsB_N, 0.0, pC, m_iColsB_N);

		
		return CL_DEV_SUCCESS;
	}

	cl_dev_err_code GetLastError() const
	{
		return m_lastError;
	}

	static size_t GetParamCount() {return m_sParams.GetParamCount();}
	static size_t GetParamSize() {return m_sParams.GetParamSize();}
	static const cl_kernel_argument* GetKernelParams() {return m_sParams.pArgDescriptor;}
	static const cl_kernel_argument_info* GetKernelArgInfo() {return m_sParams.pArgInfoDescriptor;}


protected:

	int		m_iRowsA_M;
	int		m_iColsB_N;
	int		m_iK;

	MKL_FuncType	m_FuncPtr;
	const void*		m_pParamBuffer;

	cl_dev_err_code m_lastError;

	static const MKL_GEMM_Parameters<datatype>	m_sParams;
};

template<class MKL_EXECUTOR_CLASS > class MKLKernel : public Intel::OpenCL::BuiltInKernels::IBuiltInKernel
{
public:
	MKLKernel(const char* szName, Intel::OpenCL::Utils::OclDynamicLib::func_t pFuncPtr) :
		m_szFuncName(szName), m_pMKLFuncPtr((typename MKL_EXECUTOR_CLASS::MKL_FuncType)pFuncPtr)
	{
	}

	cl_dev_err_code	Execute(cl_dev_cmd_param_kernel* pCmdParams, void* pParamBuffer, Intel::OpenCL::BuiltInKernels::OMPExecutorThread* pThread) const
	{
		MKL_EXECUTOR_CLASS executor(m_pMKLFuncPtr, pCmdParams->glb_wrk_size, pParamBuffer);
		return pThread->Execute(executor);
	}

	size_t GetParamSize() const {return MKL_EXECUTOR_CLASS::GetParamSize();}


	//ICLDevBackendKernel
	unsigned long long int GetKernelID() const { return (unsigned long long int)this;}
	const char*	GetKernelName() const {return m_szFuncName.c_str();}
	int GetKernelParamsCount() const {return (int)MKL_EXECUTOR_CLASS::GetParamCount();}
	const cl_kernel_argument* GetKernelParams() const { return MKL_EXECUTOR_CLASS::GetKernelParams();}
	const cl_kernel_argument_info* GetKernelArgInfo() const { return MKL_EXECUTOR_CLASS::GetKernelArgInfo();}
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
	};

	std::string									m_szFuncName;
	typename MKL_EXECUTOR_CLASS::MKL_FuncType	m_pMKLFuncPtr;
	MKLKernelProperties 						m_mklProperties;
};

}}}
