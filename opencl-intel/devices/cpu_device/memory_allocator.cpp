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

MemoryAllocator::MemoryAllocator(cl_int devId, IOCLDevLogDescriptor *logDesc) :
	m_iDevId(devId), m_pLogDescriptor(logDesc), m_iLogHandle(0), m_objHandles(1, UINT_MAX)
{
	if ( NULL != logDesc )
	{
		cl_int ret = m_pLogDescriptor->clLogCreateClient(m_iDevId, L"CPU Device: Memory Allocator", &m_iLogHandle);
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
	TMemObjectsMap::const_iterator it;

	for(it=m_mapObjects.begin(); m_mapObjects.end() != it ; ++it )
	{
		SMemObjectDescriptor* pObjDesc = it->second;

		// Did we allocate the buffer
		if ( pObjDesc->pHostPtr != pObjDesc->objDecr.pData )
		{
			//VirtualFree(pObjDesc->objDecr.pData, 0, MEM_RELEASE);
			ALIGNED_FREE(pObjDesc->objDecr.pData);
		}

		delete pObjDesc->myHandle;
		delete pObjDesc;
	}
	m_mapObjects.clear();

	if (0 != m_iLogHandle)
	{
		m_pLogDescriptor->clLogReleaseClient(m_iLogHandle);
	}
}

// Checks that given object is valid object and belongs to memory allocator
cl_dev_err_code	MemoryAllocator::ValidateObject( cl_dev_mem IN memObj )
{
	CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("ValidateObject enter"));

	if ( (NULL == memObj) || (memObj->allocId != m_iDevId) )
	{
		return CL_DEV_INVALID_MEM_OBJECT;
	}

	return CL_DEV_SUCCESS;
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
cl_dev_err_code MemoryAllocator::GetSupportedImageFormats( cl_dev_mem_flags IN flags, cl_dev_mem_object_type IN imageType,
				cl_uint IN numEntries, cl_image_format* OUT formats, cl_uint* OUT numEntriesRet)
{
	//image_type describes the image type and must be either CL_MEM_OBJECT_IMAGE2D or
	//CL_MEM_OBJECT_IMAGE3D
	if(CL_DEV_MEM_OBJECT_BUFFER == imageType)
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
		memcpy(formats, suportedImageFormats, uiNumEntries * sizeof(suportedImageFormats[0]));
	}
	if(NULL != numEntriesRet)
	{
		*numEntriesRet = uiNumEntries;
	}

	return CL_DEV_SUCCESS;
}
/****************************************************************************************************************
 AllocateImage
	Description
		Allocate Image\buffer Memory
	Input
		dim[0] specifies width of the image in pixel,
		dim[1] specifies height of the image in pixel,
		dim[2] specifies depth of the image in pixel,
	Output
		return value is a pointer to memory buffer
********************************************************************************************************************/
void*	MemoryAllocator::AllocateMem(cl_uint dim_count, const size_t* dim, size_t* pitch)
{
	size_t allocMemSize;

	allocMemSize = dim[0];
	//Allocate memory according to pitch size and dim size
	for(unsigned int i=1; i<dim_count; i++)
	{
		allocMemSize += dim[i] * pitch[i-1];
	}

	// Use Virtual memory allocator
	// Enable read/write access
	//void *pBuffer = VirtualAlloc(NULL, allocMemSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	void *pBuffer = ALIGNED_MALLOC(allocMemSize, CPU_DEV_MAXIMUM_ALIGN);
#ifdef _DEBUG
	memset(pBuffer, 0x69, allocMemSize);
#endif
	return pBuffer;
}
/****************************************************************************************************************
 ElementSize
	Description
		This function calculated the expected element size
	Input
		format 				    Image format
********************************************************************************************************************/
size_t MemoryAllocator::ElementSize(const cl_image_format* format)
{
	size_t stChannels = 0;
	size_t stChSize = 0;
	switch (format->image_channel_order)
	{
	case CL_R:case CL_A:case CL_LUMINANCE:case CL_INTENSITY:
	case CL_RGB:	// Special case, must be used only with specific data type
		stChannels = 1;
		break;
	case CL_RG:case CL_RA:
		stChannels = 2;
		break;
	case CL_RGBA: case CL_ARGB: case CL_BGRA:
		stChannels = 4;
		break;
	default:
		assert(0);
	}
	switch (format->image_channel_data_type)
	{
		case (CL_SNORM_INT8):
		case (CL_UNORM_INT8):
		case (CL_SIGNED_INT8):
		case (CL_UNSIGNED_INT8):
				stChSize = 1;
				break;
		case (CL_SNORM_INT16):
		case (CL_UNORM_INT16):
		case (CL_UNSIGNED_INT16):
		case (CL_SIGNED_INT16):
		case (CL_HALF_FLOAT):
		case CL_UNORM_SHORT_555:
		case CL_UNORM_SHORT_565:
				stChSize = 2;
				break;
		case (CL_SIGNED_INT32):
		case (CL_UNSIGNED_INT32):
		case (CL_FLOAT):
		case CL_UNORM_INT_101010:
				stChSize = 4;
				break;
		default:
				assert(0);
	}

	return stChannels*stChSize;
}


