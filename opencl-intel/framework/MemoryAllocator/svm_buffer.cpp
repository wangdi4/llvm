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

#include "svm_buffer.h"
#include "cl_shared_ptr.hpp"

using namespace Intel::OpenCL::Framework;

// SharedPointerArg methods:

cl_err_code SharedPointerArg::Initialize(cl_mem_flags clMemFlags,
                                         const cl_image_format* pclImageFormat,
                                         unsigned int dim_count,
                                         const size_t* dimension,
                                         const size_t* pitches,
                                         void* pHostPtr,
                                         cl_rt_memobj_creation_flags creation_flags,
                                         size_t force_alignment)
{
    ASSERT_RET_VAL(false, "this method should never be called", CL_INVALID_OPERATION);
}

cl_err_code SharedPointerArg::UpdateHostPtr(cl_mem_flags clMemFlags, void* pHostPtr)
{
    ASSERT_RET_VAL(false, "this method should never be called", CL_INVALID_OPERATION);
}

cl_err_code SharedPointerArg::ReadData(void* pOutData, const size_t* pszOrigin, const size_t* pszRegion, size_t szRowPitch, size_t szSlicePitch)
{
    ASSERT_RET_VAL(false, "this method should never be called", CL_INVALID_OPERATION);
}

cl_err_code SharedPointerArg::WriteData(const void* pOutData,    const size_t* pszOrigin, const size_t* pszRegion, size_t szRowPitch, size_t szSlicePitch)
{
    ASSERT_RET_VAL(false, "this method should never be called", CL_INVALID_OPERATION);
}

bool SharedPointerArg::IsSynchDataWithHostRequired(cl_dev_cmd_param_map* IN pMapInfo, void* IN pHostMapDataPtr) const
{
    ASSERT_RET_VAL(false, "this method should never be called", false);
}

cl_err_code SharedPointerArg::SynchDataToHost(cl_dev_cmd_param_map* IN pMapInfo, void* IN pHostMapDataPtr)
{
    ASSERT_RET_VAL(false, "this method should never be called", CL_INVALID_OPERATION);
}

cl_err_code SharedPointerArg::SynchDataFromHost(cl_dev_cmd_param_map* IN pMapInfo, void* IN pHostMapDataPtr)
{
    ASSERT_RET_VAL(false, "this method should never be called", CL_INVALID_OPERATION);
}

cl_err_code SharedPointerArg::CreateSubBuffer(cl_mem_flags clFlags, cl_buffer_create_type buffer_create_type,    const void* buffer_create_info, SharedPtr<MemoryObject>* ppBuffer)
{
    ASSERT_RET_VAL(false, "this method should never be called", CL_INVALID_OPERATION);
}

bool SharedPointerArg::IsSupportedByDevice(const SharedPtr<FissionableDevice>& pDevice)
{
    ASSERT_RET_VAL(false, "this method should never be called", false);
}

cl_err_code    SharedPointerArg::MemObjCreateDevMappedRegion(const SharedPtr<FissionableDevice>&, cl_dev_cmd_param_map* cmd_param_map, void** pHostMapDataPtr)
{
    ASSERT_RET_VAL(false, "this method should never be called", CL_INVALID_OPERATION);
}

cl_err_code    SharedPointerArg::MemObjReleaseDevMappedRegion(const SharedPtr<FissionableDevice>&, cl_dev_cmd_param_map*    cmd_param_map, void* pHostMapDataPtr, bool force_unmap)
{
    ASSERT_RET_VAL(false, "this method should never be called", CL_INVALID_OPERATION);
}

// BufferPointerArg methods:

cl_err_code BufferPointerArg::LockOnDevice(IN const SharedPtr<FissionableDevice>& dev, IN MemObjUsage usage, OUT MemObjUsage* pOutActuallyUsage, OUT SharedPtr<OclEvent>& pOutEvent)
{
    return m_pBuf->LockOnDevice(dev, usage, pOutActuallyUsage, pOutEvent);
}

cl_err_code BufferPointerArg::UnLockOnDevice(IN const SharedPtr<FissionableDevice>& dev, IN MemObjUsage usage)
{
    return m_pBuf->UnLockOnDevice(dev, usage);
}

cl_err_code BufferPointerArg::GetDimensionSizes(size_t* pszRegion) const
{
    return m_pBuf->GetDimensionSizes(pszRegion);
}

size_t BufferPointerArg::GetRowPitchSize() const
{
    return m_pBuf->GetRowPitchSize();
}

size_t BufferPointerArg::GetSlicePitchSize() const
{
    return m_pBuf->GetSlicePitchSize();
}

size_t BufferPointerArg::GetPixelSize() const
{
    return m_pBuf->GetPixelSize();
}

void BufferPointerArg::GetLayout(OUT size_t* dimensions, OUT size_t* rowPitch, OUT size_t* slicePitch) const
{
    m_pBuf->GetLayout(dimensions, rowPitch, slicePitch);
}

cl_err_code BufferPointerArg::CheckBounds(const size_t* pszOrigin, const size_t* pszRegion) const
{
    return m_pBuf->CheckBounds(pszOrigin, pszRegion);
}

