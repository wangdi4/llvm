// INTEL CONFIDENTIAL
//
// Copyright 2006-2018 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

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
	if ( nullptr != logDesc )
	{
		cl_int ret = m_pLogDescriptor->clLogCreateClient(m_iDevId, TEXT("CPU Device: Memory Allocator"), &m_iLogHandle);
		if(CL_DEV_SUCCESS != ret)
		{
			m_iLogHandle = 0;
		}
	}

	CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("MemoryAllocator Created"));
}

MemoryAllocator::~MemoryAllocator()
{
	CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("MemoryAllocator Destructed"));

	if (0 != m_iLogHandle)
	{
		m_pLogDescriptor->clLogReleaseClient(m_iLogHandle);
	}
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
	assert( nullptr != pAllocProp );

	pAllocProp->bufferSharingGroupId = CL_DEV_CPU_BUFFER_SHARING_GROUP_ID;
	pAllocProp->imageSharingGroupId  = CL_DEV_CPU_IMAGE_SHARING_GROUP_ID;
	pAllocProp->mustAllocRawMemory   = false; // CPU should not allocate data instead of RT
	pAllocProp->usedByDMA            = false;
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
	CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("Enter node_id:%d, flags:%x"), node_id, flags);

	assert(nullptr != memObj);
	assert(nullptr != dim);
	assert(MAX_WORK_DIM >= dim_count);
	assert(nullptr != pRTMemObjService );

	// Allocate memory for memory object
	CPUDevMemoryObject*	pMemObj = new CPUDevMemoryObject(m_iLogHandle, m_pLogDescriptor,
															node_id, flags,
															format,
															dim_count, dim,
															pRTMemObjService, m_pImageService);
	if ( nullptr == pMemObj )
	{
		CpuErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), "Memory Object allocation failed");
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

	CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), "Done");

	return CL_DEV_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// Private functions
void* MemoryAllocator::CalculateOffsetPointer(void* pBasePtr, cl_uint dim_count, const size_t* origin, const size_t* pitch, size_t elemSize)
{
	char* lockedPtr = (char*)pBasePtr;
	if ( nullptr != origin )
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
m_pBackingStore(nullptr),  m_pImageService(pImageService)
{
	assert( nullptr != m_pRTMemObjService);

	// Get only if there is available backing store.
	cl_dev_err_code bsErr;
	bsErr = m_pRTMemObjService->GetBackingStore(CL_DEV_BS_GET_ALWAYS, &m_pBackingStore);
	assert( CL_DEV_SUCCEEDED(bsErr) && (nullptr != m_pBackingStore) );

	m_pBackingStore->AddPendency();
	
	m_objDecr.dim_count = (cl_uint)dimCount;

	if ( nullptr != pImgFormat )
	{
		// Convert from User to Kernel format
		m_objDecr.format.image_channel_data_type = pImgFormat->image_channel_data_type - CL_SNORM_INT8 + CLK_SNORM_INT8;
		m_objDecr.format.image_channel_order = pImgFormat->image_channel_order - CL_R + CLK_R;
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
	m_objDecr.pData = nullptr;
    m_objDecr.memObjType = m_pRTMemObjService->GetMemObjectType();
}

CPUDevMemoryObject::~CPUDevMemoryObject()
{
	m_pBackingStore->RemovePendency();
}

cl_dev_err_code CPUDevMemoryObject::Init()
{
	m_objDecr.pData = m_pBackingStore->GetRawData();
	MEMCPY_S( m_objDecr.pitch, sizeof(m_objDecr.pitch), m_pBackingStore->GetPitch(), sizeof(m_objDecr.pitch) );

	//allocating the memory on the device by querying the backend for the size
	void* auxObject = nullptr;
	if (m_objDecr.memObjType != CL_MEM_OBJECT_BUFFER && m_objDecr.memObjType != CL_MEM_OBJECT_PIPE)
	{
		size_t auxObjectSize=m_pImageService->GetAuxilarySize();
		auxObject = ALIGNED_MALLOC( auxObjectSize, CPU_DEV_MAXIMUM_ALIGN);
		if( nullptr == auxObject )
		{
			CpuErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("Allocate aux image object failed"));
			return CL_DEV_ERROR_FAIL;
		}

		cl_dev_err_code rtErr = m_pImageService->CreateImageObject(&m_objDecr, (image_aux_data*)auxObject);
		if( CL_DEV_FAILED(rtErr) )
		{
			CpuErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("Create image failed"));
			return CL_DEV_ERROR_FAIL;
		}
	}
	else
		m_objDecr.imageAuxData = nullptr;

	return CL_DEV_SUCCESS;
}

