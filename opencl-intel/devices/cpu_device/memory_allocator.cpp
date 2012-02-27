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
#include "cpu_logger.h"
#include "cpu_dev_limits.h"
#include "cpu_device.h"
#include "cl_sys_defines.h"

#include<stdlib.h>

// The flag below enables a check that allows only a single use of cl_mem objects
// The "lock" on the memory object is obtained during the call to NDRange->Init() and is released when the kernel is done executing
// This is useful only for debugging - it is not conformant to the spec
// So, keep the flag undefined unless you are debugging potential race conditions in kernel executions
//#define _ENABLE_LOCK_OBJECTS_

using namespace Intel::OpenCL::CPUDevice;

MemoryAllocator::MemoryAllocator(cl_int devId, IOCLDevLogDescriptor *logDesc, cl_ulong maxAllocSize, ICLDevBackendImageService* pImageService) :
	m_iDevId(devId), m_maxAllocSize(maxAllocSize), m_pLogDescriptor(logDesc), m_iLogHandle(0), m_pImageService(pImageService)
{
	if ( NULL != logDesc )
	{
		cl_int ret = m_pLogDescriptor->clLogCreateClient(m_iDevId, TEXT("CPU Device: Memory Allocator"), &m_iLogHandle);
		if(CL_DEV_SUCCESS != ret)
		{
			m_iLogHandle = 0;
		}
	}

	CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("MemoryAllocator Created"));
}

MemoryAllocator::~MemoryAllocator()
{
	CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("MemoryAllocator Distructed"));

	if (0 != m_iLogHandle)
	{
		m_pLogDescriptor->clLogReleaseClient(m_iLogHandle);
	}
}

/****************************************************************************************************************
 GetSupportedImageFormats
	Description
		This function returns the list of image formats supported by an OCL implementation when the information about
		an image memory object is specified and device supports image objects.
	Input
		flags					A bit-field that is used to specify allocation and usage information such as the memory arena
								that should be used to allocate the image object and how it will be used.
		imageType				Describes the image type as described in (cl_dev_mem_object_type).Only image formats are supported.
		numEntries				Specifies the number of entries that can be returned in the memory location given by formats.
								If value is 0 and formats is NULL, the num_entries_ret returns actual number of supported formats.
	Output
		formats					A pointer to array of structures that describes format properties of the image to be allocated.
								Refer to OCL spec section 5.2.4.1 for a detailed description of the image format descriptor.
		numEntriesRet			The actual number of supported image formats for a specific context and values specified by flags.
								If the value is NULL, it is ignored.
		Description
								Return the minimum number of image formats that should be supported according to Spec
	 Returns
		CL_DEV_SUCCESS			The function is executed successfully.
		CL_DEV_INVALID_VALUE	If values specified in parameters is not valid or if num_entries is 0 and formats is not NULL.
********************************************************************************************************************/
cl_dev_err_code MemoryAllocator::GetSupportedImageFormats( cl_mem_flags IN flags, cl_mem_object_type IN imageType,
				cl_uint IN numEntries, cl_image_format* OUT formats, cl_uint* OUT numEntriesRet) const
{
	//image_type describes the image type and must be either CL_MEM_OBJECT_IMAGE2D or
	//CL_MEM_OBJECT_IMAGE3D
	if(CL_MEM_OBJECT_BUFFER == imageType)
	{
		return CL_DEV_INVALID_VALUE;
	}

	if(0 == numEntries && NULL != formats)
	{
		return CL_DEV_INVALID_VALUE;
	}

	unsigned int uiNumEntries = NUM_OF_SUPPORTED_IMAGE_FORMATS;

	if(NULL != formats)
	{
		uiNumEntries = min(uiNumEntries, numEntries);
		memcpy(formats, supportedImageFormats, uiNumEntries * sizeof(supportedImageFormats[0]));
	}
	if(NULL != numEntriesRet)
	{
		*numEntriesRet = uiNumEntries;
	}

	return CL_DEV_SUCCESS;
}
/****************************************************************************************************************
 GetAllocProperties
	Description
		This function return allocator properties per memory object
	Input
		format 				    Image format
********************************************************************************************************************/
cl_dev_err_code MemoryAllocator::GetAllocProperties( cl_mem_object_type IN memObjType,	cl_dev_alloc_prop* OUT pAllocProp )
{
	assert( NULL != pAllocProp );

	pAllocProp->bufferSharingGroupId = CL_DEV_CPU_BUFFER_SHARING_GROUP_ID;
	pAllocProp->imageSharingGroupId  = CL_DEV_CPU_IMAGE_SHARING_GROUP_ID;
	pAllocProp->hostUnified          = true;
	pAllocProp->usedByDMA			 = false;
	pAllocProp->alignment            = CPU_DEV_MAXIMUM_ALIGN;
	pAllocProp->preferred_alignment  = CPU_DEV_MAXIMUM_ALIGN;
	pAllocProp->maxBufferSize        = m_maxAllocSize;
    pAllocProp->imagesSupported      = true;
	pAllocProp->DXSharing            = false;
	pAllocProp->GLSharing            = false;

	return CL_DEV_SUCCESS;
}

