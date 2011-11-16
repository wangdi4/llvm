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
#include "Context.h"
#include "program.h"
#include "cl_sys_defines.h"
#include <cl_objects_map.h>
#include <Device.h>
#include <assert.h>
#include <cl_utils.h>

using namespace Intel::OpenCL::Utils;
using namespace Intel::OpenCL::Framework;


using namespace std;

///////////////////////////////////////////////////////////////////////////////////////////////////
// DeviceKernel C'tor
///////////////////////////////////////////////////////////////////////////////////////////////////
DeviceKernel::DeviceKernel(Kernel *        pKernel, 
						   FissionableDevice *        pDevice,
						   cl_dev_program  devProgramId,
						   const char *    psKernelName, 
						   LoggerClient *  pLoggerClient,
						   cl_err_code *   pErr) 
: OCLObjectBase("DeviceKernel"), m_clDevKernel(CL_INVALID_HANDLE), m_pKernel(pKernel), m_pDevice(pDevice)
{
	assert ( pErr != NULL );

	SET_LOGGER_CLIENT(pLoggerClient);
	LOG_DEBUG(TEXT("%S"), TEXT("DeviceKernel C'tor enter"));
    m_sKernelPrototype.m_psKernelName = NULL;
	m_sKernelPrototype.m_uiArgsCount  = 0;
	m_sKernelPrototype.m_pArgs        = NULL;

	if (NULL == m_pKernel || NULL == m_pDevice || NULL == psKernelName || CL_INVALID_HANDLE == devProgramId)
	{
		LOG_ERROR(TEXT("%S"), TEXT("NULL == m_pKernel || NULL == m_pDevice || NULL == psKernelName || CL_INVALID_HANDLE == devProgramId"));
		*pErr = CL_INVALID_VALUE;
		return;
	}

	m_pDevice->AddPendency(this);

	// update kernel prototype
	size_t szNameLength = strlen(psKernelName) + 1;
	m_sKernelPrototype.m_psKernelName = new char[szNameLength];
	if (NULL == m_sKernelPrototype.m_psKernelName)
	{
		LOG_ERROR(TEXT("new char[%d] == NULL"), strlen(psKernelName) + 1);
		*pErr = CL_OUT_OF_HOST_MEMORY;
		return;
	}

	// copy kernel name;
	STRCPY_S(m_sKernelPrototype.m_psKernelName, szNameLength, psKernelName);

	// get kernel id
	cl_dev_err_code clErrRet = m_pDevice->GetDeviceAgent()->clDevGetKernelId(devProgramId, m_sKernelPrototype.m_psKernelName, &m_clDevKernel);
	if (CL_DEV_FAILED(clErrRet))
	{
		LOG_ERROR(TEXT("%S"), TEXT("Device->GetKernelId failed"));
		delete[] m_sKernelPrototype.m_psKernelName;
		m_sKernelPrototype.m_psKernelName = NULL;
		*pErr = (clErrRet == CL_DEV_INVALID_KERNEL_NAME) ? CL_INVALID_KERNEL_NAME : CL_OUT_OF_HOST_MEMORY;
		return;
	}

	// get kernel prototype
	size_t szArgsCount = 0;
	clErrRet = m_pDevice->GetDeviceAgent()->clDevGetKernelInfo(m_clDevKernel, CL_DEV_KERNEL_PROTOTYPE, 0, NULL, &szArgsCount);
	if (CL_DEV_FAILED(clErrRet))
	{
		*pErr = clErrRet;
		return;
	}
	assert(szArgsCount / sizeof(cl_kernel_argument) <= CL_MAX_UINT32);
	m_sKernelPrototype.m_uiArgsCount = (cl_uint)(szArgsCount / sizeof(cl_kernel_argument));
	m_sKernelPrototype.m_pArgs = new cl_kernel_argument[m_sKernelPrototype.m_uiArgsCount];
	if (NULL == m_sKernelPrototype.m_pArgs)
	{
		*pErr = CL_OUT_OF_HOST_MEMORY;
		delete[] m_sKernelPrototype.m_psKernelName;
		m_sKernelPrototype.m_psKernelName = NULL;
		return;
	}

	clErrRet = m_pDevice->GetDeviceAgent()->clDevGetKernelInfo(m_clDevKernel, CL_DEV_KERNEL_PROTOTYPE, szArgsCount, m_sKernelPrototype.m_pArgs, NULL);
	if (CL_DEV_FAILED(clErrRet))
	{
		*pErr = (clErrRet == CL_DEV_INVALID_KERNEL_NAME) ? CL_INVALID_KERNEL_NAME : CL_OUT_OF_HOST_MEMORY;
		return;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// DeviceKernel D'tor
///////////////////////////////////////////////////////////////////////////////////////////////////
DeviceKernel::~DeviceKernel()
{
	LOG_DEBUG(TEXT("%S"), TEXT("DeviceKernel D'tor enter"));
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
	m_pDevice->RemovePendency(this);
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

	return true;
}


KernelArg::KernelArg(cl_uint uiIndex, size_t szSize, void * pValue, cl_kernel_argument clKernelArgType) :
m_uiIndex(uiIndex), m_szSize(szSize), m_pValue(pValue), m_clKernelArgType(clKernelArgType)
{
	if (m_clKernelArgType.type == CL_KRNL_ARG_COMPOSITE)
	{
		m_pValue = new char[m_szSize];
		MEMCPY_S(m_pValue, m_szSize, pValue, szSize);
		return;
	}
	if (m_clKernelArgType.type <= CL_KRNL_ARG_VECTOR)
	{
		m_pValue = new char[m_szSize];
		MEMCPY_S(m_pValue, m_szSize, pValue, szSize);
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
	if (NULL != m_pValue)
	{
		if (m_clKernelArgType.type <= CL_KRNL_ARG_VECTOR)
		{
			char* temp = (char*)m_pValue;
			delete [] temp;
			m_pValue = NULL;
		}
		else if (CL_KRNL_ARG_PTR_LOCAL == m_clKernelArgType.type)
		{
			delete (void**)m_pValue;
			m_pValue = NULL;
		}
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

bool KernelArg::IsLocalPtr() const
{
	return (m_clKernelArgType.type == CL_KRNL_ARG_PTR_LOCAL);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Kernel C'tor
///////////////////////////////////////////////////////////////////////////////////////////////////
Kernel::Kernel(Program * pProgram, const char * psKernelName, size_t szNumDevices, ocl_entry_points * pOclEntryPoints)
: OCLObject<_cl_kernel_int>("Kernel"), m_pProgram(pProgram), m_szAssociatedDevices(szNumDevices), m_ppArgs(NULL), m_numValidArgs(0)
{
    // Sign to be dependent on the program, ensure the program will be deleted only after the object is
    m_pProgram->AddPendency(this);

	size_t szNameLength = strlen(psKernelName) + 1;
	m_sKernelPrototype.m_psKernelName = new char[szNameLength];
	//Todo: what if allocation fails here?
	if (NULL != m_sKernelPrototype.m_psKernelName)
	{
		STRCPY_S(m_sKernelPrototype.m_psKernelName, szNameLength, psKernelName);
	}
        
	m_sKernelPrototype.m_pArgs = NULL;
	m_sKernelPrototype.m_uiArgsCount = 0;

	m_ppDeviceKernels = new DeviceKernel* [m_szAssociatedDevices];
	memset(m_ppDeviceKernels, 0, sizeof(DeviceKernel *) * m_szAssociatedDevices);
	//Todo: or here?

	m_handle.object   = this;
	m_handle.dispatch = (KHRicdVendorDispatch*)pOclEntryPoints;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// Kernel D'tor
///////////////////////////////////////////////////////////////////////////////////////////////////
Kernel::~Kernel()
{
	LOG_DEBUG(TEXT("%S"), TEXT("Kernel D'tor enter"));

	// release kernel prototype
	if (m_sKernelPrototype.m_psKernelName)
	{
		delete[] m_sKernelPrototype.m_psKernelName;
	}
	if (m_sKernelPrototype.m_pArgs)
	{
		delete[] m_sKernelPrototype.m_pArgs;
	}

    m_pProgram->RemovePendency(this);

	if (m_ppDeviceKernels)
	{
		// clear device kernels
		for (size_t i = 0; i < m_szAssociatedDevices; ++i)
		{
			delete m_ppDeviceKernels[i];
		}
		delete[] m_ppDeviceKernels;
	}
	if (m_ppArgs)
	{
		// delete kernel arguments
		for (cl_uint i = 0; i < m_sKernelPrototype.m_uiArgsCount; ++i)
		{
			delete m_ppArgs[i];
		}
		delete[] m_ppArgs;
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// Kernel::GetInfo
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code	Kernel::GetInfo(cl_int iParamName, size_t szParamValueSize, void * pParamValue, size_t * pszParamValueSizeRet)
{
	LOG_DEBUG(TEXT("Enter Kernel::GetInfo (iParamName=%d, szParamValueSize=%d, pParamValue=%d, pszParamValueSizeRet=%d)"),
		iParamName, szParamValueSize, pParamValue, pszParamValueSizeRet);

	if (NULL == pParamValue && NULL == pszParamValueSizeRet)
	{
		return CL_INVALID_VALUE;
	}
	size_t szParamSize = 0;
	void * pValue = NULL;
	cl_ulong iParam = 0;
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
			iParam = (cl_long)(const_cast<Context*>(m_pProgram->GetContext())->GetHandle());
			pValue = &iParam;
		}
		break;
	case CL_KERNEL_PROGRAM:
		if (NULL != m_pProgram)
		{
			szParamSize = sizeof(cl_program);
			iParam = (cl_ulong)m_pProgram->GetHandle();
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
		MEMCPY_S(pParamValue, szParamValueSize, pValue, szParamSize);
	}
	
	return CL_SUCCESS;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// Kernel::GetWorkGroupInfo
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code	Kernel::GetWorkGroupInfo(FissionableDevice* pDevice, cl_int iParamName, size_t szParamValueSize, void * pParamValue, size_t * pszParamValueSizeRet)
{
	LOG_DEBUG(TEXT("Enter Kernel::GetWorkGroupInfo (pDevice=%p, iParamName=%d, szParamValueSize=%d, pParamValue=%d, pszParamValueSizeRet=%d)"),
		pDevice, iParamName, szParamValueSize, pParamValue, pszParamValueSizeRet);

#ifdef _DEBUG
	assert ( "No context assigned to the kernel" && (NULL != m_pProgram) && (NULL != m_pProgram->GetContext()) );
#endif

	// check input parameters
	if (NULL == pParamValue && NULL == pszParamValueSizeRet)
	{
		return CL_INVALID_VALUE;
	}

	//get device
	cl_dev_kernel clDevKernel = GetDeviceKernelId(pDevice);

	cl_err_code clErr = CL_SUCCESS;
	switch (iParamName)
	{
	case CL_KERNEL_WORK_GROUP_SIZE:
		clErr = pDevice->GetDeviceAgent()->clDevGetKernelInfo(clDevKernel, CL_DEV_KERNEL_MAX_WG_SIZE, szParamValueSize, pParamValue, pszParamValueSizeRet);
		break;
	case CL_KERNEL_COMPILE_WORK_GROUP_SIZE:
		clErr = pDevice->GetDeviceAgent()->clDevGetKernelInfo(clDevKernel, CL_DEV_KERNEL_WG_SIZE_REQUIRED, szParamValueSize, pParamValue, pszParamValueSizeRet);
		break;
	case CL_KERNEL_LOCAL_MEM_SIZE:
		clErr = pDevice->GetDeviceAgent()->clDevGetKernelInfo(clDevKernel, CL_DEV_KERNEL_IMPLICIT_LOCAL_SIZE, szParamValueSize, pParamValue, pszParamValueSizeRet);
		break;
	case CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE:
		clErr = pDevice->GetDeviceAgent()->clDevGetKernelInfo(clDevKernel, CL_DEV_KERNEL_WG_SIZE, szParamValueSize, pParamValue, pszParamValueSizeRet);
		break;
	case CL_KERNEL_PRIVATE_MEM_SIZE:
		clErr = pDevice->GetDeviceAgent()->clDevGetKernelInfo(clDevKernel, CL_DEV_KERNEL_PRIVATE_SIZE, szParamValueSize, pParamValue, pszParamValueSizeRet);
		break;

	default:
		clErr = CL_INVALID_VALUE;
	}

	if( (signed)CL_DEV_INVALID_KERNEL == clErr )
	{
		clErr = CL_INVALID_KERNEL;
	}
	if( (signed)CL_DEV_INVALID_VALUE == clErr )
	{
		clErr = CL_INVALID_VALUE;
	}

	return clErr;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Kernel::CreateDeviceKernels
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Kernel::CreateDeviceKernels(DeviceProgram** ppDevicePrograms)
{
	if (NULL == ppDevicePrograms)
	{
		return CL_INVALID_VALUE;
	}
	
	cl_err_code clErrRet = CL_SUCCESS;
	DeviceKernel * pDeviceKernel = NULL, * pPrevDeviceKernel = NULL;
	bool bResult = false;
	size_t i;
	
	for(i = 0; i < m_szAssociatedDevices; ++i)
	{

		// get build status and check that there is a valid binary;
		cl_build_status clBuildStatus = ppDevicePrograms[i]->GetBuildStatus();
		if (clBuildStatus != CL_BUILD_SUCCESS)
		{
            continue;
		}
		const FissionableDevice* pDevice = ppDevicePrograms[i]->GetDevice();
		if (NULL != GetDeviceKernel(pDevice))
		{
			LOG_ERROR(TEXT("Already have a kernel for device ID(%d)"), pDevice->GetId());
			continue;
		}
		
		// create the device kernel object
		pDeviceKernel = new DeviceKernel(this, const_cast<FissionableDevice*>(ppDevicePrograms[i]->GetDevice()), ppDevicePrograms[i]->GetDeviceProgramHandle(), m_sKernelPrototype.m_psKernelName, GET_LOGGER_CLIENT, &clErrRet);
		if (!pDeviceKernel)
		{
			clErrRet = CL_OUT_OF_HOST_MEMORY;
		}
		if (CL_FAILED(clErrRet))
		{
			LOG_ERROR(TEXT("new DeviceKernel(...) failed (returned %S)"), ClErrTxt(clErrRet));
			delete pDeviceKernel;
			break;
		}
		
		// check kernel definition - compare previous kernel to the next one
		if (NULL != pPrevDeviceKernel)
		{
			bResult = pDeviceKernel->CheckKernelDefinition(pPrevDeviceKernel);
			if (false == bResult)
			{
				LOG_ERROR(TEXT("%S"), TEXT("CheckKernelDefinition failed (returned false)"));
				delete pDeviceKernel;
				clErrRet = CL_INVALID_KERNEL_DEFINITION;
				break;
			}
		}

		// update previous device kernel pointer
		pPrevDeviceKernel = pDeviceKernel;
		
		// add new device kernel to the objects map list
		m_ppDeviceKernels[i] = pDeviceKernel;
	}
	
	if (CL_FAILED(clErrRet))
	{
		// Delete already-created device kernels
		for (size_t j = 0; j < i; ++j)
		{
			delete m_ppDeviceKernels[j];
		}
		delete [] m_ppDeviceKernels;
		m_ppDeviceKernels = NULL;
		return clErrRet;
	}

	// set the kernel prototype for the current kernel
	if (NULL != pDeviceKernel)
	{
		SKernelPrototype sKernelPrototype = (SKernelPrototype)pDeviceKernel->GetPrototype();
		SetKernelPrototype(sKernelPrototype);
	}

	return CL_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Kernel::SetKernelPrototype
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Kernel::SetKernelPrototype(SKernelPrototype sKernelPrototype)
{
	//cl_start;

	//We initialized our name at kernel creation, no need to do it again
	assert(sKernelPrototype.m_psKernelName);
	assert(m_sKernelPrototype.m_psKernelName);
	assert(0 == strcmp(sKernelPrototype.m_psKernelName, m_sKernelPrototype.m_psKernelName));

	m_sKernelPrototype.m_uiArgsCount = sKernelPrototype.m_uiArgsCount;

	if (NULL == m_sKernelPrototype.m_pArgs)
	{
		m_sKernelPrototype.m_pArgs = new cl_kernel_argument[sKernelPrototype.m_uiArgsCount];
		if (NULL == m_sKernelPrototype.m_pArgs)
		{
			return CL_OUT_OF_HOST_MEMORY;
		}
		MEMCPY_S(m_sKernelPrototype.m_pArgs, sKernelPrototype.m_uiArgsCount*sizeof(cl_kernel_argument), sKernelPrototype.m_pArgs, sKernelPrototype.m_uiArgsCount*sizeof(cl_kernel_argument));
	}
	if (NULL == m_ppArgs)
	{
		m_ppArgs = new KernelArg* [m_sKernelPrototype.m_uiArgsCount];
		memset(m_ppArgs, 0, sizeof(KernelArg *) * m_sKernelPrototype.m_uiArgsCount);
	}
	//cl_return CL_SUCCESS;
	return CL_SUCCESS;
}
cl_err_code Kernel::SetKernelArg(cl_uint uiIndex, size_t szSize, const void * pValue)
{
	//cl_start;

	LOG_DEBUG(TEXT("Enter SetKernelArg (uiIndex=%d, szSize=%d, pValue=%d"), uiIndex, szSize, pValue);

	assert ( m_pProgram != NULL );
	assert ( m_pProgram->GetContext() != NULL );

	// check argument's index
	if (uiIndex > m_sKernelPrototype.m_uiArgsCount - 1)
	{
		return CL_INVALID_ARG_INDEX;
	}

	// TODO: check for NULL and __local / __global / ... qualifier mismatches

	// check for invalid arg sizes
	cl_kernel_argument clArg = m_sKernelPrototype.m_pArgs[uiIndex];
	cl_kernel_arg_type clArgType = clArg.type;
	size_t szArgSize = clArg.size_in_bytes;

	bool bIsMemObj = false;
	bool bIsSampler = false;

	cl_err_code clErr = CL_SUCCESS;
	Context * pContext = const_cast<Context *>(m_pProgram->GetContext());
	Sampler * pSampler = NULL;

	// check first if this is a sampler - we have to check the type through the pointer because the device
	// identify the sampler parameter as CL_KRNL_ARG_INT
	if ( clArgType == CL_KRNL_ARG_SAMPLER )
	{
		 if (sizeof(cl_sampler) != szSize )
		{
			return CL_INVALID_ARG_SIZE;
		}
		bIsSampler = true;
	}

	// memory object
	else if ((clArgType == CL_KRNL_ARG_PTR_GLOBAL)	||
	    (clArgType == CL_KRNL_ARG_PTR_CONST)	||
		(clArgType == CL_KRNL_ARG_PTR_IMG_2D)	||
		(clArgType == CL_KRNL_ARG_PTR_IMG_3D)   ||
        (clArgType == CL_KRNL_ARG_PTR_IMG_2D_ARR))
	{
		if (sizeof(cl_mem) != szSize)
		{
			return CL_INVALID_ARG_SIZE;
		}
		bIsMemObj = true;
	}

	else if (clArgType == CL_KRNL_ARG_PTR_LOCAL)
	{
		if (0 == szSize)
		{
			return CL_INVALID_ARG_SIZE;
		}
		if (NULL != pValue)
		{
			return CL_INVALID_ARG_VALUE;
		}
	}

	else if (clArgType == CL_KRNL_ARG_VECTOR)
	{
		szArgSize = (szArgSize & 0xFFFF) * ((szArgSize >> 16) & 0xFFFF);
		if (szSize != szArgSize)
		{
			return CL_INVALID_ARG_SIZE;
		}
	}

	else if (clArgType == CL_KRNL_ARG_COMPOSITE)
	{
		if (szSize != szArgSize)
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
	MemoryObject * pMemObj = NULL;
	KernelArg * pKernelArg = NULL;
	
	if (true == bIsMemObj)
	{
		// TODO: Why we need this check
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

		if (NULL != m_ppArgs[uiIndex])
		{
			delete m_ppArgs[uiIndex];
		}
		else
		{
			m_numValidArgs++;
		}
		m_ppArgs[uiIndex] = pKernelArg;
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
		if (NULL != m_ppArgs[uiIndex])
		{
			delete m_ppArgs[uiIndex];
		}
		else
		{
			m_numValidArgs++;
		}
		m_ppArgs[uiIndex] = pKernelArg;
		return CL_SUCCESS;
	}

	if (NULL != m_ppArgs[uiIndex])
	{
		m_ppArgs[uiIndex]->ModifyValue(szSize, (void*)pValue);;
	}
	else
	{
		pKernelArg = new KernelArg(uiIndex, szSize, (void*)pValue, clArg);
		m_ppArgs[uiIndex] = pKernelArg;
		m_numValidArgs++;
	}

	return CL_SUCCESS;
}
const Context * Kernel::GetContext() const
{
	return m_pProgram->GetContext(); 
}
size_t Kernel::GetKernelArgsCount() const
{
	return m_sKernelPrototype.m_uiArgsCount;
}

bool Kernel::IsValidKernelArgs() const
{
	return m_numValidArgs == m_sKernelPrototype.m_uiArgsCount;
}

DeviceKernel* Kernel::GetDeviceKernel(const FissionableDevice* pDevice) const
{
	assert (m_ppDeviceKernels);
	for (size_t i = 0; i < m_szAssociatedDevices; ++i)
	{
		if (NULL != m_ppDeviceKernels[i] && pDevice->GetId() == m_ppDeviceKernels[i]->GetDeviceId())
		{
			return m_ppDeviceKernels[i];
		}
	}
	return NULL;
}

const KernelArg * Kernel::GetKernelArg(size_t uiIndex) const
{
	assert (uiIndex < m_sKernelPrototype.m_uiArgsCount);
	return m_ppArgs[uiIndex];
}

cl_dev_kernel Kernel::GetDeviceKernelId(FissionableDevice* pDevice) const
{
	DeviceKernel* pDeviceKernel = GetDeviceKernel(pDevice);
	if (pDeviceKernel)
	{
		return pDeviceKernel->GetId();
	}
	return CL_INVALID_HANDLE;
}

bool Kernel::IsValidExecutable(const FissionableDevice* pDevice) const
{
	DeviceKernel* pDeviceKernel = GetDeviceKernel(pDevice);
	return NULL != pDeviceKernel;
}

/////////////////////////////////////////////////////////////////////
// OpenCL 1.2 functions
/////////////////////////////////////////////////////////////////////

const char* Kernel::g_szArgTypeNames[] = 
{
	"int",
	"uint",
	"float",
	"double",
	"vector",
	"sampler_t",
	"composite", // Strucure
	"local*",
	"global*",
	"constant*",
	"__image_2d_t",
	"__image_3d_t",
	"__image_2d_array"
};

// TODO: Remove after 1.2 headers integration
#ifdef CL_KERNEL_ARG_ADDRESS_QUALIFIER
#pragma error
#endif
/* cl_kernel_arg_info */
#define CL_KERNEL_ARG_ADDRESS_QUALIFIER             0x1300
#define CL_KERNEL_ARG_ACCESS_QUALIFIER              0x1301
#define CL_KERNEL_ARG_TYPE_NAME                     0x1302
#define CL_KERNEL_ARG_NAME                          0x1303
#define CL_KERNEL_ARG_ATTRIBUTES                    0x1304
#define CL_KERNEL_ATTRIBUTES                        0x1305

cl_err_code Kernel::GetKernelArgInfo (	cl_uint argIndx,
								cl_kernel_arg_info paramName,
								size_t      szParamValueSize,
								void *      pParamValue,
								size_t *    pszParamValueSizeRet)
{
	size_t stParamSize;
	const void* pValue;

    if  (!pszParamValueSizeRet && !pParamValue)
    {
        return CL_INVALID_VALUE;
    }
	if ( argIndx > (GetKernelArgsCount()-1) )
	{
		return CL_INVALID_VALUE;
	}

	// Initial implementation requried by the common runtime
	switch ( paramName )
	{
	case CL_KERNEL_ARG_TYPE_NAME:
		pValue = g_szArgTypeNames[m_sKernelPrototype.m_pArgs[argIndx].type];
		stParamSize = strlen((const char*)pValue);      
		break;
	default:
		return CL_INVALID_VALUE;        
	}
    
    if (NULL != pParamValue)
    {
	    if (szParamValueSize >= (stParamSize+1))
	    {
		    MEMCPY_S(pParamValue, szParamValueSize, pValue, stParamSize);
            ((char*)pParamValue)[stParamSize] = '\0';
	    }
        else
        {
            return CL_INVALID_VALUE;
        }
    }
	
    if ( NULL != pszParamValueSizeRet )
	{
		*pszParamValueSizeRet = stParamSize+1;
	}
	return CL_SUCCESS;
}
