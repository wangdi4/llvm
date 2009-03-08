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


const cl_image_format suportedImageFormats[] = {
	{CL_RGBA, CL_UNORM_INT8},
	{CL_RGBA, CL_UNORM_INT16},
	{CL_RGBA, CL_SIGNED_INT8},
	{CL_RGBA, CL_SIGNED_INT16},
	{CL_RGBA, CL_SIGNED_INT32},
	{CL_RGBA, CL_UNSIGNED_INT8},
	{CL_RGBA, CL_UNSIGNED_INT16},
	{CL_RGBA, CL_UNSIGNED_INT32},
	{CL_RGBA, CL_FLOAT},
	{CL_RGBA, CL_HALF_FLOAT},
	{CL_BGRA, CL_UNORM_INT8}
};

const unsigned int NUM_OF_SUPPORTED_IMAGE_FORMATS = sizeof(suportedImageFormats)/sizeof(cl_image_format);
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
cl_int MemoryAllocator::GetSupportedImageFormats( cl_dev_mem_flags IN flags, cl_dev_mem_object_type IN imageType,
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
	
	unsigned int uiNumEntries = min(numEntries, NUM_OF_SUPPORTED_IMAGE_FORMATS);

	if(NULL != formats)
	{
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
void*	MemoryAllocator::AllocateMem(cl_uint	dim_count, const size_t* dim, size_t expectedPitchRowSize, size_t expectedPitchSliceSize)
{
	size_t allocatedMemSize;
	size_t pitch[MAX_WORK_DIM-1];
	pitch[0] = expectedPitchRowSize;
	pitch[1] = expectedPitchSliceSize;


	allocatedMemSize = dim[0];
	//Allocate memory according to pitch size and dim size
	for(unsigned int i=1; i<dim_count; i++)
	{
		allocatedMemSize += dim[i] * pitch[i-1];
	}
	return _aligned_malloc(allocatedMemSize, CPU_DCU_LINE_SIZE);
}
/****************************************************************************************************************
 ElementSize
	Description
		This function calculated the expected elemnt size
	Input
		format 				    Image format
********************************************************************************************************************/
size_t MemoryAllocator::ElementSize(const cl_image_format* format)
{
	if (CL_RGBA == format->image_channel_order || CL_RGBA == format->image_channel_order)
	{
		switch (format->image_channel_data_type)
		{
			case (CL_UNORM_INT8):
			case (CL_SIGNED_INT8):
					return 4;
			case (CL_UNORM_INT16):
			case (CL_UNSIGNED_INT16):
			case (CL_HALF_FLOAT):
					return 8;
			case (CL_SIGNED_INT32):
			case (CL_UNSIGNED_INT32):
					return  16;
			case (CL_FLOAT):
					return 4 * sizeof (float);
			default: 
					assert(0);
					return 0;
		}
		
	}
	assert(0);
	return 0;
}


cl_int MemoryAllocator::CreateObject( cl_dev_mem_flags IN flags, const cl_image_format* IN format,
									 cl_uint dim_count, const size_t* dim,
									 void*	buffer_ptr, const size_t* pitch,
									 cl_dev_mem* OUT memObj )
{
	InfoLog(m_logDescriptor, m_iLogHandle, L"CreateObject enter");
	size_t expectedPitchRowSize = 0;
	size_t expectedPitchSliceSize = 0;
	size_t uiElementSize = 1; 

	if ( (NULL == memObj) || (NULL == dim) )
	{
		return CL_DEV_INVALID_VALUE;
	}

	if ( 1 != dim_count && NULL == buffer_ptr && NULL != pitch ) 
	{
		InfoLog(m_logDescriptor, m_iLogHandle, L"Pich must be 0 when host ptr is NULL");
		return CL_DEV_INVALID_VALUE;
	}


	if ( 1 != dim_count)
	{
		/*dim[0] is image width */
		uiElementSize = ElementSize(format);

		expectedPitchRowSize = uiElementSize * dim[0];

		if (NULL != pitch)
		{
			if(pitch[0] < expectedPitchRowSize && 0 != pitch[0])
			{
					if( NULL == buffer_ptr )
					{
						InfoLog(m_logDescriptor, m_iLogHandle, L"Wrong Pitch Value Enterd");
						return CL_DEV_INVALID_VALUE;
					}
			}
			else if (0 != pitch[0])
			{
					expectedPitchRowSize = pitch[0];
			}
		}
		if ( 3 == dim_count)
		{
			//expectedPitchSliceSize >= image_row_pitch * image_height
			expectedPitchSliceSize = expectedPitchRowSize * dim[1];
			if (NULL != pitch) 
			{
				if(pitch[1] < expectedPitchSliceSize && 0 != pitch[1])
				{
						if( NULL == buffer_ptr )
						{
							InfoLog(m_logDescriptor, m_iLogHandle, L"Wrong Pitch Value Enterd");
							return CL_DEV_INVALID_VALUE;
						}
				}
				else if (0 != pitch[1])
				{
						expectedPitchSliceSize = pitch[1];
				}
			}
		}

	}
	
	if ( MAX_WORK_DIM < dim_count )
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
		pMemObjDesc->pObject = AllocateMem(dim_count, dim, expectedPitchRowSize, expectedPitchSliceSize);
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
	pMemObjDesc->stPitch[0] = expectedPitchRowSize;
	pMemObjDesc->stPitch[1] = expectedPitchSliceSize;
	pMemObjDesc->memFlags = flags;
	pMemObjDesc->uiElementSize = uiElementSize;
	if ( NULL != format )
	{
		memcpy(&pMemObjDesc->imgFormat, format, sizeof(cl_image_format));
	} else
	{
		memset(&pMemObjDesc->imgFormat, 0, sizeof(cl_image_format));
	}

	memcpy(pMemObjDesc->stDim, dim, dim_count * sizeof(size_t));

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
							void** OUT ptr, size_t* OUT pitch, size_t* OUT minimiumPitch, size_t* OUT uiElementSize)
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

	if(NULL != pitch)
	{
		pitch[0] = pObjDesc->stPitch[0];
		pitch[1] = pObjDesc->stPitch[1];
	}

	if(NULL != minimiumPitch && dim_count > 1)
	{
		minimiumPitch[0] = pObjDesc->uiElementSize * pObjDesc->stDim[0];
		minimiumPitch[1] = minimiumPitch[0] * pObjDesc->stDim[1];
	}

	if (NULL != uiElementSize)
	{
		*uiElementSize = pObjDesc->uiElementSize;
	}
	cl_char* lockedPtr = (cl_char*)pObjDesc->pObject;
	if ( NULL != origin )
	{
		if(dim_count > 2)
		{
			lockedPtr += origin[2] * pObjDesc->stPitch[1]; //Z*slice size - slice size is >= image_row_pitch * image_height
		}
		if(dim_count > 1)
		{
			lockedPtr += origin[1] * pObjDesc->stPitch[0]; //y * image width pitch 
		}

		lockedPtr += origin[0] * pObjDesc->uiElementSize; //Origin is in Pixels
	}

	*ptr = lockedPtr;

	return CL_DEV_SUCCESS;
}

cl_int MemoryAllocator::UnLockObject(cl_dev_mem IN pMemObj, void* IN ptr)
{
	InfoLog(m_logDescriptor, m_iLogHandle, L"UnLockObject enter");
	return CL_DEV_SUCCESS;
}

cl_int MemoryAllocator::CreateMappedRegion(cl_dev_cmd_param_map* INOUT pMapParams)
{
	InfoLog(m_logDescriptor, m_iLogHandle, L"CreateMappedRegion enter");

	return 	LockObject(pMapParams->memObj, pMapParams->dim_count, pMapParams->origin, &(pMapParams->ptr), pMapParams->pitch, NULL, NULL);
}

cl_int MemoryAllocator::ReleaseMappedRegion( cl_dev_cmd_param_map* IN pMapParams )
{
	InfoLog(m_logDescriptor, m_iLogHandle, L"ReleaseMappedRegion enter");
	return UnLockObject(pMapParams->memObj, pMapParams->ptr);
}