cl_dev_err_code MemoryAllocator::CreateObject( cl_dev_subdevice_id node_id, cl_mem_flags flags, const cl_image_format* format,
					 size_t dim_count, const size_t* dim,
					 IOCLDevRTMemObjectService* pRTMemObjService,
					 IOCLDevMemoryObject*  *memObj)
{
	CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("CreateObject enter"));

	assert(NULL != memObj);
	assert(NULL != dim);
	assert(MAX_WORK_DIM >= dim_count);
	assert(NULL != pRTMemObjService );

	// Allocate memory for memory object
	CPUDevMemoryObject*	pMemObj = new CPUDevMemoryObject(m_iLogHandle, m_pLogDescriptor, 
															node_id, flags,
															format, 
															dim_count, dim,
															pRTMemObjService, m_pImageService);
	if ( NULL == pMemObj )
	{
		CpuErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("Memory Object allocation failed"));
		return CL_DEV_OBJECT_ALLOC_FAIL;
	}

	cl_dev_err_code rc = pMemObj->Init();
	if ( CL_DEV_FAILED(rc) )
	{
		CpuErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("Memory Object descriptor allocation failed, rc=%x"), rc);
		delete pMemObj;
		return rc;
	}

	*memObj = pMemObj;

	return CL_DEV_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// Private functions
void* MemoryAllocator::CalculateOffsetPointer(void* pBasePtr, cl_uint dim_count, const size_t* origin, const size_t* pitch, size_t elemSize)
{
	char* lockedPtr = (char*)pBasePtr;
	if ( NULL != origin )
	{
		lockedPtr += origin[0] * elemSize; //Origin is in Pixels
		for(unsigned i=1; i<dim_count; ++i)
		{
			lockedPtr += origin[i] * pitch[i-1]; //y * image width pitch
		}
	}

	return lockedPtr;
}

//-----------------------------------------------------------------------------------------------------
// CPUDevMemoryObject
//-----------------------------------------------------------------------------------------------------

CPUDevMemoryObject::CPUDevMemoryObject(cl_int iLogHandle, IOCLDevLogDescriptor* pLogDescriptor, 
				   cl_dev_subdevice_id nodeId, cl_mem_flags memFlags,
				   const cl_image_format* pImgFormat, 
				   size_t dimCount, const size_t* dim,
				   IOCLDevRTMemObjectService* pRTMemObjService,
				   ICLDevBackendImageService* pImageService):
m_pLogDescriptor(pLogDescriptor), m_iLogHandle(iLogHandle),
m_nodeId(nodeId), m_memFlags(memFlags), m_pRTMemObjService(pRTMemObjService),
m_pBackingStore(NULL),  m_pImageService(pImageService)
{
	assert( NULL != m_pRTMemObjService);

	// Get only if there is available backing store.
	cl_dev_err_code bsErr;
	bsErr = m_pRTMemObjService->GetBackingStore(CL_DEV_BS_GET_ALWAYS, &m_pBackingStore);
	assert( CL_DEV_SUCCEEDED(bsErr) && (NULL != m_pBackingStore) );

	m_pBackingStore->AddPendency();
	
	m_objDecr.dim_count = (cl_uint)dimCount;

	if ( NULL != pImgFormat )
	{
		// Convert from User to Kernel format
		m_objDecr.format.image_channel_data_type = pImgFormat->image_channel_data_type - CL_SNORM_INT8;
		m_objDecr.format.image_channel_order = pImgFormat->image_channel_order - CL_R;
	}
	if ( 1 == dimCount )
	{
		m_objDecr.dimensions.buffer_size = dim[0];
	} else
	{
		const size_t* dims = m_pBackingStore->GetDimentions();
		for (unsigned int i = 0; i < dimCount; ++i)
		{
			m_objDecr.dimensions.dim[i] = (unsigned int)dims[i];
		}
	}

	m_objDecr.uiElementSize = (unsigned int)m_pBackingStore->GetElementSize();
	m_objDecr.pData = NULL;
    m_objDecr.memObjType = m_pRTMemObjService->GetMemObjectType();
}

CPUDevMemoryObject::~CPUDevMemoryObject()
{
	m_pBackingStore->RemovePendency();
}

