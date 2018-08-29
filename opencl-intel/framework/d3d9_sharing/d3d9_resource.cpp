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

#include "d3d9_resource.h"
#include "cl_logger.h"
#include "cl_shared_ptr.hpp"

using namespace Intel::OpenCL::Utils;

namespace Intel { namespace OpenCL { namespace Framework
{

D3D9Surface::~D3D9Surface()
{
    if (GetResourceInfo() != nullptr)
    {
        IDirect3DSurface9* const pSurface = static_cast<IDirect3DSurface9*>(GetResourceInfo()->m_pResource);
        D3D9Context& context = *m_pContext.DynamicCast<D3D9Context>();
        if (nullptr != context.GetSurfaceLocker(pSurface))
        {
            context.ReleaseSurfaceLocker(pSurface);
        }        
    }
}

cl_image_format D3D9Surface::GetClImageFormat(const D3DResourceInfo<IDirect3DResource9>& resourceInfo) const
{
    const D3D9SurfaceResourceInfo& surfaceInfo = static_cast<const D3D9SurfaceResourceInfo&>(resourceInfo);
    return MapD3DFormat2OclFormat(GetDesc(resourceInfo).Format, surfaceInfo.m_plane);
}

D3DSURFACE_DESC D3D9Surface::GetDesc(const D3DResourceInfo<IDirect3DResource9>& resourceInfo) const
{
    IDirect3DSurface9* const pSurface =
        static_cast<IDirect3DSurface9*>(resourceInfo.m_pResource);
    D3DSURFACE_DESC desc;
    const HRESULT res = pSurface->GetDesc(&desc);
    assert(D3D_OK == res);
    return desc;
}

void* D3D9Surface::Lock()
{
    IDirect3DSurface9* const pSurface =
        static_cast<IDirect3DSurface9*>(GetResourceInfo()->m_pResource);
    D3DLOCKED_RECT lockedRect;
    HRESULT res;
    const D3DFORMAT format = GetDesc(*GetResourceInfo()).Format;
    if (MAKEFOURCC('N', 'V', '1', '2') != format && MAKEFOURCC('Y', 'V', '1', '2') != format)
    {
        res = pSurface->LockRect(&lockedRect, nullptr, GetD3D9Flags());            
        assert(D3D_OK == res);
        return lockedRect.pBits;
    }
    /*              NV12 format                                 YV12 format

        +-------------------------------+            +-------------------------------+            
        |                       |       |            |                       |       |
        |           Y           |D3D    | plane 0    |           Y           |D3D    | plane 0
        |                       |cache  |            |                       |cache  |
        |                       |       |            |                       |       |
        +-----------------------+-------+            +-----------+---+-------+---+---+
        |           UV          |D3D    | plane 1    |     U     |ca-|     V     |ca-|
        |                       |cache  |            |           |che|           |che|
        +-----------------------+-------+            +-----------+---+-----------+---+
                                                        plane 1         plane 2
    */
    const UINT plane = dynamic_cast<const D3D9SurfaceResourceInfo*>(GetResourceInfo())->m_plane;
    SurfaceLocker* const pSurfaceLocker = m_pContext.DynamicCast<D3D9Context>()->GetSurfaceLocker(pSurface);
    void* const pData = pSurfaceLocker->Lock();
	assert(nullptr != pData);
	if (nullptr == pData)
	{
		return nullptr;
	}
    assert(0 == plane || 1 == plane || 2 == plane);
    if (0 == plane)
    {
        return pData;
    }
    const UINT height = GetDesc(*GetResourceInfo()).Height;
    if (1 == plane)
    {
        return (char*)pData + height * pSurfaceLocker->GetPitch();
    }
    assert(height * pSurfaceLocker->GetPitch() % 4 == 0);
    return (char*)pData + height * pSurfaceLocker->GetPitch() * 5 / 4;
}

bool D3D9Surface::ObtainPitch(size_t& szPitch)
{
    IDirect3DSurface9* const pSurface =
        static_cast<IDirect3DSurface9*>(GetResourceInfo()->m_pResource);
    const SurfaceLocker* const pSurfaceLocker = m_pContext.DynamicCast<D3D9Context>()->GetSurfaceLocker(pSurface);
    if (nullptr != pSurfaceLocker)
    {
        if (MAKEFOURCC('Y', 'V', '1', '2') != GetDesc(*GetResourceInfo()).Format ||
            0 == dynamic_cast<const D3D9SurfaceResourceInfo*>(GetResourceInfo())->m_plane)
        {
            szPitch = pSurfaceLocker->GetPitch();
        }
        else
        {
            assert(pSurfaceLocker->GetPitch() % 2 == 0);
            szPitch = pSurfaceLocker->GetPitch() / 2;                
        }            
        return true;
    }
    D3DLOCKED_RECT lockedRect;
    HRESULT res = pSurface->LockRect(&lockedRect, nullptr, GetD3D9Flags());
    if (D3D_OK != res)
    {
        return false;
    }
    szPitch = lockedRect.Pitch;
    res = pSurface->UnlockRect();
    assert(D3D_OK == res);
    return true;
}

void D3D9Surface::Unlock()
{
    IDirect3DSurface9* const pSurface =
        static_cast<IDirect3DSurface9*>(GetResourceInfo()->m_pResource);
    SurfaceLocker* const pSurfaceLocker = m_pContext.DynamicCast<D3D9Context>()->GetSurfaceLocker(pSurface);
    if (nullptr != pSurfaceLocker)
    {
        pSurfaceLocker->Unlock();
    }
    else
    {
        const HRESULT res = pSurface->UnlockRect();            
        assert(D3D_OK == res);
    }        
}

UINT D3D9Surface::GetWidth(const D3DResourceInfo<IDirect3DResource9>& resourceInfo) const
{
    return GetDesc(resourceInfo).Width;
}

UINT D3D9Surface::GetHeight(const D3DResourceInfo<IDirect3DResource9>& resourceInfo) const
{
    return GetDesc(resourceInfo).Height;
}

cl_err_code D3D9Surface::GetImageInfoInternal(const cl_image_info clParamName, size_t& szSize,
    void* pParamValue, const size_t szParamValueSize) const
{
    const void* pValue;
    const UINT plane = dynamic_cast<const D3D9SurfaceResourceInfo*>(GetResourceInfo())->m_plane;
    const D3DFORMAT format = GetDesc(*GetResourceInfo()).Format;
    assert(0 == plane || 1 == plane || 2 == plane);
    size_t szHeight, szWidth;

    const D3DContext<IDirect3DResource9, IDirect3DDevice9>& context = *m_pContext.DynamicCast<D3DContext<IDirect3DResource9, IDirect3DDevice9>>();
    if (clParamName == static_cast<const ID3D9Definitions&>(context.GetD3dDefinitions()).GetImageDx9MediaPlane())
    {
        szSize = sizeof(UINT);
        pValue = &plane;
    }
    else
    {
        switch (clParamName)
            {
            case CL_IMAGE_HEIGHT:
                if (MAKEFOURCC('N', 'V', '1', '2') == format || MAKEFOURCC('Y', 'V', '1', '2') == format)
                {
                    const UINT height = GetDesc(*GetResourceInfo()).Height;
                    if (0 == plane)
                    {
                        szHeight = height;
                    }
                    else
                    {
                        assert(height % 2 == 0);
                        szHeight = height / 2;
                }
                szSize = sizeof(size_t);
                pValue = &szHeight;
                break;
            }
        case CL_IMAGE_WIDTH:
            if (MAKEFOURCC('N', 'V', '1', '2') == format || MAKEFOURCC('Y', 'V', '1', '2') == format)
            {
                const UINT width = GetDesc(*GetResourceInfo()).Width;
                if (0 == plane)
                {
                    szWidth = width;
                }
                else
                {
                    assert(width % 2 == 0);
                    // In NV12 we also divide by 2, since the size of each element is 2.
                    szWidth = width / 2;
                }
                szSize = sizeof(size_t);
                pValue = &szWidth;
                break;
            }
            // else fall through          
        default:
            return D3DImage2D<IDirect3DResource9, IDirect3DDevice9, D3DSURFACE_DESC>::GetImageInfoInternal(clParamName, szSize, pParamValue, szParamValueSize);
        }            
    }
    if (nullptr != pParamValue && szSize > 0)
    {
        MEMCPY_S(pParamValue, szParamValueSize, pValue, szSize);
    }
    return CL_SUCCESS;
}

size_t D3D9Surface::GetPixelSize() const
{
    const cl_image_format clFormat = MapD3DFormat2OclFormat(GetDesc(*GetResourceInfo()).Format,
        dynamic_cast<const D3D9SurfaceResourceInfo*>(GetResourceInfo())->m_plane);
    return clGetPixelBytesCount(&clFormat);
}

size_t D3D9Surface::GetMemObjSize() const
{
    const D3DSURFACE_DESC desc = GetDesc(*GetResourceInfo());
    const UINT plane = dynamic_cast<const D3D9SurfaceResourceInfo*>(GetResourceInfo())->m_plane;
    const cl_image_format clFormat = MapD3DFormat2OclFormat(desc.Format, plane);
    switch ((unsigned long int)desc.Format)
    {
    case MAKEFOURCC('N', 'V', '1', '2'):
        assert(0 == plane || 1 == plane);
        if (0 == plane)
        {
            return sizeof(cl_uchar) * GetPitch() * desc.Height;
        }
        assert(GetPitch() * desc.Height % 2 == 0);
        return sizeof(cl_uchar) * GetPitch() * desc.Height / 2;
    case MAKEFOURCC('Y', 'V', '1', '2'):
        assert(0 == plane || 1 == plane || 2 == plane);
        if (0 == plane)
        {
            return sizeof(cl_uchar) * GetPitch() * desc.Height;
        }
        assert(GetPitch() * desc.Height % 4 == 0);
        return sizeof(cl_uchar) * GetPitch() * desc.Height / 4;
    default:
        return clGetPixelBytesCount(&clFormat) * GetPitch() * desc.Height;
    }        
}

cl_err_code D3D9Surface::CheckBounds(const size_t* pszOrigin, const size_t* pszRegion) const
{
    const D3DSURFACE_DESC desc = GetDesc(*GetResourceInfo());
    const UINT plane = dynamic_cast<const D3D9SurfaceResourceInfo*>(GetResourceInfo())->m_plane;

    if (MAKEFOURCC('N', 'V', '1', '2') == desc.Format)
    {
        assert(desc.Height % 2 == 0);
        if (1 == plane && pszOrigin[1] + pszRegion[1] > desc.Height / 2)
        {
            return CL_INVALID_VALUE;
        }
    }
    else if (MAKEFOURCC('Y', 'V', '1', '2') == desc.Format)
    {
        assert(desc.Height % 2 == 0);
        assert(desc.Width % 2 == 0);
        if (0 != plane && (pszOrigin[1] + pszRegion[1] > desc.Height / 2 ||
                           pszOrigin[0] + pszRegion[0] > desc.Width / 2))
        {
            return CL_INVALID_VALUE;
        }
    }
    return D3DImage2D<IDirect3DResource9, IDirect3DDevice9, D3DSURFACE_DESC>::CheckBounds(pszOrigin, pszRegion);
}

void D3D9Surface::FillDimensions(const D3DResourceInfo<IDirect3DResource9>& resourceInfo, size_t* const pszDims) const
{
    const D3DSURFACE_DESC desc = GetDesc(resourceInfo);        
    if (MAKEFOURCC('N', 'V', '1', '2') != desc.Format &&
        MAKEFOURCC('Y', 'V', '1', '2') != desc.Format)
    {
        pszDims[0] = desc.Width;
        pszDims[1] = desc.Height;
    }
    else
    {
        const UINT plane = dynamic_cast<const D3D9SurfaceResourceInfo&>(resourceInfo).m_plane;
        assert(0 == plane || 1 == plane || 2 == plane);
        if (0 == plane)
        {
            pszDims[0] = desc.Width;
            pszDims[1] = desc.Height;
        }
        else
        {
            assert(desc.Width % 2 == 0);
            assert(desc.Height % 2 == 0);
            pszDims[0] = desc.Width / 2;
            pszDims[1] = desc.Height / 2;
        }
    }
}

bool D3D9Surface::IsValidlyCreated(D3DResourceInfo<IDirect3DResource9>& resourceInfo) const
{
    const ID3DSharingDefinitions& d3d9Definitions = GetContext().DynamicCast<D3D9Context>()->GetD3dDefinitions();

    // In Khronos spec it says that we should check that the surface has been created in D3DPOOL_DEFAULT just for adapter_type CL_ADAPTER_D3D9_KHR
    if (d3d9Definitions.GetVersion() == ID3DSharingDefinitions::D3D9_INTEL ||
        d3d9Definitions.GetVersion() == ID3DSharingDefinitions::D3D9_KHR && CL_ADAPTER_D3D9_KHR == static_cast<D3D9SurfaceResourceInfo&>(resourceInfo).m_adapterType)
    {
        return D3DPOOL_DEFAULT == GetDesc(resourceInfo).Pool;
    }
    else
    {
        return true;
    }
}

size_t D3D11Buffer::GetMemObjSize() const
{
    D3D11_BUFFER_DESC desc;        
    static_cast<ID3D11Buffer*>(GetResourceInfo()->m_pResource)->GetDesc(&desc);
    return desc.ByteWidth;
}

cl_err_code D3D11Buffer::Initialize(cl_mem_flags clMemFlags, const cl_image_format* pclImageFormat, unsigned int dim_count, const size_t* dimension,
    const size_t* pitches, void* pHostPtr, cl_rt_memobj_creation_flags	creation_flags)
{
    D3DResourceInfo<ID3D11Resource>& resourceInfo = *static_cast<D3DResourceInfo<ID3D11Resource>*>(pHostPtr);
    ID3D11Buffer* const pBuffer = static_cast<ID3D11Buffer*>(resourceInfo.m_pResource);
    
    m_pBufferMapper = new D3d11BufferMapper(pBuffer, D3d11BufferMapper::GetD3d11Map(clMemFlags));
    if (nullptr == m_pBufferMapper)
    {
        return CL_OUT_OF_HOST_MEMORY;
    }
    return D3DResource::Initialize(clMemFlags, pclImageFormat, dim_count, dimension, pitches, pHostPtr, creation_flags);
}

bool D3D11Buffer::IsValidlyCreated(D3DResourceInfo<ID3D11Resource>& resourceInfo) const
{
    D3D11_BUFFER_DESC desc;        
    static_cast<ID3D11Buffer*>(resourceInfo.m_pResource)->GetDesc(&desc);
    return D3D11_USAGE_IMMUTABLE != desc.Usage;
}

void* D3D11Buffer::Lock()
{
    return m_pBufferMapper->Map();
}

void D3D11Buffer::Unlock()
{
    m_pBufferMapper->Unmap();
}

cl_err_code D3D11Buffer::CheckBounds(const size_t* pszOrigin, const size_t* pszRegion) const
{
    return *pszOrigin + *pszRegion <= m_stMemObjSize ? CL_SUCCESS : CL_INVALID_VALUE ;
}

void D3D11Buffer::FillDimensions(const D3DResourceInfo<ID3D11Resource>& resourceInfo, size_t* const pszDims) const
{
    D3D11_BUFFER_DESC desc;
    static_cast<ID3D11Buffer*>(resourceInfo.m_pResource)->GetDesc(&desc);
    pszDims[0] = desc.ByteWidth;
}

cl_err_code D3D11Texture2D::GetImageInfoInternal(const cl_image_info clParamName, size_t& szSize, void* pParamValue, const size_t szParamValueSize) const
{
    const void* pValue;

    switch (clParamName)
    {
    case CL_IMAGE_D3D11_SUBRESOURCE_KHR:
        szSize = sizeof(UINT);
        pValue = &static_cast<const D3D11TextureResourceInfo*>(GetResourceInfo())->m_uiSubresource;
        break;
    default:
        return D3DImage2D<ID3D11Resource, ID3D11Device, D3D11_TEXTURE2D_DESC>::GetImageInfoInternal(clParamName, szSize, pParamValue, szParamValueSize);
    }
    if (nullptr != pParamValue && szSize > 0)
    {
        MEMCPY_S(pParamValue, szParamValueSize, pValue, szSize);
    }
    return CL_SUCCESS;
}

cl_err_code D3D11Texture2D::Initialize(cl_mem_flags	clMemFlags, const cl_image_format* pclImageFormat, unsigned int dim_count, const size_t* dimension,
    const size_t* pitches, void* pHostPtr, cl_rt_memobj_creation_flags	creation_flags)
{
    const D3D11TextureResourceInfo& textureInfo = *static_cast<const D3D11TextureResourceInfo*>(pHostPtr);
    D3D11_TEXTURE2D_DESC desc;
    ID3D11Texture2D* const pTexture2d = static_cast<ID3D11Texture2D*>(textureInfo.m_pResource);
    
    pTexture2d->GetDesc(&desc);
    /* If they user has created the texture with MipLevels = 0 to generate a full set of subtextures, then the Direct3D RT updates this field to the actual number of mip levels, so we won't
        see 0 here */
    if (textureInfo.m_uiSubresource >= desc.ArraySize * desc.MipLevels)
    {
        return CL_INVALID_VALUE;
    }
    
    const D3D11_MAP mapType = D3d11Texture2DMapper::GetD3d11Map(clMemFlags);
    m_pTexture2DMapper = new D3d11Texture2DMapper(pTexture2d, mapType, textureInfo.m_uiSubresource);
    if (!m_pTexture2DMapper)
    {
        return CL_OUT_OF_HOST_MEMORY;
    }
    return D3DResource<ID3D11Resource, ID3D11Device>::Initialize(clMemFlags, pclImageFormat, dim_count, dimension, pitches, pHostPtr, creation_flags);
}

bool D3D11Texture2D::IsValidlyCreated(D3DResourceInfo<ID3D11Resource>& resourceInfo) const
{
    D3D11_TEXTURE2D_DESC desc;
    static_cast<ID3D11Texture2D*>(resourceInfo.m_pResource)->GetDesc(&desc);
    return D3D11_USAGE_IMMUTABLE != desc.Usage;
}

cl_image_format D3D11Texture2D::GetClImageFormat(const D3DResourceInfo<ID3D11Resource>& resourceInfo) const
{
    D3D11_TEXTURE2D_DESC desc;
    static_cast<ID3D11Texture2D*>(resourceInfo.m_pResource)->GetDesc(&desc);
    return MapDxgiFormat2OclFormat(desc.Format);
}

bool D3D11Texture2D::ObtainPitch(size_t& szPitch)
{
    szPitch = m_pTexture2DMapper->GetRowPitch();
    return szPitch > 0;
}

void* D3D11Texture2D::Lock()
{
    return m_pTexture2DMapper->Map();
}

void D3D11Texture2D::Unlock()
{
    m_pTexture2DMapper->Unmap();
}

UINT D3D11Texture2D::GetWidth(const D3DResourceInfo<ID3D11Resource>& resourceInfo) const
{
    D3D11_TEXTURE2D_DESC desc;
    static_cast<ID3D11Texture2D*>(resourceInfo.m_pResource)->GetDesc(&desc);
    const UINT uiSubresource = static_cast<const D3D11TextureResourceInfo&>(resourceInfo).m_uiSubresource;
    // 0 MipLevels is used to generate a full set of sub-textures
    assert(desc.MipLevels > 0);
    const UINT uiSubresourceMipLevel = uiSubresource % desc.MipLevels;
	if ((desc.Width >> uiSubresourceMipLevel) > 1)
    {
        return desc.Width >> uiSubresourceMipLevel;    // width = desc.Width / 2^uiSubresourceMipLevel
    }
    else
    {
        return 1;
    }
}

UINT D3D11Texture2D::GetHeight(const D3DResourceInfo<ID3D11Resource>& resourceInfo) const
{
    D3D11_TEXTURE2D_DESC desc;
    static_cast<ID3D11Texture2D*>(resourceInfo.m_pResource)->GetDesc(&desc);
    const UINT uiSubresource = static_cast<const D3D11TextureResourceInfo&>(resourceInfo).m_uiSubresource;
    // 0 MipLevels is used to generate a full set of sub-textures
    assert(desc.MipLevels > 0);
    const UINT uiSubresourceMipLevel = uiSubresource % desc.MipLevels;
	if ((desc.Height >> uiSubresourceMipLevel) > 1)
    {
        return desc.Height >> uiSubresourceMipLevel;   // height = desc.Height / 2^uiSubresourceMipLevel
    }
    else
    {
        return 1;
    }
}

cl_err_code D3D11Texture3D::Initialize(cl_mem_flags	clMemFlags, const cl_image_format* pclImageFormat, unsigned int dim_count, const size_t* dimension,
    const size_t* pitches, void* pHostPtr, cl_rt_memobj_creation_flags creation_flags)
{
    const D3D11TextureResourceInfo& textureInfo = *static_cast<const D3D11TextureResourceInfo*>(pHostPtr);
    D3D11_TEXTURE3D_DESC desc;        
    ID3D11Texture3D* const pTexture3D = static_cast<ID3D11Texture3D*>(textureInfo.m_pResource);
    
    pTexture3D->GetDesc(&desc);
    if (textureInfo.m_uiSubresource >= desc.MipLevels)
    {
        return CL_INVALID_VALUE;
    }
    
    const D3D11_MAP mapType = D3d11Mapper<ID3D11Texture3D, D3D11_TEXTURE3D_DESC>::GetD3d11Map(clMemFlags);
    m_pTexture3DMapper = new D3d11Texture3DMapper(pTexture3D, mapType, textureInfo.m_uiSubresource);
    if (!m_pTexture3DMapper)
    {
        return CL_OUT_OF_HOST_MEMORY;
    }
    return D3DResource<ID3D11Resource, ID3D11Device>::Initialize(clMemFlags, pclImageFormat, dim_count, dimension, pitches, pHostPtr, creation_flags);
}

bool D3D11Texture3D::IsValidlyCreated(D3DResourceInfo<ID3D11Resource>& resourceInfo) const
{
    D3D11_TEXTURE3D_DESC desc;
    static_cast<ID3D11Texture3D*>(resourceInfo.m_pResource)->GetDesc(&desc);
    return D3D11_USAGE_IMMUTABLE != desc.Usage;
}

// for the 3 following methods: dimension X = desc.X / 2^subresource

UINT D3D11Texture3D::GetWidth(const D3DResourceInfo<ID3D11Resource>& resourceInfo) const
{
    D3D11_TEXTURE3D_DESC desc;
    static_cast<ID3D11Texture3D*>(resourceInfo.m_pResource)->GetDesc(&desc);
	
	const UINT uiWidth = desc.Width >> static_cast<const D3D11TextureResourceInfo&>(resourceInfo).m_uiSubresource;
	if (uiWidth > 0)
    {
        return uiWidth;
    }
    else
    {
        return 1;
    }        
}

UINT D3D11Texture3D::GetHeight(const D3DResourceInfo<ID3D11Resource>& resourceInfo) const
{
    D3D11_TEXTURE3D_DESC desc;
    static_cast<ID3D11Texture3D*>(resourceInfo.m_pResource)->GetDesc(&desc);

	const UINT uiHeight = desc.Height >> static_cast<const D3D11TextureResourceInfo&>(resourceInfo).m_uiSubresource;
    if (uiHeight > 1)
    {
        return uiHeight;
    }
    else
    {
        return 1;
    }        
}

UINT D3D11Texture3D::GetDepth(const D3DResourceInfo<ID3D11Resource>& resourceInfo) const
{
    D3D11_TEXTURE3D_DESC desc;
    static_cast<ID3D11Texture3D*>(resourceInfo.m_pResource)->GetDesc(&desc);

	const UINT uiDepth = desc.Depth >> static_cast<const D3D11TextureResourceInfo&>(resourceInfo).m_uiSubresource;
    if (uiDepth > 1)
    {
        return uiDepth;
    }
    else
    {
        return 1;
    }
}

cl_err_code D3D11Texture3D::GetImageInfoInternal(const cl_image_info clParamName, size_t& szSize,
    void* pParamValue, const size_t szParamValueSize) const
{
    size_t szVal;
    const void* pValue;

    switch (clParamName)
    {
    case CL_IMAGE_D3D11_SUBRESOURCE_KHR:
        szSize = sizeof(UINT);
        pValue = &static_cast<const D3D11TextureResourceInfo*>(GetResourceInfo())->m_uiSubresource;
        break;
    case CL_IMAGE_ROW_PITCH:
        szSize = sizeof(size_t);
        pValue = m_szPitch;
        break;            
    case CL_IMAGE_WIDTH:
        szSize = sizeof(size_t);
        szVal = GetWidth(*GetResourceInfo());
        pValue = &szVal;
        break;            
    case CL_IMAGE_HEIGHT:
        szSize = sizeof(size_t);
        szVal = GetHeight(*GetResourceInfo());
        pValue = &szVal;
        break;            
    case CL_IMAGE_DEPTH:
        szSize = sizeof(size_t);
        szVal = GetDepth(*GetResourceInfo());
        pValue = &szVal;
        break;
    case CL_IMAGE_SLICE_PITCH:
        szSize = sizeof(size_t);
        pValue = &m_szPitch[1];
        break;
    default:
        return D3DResource::GetImageInfoInternal(clParamName, szSize, pParamValue, szParamValueSize);
    }
    if (nullptr != pParamValue && szSize > 0)
    {
        MEMCPY_S(pParamValue, szParamValueSize, pValue, szSize);
    }
    return CL_SUCCESS;
}

void* D3D11Texture3D::Lock()
{
    return m_pTexture3DMapper->Map();
}

void D3D11Texture3D::Unlock()
{
    return m_pTexture3DMapper->Unmap();
}

void D3D11Texture3D::FillDimensions(const D3DResourceInfo<ID3D11Resource>& resourceInfo, size_t* const pszDims) const
{
    pszDims[0] = GetWidth(resourceInfo);
    pszDims[1] = GetHeight(resourceInfo);
    pszDims[2] = GetDepth(resourceInfo);
}

bool D3D11Texture3D::ObtainPitches()
{
    m_szPitch[0] = m_pTexture3DMapper->GetRowPitch();
    m_szPitch[1] = m_pTexture3DMapper->GetDepthPitch();
    return m_szPitch[0] > 0 && m_szPitch[1] > 0;
}

const size_t* D3D11Texture3D::GetPitches() const { return m_szPitch; }

cl_image_format D3D11Texture3D::GetClImageFormat(const D3DResourceInfo<ID3D11Resource>& resourceInfo) const
{
    D3D11_TEXTURE3D_DESC desc;
    static_cast<ID3D11Texture3D*>(resourceInfo.m_pResource)->GetDesc(&desc);
    return MapDxgiFormat2OclFormat(desc.Format);
}

size_t D3D11Texture3D::GetMemObjSize() const
{
    return m_pTexture3DMapper->GetRowPitch() * m_pTexture3DMapper->GetDepthPitch() * GetDepth(*GetResourceInfo());
}

cl_err_code D3D11Texture3D::CheckBounds(const size_t* pszOrigin, const size_t* pszRegion) const
{
    if (pszOrigin[0] + pszRegion[0] > GetWidth(*GetResourceInfo()))
    {
        return CL_INVALID_VALUE;
    }
    if (pszOrigin[1] + pszRegion[1] > GetHeight(*GetResourceInfo()))
    {
        return CL_INVALID_VALUE;
    }
    if (pszOrigin[2] + pszRegion[2] > GetDepth(*GetResourceInfo()))
    {
        return CL_INVALID_VALUE;
    }
    return CL_SUCCESS;
}

}}}