cl_dev_err_code MemoryAllocator::CreateObject( cl_dev_mem_flags IN flags, const cl_image_format* IN format,
									 cl_uint dim_count, const size_t* dim,
									 void*	buffer_ptr, const size_t* pitch, cl_dev_host_ptr_flags host_flags,
									 cl_dev_mem* OUT memObj )
{
	CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("CreateObject enter"));
	size_t bufferPitch[MAX_WORK_DIM-1];
	//size_t expectedPitchRowSize = 0;
	//size_t expectedPitchSliceSize = 0;
	size_t uiElementSize = 1;

#ifdef _DEBUG
	if ( (NULL == memObj) || (NULL == dim) )
	{
		return CL_DEV_INVALID_VALUE;
	}

	if ( MAX_WORK_DIM < dim_count )
	{
		return CL_DEV_INVALID_IMG_SIZE;
	}

	if ( (1 != dim_count) && (NULL == buffer_ptr) && (NULL != pitch) )
	{
		CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("Pich must be 0 when host ptr is NULL"));
		return CL_DEV_INVALID_VALUE;
	}
#endif

	if ( 1 < dim_count)
	{
		/*dim[0] is image width */
		uiElementSize = ElementSize(format);

		bufferPitch[0] = uiElementSize * dim[0];
		for(unsigned i=1; i<dim_count-1; ++i)
		{
			bufferPitch[i] = bufferPitch[i-1]*dim[i];
		}
	}

#ifdef _DEBUG
	// Test for valid pitch values
	if ( NULL != pitch )
	{
		for(unsigned i=0; i < dim_count-1; ++i)
		{
			if ( pitch[i] < bufferPitch[i])
			{
				CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("Wrong Pitch Value Enterd"));
				return CL_DEV_INVALID_VALUE;
			}
		}
	}
