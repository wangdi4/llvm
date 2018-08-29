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

// SVMPointerArg methods:

cl_err_code SVMPointerArg::Initialize(cl_mem_flags    clMemFlags,    const cl_image_format* pclImageFormat, unsigned int    dim_count, const size_t* dimension,    const size_t* pitches,
    void* pHostPtr,    cl_rt_memobj_creation_flags    creation_flags)
{
    ASSERT_RET_VAL(false, "this method should never be called", CL_INVALID_OPERATION);
}

cl_err_code SVMPointerArg::UpdateHostPtr(cl_mem_flags clMemFlags, void* pHostPtr)
{
    ASSERT_RET_VAL(false, "this method should never be called", CL_INVALID_OPERATION);
}

cl_err_code SVMPointerArg::ReadData(void* pOutData, const size_t* pszOrigin, const size_t* pszRegion, size_t szRowPitch, size_t szSlicePitch)
{
    ASSERT_RET_VAL(false, "this method should never be called", CL_INVALID_OPERATION);
}

cl_err_code SVMPointerArg::WriteData(const void* pOutData,    const size_t* pszOrigin, const size_t* pszRegion, size_t szRowPitch, size_t szSlicePitch)
{
    ASSERT_RET_VAL(false, "this method should never be called", CL_INVALID_OPERATION);
}

bool SVMPointerArg::IsSynchDataWithHostRequired(cl_dev_cmd_param_map* IN pMapInfo, void* IN pHostMapDataPtr) const
{
    ASSERT_RET_VAL(false, "this method should never be called", false);
}

cl_err_code SVMPointerArg::SynchDataToHost(cl_dev_cmd_param_map* IN pMapInfo, void* IN pHostMapDataPtr)
{
    ASSERT_RET_VAL(false, "this method should never be called", CL_INVALID_OPERATION);
}

cl_err_code SVMPointerArg::SynchDataFromHost(cl_dev_cmd_param_map* IN pMapInfo, void* IN pHostMapDataPtr)
{
    ASSERT_RET_VAL(false, "this method should never be called", CL_INVALID_OPERATION);
}

cl_err_code SVMPointerArg::CreateSubBuffer(cl_mem_flags clFlags, cl_buffer_create_type buffer_create_type,    const void* buffer_create_info, SharedPtr<MemoryObject>* ppBuffer)
{
    ASSERT_RET_VAL(false, "this method should never be called", CL_INVALID_OPERATION);
}

bool SVMPointerArg::IsSupportedByDevice(const SharedPtr<FissionableDevice>& pDevice)
{
    ASSERT_RET_VAL(false, "this method should never be called", false);
}

cl_err_code    SVMPointerArg::MemObjCreateDevMappedRegion(const SharedPtr<FissionableDevice>&, cl_dev_cmd_param_map* cmd_param_map, void** pHostMapDataPtr)
{
    ASSERT_RET_VAL(false, "this method should never be called", CL_INVALID_OPERATION);
}

cl_err_code    SVMPointerArg::MemObjReleaseDevMappedRegion(const SharedPtr<FissionableDevice>&, cl_dev_cmd_param_map*    cmd_param_map, void* pHostMapDataPtr, bool force_unmap)
{
    ASSERT_RET_VAL(false, "this method should never be called", CL_INVALID_OPERATION);
}

// SVMBufferPointerArg methods:

cl_err_code SVMBufferPointerArg::LockOnDevice(IN const SharedPtr<FissionableDevice>& dev, IN MemObjUsage usage, OUT MemObjUsage* pOutActuallyUsage, OUT SharedPtr<OclEvent>& pOutEvent)
{
    return m_pSvmBuf->LockOnDevice(dev, usage, pOutActuallyUsage, pOutEvent);
}

cl_err_code SVMBufferPointerArg::UnLockOnDevice(IN const SharedPtr<FissionableDevice>& dev, IN MemObjUsage usage)
{
    return m_pSvmBuf->UnLockOnDevice(dev, usage);
}

cl_err_code SVMBufferPointerArg::GetDimensionSizes(size_t* pszRegion) const
{
    return m_pSvmBuf->GetDimensionSizes(pszRegion);
}

size_t SVMBufferPointerArg::GetRowPitchSize() const
{
    return m_pSvmBuf->GetRowPitchSize();
}

size_t SVMBufferPointerArg::GetSlicePitchSize() const
{
    return m_pSvmBuf->GetSlicePitchSize();
}

size_t SVMBufferPointerArg::GetPixelSize() const
{
    return m_pSvmBuf->GetPixelSize();
}

void SVMBufferPointerArg::GetLayout(OUT size_t* dimensions, OUT size_t* rowPitch, OUT size_t* slicePitch) const
{
    m_pSvmBuf->GetLayout(dimensions, rowPitch, slicePitch);
}

cl_err_code SVMBufferPointerArg::CheckBounds(const size_t* pszOrigin, const size_t* pszRegion) const
{
    return m_pSvmBuf->CheckBounds(pszOrigin, pszRegion);
}

cl_err_code SVMBufferPointerArg::CheckBoundsRect(const size_t* pszOrigin, const size_t* pszRegion, size_t szRowPitch, size_t szSlicePitch) const
{
    return m_pSvmBuf->CheckBoundsRect(pszOrigin, pszRegion, szRowPitch, szSlicePitch);
}

