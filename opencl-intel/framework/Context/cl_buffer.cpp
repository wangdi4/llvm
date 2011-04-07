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
#include "cl_sys_defines.h"
#include <assert.h>
using namespace std;
using namespace Intel::OpenCL::Utils;
using namespace Intel::OpenCL::Framework;

#include <Device.h>

///////////////////////////////////////////////////////////////////////////////////////////////////
// Buffer C'tor
///////////////////////////////////////////////////////////////////////////////////////////////////
Buffer::Buffer(Context * pContext, cl_mem_flags clMemFlags, size_t szBufferSize, ocl_entry_points * pOclEntryPoints, cl_err_code * pErrCode):
	MemoryObject(pContext, clMemFlags, pOclEntryPoints, pErrCode), 
	m_subBuffers(NULL)
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
	
	if (false == IsSubBuffer())
	{
		m_subBuffers = new OCLObjectsMap<_cl_mem_int>();
	}
	*pErrCode = CL_SUCCESS;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// Buffer D'tor
///////////////////////////////////////////////////////////////////////////////////////////////////
Buffer::~Buffer()
{
	LOG_DEBUG(TEXT("%S"), TEXT("Enter MemoryObject D'tor"));

	if ( NULL != m_subBuffers )
    {
        delete m_subBuffers;
        m_subBuffers = NULL;
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// Buffer::CreateDeviceResource
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Buffer::CreateDeviceResource(cl_device_id clDeviceId)
{
	LOG_DEBUG(L"Enter CreateDeviceResource (clDeviceId=%d)", clDeviceId);

	DeviceMemoryObject* pDevMemObj = GetDeviceMemoryObject(clDeviceId);
	if (NULL == pDevMemObj)
	{
		LOG_ERROR(TEXT("Can't find device %d"), clDeviceId);
		return CL_INVALID_DEVICE;
	}
	
	return pDevMemObj->AllocateBuffer(m_clFlags, m_szMemObjSize, m_pMemObjData);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// Buffer::ReadData
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Buffer::ReadData(void * pData, const size_t * pszOrigin, const size_t * pszRegion, size_t szRowPitch, size_t szSlicePitch)
{
    size_t szOffset = pszOrigin[0];
    size_t szDataSize = pszRegion[0];

    LOG_DEBUG(TEXT("Enter ReadData (szDataSize=%d, pData=%d, szOffset=%d)"), szDataSize, pData, szOffset);

    if (NULL == pData || 0 == szDataSize || szOffset+szDataSize > m_szMemObjSize ||  NULL == m_pMemObjData)
	{
		return CL_INVALID_VALUE;
	}

    MEMCPY_S(pData, szDataSize, (void*)((cl_uchar*)m_pMemObjData+szOffset), szDataSize);

	return CL_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Buffer::WriteData
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Buffer::WriteData(const void * pData, const size_t * pszOrigin, const size_t * pszRegion, size_t szRowPitch, size_t szSlicePitch)
{
    size_t szOffset = pszOrigin[0];
    size_t szDataSize = pszRegion[0];

    LOG_DEBUG(TEXT("Enter WriteData (szDataSize=%d, pData=%d, szOffset=%d)"), szDataSize, pData, szOffset);

    if (NULL == pData || 0 == szDataSize || szOffset+szDataSize > m_szMemObjSize ||  NULL == m_pMemObjData)
	{
		return CL_INVALID_VALUE;
	}

    MEMCPY_S((void*)((cl_uchar*)m_pMemObjData+szOffset), szDataSize, pData, szDataSize);

	return CL_SUCCESS;
}

size_t Buffer::GetSize() const
{
	return m_szMemObjSize;
}

void Buffer::GetLayout( OUT size_t* dimensions, OUT size_t* rowPitch, OUT size_t* slicePitch ) const
{
	if (dimensions)
	{
		dimensions[0] = GetSize();
		for (int i = 1; i < MAX_WORK_DIM; i++)
		{
			dimensions[i] = 1;
		}
	}
	*rowPitch = *slicePitch = 0;	
}


bool Buffer::CheckBounds( const size_t* pszOrigin, const size_t* pszRegion) const
{

    return ((*pszOrigin + *pszRegion) <= (m_szMemObjSize));
}

bool Buffer::CheckBounds( const size_t* pszOrigin, const size_t* pszRegion, size_t szRowPitch, size_t szSlicePitch) const
{
	
	size_t totalSize = 
	// offset
		pszOrigin[2] * szSlicePitch + pszOrigin[1] * szRowPitch + pszOrigin[0] + 
	// data size
		(pszRegion[2]-1)*szSlicePitch + (pszRegion[1]-1) * szRowPitch + pszRegion[0];
	return (totalSize <= m_szMemObjSize);    
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

void Buffer::AddSubBuffer(SubBuffer* pSubBuffer)
{		
	assert(pSubBuffer && pSubBuffer->IsSubBuffer());		
	m_subBuffers->AddObject(pSubBuffer);
}

void Buffer::GetSubBuffers(int szCount,  SubBuffer** pSubBuffers, cl_uint* retCount)
{ 
	m_subBuffers->GetObjects(szCount, reinterpret_cast<OCLObject<_cl_mem_int>**>(pSubBuffers), retCount); 
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// SubBuffer::SubBuffer
///////////////////////////////////////////////////////////////////////////////////////////////////
SubBuffer::SubBuffer(Context * pContext, cl_mem_flags clMemFlags, ocl_entry_points * pOclEntryPoints, cl_err_code * pErrCode):
Buffer(pContext, clMemFlags, 0, pOclEntryPoints, pErrCode),
m_Origin(0),
m_szSize(0),
m_pParentBuffer(NULL)
{
	if (CL_FAILED(pErrCode))
	{
		return;
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// SubBuffer::~SubBuffer
///////////////////////////////////////////////////////////////////////////////////////////////////
SubBuffer::~SubBuffer()
{
	if (m_pParentBuffer)
	{		
		m_pParentBuffer->RemovePendency();
	}
	m_pMemObjData = NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// SubBuffer::CheckIfSupportedByDevice
///////////////////////////////////////////////////////////////////////////////////////////////////
bool SubBuffer::CheckIfSupportedByDevice(cl_device_id deviceId)
{
	cl_uint         align;
	size_t          rsize;

	Intel::OpenCL::Framework::FissionableDevice* ppDevice = NULL;
	GetContext()->GetDevice(deviceId,&ppDevice);		
	ppDevice->GetInfo(CL_DEVICE_MEM_BASE_ADDR_ALIGN, sizeof(align), &align, &rsize);		
	return ((m_Origin % align) == 0);	
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// SubBuffer::initialize
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code SubBuffer::initialize(Buffer* pBuffer, cl_buffer_create_type buffer_create_type, const void * buffer_create_info)
{			
	if (buffer_create_type == CL_BUFFER_CREATE_TYPE_REGION)
	{
		const cl_buffer_region* region = reinterpret_cast<const cl_buffer_region*>(buffer_create_info);
		if (region->size == 0 )
		{
			return CL_INVALID_BUFFER_SIZE;
		}

		if (region->origin + region->size > pBuffer->GetSize())
		{
			return CL_INVALID_VALUE;
		}
		LOG_DEBUG(TEXT("Enter SubBuffer::initialize buffer_create_type=%d, origin=%d size=%d)"),region->origin, region->size);		

		m_Origin = region->origin;
		m_szSize = region->size;

		m_szMemObjSize = region->size;

		if (pBuffer->GetFlags() & CL_MEM_WRITE_ONLY)
		{
			if (m_clFlags & CL_MEM_READ_ONLY || m_clFlags & CL_MEM_READ_WRITE)					
			{
				return CL_INVALID_VALUE;
			}
		}
		else if (pBuffer->GetFlags() & CL_MEM_READ_ONLY)
		{
			cl_mem_flags nonReadOnlyFlag = CL_MEM_WRITE_ONLY | CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR | CL_MEM_COPY_HOST_PTR | CL_MEM_ALLOC_HOST_PTR;			
			if (m_clFlags & nonReadOnlyFlag)
			{
				return CL_INVALID_VALUE;
			}
		}

		if ((m_clFlags & CL_MEM_USE_HOST_PTR) && !(pBuffer->GetFlags() & CL_MEM_USE_HOST_PTR) )
		{
			return CL_INVALID_VALUE;
		}							

		// My runtime copy is at offset region->origin from my parent buffer object.
		m_pMemObjData = pBuffer->GetData(&region->origin);		
		m_pHostPtr = m_pMemObjData;
		
		/// Check there exists a device in the context support the sub_buffer alignment
		cl_uint uiAllDevices = 0;
		cl_device_id * pclAllDeviceList = NULL;		
		pclAllDeviceList = m_pContext->GetDeviceIds(&uiAllDevices);
		
		bool allDevicesAreMisAligned = true;
		for (cl_uint i=0; i < uiAllDevices; i++)
		{
			if (CheckIfSupportedByDevice(pclAllDeviceList[i]))
			{
				allDevicesAreMisAligned = false;
				break;				
			}			
		}	
		if (allDevicesAreMisAligned)
		{
			LOG_ERROR(TEXT("No devices in context %p associated with buffer for which the origin %d\
					   value is aligned to the CL_DEVICE_MEM_BASE_ADDR_ALIGN value."),m_pContext, region->origin);
			return CL_MISALIGNED_SUB_BUFFER_OFFSET;
		}		
	}
	else
	{			
		return CL_INVALID_VALUE;
	}
	m_pParentBuffer = pBuffer;
	m_pParentBuffer->AddPendency();	

	cl_device_id parentLocation = m_pParentBuffer->GetDataLocation();
	if (m_pParentBuffer->IsAllocated(parentLocation))
	{
		SetDataLocation(parentLocation);
	}

	// Not in use now, but will be used later when we add support for multiple devices.
	pBuffer->AddSubBuffer(this);
	return CL_SUCCESS;
}
