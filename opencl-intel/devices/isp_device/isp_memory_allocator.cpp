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
//  isp_memory_allocator.cpp
//  Implementation of the Class ISPMemoryAllocator
///////////////////////////////////////////////////////////

#include "stdafx.h"

#include "isp_memory_allocator.h"
#include "isp_logger.h"
#include "isp_utils.h"

#include <cl_sys_defines.h>

using namespace Intel::OpenCL::ISPDevice;

ISPMemoryAllocator::ISPMemoryAllocator(cl_int devId, IOCLDevLogDescriptor *logDesc, cl_ulong maxAllocSize) :
    m_iDevId(devId), m_pLogDescriptor(logDesc), m_iLogHandle(0), m_maxAllocSize(maxAllocSize)
{
    if (nullptr != logDesc)
    {
        cl_int ret = m_pLogDescriptor->clLogCreateClient(m_iDevId, TEXT("ISP Device: Memory Allocator"), &m_iLogHandle);
        if (CL_DEV_SUCCESS != ret)
        {
            m_iLogHandle = 0;
        }
    }

    IspInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("ISPDevice: Memory Allocator - Constructed"));
}

ISPMemoryAllocator::~ISPMemoryAllocator()
{
    IspInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("ISPDevice: Memory Allocator - Destructed"));

    if (0 != m_iLogHandle)
    {
        m_pLogDescriptor->clLogReleaseClient(m_iLogHandle);
    }
}

cl_dev_err_code ISPMemoryAllocator::Init()
{
    return CL_DEV_SUCCESS;
}

/****************************************************************************************************************
 GetAllocProperties
    Description
        This function return allocator properties per memory object
    Input
        memObjType              Memory object type (buffer/image1D/2D etc.)
    Output
        pAllocProp              The returned allocator properties
    Returns
        CL_DEV_SUCCESS          The function is executed successfully.
********************************************************************************************************************/
cl_dev_err_code ISPMemoryAllocator::GetAllocProperties(cl_mem_object_type IN memObjType, cl_dev_alloc_prop* OUT pAllocProp)
{
    IspInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("GetAllocProperties enter"));

    assert(nullptr != pAllocProp && "Framework should not have called this with null parameter");

    pAllocProp->bufferSharingGroupId = CL_DEV_ISP_BUFFER_SHARING_GROUP_ID;
    pAllocProp->imageSharingGroupId  = CL_DEV_ISP_IMAGE_SHARING_GROUP_ID;
    pAllocProp->mustAllocRawMemory   = true;
    pAllocProp->usedByDMA            = true;   // TODO: Set to false?
    pAllocProp->alignment            = PAGE_4K_SIZE;
    pAllocProp->preferred_alignment  = PAGE_4K_SIZE;
    pAllocProp->maxBufferSize        = m_maxAllocSize;
    pAllocProp->imagesSupported      = false;   // TODO: image support
    pAllocProp->DXSharing            = false;
    pAllocProp->GLSharing            = false;

    return CL_DEV_SUCCESS;
}