#endif

	// Allocate new object handle
	unsigned int uiNewObject;

	if ( !m_objHandles.AllocateHandle(&uiNewObject) )
	{
		CpuErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("Can't allocate new object handle"));
		return CL_DEV_OBJECT_ALLOC_FAIL;
	}
	// Allocate memory for memory object
	cl_dev_mem	pMemObj = new _cl_dev_mem;

	if ( NULL == pMemObj )
	{
		m_objHandles.FreeHandle(uiNewObject);
		CpuErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("Memory Object allocation failed"));
		return CL_DEV_OBJECT_ALLOC_FAIL;
	}

	// Allocate memory for object descriptor
	SMemObjectDescriptor*	pMemObjDesc = new SMemObjectDescriptor;
	if ( NULL == pMemObjDesc )
	{
		CpuErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("Memory Object descriptor allocation failed"));
		m_objHandles.FreeHandle(uiNewObject);
		delete pMemObj;
		return CL_DEV_OBJECT_ALLOC_FAIL;
	}

	memset(pMemObjDesc, 0, sizeof(SMemObjectDescriptor));

	pMemObjDesc->bObjLocked = false;
	// Store external buffer pointer & its usage
	pMemObjDesc->pHostPtr = buffer_ptr;
	pMemObjDesc->clHostPtrFlags = host_flags;

	bool bNotAligned = ( (((int)(size_t)buffer_ptr) & (CPU_DEV_MAXIMUM_ALIGN-1)) != 0);

	if ( NULL == buffer_ptr || bNotAligned) 		// Allocate memory for the new object
	{
		pMemObjDesc->objDecr.pData = AllocateMem(dim_count, dim, bufferPitch);
		if ( NULL == pMemObjDesc->objDecr.pData )
		{
			CpuErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("Memory Object memory buffer Allocation failed"));
			m_objHandles.FreeHandle(uiNewObject);
			delete pMemObjDesc;
			delete pMemObj;

			return CL_DEV_OBJECT_ALLOC_FAIL;
		}
		// Set pitch of the object
		memcpy(pMemObjDesc->objDecr.pitch, bufferPitch, (dim_count-1)*sizeof(size_t));
	} else
	{
		// The object pointers to external buffer
		pMemObjDesc->objDecr.pData = buffer_ptr;
		memcpy(pMemObjDesc->objDecr.pitch, pitch, (dim_count-1)*sizeof(size_t));
	}

	pMemObjDesc->objDecr.dim_count = dim_count;
	pMemObjDesc->memFlags = flags;
	assert(uiElementSize <= CL_MAX_UINT32);
	pMemObjDesc->objDecr.uiElementSize = (unsigned int)uiElementSize;
	if ( NULL != format )
	{
		// Convert from User to Kernel format
		pMemObjDesc->objDecr.format.image_channel_data_type = format->image_channel_data_type - CL_SNORM_INT8;
		pMemObjDesc->objDecr.format.image_channel_order = format->image_channel_order - CL_R;
	} else
	{
		memset(&pMemObjDesc->objDecr.format, 0, sizeof(cl_image_format));
	}

	for (size_t i = 0; i < dim_count; i++) {
		pMemObjDesc->objDecr.dim[i] = (unsigned int)dim[i];
	}

	pMemObjDesc->myHandle = pMemObj;

	// Copy initial data if required:
	//		Data available and our PTR is not HOST ptr
	if ( (host_flags & CL_DEV_HOST_PTR_DATA_AVAIL) &&
		(pMemObjDesc->pHostPtr != pMemObjDesc->objDecr.pData) )
	{
		SMemCpyParams sCpyPrm;

		sCpyPrm.uiDimCount = dim_count;
		sCpyPrm.pSrc = (cl_char*)pMemObjDesc->pHostPtr;

		sCpyPrm.pDst = (cl_char*)pMemObjDesc->objDecr.pData;
		for(unsigned int i=0; i< dim_count-1; i++)
		{
			sCpyPrm.vDstPitch[i] = pMemObjDesc->objDecr.pitch[i];

			if(NULL != pitch)
			{
				sCpyPrm.vSrcPitch[i] = pitch[i];
			}
			else
			{
				sCpyPrm.vSrcPitch[i] = 0;
			}

		}
		for(unsigned int i=0; i< dim_count; i++)
		{
			sCpyPrm.vRegion[i] = pMemObjDesc->objDecr.dim[i];

		}



		sCpyPrm.vRegion[0] = sCpyPrm.vRegion[0] * uiElementSize;
		// Copy original buffer to internal area
		CopyMemoryBuffer(&sCpyPrm);
	}

	m_muObjectMap.Lock();
	m_mapObjects[uiNewObject] = pMemObjDesc;
	m_muObjectMap.Unlock();

	// Add object to map
	pMemObj->allocId = m_iDevId;
	pMemObj->objHandle = (void*)uiNewObject;

	*memObj = pMemObj;

	return CL_DEV_SUCCESS;
}

cl_dev_err_code MemoryAllocator::ReleaseObject( cl_dev_mem IN memObj )
{
	CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("ReleaseObject enter"));

	if ( (NULL == memObj) || (m_iDevId != memObj->allocId) )
	{
		return CL_DEV_INVALID_MEM_OBJECT;
	}

	TMemObjectsMap::iterator it;

	m_muObjectMap.Lock();
	it = m_mapObjects.find((unsigned int)(size_t)memObj->objHandle);
	if ( m_mapObjects.end() == it )
	{
		m_muObjectMap.Unlock();
		CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("Memory object coudn't be found"));
		return CL_DEV_INVALID_MEM_OBJECT;
	}

	SMemObjectDescriptor* pObjDesc = it->second;
	if ( pObjDesc->bObjLocked )
	{
		m_muObjectMap.Unlock();
		CpuErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("Cannot release locked object, id: %d"),
			(size_t)memObj->objHandle );
		return CL_DEV_OBJECT_ALREADY_LOCKED;
	}
	// delete object from map
	m_mapObjects.erase(it);
	m_muObjectMap.Unlock();

	// Did we allocate the buffer
	if ( pObjDesc->pHostPtr != pObjDesc->objDecr.pData )
	{
		//VirtualFree(pObjDesc->objDecr.pData, 0, MEM_RELEASE);
		ALIGNED_FREE(pObjDesc->objDecr.pData);
	}

	delete pObjDesc;
	delete memObj;

	return CL_DEV_SUCCESS;
}

