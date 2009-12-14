// Copyright (c) 2006-2007 Intel Corporation
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

///////////////////////////////////////////////////////////////////////////////////////////////////
//  kernel.cpp
//  Implementation of the Kernel class
//  Created on:      10-Dec-2008 2:08:23 PM
//  Original author: ulevy
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "kernel.h"
#include "context.h"
#include "program_binary.h"
#include "program.h"
#include <cl_objects_map.h>
#include <device.h>
#include <assert.h>
#include <cl_utils.h>
using namespace Intel::OpenCL::Utils;
using namespace Intel::OpenCL::Framework;

using namespace std;

///////////////////////////////////////////////////////////////////////////////////////////////////
// DeviceKernel C'tor
///////////////////////////////////////////////////////////////////////////////////////////////////
DeviceKernel::DeviceKernel(Kernel * pKernel, 
						   ProgramBinary * pProgBin, 
						   const char * psKernelName, 
						   LoggerClient * pLoggerClient,
						   cl_err_code * pErr)
{
	//cl_start;

	m_sKernelPrototype.m_psKernelName = NULL;
	m_sKernelPrototype.m_pArgs = NULL;

#ifdef _DEBUG
	assert ( pErr != NULL );
#endif

	SET_LOGGER_CLIENT(pLoggerClient);
	LOG_DEBUG(L"DeviceKernel C'tor enter");

	if (NULL == pKernel || NULL == pProgBin || NULL == psKernelName || NULL == pProgBin->GetDevice())
	{
		LOG_ERROR(L"NULL == pProgBin || NULL == pKernel || NULL == psKernelName || NULL == pProgBin->GetDevice()");
		*pErr = CL_INVALID_VALUE;
		return;
	}

	m_pKernel = pKernel;
	m_pDevice = (Device*)pProgBin->GetDevice();

	// update kernel prototype
	m_sKernelPrototype.m_psKernelName = new char[strlen(psKernelName) + 1];
	if (NULL == m_sKernelPrototype.m_psKernelName)
	{
		LOG_ERROR(L"new char[%d] == NULL", strlen(psKernelName) + 1);
		delete[] m_sKernelPrototype.m_psKernelName;
		m_sKernelPrototype.m_psKernelName = NULL;
		*pErr = CL_OUT_OF_HOST_MEMORY;
		return;
	}
	// copy kernel name;
	strcpy_s(m_sKernelPrototype.m_psKernelName, strlen(psKernelName) + 1, psKernelName);

	// get kernel id
	cl_err_code clErrRet = m_pDevice->GetKernelId(pProgBin->GetId(), m_sKernelPrototype.m_psKernelName, &m_clDevKernel);
	if (CL_FAILED(clErrRet))
	{
		LOG_ERROR(L"Device->GetKernelId failed");
		delete[] m_sKernelPrototype.m_psKernelName;
		m_sKernelPrototype.m_psKernelName = NULL;
		*pErr = (clErrRet==CL_DEV_INVALID_KERNEL_NAME) ? CL_INVALID_KERNEL_NAME : CL_OUT_OF_HOST_MEMORY;
		return;
	}

	// get kernel prototype
	size_t szArgsCount = 0;
	clErrRet = m_pDevice->GetKernelInfo(m_clDevKernel, CL_DEV_KERNEL_PROTOTYPE, 0, NULL, &szArgsCount);
	if (CL_FAILED(clErrRet))
	{
		*pErr = clErrRet;
		return;
	}
	m_sKernelPrototype.m_uiArgsCount = szArgsCount / sizeof(cl_kernel_argument);
	m_sKernelPrototype.m_pArgs = new cl_kernel_argument[m_sKernelPrototype.m_uiArgsCount];
	if (NULL == m_sKernelPrototype.m_pArgs)
	{
		*pErr = CL_OUT_OF_HOST_MEMORY;
		return;
	}
	clErrRet = m_pDevice->GetKernelInfo(m_clDevKernel, CL_DEV_KERNEL_PROTOTYPE, szArgsCount, m_sKernelPrototype.m_pArgs, NULL);
	if (CL_FAILED(clErrRet))
	{
		*pErr = (clErrRet==CL_DEV_INVALID_KERNEL_NAME) ? CL_INVALID_KERNEL_NAME : CL_OUT_OF_HOST_MEMORY;
		return;
	}
	//cl_return;
	return;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// DeviceKernel D'tor
///////////////////////////////////////////////////////////////////////////////////////////////////
DeviceKernel::~DeviceKernel()
{
	LOG_DEBUG(L"DeviceKernel D'tor enter");
	if (NULL != m_sKernelPrototype.m_psKernelName)
	{
		delete[] m_sKernelPrototype.m_psKernelName;
		m_sKernelPrototype.m_psKernelName = NULL;
	}
	if (NULL != m_sKernelPrototype.m_pArgs)
	{
		delete[] m_sKernelPrototype.m_pArgs;
		m_sKernelPrototype.m_pArgs = NULL;
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// DeviceKernel D'tor
///////////////////////////////////////////////////////////////////////////////////////////////////
bool DeviceKernel::CheckKernelDefinition(DeviceKernel * pKernel)
{
	//cl_start;

	if (NULL == pKernel)
	{
		return false;
	}
	SKernelPrototype sKernelPrototype = pKernel->GetPrototype();
	if (strcmp(sKernelPrototype.m_psKernelName, m_sKernelPrototype.m_psKernelName) != 0)
	{
		return false;
	}
	if (sKernelPrototype.m_uiArgsCount != m_sKernelPrototype.m_uiArgsCount)
	{
		return false;
	}
	for (cl_uint ui=0; ui<m_sKernelPrototype.m_uiArgsCount; ++ui)
	{
		if ((sKernelPrototype.m_pArgs[ui].type != m_sKernelPrototype.m_pArgs[ui].type) ||
			(sKernelPrototype.m_pArgs[ui].size_in_bytes != m_sKernelPrototype.m_pArgs[ui].size_in_bytes))
		{
			return false;
		}
	}
	// kernel prototypes are identical

	//cl_return true;
	return true;
}
KernelArg::KernelArg(cl_uint uiIndex, size_t szSize, void * pValue, cl_kernel_argument clKernelArgType)
{
	m_uiIndex = uiIndex;
	m_szSize = szSize;
	m_pValue = pValue;
	m_clKernelArgType = clKernelArgType;

	if (m_clKernelArgType.type <= CL_KRNL_ARG_VECTOR)
	{
		m_pValue = new char[m_szSize];
		memcpy_s(m_pValue, m_szSize, pValue, szSize);
	}

	if (m_clKernelArgType.type == CL_KRNL_ARG_PTR_LOCAL)
	{
		m_pValue = NULL;
		if (m_szSize > 0)
		{
			m_pValue = new void*;
			*((void**)m_pValue) = (void*)m_szSize;
			m_szSize = sizeof(void*);
		}
	}
}
KernelArg::~KernelArg()
{
	if ((m_clKernelArgType.type <= CL_KRNL_ARG_VECTOR) && (NULL != m_pValue))
	{
		delete [] m_pValue;
		m_pValue = NULL;
	}
	else if ((m_clKernelArgType.type == CL_KRNL_ARG_PTR_LOCAL) && (NULL != m_pValue))
	{
		delete m_pValue;
		m_pValue = NULL;
	}
}

bool KernelArg::IsMemObject() const
{
	return (m_clKernelArgType.type >= CL_KRNL_ARG_PTR_GLOBAL);
}

/*
bool KernelArg::IsBuffer() const
{
	return ((m_clKernelArgType.type == CL_KRNL_ARG_PTR_GLOBAL)	|| 
			//(m_clKernelArgType.type == CL_KRNL_ARG_PTR_LOCAL)	||
			(m_clKernelArgType.type == CL_KRNL_ARG_PTR_CONST));
}
bool KernelArg::IsImage() const
{
	return ((m_clKernelArgType.type == CL_KRNL_ARG_PTR_IMG_2D)	|| 
			(m_clKernelArgType.type == CL_KRNL_ARG_PTR_IMG_3D));
}
*/

bool KernelArg::IsSampler() const
{
	return (m_clKernelArgType.type == CL_KRNL_ARG_SAMPLER);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// Kernel C'tor
///////////////////////////////////////////////////////////////////////////////////////////////////
Kernel::Kernel(Program * pProgram, const char * psKernelName)
{
	m_pProgram = pProgram;

    // Sign to be dependent on the program, ensure the program will be delated only after the object is
    m_pProgram->AddPendency();
        

	m_psKernelName = new char[strlen(psKernelName) + 1];
	if (NULL != m_psKernelName)
	{
		strcpy_s(m_psKernelName, strlen(psKernelName) + 1, psKernelName);
	}

	m_sKernelPrototype.m_psKernelName = NULL;
	m_sKernelPrototype.m_pArgs = NULL;
	m_sKernelPrototype.m_uiArgsCount = 0;

	m_pHandle = new _cl_kernel;
	m_pHandle->object = this;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// Kernel D'tor
///////////////////////////////////////////////////////////////////////////////////////////////////
Kernel::~Kernel()
{
	LOG_DEBUG(L"Kernel D'tor enter");
	delete[] m_psKernelName;

	// release kernel prototype
	delete[] m_sKernelPrototype.m_psKernelName;
	delete[] m_sKernelPrototype.m_pArgs;

    m_pProgram->RemovePendency();

	// clear device kernels
	map<cl_device_id, DeviceKernel*>::iterator it_kernel = m_mapDeviceKernels.begin();
	while( it_kernel != m_mapDeviceKernels.end())
	{
		if (NULL != it_kernel->second)
		{
			delete it_kernel->second;
		}
		it_kernel++;
	}
	m_mapDeviceKernels.clear();

	// delete kernel arguments
	map<cl_uint,KernelArg*>::iterator it_arg = m_mapKernelArgs.begin();
	while (it_arg != m_mapKernelArgs.end())
	{
		KernelArg * pKernelArg = it_arg->second;
		if (NULL != pKernelArg)
		{
			delete pKernelArg;
		}
		it_arg++;
	}
	m_mapKernelArgs.clear();

	if (NULL != m_pHandle)
	{
		delete m_pHandle;
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// Kernel::GetInfo
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code	Kernel::GetInfo(cl_int iParamName, size_t szParamValueSize, void * pParamValue, size_t * pszParamValueSizeRet)
{
	LOG_DEBUG(L"Enter Kernel::GetInfo (iParamName=%d, szParamValueSize=%d, pParamValue=%d, pszParamValueSizeRet=%d)",
		iParamName, szParamValueSize, pParamValue, pszParamValueSizeRet);

	if (NULL == pParamValue && NULL == pszParamValueSizeRet)
	{
		return CL_INVALID_VALUE;
	}
	size_t szParamSize = 0;
	void * pValue = NULL;
	cl_int iParam = 0;
	switch (iParamName)
	{
	case CL_KERNEL_FUNCTION_NAME:
		if (NULL != m_sKernelPrototype.m_psKernelName)
		{
			szParamSize = strlen(m_sKernelPrototype.m_psKernelName) + 1;
			pValue = m_sKernelPrototype.m_psKernelName;
		}
		break;
	case CL_KERNEL_NUM_ARGS:
		szParamSize = sizeof(cl_uint);
		pValue = &(m_sKernelPrototype.m_uiArgsCount);
		break;
	case CL_KERNEL_REFERENCE_COUNT:
		szParamSize = sizeof(cl_uint);
		pValue = &m_uiRefCount;
		break;
	case CL_KERNEL_CONTEXT:
		if (NULL != m_pProgram && NULL != m_pProgram->GetContext())
		{
			szParamSize = sizeof(cl_context);
			iParam = m_pProgram->GetContext()->GetId();
			pValue = &iParam;
		}
		break;
	case CL_KERNEL_PROGRAM:
		if (NULL != m_pProgram)
		{
			szParamSize = sizeof(cl_program);
			iParam = m_pProgram->GetId();
			pValue = &iParam;
		}
		break;
	default:
		return CL_INVALID_VALUE;
		break;
	}

	if (NULL != pParamValue && szParamValueSize < szParamSize)
	{
		return CL_INVALID_VALUE;
	}

	if (NULL != pszParamValueSizeRet)
	{
		*pszParamValueSizeRet = szParamSize;
	}

	if (NULL != pParamValue && szParamSize > 0)
	{
		memcpy_s(pParamValue, szParamValueSize, pValue, szParamSize);
	}
	
	return CL_SUCCESS;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// Kernel::GetWorkGroupInfo
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code	Kernel::GetWorkGroupInfo(cl_device_id clDevice, cl_int iParamName, size_t szParamValueSize, void * pParamValue, size_t * pszParamValueSizeRet)
{
	LOG_DEBUG(L"Enter Kernel::GetWorkGroupInfo (clDevice=%d, iParamName=%d, szParamValueSize=%d, pParamValue=%d, pszParamValueSizeRet=%d)",
		clDevice, iParamName, szParamValueSize, pParamValue, pszParamValueSizeRet);

#ifdef _DEBUG
	assert ( "No context assigned to the kernel" && (NULL != m_pProgram) && (NULL != m_pProgram->GetContext()) );
#endif

	// check input parameters
	if (NULL == pParamValue && NULL == pszParamValueSizeRet)
	{
		return CL_INVALID_VALUE;
	}

	//get device
	Device * pDevice = NULL;
	Context * pContext = (Context*)GetContext();
	cl_err_code clErr = pContext->GetDevice(clDevice, &pDevice);
	if (CL_FAILED(clErr) || NULL == pDevice)
	{
		return CL_INVALID_DEVICE;
	}

	cl_dev_kernel clDevKernel = (cl_dev_kernel)GetDeviceKernelId(clDevice);

	switch (iParamName)
	{
	case CL_KERNEL_WORK_GROUP_SIZE:
		clErr = pDevice->GetKernelInfo(clDevKernel, CL_DEV_KERNEL_WG_SIZE, szParamValueSize, pParamValue, pszParamValueSizeRet);
		break;
	case CL_KERNEL_COMPILE_WORK_GROUP_SIZE:
		clErr = pDevice->GetKernelInfo(clDevKernel, CL_DEV_KERNEL_WG_SIZE_REQUIRED, szParamValueSize, pParamValue, pszParamValueSizeRet);
		break;
	case CL_KERNEL_LOCAL_MEM_SIZE:
		clErr = pDevice->GetKernelInfo(clDevKernel, CL_DEV_KERNEL_IMPLICIT_LOCAL_SIZE, szParamValueSize, pParamValue, pszParamValueSizeRet);
		break;
	default:
		clErr = CL_INVALID_VALUE;
	}

	if( CL_DEV_INVALID_KERNEL == clErr )
	{
		clErr = CL_INVALID_KERNEL;
	}
	if( CL_DEV_INVALID_VALUE == clErr )
	{
		clErr = CL_INVALID_VALUE;
	}

	return clErr;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// Kernel::CreateDeviceKernels
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Kernel::CreateDeviceKernels(cl_uint uiBinariesCount, ProgramBinary ** ppBinaries)
{
	//cl_start;

	LOG_DEBUG(L"Enter AddDeviceKernel (uiBinariesCount=%d, ppBinaries=%d)", uiBinariesCount, ppBinaries);
	if (NULL == ppBinaries)
	{
		return CL_INVALID_VALUE;
	}
	
	cl_err_code clErrRet = CL_SUCCESS;
	Device * pDevice = NULL;
	DeviceKernel * pDeviceKernel = NULL, * pPrevDeviceKernel = NULL;
	bool bResult = false;
	
	for(cl_uint ui=0; ui<uiBinariesCount; ++ui)
	{
		// check that this is valid binary
		if (NULL == ppBinaries[ui] || NULL == ppBinaries[ui]->GetDevice())
		{
			LOG_DEBUG(L"NULL == ppBinaries[%d] || NULL == ppBinaries[%d]->GetDevice()", ui, ui);
			break;
		}
		// get build status and check that there is a valid binary;
		cl_build_status clBuildStatus = (cl_build_status)ppBinaries[ui]->GetStatus();
		if (clBuildStatus != CL_BUILD_SUCCESS)
		{
			break;
		}
		pDevice = (Device*)ppBinaries[ui]->GetDevice();
		cl_device_id clDeviceId = (cl_device_id)pDevice->GetId();
		if (m_mapDeviceKernels.find(clDeviceId) != m_mapDeviceKernels.end())
		{
			LOG_DEBUG(L"m_pDeviceKernels->IsExists(%d) = true", clDeviceId);
			break;
		}
		
		// create the device kernel object
		pDeviceKernel = new DeviceKernel(this, ppBinaries[ui], m_psKernelName, GET_LOGGER_CLIENT, &clErrRet);
		if (CL_FAILED(clErrRet))
		{
			LOG_ERROR(L"new DeviceKernel(...) faild (returned %ws)", ClErrTxt(clErrRet));
			delete pDeviceKernel;
			break;
		}
		
		// check kernel definition - compare previous kernel to the next one
		if (NULL != pPrevDeviceKernel)
		{
			bResult = pDeviceKernel->CheckKernelDefinition(pPrevDeviceKernel);
			if (false == bResult)
			{
				LOG_ERROR(L"CheckKernelDefinition failed (returned false)");
				delete pDeviceKernel;
				clErrRet = CL_INVALID_KERNEL_DEFINITION;
				break;
			}
		}

		// update previous device kernel pointer
		pPrevDeviceKernel = pDeviceKernel;
		
		// add new device kernel to the objects map list
		m_mapDeviceKernels[clDeviceId] = pDeviceKernel;
	}
	
	if (CL_FAILED(clErrRet))
	{
		// empty device kernels list
		for( map<cl_device_id, DeviceKernel*>::iterator it = m_mapDeviceKernels.begin(); it != m_mapDeviceKernels.end(); it++)
		{
			delete it->second;
		}
		m_mapDeviceKernels.clear();
		return clErrRet;
	}

	// set the kernel prototype for the current kernel
	if (NULL != pDeviceKernel)
	{
		SKernelPrototype sKernelPrototype = (SKernelPrototype)pDeviceKernel->GetPrototype();
		SetKernelPrototype(sKernelPrototype);
	}

	if (m_mapDeviceKernels.size() == 0)
	{
		return CL_INVALID_PROGRAM_EXECUTABLE;
	}

	//cl_return CL_SUCCESS;
	return CL_SUCCESS;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// Kernel::AddDeviceKernel
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Kernel::AddDeviceKernel(cl_dev_kernel clDeviceKernel, ProgramBinary *pProgBin)
{
	LOG_DEBUG(L"Enter AddDeviceKernel (clDeviceKernel=%d, pProgBin=%d)", clDeviceKernel, pProgBin);

#ifdef _DEBUG
	assert(NULL != pProgBin);
	assert(NULL != pProgBin->GetDevice());
#endif

	cl_err_code clErr = CL_SUCCESS;
	SKernelPrototype sKernelPrototype;

	cl_device_id iDeviceId = (cl_device_id)pProgBin->GetDevice()->GetId();

	// check if the current device kernel already exists
	//bool bResult = m_pDeviceKernels->IsExists((cl_int)clDeviceKernel);
	if (m_mapDeviceKernels.find(iDeviceId) != m_mapDeviceKernels.end())
	{
		return CL_SUCCESS;
	}

	// create new device kernel
	DeviceKernel * pDeviceKernel = new DeviceKernel(this, pProgBin, m_psKernelName, GET_LOGGER_CLIENT, &clErr);
	if (CL_FAILED(clErr))
	{
		LOG_ERROR(L"new DeviceKernel failed - returned %ws", ClErrTxt(clErr));
		if(NULL != pDeviceKernel)
		{
			delete pDeviceKernel;
		}
		return clErr;
	}

	// move through all attached device kernel and check definition
	for(map<cl_device_id, DeviceKernel*>::iterator it = m_mapDeviceKernels.begin(); it != m_mapDeviceKernels.end(); it++)
	{
		bool bResult = pDeviceKernel->CheckKernelDefinition(it->second);
		if (false == bResult)
		{
			LOG_ERROR(L"CheckKernelDefinition with device kernel %d = false", it->second->GetId());
			delete pDeviceKernel;
			return CL_INVALID_KERNEL_DEFINITION;
		}
	}

	// set kernel prototype
	sKernelPrototype = (SKernelPrototype)pDeviceKernel->GetPrototype();
	clErr = SetKernelPrototype(sKernelPrototype);
	if (CL_FAILED(clErr))
	{
		delete pDeviceKernel;
		return clErr;
	}

	m_mapDeviceKernels[iDeviceId] = pDeviceKernel;
	return clErr;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Kernel::Release
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Kernel::Release()
{
	cl_err_code clErr = OCLObject::Release();
	if (CL_FAILED(clErr))
	{
		return clErr;
	}
	return CL_SUCCESS;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// Kernel::SetKernelPrototype
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Kernel::SetKernelPrototype(SKernelPrototype sKernelPrototype)
{
	//cl_start;

	cl_err_code clErr = CL_SUCCESS;
	size_t szNameLength = 0;
	if (NULL == m_sKernelPrototype.m_psKernelName)
	{
		if (NULL != sKernelPrototype.m_psKernelName)
		{
			szNameLength = strlen(sKernelPrototype.m_psKernelName) + 1;
			m_sKernelPrototype.m_psKernelName = new char[szNameLength];
			if (NULL == m_sKernelPrototype.m_psKernelName)
			{
				return CL_OUT_OF_HOST_MEMORY;
			}
			strcpy_s(m_sKernelPrototype.m_psKernelName, szNameLength, sKernelPrototype.m_psKernelName);
		}
		m_sKernelPrototype.m_uiArgsCount = sKernelPrototype.m_uiArgsCount;
	}

	if (NULL == m_sKernelPrototype.m_pArgs)
	{
		m_sKernelPrototype.m_pArgs = new cl_kernel_argument[m_sKernelPrototype.m_uiArgsCount];
		if (NULL == m_sKernelPrototype.m_pArgs)
		{
			return CL_OUT_OF_HOST_MEMORY;
		}
		memcpy_s(m_sKernelPrototype.m_pArgs, m_sKernelPrototype.m_uiArgsCount*sizeof(cl_kernel_argument), sKernelPrototype.m_pArgs, sKernelPrototype.m_uiArgsCount*sizeof(cl_kernel_argument));
	}
	//cl_return CL_SUCCESS;
	return CL_SUCCESS;
}
cl_err_code Kernel::SetKernelArg(cl_uint uiIndex, size_t szSize, const void * pValue)
{
	//cl_start;

	LOG_DEBUG(L"Enter SetKernelArg (uiIndex=%d, szSize=%d, pValue=%d", uiIndex, szSize, pValue);

#ifdef _DEBUG
	assert ( m_pProgram != NULL );
	assert ( m_pProgram->GetContext() != NULL );
#endif

	// check argument's index
	if (uiIndex > m_sKernelPrototype.m_uiArgsCount - 1)
	{
		return CL_INVALID_ARG_INDEX;
	}

	// TODO: check for NULL and __local / __global / ... qualifier missmatches

	// check for invalid arg sizes
	cl_kernel_argument clArg = m_sKernelPrototype.m_pArgs[uiIndex];
	cl_kernel_arg_type clArgType = clArg.type;
	size_t szArgSize = clArg.size_in_bytes;

	bool bIsMemObj = false;
	bool bIsSampler = false;

	cl_err_code clErr = CL_SUCCESS;
	Context * pContext = (Context*)m_pProgram->GetContext();
	Sampler * pSampler = NULL;

	// check first if this is a sampler - we have to check the type through the pointer because the device
	// identify the sampler parameter as CL_KRNL_ARG_INT
	if ((pValue != NULL) && CL_SUCCEEDED(pContext->GetSampler(*((cl_sampler*)pValue), &pSampler)))
	{
		if (CL_SUCCEEDED(clErr))
		{
			if ( !((clArgType == CL_KRNL_ARG_SAMPLER) ||
				 (clArgType == CL_KRNL_ARG_INT) )      ||
				 (sizeof(cl_sampler) != szSize))
			{
				return CL_INVALID_ARG_SIZE;
			}
			bIsSampler = true;
		}
	}

	// memory object
	else if ((clArgType == CL_KRNL_ARG_PTR_GLOBAL)	||
	    (clArgType == CL_KRNL_ARG_PTR_CONST)	||
		(clArgType == CL_KRNL_ARG_PTR_IMG_2D)	||
		(clArgType == CL_KRNL_ARG_PTR_IMG_3D))
	{
		if (sizeof(cl_mem) != szSize)
		{
			return CL_INVALID_ARG_SIZE;
		}
		bIsMemObj = true;
	}

	else if (clArgType == CL_KRNL_ARG_PTR_LOCAL)
	{
	}

	else
	{	// other type = check size
		if (szSize != szArgSize)
		{
			return CL_INVALID_ARG_SIZE;
		}
	}

	// set arguments
	MemoryObject * pMemObj = NULL;
	KernelArg * pKernelArg = NULL;
	
	if (true == bIsMemObj)
	{
		if (NULL != pValue)
		{
			// value is not NULL - get memory object from context
			cl_mem clMemId = *((cl_mem*)(pValue));
			clErr = pContext->GetMemObject(clMemId, &pMemObj);
			if (CL_FAILED(clErr))
			{
				return CL_INVALID_MEM_OBJECT;
			}
			// TODO: check Memory properties
			pKernelArg = new KernelArg(uiIndex, sizeof(MemoryObject*), pMemObj, clArg);
		}
		else
		{
			pKernelArg = new KernelArg(uiIndex, sizeof(MemoryObject*), NULL, clArg);
		}
		m_mapKernelArgs[uiIndex] = pKernelArg;
		//cl_return CL_SUCCESS;
		return CL_SUCCESS;
	}
	else if (true == bIsSampler)
	{
		if (NULL == pValue)
		{
			return CL_INVALID_SAMPLER;
		}
		cl_sampler clSamplerId = *((cl_sampler*)(pValue));
		clErr = pContext->GetSampler(clSamplerId, &pSampler);
		if (CL_FAILED(clErr))
		{
			return CL_INVALID_SAMPLER;
		}
		clArg.type = CL_KRNL_ARG_SAMPLER;
		clArg.size_in_bytes = 0;
		pKernelArg = new KernelArg(uiIndex, sizeof(Sampler*), pSampler, clArg);
		m_mapKernelArgs[uiIndex] = pKernelArg;
		//cl_return CL_SUCCESS;
		return CL_SUCCESS;
	}

	pKernelArg = new KernelArg(uiIndex, szSize, (void*)pValue, clArg);
	m_mapKernelArgs[uiIndex] = pKernelArg;
	//cl_return CL_SUCCESS;
	return CL_SUCCESS;
}
const Context * Kernel::GetContext() const
{
	return m_pProgram->GetContext(); 
}
size_t Kernel::GetKernelArgsCount() const
{
	return m_mapKernelArgs.size();
}

bool Kernel::IsValidKernelArgs() const
{
    return ( m_mapKernelArgs.size() == m_sKernelPrototype.m_uiArgsCount);
}

const KernelArg * Kernel::GetKernelArg(cl_uint uiIndex)
{
	if (uiIndex >= m_mapKernelArgs.size())
	{
		return NULL;
	}
	map<cl_uint, KernelArg*>::iterator it = m_mapKernelArgs.find(uiIndex);
	if (it != m_mapKernelArgs.end())
	{
		return it->second;
	}
	return NULL;
}
cl_dev_kernel Kernel::GetDeviceKernelId(cl_device_id clDeviceId)
{
	map<cl_device_id, DeviceKernel*>::iterator it = m_mapDeviceKernels.find(clDeviceId);
	return (it == m_mapDeviceKernels.end()) ? CL_INVALID_HANDLE : it->second->GetId();
}

bool Kernel::IsValidExecutable(cl_device_id clDeviceId)
{
	LOG_DEBUG(L"Enter IsValidExecutable(clDeviceId=%d)", clDeviceId);
	
	std::map<cl_device_id, DeviceKernel*>::iterator it = m_mapDeviceKernels.find(clDeviceId);
	return (it != m_mapDeviceKernels.end());
}