/****************************************************************************************************************
 CreateMemoryObject
    Description
        Creates a device memory object
    Input
        node_id                     Subdevice ID (not used for now)
        flags                       Memory of flags of the requested memory object
        format                      Image format (NULL if not image)
        dim_count                   Dimensions count of the memory object (number of dimensions in dim parameter)
        dim                         Dimensions of the requested memory object
        pRTMemObjService            Runtime callbacks to be used in the memory object
    Output
        memObj                      The returned memory object
    Returns
        CL_DEV_SUCCESS              The function is executed successfully.
        CL_DEV_INVALID_VALUE        If dimensions or dimensions count are invalid.
        CL_DEV_OBJECT_ALLOC_FAIL    If there was a failure to allocate the memory object.
        CL_DEV_ERROR_FAIL           If there was a failure getting the backing store,
                                     or cannot create the mem obj with specified flags.
********************************************************************************************************************/
cl_dev_err_code ISPMemoryAllocator::CreateMemoryObject(cl_dev_subdevice_id IN node_id, cl_mem_flags IN flags, const cl_image_format* IN format,
                     size_t IN dim_count, const size_t* IN dim,
                     IOCLDevRTMemObjectService* IN pRTMemObjService,
                     IOCLDevMemoryObject** OUT memObj)
{
    IspInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("CreateMemoryObject enter"));

    assert(nullptr != dim && "Framework should not have called this with null parameters");
    assert(nullptr != pRTMemObjService && "Framework should not have called this with null parameters");
    assert(nullptr != memObj && "Framework should not have called this with null parameters");
    assert(0 != dim_count && "Invalid dimensions count");

    assert(0 == node_id && "Subdevices feature is currently not supported");

    if (dim_count > MAX_WORK_DIM)
    {
        IspErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("Can't create memory object with %lu dimensions, currently max dimensions is %lu"), dim_count, MAX_WORK_DIM);
        return CL_DEV_INVALID_VALUE;
    }

    // Allocate memory for memory object
    // TODO: image support
    ISPMemoryObject* pIspMemObj = new ISPMemoryObject(m_iLogHandle, m_pLogDescriptor, flags, pRTMemObjService);
    if (nullptr == pIspMemObj)
    {
        IspErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), "Memory Object allocation failed");
        return CL_DEV_OBJECT_ALLOC_FAIL;
    }
    cl_dev_err_code retCode = pIspMemObj->Init((cl_uint)dim_count, dim, format);
    if (CL_DEV_FAILED(retCode))
    {
        IspErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("Memory Object descriptor initializing failed, retCode=%x"), retCode);
        delete pIspMemObj;
        return retCode;
    }

    *memObj = pIspMemObj;

    IspInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), "CreateMemoryObject exit");

    return CL_DEV_SUCCESS;
}


//-----------------------------------------------------------------------------------------------------
// ISPMemoryObject
//-----------------------------------------------------------------------------------------------------
ISPMemoryObject::ISPMemoryObject(cl_int iLogHandle, IOCLDevLogDescriptor* pLogDescriptor,
    cl_mem_flags memFlags, IOCLDevRTMemObjectService* pRTMemObjService) :
m_iLogHandle(iLogHandle), m_pLogDescriptor(pLogDescriptor),
m_memFlags(memFlags), m_pRTMemObjService(pRTMemObjService), m_pBackingStore(nullptr)
{
    memset(&m_objDescriptor, 0, sizeof(m_objDescriptor));
}

ISPMemoryObject::~ISPMemoryObject()
{
    if (nullptr != m_pBackingStore)
    {
        m_pBackingStore->RemovePendency();
    }
}

cl_dev_err_code ISPMemoryObject::Init(cl_uint dimCount, const size_t* dim, const cl_image_format* pImgFormat)
{
    IspInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("Initializing memory object enter"));

    if (MAX_WORK_DIM < dimCount || nullptr == dim)
    {
        IspErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("Invalid parameters"));
        return CL_DEV_INVALID_VALUE;
    }

    m_objDescriptor.dim_count = (cl_uint)dimCount;
    if (1 == dimCount)
    {
        m_objDescriptor.dimensions.buffer_size = dim[0];
    }
    else
    {
        for (unsigned int i = 0; i < dimCount; ++i)
        {
            m_objDescriptor.dimensions.dim[i] = (unsigned int) dim[i];
        }
    }
    if (nullptr != pImgFormat)
    {
        m_objDescriptor.format = *pImgFormat;
    }

    //assert(NULL != m_pCameraShim && "ISP Memory object cannot be created without Camera Service");
    assert(nullptr != m_pRTMemObjService && "ISP Memory object cannot be created without RT memory object service");

    // Get only if there is available backing store.
    cl_dev_err_code err = m_pRTMemObjService->GetBackingStore(CL_DEV_BS_GET_ALWAYS, &m_pBackingStore);
    if (CL_DEV_FAILED(err) || nullptr == m_pBackingStore)
    {
        IspErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("Getting the backing store from RT has failed"));
        return CL_DEV_ERROR_FAIL;
    }
    m_pBackingStore->AddPendency();

    MEMCPY_S( m_objDescriptor.pitch, sizeof(m_objDescriptor.pitch), m_pBackingStore->GetPitch(), sizeof(m_objDescriptor.pitch) );

    m_objDescriptor.pData = m_pBackingStore->GetRawData();
    m_objDescriptor.uiElementSize = m_pBackingStore->GetElementSize();
    // TODO: image support
    m_objDescriptor.imageAuxData = nullptr;
    m_objDescriptor.memObjType = m_pRTMemObjService->GetMemObjectType();

    return CL_DEV_SUCCESS;
}

