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

#pragma once

#include <d3d9.h>

namespace Intel { namespace OpenCL { namespace Framework
{

    /**
     * @struct  D3D9ResourceInfo
     *
     * @brief   Information about the Direct3D 9 resource. 
     *
     * @author  Aharon
     * @date    7/6/2011
     */

    struct D3D9ResourceInfo
    {
        IDirect3DResource9* const m_pResource;

        /**
         * @fn  explicit D3D9ResourceInfo(IDirect3DResource9* const pResource);
         *
         * @brief   Constructor.
         *
         * @author  Aharon
         * @date    7/7/2011
         *
         * @param   pResource   the resource.
         */

        explicit D3D9ResourceInfo(IDirect3DResource9* const pResource) : m_pResource(pResource) { }

        /**
         * @fn  virtual bool operator<(const D3D9ResourceInfo& other) const
         *
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

        virtual bool operator<(const D3D9ResourceInfo& other) const { return m_pResource < other.m_pResource; }

    };

    /**
     * @struct  D3D9TextureResourceInfo
     *
     * @brief   Information about the Direct3D 9 texture resource. 
     *
     * @author  Aharon
     * @date    7/20/2011
     */

    struct D3D9TextureResourceInfo : public D3D9ResourceInfo
    {
        const UINT m_uiMipLevel;

        /**
         * @fn  D3D9TextureResourceInfo(IDirect3DResource9* const pResource, const UINT uiMipLevel)
         *
         * @brief   Constructor.
         *
         * @author  Aharon
         * @date    7/20/2011
         *
         * @param   pResource           The resource.
         * @param   uiMipLevel          The mip level.
         */

        D3D9TextureResourceInfo(IDirect3DResource9* const pResource, const UINT uiMipLevel) :
            D3D9ResourceInfo(pResource), m_uiMipLevel(uiMipLevel) { }
    
        // inherited methods:

        virtual bool operator<(const D3D9ResourceInfo& other) const
        {
            if (m_pResource == other.m_pResource)
                return m_uiMipLevel < dynamic_cast<const D3D9TextureResourceInfo&>(other).m_uiMipLevel;
            return D3D9ResourceInfo::operator <(other);
        }
    };

    /**
     * @struct  D3D9CubeTextureResourceInfo
     *
     * @brief   Information about the Direct3D 9 cube texture resource. 
     *
     * @author  Aharon
     * @date    7/24/2011
     */

    struct D3D9CubeTextureResourceInfo : public D3D9TextureResourceInfo
    {
        const D3DCUBEMAP_FACES m_facetype;

        /**
         * @fn  D3D9CubeTextureResourceInfo(IDirect3DResource9* const pResource, const UINT uiMipLevel,
         *      D3DCUBEMAP_FACES facetype);
         *
         * @brief   Constructor.
         *
         * @author  Aharon
         * @date    7/24/2011
         *
         * @param   pResource           Rhe resource.
         * @param   uiMipLevel          The mip level.
         * @param   facetype            The facetype.
         */

        D3D9CubeTextureResourceInfo(IDirect3DResource9* const pResource, const UINT uiMipLevel,
            D3DCUBEMAP_FACES facetype) : D3D9TextureResourceInfo(pResource, uiMipLevel), m_facetype(facetype) { }

        // inherited methods:

        virtual bool operator<(const D3D9ResourceInfo& other) const
        {
            const D3D9CubeTextureResourceInfo& otherCubeTextureResourceInfo =
                dynamic_cast<const D3D9CubeTextureResourceInfo&>(other);
            if (m_pResource == otherCubeTextureResourceInfo.m_pResource &&
                m_uiMipLevel == otherCubeTextureResourceInfo.m_uiMipLevel)
            {
                return m_facetype < otherCubeTextureResourceInfo.m_facetype;
            }
            return D3D9TextureResourceInfo::operator <(other);
        }
    };

     /**
      * @fn IDirect3DDevice9* ParseD3D9ContextOptions(const cl_context_properties* const pProperties);
      *
      * @brief  This function parses an array of cl_context_properties elements and tries to find in
      *         them a CL_CONTEXT_D3D9_DEVICE_Intel property and if it finds such, it returns it.
      *
      * @author Aharon
      * @date   6/30/2011
      *
      * @param  pProperties the array of cl_context_properties elements.
      *
      * @return value found or NULL if no such property is found.
      */

     IDirect3DDevice9* ParseD3D9ContextOptions(const cl_context_properties* const pProperties);

}}}
