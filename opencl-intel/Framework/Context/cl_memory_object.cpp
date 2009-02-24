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
//  cl_memory_object.cpp
//  Implementation of the MemoryObject Class
//  Created on:      10-Dec-2008 2:08:23 PM
//  Original author: ulevy
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "cl_memory_object.h"
#include <device.h>
#include <assert.h>
using namespace std;
using namespace Intel::OpenCL::Framework;

///////////////////////////////////////////////////////////////////////////////////////////////////
// DeviceMemoryObject C'tor
///////////////////////////////////////////////////////////////////////////////////////////////////
DeviceMemoryObject::DeviceMemoryObject(Device * pDevice)
{
	m_pLoggerClient = new LoggerClient(L"device_memory_objetc", LL_DEBUG);
	m_pDevice = pDevice;
	m_bAllocated = false;
	m_bDataValid = false;
	m_clDevMemId = 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// DeviceMemoryObject D'tor
///////////////////////////////////////////////////////////////////////////////////////////////////
DeviceMemoryObject::~DeviceMemoryObject()
{
	delete m_pLoggerClient;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// DeviceMemoryObject::AllocateBuffer
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code DeviceMemoryObject::AllocateBuffer(cl_mem_flags clMemFlags, size_t szBuffersize, void * pHostPtr)
{
	InfoLog(m_pLoggerClient, L"Enter AllocateBuffer (clMemFlags=%d, szBuffersize=%d", clMemFlags, szBuffersize);

#ifdef _DEBUG
	assert ( NULL != m_pDevice );
#endif

	int iMemFlags = 0;
	cl_dev_mem_flags clDevMemFlags;

	if (clMemFlags & (CL_MEM_USE_HOST_PTR | CL_MEM_ALLOC_HOST_PTR | CL_MEM_COPY_HOST_PTR))
	{
		iMemFlags |= CL_DEV_MEM_HOST_MEM;
	}
	if (clMemFlags & CL_MEM_READ_ONLY)
	{
		iMemFlags |= CL_DEV_MEM_READ;
	}
	else if (clMemFlags & CL_MEM_WRITE_ONLY)
	{
		iMemFlags |= CL_DEV_MEM_WRITE;
	}
	else if (clMemFlags & CL_MEM_READ_WRITE)
	{
		iMemFlags |= CL_DEV_MEM_READ_WRITE;
	}
	clDevMemFlags = (cl_dev_mem_flags)iMemFlags;

    OclAutoMutex CS(&m_oclLocker); // release on return
	cl_err_code clErr = m_pDevice->CreateMemoryObject(clDevMemFlags, NULL, 1, &szBuffersize, pHostPtr, NULL, &m_clDevMemId);
	if (CL_SUCCEEDED(clErr))
	{
		m_bAllocated = true;
	}
	return clErr;
    // End Critical section
}

cl_err_code DeviceMemoryObject::Release()
{
    OclAutoMutex CS(&m_oclLocker);
	if (false == m_bAllocated)
	{
		return CL_SUCCESS;
	}
	cl_err_code clErr = m_pDevice->DeleteMemoryObject(m_clDevMemId);
	if (CL_SUCCEEDED(clErr))
	{
		m_bAllocated = false;
	}
	return clErr;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// MemoryObject C'tor
///////////////////////////////////////////////////////////////////////////////////////////////////
MemoryObject::MemoryObject(Context * pContext, cl_mem_flags clMemFlags, void * pHostPtr, cl_err_code * pErr)
{
#ifdef _DEBUG
	assert ( NULL != pErr );
	assert ( NULL != pContext );
#endif

	*pErr = CL_SUCCESS;

	m_pLoggerClient = new LoggerClient(L"memory_object", LL_DEBUG);
	m_pContext = pContext;
	m_clFlags = clMemFlags;
	m_pHostPtr = pHostPtr;

	m_clMemObjectType = 0;
	m_uiMapCount = 0;

	// check input flags
	if ((0 == clMemFlags) ||
		(m_clFlags & (CL_MEM_WRITE_ONLY & CL_MEM_READ_ONLY)) )
	{
		ErrLog (m_pLoggerClient, L"0 == clMemFlags");
		*pErr = CL_INVALID_VALUE;
		return;
	}

	// check invalid usage of host ptr
	if (((NULL == m_pHostPtr) && (m_clFlags & (CL_MEM_USE_HOST_PTR | CL_MEM_COPY_HOST_PTR)))	||
		((NULL != m_pHostPtr) && !(m_clFlags & (CL_MEM_USE_HOST_PTR | CL_MEM_COPY_HOST_PTR))))
	{
		ErrLog (m_pLoggerClient, L"invalid usage of host ptr");
		*pErr = CL_INVALID_HOST_PTR;
	}

	cl_uint uiNumDevices = 0;
	Device ** ppDevices = NULL;
	// get number of devices
	cl_err_code clErr = m_pContext->GetDevices(0, NULL, &uiNumDevices);
	if (CL_FAILED(clErr))
	{
		*pErr = clErr;
	}
	ppDevices = new Device * [uiNumDevices];
	if (NULL == ppDevices)
	{
		*pErr = CL_OUT_OF_HOST_MEMORY;
	}
	// get devices list from the context
	clErr = m_pContext->GetDevices(uiNumDevices, ppDevices, NULL);
	if (CL_FAILED(clErr))
	{
		delete[] ppDevices;
		*pErr = clErr;
		return;
	}
	//create device memory object for each device
	for (cl_uint ui=0; ui<uiNumDevices; ++ui)
	{
		Device * pDevice = ppDevices[ui];
		if (NULL != pDevice)
		{
			DeviceMemoryObject * pDevMemObj = new DeviceMemoryObject(pDevice);
			m_mapDeviceMemObjects[(cl_device_id)(pDevice->GetId())] = pDevMemObj;
		}
	}
	delete[] ppDevices;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// MemoryObject D'tor
///////////////////////////////////////////////////////////////////////////////////////////////////
MemoryObject::~MemoryObject()
{
	InfoLog(m_pLoggerClient, L"Enter MemoryObject D'tor");

	map<cl_device_id, DeviceMemoryObject*>::iterator it = m_mapDeviceMemObjects.begin();
	while (it != m_mapDeviceMemObjects.end())
	{
		DeviceMemoryObject * pDeviceMemObj = it->second;
		if (NULL != pDeviceMemObj)
		{
			pDeviceMemObj->Release();
			delete pDeviceMemObj;
		}
		it++;
	}
	m_mapDeviceMemObjects.clear();
	delete m_pLoggerClient;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// MemoryObject::IsAllocated
///////////////////////////////////////////////////////////////////////////////////////////////////
bool MemoryObject::IsAllocated(cl_device_id clDeviceId)
{
	InfoLog(m_pLoggerClient, L"Enter IsReady (clDeviceId=%d)", clDeviceId);

	map<cl_device_id, DeviceMemoryObject*>::iterator it = m_mapDeviceMemObjects.find(clDeviceId);
	if (it == m_mapDeviceMemObjects.end())
	{
		// device not found
		return false;
	}
	// get the device memory objectand check if it was allocated
	DeviceMemoryObject * pDevMemObj = it->second;
	return pDevMemObj->IsAllocated();
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// MemoryObject::GetDeviceMemoryHndl
// If there is no resource in this device, or resource is not valid, 0 hndl is returned.
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_dev_mem MemoryObject::GetDeviceMemoryHndl( cl_device_id clDeviceId )
{
	InfoLog(m_pLoggerClient, L"Enter GetDeviceMemoryHndl");

	map<cl_device_id, DeviceMemoryObject*>::iterator it = m_mapDeviceMemObjects.find(clDeviceId);
	if (it == m_mapDeviceMemObjects.end())
	{
		// device not found
		return 0;
	}
	// get the device memory objectand check if it was allocated
	DeviceMemoryObject * pDevMemObj = it->second;
	return pDevMemObj->GetDeviceMemoryId();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// MemoryObject::GetDataLocation
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_device_id MemoryObject::GetDataLocation()
{
	InfoLog(m_pLoggerClient, L"Enter GetDataLocation");

	for (map<cl_device_id, DeviceMemoryObject*>::iterator it = m_mapDeviceMemObjects.begin(); it != m_mapDeviceMemObjects.end(); it++)
	{
		DeviceMemoryObject * pDevMemObj = it->second;
		if (pDevMemObj->IsDataValid())
		{
			return it->first;
		}
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// MemoryObject::SetDataLocation
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code MemoryObject::SetDataLocation(cl_device_id clDevice)
{
	InfoLog(m_pLoggerClient, L"Enter SetDataLocation (clDevice=%d)", clDevice);

	map<cl_device_id, DeviceMemoryObject*>::iterator it = m_mapDeviceMemObjects.find(clDevice);
	if (it == m_mapDeviceMemObjects.end())
	{
		ErrLog(m_pLoggerClient, L"Can't find device %d", clDevice);
		return CL_INVALID_DEVICE;
	}

	for (it = m_mapDeviceMemObjects.begin(); it != m_mapDeviceMemObjects.end(); it++)
	{
		DeviceMemoryObject * pDevMemObj = it->second;

#ifdef _DEBUG
		assert ( pDevMemObj != NULL );
#endif
		if (it->first != clDevice)
		{
			pDevMemObj->SetDataValid(false);
		}
		else
		{
			pDevMemObj->SetDataValid(true);
		}
	}
	return CL_SUCCESS;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// MemoryObject::GetInfo
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code	MemoryObject::GetInfo(cl_int iParamName, size_t szParamValueSize, void * pParamValue, size_t * pszParamValueSizeRet)
{
	InfoLog(m_pLoggerClient, L"Enter MemoryObject::GetInfo (iParamName=%d, szParamValueSize=%d, pParamValue=%d, pszParamValueSizeRet=%d)", 
		iParamName, szParamValueSize, pParamValue, pszParamValueSizeRet);

	if ((NULL == pParamValue && NULL == pszParamValueSizeRet) || 
		(NULL == pParamValue && iParamName != 0))
	{
		return CL_INVALID_VALUE;
	}
	size_t szSize = 0;
	size_t szParam = 0;
	cl_context clContext = 0;
	void * pValue = NULL;
	
	cl_err_code clErrRet = CL_SUCCESS;
	switch ( (cl_mem_info)iParamName )
	{

	case CL_MEM_TYPE:
		szSize = sizeof(cl_mem_object_type);
		pValue = &m_clMemObjectType;
		break;
	case CL_MEM_FLAGS:
		szSize = sizeof(cl_mem_flags);
		pValue = &m_clFlags;
		break;
	case CL_MEM_SIZE:
		szSize = sizeof(size_t);
		szParam = (size_t)GetSize();
		pValue = &szParam;
		break;
	case CL_MEM_HOST_PTR:
		szSize = sizeof(void*);
		pValue = &m_pHostPtr;
		break;
	case CL_MEM_MAP_COUNT:
		szSize = sizeof(cl_uint);
		pValue = &m_uiMapCount;
		break;
		return CL_ERR_NOT_IMPLEMENTED;
	case CL_MEM_REFERENCE_COUNT:
		szSize = sizeof(cl_uint);
		pValue = &m_uiRefCount;
		break;
	case CL_MEM_CONTEXT:
		szSize = sizeof(cl_context);
		clContext = (cl_context)m_pContext->GetId();
		pValue = &clContext;
	default:
		ErrLog(m_pLoggerClient, L"param_name (=%d) isn't valid", iParamName);
		return CL_INVALID_VALUE;
	}
	if (CL_FAILED(clErrRet))
	{
		return clErrRet;
	}
	// if param_value == NULL return only param value size
	if (NULL == pParamValue)
	{
		*pszParamValueSizeRet = szSize;
		return CL_SUCCESS;
	}
	// if param_value_size < actual value size return CL_INVALID_VALUE
	if (NULL != pParamValue && szParamValueSize < szSize)
	{
		ErrLog(m_pLoggerClient, L"szParamValueSize (=%d) < szSize (=%d)", szParamValueSize, szSize);
		return CL_INVALID_VALUE;
	}
	memcpy_s(pParamValue, szSize, pValue, szParamValueSize);
	return CL_SUCCESS;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// MemoryObject::Release
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code MemoryObject::Release()
{
	return OCLObject::Release();
}