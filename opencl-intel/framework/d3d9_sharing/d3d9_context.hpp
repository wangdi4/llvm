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

#include "d3d9_context.h"
#include "d3d9_resource.h"

#define CL_DX9_OBJECT_SURFACE     0x3002

namespace Intel { namespace OpenCL { namespace Framework
{

template<typename RESOURCE_TYPE, typename DEV_TYPE>
D3DContext<RESOURCE_TYPE, DEV_TYPE>::D3DContext(const cl_context_properties* clProperties, cl_uint uiNumDevices,
    cl_uint uiNumRootDevices, SharedPtr<FissionableDevice>* ppDevices, logging_fn pfnNotify,
    void* pUserData, cl_err_code* pclErr, ocl_entry_points* pOclEntryPoints,
    ocl_gpa_data* pGPAData, IUnknown* const pD3D9Device, cl_context_properties iDevType, const ID3DSharingDefinitions* pd3d9Definitions,
    ContextModule& contextModule, bool bIsInteropUserSync) :
Context(clProperties, uiNumDevices, uiNumRootDevices, ppDevices, pfnNotify, pUserData,
    pclErr, pOclEntryPoints, pGPAData, contextModule),
    m_pD3DDevice(pD3D9Device), m_iDeviceType(iDevType), m_bIsInteropUserSync(bIsInteropUserSync), m_pd3dDefinitions(pd3d9Definitions)
{        
    /* The spec states that we should return "CL_INVALID_D3D9_DEVICE_Intel if the value of the property CL_CONTEXT_D3D9_DEVICE_Intel is non-NULL and does not specify a valid
    Direct3D 9 device with which the cl_device_ids against which this context is to be created may interoperate." However, I don't know how to check the validity of the
    device. I guess a non-valid device will cause a segmentation fault if one of its methods are called and we don't want that. */

    // The following is disabled since media_sharing from OCL1.2::extensions  doesn't
    // require that we should Inc/Dec reference count of the D3D9 interface, in contrary
    // to whats required for D3D10/11. Besides AddRef/Release of D3DDevice is causing D3D9
    // mem leaks, probably due to driver bug.

    //m_pD3DDevice->AddRef();

    for (cl_uint i = 0; i < m_pOriginalNumDevices; i++)
    {
        m_ppAllDevices[i]->SetD3DDevice(pD3D9Device, iDevType);
    }
}

template<typename RESOURCE_TYPE, typename DEV_TYPE>
D3DContext<RESOURCE_TYPE, DEV_TYPE>::~D3DContext()
{
    // The following is disabled since media_sharing from OCL1.2::extensions  doesn't
    // require that we should Inc/Dec reference count of the D3D9 interface, in contrary
    // to whats required for D3D10/11. Besides AddRef/Release of D3DDevice is causing D3D9

    //m_pD3DDevice->Release();

    for (cl_uint i = 0; i < m_pOriginalNumDevices; i++)
    {
        m_ppAllDevices[i]->SetD3DDevice(nullptr, 0);
    }
    delete m_pd3dDefinitions;
}

template<typename RESOURCE_TYPE, typename DEV_TYPE>
cl_err_code D3DContext<RESOURCE_TYPE, DEV_TYPE>::CreateD3DResource(cl_mem_flags clFlags, D3DResourceInfo<RESOURCE_TYPE>* const pResourceInfo, SharedPtr<MemoryObject>* const ppMemObj,
                                                                    cl_mem_object_type clObjType, cl_uint uiDimCnt, UINT plane)
{
    Intel::OpenCL::Utils::OclAutoMutex mtx(&m_mutex, true);

    if (m_resourceInfoSet.find(pResourceInfo) != m_resourceInfoSet.end())
    {
        LOG_ERROR(TEXT("%s"), "A cl_mem from this D3DResourceInfo has already been created");
        delete pResourceInfo;
        return m_pd3dDefinitions->GetInvalidResource();    // Khronos DX9 Media Sharing doesn't say anything about this, but the other 2 spec do.
    }
    assert(nullptr != ppMemObj);
    SharedPtr<MemoryObject> pMemObj;

    cl_err_code clErr = MemoryObjectFactory::GetInstance()->CreateMemoryObject(m_devTypeMask, clObjType, m_pd3dDefinitions->GetGfxSysSharing(), this, &pMemObj);
    if (CL_FAILED(clErr))
    {
        LOG_ERROR(TEXT("Error creating new D3DResource, returned: %s"), ClErrTxt(clErr));
        delete pResourceInfo;
        return clErr;
    }
    
    D3DResource<RESOURCE_TYPE, DEV_TYPE>& d3d9Resource = *pMemObj.DynamicCast<D3DResource<RESOURCE_TYPE, DEV_TYPE>>();
    if (!d3d9Resource.IsValidlyCreated(*pResourceInfo))
    {
        LOG_ERROR(TEXT("%s"), "resource is not a Direct3D 9 resource created in D3DPOOL_DEFAULT");
        d3d9Resource.Release();
        return m_pd3dDefinitions->GetInvalidResource();
    }
    if (CL_DX9_OBJECT_SURFACE == clObjType && MAXUINT != plane)
    {
        clErr = HandlePlanarSurface(pResourceInfo, clFlags);
        if (CL_SUCCESS != clErr)
        {
            d3d9Resource.Release();
            return clErr;
        }
    }
    size_t dims[3];
    d3d9Resource.FillDimensions(*pResourceInfo, dims);
    const cl_image_format clFormat = d3d9Resource.GetClImageFormat(*pResourceInfo);
    if (clFormat.image_channel_order != 0 || clFormat.image_channel_data_type != 0)
    {
        if (!(clFlags & CL_MEM_WRITE_ONLY))
        {
            clErr = CheckSupportedImageFormat(&clFormat, CL_MEM_READ_ONLY, d3d9Resource.GetChildMemObjectType());
        }
        if (CL_SUCCESS != clErr)
        {
            d3d9Resource.Release();
            return clErr;
        }
        if (!(clFlags & CL_MEM_READ_ONLY))
        {
            clErr = CheckSupportedImageFormat(&clFormat, CL_MEM_WRITE_ONLY, d3d9Resource.GetChildMemObjectType());
        }        
        if (CL_SUCCESS != clErr)
        {
            d3d9Resource.Release();
            return clErr;
        }
        clErr = d3d9Resource.Initialize(clFlags, &clFormat, uiDimCnt, dims, nullptr, pResourceInfo, 0);
    }
    else
    {
        clErr = d3d9Resource.Initialize(clFlags, nullptr, uiDimCnt, dims, nullptr, pResourceInfo, 0);
    }
    // resourceInfo will be deleted in pD3D9Resource's destructor    
    if (CL_FAILED(clErr))
    {
        LOG_ERROR(TEXT("Failed to initialize data, pD3D9Resource->Initialize(pHostPtr = %s"),
            ClErrTxt(clErr));
        d3d9Resource.Release();
        return clErr;
    }
    m_resourceInfoSet.insert(pResourceInfo);
    m_mapMemObjects.AddObject(pMemObj);
    *ppMemObj = pMemObj;
    return CL_SUCCESS;
}

template<typename RESOURCE_TYPE, typename DEV_TYPE>
bool D3DContext<RESOURCE_TYPE, DEV_TYPE>::D3DResourceInfoComparator::operator()(
    const D3DResourceInfo<RESOURCE_TYPE>* left,
    const D3DResourceInfo<RESOURCE_TYPE>* right) const
{
    if (typeid(*left) != typeid(*right))
        return &typeid(*left) < &typeid(*right);
    return *left < *right;
}

}}}
