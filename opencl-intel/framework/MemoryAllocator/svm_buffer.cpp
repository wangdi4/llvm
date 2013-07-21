// Copyright (c) 2006-2007 Intel Corporation
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
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES{ } LOSS OF USE, DATA, OR
// PROFITS{ } OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THESE
// MATERIALS, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Intel Corporation is the author of the Materials, and requests that all
// problem reports or change requests be submitted to it directly

#include "svm_buffer.h"
#include "cl_shared_ptr.hpp"

using namespace Intel::OpenCL::Framework;

// SVMPointerArg methods:

cl_err_code SVMPointerArg::Initialize(cl_mem_flags	clMemFlags,	const cl_image_format* pclImageFormat, unsigned int	dim_count, const size_t* dimension,	const size_t* pitches,
	void* pHostPtr,	cl_rt_memobj_creation_flags	creation_flags)
{
	ASSERT_RET_VAL(false, "this method should never be called", CL_INVALID_OPERATION);
}

cl_err_code SVMPointerArg::UpdateHostPtr(cl_mem_flags clMemFlags, void* pHostPtr)
{
	ASSERT_RET_VAL(false, "this method should never be called", CL_INVALID_OPERATION);
}

cl_err_code SVMPointerArg::LockOnDevice(IN const ConstSharedPtr<FissionableDevice>& dev, IN MemObjUsage usage, OUT MemObjUsage* pOutActuallyUsage, OUT SharedPtr<OclEvent>& pOutEvent)
{
	if (m_pSvmBuf != NULL)
	{
		return m_pSvmBuf->LockOnDevice(dev, usage, pOutActuallyUsage, pOutEvent);
	}
	return CL_SUCCESS;
}

cl_err_code SVMPointerArg::UnLockOnDevice(IN const ConstSharedPtr<FissionableDevice>& dev, IN MemObjUsage usage)
{
	if (m_pSvmBuf != NULL)
	{
		return m_pSvmBuf->UnLockOnDevice(dev, usage);
	}
	return CL_SUCCESS;
}

cl_err_code SVMPointerArg::ReadData(void* pOutData, const size_t* pszOrigin, const size_t* pszRegion, size_t szRowPitch, size_t szSlicePitch)
{
	ASSERT_RET_VAL(false, "this method should never be called", CL_INVALID_OPERATION);
}

cl_err_code SVMPointerArg::WriteData(const void* pOutData,	const size_t* pszOrigin, const size_t* pszRegion, size_t szRowPitch, size_t szSlicePitch)
{
	ASSERT_RET_VAL(false, "this method should never be called", CL_INVALID_OPERATION);
}

cl_err_code SVMPointerArg::GetDimensionSizes(size_t* pszRegion) const
{
	if (m_pSvmBuf != NULL)
	{
		return m_pSvmBuf->GetDimensionSizes(pszRegion);
	}
	return CL_SUCCESS;
}

size_t SVMPointerArg::GetRowPitchSize() const
{
	if (m_pSvmBuf != NULL)
	{
		return m_pSvmBuf->GetRowPitchSize();
	}
	return 0;
}

size_t SVMPointerArg::GetSlicePitchSize() const
{
	if (m_pSvmBuf != NULL)
	{
		return m_pSvmBuf->GetSlicePitchSize();
	}
	return 0;
}

size_t SVMPointerArg::GetPixelSize() const
{
	if (m_pSvmBuf != NULL)
	{
		m_pSvmBuf->GetPixelSize();
	}
	return 0;
}

void SVMPointerArg::GetLayout(OUT size_t* dimensions, OUT size_t* rowPitch, OUT size_t* slicePitch) const
{
	if (m_pSvmBuf != NULL)
	{
		m_pSvmBuf->GetLayout(dimensions, rowPitch, slicePitch);
	}
}

cl_err_code SVMPointerArg::CheckBounds(const size_t* pszOrigin, const size_t* pszRegion) const
{
	if (m_pSvmBuf != NULL)
	{
		return m_pSvmBuf->CheckBounds(pszOrigin, pszRegion);
	}
	return CL_SUCCESS;
}

cl_err_code SVMPointerArg::CheckBoundsRect(const size_t* pszOrigin, const size_t* pszRegion, size_t szRowPitch, size_t szSlicePitch) const
{
	if (m_pSvmBuf != NULL)
	{
		return m_pSvmBuf->CheckBoundsRect(pszOrigin, pszRegion, szRowPitch, szSlicePitch);
	}
	return CL_SUCCESS;
}

