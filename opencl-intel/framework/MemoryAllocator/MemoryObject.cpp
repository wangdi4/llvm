// Copyright (c) 2006-2010 Intel Corporation
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
//  MemoryObject.cpp
//  Implementation of the MemoryObject Class
//  Created on:      10-Dec-2008 2:08:23 PM
//  Original author: ulevy

#include "MemoryObject.h"
#include "Context.h"

#include <cl_synch_objects.h>
#if defined (DX9_MEDIA_SHARING)
#include "d3d9_resource.h"
#endif

using namespace std;
using namespace Intel::OpenCL::Framework;
using namespace Intel::OpenCL::Utils;

MemoryObject::MemoryObject(Context * pContext, ocl_entry_points * pOclEntryPoints): OCLObject<_cl_mem_int>("MemoryObject"),
	m_pContext(pContext), m_clMemObjectType(0), m_clFlags(0),
	m_pHostPtr(NULL), m_pBackingStore(NULL), m_uiNumDim(0), m_pMemObjData(NULL), m_pParentObject(NULL),
	m_mapCount(0), m_pLocation(NULL), m_stMemObjSize(0)
{
	assert ( NULL != m_pContext );

	memset(m_stOrigin, 0, sizeof(m_stOrigin));

	m_pContext->AddPendency(this);

	m_mapMappedRegions.clear();

	m_handle.object   = this;
	m_handle.dispatch = (KHRicdVendorDispatch*)pOclEntryPoints;
}

