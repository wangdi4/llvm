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
using namespace Intel::OpenCL::Utils;
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

	if ((clMemFlags & (CL_MEM_USE_HOST_PTR | CL_MEM_ALLOC_HOST_PTR | CL_MEM_COPY_HOST_PTR)) == 1)
	{
		iMemFlags |= CL_DEV_MEM_HOST_MEM;
	}
	if ((clMemFlags & CL_MEM_READ_ONLY) == 1)
	{
		iMemFlags |= CL_DEV_MEM_READ;
	}
	else if ((clMemFlags & CL_MEM_WRITE_ONLY) == 1)
	{
		iMemFlags |= CL_DEV_MEM_WRITE;
	}
	else if ((clMemFlags & CL_MEM_READ_WRITE) == 1)
	{
		iMemFlags |= CL_DEV_MEM_READ_WRITE;
	}
	clDevMemFlags = (cl_dev_mem_flags)iMemFlags;

	cl_err_code clErr = m_pDevice->CreateMemoryObject(clDevMemFlags, NULL, 1, &szBuffersize, pHostPtr, NULL, &m_clDevMemId);
	if (CL_SUCCEEDED(clErr))
	{
		m_bAllocated = true;
	}
	return clErr;
}

cl_err_code DeviceMemoryObject::Release()
{
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
#endif

	m_pLoggerClient = new LoggerClient(L"memory_object", LL_DEBUG);
	m_pContext = pContext;
	m_clFlags = clMemFlags;
	m_pHostPtr = pHostPtr;

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

