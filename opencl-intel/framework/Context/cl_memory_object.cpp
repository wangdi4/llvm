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
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THESE
// MATERIALS, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// 
// Intel Corporation is the author of the Materials, and requests that all
// problem reports or change requests be submitted to it directly

///////////////////////////////////////////////////////////////////////////////////////////////////
//  cl_memory_object.cpp
//  Implementation of the MemoryObject Class
//  Created on:      10-Dec-2008 2:08:23 PM
//  Original author: ulevy
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "cl_memory_object.h"
#include <device.h>
#include <assert.h>
#include <Windows.h>

using namespace std;
using namespace Intel::OpenCL::Framework;

///////////////////////////////////////////////////////////////////////////////////////////////////
// DeviceMemoryObject C'tor
///////////////////////////////////////////////////////////////////////////////////////////////////
DeviceMemoryObject::DeviceMemoryObject(Device * pDevice, LoggerClient * pLoggerClient)
{
	SET_LOGGER_CLIENT(pLoggerClient);
	m_pDevice = pDevice;
	m_bAllocated = false;
	m_bDataValid = false;
	m_clDevMemId = 0;
	m_uiMapCount = 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// DeviceMemoryObject D'tor
///////////////////////////////////////////////////////////////////////////////////////////////////
DeviceMemoryObject::~DeviceMemoryObject()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// DeviceMemoryObject::AllocateBuffer
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code DeviceMemoryObject::AllocateBuffer(cl_mem_flags clMemFlags, size_t szBuffersize, void * pHostPtr)
{
	LOG_DEBUG(L"Enter AllocateBuffer (clMemFlags=%d, szBuffersize=%d", clMemFlags, szBuffersize);

#ifdef _DEBUG
	assert ( NULL != m_pDevice );
#endif

	cl_dev_mem_flags clDevMemFlags = GetDevMemFlags(clMemFlags);
	cl_dev_host_ptr_flags clDevHostFlags = GetDevHostFlags(clMemFlags);

    OclAutoMutex CS(&m_oclLocker); // release on return
	if ( m_bAllocated )
	{
		return CL_SUCCESS;
	}

	cl_err_code clErr = m_pDevice->CreateMemoryObject(clDevMemFlags, NULL, 1, &szBuffersize, pHostPtr, NULL,
							clDevHostFlags, &m_clDevMemId);

	if (CL_SUCCEEDED(clErr))
	{
		m_bAllocated = true;
	}
	return clErr;
    // End Critical section
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// DeviceMemoryObject::AllocateImage2D
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code DeviceMemoryObject::AllocateImage2D(cl_mem_flags clMemFlags,
												const cl_image_format * pclImageFormat,
												size_t szImageWidth, 
												size_t szImageHeight, 
												size_t szImageRowPitch, 
												void * pHostPtr)
{
	LOG_DEBUG(L"Enter AllocateImage2D (clMemFlags=%d, szImageWidth=%d, szImageHeight=%d, szImageRowPitch=%d", 
		clMemFlags, szImageWidth, szImageHeight, szImageRowPitch);

#ifdef _DEBUG
	assert ( NULL != m_pDevice );
#endif

	cl_dev_mem_flags clDevMemFlags = GetDevMemFlags(clMemFlags);
	cl_dev_host_ptr_flags clDevHostFlags = GetDevHostFlags(clMemFlags);

	size_t pszDims[2] = {szImageWidth, szImageHeight};
    // If no HostPtr no need for pitch OUT parameter???
    size_t* pszPitch = (( NULL == pHostPtr) ? NULL : &szImageRowPitch);

    OclAutoMutex CS(&m_oclLocker); // release on return
	if ( m_bAllocated )
	{
		return CL_SUCCESS;
	}

	cl_err_code clErr = m_pDevice->CreateMemoryObject(clDevMemFlags, pclImageFormat, 2, pszDims, pHostPtr, pszPitch,
							clDevHostFlags, &m_clDevMemId);
	if (CL_SUCCEEDED(clErr))
	{
		m_bAllocated = true;
	}
	return clErr;
    // End Critical section
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// DeviceMemoryObject::AllocateImage3D
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code DeviceMemoryObject::AllocateImage3D(cl_mem_flags clMemFlags,
												const cl_image_format * pclImageFormat,
												size_t szImageWidth, 
												size_t szImageHeight, 
												size_t szImageDepth, 
												size_t szImageRowPitch, 
												size_t szImageSlicePitch,
												void * pHostPtr)
{
	LOG_DEBUG(L"Enter AllocateImage2D (clMemFlags=%d, szImageWidth=%d, szImageHeight=%d, szImageDepth=%d, szImageRowPitch=%d, szImageSlicePitch=%d", 
		clMemFlags, szImageWidth, szImageHeight, szImageDepth, szImageRowPitch, szImageSlicePitch);

#ifdef _DEBUG
	assert ( NULL != m_pDevice );
#endif

	cl_dev_mem_flags clDevMemFlags = GetDevMemFlags(clMemFlags);
	cl_dev_host_ptr_flags clDevHostFlags = GetDevHostFlags(clMemFlags);

	size_t pszDims[3] = {szImageWidth, szImageHeight, szImageDepth};
	size_t pszSlices[2] = {szImageRowPitch, szImageSlicePitch};

    // If no HostPtr no need for pitch OUT parameter???
    size_t* pszPitch = (( NULL == pHostPtr) ? NULL : pszSlices);

	OclAutoMutex CS(&m_oclLocker); // release on return
	if ( m_bAllocated )
	{
		return CL_SUCCESS;
	}

	cl_err_code clErr = m_pDevice->CreateMemoryObject(clDevMemFlags, pclImageFormat, 3, pszDims, pHostPtr, pszPitch,
							clDevHostFlags, &m_clDevMemId);
	if (CL_SUCCEEDED(clErr))
	{
		m_bAllocated = true;
	}
	return clErr;
    // End Critical section
}

cl_err_code DeviceMemoryObject::Release()
{
	LOG_DEBUG(L"Enter DeviceMemoryObject::Release");
    OclAutoMutex CS(&m_oclLocker);
	if (false == m_bAllocated)
	{
		return CL_SUCCESS;
	}
	cl_err_code clErr = m_pDevice->DeleteMemoryObject(m_clDevMemId);
	if (CL_SUCCEEDED(clErr))
	{
		m_bAllocated = false;
	}
	return clErr;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// DeviceMemoryObject::CreateMappedRegion
///////////////////////////////////////////////////////////////////////////////////////////////////
void * DeviceMemoryObject::CreateMappedRegion(cl_map_flags   clMapFlags, 
                                              size_t         szNumDims,
											  const size_t * szOrigins,
											  const size_t * szRegions, 
											  size_t * pszImageRowPitch,
											  size_t * pszImageSlicePitch)
{
	LOG_DEBUG(L"Enter CreateMappedRegion(clMapFlags=%d, szNumDims=%d, szOrigins=%d, szRegions=%d, pszImageRowPitch=%d, pszImageSlicePitch=%d",
		clMapFlags, szNumDims, szOrigins, szRegions, pszImageRowPitch, pszImageSlicePitch);

	cl_dev_cmd_param_map * pclDevCmdParamMap = NULL;
	OclAutoMutex CS(&m_oclLocker); // release on return

	// check that the region hasn't been mapped before
	map<void*, cl_dev_cmd_param_map*>::iterator it = m_mapMappedRegions.begin();
	while (it != m_mapMappedRegions.end())
	{
		pclDevCmdParamMap = it->second;
		if (pclDevCmdParamMap->dim_count == szNumDims)
		{
			if ((szNumDims == 1) && (szOrigins[0] == pclDevCmdParamMap->origin[0])
								 && (szRegions[0] == pclDevCmdParamMap->region[0]))
			{
				return NULL;
			}
			if ((szNumDims == 2) && (szOrigins[0] == pclDevCmdParamMap->origin[0])
								 && (szOrigins[1] == pclDevCmdParamMap->origin[1])
								 && (szRegions[0] == pclDevCmdParamMap->region[0])
								 && (szRegions[1] == pclDevCmdParamMap->region[1]))
			{
				return NULL;
			}
			if ((szNumDims == 3) && (szOrigins[0] == pclDevCmdParamMap->origin[0])
								 && (szOrigins[1] == pclDevCmdParamMap->origin[1])
								 && (szOrigins[2] == pclDevCmdParamMap->origin[2])
								 && (szRegions[0] == pclDevCmdParamMap->region[0])
								 && (szRegions[1] == pclDevCmdParamMap->region[1])
								 && (szRegions[2] == pclDevCmdParamMap->region[2]))
			{
				return NULL;
			}
		}
		it++;
	}

	// create new map parameter structure and assign value to it
	pclDevCmdParamMap = new cl_dev_cmd_param_map();
	
	pclDevCmdParamMap->ptr = NULL;
	pclDevCmdParamMap->memObj = m_clDevMemId;
	pclDevCmdParamMap->flags = GetDevMapFlags(clMapFlags);
    pclDevCmdParamMap->dim_count = szNumDims;
	memcpy(pclDevCmdParamMap->origin, szOrigins, sizeof(size_t)*szNumDims);
	memcpy(pclDevCmdParamMap->region, szRegions, sizeof(size_t)*szNumDims);

	cl_err_code clErr = m_pDevice->CreateMappedRegion(pclDevCmdParamMap);

	if (CL_SUCCEEDED(clErr))
	{
		if (NULL != pszImageRowPitch)
		{
			*pszImageRowPitch = pclDevCmdParamMap->pitch[0];
		}
		if (NULL != pszImageSlicePitch)
		{
			*pszImageSlicePitch = pclDevCmdParamMap->pitch[1];
		}
		m_mapMappedRegions[pclDevCmdParamMap->ptr] = pclDevCmdParamMap;
	}
	return pclDevCmdParamMap->ptr;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// DeviceMemoryObject::ReleaseMappedRegion
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code DeviceMemoryObject::ReleaseMappedRegion(void * pMappedPtr)
{	
	OclAutoMutex CS(&m_oclLocker); // release on return
	map<void*, cl_dev_cmd_param_map*>::iterator it = m_mapMappedRegions.find(pMappedPtr);
	if (it == m_mapMappedRegions.end())
	{
		return CL_INVALID_VALUE;
	}
	cl_err_code clErr = m_pDevice->ReleaseMappedRegion(it->second);
    delete it->second;
	if (CL_FAILED(clErr))
	{
		return clErr;
	}
	m_mapMappedRegions.erase(it);

	return CL_SUCCESS;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// DeviceMemoryObject::GetMappedRegionInfo
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_dev_cmd_param_map* DeviceMemoryObject::GetMappedRegionInfo(void* mappedPtr)
{
	OclAutoMutex CS(&m_oclLocker); // release on return
	map<void*, cl_dev_cmd_param_map*>::iterator it = m_mapMappedRegions.find(mappedPtr);
	if (it == m_mapMappedRegions.end())
	{
		return NULL;
	}
	cl_dev_cmd_param_map * pclDevCmdParamMap = it->second;
	if (NULL == pclDevCmdParamMap)
	{
		return NULL;
	}
	return pclDevCmdParamMap;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// DeviceMemoryObject::ConvertMemFlagsToDevMemFlags
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_dev_mem_flags DeviceMemoryObject::GetDevMemFlags(cl_mem_flags clMemFlags)
{
	int iMemFlags = 0;
	cl_dev_mem_flags clDevMemFlags;

	if (clMemFlags & (CL_MEM_USE_HOST_PTR | CL_MEM_ALLOC_HOST_PTR | CL_MEM_COPY_HOST_PTR))
	{
		iMemFlags |= CL_DEV_MEM_HOST_MEM;
	}
	if (clMemFlags & CL_MEM_READ_ONLY)
	{
		iMemFlags |= CL_DEV_MEM_READ;
	}
	else if (clMemFlags & CL_MEM_WRITE_ONLY)
	{
		iMemFlags |= CL_DEV_MEM_WRITE;
	}
	else if (clMemFlags & CL_MEM_READ_WRITE)
	{
		iMemFlags |= CL_DEV_MEM_READ_WRITE;
	}
	clDevMemFlags = (cl_dev_mem_flags)iMemFlags;
	return clDevMemFlags;
}

cl_dev_map_flags DeviceMemoryObject::GetDevMapFlags(cl_map_flags clMapFlags)
{
	int iMapFlags = 0;
	if (clMapFlags & CL_MAP_READ)
	{
		iMapFlags |= CL_DEV_MAP_READ;
	}
	if (clMapFlags & CL_MAP_WRITE)
	{
		iMapFlags |= CL_DEV_MAP_WRITE;
	}
	return (cl_dev_map_flags)iMapFlags;
}

cl_dev_host_ptr_flags DeviceMemoryObject::GetDevHostFlags(cl_mem_flags clMemFlags)
{
	int iHostFlags = CL_DEV_HOST_PTR_NONE;

	if ( CL_MEM_USE_HOST_PTR & clMemFlags )
	{
		iHostFlags = CL_DEV_HOST_PTR_DATA_AVAIL | CL_DEV_HOST_PTR_MAPPED_REGION;
	}
	else if ( CL_MEM_COPY_HOST_PTR & clMemFlags )
	{
		iHostFlags = CL_DEV_HOST_PTR_DATA_AVAIL;
	}

	return (cl_dev_host_ptr_flags)iHostFlags;
}

cl_dev_mem_object_type DeviceMemoryObject::GetDevMemObjType(cl_mem_object_type clMemObjType)
{
	cl_dev_mem_object_type clDevMemObjType = CL_DEV_MEM_OBJECT_BUFFER;
	
	if (clMemObjType == CL_MEM_OBJECT_BUFFER)
	{
		clDevMemObjType = CL_DEV_MEM_OBJECT_BUFFER;
	}
	else if (clMemObjType == CL_MEM_OBJECT_IMAGE2D)
	{
		clDevMemObjType = CL_DEV_MEM_OBJECT_IMAGE2D;
	}
	else if (clMemObjType == CL_MEM_OBJECT_IMAGE3D)
	{
		clDevMemObjType = CL_DEV_MEM_OBJECT_IMAGE3D;
	}
	return clDevMemObjType;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// MemoryObject C'tor
///////////////////////////////////////////////////////////////////////////////////////////////////
MemoryObject::MemoryObject(Context * pContext, cl_mem_flags clMemFlags, void * pHostPtr, cl_err_code * pErr)
{
#ifdef _DEBUG
	assert ( NULL != pErr );
	assert ( NULL != pContext );
#endif

	INIT_LOGGER_CLIENT(L"MemoryObject", LL_DEBUG);

	*pErr = CL_SUCCESS;

	m_pContext = pContext;
	m_clFlags = clMemFlags;

	m_clMemObjectType = 0;
	m_uiMapCount = 0;

	m_szMemObjSize = 0;
	m_pMemObjData = NULL;

	m_lDataOnHost = 0;

    // Sign to be dependent on the context, ensure the context will be delated only after the object was
    m_pContext->AddPendency();

	// assign default value
	if ( !(m_clFlags & (CL_MEM_READ_ONLY | CL_MEM_WRITE_ONLY | CL_MEM_READ_WRITE)) )
	{
		m_clFlags = m_clFlags | CL_MEM_READ_WRITE;
	}

	// check input memory flags
	*pErr = CheckMemFlags(m_clFlags);
	if (CL_FAILED(*pErr))
	{
		LOG_ERROR(L"CheckMemFlags(%d) = %ws", m_clFlags, ClErrTxt(*pErr));
		return;
	}

	// check invalid usage of host ptr
	if (((NULL == pHostPtr) && (m_clFlags & (CL_MEM_USE_HOST_PTR | CL_MEM_COPY_HOST_PTR)))	||
		((NULL != pHostPtr) && !(m_clFlags & (CL_MEM_USE_HOST_PTR | CL_MEM_COPY_HOST_PTR))))
	{
		LOG_ERROR(L"invalid usage of host ptr");
		*pErr = CL_INVALID_HOST_PTR;
	}

	// save host ptr only if CL_MEM_USE_HOST_PTR is set
	m_pHostPtr = NULL;
	if (m_clFlags & CL_MEM_USE_HOST_PTR)
	{
		m_pHostPtr = pHostPtr;
	}
	
	//create device memory object for each device
	cl_uint uiNumDevices = 0;
	Device ** ppDevices = m_pContext->GetDevices(&uiNumDevices);

	for (cl_uint ui=0; ui<uiNumDevices; ++ui)
	{
#ifdef _DEBUG
		assert (NULL != ppDevices[ui]);
#endif
		DeviceMemoryObject * pDevMemObj = new DeviceMemoryObject(ppDevices[ui], GET_LOGGER_CLIENT);
		m_mapDeviceMemObjects[(cl_device_id)(ppDevices[ui]->GetId())] = pDevMemObj;
	}
	m_pHandle = new _cl_mem;
	m_pHandle->object = this;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// MemoryObject D'tor
///////////////////////////////////////////////////////////////////////////////////////////////////
MemoryObject::~MemoryObject()
{
	LOG_DEBUG(L"Enter MemoryObject D'tor");

    m_pContext->RemovePendency();

	map<cl_device_id, DeviceMemoryObject*>::iterator it = m_mapDeviceMemObjects.begin();
	while (it != m_mapDeviceMemObjects.end())
	{
		DeviceMemoryObject * pDeviceMemObj = it->second;
		if (NULL != pDeviceMemObj)
		{
			pDeviceMemObj->Release();
			delete pDeviceMemObj;
		}
		it++;
	}
	m_mapDeviceMemObjects.clear();

	if (NULL != m_pMemObjData)
	{
		if (!(m_clFlags & CL_MEM_USE_HOST_PTR))
		{
			_aligned_free(m_pMemObjData);
		}
		m_pMemObjData = NULL;
	}

	RELEASE_LOGGER_CLIENT;

	if (NULL != m_pHandle)
	{
		delete m_pHandle;
	}

}
///////////////////////////////////////////////////////////////////////////////////////////////////
// MemoryObject::Initialize
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code MemoryObject::Initialize(void * pHostPtr)
{
	if (m_clFlags & CL_MEM_USE_HOST_PTR)
	{
#ifdef _DEBUG
		assert(NULL != pHostPtr);
#endif
		// in case that we're using host ptr we don't need to allocated memory for the buffer
		// just use the host ptr instead
		m_pMemObjData = m_pHostPtr;
	}
	else
	{
		m_pMemObjData = _aligned_malloc(m_szMemObjSize, CPU_DCU_LINE_SIZE);
		if (NULL == m_pMemObjData)
		{
			return CL_OUT_OF_HOST_MEMORY;
		}
		
		if (m_clFlags & CL_MEM_COPY_HOST_PTR)
		{
#ifdef _DEBUG
			assert(NULL != pHostPtr);
#endif
			errno_t err = memcpy_s(m_pMemObjData, m_szMemObjSize, pHostPtr, m_szMemObjSize);
			if (err)
			{
				return CL_OUT_OF_HOST_MEMORY;
			}
			m_lDataOnHost = 1; //true
			m_pHostPtr = m_pMemObjData;
		}
	}
	return CL_SUCCESS;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// MemoryObject::IsAllocated
///////////////////////////////////////////////////////////////////////////////////////////////////
bool MemoryObject::IsAllocated(cl_device_id clDeviceId)
{
	LOG_DEBUG(L"Enter IsReady (clDeviceId=%d)", clDeviceId);

	if ( ::InterlockedCompareExchange(&m_lDataOnHost, 0, 0) && (0 == clDeviceId))
	{
		return true;
	}

	map<cl_device_id, DeviceMemoryObject*>::iterator it = m_mapDeviceMemObjects.find(clDeviceId);
	if (it == m_mapDeviceMemObjects.end())
	{
		// device not found
		return false;
	}
	// get the device memory objectand check if it was allocated
	DeviceMemoryObject * pDevMemObj = it->second;
	return pDevMemObj->IsAllocated();
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// MemoryObject::GetDeviceMemoryHndl
// If there is no resource in this device, or resource is not valid, 0 hndl is returned.
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_dev_mem MemoryObject::GetDeviceMemoryHndl( cl_device_id clDeviceId )
{
	LOG_DEBUG(L"Enter GetDeviceMemoryHndl");

	map<cl_device_id, DeviceMemoryObject*>::iterator it = m_mapDeviceMemObjects.find(clDeviceId);
	if (it == m_mapDeviceMemObjects.end())
	{
		// device not found
		return 0;
	}
	// get the device memory objectand check if it was allocated
	DeviceMemoryObject * pDevMemObj = it->second;
	return pDevMemObj->GetDeviceMemoryId();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// MemoryObject::GetDataLocation
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_device_id MemoryObject::GetDataLocation()
{
	LOG_DEBUG(L"Enter GetDataLocation");

	if ( ::InterlockedCompareExchange(&m_lDataOnHost, 0, 0) )
	{
		return 0;
	}

	for (map<cl_device_id, DeviceMemoryObject*>::iterator it = m_mapDeviceMemObjects.begin(); it != m_mapDeviceMemObjects.end(); it++)
	{
		DeviceMemoryObject * pDevMemObj = it->second;
		if (pDevMemObj->IsDataValid())
		{
			return it->first;
		}
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// MemoryObject::SetDataLocation
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code MemoryObject::SetDataLocation(cl_device_id clDevice)
{
	LOG_DEBUG(L"Enter SetDataLocation (clDevice=%d)", clDevice);

	map<cl_device_id, DeviceMemoryObject*>::iterator it = m_mapDeviceMemObjects.find(clDevice);
	if (it == m_mapDeviceMemObjects.end())
	{
		LOG_ERROR(L"Can't find device %d", clDevice);
		return CL_INVALID_DEVICE;
	}

	for (it = m_mapDeviceMemObjects.begin(); it != m_mapDeviceMemObjects.end(); it++)
	{
		DeviceMemoryObject * pDevMemObj = it->second;

#ifdef _DEBUG
		assert ( pDevMemObj != NULL );
#endif
		if (it->first != clDevice)
		{
			pDevMemObj->SetDataValid(false);
		}
		else
		{
			pDevMemObj->SetDataValid(true);
		}
	}
	// if data on host (m_lDataOnHost==1), change to false (0)
	::InterlockedExchange(&m_lDataOnHost, 0);
	return CL_SUCCESS;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// MemoryObject::GetInfo
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code	MemoryObject::GetInfo(cl_int iParamName, size_t szParamValueSize, void * pParamValue, size_t * pszParamValueSizeRet)
{
	LOG_DEBUG(L"Enter MemoryObject::GetInfo (iParamName=%d, szParamValueSize=%d, pParamValue=%d, pszParamValueSizeRet=%d)", 
		iParamName, szParamValueSize, pParamValue, pszParamValueSizeRet);

	if ((NULL == pParamValue && NULL == pszParamValueSizeRet) || 
		(NULL == pParamValue && iParamName != 0))
	{
		return CL_INVALID_VALUE;
	}
	size_t szSize = 0;
	size_t szParam = 0;
	cl_context clContext = 0;
	void * pValue = NULL;
	
	cl_err_code clErrRet = CL_SUCCESS;
	switch ( (cl_mem_info)iParamName )
	{

	case CL_MEM_TYPE:
		szSize = sizeof(cl_mem_object_type);
		pValue = &m_clMemObjectType;
		break;
	case CL_MEM_FLAGS:
		szSize = sizeof(cl_mem_flags);
		pValue = &m_clFlags;
		break;
	case CL_MEM_SIZE:
		szSize = sizeof(size_t);
		szParam = (size_t)GetSize();
		pValue = &szParam;
		break;
	case CL_MEM_HOST_PTR:
		szSize = sizeof(void*);
		pValue = &m_pHostPtr;
		break;
	case CL_MEM_MAP_COUNT:
		szSize = sizeof(cl_uint);
		pValue = &m_uiMapCount;
		break;
	case CL_MEM_REFERENCE_COUNT:
		szSize = sizeof(cl_uint);
		pValue = &m_uiRefCount;
		break;
	case CL_MEM_CONTEXT:
		szSize = sizeof(cl_context);
		clContext = (cl_context)m_pContext->GetId();
		pValue = &clContext;
		break;
	default:
		LOG_ERROR(L"param_name (=%d) isn't valid", iParamName);
		return CL_INVALID_VALUE;
	}
	if (CL_FAILED(clErrRet))
	{
		return clErrRet;
	}

	// if param_value_size < actual value size return CL_INVALID_VALUE
	if (NULL != pParamValue && szParamValueSize < szSize)
	{
		LOG_ERROR(L"szParamValueSize (=%d) < szSize (=%d)", szParamValueSize, szSize);
		return CL_INVALID_VALUE;
	}

	// return param value size
	if (NULL != pszParamValueSizeRet)
	{
		*pszParamValueSizeRet = szSize;
	}

	if (NULL != pParamValue && szSize > 0)
	{
		memcpy_s(pParamValue, szParamValueSize, pValue, szSize);
	}

	return CL_SUCCESS;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// MemoryObject::Release
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code MemoryObject::Release()
{
	return OCLObject::Release();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// MemoryObject::CheckMemFlags
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code MemoryObject::CheckMemFlags(cl_mem_flags clMemFlags)
{
	if ( (0 == clMemFlags) ||
		 ((clMemFlags & CL_MEM_READ_ONLY) && (clMemFlags & CL_MEM_WRITE_ONLY)) ||
		 ((clMemFlags & CL_MEM_READ_ONLY) && (clMemFlags & CL_MEM_READ_WRITE)) ||
		 ((clMemFlags & CL_MEM_WRITE_ONLY) && (clMemFlags & CL_MEM_READ_WRITE))||
         ((clMemFlags & CL_MEM_USE_HOST_PTR) && (clMemFlags & CL_MEM_ALLOC_HOST_PTR))
         )          
	{
		return CL_INVALID_VALUE;
	}
	return CL_SUCCESS;
}