cl_dev_err_code MemoryAllocator::LockObject(cl_dev_mem IN pMemObj, cl_uint dim_count, const size_t* IN origin,
							void** OUT ptr, size_t* OUT pitch, size_t* OUT uiElementSize)
{
	OclAutoMutex lock(&m_muObjectMap, false);	// no auto lock

	if ( NULL == pMemObj )
	{
//		CpuErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("pMemObj is NULL"));
		return CL_DEV_INVALID_MEM_OBJECT;
	}
	if ( m_iDevId != pMemObj->allocId )
	{
		CpuErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("Invalid device ID:%X"), pMemObj->allocId);
		return CL_DEV_INVALID_MEM_OBJECT;
	}

	if ( NULL == ptr )
	{
		return CL_DEV_INVALID_VALUE;
	}

	TMemObjectsMap::const_iterator it;

	m_muObjectMap.Lock();
	it = m_mapObjects.find((unsigned int)(size_t)pMemObj->objHandle);
	if ( m_mapObjects.end() == it )
	{
		m_muObjectMap.Unlock();
		CpuErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("Memory object coudn't be found, id: %d"),
			(size_t)pMemObj->objHandle );
		return CL_DEV_INVALID_MEM_OBJECT;
	}

	SMemObjectDescriptor* pObjDesc = it->second;
#ifdef _ENABLE_LOCK_OBJECTS_
	if ( pObjDesc->bObjLocked )
	{
		CpuErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("Memory object already locked, id: %d"),
			(size_t)pMemObj->objHandle );
		return CL_DEV_OBJECT_ALREADY_LOCKED;
	}
#endif

	cl_dev_err_code rc = CalculateOffsetPointer(pObjDesc, pObjDesc->objDecr.pData, dim_count, origin, ptr, pitch, uiElementSize);
	if ( CL_DEV_FAILED(rc) )
	{
		CpuErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("Failed to calculate offset, id: %d, rc=%x"),
			(size_t)pMemObj->objHandle, rc );
		return rc;
	}

#ifdef _ENABLE_LOCK_OBJECTS_
	pObjDesc->bObjLocked = true;
#endif

	return CL_DEV_SUCCESS;
}

cl_dev_err_code MemoryAllocator::LockObject(cl_dev_mem IN pMemObj, cl_mem_obj_descriptor* OUT *pMemObjDesc)
{
	OclAutoMutex lock(&m_muObjectMap, false);	// no auto lock

	if ( m_iDevId != pMemObj->allocId)
	{
		CpuErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("Invalid device ID:%X"), pMemObj->allocId);
		return CL_DEV_INVALID_MEM_OBJECT;
	}

	TMemObjectsMap::const_iterator it;

	m_muObjectMap.Lock();
	it = m_mapObjects.find((unsigned int)(size_t)pMemObj->objHandle);
	if ( m_mapObjects.end() == it )
	{
		m_muObjectMap.Unlock();
		CpuErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("Memory object coudn't be found, id: %d"),
			(size_t)pMemObj->objHandle );
		return CL_DEV_INVALID_MEM_OBJECT;
	}

	SMemObjectDescriptor* pObjDesc = it->second;
#ifdef _ENABLE_LOCK_OBJECTS_
	if ( pObjDesc->bObjLocked )
	{
		CpuErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("Memory object already locked, id: %d"),
			(size_t)pMemObj->objHandle );
		return CL_DEV_OBJECT_ALREADY_LOCKED;
	}
#endif

	*pMemObjDesc = &pObjDesc->objDecr;

#ifdef _ENABLE_LOCK_OBJECTS_
	pObjDesc->bObjLocked = true;
#endif

	return CL_DEV_SUCCESS;
}

cl_dev_err_code MemoryAllocator::UnLockObject(cl_dev_mem IN pMemObj, void* IN ptr)
{
	OclAutoMutex lock(&m_muObjectMap, false);	// no auto lock

	if ( NULL == ptr )
	{
		CpuErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("Cannot unlock NULL pointer"));
		return CL_DEV_INVALID_VALUE;
	}

	if ( m_iDevId != pMemObj->allocId)
	{
		CpuErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("Invalid device ID:%X"), pMemObj->allocId);
		return CL_DEV_INVALID_MEM_OBJECT;
	}

	TMemObjectsMap::const_iterator it;

	m_muObjectMap.Lock();
	it = m_mapObjects.find((unsigned int)(size_t)pMemObj->objHandle);
	if ( m_mapObjects.end() == it )
	{
		m_muObjectMap.Unlock();
		CpuErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("Memory object coudn't be found, id: %d"),
			(size_t)pMemObj->objHandle );
		return CL_DEV_INVALID_MEM_OBJECT;
	}

