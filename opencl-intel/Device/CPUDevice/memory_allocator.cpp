// Copyright (c) 2006-2008 Intel Corporation
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

///////////////////////////////////////////////////////////
//  MemoryAllocator.cpp
//  Implementation of the Class MemoryAllocator
//  Created on:      16-Dec-2008 4:54:53 PM
//  Original author: efiksman
///////////////////////////////////////////////////////////

#include "stdafx.h"

#include "memory_allocator.h"
#include "cl_logger.h"
#include "cpu_dev_limits.h"

#include<stdlib.h>


using namespace Intel::OpenCL::CPUDevice;

MemoryAllocator::MemoryAllocator(cl_int devId, cl_dev_log_descriptor *logDesc) :
	m_iDevId(devId), m_objHandles(1, UINT_MAX)
{
	if ( NULL == logDesc )
	{
		memset(&m_logDescriptor, 0, sizeof(cl_dev_log_descriptor));
	}
	else
	{
		memcpy_s(&m_logDescriptor, sizeof(cl_dev_log_descriptor), logDesc, sizeof(cl_dev_log_descriptor));
	}
	

	cl_int ret = m_logDescriptor.pfnclLogCreateClient(m_iDevId, L"CPU Device: Memory Allocator", &m_iLogHandle);
	if(CL_DEV_SUCCESS != ret)
	{
		//TBD
		m_iLogHandle = 0;
	}

	InfoLog(m_logDescriptor, m_iLogHandle, L"MemoryAllocator Created");
}

MemoryAllocator::~MemoryAllocator()
{
	InfoLog(m_logDescriptor, m_iLogHandle, L"MemoryAllocator Distructed");
	TMemObjectsMap::const_iterator it;

	for(it=m_mapObjects.begin(); m_mapObjects.end() != it ; ++it )
	{
		SMemObjectDescriptor* pObjDesc = it->second;

		_aligned_free(pObjDesc->pObject);
		delete pObjDesc->myHandle;
		delete pObjDesc;
	}

	cl_int ret = m_logDescriptor.pfnclLogReleaseClient(m_iLogHandle);
}

// Checks that given object is valid object and belongs to memory allocator
cl_int	MemoryAllocator::ValidateObject( cl_dev_mem IN memObj )
{
	InfoLog(m_logDescriptor, m_iLogHandle, L"ValidateObject enter");

	if ( memObj->allocId != m_iDevId )
	{
		return CL_DEV_INVALID_MEM_OBJECT;
	}

	return CL_DEV_SUCCESS;
}

cl_int MemoryAllocator::CreateObject( cl_dev_mem_flags IN flags, const cl_image_format* IN format,
									 cl_uint dim_count, const size_t* dim,
									 void*	buffer_ptr, const size_t* pitch,
									 cl_dev_mem* OUT memObj )
{
	InfoLog(m_logDescriptor, m_iLogHandle, L"CreateObject enter");
	if ( (NULL == memObj) || (NULL == dim) )
	{
		return CL_DEV_INVALID_VALUE;
	}

	if ( (NULL != format) || ( 1 != dim_count ) || (NULL != pitch) )
	{
		InfoLog(m_logDescriptor, m_iLogHandle, L"Only 1D buffers are supported");
		// Only 1D buffers are supported for now
		return CL_DEV_INVALID_VALUE;
	}

	if ( MAX_DIMENSION < dim_count )
	{
		return CL_DEV_INVALID_IMG_SIZE;
	}

	// Allocate new object handle
	unsigned int uiNewObject;

	if ( !m_objHandles.AllocateHandle(&uiNewObject) )
	{
		ErrLog(m_logDescriptor, m_iLogHandle, L"Can't allocate new object handle");
		return CL_DEV_OBJECT_ALLOC_FAIL;
	}
	// Allocate memory for memory object
	cl_dev_mem	pMemObj = new _cl_dev_mem;

	if ( NULL == pMemObj )
	{
		m_objHandles.FreeHandle(uiNewObject);
		ErrLog(m_logDescriptor, m_iLogHandle, L"Memory Object allocation failed");
		return CL_DEV_OBJECT_ALLOC_FAIL;
	}

	// Allocate memory for object descriptor
	SMemObjectDescriptor*	pMemObjDesc = new SMemObjectDescriptor;
	if ( NULL == pMemObjDesc )
	{
		ErrLog(m_logDescriptor, m_iLogHandle, L"Memory Object descriptor allocation failed");
		m_objHandles.FreeHandle(uiNewObject);
		delete pMemObj;
		return CL_DEV_OBJECT_ALLOC_FAIL;
	}

	if ( NULL == buffer_ptr ) 		// Allocate memory for the new object
	{
		pMemObjDesc->pObject = _aligned_malloc(dim[0], CPU_DCU_LINE_SIZE);
		if ( NULL == pMemObjDesc->pObject )
		{
			ErrLog(m_logDescriptor, m_iLogHandle, L"Memory Object memory buffer Allocation failed");
			m_objHandles.FreeHandle(uiNewObject);
			delete pMemObjDesc;
			delete pMemObj;

			return CL_DEV_OBJECT_ALLOC_FAIL;
		}

		pMemObjDesc->bFreeRequired = true;	// Just created object must be release on destrcution
	} else
	{
		pMemObjDesc->pObject = buffer_ptr;
		pMemObjDesc->bFreeRequired = false;	// The memory buffer should not be released
	}

	pMemObjDesc->uiDimCount = dim_count;
	pMemObjDesc->memFlags = flags;
	if ( NULL != format )
	{
		memcpy(&pMemObjDesc->imgFormat, format, sizeof(cl_image_format));
	} else
	{
		memset(&pMemObjDesc->imgFormat, 0, sizeof(cl_image_format));
	}

	memcpy(pMemObjDesc->stDim, dim, dim_count);

	pMemObjDesc->myHandle = pMemObj;

	m_muObjectMap.Lock();
	m_mapObjects[uiNewObject] = pMemObjDesc;
	m_muObjectMap.Unlock();

	// Add object to map
	pMemObj->allocId = m_iDevId;
	pMemObj->objHandle = (void*)uiNewObject;

	*memObj = pMemObj;

	return CL_DEV_SUCCESS;
}