cl_dev_err_code CPUDevMemoryObject::Init()
{
	m_objDecr.pData = m_pBackingStore->GetRawData();
	memcpy( m_objDecr.pitch, m_pBackingStore->GetPitch(), sizeof(m_objDecr.pitch) );

	//allocating the memory on the device by querying the backend for the size
	void* auxObject = NULL;
	if (m_objDecr.memObjType != CL_MEM_OBJECT_BUFFER)
	{
		size_t auxObjectSize=m_pImageService->GetAuxilarySize();
		auxObject = ALIGNED_MALLOC( auxObjectSize, CPU_DEV_MAXIMUM_ALIGN);
		if( NULL == auxObject )
		{
			CpuErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("Allocate aux image object failed"));
			return CL_DEV_ERROR_FAIL;
		}

		cl_dev_err_code rtErr = m_pImageService->CreateImageObject(&m_objDecr, (image_aux_data*)auxObject);
		if( CL_DEV_FAILED(rtErr) )
		{
			CpuErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("Create image failed"));
			return CL_DEV_ERROR_FAIL;
		}
	}
	else
		m_objDecr.imageAuxData = NULL;

	return CL_DEV_SUCCESS;
}

cl_dev_err_code CPUDevMemoryObject::clDevMemObjRelease( )
{
	CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("ReleaseObject enter"));

	void* auxObject = NULL;
	if(m_pImageService != NULL)
	{
		m_pImageService->DeleteImageObject(&m_objDecr, &auxObject);
	}

	if (NULL != auxObject)
	{
		ALIGNED_FREE( auxObject );
	}

	delete this;
	return CL_DEV_SUCCESS;
}

cl_dev_err_code CPUDevMemoryObject::clDevMemObjGetDescriptor(cl_device_type dev_type, cl_dev_subdevice_id node_id, cl_dev_memobj_handle *handle)
{
	assert(NULL != handle);

	*handle = (void*)(&m_objDecr);
	return CL_DEV_SUCCESS;
}

cl_dev_err_code CPUDevMemoryObject::clDevMemObjCreateMappedRegion(cl_dev_cmd_param_map* pMapParams)
{
	CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("CreateMappedRegion enter"));

	pMapParams->memObj = this;

	void*	pMapPtr = NULL;
	// Determine which pointer to use
	pMapPtr = m_objDecr.pData;
	size_t*	pitch = m_objDecr.pitch;

	assert(pMapParams->dim_count == m_objDecr.dim_count);
	pMapParams->ptr = MemoryAllocator::CalculateOffsetPointer(pMapPtr, m_objDecr.dim_count, pMapParams->origin, pitch, m_objDecr.uiElementSize);
	MEMCPY_S(pMapParams->pitch, sizeof(size_t)*(MAX_WORK_DIM-1), pitch, sizeof(size_t)*(m_objDecr.dim_count-1));
	return CL_DEV_SUCCESS;
}

cl_dev_err_code CPUDevMemoryObject::clDevMemObjReleaseMappedRegion( cl_dev_cmd_param_map* IN pMapParams )
{
	CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("ReleaseMappedRegion enter"));
	return CL_DEV_SUCCESS;
}

cl_dev_err_code CPUDevMemoryObject::clDevMemObjCreateSubObject( cl_mem_flags mem_flags, const size_t *origin,
										   const size_t *size, IOCLDevMemoryObject** ppSubBuffer )
{
	CPUDevMemorySubObject* pSubObject = new CPUDevMemorySubObject(m_iLogHandle, m_pLogDescriptor, this);
	if ( NULL == pSubObject )
	{
		return CL_DEV_OUT_OF_MEMORY;
	}

	cl_dev_err_code devErr = pSubObject->Init(mem_flags, origin, size);
	if ( CL_DEV_FAILED(devErr) )
	{
		delete pSubObject;
		return devErr;
	}

	assert (NULL != ppSubBuffer);
	*ppSubBuffer = pSubObject;
	return CL_DEV_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
CPUDevMemorySubObject::CPUDevMemorySubObject(cl_int iLogHandle, IOCLDevLogDescriptor* pLogDescriptor, CPUDevMemoryObject* pParent) :
	CPUDevMemoryObject(iLogHandle, pLogDescriptor), m_pParent(pParent)

{
}

cl_dev_err_code CPUDevMemorySubObject::Init(cl_mem_flags mem_flags, const size_t *origin, const size_t *size)
{
	MEMCPY_S(&m_objDecr, sizeof(cl_mem_obj_descriptor), &m_pParent->m_objDecr, sizeof(cl_mem_obj_descriptor));

	// Update dimensions
	m_objDecr.pData = MemoryAllocator::CalculateOffsetPointer(m_objDecr.pData, m_objDecr.dim_count, origin, m_objDecr.pitch, m_objDecr.uiElementSize);

	if ( CL_MEM_OBJECT_BUFFER == m_objDecr.memObjType )
	{
		m_objDecr.dimensions.buffer_size = size[0];
	}
	else
	{
		for(int i=0; i<MAX_WORK_DIM; ++i)
		{
			m_objDecr.dimensions.dim[i] = (unsigned int)size[i];
		}
	}

	m_memFlags = mem_flags;

	m_pBackingStore = m_pParent->m_pBackingStore;
	m_pBackingStore->AddPendency();

	return CL_DEV_SUCCESS;
}

