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
#include "cl_sys_defines.h"
#include <Device.h>
#include <assert.h>
#include <cl_buffer.h>
#include "cl_sys_defines.h"


using namespace std;
using namespace Intel::OpenCL::Framework;

///////////////////////////////////////////////////////////////////////////////////////////////////
// DeviceMemoryObject C'tor
///////////////////////////////////////////////////////////////////////////////////////////////////
DeviceMemoryObject::DeviceMemoryObject(FissionableDevice * pDevice, LoggerClient * pLoggerClient) : m_bAllocated(false), m_bDataValid(false),m_pDevice(pDevice),m_clDevMemId(0)
{
	SET_LOGGER_CLIENT(pLoggerClient);
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
	LOG_DEBUG(TEXT("Enter AllocateBuffer (clMemFlags=%d, szBuffersize=%d"), clMemFlags, szBuffersize);

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

	cl_err_code clErr = m_pDevice->GetDeviceAgent()->clDevCreateMemoryObject(clDevMemFlags, NULL, 1, &szBuffersize, pHostPtr, NULL,
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
	LOG_DEBUG(TEXT("Enter AllocateImage2D (clMemFlags=%d, szImageWidth=%d, szImageHeight=%d, szImageRowPitch=%d"), 
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

	cl_err_code clErr = m_pDevice->GetDeviceAgent()->clDevCreateMemoryObject(clDevMemFlags, pclImageFormat, 2, pszDims, pHostPtr, pszPitch,
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
	LOG_DEBUG(TEXT("Enter AllocateImage2D (clMemFlags=%d, szImageWidth=%d, szImageHeight=%d, szImageDepth=%d, szImageRowPitch=%d, szImageSlicePitch=%d"), 
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

	cl_err_code clErr = m_pDevice->GetDeviceAgent()->clDevCreateMemoryObject(clDevMemFlags, pclImageFormat, 3, pszDims, pHostPtr, pszPitch,
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
	LOG_DEBUG(TEXT("%S"), TEXT("Enter DeviceMemoryObject::Release"));
    OclAutoMutex CS(&m_oclLocker);
	if (false == m_bAllocated)
	{
		return CL_SUCCESS;
	}
	cl_err_code clErr = m_pDevice->GetDeviceAgent()->clDevDeleteMemoryObject(m_clDevMemId);
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
	LOG_DEBUG(TEXT("Enter CreateMappedRegion(clMapFlags=%d, szNumDims=%d, szOrigins=%d, szRegions=%d, pszImageRowPitch=%d, pszImageSlicePitch=%d"),
		clMapFlags, szNumDims, szOrigins, szRegions, pszImageRowPitch, pszImageSlicePitch);

	MapParamPerPtr * pclDevCmdParamMap = NULL;
	void* pPrevMapping = NULL;
	OclAutoMutex CS(&m_oclLocker); // release on return

	// check that the region hasn't been mapped before
	map<void*, MapParamPerPtr*>::iterator it = m_mapMappedRegions.begin();
	while (it != m_mapMappedRegions.end())
	{
		pclDevCmdParamMap = it->second;
		if (pclDevCmdParamMap->dim_count == szNumDims)
		{
			if ((szNumDims == 1) && (szOrigins[0] == pclDevCmdParamMap->origin[0])
								 && (szRegions[0] == pclDevCmdParamMap->region[0]))
			{
				pPrevMapping = it->first;
				break;
			}
			if ((szNumDims == 2) && (szOrigins[0] == pclDevCmdParamMap->origin[0])
								 && (szOrigins[1] == pclDevCmdParamMap->origin[1])
								 && (szRegions[0] == pclDevCmdParamMap->region[0])
								 && (szRegions[1] == pclDevCmdParamMap->region[1]))
			{
				pPrevMapping = it->first;
				break;
			}
			if ((szNumDims == 3) && (szOrigins[0] == pclDevCmdParamMap->origin[0])
								 && (szOrigins[1] == pclDevCmdParamMap->origin[1])
								 && (szOrigins[2] == pclDevCmdParamMap->origin[2])
								 && (szRegions[0] == pclDevCmdParamMap->region[0])
								 && (szRegions[1] == pclDevCmdParamMap->region[1])
								 && (szRegions[2] == pclDevCmdParamMap->region[2]))
			{
				pPrevMapping = it->first;
				break;
			}
		}
		it++;
	}
	if (pPrevMapping)
	{
		it->second->refCount++;
		if (NULL != pszImageRowPitch)
		{
			*pszImageRowPitch = pclDevCmdParamMap->pitch[0];
		}
		if (NULL != pszImageSlicePitch)
		{
			*pszImageSlicePitch = pclDevCmdParamMap->pitch[1];
		}
		return pPrevMapping;
	}

	// create new map parameter structure and assign value to it
	pclDevCmdParamMap = new MapParamPerPtr();
	if (!pclDevCmdParamMap)
	{
		return NULL;
	}
	
	pclDevCmdParamMap->ptr = NULL;
	pclDevCmdParamMap->memObj = m_clDevMemId;
	pclDevCmdParamMap->flags = GetDevMapFlags(clMapFlags);
	assert(szNumDims <= UINT_MAX);
    pclDevCmdParamMap->dim_count = (cl_uint)szNumDims;
	pclDevCmdParamMap->refCount  = 1;
	memcpy(pclDevCmdParamMap->origin, szOrigins, sizeof(size_t) * MIN(szNumDims, 3) );
	memcpy(pclDevCmdParamMap->region, szRegions, sizeof(size_t) * MIN(szNumDims, 3) );

	cl_err_code clErr = m_pDevice->GetDeviceAgent()->clDevCreateMappedRegion(pclDevCmdParamMap);

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
	map<void*, MapParamPerPtr*>::iterator it = m_mapMappedRegions.find(pMappedPtr);
	if (it == m_mapMappedRegions.end())
	{
		return CL_INVALID_VALUE;
	}
	size_t newRef = --it->second->refCount;
	if (newRef > 0)
	{
		return CL_SUCCESS;
	}

	cl_err_code clErr = m_pDevice->GetDeviceAgent()->clDevReleaseMappedRegion(it->second);
    	delete it->second;
	m_mapMappedRegions.erase(it);

	return clErr;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// DeviceMemoryObject::GetMappedRegionInfo
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_dev_cmd_param_map* DeviceMemoryObject::GetMappedRegionInfo(void* mappedPtr)
{
	OclAutoMutex CS(&m_oclLocker); // release on return
	map<void*, MapParamPerPtr*>::iterator it = m_mapMappedRegions.find(mappedPtr);
	if (it == m_mapMappedRegions.end())
	{
		return NULL;
	}
	MapParamPerPtr * pclDevCmdParamMap = it->second;
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
MemoryObject::MemoryObject(Context * pContext, cl_mem_flags clMemFlags, ocl_entry_points * pOclEntryPoints, cl_err_code * pErr) : 
m_clMemObjectType(0),m_clFlags(clMemFlags),m_pContext(pContext),m_ppDeviceMemObjects(NULL),m_mapCount(0), m_lDataOnHost(0), m_szMemObjSize(0), m_pMemObjData(NULL) 
{
	assert ( NULL != pErr );
	assert ( NULL != m_pContext );

	INIT_LOGGER_CLIENT(L"MemoryObject", LL_DEBUG);

	*pErr = CL_SUCCESS;
	
    // Sign to be dependent on the context, ensure the context will be deleted only after the object was
    m_pContext->AddPendency();

	// assign default value
	if ( !(m_clFlags & (CL_MEM_READ_ONLY | CL_MEM_WRITE_ONLY | CL_MEM_READ_WRITE)) )
	{
		m_clFlags |= CL_MEM_READ_WRITE;
	}

	// check input memory flags
	*pErr = CheckMemFlags(m_clFlags);
	if (CL_FAILED(*pErr))
	{
		LOG_ERROR(TEXT("CheckMemFlags(%d) = %S"), m_clFlags, ClErrTxt(*pErr));
		return;
	}
	
	
	//create device memory object for each root device of devices participating in this context
	cl_uint uiNumDevices = 0;
	FissionableDevice ** ppDevices = m_pContext->GetDevices(&uiNumDevices);
	assert(uiNumDevices > 0);

    //Create a unique list of relevant root-level devices 
    Device** ppRootDevices = new Device*[uiNumDevices];
    if (NULL == ppRootDevices)
    {
        *pErr = CL_OUT_OF_HOST_MEMORY;
        return;
    }
    ppRootDevices[0] = ppDevices[0]->GetRootDevice();
    cl_uint nextRoot = 1;
    for (cl_uint i = 1; i < uiNumDevices; ++i)
    {
        Device* pDevice = ppDevices[i]->GetRootDevice();
        bool add = true;
        for (cl_uint j = 0; j < nextRoot; ++j)
        {
            if (pDevice == ppRootDevices[j])
            {
                add = false;
            }
        }
        if (add)
        {
            ppDevices[nextRoot++] = pDevice;
        }
    }

	m_szNumDevices = (size_t)nextRoot;

	m_ppDeviceMemObjects = new DeviceMemoryObject*[m_szNumDevices];
	if (!m_ppDeviceMemObjects)
	{
		*pErr = CL_OUT_OF_HOST_MEMORY;
		return;
	}

	for (cl_uint ui = 0; ui < m_szNumDevices; ++ui)
	{
		assert (NULL != ppDevices[ui]);
		m_ppDeviceMemObjects[ui] = new DeviceMemoryObject(ppRootDevices[ui], GET_LOGGER_CLIENT);
		if (NULL == m_ppDeviceMemObjects[ui])
		{
			for (cl_uint uj = 0; uj < ui; ++uj)
			{
				delete m_ppDeviceMemObjects[uj];
			}
			delete[] m_ppDeviceMemObjects;
            delete[] ppRootDevices;
			*pErr = CL_OUT_OF_HOST_MEMORY;
			return;
		}
	}
    delete[] ppRootDevices;
	m_handle.object = this;
	m_handle.dispatch = pOclEntryPoints;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// MemoryObject::addDtorNotifierCallback
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code MemoryObject::registerDtorNotifierCallback(mem_dtor_fn pfn_notify, void* pUserData)
{
	
	if (!pfn_notify)
	{
		// handle to given function is NULL
		return CL_INVALID_VALUE;
	}
	
	MemDtorNotifyData* notifyData = new MemDtorNotifyData;
	if (NULL == notifyData)
	{
		return CL_OUT_OF_HOST_MEMORY;
	}
	notifyData->first = pfn_notify;
	notifyData->second = pUserData;
	
	OclAutoMutex CS(&m_oclLocker); // release on return
	m_pfnNotifiers.push(notifyData);
	return CL_SUCCESS;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// MemoryObject D'tor
///////////////////////////////////////////////////////////////////////////////////////////////////
MemoryObject::~MemoryObject()
{
	LOG_DEBUG(TEXT("%S"), TEXT("Enter MemoryObject D'tor"));

	// Call all notifier callbacks (calling happens in reverse order)
	while (!m_pfnNotifiers.empty())
	{
		 MemDtorNotifyData* notifyData = m_pfnNotifiers.top();
		 notifyData->first(GetHandle(),notifyData->second);
		 m_pfnNotifiers.pop();
	}
    m_pContext->RemovePendency();

	if (m_ppDeviceMemObjects)
	{
		for (size_t i = 0; i < m_szNumDevices; ++i)
		{
			m_ppDeviceMemObjects[i]->Release();
			delete m_ppDeviceMemObjects[i];
		}
		delete[] m_ppDeviceMemObjects;
	}

	if (NULL != m_pMemObjData)
	{
		if (!(m_clFlags & CL_MEM_USE_HOST_PTR))
		{
			ALIGNED_FREE(m_pMemObjData);
		}
		m_pMemObjData = NULL;
	}

	RELEASE_LOGGER_CLIENT;

}
///////////////////////////////////////////////////////////////////////////////////////////////////
// MemoryObject::Initialize
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code MemoryObject::Initialize(void * pHostPtr)
{
	m_lDataOnHost = 0;
	// check invalid usage of host ptr
	if (((NULL == pHostPtr) && (m_clFlags & (CL_MEM_USE_HOST_PTR | CL_MEM_COPY_HOST_PTR)))	||
		((NULL != pHostPtr) && !(m_clFlags & (CL_MEM_USE_HOST_PTR | CL_MEM_COPY_HOST_PTR))))
	{
		LOG_ERROR(TEXT("%S"), TEXT("invalid usage of host ptr"));
		return CL_INVALID_HOST_PTR;
	}

	// save host ptr only if CL_MEM_USE_HOST_PTR is set
	m_pHostPtr = NULL;	
	if (m_clFlags & CL_MEM_USE_HOST_PTR)
	{
		m_pHostPtr = pHostPtr;

#ifdef _DEBUG
		assert(NULL != pHostPtr);
#endif
		// in case that we're using host ptr we don't need to allocated memory for the buffer
		// just use the host ptr instead
		m_pMemObjData = m_pHostPtr;
		m_lDataOnHost = 1; //true
	}
	else
	{
		m_pMemObjData = ALIGNED_MALLOC(m_szMemObjSize, CPU_MAXIMUM_ALIGN);
		if (NULL == m_pMemObjData)
		{
			return CL_OUT_OF_HOST_MEMORY;
		}
		
		if (m_clFlags & CL_MEM_COPY_HOST_PTR)
		{
#ifdef _DEBUG
			assert(NULL != pHostPtr);
#endif
			errno_t err = MEMCPY_S(m_pMemObjData, m_szMemObjSize, pHostPtr, m_szMemObjSize);
			if (err)
			{
				return CL_OUT_OF_HOST_MEMORY;
			}			
			m_pHostPtr = m_pMemObjData;
			m_lDataOnHost = 1; //true
		}
	}
	return CL_SUCCESS;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// MemoryObject::IsAllocated
///////////////////////////////////////////////////////////////////////////////////////////////////
bool MemoryObject::IsAllocated(cl_device_id clDeviceId)
{
	LOG_DEBUG(TEXT("Enter IsReady (clDeviceId=%d)"), clDeviceId);
	
	if ( (1 == m_lDataOnHost) && (0 == clDeviceId))
	{
		return true;
	}
	DeviceMemoryObject* pDevMemObj = GetDeviceMemoryObject(clDeviceId);
	if (NULL == pDevMemObj)
	{
		return false;
	}
	return pDevMemObj->IsAllocated();
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// MemoryObject::GetDeviceMemoryHndl
// If there is no resource in this device, or resource is not valid, 0 hndl is returned.
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_dev_mem MemoryObject::GetDeviceMemoryHndl( cl_device_id clDeviceId )
{
	LOG_DEBUG(TEXT("%S"), TEXT("Enter GetDeviceMemoryHndl"));
	DeviceMemoryObject* pDevMemObj = GetDeviceMemoryObject(clDeviceId);
	if (NULL == pDevMemObj)
	{
		return 0;
	}
	return pDevMemObj->GetDeviceMemoryId();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// MemoryObject::GetDataLocation
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_device_id MemoryObject::GetDataLocation(cl_device_id clDeviceId)
{
	LOG_DEBUG(TEXT("%S"), TEXT("Enter GetDataLocation"));
	
	cl_device_id devLocation = 0;
	{
		for (size_t i = 0; i < m_szNumDevices; ++i)
		{
			DeviceMemoryObject*& pDevMemObj = m_ppDeviceMemObjects[i];
			if (pDevMemObj->IsDataValid())
			{
				if (clDeviceId == pDevMemObj->GetDeviceId())
				{				
					return clDeviceId;
				}			
				else if (!devLocation)
				{	
					devLocation = pDevMemObj->GetDeviceId();
				}
			}
		}
	}

	// if found device where the memory object is valid, return it.
	// in case the mem obj is not anywhere, here we return 0 (zero)
	return devLocation;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// MemoryObject::SetDataLocation
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code MemoryObject::SetDataLocation(cl_device_id clDevice)
{
	LOG_DEBUG(TEXT("Enter SetDataLocation (clDevice=%d)"), clDevice);

	if (0 == clDevice)
	{
		m_lDataOnHost = 1;
		return CL_SUCCESS;
	}

	DeviceMemoryObject* pDevMemObj = GetDeviceMemoryObject(clDevice);
	if (NULL == pDevMemObj)
	{
		LOG_ERROR(L"Can't find device %d", clDevice);
		return CL_INVALID_DEVICE;
	}

	for (size_t i = 0; i < m_szNumDevices; ++i)
	{
		DeviceMemoryObject*& pDevMemObj = m_ppDeviceMemObjects[i];
		assert ( pDevMemObj != NULL );

		if (clDevice != pDevMemObj->GetDeviceId())
		{
			pDevMemObj->SetDataValid(false);
		}
		else
		{
			pDevMemObj->SetDataValid(true);
		}
	}
	m_lDataOnHost = 0;
	return CL_SUCCESS;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// MemoryObject::GetInfo
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code	MemoryObject::GetInfo(cl_int iParamName, size_t szParamValueSize, void * pParamValue, size_t * pszParamValueSizeRet)
{
	LOG_DEBUG(TEXT("Enter MemoryObject::GetInfo (iParamName=%d, szParamValueSize=%d, pParamValue=%d, pszParamValueSizeRet=%d)"), 
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
		szParam = m_mapCount;
		pValue  = &szParam;
		break;
	case CL_MEM_REFERENCE_COUNT:
		szSize = sizeof(cl_uint);
		pValue = &m_uiRefCount;
		break;
	case CL_MEM_CONTEXT:
		szSize = sizeof(cl_context);
		clContext = (cl_context)m_pContext->GetHandle();
		pValue = &clContext;
		break;
	case CL_MEM_ASSOCIATED_MEMOBJECT:
		{
			szSize = sizeof(cl_mem);
			cl_mem ret = NULL;
			if (GetType() == CL_MEM_OBJECT_BUFFER)
			{
				Buffer* pBuffer = reinterpret_cast<Buffer*>(this);
				if (pBuffer->IsSubBuffer())
				{
					SubBuffer* pSubBuffer = reinterpret_cast<SubBuffer*>(this);
					ret = pSubBuffer->m_pParentBuffer->GetHandle();
				}								
			}						
			pValue = &ret;			
		}
		break;
	case CL_MEM_OFFSET:
		{			
			szSize = sizeof(size_t);
			size_t ret = 0;
			if (GetType() == CL_MEM_OBJECT_BUFFER)
			{
				Buffer* pBuffer = reinterpret_cast<Buffer*>(this);
				if (pBuffer->IsSubBuffer())
				{
					SubBuffer* pSubBuffer = reinterpret_cast<SubBuffer*>(this);
					ret = pSubBuffer->m_Origin;
				}								
			}						
			pValue = &ret;
		}
		break;
	default:
		LOG_ERROR(TEXT("param_name (=%d) isn't valid"), iParamName);
		return CL_INVALID_VALUE;
	}
	if (CL_FAILED(clErrRet))
	{
		return clErrRet;
	}

	// if param_value_size < actual value size return CL_INVALID_VALUE
	if (NULL != pParamValue && szParamValueSize < szSize)
	{
		LOG_ERROR(TEXT("szParamValueSize (=%d) < szSize (=%d)"), szParamValueSize, szSize);
		return CL_INVALID_VALUE;
	}

	// return param value size
	if (NULL != pszParamValueSizeRet)
	{
		*pszParamValueSizeRet = szSize;
	}

	if (NULL != pParamValue && szSize > 0 && pValue)
	{
		MEMCPY_S(pParamValue, szParamValueSize, pValue, szSize);
	}

	return CL_SUCCESS;
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
DeviceMemoryObject* MemoryObject::GetDeviceMemoryObject(cl_device_id devId)
{
    //Todo: will need synchronization once we actually fission it

    //Translate devId to root device ID
    FissionableDevice* pDevice;
    if (CL_SUCCESS != m_pContext->GetDevice(devId, &pDevice))
    {
        return NULL;
    }
    cl_device_id rootId = pDevice->GetRootDevice()->GetHandle();

	for (size_t i = 0; i < m_szNumDevices; ++i)
	{
		if (rootId == m_ppDeviceMemObjects[i]->GetDeviceId())
		{
			return m_ppDeviceMemObjects[i];
		}
	}
	return NULL;
}

cl_err_code MemoryObject::ReleaseMappedRegion(cl_device_id clDeviceId, void* mappedPtr)
{
	LOG_DEBUG(L"Enter ReleaseMappedRegion (clDeviceId=%d, mappedPtr=%d)", clDeviceId, mappedPtr);

	DeviceMemoryObject* pDevMemObj = GetDeviceMemoryObject(clDeviceId);
	if (NULL == pDevMemObj)
	{
		return CL_DEVICE_NOT_FOUND;
	}
	--m_mapCount;
	return pDevMemObj->ReleaseMappedRegion(mappedPtr);
}

void* MemoryObject::GetMappedRegionInfo( cl_device_id clDeviceId, void* mappedPtr)
{
	LOG_DEBUG(L"Enter GetMappedRegionInfo (clDeviceId=%d, mappedPtr=%d)", clDeviceId, mappedPtr);

	DeviceMemoryObject* pDevMemObj = GetDeviceMemoryObject(clDeviceId);
	if (NULL == pDevMemObj)
	{
		return NULL;
	}
	return pDevMemObj->GetMappedRegionInfo(mappedPtr);
}

void* MemoryObject::CreateMappedRegion(cl_device_id clDeviceId, 
								 cl_map_flags clMapFlags, 
								 const size_t * szOrigins, 
								 const size_t * szRegions, 
								 size_t * pszImageRowPitch, 
								 size_t * pszImageSlicePitch)
{
	LOG_DEBUG(L"Enter CreateMappedRegion (clDeviceId=%d, cl_map_flags=%d, szOrigins=%d, szRegions=%d, pszImageRowPitch=%d, pszImageSlicePitch=%d)", 
		clDeviceId, clMapFlags, szOrigins, szRegions, pszImageRowPitch, pszImageSlicePitch);

	// get device memory object

	DeviceMemoryObject* pDevMemObj = GetDeviceMemoryObject(clDeviceId);
	if (NULL == pDevMemObj)
	{
		return NULL;
	}

	if (false == pDevMemObj->IsAllocated())
	{
		return NULL;
	}
	++m_mapCount;
	return pDevMemObj->CreateMappedRegion(clMapFlags, GetNumDimensions(), szOrigins, szRegions, pszImageRowPitch, pszImageSlicePitch);
}

cl_err_code MemoryObject::NotifyDeviceFissioned(FissionableDevice* parent, size_t count, FissionableDevice** children)
{
    //Todo: implement
    //Left empty intentionally until we implement a memory manager
    return CL_SUCCESS;
}