cl_int MemoryAllocator::ReleaseObject( cl_dev_mem IN memObj )
{
	InfoLog(m_logDescriptor, m_iLogHandle, L"ReleaseObject enter");

	if ( (NULL == memObj) || (m_iDevId != memObj->allocId) )
	{
		return CL_INVALID_MEM_OBJECT;
	}

	TMemObjectsMap::const_iterator it;

	m_muObjectMap.Lock();
	it = m_mapObjects.find((unsigned int)memObj->objHandle);
	if ( m_mapObjects.end() == it )
	{
		m_muObjectMap.Unlock();
		InfoLog(m_logDescriptor, m_iLogHandle, L"Memory object coudn't be found");
		return CL_DEV_INVALID_MEM_OBJECT;
	}

	SMemObjectDescriptor* pObjDesc = it->second;
	m_muObjectMap.Unlock();

	if ( pObjDesc->bFreeRequired )
	{
		_aligned_free(pObjDesc->pObject);
	}

	delete pObjDesc;
	delete memObj;

	return CL_DEV_SUCCESS;
}

cl_int MemoryAllocator::LockObject(cl_dev_mem IN pMemObj, cl_uint dim_count, const size_t* IN origin,
							void** OUT ptr, size_t* OUT pitch)
{
	InfoLog(m_logDescriptor, m_iLogHandle, L"LockObject enter");

	if ( NULL == ptr )
	{
		return CL_DEV_INVALID_VALUE;
	}

	if ( m_iDevId != pMemObj->allocId)
	{
		InfoLog(m_logDescriptor, m_iLogHandle, L"Invalid device ID:%X", pMemObj->allocId);
		return CL_DEV_INVALID_MEM_OBJECT;
	}

	TMemObjectsMap::const_iterator it;

	m_muObjectMap.Lock();
	it = m_mapObjects.find((unsigned int)pMemObj->objHandle);
	if ( m_mapObjects.end() == it )
	{
		m_muObjectMap.Unlock();
		InfoLog(m_logDescriptor, m_iLogHandle, L"Memory object coudn't be found");
		return CL_DEV_INVALID_MEM_OBJECT;
	}

	SMemObjectDescriptor* pObjDesc = it->second;
	m_muObjectMap.Unlock();

	if ( (-1 != dim_count) && (dim_count != pObjDesc->uiDimCount) )
	{
		return CL_DEV_INVALID_IMG_SIZE;
	}

	cl_char* lockedPtr = (cl_char*)pObjDesc->pObject;
	if ( NULL != origin )
	{
		lockedPtr += origin[0];
	}

	*ptr = lockedPtr;

	return CL_DEV_SUCCESS;
}

cl_int MemoryAllocator::UnLockObject(cl_dev_mem IN pMemObj, void* IN ptr)
{
	InfoLog(m_logDescriptor, m_iLogHandle, L"UnLockObject enter");
	return CL_DEV_SUCCESS;
}

cl_int MemoryAllocator::CreateMappedRegion(cl_dev_mem IN memObj, cl_uint IN dim_count, const size_t* IN origin,
										   const size_t* IN region,	void** OUT ptr, size_t* OUT pitch)
{
	InfoLog(m_logDescriptor, m_iLogHandle, L"CreateMappedRegion enter");

	return 	LockObject(memObj, dim_count, origin, ptr, pitch);
}

cl_int MemoryAllocator::ReleaseMappedRegion( cl_dev_mem IN memObj, void* IN ptr )
{
	InfoLog(m_logDescriptor, m_iLogHandle, L"ReleaseMappedRegion enter");
	return CL_DEV_SUCCESS;
}

