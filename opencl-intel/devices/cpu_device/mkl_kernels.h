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

#include <map>
#include <string>
#include <tbb/tbb.h>

#include "dispatcher_commands.h"
#include "builtin_kernels.h"
#include "mkl_types.h"
#include "mkl_cblas.h"

using namespace Intel::OpenCL::DeviceBackend;

namespace Intel { namespace OpenCL { namespace CPUDevice {

class MKLExecutor
{
public:
	virtual void	Execute() = 0;
};

cl_kernel_arg_address_qualifier ArgType2AddrQual(cl_kernel_arg_type type);

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
			MKLParamBase(szArgName, szArgTypeName, ArgType2AddrQual(argType), accsQual, typeQual), m_pParent(parent)
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
			pABuffer("pA", "const void*", CL_KERNEL_ARG_ACCESS_NONE, CL_KERNEL_ARG_TYPE_CONST, CL_KRNL_ARG_PTR_GLOBAL, this),
			pBBuffer("pB", "const void*",CL_KERNEL_ARG_ACCESS_NONE, CL_KERNEL_ARG_TYPE_CONST, CL_KRNL_ARG_PTR_GLOBAL, this),
			pCBuffer("pC", "void*", CL_KERNEL_ARG_ACCESS_NONE, CL_KERNEL_ARG_TYPE_NONE, CL_KRNL_ARG_PTR_GLOBAL,this),
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

template<typename datatype > class MKL_GEMM_Executor : public MKLExecutor
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

	void	Execute()
	{
		int grain_size = 1;
		tbb::parallel_for(tbb::blocked_range2d<int>(0, m_iRowsA_M, grain_size,
											   0, m_iColsB_N, grain_size),
												*this,
												tbb::auto_partitioner());
	}

