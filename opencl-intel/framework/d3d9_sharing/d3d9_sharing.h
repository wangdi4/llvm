// Copyright (c) 2006-2012 Intel Corporation
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

#pragma once

#include <d3d9.h>
#include "d3d9_definitions.h"

namespace Intel { namespace OpenCL { namespace Framework
{

/**
 * @struct  D3DResourceInfo
 *
 * @brief   Information about the Direct3D 9 resource. 
 *
 * @param RESOURCE_TYPE the type of the Direct 3D resource
 *
 * @author  Aharon
 * @date    7/6/2011
 */

template<typename RESOURCE_TYPE>
struct D3DResourceInfo
{
    RESOURCE_TYPE* const m_pResource;

    /**
     * @brief   Constructor.
     *
     * @author  Aharon
     * @date    7/7/2011
     *
     * @param   pResource   the resource.
     */

    explicit D3DResourceInfo(RESOURCE_TYPE* const pResource) : m_pResource(pResource) { }

    /**
     * @brief   Less-than comparison operator.
     *
     * @author  Aharon
     * @date    7/25/2011
     *
     * @param   other   The other (it is guaranteed that the concrete class of &lt;param&gt;other&lt;
     *                  param&gt; is the same as that of this).
     *
     * @return  true if the first parameter is less than the second.
     */

    virtual bool operator<(const D3DResourceInfo& other) const { return m_pResource < other.m_pResource; }

};


/**
 * @struct D3D9SurfaceResourceInfo
 * 
 *  @brief  Information about the Direct3D 9 surface resource
 *  		
 *  @author Aharon
 *  @date   9/13/2011
 */
struct D3D9SurfaceResourceInfo : public D3DResourceInfo<IDirect3DResource9>
{        
    HANDLE m_sharehandle;
    UINT m_plane;
    cl_dx9_media_adapter_type_khr m_adapterType;    // this may be 0 in case of Intel version of the extension

    /**
     * @brief   Constructor.
     *
     * @author  Aharon
     * @date    9/13/2011
     * 			
     * @param   pResource   The resource
     * @param   sharehandle The shared handle of the pResource used to share the surface between devices
     * @param   plane       The plane of pResource to share, for planar surface formats
     * @param   adapterType type of adapter in case of Khronos extension: CL_CONTEXT_ADAPTER_D3D9_KHR,
     *                      CL_CONTEXT_ADAPTER_D3D9EX_KHR, CL_CONTEXT_ADAPTER_DXVA_KHR; 0 in case of Intel extension.
     */
    D3D9SurfaceResourceInfo(IDirect3DResource9* const pResource, HANDLE sharehandle, UINT plane, cl_dx9_media_adapter_type_khr adapterType) :
        D3DResourceInfo(pResource), m_sharehandle(sharehandle), m_plane(plane), m_adapterType(adapterType) { }

    // inherited methods:

    virtual bool operator<(const D3DResourceInfo& other) const
    {
        if (m_pResource == other.m_pResource)
            return m_plane < dynamic_cast<const D3D9SurfaceResourceInfo&>(other).m_plane;
        return D3DResourceInfo::operator <(other);
    }
};

/**
 * @struct  D3D11TextureResourceInfo
 *
 * @brief   Information about the Direct3D 11 texture resource.
 *
 * @author  Aharon
 * @date    7/20/2011
 */

struct D3D11TextureResourceInfo : public D3DResourceInfo<ID3D11Resource>
{
    const UINT m_uiSubresource;

    /**
     * @brief   Constructor.
     *
     * @author  Aharon
     * @date    7/20/2011
     *
     * @param   pResource           The resource.
     * @param   uiSubresource          The mip level.
     */

    D3D11TextureResourceInfo(ID3D11Resource* const pResource, const UINT uiSubresource) :
        D3DResourceInfo<ID3D11Resource>(pResource), m_uiSubresource(uiSubresource) { }

    // inherited methods:

    virtual bool operator<(const D3DResourceInfo<ID3D11Resource>& other) const
    {
        if (m_pResource == other.m_pResource)
            return m_uiSubresource < static_cast<const D3D11TextureResourceInfo&>(other).m_uiSubresource;
        return D3DResourceInfo<ID3D11Resource>::operator <(other);
    }
};

/**
 * Check if Direct3D 9 Sharing options are valid if they are present
 * 
 * @param propertyMap   map of properties and their values
 * @param device [out]  a reference to IUnknown* in which the pointer to the Direct3D 9 device is
 * 						to be stored if such is found in pProperties, otherwise it is set to
 * 						NULL
 * @param iDevType [out] a reference to int in which the type of the device is to be stored if such is found in pProperties
 * @param pFactory [out] a new ID3DSharingDefinitions for the version of the extension used or NULL if the extension is not used at all
 * @return CL_INVALID_D3D9_DEVICE_INTEL in case more than one device is found, CL_SUCCESS
 * 		   otherwise						
 */

cl_err_code ParseD3DContextOptions(const std::map<cl_context_properties, cl_context_properties>& propertyMap,
    IUnknown*& device, int& iDevType, const ID3DSharingDefinitions*& pFactory);

/**
 * @param   d3dFormat   the D3DFORMAT.
 * @param   uiPlane     (optional) the plane.
 * @return  the cl_image_format corresponding to &lt;param&gt;format&lt;param&gt; and &lt;
 *          param&gt;uiPlane&lt;param&gt; and an invalid value (0 in both its fields) in case
 *          there is no supported OpenCL format that corresponds to the D3DFORMAT specified by
 *          &lt;param&gt;format&lt;param&gt; and &lt;param&gt;uiPlane&lt;param&gt;.
 */
cl_image_format MapD3DFormat2OclFormat(const D3DFORMAT d3dFormat, unsigned int uiPlane);

/**
 * @param dxgiFormat a DXGI_FORMAT
 * @return the cl_image_format corresponding to dxgiFormat or a zeroed structure if dxgiFormat is an invalid value
 */
cl_image_format MapDxgiFormat2OclFormat(const DXGI_FORMAT dxgiFormat);

}}}
