
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
//  MemoryAllocator.h
//  Implementation of the Class MemoryAllocator
//  Created on:      16-Dec-2008 4:54:53 PM
//  Original author: efiksman
///////////////////////////////////////////////////////////

#pragma once

#include "cl_device_api.h"
#include "handle_allocator.h"
#include "cl_types.h"
#include <map>

using namespace Intel::OpenCL::Utils;

namespace Intel { namespace OpenCL { namespace CPUDevice {

struct SMemCpyParams
{
	cl_uint			uiDimCount;
	cl_char*		pSrc;
	size_t			vSrcPitch[MAX_WORK_DIM-1];
	cl_char*		pDst;
	size_t			vDstPitch[MAX_WORK_DIM-1];
	size_t			vRegion[MAX_WORK_DIM];
};

class MemoryAllocator
{

public:
	MemoryAllocator(cl_int devId, cl_dev_log_descriptor *logDesc);
	virtual ~MemoryAllocator();

	//Image Info Function
	cl_int GetSupportedImageFormats( cl_dev_mem_flags IN flags, cl_dev_mem_object_type IN imageType,
				cl_uint IN numEntries, cl_image_format* OUT formats, cl_uint* OUT numEntriesRet);
	// Create/Release functions
	cl_int	CreateObject( cl_dev_mem_flags IN flags, const cl_image_format* IN format,
							cl_uint	IN dim_count, const size_t* dim, void*	buffer_ptr, const size_t* pitch,
							cl_dev_host_ptr_flags host_flags,
							cl_dev_mem* OUT memObj );
	cl_int	ReleaseObject( cl_dev_mem IN memObj );

	// Checks that given object is valid object and belongs to memory allocator
	cl_int	ValidateObject( cl_dev_mem IN memObj );

	// Lock/Unlock functions
	// This function retrievs a pointer to data inside the object
	cl_int	LockObject(cl_dev_mem IN pMemObj, cl_uint IN dim_count, const size_t* origin,
							void** OUT ptr, size_t* OUT pitch, size_t* OUT uiElementSize);
	// This function retrives a pointer to object descriptor
	cl_int	LockObject(cl_dev_mem IN pMemObj, cl_mem_obj_descriptor* OUT *pMemObjDesc);
	// This function unlocks both locks
	cl_int	UnLockObject(cl_dev_mem IN memObj, void* IN ptr);

	// Mapped region functions
	cl_int	CreateMappedRegion( cl_dev_cmd_param_map* INOUT pMapParams );
	cl_int	ReleaseMappedRegion( cl_dev_cmd_param_map* IN pMapParams );

	// Utility functions
	static void CopyMemoryBuffer(SMemCpyParams* pCopyCmd);

protected:
	struct SMemObjectDescriptor
	{
		cl_dev_mem					myHandle;
		cl_mem_obj_descriptor		objDecr;
		cl_dev_mem_flags			memFlags;
		cl_dev_host_ptr_flags		clHostPtrFlags;
		void*						pHostPtr;						// A pointer provided by framework
		bool						bObjLocked;
	};

	void*	AllocateMem(cl_uint	dim_count, const size_t* dim, size_t* pitch);
	size_t  ElementSize(const cl_image_format* format);

	cl_int CalculateOffsetPointer(const SMemObjectDescriptor* pObjDesc, void* pBasePtr,
								   cl_uint dim_count, const size_t* IN origin,
									void** OUT ptr, size_t* OUT pitch, size_t* OUT uiElementSize);

	cl_int					m_iDevId;
	cl_dev_log_descriptor   m_logDescriptor;
	cl_int					m_iLogHandle;

	// Object Management
	typedef	std::map<unsigned int, SMemObjectDescriptor*>	TMemObjectsMap;

	TMemObjectsMap					m_mapObjects;
	OclMutex						m_muObjectMap;
	HandleAllocator<unsigned int>	m_objHandles;

};

}}}