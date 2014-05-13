// Copyright (c) 2014 Intel Corporation
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
//  isp_memory_allocator.h
//  Implementation of the Class ISPMemoryAllocator
///////////////////////////////////////////////////////////

#pragma once

#include <cl_device_api.h>
#include <cl_types.h>


#include "res/Ilibshim.h"


namespace Intel { namespace OpenCL { namespace ISPDevice {

class ISPMemoryAllocator
{
public:
    ISPMemoryAllocator(cl_int devId, IOCLDevLogDescriptor *logDesc, cl_ulong maxAllocSize);
    virtual ~ISPMemoryAllocator();

    cl_dev_err_code Init();

    cl_dev_err_code GetAllocProperties(cl_mem_object_type IN memObjType, cl_dev_alloc_prop* OUT pAllocProp);
    cl_dev_err_code CreateMemoryObject(cl_dev_subdevice_id IN node_id, cl_mem_flags IN flags, const cl_image_format* IN format,
                                       size_t  IN dim_count, const size_t* IN dim_size, IOCLDevRTMemObjectService* IN pRTService,
                                       IOCLDevMemoryObject* OUT *memObj);

protected:

    cl_int                  m_iDevId;
    IOCLDevLogDescriptor*   m_pLogDescriptor;
    cl_int                  m_iLogHandle;
    cl_ulong                m_maxAllocSize;
    //CameraShim*             m_pCameraShim;

};


class ISPMemoryObject : public IOCLDevMemoryObject
{
public:
    ISPMemoryObject(cl_int iLogHandle, IOCLDevLogDescriptor* pLogDescriptor,
        cl_mem_flags memFlags, IOCLDevRTMemObjectService* pRTMemObjService);

    virtual ~ISPMemoryObject();

    cl_dev_err_code Init(cl_uint dimCount, const size_t* dim, const cl_image_format* pImgFormat);

    // IOCLDevMemoryObject interfaces
    virtual cl_dev_err_code clDevMemObjCreateMappedRegion(cl_dev_cmd_param_map* pMapParams);
    virtual cl_dev_err_code clDevMemObjUnmapAndReleaseMappedRegion(cl_dev_cmd_param_map* pMapParams);
    virtual cl_dev_err_code clDevMemObjReleaseMappedRegion(cl_dev_cmd_param_map* pMapParams);
    virtual cl_dev_err_code clDevMemObjGetDescriptor(cl_device_type dev_type, cl_dev_subdevice_id node_id, cl_dev_memobj_handle *handle);
    virtual cl_dev_err_code clDevMemObjCreateSubObject(cl_mem_flags memFlags, const size_t IN *origin, const size_t IN *size,
                                    IOCLDevRTMemObjectService IN *pRTMemObjService, IOCLDevMemoryObject* OUT *pSubObject);
    virtual cl_dev_err_code clDevMemObjUpdateBackingStore(void* operation_handle, cl_dev_bs_update_state* pUpdateState);
    virtual cl_dev_err_code clDevMemObjUpdateFromBackingStore(void* operation_handle, cl_dev_bs_update_state* pUpdateState);
    virtual cl_dev_err_code clDevMemObjInvalidateData();
    virtual cl_dev_err_code clDevMemObjRelease();


    cl_mem_obj_descriptor GetObjDescriptor() const { return m_objDescriptor; }

protected:
    IOCLDevLogDescriptor*       m_pLogDescriptor;
    cl_int                      m_iLogHandle;

    //CameraShim*                 m_pCameraShim;

    cl_mem_flags                m_memFlags;
    IOCLDevRTMemObjectService*  m_pRTMemObjService;
    IOCLDevBackingStore*        m_pBackingStore;

    cl_mem_obj_descriptor       m_objDescriptor;
};

class ISPMemorySubObject : public ISPMemoryObject
{
public:
    ISPMemorySubObject(cl_int iLogHandle, IOCLDevLogDescriptor* pLogDescriptor,
        cl_mem_flags memFlags, IOCLDevRTMemObjectService* pRTMemObjService);

    cl_dev_err_code Init(ISPMemoryObject* parent, const size_t* origin, const size_t* size);
protected:
    ISPMemoryObject* m_pParent;
};

}}} // namespace Intel { namespace OpenCL { namespace ISPDevice {
