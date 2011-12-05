
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


namespace Intel { namespace OpenCL { namespace CPUDevice {

class MemoryAllocator
{

public:
	MemoryAllocator(cl_int devId, IOCLDevLogDescriptor *pLogDesc, cl_ulong maxAllocSize);
	virtual ~MemoryAllocator();

	//Image Info Function
	cl_dev_err_code GetSupportedImageFormats( cl_mem_flags IN flags, cl_mem_object_type IN imageType,
				cl_uint IN numEntries, cl_image_format* OUT formats, cl_uint* OUT numEntriesRet);
	cl_dev_err_code GetAllocProperties( cl_mem_object_type IN memObjType,	cl_dev_alloc_prop* OUT pAllocProp );
	// Create/Release functions
	cl_dev_err_code	CreateObject( cl_dev_subdevice_id node_id, cl_mem_flags flags, const cl_image_format* format,
							size_t	dim_count, const size_t* dim,
							IOCLDevRTMemObjectService*	pRTMemObjService,
							IOCLDevMemoryObject* *memObj );


	// Utility functions
	static void* CalculateOffsetPointer(void* pBasePtr, cl_uint dim_count, const size_t* origin, const size_t* pitch, size_t elemSize);
protected:
	size_t GetElementSize(const cl_image_format* format);
	cl_int					m_iDevId;
    cl_ulong      m_maxAllocSize;
	IOCLDevLogDescriptor*	m_pLogDescriptor;
	cl_int					m_iLogHandle;
	ClHeap					m_lclHeap;
};

class CPUDevMemoryObject : public IOCLDevMemoryObject
{
public:
	friend class CPUDevMemorySubObject;

	CPUDevMemoryObject(cl_int iLogHandle, IOCLDevLogDescriptor* pLogDescriptor, ClHeap lclHeap,
		cl_dev_subdevice_id nodeId, cl_mem_flags memFlags,
		const cl_image_format* pImgFormat, size_t elemSize,
		size_t dimCount, const size_t* dim,
		IOCLDevRTMemObjectService*	pRTMemObjService);

	~CPUDevMemoryObject();

	cl_dev_err_code Init();

	cl_dev_err_code clDevMemObjCreateMappedRegion( cl_dev_cmd_param_map*	pMapParams );
	cl_dev_err_code clDevMemObjReleaseMappedRegion( cl_dev_cmd_param_map* pMapParams );
	cl_dev_err_code clDevMemObjRelease();
	cl_dev_err_code clDevMemObjGetDescriptor(cl_device_type dev_type, cl_dev_subdevice_id node_id, cl_dev_memobj_handle *handle);

	cl_dev_err_code clDevMemObjCreateSubObject( cl_mem_flags mem_flags,
					const size_t *origin, const size_t *size, IOCLDevMemoryObject** ppSubObject );

protected:
	CPUDevMemoryObject(cl_int iLogHandle, IOCLDevLogDescriptor* pLogDescriptor) :
		m_lclHeap(NULL), m_pLogDescriptor(pLogDescriptor), m_iLogHandle(iLogHandle),
			m_nodeId(NULL), m_memFlags(0),
			m_pRTMemObjService(NULL), m_pBackingStore(NULL), m_pHostPtr(NULL) {}

	ClHeap					m_lclHeap;
	IOCLDevLogDescriptor*	m_pLogDescriptor;
	cl_int					m_iLogHandle;

	// Object Management
	cl_dev_subdevice_id		m_nodeId;

	cl_mem_obj_descriptor		m_objDecr;
	cl_mem_flags				m_memFlags;
	IOCLDevRTMemObjectService*	m_pRTMemObjService;
	IOCLDevBackingStore*		m_pBackingStore;
	void*						m_pHostPtr;			// A pointer provided by the framework
	size_t						m_hostPitch[MAX_WORK_DIM-1];

};

class CPUDevMemorySubObject : public CPUDevMemoryObject
{
public:
	CPUDevMemorySubObject(cl_int iLogHandle, IOCLDevLogDescriptor* pLogDescriptor, CPUDevMemoryObject* pParent);

	cl_dev_err_code Init(cl_mem_flags mem_flags, const size_t *origin, const size_t *size);

protected:
	CPUDevMemoryObject* m_pParent;
};

class CPUDevBackingStore : public IOCLDevBackingStore
{
public:
	CPUDevBackingStore(void* ptr, const size_t pitch[]);
	void* GetRawData() const {return m_ptr;}
	cl_dev_bs_description GetRawDataDecription() const {return CL_DEV_BS_RT_ALLOCATED;}
    size_t GetRawDataSize() const {return 0;}
	bool IsDataValid() const { return true;}
	size_t GetDimCount() const {return 0;}
	const size_t* GetDimentions() const {return NULL;}
	const size_t* GetPitch() const {return m_pitch;}

    size_t GetRawDataOffset(const size_t*) const { return 0; }
    const cl_image_format& GetFormat() const { return *((cl_image_format*)0); }
    size_t GetElementSize() const { return 0; }

	int AddPendency();
	int RemovePendency();

protected:
	virtual ~CPUDevBackingStore() {};

	void*			m_ptr;
	size_t			m_pitch[MAX_WORK_DIM-1];
	AtomicCounter	m_refCount;
};

}}}