cl_err_code BufferPointerArg::CheckBoundsRect(const size_t* pszOrigin, const size_t* pszRegion, size_t szRowPitch, size_t szSlicePitch) const
{
    return m_pBuf->CheckBoundsRect(pszOrigin, pszRegion, szRowPitch, szSlicePitch);
}

void* BufferPointerArg::GetBackingStoreData(const size_t* pszOrigin) const
{
    return (char*)m_pBuf->GetAddr() + m_szOffset;
}

cl_err_code BufferPointerArg::CreateDeviceResource(const SharedPtr<FissionableDevice>& pDevice)
{
    return m_pBuf->CreateDeviceResource(pDevice);
}

cl_err_code BufferPointerArg::GetDeviceDescriptor(const SharedPtr<FissionableDevice>& IN pDevice, IOCLDevMemoryObject* OUT* ppDevObject, SharedPtr<OclEvent> OUT* ppEvent)
{
    IOCLDevMemoryObject* pBufDevObj;
    const cl_err_code err = m_pBuf->GetDeviceDescriptor(pDevice, &pBufDevObj, ppEvent);    // what about ppEvent?
    if (CL_FAILED(err))
    {
        return err;
    }
    *ppDevObject = new PointerArgDevMemoryObject(this, pBufDevObj, m_szOffset);
    return CL_SUCCESS;
}

cl_err_code BufferPointerArg::UpdateDeviceDescriptor(const SharedPtr<FissionableDevice>& IN pDevice, IOCLDevMemoryObject* OUT* ppDevObject)
{
    return m_pBuf->UpdateDeviceDescriptor(pDevice, ppDevObject);
}

// SharedPointerArg::PointerArgDevMemoryObject methods:

SharedPointerArg::PointerArgDevMemoryObject::PointerArgDevMemoryObject(const SharedPtr<SharedPointerArg>& pPtrArg, IOCLDevMemoryObject* pBufDevMemObj, size_t szOffset) :
    m_pBufDevMemObj(pBufDevMemObj)
{
    m_objDecr.dimensions.buffer_size = 0;    // unknown
    m_objDecr.dim_count = 1;
    m_objDecr.memObjType = CL_MEM_OBJECT_BUFFER;
    m_objDecr.pData = pPtrArg->GetBackingStoreData();
}

cl_dev_err_code SharedPointerArg::PointerArgDevMemoryObject::clDevMemObjCreateMappedRegion(cl_dev_cmd_param_map* pMapParams)
{
    ASSERT_RET_VAL(false, "this method should never be called", CL_DEV_INVALID_OPERATION);
}

cl_dev_err_code SharedPointerArg::PointerArgDevMemoryObject::clDevMemObjUnmapAndReleaseMappedRegion(cl_dev_cmd_param_map* pMapParams)
{
    ASSERT_RET_VAL(false, "this method should never be called", CL_DEV_INVALID_OPERATION);
}

cl_dev_err_code SharedPointerArg::PointerArgDevMemoryObject::clDevMemObjReleaseMappedRegion(cl_dev_cmd_param_map* pMapParams)
{
    ASSERT_RET_VAL(false, "this method should never be called", CL_DEV_INVALID_OPERATION);
}

cl_dev_err_code SharedPointerArg::PointerArgDevMemoryObject::clDevMemObjGetDescriptor(cl_device_type dev_type, cl_dev_subdevice_id node_id, cl_dev_memobj_handle* handle)
{
    *handle = &m_objDecr;
    return CL_DEV_SUCCESS;
}

cl_dev_err_code SharedPointerArg::PointerArgDevMemoryObject::clDevMemObjCreateSubObject(cl_mem_flags mem_flags, const size_t IN* origin, const size_t IN* size, IOCLDevRTMemObjectService IN* pBSService,
    IOCLDevMemoryObject* OUT* ppSubObject)
{
    ASSERT_RET_VAL(false, "this method should never be called", CL_DEV_INVALID_OPERATION);
}

cl_dev_err_code SharedPointerArg::PointerArgDevMemoryObject::clDevMemObjUpdateBackingStore(void* operation_handle, cl_dev_bs_update_state* pUpdateState)
{
    if (nullptr != m_pBufDevMemObj)
    {
        return m_pBufDevMemObj->clDevMemObjUpdateBackingStore(operation_handle, pUpdateState);
    }
    return CL_DEV_SUCCESS;
}
    
cl_dev_err_code SharedPointerArg::PointerArgDevMemoryObject::clDevMemObjUpdateFromBackingStore(void* operation_handle, cl_dev_bs_update_state* pUpdateState)
{
    if (nullptr != m_pBufDevMemObj)
    {
        return m_pBufDevMemObj->clDevMemObjUpdateFromBackingStore(operation_handle, pUpdateState);
    }
    return CL_DEV_SUCCESS;
}

cl_dev_err_code SharedPointerArg::PointerArgDevMemoryObject::clDevMemObjInvalidateData()
{
    if (nullptr != m_pBufDevMemObj)
    {
        return m_pBufDevMemObj->clDevMemObjInvalidateData();
    }
    return CL_DEV_SUCCESS;
}

cl_dev_err_code SharedPointerArg::PointerArgDevMemoryObject::clDevMemObjRelease()
{
    if (nullptr != m_pBufDevMemObj)
    {
        return m_pBufDevMemObj->clDevMemObjRelease();
    }
    return CL_DEV_SUCCESS;
}