void* SVMPointerArg::GetBackingStoreData(const size_t* pszOrigin) const
{
	return (char*)m_pSvmBuf->GetAddr() + m_szOffset;
}

cl_err_code SVMPointerArg::CreateDeviceResource(const SharedPtr<FissionableDevice>& pDevice)
{
	if (m_pSvmBuf != NULL)
	{
		return m_pSvmBuf->CreateDeviceResource(pDevice);
	}
	return CL_SUCCESS;
}

cl_err_code SVMPointerArg::GetDeviceDescriptor(const SharedPtr<FissionableDevice>& IN pDevice, IOCLDevMemoryObject* OUT* ppDevObject, SharedPtr<OclEvent> OUT* ppEvent)
{
	if (m_pSvmBuf != NULL)
	{
		IOCLDevMemoryObject* pSvmBufDevObj;
		const cl_err_code err = m_pSvmBuf->GetDeviceDescriptor(pDevice, &pSvmBufDevObj, ppEvent);	// what about ppEvent?
		if (CL_FAILED(err))
		{
			return err;
		}
		*ppDevObject = new SVMPointerArgDevMemoryObject(this, *pSvmBufDevObj, m_szOffset);
	}
	return CL_SUCCESS;
}

cl_err_code SVMPointerArg::UpdateDeviceDescriptor(const SharedPtr<FissionableDevice>& IN pDevice, IOCLDevMemoryObject* OUT* ppDevObject)
{
	if (m_pSvmBuf != NULL)
	{
		return m_pSvmBuf->UpdateDeviceDescriptor(pDevice, ppDevObject);
	}
	return CL_SUCCESS;
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

cl_err_code SVMPointerArg::CreateSubBuffer(cl_mem_flags clFlags, cl_buffer_create_type buffer_create_type,	const void* buffer_create_info, SharedPtr<MemoryObject>* ppBuffer)
{
	ASSERT_RET_VAL(false, "this method should never be called", CL_INVALID_OPERATION);
}

bool SVMPointerArg::IsSupportedByDevice(const SharedPtr<FissionableDevice>& pDevice)
{
	ASSERT_RET_VAL(false, "this method should never be called", false);
}

cl_err_code	SVMPointerArg::MemObjCreateDevMappedRegion(const SharedPtr<FissionableDevice>&, cl_dev_cmd_param_map* cmd_param_map, void** pHostMapDataPtr)
{
	ASSERT_RET_VAL(false, "this method should never be called", CL_INVALID_OPERATION);
}

cl_err_code	SVMPointerArg::MemObjReleaseDevMappedRegion(const SharedPtr<FissionableDevice>&, cl_dev_cmd_param_map*	cmd_param_map, void* pHostMapDataPtr)
{
	ASSERT_RET_VAL(false, "this method should never be called", CL_INVALID_OPERATION);
}

// SVMPointerArg::SVMPointerArgDevMemoryObject methods:

SVMPointerArg::SVMPointerArgDevMemoryObject::SVMPointerArgDevMemoryObject(const SharedPtr<SVMPointerArg>& pSvmPtrArg, IOCLDevMemoryObject& svmBufDevMemObj, size_t szOffset) :
	m_svmBufDevMemObj(svmBufDevMemObj), m_szOffset(szOffset)
{
	m_objDecr.dimensions.buffer_size = 0;	// unknown
	m_objDecr.dim_count = 1;
	m_objDecr.memObjType = CL_MEM_OBJECT_BUFFER;
	m_objDecr.pData = pSvmPtrArg->GetBackingStoreData();
}

cl_dev_err_code SVMPointerArg::SVMPointerArgDevMemoryObject::clDevMemObjCreateMappedRegion(cl_dev_cmd_param_map* pMapParams)
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
	return m_svmBufDevMemObj.clDevMemObjUpdateBackingStore(operation_handle, pUpdateState);
}
    
cl_dev_err_code SVMPointerArg::SVMPointerArgDevMemoryObject::clDevMemObjUpdateFromBackingStore(void* operation_handle, cl_dev_bs_update_state* pUpdateState)
{
	return m_svmBufDevMemObj.clDevMemObjUpdateFromBackingStore(operation_handle, pUpdateState);
}

cl_dev_err_code SVMPointerArg::SVMPointerArgDevMemoryObject::clDevMemObjInvalidateData()
{
	return m_svmBufDevMemObj.clDevMemObjInvalidateData();
}

cl_dev_err_code SVMPointerArg::SVMPointerArgDevMemoryObject::clDevMemObjRelease()
{
	return m_svmBufDevMemObj.clDevMemObjRelease();
}