void* SVMBufferPointerArg::GetBackingStoreData(const size_t* pszOrigin) const
{
    return (char*)m_pSvmBuf->GetAddr() + m_szOffset;
}

cl_err_code SVMBufferPointerArg::CreateDeviceResource(const SharedPtr<FissionableDevice>& pDevice)
{
    return m_pSvmBuf->CreateDeviceResource(pDevice);
}

cl_err_code SVMBufferPointerArg::GetDeviceDescriptor(const SharedPtr<FissionableDevice>& IN pDevice, IOCLDevMemoryObject* OUT* ppDevObject, SharedPtr<OclEvent> OUT* ppEvent)
{
    IOCLDevMemoryObject* pSvmBufDevObj;
    const cl_err_code err = m_pSvmBuf->GetDeviceDescriptor(pDevice, &pSvmBufDevObj, ppEvent);    // what about ppEvent?
    if (CL_FAILED(err))
    {
        return err;
    }
    *ppDevObject = new SVMPointerArgDevMemoryObject(this, pSvmBufDevObj, m_szOffset);
    return CL_SUCCESS;
}

cl_err_code SVMBufferPointerArg::UpdateDeviceDescriptor(const SharedPtr<FissionableDevice>& IN pDevice, IOCLDevMemoryObject* OUT* ppDevObject)
{
    return m_pSvmBuf->UpdateDeviceDescriptor(pDevice, ppDevObject);
}

// SVMPointerArg::SVMPointerArgDevMemoryObject methods:

SVMPointerArg::SVMPointerArgDevMemoryObject::SVMPointerArgDevMemoryObject(const SharedPtr<SVMPointerArg>& pSvmPtrArg, IOCLDevMemoryObject* pSvmBufDevMemObj, size_t szOffset) :
    m_pSvmBufDevMemObj(pSvmBufDevMemObj)
{
    m_objDecr.dimensions.buffer_size = 0;    // unknown
    m_objDecr.dim_count = 1;
    m_objDecr.memObjType = CL_MEM_OBJECT_BUFFER;
    m_objDecr.pData = pSvmPtrArg->GetBackingStoreData();
}

cl_dev_err_code SVMPointerArg::SVMPointerArgDevMemoryObject::clDevMemObjCreateMappedRegion(cl_dev_cmd_param_map* pMapParams)
{
    ASSERT_RET_VAL(false, "this method should never be called", CL_DEV_INVALID_OPERATION);
}

cl_dev_err_code SVMPointerArg::SVMPointerArgDevMemoryObject::clDevMemObjUnmapAndReleaseMappedRegion(cl_dev_cmd_param_map* pMapParams)
{
    ASSERT_RET_VAL(false, "this method should never be called", CL_DEV_INVALID_OPERATION);
}

cl_dev_err_code SVMPointerArg::SVMPointerArgDevMemoryObject::clDevMemObjReleaseMappedRegion(cl_dev_cmd_param_map* pMapParams)
{
    ASSERT_RET_VAL(false, "this method should never be called", CL_DEV_INVALID_OPERATION);
}

cl_dev_err_code SVMPointerArg::SVMPointerArgDevMemoryObject::clDevMemObjGetDescriptor(cl_device_type dev_type, cl_dev_subdevice_id node_id, cl_dev_memobj_handle* handle)
{
    *handle = &m_objDecr;
    return CL_DEV_SUCCESS;
}

cl_dev_err_code SVMPointerArg::SVMPointerArgDevMemoryObject::clDevMemObjCreateSubObject(cl_mem_flags mem_flags, const size_t IN* origin, const size_t IN* size, IOCLDevRTMemObjectService IN* pBSService,
    IOCLDevMemoryObject* OUT* ppSubObject)
{
    ASSERT_RET_VAL(false, "this method should never be called", CL_DEV_INVALID_OPERATION);
}

cl_dev_err_code SVMPointerArg::SVMPointerArgDevMemoryObject::clDevMemObjUpdateBackingStore(void* operation_handle, cl_dev_bs_update_state* pUpdateState)
{
    if (nullptr != m_pSvmBufDevMemObj)
    {
        return m_pSvmBufDevMemObj->clDevMemObjUpdateBackingStore(operation_handle, pUpdateState);
    }
    return CL_DEV_SUCCESS;
}
    
cl_dev_err_code SVMPointerArg::SVMPointerArgDevMemoryObject::clDevMemObjUpdateFromBackingStore(void* operation_handle, cl_dev_bs_update_state* pUpdateState)
{
    if (nullptr != m_pSvmBufDevMemObj)
    {
        return m_pSvmBufDevMemObj->clDevMemObjUpdateFromBackingStore(operation_handle, pUpdateState);
    }
    return CL_DEV_SUCCESS;
}

cl_dev_err_code SVMPointerArg::SVMPointerArgDevMemoryObject::clDevMemObjInvalidateData()
{
    if (nullptr != m_pSvmBufDevMemObj)
    {
        return m_pSvmBufDevMemObj->clDevMemObjInvalidateData();
    }
    return CL_DEV_SUCCESS;
}

cl_dev_err_code SVMPointerArg::SVMPointerArgDevMemoryObject::clDevMemObjRelease()
{
    if (nullptr != m_pSvmBufDevMemObj)
    {
        return m_pSvmBufDevMemObj->clDevMemObjRelease();
    }
    return CL_DEV_SUCCESS;
}