cl_dev_err_code CPUDevMemoryObject::clDevMemObjRelease( )
{
	CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("ReleaseObject enter"));

	void* auxObject = nullptr;
	if(m_pImageService != nullptr)
	{
		m_pImageService->DeleteImageObject(&m_objDecr, &auxObject);
	}

	if (nullptr != auxObject)
	{
		ALIGNED_FREE( auxObject );
	}

	delete this;
	return CL_DEV_SUCCESS;
}

cl_dev_err_code CPUDevMemoryObject::clDevMemObjGetDescriptor(cl_device_type dev_type, cl_dev_subdevice_id node_id, cl_dev_memobj_handle *handle)
{
	assert(nullptr != handle);

	*handle = (void*)(&m_objDecr);
	return CL_DEV_SUCCESS;
}

cl_dev_err_code CPUDevMemoryObject::clDevMemObjCreateMappedRegion(cl_dev_cmd_param_map* pMapParams)
{
	CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("CreateMappedRegion enter"));

	pMapParams->memObj = this;

	void*	pMapPtr = nullptr;
	// Determine which pointer to use
	pMapPtr = m_objDecr.pData;
	size_t*	pitch = m_objDecr.pitch;

	assert(pMapParams->dim_count == m_objDecr.dim_count);
	pMapParams->ptr = MemoryAllocator::CalculateOffsetPointer(pMapPtr, m_objDecr.dim_count, pMapParams->origin, pitch, m_objDecr.uiElementSize);
	MEMCPY_S(pMapParams->pitch, sizeof(size_t)*(MAX_WORK_DIM-1), pitch, sizeof(size_t)*(MAX_WORK_DIM-1));
	return CL_DEV_SUCCESS;
}

cl_dev_err_code CPUDevMemoryObject::clDevMemObjReleaseMappedRegion( cl_dev_cmd_param_map* IN pMapParams )
{
	CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("ReleaseMappedRegion enter"));
	return CL_DEV_SUCCESS;
}

cl_dev_err_code CPUDevMemoryObject::clDevMemObjCreateSubObject( cl_mem_flags mem_flags, const size_t *origin,
										   const size_t *size,
										   IOCLDevRTMemObjectService IN *pBSService,
										   IOCLDevMemoryObject** ppSubBuffer )
{
	CPUDevMemorySubObject* pSubObject = new CPUDevMemorySubObject(m_iLogHandle, m_pLogDescriptor, this);
	if ( nullptr == pSubObject )
	{
		return CL_DEV_OUT_OF_MEMORY;
	}

	cl_dev_err_code devErr = pSubObject->Init(mem_flags, origin, size, pBSService);
	if ( CL_DEV_FAILED(devErr) )
	{
		delete pSubObject;
		return devErr;
	}

	assert (nullptr != ppSubBuffer);
	*ppSubBuffer = pSubObject;
	return CL_DEV_SUCCESS;
}

cl_dev_err_code CPUDevMemoryObject::clDevMemObjUpdateBackingStore(
                            void* operation_handle, cl_dev_bs_update_state* pUpdateState )
{
	CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("clDevMemObjUpdateBackingStore enter"));
    assert( nullptr != pUpdateState );
    *pUpdateState = CL_DEV_BS_UPDATE_COMPLETED;
    return CL_DEV_SUCCESS;
}

cl_dev_err_code CPUDevMemoryObject::clDevMemObjUpdateFromBackingStore(
                            void* operation_handle, cl_dev_bs_update_state* pUpdateState )
{
	CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("clDevMemObjUpdateFromBackingStore enter"));
    assert( nullptr != pUpdateState );
    *pUpdateState = CL_DEV_BS_UPDATE_COMPLETED;
    return CL_DEV_SUCCESS;
}

cl_dev_err_code CPUDevMemoryObject::clDevMemObjInvalidateData( )
{
	CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("clDevMemObjInvalidateData enter"));
    return CL_DEV_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
CPUDevMemorySubObject::CPUDevMemorySubObject(cl_int iLogHandle, IOCLDevLogDescriptor* pLogDescriptor, CPUDevMemoryObject* pParent) :
	CPUDevMemoryObject(iLogHandle, pLogDescriptor), m_pParent(pParent)

{
}

cl_dev_err_code CPUDevMemorySubObject::Init(cl_mem_flags mem_flags, const size_t *origin, const size_t *size,
                                            IOCLDevRTMemObjectService IN *pBSService)
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

    m_pRTMemObjService = pBSService;

    cl_dev_err_code bsErr = m_pRTMemObjService->GetBackingStore(CL_DEV_BS_GET_ALWAYS, &m_pBackingStore);
    assert( CL_DEV_SUCCEEDED(bsErr) && (nullptr != m_pBackingStore) );

    if (CL_DEV_FAILED(bsErr) || (nullptr == m_pBackingStore))
    {
        CpuErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("GetBackingStore failed"));
        return CL_DEV_ERROR_FAIL;
    }

	m_pBackingStore->AddPendency();

	return CL_DEV_SUCCESS;
}