// IOCLDevMemoryObject interfaces
cl_dev_err_code ISPMemoryObject::clDevMemObjCreateMappedRegion(cl_dev_cmd_param_map* pMapParams)
{
    IspInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("clDevMemObjCreateMappedRegion enter"));

    // Since memory is already shared with host, we return pointer inside the memory object itself

    pMapParams->memObj = this;

    // Determine which pointer to use
    void* pMapPtr = m_objDescriptor.pData;
    size_t* pitch = m_objDescriptor.pitch;
    assert(pMapParams->dim_count == m_objDescriptor.dim_count);
    pMapParams->ptr = CalculateOffsetPointer(pMapPtr, m_objDescriptor.dim_count, pMapParams->origin, pitch, m_objDescriptor.uiElementSize);
    MEMCPY_S(pMapParams->pitch, sizeof(size_t)*(MAX_WORK_DIM-1), pitch, sizeof(size_t)*(MAX_WORK_DIM-1));

    return CL_DEV_SUCCESS;
}

cl_dev_err_code ISPMemoryObject::clDevMemObjUnmapAndReleaseMappedRegion(cl_dev_cmd_param_map* pMapParams)
{
    IspInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("clDevMemObjUnmapAndReleaseMappedRegion enter"));
    return clDevMemObjReleaseMappedRegion(pMapParams);
}

cl_dev_err_code ISPMemoryObject::clDevMemObjReleaseMappedRegion(cl_dev_cmd_param_map* pMapParams)
{
    IspInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("clDevMemObjReleaseMappedRegion enter"));
    // nothing to do
    return CL_DEV_SUCCESS;
}

cl_dev_err_code ISPMemoryObject::clDevMemObjGetDescriptor(cl_device_type dev_type, cl_dev_subdevice_id node_id, cl_dev_memobj_handle *handle)
{
    IspInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("clDevMemObjGetDescriptor enter"));
    assert(nullptr != handle && "Invalid parameter for returned descriptor");
    assert(0 == node_id && "Subdevices feature is currently not supported");

    *handle = (void*) (&m_objDescriptor);
    return CL_DEV_SUCCESS;
}

cl_dev_err_code ISPMemoryObject::clDevMemObjCreateSubObject(cl_mem_flags memFlags, const size_t IN *origin, const size_t IN *size,
                                    IOCLDevRTMemObjectService IN *pRTMemObjService, IOCLDevMemoryObject* OUT *pSubObject)
{
    IspInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("clDevMemObjCreateSubObject enter"));

    ISPMemorySubObject* pIspSubObject = new ISPMemorySubObject(m_iLogHandle, m_pLogDescriptor, memFlags, pRTMemObjService);
    if (nullptr == pIspSubObject)
    {
        IspErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), "Memory Object allocation failed");
        return CL_DEV_OBJECT_ALLOC_FAIL;
    }

    cl_dev_err_code retCode = pIspSubObject->Init(this, origin, size);
    if (CL_DEV_FAILED(retCode))
    {
        IspErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("Memory Object descriptor initializing failed, retCode=%x"), retCode);
        delete pIspSubObject;
        return retCode;
    }

    assert (nullptr != pSubObject && "Invalid parameter for returned sub-buffer");
    *pSubObject = pIspSubObject;

    return CL_DEV_SUCCESS;
}