MemoryObject::~MemoryObject()
{
	m_pContext->RemovePendency(this);
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

	OclAutoMutex CS(&m_muNotifiers); // release on return
	m_pfnNotifiers.push(notifyData);
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
	cl_mem	clMem = 0;
	const void * pValue = NULL;

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
		szSize  = sizeof(cl_uint);
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
		szSize = sizeof(cl_mem);
		pValue = &clMem;
		if ( NULL != m_pParentObject )
		{
			clMem = m_pParentObject->GetHandle();
		}
		break;
	case CL_MEM_OFFSET:
		szSize = sizeof(size_t);
		szParam = m_stOrigin[0];
		pValue = &szParam;
		break;
#if 0   // disabled until changes in the spec regarding 2D image arrays are made
    case CL_MEM_ARRAY_SIZE:
        szSize = sizeof(size_t);
        if (CL_MEM_OBJECT_IMAGE2D_ARRAY == GetType())
        {
			szParam = (dynamic_cast<IMemoryObjectArray*>(this))->GetNumObjects();
        }
        else
        {
            szParam = 0;
        }
        pValue = &szParam;
        break;
#endif
#if defined (DX9_MEDIA_SHARING)
    case CL_MEM_DX9_RESOURCE_INTEL:
        {
            const D3D9Resource* const pD3d9Resource = dynamic_cast<D3D9Resource*>(this);
            if (NULL == pD3d9Resource)
            {
                return CL_INVALID_DX9_RESOURCE_INTEL;
            }
            szSize = sizeof(IDirect3DResource9*);
            pValue = &pD3d9Resource->GetResourceInfo()->m_pResource;
            break;
        }
    case CL_MEM_DX9_SHARED_HANDLE_INTEL:
        {
            const D3D9Surface* const pD3d9Surface = dynamic_cast<D3D9Surface*>(this);
            if (NULL == pD3d9Surface)
            {
                return CL_INVALID_DX9_RESOURCE_INTEL;                
            }
            szSize = sizeof(HANDLE);
            pValue = &static_cast<const D3D9SurfaceResourceInfo*>(pD3d9Surface->GetResourceInfo())->m_sharehandle;
            break;
        }
#else
	return CL_INVALID_OPERATION;
#endif
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
// MemoryObject::NotifyDestruction
///////////////////////////////////////////////////////////////////////////////////////////////////
void MemoryObject::NotifyDestruction()
{
	// Call all notifier callbacks (calling happens in reverse order)
	while (!m_pfnNotifiers.empty())
	{
		MemDtorNotifyData* notifyData = m_pfnNotifiers.top();
		notifyData->first(GetHandle(),notifyData->second);
		m_pfnNotifiers.pop();
	}
}

cl_err_code MemoryObject::CreateMappedRegion(
	const FissionableDevice*    IN pDevice,
	cl_map_flags    IN clMapFlags,
	const size_t*   IN pOrigin,
	const size_t*   IN pRegion,
	size_t*         OUT pImageRowPitch,
	size_t*         OUT pImageSlicePitch,
	cl_dev_cmd_param_map* OUT *pMapInfo,
	void*                 OUT *pHostMapDataPtr
	)
{
	LOG_DEBUG(TEXT("Enter CreateMappedRegion(pDevice = %p)"), pDevice);

	assert(NULL != pMapInfo);

	MapParamPerPtr * pclDevCmdParamMap = NULL;
	void* pPrevMapping = NULL;

	OclAutoMutex CS(&m_muMappedRegions); // release on return

	// check if the region was mapped before
	map<void*, MapParamPerPtr*>::iterator it = m_mapMappedRegions.begin();
	while ( it != m_mapMappedRegions.end() )
	{
		pclDevCmdParamMap = it->second;
		assert( pclDevCmdParamMap->cmd_param_map.dim_count == m_uiNumDim);

		bool bMatch = true;
		for (size_t st=0; st<m_uiNumDim; ++st)
		{
			bMatch &= (pOrigin[st] == pclDevCmdParamMap->cmd_param_map.origin[st]) &&
				(pRegion[st] == pclDevCmdParamMap->cmd_param_map.region[st]);
		}
		if ( bMatch )
		{
			pPrevMapping = it->first;
			break;
		}
		it++;
	}

	//If map already exists, increase the ref counter and return the previous pointer
	if (NULL != pPrevMapping)
	{
		it->second->refCount++;
		m_mapCount++;
		if (NULL != pImageRowPitch)
		{
			*pImageRowPitch = pclDevCmdParamMap->cmd_param_map.pitch[0];
		}
		if (NULL != pImageSlicePitch)
		{
			*pImageSlicePitch = pclDevCmdParamMap->cmd_param_map.pitch[1];
		}

		*pMapInfo = &pclDevCmdParamMap->cmd_param_map;
        *pHostMapDataPtr = pPrevMapping;
		return CL_SUCCESS;
	}

	// else, create new map parameter structure and assign value to it
	pclDevCmdParamMap = new MapParamPerPtr();
	if ( NULL == pclDevCmdParamMap)
	{
		return CL_OUT_OF_HOST_MEMORY;
	}

	// Update map parameters
	pclDevCmdParamMap->cmd_param_map.dim_count = m_uiNumDim;
	MEMCPY_S(pclDevCmdParamMap->cmd_param_map.origin, sizeof(size_t[MAX_WORK_DIM]), pOrigin, sizeof(size_t)*m_uiNumDim);
	MEMCPY_S(pclDevCmdParamMap->cmd_param_map.region, sizeof(size_t[MAX_WORK_DIM]), pRegion, sizeof(size_t)*m_uiNumDim);

	cl_err_code err = MemObjCreateDevMappedRegion(pDevice, &pclDevCmdParamMap->cmd_param_map, pHostMapDataPtr);
	if (CL_FAILED(err))
	{
		return err;
	}

	pclDevCmdParamMap->refCount = 1;
	pclDevCmdParamMap->pDevice = pDevice;
	if (NULL != pImageRowPitch)
	{
		*pImageRowPitch = pclDevCmdParamMap->cmd_param_map.pitch[0];
	}
	if (NULL != pImageSlicePitch)
	{
		*pImageSlicePitch = pclDevCmdParamMap->cmd_param_map.pitch[1];
	}

	m_mapMappedRegions[*pHostMapDataPtr] = pclDevCmdParamMap;
	m_mapCount++;

	*pMapInfo = &pclDevCmdParamMap->cmd_param_map;
	return CL_SUCCESS;
}

cl_err_code MemoryObject::GetMappedRegionInfo(const FissionableDevice* IN pDevice, void* IN mappedPtr, cl_dev_cmd_param_map* OUT *pMapInfo)
{
	LOG_DEBUG(TEXT("Enter GetMappedRegionInfo (pDevice=%x, mappedPtr=%d)"), pDevice, mappedPtr);
	assert(NULL!=pMapInfo);
	OclAutoMutex CS(&m_muMappedRegions); // release on return

	// check if the region was mapped before
	map<void*, MapParamPerPtr*>::iterator it = m_mapMappedRegions.find(mappedPtr);
	if ( it == m_mapMappedRegions.end() )
	{
		return CL_INVALID_VALUE;
	}

	*pMapInfo = &(it->second->cmd_param_map);
	return CL_SUCCESS;
}
cl_err_code MemoryObject::ReleaseMappedRegion( cl_dev_cmd_param_map* IN pMapInfo, void* IN pHostMapDataPtr )
{
	LOG_DEBUG(TEXT("Enter ReleaseMappedRegion (mapInfo=%P)"), pMapInfo);

	OclAutoMutex CS(&m_muMappedRegions); // release on return

	// check if the region was mapped before
	map<void*, MapParamPerPtr*>::iterator it = m_mapMappedRegions.find(pHostMapDataPtr);
	if ( it == m_mapMappedRegions.end() )
	{
		return CL_INVALID_VALUE;
	}

	size_t newRef = --(it->second->refCount);
	if ( newRef > 0)
	{
		return CL_SUCCESS;
	}

	cl_err_code err = MemObjReleaseDevMappedRegion(it->second->pDevice, &(it->second->cmd_param_map), pHostMapDataPtr);

	delete it->second;
	m_mapMappedRegions.erase(it);

	return err;
}
