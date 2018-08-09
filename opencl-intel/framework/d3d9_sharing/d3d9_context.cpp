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

#include <cstdlib>
#include "d3d9_context.h"
#include "Device.h"
#include "cl_logger.h"
#include "d3d9_resource.h"

#pragma comment (lib, "d3dx9d.lib")
#pragma comment (lib, "d3d9.lib")
#pragma comment (lib, "d3d11.lib")

using namespace Intel::OpenCL::Utils;

namespace Intel { namespace OpenCL { namespace Framework
{

cl_err_code ParseD3DContextOptions(const std::map<cl_context_properties, cl_context_properties>& propertyMap,
    IUnknown*& device, cl_context_properties& iDevType, const ID3DSharingDefinitions*& pD3dDefs)
{
    device = nullptr;
    for (std::map<cl_context_properties, cl_context_properties>::const_iterator iter = propertyMap.begin(); iter != propertyMap.end(); iter++)
    {
        if (CL_CONTEXT_D3D9_DEVICE_INTEL == iter->first || CL_CONTEXT_D3D9EX_DEVICE_INTEL == iter->first || CL_CONTEXT_DXVA_DEVICE_INTEL == iter->first)
        {
            if (nullptr != device)
            {
                device = nullptr;
                delete pD3dDefs;
                return CL_INVALID_DX9_DEVICE_INTEL;
            }
            iDevType = iter->first;
            device = (IUnknown*)iter->second;
            pD3dDefs = new IntelD3D9Definitions(); // this will be deleted by the context when it is destroyed
            if (nullptr == pD3dDefs)
            {
                return CL_OUT_OF_HOST_MEMORY;
            }
        }
        else if (CL_CONTEXT_ADAPTER_D3D9_KHR == iter->first || CL_CONTEXT_ADAPTER_D3D9EX_KHR == iter->first || CL_CONTEXT_ADAPTER_DXVA_KHR == iter->first)
        {
            if (nullptr != device)
            {
                device = nullptr;
                delete pD3dDefs;
                return CL_INVALID_DX9_MEDIA_ADAPTER_KHR;
            }
            iDevType = iter->first;
            device = (IUnknown*)iter->second;
            pD3dDefs = new KhrD3D9Definitions(); // this will be deleted by the context when it is destroyed
            if (nullptr == pD3dDefs)
            {
                return CL_OUT_OF_HOST_MEMORY;
            }
        }
        else if (CL_CONTEXT_D3D11_DEVICE_KHR == iter->first)
        {
            if (nullptr != device)
            {
                device = nullptr;
                delete pD3dDefs;
                return CL_INVALID_D3D11_DEVICE_KHR;
            }
            iDevType = iter->first;
            device = (IUnknown*)iter->second;
            pD3dDefs = new D3D11Definitions(); // this will be deleted by the context when it is destroyed
            if (nullptr == pD3dDefs)
            {
                return CL_OUT_OF_HOST_MEMORY;
            }
        }
    }    
    return CL_SUCCESS;
}

cl_image_format MapD3DFormat2OclFormat(const D3DFORMAT d3dFormat, unsigned int uiPlane)
{
    cl_image_format clFormat = { 0, 0 };   // invalid value

    switch (d3dFormat)
    {
    case D3DFMT_R32F:
        clFormat.image_channel_order = CL_R;
        clFormat.image_channel_data_type = CL_FLOAT;
        break;
    case D3DFMT_R16F:
        clFormat.image_channel_order = CL_R;
        clFormat.image_channel_data_type = CL_HALF_FLOAT;
        break;
    case D3DFMT_L16:
        clFormat.image_channel_order = CL_R;
        clFormat.image_channel_data_type = CL_UNORM_INT16;
        break;
    case D3DFMT_A8:
        clFormat.image_channel_order = CL_A;
        clFormat.image_channel_data_type = CL_UNORM_INT8;
        break;
    case D3DFMT_L8:
        clFormat.image_channel_order = CL_R;
        clFormat.image_channel_data_type = CL_UNORM_INT8;
        break;
    case D3DFMT_G32R32F:
        clFormat.image_channel_order = CL_RG;
        clFormat.image_channel_data_type = CL_FLOAT;
        break;
    case D3DFMT_G16R16F:
        clFormat.image_channel_order = CL_RG;
        clFormat.image_channel_data_type = CL_HALF_FLOAT;
        break;
    case D3DFMT_G16R16:
        clFormat.image_channel_order = CL_RG;
        clFormat.image_channel_data_type = CL_UNORM_INT16;
        break;
    case D3DFMT_A8L8:
        clFormat.image_channel_order = CL_RG;
        clFormat.image_channel_data_type = CL_UNORM_INT8;
        break;
    case D3DFMT_A32B32G32R32F:
        clFormat.image_channel_order = CL_RGBA;
        clFormat.image_channel_data_type = CL_FLOAT;
        break;
    case D3DFMT_A16B16G16R16F:
        clFormat.image_channel_order = CL_RGBA;
        clFormat.image_channel_data_type = CL_HALF_FLOAT;
        break;
    case D3DFMT_A16B16G16R16:
        clFormat.image_channel_order = CL_RGBA;
        clFormat.image_channel_data_type = CL_UNORM_INT16;
        break;
    case D3DFMT_A8B8G8R8:
        clFormat.image_channel_order = CL_RGBA;
        clFormat.image_channel_data_type = CL_UNORM_INT8;
        break;
    case D3DFMT_X8B8G8R8:
        clFormat.image_channel_order = CL_RGBA;
        clFormat.image_channel_data_type = CL_UNORM_INT8;
        break;
    case D3DFMT_A8R8G8B8:
        clFormat.image_channel_order = CL_BGRA;
        clFormat.image_channel_data_type = CL_UNORM_INT8;
        break;
    case D3DFMT_X8R8G8B8:
        clFormat.image_channel_order = CL_BGRA;
        clFormat.image_channel_data_type = CL_UNORM_INT8;
        break;
    case MAKEFOURCC('N','V','1','2'):
        clFormat.image_channel_data_type = CL_UNORM_INT8;
        if (0 == uiPlane)
        {
            clFormat.image_channel_order = CL_R;
        }
        else if (1 == uiPlane)
        {
            clFormat.image_channel_order = CL_RG;
        }
        break;
    case MAKEFOURCC('Y','V','1','2'):
        clFormat.image_channel_order = CL_R;
        clFormat.image_channel_data_type = CL_UNORM_INT8;
        break;
	default:
		break; // prevent compilation warnings
    }
    return clFormat;
}

cl_image_format MapDxgiFormat2OclFormat(const DXGI_FORMAT dxgiFormat)
{
    static const struct {
        DXGI_FORMAT dxgiFormat;
        int iOrder;
        int iDataType;
    } dxgi2oclArr[] = {
        {DXGI_FORMAT_R32G32B32A32_FLOAT, CL_RGBA, CL_FLOAT},
        {DXGI_FORMAT_R32G32B32A32_UINT, CL_RGBA, CL_UNSIGNED_INT32},
        {DXGI_FORMAT_R32G32B32A32_SINT, CL_RGBA, CL_SIGNED_INT32},
        {DXGI_FORMAT_R16G16B16A16_FLOAT, CL_RGBA, CL_HALF_FLOAT}, 
        {DXGI_FORMAT_R16G16B16A16_UNORM, CL_RGBA, CL_UNORM_INT16},
        {DXGI_FORMAT_R16G16B16A16_UINT, CL_RGBA, CL_UNSIGNED_INT16}, 
        {DXGI_FORMAT_R16G16B16A16_SNORM, CL_RGBA, CL_SNORM_INT16},
        {DXGI_FORMAT_R16G16B16A16_SINT, CL_RGBA, CL_SIGNED_INT16}, 
        {DXGI_FORMAT_R8G8B8A8_UNORM, CL_RGBA, CL_UNORM_INT8},
        {DXGI_FORMAT_R8G8B8A8_UINT, CL_RGBA, CL_UNSIGNED_INT8}, 
        {DXGI_FORMAT_R8G8B8A8_SNORM, CL_RGBA, CL_SNORM_INT8}, 
        {DXGI_FORMAT_R8G8B8A8_SINT, CL_RGBA, CL_SIGNED_INT8}, 
        {DXGI_FORMAT_R32G32_FLOAT, CL_RG, CL_FLOAT}, 
        {DXGI_FORMAT_R32G32_UINT, CL_RG, CL_UNSIGNED_INT32},
        {DXGI_FORMAT_R32G32_SINT, CL_RG, CL_SIGNED_INT32}, 
        {DXGI_FORMAT_R16G16_FLOAT, CL_RG, CL_HALF_FLOAT},
        {DXGI_FORMAT_R16G16_UNORM, CL_RG, CL_UNORM_INT16}, 
        {DXGI_FORMAT_R16G16_UINT, CL_RG, CL_UNSIGNED_INT16},
        {DXGI_FORMAT_R16G16_SNORM, CL_RG, CL_SNORM_INT16}, 
        {DXGI_FORMAT_R16G16_SINT, CL_RG, CL_SIGNED_INT16}, 
        {DXGI_FORMAT_R8G8_UNORM, CL_RG, CL_UNORM_INT8}, 
        {DXGI_FORMAT_R8G8_UINT, CL_RG, CL_UNSIGNED_INT8}, 
        {DXGI_FORMAT_R8G8_SNORM, CL_RG, CL_SNORM_INT8}, 
        {DXGI_FORMAT_R8G8_SINT, CL_RG, CL_SIGNED_INT8}, 
        {DXGI_FORMAT_R32_FLOAT, CL_R, CL_FLOAT}, 
        {DXGI_FORMAT_R32_UINT, CL_R, CL_UNSIGNED_INT32},
        {DXGI_FORMAT_R32_SINT, CL_R, CL_SIGNED_INT32}, 
        {DXGI_FORMAT_R16_FLOAT, CL_R, CL_HALF_FLOAT}, 
        {DXGI_FORMAT_R16_UNORM, CL_R, CL_UNORM_INT16}, 
        {DXGI_FORMAT_R16_UINT, CL_R, CL_UNSIGNED_INT16}, 
        {DXGI_FORMAT_R16_SNORM, CL_R, CL_SNORM_INT16}, 
        {DXGI_FORMAT_R16_SINT, CL_R, CL_SIGNED_INT16}, 
        {DXGI_FORMAT_R8_UNORM, CL_R, CL_UNORM_INT8}, 
        {DXGI_FORMAT_R8_UINT, CL_R, CL_UNSIGNED_INT8}, 
        {DXGI_FORMAT_R8_SNORM, CL_R, CL_SNORM_INT8}, 
        {DXGI_FORMAT_R8_SINT, CL_R, CL_SIGNED_INT8}
    };    
    cl_image_format clImgFormat = {0};

    for (size_t i = 0; i < sizeof(dxgi2oclArr) / sizeof(dxgi2oclArr[0]); i++)
    {
        if (dxgi2oclArr[i].dxgiFormat == dxgiFormat)
        {
            clImgFormat.image_channel_order = dxgi2oclArr[i].iOrder;
            clImgFormat.image_channel_data_type = dxgi2oclArr[i].iDataType;
            break;
        }
    }
    return clImgFormat;
}

IDirect3DDevice9* D3D9Context::GetDevice(IDirect3DResource9* pResource) const
{
    IDirect3DDevice9* pResourceDevice;
    HRESULT res = pResource->GetDevice(&pResourceDevice);
    if (D3D_OK != res)
    {
        return nullptr;            
    }        
    return pResourceDevice;
}

cl_err_code D3D9Context::HandlePlanarSurface(D3DResourceInfo<IDirect3DResource9>* pResourceInfo, cl_mem_flags clFlags)
{
    IDirect3DSurface9* const pSurface = static_cast<IDirect3DSurface9*>(pResourceInfo->m_pResource);
    map<const IDirect3DSurface9*, SurfaceLocker*>::iterator iter = m_surfaceLockers.find(pSurface);
    SurfaceLocker* pSurfaceLocker;
    if (m_surfaceLockers.end() != iter)
    {
        pSurfaceLocker = iter->second;
        if (pSurfaceLocker->GetFlags() != D3D9Surface::GetD3D9Flags(clFlags))
        {
            LOG_ERROR(TEXT("%s"), "New D3D9Surface is to be created for an IDirect3DSurface9 for which a D3D9Surface for another plane has already been created, but the flags differ.");
            return CL_INVALID_VALUE;
        }
    }
    else
    {
        pSurfaceLocker = new SurfaceLocker(pSurface, D3D9Surface::GetD3D9Flags(clFlags));
        if (nullptr == pSurfaceLocker)
        {                
            return CL_OUT_OF_HOST_MEMORY;
        }
        m_surfaceLockers[pSurface] = pSurfaceLocker;
    }
    pSurfaceLocker->AddObject();
    return CL_SUCCESS;
}

void D3D9Context::ReleaseSurfaceLocker(const IDirect3DSurface9* pSurface)
{
    LockMutex();
    map<const IDirect3DSurface9*, SurfaceLocker*>::iterator iter = m_surfaceLockers.find(pSurface);
    assert(iter != m_surfaceLockers.end());
    iter->second->RemoveObject();
    if (0 == iter->second->GetNumObjects())
    {
        delete iter->second;
        m_surfaceLockers.erase(pSurface);            
    }
    UnlockMutex();
}

ID3D11Device* D3D11Context::GetDevice(ID3D11Resource* pResource) const
{
    ID3D11Device* pResourceDevice;
    pResource->GetDevice(&pResourceDevice);
    return pResourceDevice;
}

cl_err_code	D3D11Context::GetInfo(cl_int param_name, size_t param_value_size, void * param_value, size_t * param_value_size_ret) const
{
    if (CL_CONTEXT_D3D11_PREFER_SHARED_RESOURCES_KHR == (cl_context_info)param_name)
    {
        if (param_value != nullptr && param_value_size < sizeof(cl_bool))
        {
            return CL_INVALID_VALUE;
        }
        if (param_value != nullptr)
        {
            *(cl_bool*)param_value = CL_FALSE;
        }            
        if (param_value_size_ret != nullptr)
        {
            *param_value_size_ret = sizeof(cl_bool);
        }
        return CL_SUCCESS;
    }
    else
    {
        return D3DContext<ID3D11Resource, ID3D11Device>::GetInfo(param_name, param_value_size, param_value, param_value_size_ret);
    }
}

}}}
