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
//  cl_buffer.cpp
//  Implementation of the Buffer Class
//  Created on:      10-Dec-2008 2:08:23 PM
//  Original author: ulevy
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "cl_buffer.h"
#include <assert.h>
using namespace std;
using namespace Intel::OpenCL::Utils;
using namespace Intel::OpenCL::Framework;

///////////////////////////////////////////////////////////////////////////////////////////////////
// Buffer C'tor
///////////////////////////////////////////////////////////////////////////////////////////////////
Buffer::Buffer(Context * pContext, cl_mem_flags clMemFlags, void * pHostPtr, size_t szBufferSize, cl_err_code * pErrCode):
		MemoryObject(pContext, clMemFlags, pHostPtr, pErrCode)
{
#ifdef _DEBUG
	assert ( NULL != pErrCode );
#endif
	m_pBufferData = NULL;
	if (CL_FAILED(*pErrCode))
	{
		return;
	}
	m_clMemObjectType = CL_MEM_OBJECT_BUFFER;

	m_szBufferSize = szBufferSize;
	m_pBufferData = new char[m_szBufferSize];
	if (NULL == m_pBufferData)
	{
		*pErrCode = CL_OUT_OF_HOST_MEMORY;
		return;
	}

	if (clMemFlags & CL_MEM_COPY_HOST_PTR)
	{
		errno_t err = memcpy_s(m_pBufferData, m_szBufferSize, pHostPtr, szBufferSize);
		if (err)
		{
			*pErrCode = CL_OUT_OF_HOST_MEMORY;
			return;
		}
		m_bDataOnHost = true;
	}

	*pErrCode = CL_SUCCESS;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// Buffer D'tor
///////////////////////////////////////////////////////////////////////////////////////////////////
Buffer::~Buffer()
{
	InfoLog(m_pLoggerClient, L"Enter MemoryObject D'tor");
	if (NULL != m_pBufferData)
	{
		delete[] m_pBufferData;
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// Buffer::Release()
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Buffer::Release()
{
	return MemoryObject::Release();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// Buffer::CreateDeviceResource
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Buffer::CreateDeviceResource(cl_device_id clDeviceId)
{
	InfoLog(m_pLoggerClient, L"Enter CreateDeviceResource (clDeviceId=%d)", clDeviceId);

	map<cl_device_id, DeviceMemoryObject*>::iterator it = m_mapDeviceMemObjects.find(clDeviceId);
	if (it == m_mapDeviceMemObjects.end())
	{
		ErrLog(m_pLoggerClient, L"Can't find device %d", clDeviceId);
		return CL_INVALID_DEVICE;
	}

	DeviceMemoryObject * pDevMemObj = it->second;

#ifdef _DEBUG
	assert ( pDevMemObj != NULL );
#endif

	return pDevMemObj->AllocateBuffer(m_clFlags, m_szBufferSize, m_pHostPtr);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// Buffer::ReadData
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Buffer::ReadData(void * pData, size_t szOffset, size_t szDataSize)
{
	InfoLog(m_pLoggerClient, L"Enter ReadData (szDataSize=%d, pData=%d, szOffset=%d)", szDataSize, pData, szOffset);

    if (NULL == pData || 0 == szDataSize || szOffset+szDataSize > m_szBufferSize ||  NULL == m_pBufferData)
	{
		return CL_INVALID_VALUE;
	}

    memcpy_s(pData, szDataSize, (void*)((cl_uchar*)m_pBufferData+szOffset), szDataSize);

	return CL_SUCCESS;
}

size_t Buffer::GetSize() const
{
	return m_szBufferSize;
}
void * Buffer::GetData() const
{
	return m_pBufferData;
}