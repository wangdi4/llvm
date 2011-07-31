
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

#include <cl_device_api.h>
#include <cl_types.h>
#include <cl_heap.h>
#include <cl_synch_objects.h>
#include <map>

using namespace Intel::OpenCL::Utils;


namespace Intel { namespace OpenCL { namespace MICDevice {

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
	MemoryAllocator(cl_int devId, IOCLDevLogDescriptor *pLogDesc, unsigned long long maxAllocSize);
	virtual ~MemoryAllocator();

	//Image Info Function
	cl_dev_err_code GetSupportedImageFormats( cl_mem_flags IN flags, cl_mem_object_type IN imageType,
				cl_uint IN numEntries, cl_image_format* OUT formats, cl_uint* OUT numEntriesRet);
	cl_dev_err_code GetAllocProperties( cl_mem_object_type IN memObjType,	cl_dev_alloc_prop* OUT pAllocProp );
	// Create/Release functions
	cl_dev_err_code	CreateObject( cl_dev_subdevice_id node_id, cl_mem_flags flags, const cl_image_format* format,
							size_t	dim_count, const size_t* dim, void*	buffer_ptr, const size_t* pitch,
							cl_dev_host_ptr_flags host_flags,
							IOCLDevMemoryObject* *memObj );


	// Utility functions
	static void CopyMemoryBuffer(SMemCpyParams* pCopyCmd);
	static void* CalculateOffsetPointer(void* pBasePtr, cl_uint dim_count, const size_t* origin, const size_t* pitch, size_t elemSize);
protected:
	size_t GetElementSize(const cl_image_format* format);
	cl_int					m_iDevId;
	IOCLDevLogDescriptor*	m_pLogDescriptor;
	cl_int					m_iLogHandle;
	ClHeap					m_lclHeap;
};

class MICDevMemoryObject : public IOCLDevMemoryObject
{
public:
	friend class MICDevMemorySubObject;

	MICDevMemoryObject(cl_int iLogHandle, IOCLDevLogDescriptor* pLogDescriptor, ClHeap lclHeap,
		cl_dev_subdevice_id nodeId, cl_mem_flags memFlags,
		const cl_image_format* pImgFormat, size_t elemSize,
		size_t dimCount, const size_t* dim,
		void* pBuffer, const size_t* pPitches,
		cl_dev_host_ptr_flags hostFlags);

	MICDevMemoryObject(cl_int iLogHandle, IOCLDevLogDescriptor* pLogDescriptor) :
		m_lclHeap(NULL), m_pLogDescriptor(pLogDescriptor), m_iLogHandle(iLogHandle),
			m_nodeId(NULL), m_memFlags(0), m_hostPtrFlags(CL_DEV_HOST_PTR_NONE), m_pHostPtr(NULL) {}

	cl_dev_err_code Init();

	cl_dev_err_code clDevMemObjCreateMappedRegion( cl_dev_cmd_param_map*	pMapParams );
	cl_dev_err_code clDevMemObjReleaseMappedRegion( cl_dev_cmd_param_map* pMapParams );
	cl_dev_err_code clDevMemObjRelease();
	cl_dev_err_code clDevMemObjGetDescriptor(cl_device_type dev_type, cl_dev_subdevice_id node_id, cl_dev_memobj_handle *handle);

	cl_dev_err_code clDevMemObjCreateSubObject( cl_mem_flags mem_flags,
					const size_t *origin, const size_t *size, IOCLDevMemoryObject** ppSubObject );

protected:
	ClHeap					m_lclHeap;
	IOCLDevLogDescriptor*	m_pLogDescriptor;
	cl_int					m_iLogHandle;

	// Object Management
	cl_dev_subdevice_id		m_nodeId;

	cl_mem_obj_descriptor	m_objDecr;
	cl_mem_flags			m_memFlags;
	cl_dev_host_ptr_flags	m_hostPtrFlags;
	void*					m_pHostPtr;			// A pointer provided by the framework
	size_t					m_hostPitch[MAX_WORK_DIM-1];

};

class MICDevMemorySubObject : public MICDevMemoryObject
{
public:
	MICDevMemorySubObject(cl_int iLogHandle, IOCLDevLogDescriptor* pLogDescriptor, MICDevMemoryObject* pParent);

	cl_dev_err_code Init(cl_mem_flags mem_flags, const size_t *origin, const size_t *size);

protected:
	MICDevMemoryObject* m_pParent;
};
}}}
