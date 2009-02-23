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
//  Context.cpp
//  Implementation of the Class Context
//  Created on:      10-Dec-2008 2:08:23 PM
//  Original author: ulevy
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "Context.h"
#include "program.h"
#include "cl_buffer.h"
#include <cl_utils.h>
#include <device.h>
#include <cl_objects_map.h>

// for debug...???
#include <limits.h>
#include <assert.h>
using namespace std;
using namespace Intel::OpenCL::Utils;
using namespace Intel::OpenCL::Framework;

///////////////////////////////////////////////////////////////////////////////////////////////////
// Context C'tor
///////////////////////////////////////////////////////////////////////////////////////////////////
Context::Context(cl_context_properties clProperties, cl_uint uiNumDevices, Device **ppDevices, logging_fn pfnNotify, void *pUserData)
{
	::OCLObject();
	m_pfnNotify = NULL;
	m_pUserData = NULL;

	m_pLoggerClient = new LoggerClient(L"Context", LL_DEBUG);
	InfoLog(m_pLoggerClient, L"Context constructor enter");

	m_pPrograms = new OCLObjectsMap();
	m_pDevices = new OCLObjectsMap();
	m_pMemObjects = new OCLObjectsMap();
	if (NULL == ppDevices || uiNumDevices <= 0)
	{
		return;
	}
	for (cl_uint ui=0; ui<uiNumDevices; ++ui)
	{
		m_pDevices->AddObject(ppDevices[ui], ppDevices[ui]->GetId(), false);
	}
	m_clContextProperties = clProperties;
	m_pfnNotify = pfnNotify;
	m_pUserData = pUserData;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// Context D'tor
///////////////////////////////////////////////////////////////////////////////////////////////////
Context::~Context()
{
	InfoLog(m_pLoggerClient, L"Context destructor enter");

	delete m_pLoggerClient;
	m_pLoggerClient = NULL;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// GetInfo
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Context::GetInfo(cl_int param_name, size_t param_value_size, void *param_value, size_t *param_value_size_ret)
{
	InfoLog(m_pLoggerClient, L"Context::GetInfo enter. param_name=%d, param_value_size=%d, param_value=%d, param_value_size_ret=%d", param_name, param_value_size, param_value, param_value_size_ret);
	
	if (NULL == param_value && NULL == param_value_size_ret)
	{
		return CL_INVALID_VALUE;
	}
	size_t szParamValueSize = 0;
	void * pValue = NULL;
	
	cl_err_code clErrRet = CL_SUCCESS;
	switch ( (cl_context_info)param_name )
	{

	case CL_CONTEXT_REFERENCE_COUNT:
		szParamValueSize = sizeof(cl_uint);
		pValue = &m_uiRefCount;
		break;
	case CL_CONTEXT_DEVICES:
		szParamValueSize = sizeof(cl_device_id) * m_pDevices->Count();
		pValue = new cl_device_id[m_pDevices->Count()];
		if (NULL != pValue)
		{
			clErrRet = m_pDevices->GetIDs(m_pDevices->Count(), (cl_int*)pValue, NULL);
		}
		else
		{
			clErrRet = CL_ERR_INITILIZATION_FAILED;
		}
		break;
	case CL_CONTEXT_PROPERTIES:
		szParamValueSize = sizeof(cl_context_properties);
		pValue = &m_clContextProperties;
		break;
	default:
		ErrLog(m_pLoggerClient, L"param_name (=%d) isn't valid", param_name);
		return CL_INVALID_VALUE;
	}
	if (CL_FAILED(clErrRet))
	{
		return clErrRet;
	}
	// if param_value == NULL return only param value size
	if (NULL == param_value)
	{
		*param_value_size_ret = szParamValueSize;
		return CL_SUCCESS;
	}
	// if param_value_size < actual value size return CL_INVALID_VALUE
	if (param_value_size < szParamValueSize)
	{
		ErrLog(m_pLoggerClient, L"param_value_size (=%d) < szParamValueSize (=%d)", param_value_size, szParamValueSize);
		return CL_INVALID_VALUE;
	}
	memcpy_s(param_value, szParamValueSize, pValue, param_value_size);
	return CL_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Release
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Context::Release()
{
	cl_err_code clErr = OCLObject::Release();
	if (CL_FAILED(clErr))
	{
		return clErr;
	}
	if (0 == m_uiRefCount)
	{
		m_pDevices->Clear();
		// TODO: check resources
	}
	return CL_SUCCESS;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// CreateProgramWithSource
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Context::CreateProgramWithSource(cl_uint uiCount, const char ** ppcStrings, const size_t * szLengths, Program ** ppProgram)
{
	InfoLog(m_pLoggerClient, L"CreateProgramWithSource enter. uiCount=%d, ppcStrings=%d, szLengths=%d, ppProgram=%d", uiCount, ppcStrings, szLengths, ppProgram);
	// check input parameters
	if (NULL == ppProgram)
	{
		ErrLog(m_pLoggerClient, L"NULL == ppProgram; return CL_INVALID_VALUE");
		return CL_INVALID_VALUE;
	}
	if (NULL == m_pPrograms)
	{
		ErrLog(m_pLoggerClient, L"NULL == m_pPrograms; return CL_ERR_INITILIZATION_FAILED");
		return CL_ERR_INITILIZATION_FAILED;
	}
	// create new program object
	Program * pProgram = new Program(this);
	
	// set source code in program object
	cl_err_code clErrRet = pProgram->AddSource(uiCount, ppcStrings, szLengths);
	if (CL_FAILED(clErrRet))
	{
		ErrLog(m_pLoggerClient, L"pProgram->AddSource(%d, %d, %d) = %d",uiCount, ppcStrings, szLengths, clErrRet);
		*ppProgram = NULL;
		return clErrRet;
	}
	// add program object to programs map list
	m_pPrograms->AddObject((OCLObject*)pProgram);
	*ppProgram = pProgram;
	return CL_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// CheckDevices
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Context::GetDeviceByIndex(cl_uint uiDeviceIndex, Device** ppDevice)
{
	if ( NULL == ppDevice )
    {
        return CL_INVALID_VALUE;
    }
	
    cl_err_code clErrRet = m_pDevices->GetOCLObject((cl_int)uiDeviceIndex, (OCLObject**)ppDevice);
    if ( CL_FAILED(clErrRet) || NULL == ppDevice)
    {
        return CL_ERR_KEY_NOT_FOUND;
	}
	return CL_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// CheckDevices
///////////////////////////////////////////////////////////////////////////////////////////////////
bool Context::CheckDevices(cl_uint uiNumDevices, const cl_device_id * pclDevices)
{
	InfoLog(m_pLoggerClient, L"CheckDevices enter. uiNumDevices=%d, pclDevices=%d", uiNumDevices, pclDevices);
	if (0 == uiNumDevices || NULL == pclDevices)
	{
		// invalid inputs
		ErrLog(m_pLoggerClient, L"0 == uiNumDevices || NULL == pclDevices");
		return false;
	}
	Device * pDevice = NULL;
	cl_err_code clErrRet = CL_SUCCESS;
	for (cl_uint ui=0; ui<uiNumDevices; ++ui)
	{
		clErrRet = m_pDevices->GetOCLObject((cl_int)pclDevices[ui], (OCLObject**)&pDevice);
		if ( CL_FAILED(clErrRet) || NULL == pDevice)
		{
			ErrLog(m_pLoggerClient, L"device %d was't found in this context", pclDevices[ui]);
			return false;
		}
	}
	return true;
}

cl_err_code Context::CreateProgramWithBinary(cl_uint uiNumDevices, const cl_device_id * pclDeviceList, const size_t * pszLengths, const void ** ppBinaries, cl_int * piBinaryStatus, Program ** ppProgram)
{
	InfoLog(m_pLoggerClient, L"CreateProgramWithBinary enter. uiNumDevices=%d, pclDeviceList=%d, pszLengths=%d, ppBinaries=%d, piBinaryStatus=%d, ppProgram=%d", 
		uiNumDevices, pclDeviceList, pszLengths, ppBinaries, piBinaryStatus, ppProgram);
	
	cl_err_code clErrRet = CL_SUCCESS;
	
	if (NULL == pclDeviceList || 0 == uiNumDevices || NULL == pszLengths || NULL == ppBinaries)
	{
		// invalid input args
		ErrLog(m_pLoggerClient, L"NULL == pclDeviceList || 0 == uiNumDevices || NULL == pszLengths || NULL == ppBinaries");
		return CL_INVALID_VALUE;
	}
	// check items in pszLengths and in ppBinaries
	for (cl_uint ui=0; ui<uiNumDevices; ++ui)
	{
		if (0 == pszLengths[ui] || NULL == ppBinaries[ui])
		{
			ErrLog(m_pLoggerClient, L"0 == pszLengths[%d] || NULL == ppBinaries[%d]", ui, ui);
			if (NULL != piBinaryStatus)
			{
				piBinaryStatus[ui] = CL_INVALID_VALUE;
			}
			return CL_INVALID_VALUE;
		}
	}
	
	// check devices
	bool bRes = CheckDevices(uiNumDevices, pclDeviceList);
	if (false == bRes)
	{
		ErrLog(m_pLoggerClient, L"CheckDevices(uiNumDevices, pclDeviceList) = false");
		return CL_INVALID_DEVICE;
	}

	// create program object
	Program * pProgram = new Program(this);

	// get devices and assign binaries to program object
	Device ** ppDevices = new Device * [uiNumDevices];
	if (NULL == ppDevices)
	{
		// can't allocate momery for devices
		ErrLog(m_pLoggerClient, L"Can't allocated memory for devices");
		return CL_ERR_INITILIZATION_FAILED;
	}
	
	map<cl_device_id,Device*>::iterator it;
	Device * pDevice = NULL;
	for (cl_uint ui=0; ui<uiNumDevices; ++ui)
	{
		clErrRet = m_pDevices->GetOCLObject((cl_int)pclDeviceList[ui], (OCLObject**)&pDevice);
		if (CL_SUCCEEDED(clErrRet))
		{
			ppDevices[ui] = pDevice;
		}
	}

	clErrRet = pProgram->AddBinaries(uiNumDevices, ppDevices, pszLengths, ppBinaries, piBinaryStatus);

	delete[] ppDevices;
	m_pPrograms->AddObject((OCLObject*)pProgram);
	*ppProgram = pProgram;
	return clErrRet;
}
cl_err_code Context::GetDevices(cl_uint uiNumDevices, Device ** ppDevices, cl_uint * puiNumDevicesRet)
{
	if ((NULL == ppDevices && NULL == puiNumDevicesRet) ||
		(0 != uiNumDevices && NULL == ppDevices))
	{
		return CL_INVALID_VALUE;
	}

	if (NULL == m_pDevices)
	{
		return CL_ERR_INITILIZATION_FAILED;
	}

	return m_pDevices->GetObjects(uiNumDevices, (OCLObject**)ppDevices, puiNumDevicesRet);
}
cl_err_code Context::RemoveProgram(cl_program clProgramId)
{
	InfoLog(m_pLoggerClient, L"Enter RemoveProgram (clProgramId=%d)", clProgramId);
	if (NULL == m_pPrograms)
	{
		return CL_ERR_INITILIZATION_FAILED;
	}
	Program * pProgram = NULL;
	cl_err_code clErrRet = m_pPrograms->GetOCLObject((cl_int)clProgramId, (OCLObject**)&pProgram);
	if (CL_FAILED(clErrRet))
	{
		return clErrRet;
	}
	return m_pPrograms->RemoveObject((cl_int)clProgramId, NULL);
}
cl_err_code Context::RemoveMemObject(cl_mem clMem)
{
	InfoLog(m_pLoggerClient, L"Enter RemoveMemObject (clMem=%d)", clMem);
	if (NULL == m_pMemObjects)
	{
		return CL_ERR_INITILIZATION_FAILED;
	}
	MemoryObject * pMemObj = NULL;
	cl_err_code clErrRet = m_pMemObjects->GetOCLObject((cl_int)clMem, (OCLObject**)&pMemObj);
	if (CL_FAILED(clErrRet))
	{
		return clErrRet;
	}
	return m_pMemObjects->RemoveObject((cl_int)clMem, NULL);
}
cl_err_code Context::CreateBuffer(cl_mem_flags clFlags, size_t szSize, void * pHostPtr, Buffer ** ppBuffer)
{
	InfoLog(m_pLoggerClient, L"Enter CreateBuffer (cl_mem_flags=%d, szSize=%d, pHostPtr=%d, ppBuffer=%d)", 
		clFlags, szSize, pHostPtr, ppBuffer);

#ifdef _DEBUG
	assert ( NULL != ppBuffer );
	assert ( NULL != m_pMemObjects );
#endif

	cl_ulong ulMaxMemAllocSize = GetMaxMemAllocSize();

#ifdef _DEBUG
	InfoLog(m_pLoggerClient, L"GetMaxMemAllocSize() = %d", ulMaxMemAllocSize);
#endif
	
	if (szSize == 0 || szSize > ulMaxMemAllocSize)
	{
		ErrLog (m_pLoggerClient, L"szSize == %d, ulMaxMemAllocSize =%d", szSize, ulMaxMemAllocSize);
		return CL_INVALID_BUFFER_SIZE;
	}

	cl_err_code clErr = CL_SUCCESS;
	Buffer * pBuffer = new Buffer(this, clFlags, pHostPtr, szSize, &clErr);
	if (CL_FAILED(clErr))
	{
		ErrLog (m_pLoggerClient, L"Error creating new buffer, returned: %ws", ClErrTxt(clErr));
		return clErr;
	}

	m_pMemObjects->AddObject((OCLObject*)pBuffer);

	*ppBuffer = pBuffer;
	return CL_SUCCESS;
}

cl_ulong  Context::GetMaxMemAllocSize()
{
#ifdef _DEBUG
	assert ( m_pDevices != NULL );
#endif

	InfoLog(m_pLoggerClient, L"Enter GetDeviceMaxMemAllocSize");

	cl_ulong ulMemAllocSize = 0, ulMaxMemAllocSize = 0;
	cl_err_code clErr = CL_SUCCESS;
	Device * pDevice = NULL;
	
	for (cl_uint ui=0; ui<m_pDevices->Count(); ++ui)
	{
		clErr = m_pDevices->GetObjectByIndex(ui, (OCLObject**)&pDevice);
		if (CL_FAILED(clErr) || NULL == pDevice)
		{
			continue;
		}
		clErr = pDevice->GetInfo(CL_DEVICE_MAX_MEM_ALLOC_SIZE, sizeof(cl_ulong), &ulMemAllocSize, NULL);
		if (CL_FAILED(clErr))
		{
			continue;
		}
		// get minimum of all maximum
		if (0 == ui) // first iteration
		{
			ulMaxMemAllocSize = ulMemAllocSize;
		}
		else
		{
			ulMaxMemAllocSize = (ulMemAllocSize < ulMaxMemAllocSize) ? ulMemAllocSize : ulMaxMemAllocSize;
		}
	}
    if ( 0 == ulMaxMemAllocSize)
    {
        // No one declared size, probably ignore this value, set default.
        ulMaxMemAllocSize = SHRT_MAX;
    }
	return ulMaxMemAllocSize;
}

cl_err_code Context::GetMemObject(cl_mem clMemId, MemoryObject ** ppMemObj)
{
#ifdef _DEBUG
	assert ( NULL != m_pMemObjects );
#endif
	return m_pMemObjects->GetOCLObject((cl_int)clMemId, (OCLObject**)ppMemObj);
}
void Context::NotifyError(const char * pcErrInfo, const void * pPrivateInfo, size_t szCb)
{
	if (NULL != m_pfnNotify)
	{
		m_pfnNotify(pcErrInfo, pPrivateInfo, szCb, m_pUserData);
	}
}