// INTEL CONFIDENTIAL
//
// Copyright 2006-2022 Intel Corporation.
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

#include "MemoryObject.h"
#include "Context.h"
#include "cl_shared_ptr.hpp"
#include "cl_user_logger.h"
#include "context_module.h"

#if defined(DX_MEDIA_SHARING)
#include "d3d9_resource.h"
#endif

using namespace std;
using namespace Intel::OpenCL::Framework;
using namespace Intel::OpenCL::Utils;

MemoryObject::MemoryObject(SharedPtr<Context> pContext)
    : OCLObject<_cl_mem_int>(pContext.GetPtr() != NULL ? pContext->GetHandle()
                                                       : NULL,
                             "MemoryObject"),
      m_pContext(pContext), m_clMemObjectType(0), m_clFlags(0),
      m_pHostPtr(NULL), m_pBackingStore(NULL), m_uiNumDim(0),
      m_pMemObjData(NULL), m_pParentObject(NULL), m_mapCount(0),
      m_pMappedDevice(NULL), m_stMemObjSize(0),
      m_bRegisteredInContextModule(false) {
  memset(m_stOrigin, 0, sizeof(m_stOrigin));

  m_mapMappedRegions.clear();
}

MemoryObject::~MemoryObject() {}

///////////////////////////////////////////////////////////////////////////////////////////////////
// MemoryObject::addDtorNotifierCallback
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code MemoryObject::registerDtorNotifierCallback(mem_dtor_fn pfn_notify,
                                                       void *pUserData) {

  if (!pfn_notify) {
    // handle to given function is NULL
    return CL_INVALID_VALUE;
  }

  MemDtorNotifyData *notifyData = new MemDtorNotifyData;
  if (NULL == notifyData) {
    return CL_OUT_OF_HOST_MEMORY;
  }

  notifyData->first = pfn_notify;
  notifyData->second = pUserData;

  std::lock_guard<std::recursive_mutex> CS(m_muNotifiers);
  m_pfnNotifiers.push(notifyData);
  return CL_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// MemoryObject::GetInfo
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code MemoryObject::GetInfo(cl_int iParamName, size_t szParamValueSize,
                                  void *pParamValue,
                                  size_t *pszParamValueSizeRet) const {
  LOG_DEBUG(
      TEXT("Enter MemoryObject::GetInfo (iParamName=%d, szParamValueSize=%zu, "
           "pParamValue=%p, pszParamValueSizeRet=%p)"),
      iParamName, szParamValueSize, pParamValue, pszParamValueSizeRet);

  size_t szSize = 0;
  size_t szParam = 0;
  cl_bool bParam = CL_FALSE;
  cl_context clContext = 0;
  cl_mem clMem = 0;
  const void *pValue = NULL;
#if defined(DX_MEDIA_SHARING)
  cl_dx9_surface_info_khr dx9SurfaceInfo;
#endif

  switch ((cl_mem_info)iParamName) {
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
    szSize = sizeof(void *);
    pValue = &m_pHostPtr;
    break;
  case CL_MEM_MAP_COUNT:
    szSize = sizeof(cl_uint);
    szParam = m_mapCount;
    pValue = &szParam;
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
    if (NULL != m_pParentObject.GetPtr()) {
      clMem = m_pParentObject->GetHandle();
    }
    break;
  case CL_MEM_OFFSET:
    szSize = sizeof(size_t);
    szParam = m_stOrigin[0];
    pValue = &szParam;
    break;
  case CL_MEM_USES_SVM_POINTER:
    szSize = sizeof(cl_bool);
    if (m_pContext->IsSVMPointer(m_pHostPtr)) {
      bParam = CL_TRUE;
    } else {
      bParam = CL_FALSE;
    }
    pValue = &bParam;
    break;
  case CL_MEM_PROPERTIES:
    szSize = sizeof(cl_mem_properties) * m_clMemobjPropArrays.size();
    pValue = m_clMemobjPropArrays.data();
    break;
  case CL_MEM_ALLOC_BASE_PTR_INTEL:
    // CL_MEM_ALLOC_BASE_PTR_INTEL is borrowed from usm spec. Here is an
    // internal customization which allows to get MemObj base ptr. This is
    // for testing purpose only.
    szSize = sizeof(m_pMemObjData);
    pValue = &m_pMemObjData;
    break;
#if defined(DX_MEDIA_SHARING)
  /* We handle the following values here and not in D3DResource, because it is
     required to return CL_INVALID_DX9_RESOURCE_INTEL in case the object is not
     a Direct3D shared object and not CL_INVALID_VALUE. */
  /* We handle the following values here and not in D3D9Resource, because it is
     required to return CL_INVALID_DX9_RESOURCE_INTEL in case the object is not
     a Direct3D shared object and not CL_INVALID_VALUE. */
  case CL_MEM_DX9_RESOURCE_INTEL: {
    const D3DResource<IDirect3DResource9,
                      IDirect3DDevice9> *const pD3d9Resource =
        dynamic_cast<const D3DResource<IDirect3DResource9, IDirect3DDevice9> *>(
            this);
    if (NULL == pD3d9Resource) {
      return CL_INVALID_DX9_RESOURCE_INTEL;
    }
    const D3D9Context &d3d9Context =
        *pD3d9Resource->GetContext().DynamicCast<D3D9Context>();
    if (d3d9Context.GetD3dDefinitions().GetVersion() !=
        ID3DSharingDefinitions::D3D9_INTEL) {
      return CL_INVALID_DX9_RESOURCE_INTEL;
    }
    szSize = sizeof(IDirect3DResource9 *);
    pValue = &pD3d9Resource->GetResourceInfo()->m_pResource;
    break;
  }
  case CL_MEM_D3D11_RESOURCE_KHR: {
    const D3DResource<ID3D11Resource, ID3D11Device> *const pD3d11Resource =
        dynamic_cast<const D3DResource<ID3D11Resource, ID3D11Device> *>(this);
    if (NULL == pD3d11Resource) {
      return CL_INVALID_D3D11_RESOURCE_KHR;
    }
    szSize = sizeof(ID3D11Resource *);
    pValue = &pD3d11Resource->GetResourceInfo()->m_pResource;
    break;
  }
  case CL_MEM_DX9_SHARED_HANDLE_INTEL: {
    const D3D9Surface *const pD3d9Surface =
        dynamic_cast<const D3D9Surface *>(this);
    if (NULL == pD3d9Surface ||
        pD3d9Surface->GetContext()
                .DynamicCast<D3D9Context>()
                ->GetD3dDefinitions()
                .GetVersion() != ID3DSharingDefinitions::D3D9_INTEL) {
      return CL_INVALID_DX9_RESOURCE_INTEL;
    }
    szSize = sizeof(HANDLE);
    pValue = &static_cast<const D3D9SurfaceResourceInfo *>(
                  pD3d9Surface->GetResourceInfo())
                  ->m_sharehandle;
    break;
  }
  case CL_MEM_DX9_MEDIA_ADAPTER_TYPE_KHR: {
    const D3D9Surface *const pD3d9Surface =
        dynamic_cast<const D3D9Surface *>(this);
    if (NULL == pD3d9Surface ||
        pD3d9Surface->GetContext()
                .DynamicCast<D3D9Context>()
                ->GetD3dDefinitions()
                .GetVersion() != ID3DSharingDefinitions::D3D9_KHR) {
      return CL_INVALID_DX9_MEDIA_SURFACE_KHR;
    }
    szSize = sizeof(cl_dx9_media_adapter_type_khr);
    pValue = &static_cast<const D3D9SurfaceResourceInfo *>(
                  pD3d9Surface->GetResourceInfo())
                  ->m_adapterType;
    break;
  }
  case CL_MEM_DX9_MEDIA_SURFACE_INFO_KHR: {
    const D3D9Surface *const pD3d9Surface =
        dynamic_cast<const D3D9Surface *>(this);
    if (NULL == pD3d9Surface ||
        pD3d9Surface->GetContext()
                .DynamicCast<D3D9Context>()
                ->GetD3dDefinitions()
                .GetVersion() != ID3DSharingDefinitions::D3D9_KHR) {
      return CL_INVALID_DX9_MEDIA_SURFACE_KHR;
    }

    const D3D9SurfaceResourceInfo *const d3d9SurfaceResourceInfo =
        static_cast<const D3D9SurfaceResourceInfo *>(
            pD3d9Surface->GetResourceInfo());
    szSize = sizeof(dx9SurfaceInfo);
    dx9SurfaceInfo.resource =
        static_cast<IDirect3DSurface9 *>(d3d9SurfaceResourceInfo->m_pResource);
    dx9SurfaceInfo.shared_handle = d3d9SurfaceResourceInfo->m_sharehandle;
    pValue = &dx9SurfaceInfo;
    break;
  }
#else
    return CL_INVALID_OPERATION;
#endif
  default:
    LOG_ERROR(TEXT("param_name (=%d) isn't valid"), iParamName);
    return CL_INVALID_VALUE;
  }

  // if param_value_size < actual value size return CL_INVALID_VALUE
  if (NULL != pParamValue && szParamValueSize < szSize) {
    LOG_ERROR(TEXT("szParamValueSize (=%zu) < szSize (=%zu)"), szParamValueSize,
              szSize);
    return CL_INVALID_VALUE;
  }

  // return param value size
  if (NULL != pszParamValueSizeRet) {
    *pszParamValueSizeRet = szSize;
  }

  if (NULL != pParamValue && szSize > 0 && pValue) {
    MEMCPY_S(pParamValue, szParamValueSize, pValue, szSize);
  }

  return CL_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// MemoryObject::NotifyDestruction
///////////////////////////////////////////////////////////////////////////////////////////////////
void MemoryObject::NotifyDestruction() {
  cl_mem myHandle = GetHandle();
  // Call all notifier callbacks (calling happens in reverse order)
  // Memory object was already destructed - do not touch it except of notifiers
  // list
  while (!m_pfnNotifiers.empty()) {
    MemDtorNotifyData *notifyData = m_pfnNotifiers.top();
    if (FrameworkUserLogger::GetInstance()->IsApiLoggingEnabled()) {
      std::stringstream stream;
      stream << "MemObjectDestructorCallback(" << static_cast<void *>(myHandle)
             << ", " << notifyData->second << ")" << std::endl;
      FrameworkUserLogger::GetInstance()->PrintString(stream.str());
    }
    notifyData->first(myHandle, notifyData->second);
    m_pfnNotifiers.pop();
    delete notifyData;
  }
}

static void AssignPitches(const MemoryObject &memObj, size_t *pImageRowPitch,
                          size_t *pImageSlicePitch,
                          const MapParamPerPtr &clDevCmdParamMap) {
  if (NULL != pImageRowPitch) {
    *pImageRowPitch = clDevCmdParamMap.cmd_param_map
                          .pitch[0]; // for 1D image arrays *pImageRowPitch and
                                     // *pImageSlicePitch hold the same value
  }
  // image_slice_pitch returns the size in bytes of each 2D slice of a 3D image
  // or the size of each 1D or 2D image in a 1D or 2D image array for the mapped
  // region
  if (NULL != pImageSlicePitch) {
    if (memObj.GetType() != CL_MEM_OBJECT_IMAGE1D_ARRAY) {
      *pImageSlicePitch = clDevCmdParamMap.cmd_param_map.pitch[1];
    } else {
      *pImageSlicePitch = clDevCmdParamMap.cmd_param_map.pitch[0];
    }
  }
}

cl_err_code MemoryObject::CreateMappedRegion(
    SharedPtr<FissionableDevice> IN pDevice, cl_map_flags IN clMapFlags,
    const size_t *IN pOrigin, const size_t *IN pRegion,
    size_t *OUT pImageRowPitch, size_t *OUT pImageSlicePitch,
    cl_dev_cmd_param_map *OUT *pMapInfo, void *OUT *pHostMapDataPtr,
    ConstSharedPtr<FissionableDevice> OUT *pActualMappingDevice) {
  LOG_DEBUG(TEXT("Enter CreateMappedRegion(pDevice = %p)"), pDevice.GetPtr());

  assert(NULL != pMapInfo);

  MapParamPerPtr *pclDevCmdParamMap = NULL;
  void *pPrevMapping = NULL;

  std::lock_guard<std::recursive_mutex> CS(m_muMappedRegions);

  // check if the region was mapped before
  Addr2MapRegionMultiMap::iterator it = m_mapMappedRegions.begin();
  for (; it != m_mapMappedRegions.end(); ++it) {
    pclDevCmdParamMap = it->second;

    if (pclDevCmdParamMap->refCount == 0) {
      continue;
    }

    assert(pclDevCmdParamMap->cmd_param_map.dim_count == m_uiNumDim);

    bool bMatch = true;
    for (size_t st = 0; st < m_uiNumDim; ++st) {
      bMatch &= (pOrigin[st] == pclDevCmdParamMap->cmd_param_map.origin[st]) &&
                (pRegion[st] == pclDevCmdParamMap->cmd_param_map.region[st]) &&
                (clMapFlags == pclDevCmdParamMap->cmd_param_map.flags);
    }
    if (bMatch) {
      pPrevMapping = it->first;
      break;
    }
  }

  bool full_overwrite = false;
  if (CL_MAP_WRITE_INVALIDATE_REGION & clMapFlags) {
    full_overwrite = IsWholeObjectCovered(m_uiNumDim, pOrigin, pRegion);
  }

  // If map already exists, increase the ref counter and return the previous
  // pointer
  if (NULL != pPrevMapping) {
    pclDevCmdParamMap->refCount++;
    m_mapCount++;
    AssignPitches(*this, pImageRowPitch, pImageSlicePitch, *pclDevCmdParamMap);

    // it is possible that saved map flags and new one are different - merge
    pclDevCmdParamMap->cmd_param_map.flags |= clMapFlags;
    pclDevCmdParamMap->full_object_ovewrite =
        pclDevCmdParamMap->full_object_ovewrite || full_overwrite;

    *pMapInfo = &pclDevCmdParamMap->cmd_param_map;
    *pHostMapDataPtr = pPrevMapping;
    *pActualMappingDevice = m_pMappedDevice;
    return CL_SUCCESS;
  }

  // else, create new map parameter structure and assign value to it
  pclDevCmdParamMap = new MapParamPerPtr(this);
  if (NULL == pclDevCmdParamMap) {
    return CL_OUT_OF_HOST_MEMORY;
  }

  // Update map parameters
  pclDevCmdParamMap->cmd_param_map.flags = clMapFlags;
  pclDevCmdParamMap->cmd_param_map.dim_count = m_uiNumDim;
  MEMCPY_S(pclDevCmdParamMap->cmd_param_map.origin,
           sizeof(size_t[MAX_WORK_DIM]), pOrigin, sizeof(size_t) * m_uiNumDim);
  MEMCPY_S(pclDevCmdParamMap->cmd_param_map.region,
           sizeof(size_t[MAX_WORK_DIM]), pRegion, sizeof(size_t) * m_uiNumDim);
  pclDevCmdParamMap->cmd_param_map.flags = clMapFlags;
  pclDevCmdParamMap->full_object_ovewrite = full_overwrite;

  assert(((0 == m_mapCount) == (NULL == m_pMappedDevice.GetPtr())) &&
         "m_mapCount and m_pMappedDevice must be both either 0 or not");

  SharedPtr<FissionableDevice> device_to_map =
      (NULL != m_pMappedDevice.GetPtr()) ? m_pMappedDevice : pDevice;

  cl_err_code err = MemObjCreateDevMappedRegion(
      device_to_map, &pclDevCmdParamMap->cmd_param_map, pHostMapDataPtr);
  if (CL_FAILED(err)) {
    delete pclDevCmdParamMap;
    return err;
  }

  pclDevCmdParamMap->refCount = 1;
  AssignPitches(*this, pImageRowPitch, pImageSlicePitch, *pclDevCmdParamMap);

  m_mapMappedRegions.insert(
      Addr2MapRegionMultiMap::value_type(*pHostMapDataPtr, pclDevCmdParamMap));
  m_mapCount++;
  m_pMappedDevice = device_to_map;

  *pMapInfo = &pclDevCmdParamMap->cmd_param_map;
  *pActualMappingDevice = m_pMappedDevice;

  if (!m_bRegisteredInContextModule) {
    m_bRegisteredInContextModule = true;
    m_pContext->GetContextModule().RegisterMappedMemoryObject(this);
  }
  return CL_SUCCESS;
}

cl_err_code MemoryObject::GetMappedRegionInfo(
    ConstSharedPtr<FissionableDevice> IN pDevice, void *IN mappedPtr,
    cl_dev_cmd_param_map *OUT *pMapInfo,
    ConstSharedPtr<FissionableDevice> OUT *pMappedOnDevice,
    bool OUT *pbWasFullyOverwritten, bool invalidateRegion) {
  LOG_DEBUG(TEXT("Enter GetMappedRegionInfo (pDevice=%p, mappedPtr=%p)"),
            pDevice.GetPtr(), mappedPtr);
  assert(NULL != pMapInfo);
  assert(NULL != pMappedOnDevice);
  std::lock_guard<std::recursive_mutex> CS(m_muMappedRegions);

  // try to find a region that hasn't yet been invalidated
  Addr2MapRegionMultiMap::iterator it = m_mapMappedRegions.find(mappedPtr);
  bool found = false;
  for (; it != m_mapMappedRegions.end(); ++it) {
    // In the multimap find() returns pointer to the first element with a given
    // key. All elements with the same key are sequential, so we should just
    // iterate until we encounter first element with different key
    if (it->first != mappedPtr) {
      break;
    }

    MapParamPerPtr *info = it->second;
    if (info->refCount > 0) {
      found = true;
      *pMapInfo = &(info->cmd_param_map);

      assert(NULL != m_pMappedDevice.GetPtr());
      *pMappedOnDevice = m_pMappedDevice;

      if (pbWasFullyOverwritten) {
        *pbWasFullyOverwritten = info->full_object_ovewrite;
      }
      if (invalidateRegion) {
        assert(info->refCount > 0);
        --info->refCount;
        ++info->invalidateRefCount;
      }
      break;
    }
  }

  return (found ? CL_SUCCESS : CL_INVALID_VALUE);
}

cl_err_code
MemoryObject::UndoMappedRegionInvalidation(cl_dev_cmd_param_map *IN pMapInfo) {
  assert(NULL != pMapInfo);
  std::lock_guard<std::recursive_mutex> CS(m_muMappedRegions);

  Addr2MapRegionMultiMap::iterator it = m_mapMappedRegions.find(pMapInfo->ptr);
  MapParamPerPtr *info = NULL;
  for (; it != m_mapMappedRegions.end(); ++it) {
    // In the multimap find() returns pointer to the first element with a given
    // key. All elements with the same key are sequential, so we should just
    // iterate until we encounter first element with different key
    if (it->first != pMapInfo->ptr) {
      break;
    }

    if (pMapInfo == &(it->second->cmd_param_map)) {
      info = it->second;
      break;
    }
  }

  if (NULL == info) {
    assert(false && "Map region to undo invalidation was not found");
    return CL_INVALID_VALUE;
  }

  assert(info->invalidateRefCount > 0);

  ++info->refCount;
  --info->invalidateRefCount;

  return CL_SUCCESS;
}

cl_err_code MemoryObject::ReleaseMappedRegion(cl_dev_cmd_param_map *IN pMapInfo,
                                              void *IN pHostMapDataPtr,
                                              bool invalidatedBefore) {
  LOG_DEBUG(TEXT("Enter ReleaseMappedRegion (mapInfo=%p)"), pMapInfo);

  std::lock_guard<std::recursive_mutex> CS(m_muMappedRegions);

  // check if the region was mapped before
  Addr2MapRegionMultiMap::iterator it =
      m_mapMappedRegions.find(pHostMapDataPtr);
  MapParamPerPtr *info = NULL;
  for (; it != m_mapMappedRegions.end(); ++it) {
    // In the multimap find() returns pointer to the first element with a given
    // key. All elements with the same key are sequential, so we should just
    // iterate until we encounter first element with different key
    if (it->first != pHostMapDataPtr) {
      break;
    }

    if (pMapInfo == &(it->second->cmd_param_map)) {
      info = it->second;
      break;
    }
  }

  if (NULL == info) {
    return CL_INVALID_VALUE;
  }

  assert(m_mapCount >= 1);
  --m_mapCount;

  if (invalidatedBefore) {
    assert(info->invalidateRefCount > 0);
    --info->invalidateRefCount;
  } else {
    assert(info->refCount > 0);
    --info->refCount;
  }

  if ((info->refCount > 0) || (info->invalidateRefCount > 0)) {
    return CL_SUCCESS;
  }

  cl_err_code err = MemObjReleaseDevMappedRegion(
      m_pMappedDevice, &(info->cmd_param_map), pHostMapDataPtr);

  delete info;
  m_mapMappedRegions.erase(it);

  if (0 == m_mapCount) {
    m_pMappedDevice = NULL;
  }

  return err;
}

void MemoryObject::ReleaseAllMappedRegions() {
  LOG_DEBUG(TEXT("Enter ReleaseAllMappedRegions"));

  std::lock_guard<std::recursive_mutex> CS(m_muMappedRegions);

  if (0 == m_mapCount) {
    return;
  }

  Addr2MapRegionMultiMap::iterator it = m_mapMappedRegions.begin();
  Addr2MapRegionMultiMap::iterator it_end = m_mapMappedRegions.end();

  for (; it != it_end; ++it) {
    MapParamPerPtr *info = it->second;
    void *pHostPtr = it->first;

    if (NULL != info) {
      MemObjReleaseDevMappedRegion(m_pMappedDevice, &(info->cmd_param_map),
                                   pHostPtr, true);
      delete info;
    }
  }

  m_mapMappedRegions.clear();
  m_mapCount = 0;
}

int MemoryObject::ValidateChildFlags(const cl_mem_flags childFlags) {
  cl_mem_flags parentFlags = GetFlags();

  // Read/Write only
  if ((parentFlags & CL_MEM_READ_ONLY) &&
      (childFlags & (CL_MEM_WRITE_ONLY | CL_MEM_READ_WRITE))) {
    return CL_INVALID_VALUE;
  }
  if ((parentFlags & CL_MEM_WRITE_ONLY) &&
      (childFlags & (CL_MEM_READ_ONLY | CL_MEM_READ_WRITE))) {
    return CL_INVALID_VALUE;
  }

  // host read/write access
  if ((parentFlags & CL_MEM_HOST_NO_ACCESS) &&
      (childFlags & (CL_MEM_HOST_WRITE_ONLY | CL_MEM_HOST_READ_ONLY))) {
    return CL_INVALID_VALUE;
  }
  if ((parentFlags & CL_MEM_HOST_WRITE_ONLY) &&
      (childFlags & CL_MEM_HOST_READ_ONLY)) {
    return CL_INVALID_VALUE;
  }
  if ((parentFlags & CL_MEM_HOST_READ_ONLY) &&
      (childFlags & CL_MEM_HOST_WRITE_ONLY)) {
    return CL_INVALID_VALUE;
  }

  // host ptr
  if (childFlags &
      (CL_MEM_USE_HOST_PTR | CL_MEM_ALLOC_HOST_PTR | CL_MEM_COPY_HOST_PTR)) {
    return CL_INVALID_VALUE;
  }

  return CL_SUCCESS;
}

int MemoryObject::ValidateMapFlags(const cl_mem_flags mapFlags) {
  cl_mem_flags pflags = GetFlags();

  if ((mapFlags & CL_MAP_READ) &&
      (pflags & (CL_MEM_HOST_NO_ACCESS | CL_MEM_HOST_WRITE_ONLY))) {
    return CL_INVALID_VALUE;
  }

  if ((mapFlags & (CL_MAP_WRITE | CL_MAP_WRITE_INVALIDATE_REGION)) &&
      (pflags & (CL_MEM_HOST_NO_ACCESS | CL_MEM_HOST_READ_ONLY))) {
    return CL_INVALID_VALUE;
  }

  return CL_SUCCESS;
}

// Check that whole object is covered.
bool MemoryObject::IsWholeObjectCovered(cl_uint dims, const size_t *pszOrigin,
                                        const size_t *pszRegion) {
  size_t sizes[MAX_WORK_DIM];

  assert(pszOrigin && pszRegion);

  if (GetNumDimensions() != dims) {
    return false;
  }

  if (CL_SUCCESS != GetDimensionSizes(sizes)) {
    assert(false && "GetDimensionSizes(sizes) returned non-success");
    return false;
  }

  for (cl_uint i = 0; i < dims; ++i) {
    if ((pszOrigin[i] > 0) || (pszRegion[i] < sizes[i])) {
      return false;
    }
  }

  return true;
}

/******************************************************************
 * This object may hold an extra reference count as it is recorded in
 *ContextModule
 ******************************************************************/
void MemoryObject::EnterZombieState(EnterZombieStateLevel /*call_level*/) {
  if (m_bRegisteredInContextModule) {
    m_pContext->GetContextModule().UnRegisterMappedMemoryObject(
        const_cast<MemoryObject *>(this));
  }
  OCLObject<_cl_mem_int>::EnterZombieState(RECURSIVE_CALL);
}

// set the properties of the memory object
void MemoryObject::SetProperties(
    std::vector<cl_mem_properties> &clMemObjPropsArray) {
  if (clMemObjPropsArray.empty())
    return;
  m_clMemobjPropArrays.swap(clMemObjPropsArray);
}