#ifdef _ENABLE_LOCK_OBJECTS_
	SMemObjectDescriptor* pObjDesc = it->second;
	if ( !pObjDesc->bObjLocked )
	{
		CpuErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("Memory object already unlocked, id: %d"),
			(unsigned int)pMemObj->objHandle );
		return CL_DEV_INVALID_OPERATION;
	}

	pObjDesc->bObjLocked = false;
#endif

	return CL_DEV_SUCCESS;
}

cl_dev_err_code MemoryAllocator::CreateMappedRegion(cl_dev_cmd_param_map* INOUT pMapParams)
{
	CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("CreateMappedRegion enter"));

	cl_dev_mem pMemObj = pMapParams->memObj;

	if ( m_iDevId != pMemObj->allocId)
	{
		CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("Invalid device ID:%X"), pMemObj->allocId);
		return CL_DEV_INVALID_MEM_OBJECT;
	}

	TMemObjectsMap::const_iterator it;

	m_muObjectMap.Lock();
	it = m_mapObjects.find((unsigned int)(size_t)pMemObj->objHandle);
	if ( m_mapObjects.end() == it )
	{
		m_muObjectMap.Unlock();
		CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("Memory object coudn't be found"));
		return CL_DEV_INVALID_MEM_OBJECT;
	}

	SMemObjectDescriptor* pObjDesc = it->second;
	m_muObjectMap.Unlock();

	void*	pMapPtr = NULL;
	// Determine which pointer to use
	pMapPtr = pObjDesc->clHostPtrFlags & CL_DEV_HOST_PTR_MAPPED_REGION ? pObjDesc->pHostPtr : pObjDesc->objDecr.pData;

	cl_dev_err_code ret = CalculateOffsetPointer(pObjDesc, pMapPtr, pMapParams->dim_count, pMapParams->origin, &(pMapParams->ptr), pMapParams->pitch, NULL);

	return ret;
}

cl_dev_err_code MemoryAllocator::ReleaseMappedRegion( cl_dev_cmd_param_map* IN pMapParams )
{
	CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("ReleaseMappedRegion enter"));
	return CL_DEV_SUCCESS;
}

void MemoryAllocator::CopyMemoryBuffer(SMemCpyParams* pCopyCmd)
{
	// Copy 1D array only
	if ( 1 == pCopyCmd->uiDimCount )
	{
		memcpy(pCopyCmd->pDst, pCopyCmd->pSrc, pCopyCmd->vRegion[0]);
		return;
	}

	SMemCpyParams sRecParam;

	// Copy current parameters
	memcpy(&sRecParam, pCopyCmd, sizeof(SMemCpyParams));
	sRecParam.uiDimCount = pCopyCmd->uiDimCount-1;
	// Make recursion
	for(unsigned int i=0; i<pCopyCmd->vRegion[sRecParam.uiDimCount]; ++i)
	{
		CopyMemoryBuffer(&sRecParam);
		sRecParam.pSrc = sRecParam.pSrc + pCopyCmd->vSrcPitch[sRecParam.uiDimCount-1];
		sRecParam.pDst = sRecParam.pDst + pCopyCmd->vDstPitch[sRecParam.uiDimCount-1];
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// Private functions
cl_dev_err_code MemoryAllocator::CalculateOffsetPointer(const SMemObjectDescriptor* pObjDesc, void* pBasePtr,
											   cl_uint dim_count, const size_t* IN origin,
												void** OUT ptr, size_t* OUT pitch, size_t* OUT uiElementSize)
{
	if ( ((cl_uint)-1 != dim_count) && (dim_count != pObjDesc->objDecr.dim_count) )
	{
		return CL_DEV_INVALID_IMG_SIZE;
	}

	if(NULL != pitch && dim_count != 1)
	{
		// In case of Buffer, objDecr.pitch[i]=0 in all dimensions; so there is no point in reading it;
		// However, reading it can override BufferRect function parameters.
		for(unsigned i=0; i<MAX_WORK_DIM-1; ++i)
		{
			pitch[i] = pObjDesc->objDecr.pitch[i];
		}
	}

	if (NULL != uiElementSize)
	{
		*uiElementSize = pObjDesc->objDecr.uiElementSize;
	}

	cl_char* lockedPtr = (cl_char*)pBasePtr;
	if ( NULL != origin )
	{
		lockedPtr += origin[0] * pObjDesc->objDecr.uiElementSize; //Origin is in Pixels
		for(unsigned i=1; i<MAX_WORK_DIM; ++i)
		{
			lockedPtr += origin[i] * pitch[i-1]; //y * image width pitch
		}
	}

	*ptr = lockedPtr;

	return CL_DEV_SUCCESS;
}