cl_dev_err_code ISPMemoryObject::clDevMemObjUpdateBackingStore(void* operation_handle, cl_dev_bs_update_state* pUpdateState)
{
    IspInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("clDevMemObjUpdateBackingStore enter"));
    assert( nullptr != pUpdateState );
    // Update from Device to BackingStore
    // But, since device uses same memory as backing store no need to copy
    *pUpdateState = CL_DEV_BS_UPDATE_COMPLETED;
    return CL_DEV_SUCCESS;
}

cl_dev_err_code ISPMemoryObject::clDevMemObjUpdateFromBackingStore(void* operation_handle, cl_dev_bs_update_state* pUpdateState)
{
    IspInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("clDevMemObjUpdateFromBackingStore enter"));
    assert( nullptr != pUpdateState );
    // Update from BackingStore to Device
    // But, since device uses same memory as backing store no need to copy
    *pUpdateState = CL_DEV_BS_UPDATE_COMPLETED;
    return CL_DEV_SUCCESS;
}

cl_dev_err_code ISPMemoryObject::clDevMemObjInvalidateData()
{
    IspInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("clDevMemObjInvalidateData enter"));
    // Nothing to do, since device uses same memory as backing store
    return CL_DEV_SUCCESS;
}

cl_dev_err_code ISPMemoryObject::clDevMemObjRelease()
{
    IspInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("clDevMemObjRelease enter"));

    delete this;
    return CL_DEV_SUCCESS;
}


//-----------------------------------------------------------------------------------------------------
// ISPMemorySubObject
//-----------------------------------------------------------------------------------------------------
ISPMemorySubObject::ISPMemorySubObject(cl_int iLogHandle, IOCLDevLogDescriptor* pLogDescriptor,
        cl_mem_flags memFlags, IOCLDevRTMemObjectService* pRTMemObjService) :
ISPMemoryObject(iLogHandle, pLogDescriptor, memFlags, pRTMemObjService)
{
}

cl_dev_err_code ISPMemorySubObject::Init(ISPMemoryObject* parent, const size_t* origin, const size_t* size)
{
    IspInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("Initializing sub memory object enter"));

    if (nullptr == parent || nullptr == origin || nullptr == size)
    {
        IspErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("Invalid parameters"));
        return CL_DEV_INVALID_VALUE;
    }

    m_pParent = parent;
    // Copy obj descriptor from parent
    m_objDescriptor = m_pParent->GetObjDescriptor();

    // Update dimensions
    m_objDescriptor.pData = CalculateOffsetPointer(m_objDescriptor.pData, m_objDescriptor.dim_count, origin, m_objDescriptor.pitch, m_objDescriptor.uiElementSize);
    if (1 == m_objDescriptor.dim_count)
    {
        m_objDescriptor.dimensions.buffer_size = size[0];
    }
    else
    {
        for (unsigned int i = 0; i < m_objDescriptor.dim_count; ++i)
        {
            m_objDescriptor.dimensions.dim[i] = (unsigned int) size[i];
        }
    }

    // Get backing store
    assert(nullptr != m_pRTMemObjService && "ISP Memory object cannot be created without RT memory object service");
    cl_dev_err_code err = m_pRTMemObjService->GetBackingStore(CL_DEV_BS_GET_ALWAYS, &m_pBackingStore);
    if (CL_DEV_FAILED(err) || nullptr == m_pBackingStore)
    {
        IspErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("Getting the backing store from RT has failed"));
        return CL_DEV_ERROR_FAIL;
    }
    m_pBackingStore->AddPendency();

    return CL_DEV_SUCCESS;
}