	void operator() ( const tbb::blocked_range2d<int>& range ) const
	{
		cl_mem_obj_descriptor* pObjA = (cl_mem_obj_descriptor*)m_sParams.pABuffer.GetValue(m_pParamBuffer);
		cl_mem_obj_descriptor* pObjB = (cl_mem_obj_descriptor*)m_sParams.pBBuffer.GetValue(m_pParamBuffer);
		cl_mem_obj_descriptor* pObjC = (cl_mem_obj_descriptor*)m_sParams.pCBuffer.GetValue(m_pParamBuffer);

		const datatype* pA = (const datatype*)pObjA->pData +range.rows().begin()*m_iK;
		const datatype* pB = (const datatype*)pObjB->pData+range.cols().begin();
		datatype* pC = (datatype*)pObjC->pData+range.rows().begin()*m_iColsB_N+range.cols().begin();

		m_FuncPtr(CblasRowMajor, CblasNoTrans, CblasNoTrans, range.rows().size(), range.cols().size(), m_iK, 1.0,
			pA, m_iK, pB, m_iColsB_N, 0.0, pC, m_iColsB_N);
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

	static const MKL_GEMM_Parameters<datatype>	m_sParams;
};

template<class MKL_EXECUTOR_CLASS > class MKLKernelTask : public CommandBaseClass<ITask>
{
public:
	MKLKernelTask(typename MKL_EXECUTOR_CLASS::MKL_FuncType pFuncPtr, TaskDispatcher* pTD, cl_dev_cmd_desc* pCmd)
		: CommandBaseClass<ITask>(pTD, pCmd), m_pFuncPtr(pFuncPtr)
	{
	}

	bool Execute()
	{
		NotifyCommandStatusChanged(m_pCmd, CL_RUNNING, CL_DEV_SUCCESS);

		void* pParamBuffer = alloca(MKL_EXECUTOR_CLASS::GetParamSize());
		if ( NULL == pParamBuffer )
		{
			assert(0 && "alloca always success");
			NotifyCommandStatusChanged(m_pCmd, CL_COMPLETE, (int)CL_DEV_OUT_OF_MEMORY);
			return false;
		}

		cl_dev_err_code err = ExtractNDRangeParams(pParamBuffer);
		if ( CL_DEV_FAILED(err) )
		{
			NotifyCommandStatusChanged(m_pCmd, CL_COMPLETE, err);
			return false;
		}

		{	// Create new stack frame
			cl_dev_cmd_param_kernel *cmdParams = (cl_dev_cmd_param_kernel*)m_pCmd->params;
			MKL_EXECUTOR_CLASS executor(m_pFuncPtr, cmdParams->glb_wrk_size, pParamBuffer);
			executor.Execute();
		}

		NotifyCommandStatusChanged(m_pCmd, CL_COMPLETE, CL_DEV_SUCCESS);

		return true;
	}

	protected:
		typename MKL_EXECUTOR_CLASS::MKL_FuncType m_pFuncPtr;
};

template<class MKL_EXECUTOR_CLASS > class MKLKernel : public IBuiltInKernel
{
public:
	MKLKernel(const char* szName, typename MKL_EXECUTOR_CLASS::MKL_FuncType pFuncPtr) :
		m_szFuncName(szName), m_pMKLFuncPtr(pFuncPtr)
	{
	}

	cl_dev_err_code CreateBIKernelTask(TaskDispatcher* pTD, cl_dev_cmd_desc* pCmd, ITaskBase* *pTask)
	{
		if ( NULL == pTask )
		{
			return CL_DEV_INVALID_OPERATION;
		}

		*pTask = new MKLKernelTask<MKL_EXECUTOR_CLASS>(m_pMKLFuncPtr, pTD, pCmd);
		if ( NULL == *pTask )
		{
			return CL_DEV_OUT_OF_MEMORY;
		}

		return CL_DEV_SUCCESS;
	}
	
	//ICLDevBackendKernel
	unsigned long long int GetKernelID() const { return (unsigned long long int)this;}
	const char*	GetKernelName() const {return m_szFuncName.c_str();}
	int GetKernelParamsCount() const {return (int)MKL_EXECUTOR_CLASS::GetParamCount();}
	const cl_kernel_argument* GetKernelParams() const { return MKL_EXECUTOR_CLASS::GetKernelParams();}
	const cl_kernel_argument_info* GetKernelArgInfo() const { return MKL_EXECUTOR_CLASS::GetKernelArgInfo();}
	const ICLDevBackendKernelProporties* GetKernelProporties() const {return &m_mklProperties;}

protected:
	class MKLKernelProperties : public ICLDevBackendKernelProporties
	{
		unsigned int GetKernelPackCount() const {return 1;}
		const size_t* GetRequiredWorkGroupSize() const {return NULL;}
		size_t GetPrivateMemorySize() const {return 1;}
		size_t GetImplicitLocalMemoryBufferSize() const {return 0;}
		bool HasPrintOperation() const {return false;}
		bool HasBarrierOperation() const {return false;}
		bool HasKernelCallOperation() const {return false;}
	};

	std::string									m_szFuncName;
	typename MKL_EXECUTOR_CLASS::MKL_FuncType	m_pMKLFuncPtr;
	MKLKernelProperties 						m_mklProperties;
};

#define REGISTER_MKL_FUNCTION(MKL_FUNCTION_NAME,MKL_CLASS_TYPE,DATA_TYPE) \
	cl_dev_err_code MKL_FUNCTION_NAME##Creator(IBuiltInKernel* *ppBIKernel)\
	{\
		*ppBIKernel = new MKLKernel< MKL_##MKL_CLASS_TYPE##_Executor<DATA_TYPE > >(#MKL_FUNCTION_NAME, MKL_FUNCTION_NAME);\
		return CL_DEV_SUCCESS;\
	}\
	template<> const MKL_##MKL_CLASS_TYPE##_Parameters<DATA_TYPE> MKL_##MKL_CLASS_TYPE##_Executor<DATA_TYPE>::m_sParams = MKL_##MKL_CLASS_TYPE##_Parameters<DATA_TYPE>();\
	REGISTER_BUILTIN_KERNEL(MKL_FUNCTION_NAME, MKL_FUNCTION_NAME##Creator)

}}}
