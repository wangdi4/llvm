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

///////////////////////////////////////////////////////////////////////////////////////////////////
// DeviceKernel C'tor
///////////////////////////////////////////////////////////////////////////////////////////////////
DeviceKernel::DeviceKernel(Kernel * pKernel, ProgramBinary * pProgBin, const char * psKernelName, cl_err_code * pErr)
{
	m_pLoggerClient = new LoggerClient(L"DeviceKernel", LL_DEBUG);
	InfoLog(m_pLoggerClient, L"DeviceKernel C'tor enter");

	if (NULL == pKernel || NULL == pProgBin || NULL == psKernelName || NULL == pProgBin->GetDevice())
	{
		ErrLog(m_pLoggerClient, L"NULL == pProgBin || NULL == pKernel || NULL == psKernelName || NULL == pProgBin->GetDevice()");
		if (NULL != pErr)
		{
			*pErr = CL_INVALID_VALUE;
		}
		return;
	}

	m_pKernel = pKernel;
	m_pDevice = (Device*)pProgBin->GetDevice();

	// update kernel prototype
	m_sKernelPrototype.m_psKernelName = new char[strlen(psKernelName) + 1];
	if (NULL == m_sKernelPrototype.m_psKernelName)
	{
		ErrLog(m_pLoggerClient, L"new char[%d] == NULL", strlen(psKernelName) + 1);
		if (NULL != pErr)
		{
			*pErr = CL_OUT_OF_HOST_MEMORY;
		}
		return;
	}
	strcpy_s(m_sKernelPrototype.m_psKernelName, strlen(psKernelName) + 1, psKernelName);

	// get kernel id
	cl_err_code clErrRet = m_pDevice->GetKernelId(pProgBin->GetId(), m_sKernelPrototype.m_psKernelName, &m_clDevKernel);
	if (CL_FAILED(clErrRet))
	{
		if (NULL != pErr)
		{
			*pErr = clErrRet;
		}
		return;
	}

	// get kernel prototype
	size_t szArgsCount = 0;
	clErrRet = m_pDevice->GetKernelInfo(m_clDevKernel, CL_DEV_KERNEL_PROTOTYPE, 0, NULL, &szArgsCount);
	if (CL_FAILED(clErrRet))
	{
		if (NULL != pErr)
		{
			*pErr = clErrRet;
		}
		return;
	}
	m_sKernelPrototype.m_uiArgsCount = szArgsCount / sizeof(cl_kernel_argument);
	m_sKernelPrototype.m_pArgs = new cl_kernel_argument[m_sKernelPrototype.m_uiArgsCount];
	if (NULL == m_sKernelPrototype.m_pArgs)
	{
		if (NULL != pErr)
		{
			*pErr = CL_OUT_OF_HOST_MEMORY;
		}
		return;
	}
	clErrRet = m_pDevice->GetKernelInfo(m_clDevKernel, CL_DEV_KERNEL_PROTOTYPE, szArgsCount, m_sKernelPrototype.m_pArgs, NULL);
	if (CL_FAILED(clErrRet))
	{
		if (NULL != pErr)
		{
			*pErr = clErrRet;
		}
		return;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// DeviceKernel D'tor
///////////////////////////////////////////////////////////////////////////////////////////////////
DeviceKernel::~DeviceKernel()
{
	InfoLog(m_pLoggerClient, L"DeviceKernel D'tor enter");
	delete[] m_sKernelPrototype.m_psKernelName;
	delete[] m_sKernelPrototype.m_pArgs;
	delete m_pLoggerClient;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// DeviceKernel D'tor
///////////////////////////////////////////////////////////////////////////////////////////////////
bool DeviceKernel::CheckKernelDefinition(DeviceKernel * pKernel)
{
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
	return true;
}
KernelArg::KernelArg(cl_uint uiIndex, size_t szSize, void * pValue, cl_kernel_argument clKernelArgType)
{
	m_uiIndex = uiIndex;
	m_szSize = szSize;
	m_pValue = pValue;
	m_clKernelArgType = clKernelArgType;
}
KernelArg::~KernelArg()
{
}
bool KernelArg::IsBuffer() const
{
	return ((m_clKernelArgType.type == CL_KRNL_ARG_PTR_GLOBAL)	|| 
			(m_clKernelArgType.type == CL_KRNL_ARG_PTR_CONST)	||
			(m_clKernelArgType.type == CL_KRNL_ARG_PTR_LOCAL));
}
bool KernelArg::IsImage() const
{
	return ((m_clKernelArgType.type == CL_KRNL_ARG_PTR_IMG_RO)	|| 
			(m_clKernelArgType.type == CL_KRNL_ARG_PTR_IMG_WO));
}
bool KernelArg::IsSampler() const
{
	return (m_clKernelArgType.type == CL_KRNL_ARG_PTR_SAMPL);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// Kernel C'tor
///////////////////////////////////////////////////////////////////////////////////////////////////
Kernel::Kernel(Program * pProgram, const char * psKernelName)
{
	m_pLoggerClient = new LoggerClient(L"Kernel", LL_DEBUG);
	InfoLog(m_pLoggerClient, L"Kernel C'tor enter");
	m_pProgram = pProgram;
	m_psKernelName = new char[strlen(psKernelName) + 1];
	if (NULL != m_psKernelName)
	{
		strcpy_s(m_psKernelName, strlen(psKernelName) + 1, psKernelName);
	}
	m_pDeviceKernels = new OCLObjectsMap();

	m_sKernelPrototype.m_psKernelName = NULL;
	m_sKernelPrototype.m_pArgs = NULL;
	m_sKernelPrototype.m_uiArgsCount = 0;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// Kernel D'tor
///////////////////////////////////////////////////////////////////////////////////////////////////
Kernel::~Kernel()
{
	InfoLog(m_pLoggerClient, L"Kernel D'tor enter");
	delete m_psKernelName;
	delete m_pDeviceKernels;
	delete m_pLoggerClient;

	// release kernel prototype
	delete[] m_sKernelPrototype.m_psKernelName;
	delete[] m_sKernelPrototype.m_pArgs;

	DeviceKernel * pDeviceKerenl = NULL;
	for (cl_uint ui=0; ui<m_pDeviceKernels->Count(); ++ui)
	{
		cl_err_code clErr = m_pDeviceKernels->GetObjectByIndex(ui, (OCLObject**)&pDeviceKerenl);
		if (CL_SUCCEEDED(clErr) && NULL != pDeviceKerenl)
		{
			delete pDeviceKerenl;
		}
	}
	m_pDeviceKernels->Clear();

	// delete kerenl arguments
	map<cl_uint,KernelArg*>::iterator it = m_mapKernelArgs.begin();
	while (it != m_mapKernelArgs.end())
	{
		KernelArg * pKernelArg = it->second;
		if (NULL != pKernelArg)
		{
			delete pKernelArg;
		}
	}
	m_mapKernelArgs.clear();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// Kernel::GetInfo
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code	Kernel::GetInfo(cl_int iParamName, size_t szParamValueSize, void * pParamValue, size_t * pszParamValueSizeRet)
{
	InfoLog(m_pLoggerClient, L"Enter Kernel::GetInfo (param_nameiParamNamed, szParamValueSize=%d, pParamValue=%d, pszParamValueSizeRet=%d)",
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
// Kernel::CreateDeviceKernels
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Kernel::CreateDeviceKernels(cl_uint uiBinariesCount, ProgramBinary ** ppBinaries)
{
	InfoLog(m_pLoggerClient, L"Enter AddDeviceKernel (uiBinariesCount=%d, ppBinaries=%d)", 
		uiBinariesCount, ppBinaries);
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
			DbgLog(m_pLoggerClient, L"NULL == ppBinaries[%d] || NULL == ppBinaries[%d]->GetDevice()", ui, ui);
			break;
		}
		// get build status and check that there is a valid binary;
		cl_build_status clBuildStatus = (cl_build_status)ppBinaries[ui]->GetStatus();
		if (clBuildStatus != CL_BUILD_SUCCESS)
		{
			break;
		}
		pDevice = (Device*)ppBinaries[ui]->GetDevice();
		if (m_pDeviceKernels->IsExists(pDevice->GetId()))
		{
			DbgLog(m_pLoggerClient, L"m_pDeviceKernels->IsExists(%d) = true", pDevice->GetId());
			break;
		}
		
		// create the device kernel object
		pDeviceKernel = new DeviceKernel(this, ppBinaries[ui], m_psKernelName, &clErrRet);
		if (CL_FAILED(clErrRet))
		{
			ErrLog(m_pLoggerClient, L"new DeviceKernel(...) faild (returned %ws)", ClErrTxt(clErrRet));
			break;
		}
		
		// check kernel definition - compare previous kernel to the next one
		if (NULL != pPrevDeviceKernel)
		{
			bResult = pDeviceKernel->CheckKernelDefinition(pPrevDeviceKernel);
			if (false == bResult)
			{
				ErrLog(m_pLoggerClient, L"CheckKernelDefinition failed (returned false)");
				delete pDeviceKernel;
				clErrRet = CL_INVALID_KERNEL_DEFINITION;
				break;
			}
		}

		// update previous device kernel pointer
		pPrevDeviceKernel = pDeviceKernel;
		
		// add new device kernel to the objects map list
		clErrRet = m_pDeviceKernels->AddObject((OCLObject*)pDeviceKernel, pDevice->GetId(), false);
		if (CL_FAILED(clErrRet))
		{
			clErrRet = CL_INVALID_VALUE;
			delete pDeviceKernel;
			break;
		}
	}
	
	if (CL_FAILED(clErrRet))
	{
		// empty device kernels list
		for (cl_uint ui=0; ui<m_pDeviceKernels->Count(); ++ui)
		{
			clErrRet = m_pDeviceKernels->GetObjectByIndex(ui, (OCLObject**)&pDeviceKernel);
			if (CL_SUCCEEDED(clErrRet))
			{
				delete pDeviceKernel;
			}
		}
		m_pDeviceKernels->Clear();
		return clErrRet;
	}

	// set the kernel prototype for the current kernel
	if (NULL != pDeviceKernel)
	{
		SKernelPrototype sKernelPrototype = (SKernelPrototype)pDeviceKernel->GetPrototype();
		SetKernelPrototype(sKernelPrototype);
	}

	if (m_pDeviceKernels->Count() == 0)
	{
		return CL_INVALID_PROGRAM_EXECUTABLE;
	}

	return CL_SUCCESS;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// Kernel::AddDeviceKernel
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Kernel::AddDeviceKernel(cl_dev_kernel clDeviceKernel, ProgramBinary *pProgBin)
{
	InfoLog(m_pLoggerClient, L"Enter AddDeviceKernel (clDeviceKernel=%d, pProgBin=%d)", clDeviceKernel, pProgBin);

#ifdef _DEBUG
	assert(NULL != pProgBin);
	assert(NULL != pProgBin->GetDevice());
#endif

	cl_err_code clErr = CL_SUCCESS;
	SKernelPrototype sKernelPrototype;

	// check if the current device kernel already exists
	bool bResult = m_pDeviceKernels->IsExists((cl_int)clDeviceKernel);
	if (true == bResult)
	{
		return CL_SUCCESS;
	}

	// create new device kernel
	DeviceKernel * pDeviceKernel = new DeviceKernel(this, pProgBin, m_psKernelName, &clErr);
	if (CL_FAILED(clErr))
	{
		ErrLog(m_pLoggerClient, L"new DeviceKernel failed - returned %ws", ClErrTxt(clErr));
		return clErr;
	}

	// move through all attached device kernel and check definition
	for (cl_uint ui=0; ui<m_pDeviceKernels->Count(); ++ui)
	{
		DeviceKernel *pDevKer = NULL;
		clErr = m_pDeviceKernels->GetObjectByIndex(ui, (OCLObject**)&pDevKer);
		if (CL_SUCCEEDED(clErr))
		{
			bResult = pDeviceKernel->CheckKernelDefinition(pDevKer);
			if (false == bResult)
			{
				ErrLog(m_pLoggerClient, L"CheckKernelDefinition with device kernel %d = false", pDevKer->GetId());
				delete pDeviceKernel;
				return CL_INVALID_KERNEL_DEFINITION;
			}
		}
	}

	// set kernel prototype
	sKernelPrototype = (SKernelPrototype)pDeviceKernel->GetPrototype();
	clErr = SetKernelPrototype(sKernelPrototype);
	if (CL_FAILED(clErr))
	{
		return clErr;
	}

	clErr = m_pDeviceKernels->AddObject((OCLObject*)pDeviceKernel, (cl_int)clDeviceKernel, false);
	if (CL_FAILED(clErr))
	{
		delete pDeviceKernel;
	}
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
		memcpy_s(m_sKernelPrototype.m_pArgs, m_sKernelPrototype.m_uiArgsCount, sKernelPrototype.m_pArgs, sKernelPrototype.m_uiArgsCount);
	}
	return CL_SUCCESS;
}
cl_err_code Kernel::SetKernelArg(cl_uint uiIndex, size_t szSize, const void * pValue)
{
	InfoLog(m_pLoggerClient, L"Enter SetKernelArg (uiIndex=%d, szSize=%d, pValue=%d", uiIndex, szSize, pValue);

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
	
	if (sizeof(cl_mem) == szSize)
	{	
		// ivalid size of memory object
		if ((clArgType != CL_KRNL_ARG_PTR_GLOBAL)	&&
			(clArgType != CL_KRNL_ARG_PTR_LOCAL)	&&
			(clArgType != CL_KRNL_ARG_PTR_CONST)	&&
			(clArgType != CL_KRNL_ARG_PTR_IMG_RO)	&&
			(clArgType != CL_KRNL_ARG_PTR_IMG_WO))
		{
			return CL_INVALID_ARG_SIZE;
		}
	}
	else if (sizeof(cl_sampler) == szSize)
	{
		// TODO: Check for sampler size
		if (clArgType != CL_KRNL_ARG_PTR_SAMPL)
		{
			return CL_INVALID_ARG_SIZE;
		}
	}
	else
	{	// other type = check size
		if (szSize != szArgSize)
		{
			return CL_INVALID_ARG_SIZE;
		}
	}

	// set arguments
	cl_err_code clErr = CL_SUCCESS;
	Context * pContext = (Context*)m_pProgram->GetContext();
	MemoryObject * pMemObj = NULL;
	KernelArg * pKernelArg = NULL;
	
	if (szSize == sizeof(cl_mem))
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
		return CL_SUCCESS;
	}
	if (szSize == sizeof(cl_sampler))
	{
		// TODO: handle sampler arguments
	}

	pKernelArg = new KernelArg(uiIndex, szSize, (void*)pValue, clArg);
	m_mapKernelArgs[uiIndex] = pKernelArg;
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
cl_dev_kernel Kernel::GetDeviceKernelId(cl_device_id clDeviceId) const
{
	DeviceKernel * pDeviceKernel = NULL;
	cl_err_code clErr = m_pDeviceKernels->GetOCLObject((cl_int)clDeviceId, (OCLObject**)&pDeviceKernel);
	if (CL_FAILED(clErr) || NULL == pDeviceKernel)
	{
		return CL_INVALID_HANDLE;
	}
	return pDeviceKernel->GetId();
}