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
	if (CL_FAILED(*pErrCode))
	{
		return;
	}
	m_clMemObjectType = CL_MEM_OBJECT_BUFFER;

	m_szMemObjSize = szBufferSize;

	*pErrCode = CL_SUCCESS;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// Buffer D'tor
///////////////////////////////////////////////////////////////////////////////////////////////////
Buffer::~Buffer()
{
	LOG_DEBUG(L"Enter MemoryObject D'tor");
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
	LOG_DEBUG(L"Enter CreateDeviceResource (clDeviceId=%d)", clDeviceId);

	map<cl_device_id, DeviceMemoryObject*>::iterator it = m_mapDeviceMemObjects.find(clDeviceId);
	if (it == m_mapDeviceMemObjects.end())
	{
		LOG_ERROR(L"Can't find device %d", clDeviceId);
		return CL_INVALID_DEVICE;
	}

	DeviceMemoryObject * pDevMemObj = it->second;

#ifdef _DEBUG
	assert ( pDevMemObj != NULL );
#endif

	//return pDevMemObj->AllocateBuffer(m_clFlags, m_szMemObjSize, m_pHostPtr);
	return pDevMemObj->AllocateBuffer(m_clFlags, m_szMemObjSize, m_pMemObjData);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// Buffer::ReadData
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Buffer::ReadData(void * pData, const size_t * pszOrigin, const size_t * pszRegion, size_t szRowPitch, size_t szSlicePitch)
{
    size_t szOffset = pszOrigin[0];
    size_t szDataSize = pszRegion[0];

    LOG_DEBUG(L"Enter ReadData (szDataSize=%d, pData=%d, szOffset=%d)", szDataSize, pData, szOffset);

    if (NULL == pData || 0 == szDataSize || szOffset+szDataSize > m_szMemObjSize ||  NULL == m_pMemObjData)
	{
		return CL_INVALID_VALUE;
	}

    memcpy_s(pData, szDataSize, (void*)((cl_uchar*)m_pMemObjData+szOffset), szDataSize);

	return CL_SUCCESS;
}

size_t Buffer::GetSize() const
{
	return m_szMemObjSize;
}

bool Buffer::CheckBounds( const size_t* pszOrigin, const size_t* pszRegion) const
{

    return ((*pszOrigin + *pszRegion) <= (m_szMemObjSize));
}

void * Buffer::GetData( const size_t * pszOrigin ) const
{
    void* pData = NULL;
    if( NULL == pszOrigin )
    {
        pData = m_pMemObjData;
    }
    else
    {
        // Get with offset
        pData = (void*)((cl_uchar*)m_pMemObjData+pszOrigin[0]);
    }
	return pData;
}
void* Buffer::CreateMappedRegion(cl_device_id clDeviceId, 
								 cl_map_flags clMapFlags, 
								 const size_t * szOrigins, 
								 const size_t * szRegions, 
								 size_t * pszImageRowPitch, 
								 size_t * pszImageSlicePitch)
{
	LOG_DEBUG(L"Enter CreateMappedRegion (clDeviceId=%d, cl_map_flags=%d, szOrigins=%d, szRegions=%d, pszImageRowPitch=%d, pszImageSlicePitch=%d)", 
		clDeviceId, clMapFlags, szOrigins, szRegions, pszImageRowPitch, pszImageSlicePitch);
	
	// get device memory object
	map<cl_device_id, DeviceMemoryObject*>::iterator it = m_mapDeviceMemObjects.find(clDeviceId);
	if (it == m_mapDeviceMemObjects.end())
	{
		return NULL;
	}
	DeviceMemoryObject * pDevMemObj = it->second;
	if (NULL == pDevMemObj || false == pDevMemObj->IsAllocated())
	{
		return NULL;
	}
	return pDevMemObj->CreateMappedRegion(clMapFlags, 1, szOrigins, szRegions, pszImageRowPitch, pszImageSlicePitch);
}
cl_err_code Buffer::ReleaseMappedRegion(cl_device_id clDeviceId, void* mappedPtr)
{
	LOG_DEBUG(L"Enter ReleaseMappedRegion (clDeviceId=%d, mappedPtr=%d)", clDeviceId, mappedPtr);

	map<cl_device_id, DeviceMemoryObject*>::iterator it = m_mapDeviceMemObjects.find(clDeviceId);
	if (it == m_mapDeviceMemObjects.end())
	{
		return CL_DEVICE_NOT_FOUND;
	}
	DeviceMemoryObject * pDevMemObj = it->second;
	if (NULL == pDevMemObj)
	{
		return CL_DEVICE_NOT_FOUND;
	}
	return pDevMemObj->ReleaseMappedRegion(mappedPtr);
}
void* Buffer::GetMappedRegionInfo( cl_device_id clDeviceId, void* mappedPtr)
{
	LOG_DEBUG(L"Enter GetMappedRegionInfo (clDeviceId=%d, mappedPtr=%d)", clDeviceId, mappedPtr);

	map<cl_device_id, DeviceMemoryObject*>::iterator it = m_mapDeviceMemObjects.find(clDeviceId);
	if (it == m_mapDeviceMemObjects.end())
	{
		return NULL;
	}
	DeviceMemoryObject * pDevMemObj = it->second;
	if (NULL == pDevMemObj)
	{
		return NULL;
	}
	return pDevMemObj->GetMappedRegionInfo(mappedPtr);
}