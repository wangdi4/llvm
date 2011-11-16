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
#include "GraphicsApiMemoryObject.h"
#include "d3d9_context.h"
#include "cl_synch_objects.h"
#include "ocl_event.h"

namespace Intel { namespace OpenCL { namespace Framework
{
    /**
     * @class   D3D9Resource
     *
     * @brief   This class represents a Direct3D 9 resource
     *
     * @author  Aharon
     * @date    7/6/2011
     *
     * @sa  GraphicsApiMemoryObject
     */

    class D3D9Resource : public GraphicsApiMemoryObject
    {

        D3D9ResourceInfo* m_pResourceInfo;
        size_t m_szDimensions[3];
        bool m_bAcquired;

    public:

        /**
         * @fn  static cl_image_format D3D9Resource::MapD3DFormat2OclFormat(const D3DFORMAT d3dFormat,
         *      unsigned int uiPlane = 0);
         *
         * @brief   Map a D3DFormat to cl_image_format.
         *
         * @author  Aharon
         * @date    7/19/2011
         *
         * @param   d3dFormat   the D3DFORMAT.
         * @param   uiPlane     (optional) the plane.
         *
         * @return  the cl_image_format corresponding to &lt;param&gt;format&lt;param&gt; and &lt;
         *          param&gt;uiPlane&lt;param&gt; and an invalid value (0 in both its fields) in case
         *          there is no supported OpenCL format that corresponds to the D3DFORMAT specified by
         *          &lt;param&gt;format&lt;param&gt; and &lt;param&gt;uiPlane&lt;param&gt;.
         */

        static cl_image_format MapD3DFormat2OclFormat(const D3DFORMAT d3dFormat, unsigned int uiPlane = 0);

        /**
         * @fn  virtual D3D9Resource::~D3D9Resource()
         *
         * @brief   Finaliser.
         *
         * @author  Aharon
         * @date    7/6/2011
         */

        virtual ~D3D9Resource();

        /**
         * @fn  virtual bool D3D9Resource::IsCreatedInD3DPoolDefault(D3D9ResourceInfo& resourceInfo) const = 0;
         *
         * @brief   Query if this object is created in D3DPOOL_DEFAULT.
         *
         * @author  Aharon
         * @date    7/7/2011
         *
         * @param   resourceInfo    Information describing the resource (we give this parameter to
         * 							enable calling this method before having called Initialize).
         *
         * @return  true if created in D3DPOOL_DEFAULT, false if not.
         */

        virtual bool IsCreatedInD3DPoolDefault(D3D9ResourceInfo& resourceInfo) const = 0;

        /**
         * @fn  virtual void* D3D9Resource::Lock() = 0;
         *
         * @brief   Locks the shared region of the Direct3D 9 object
         *
         * @author  Aharon
         * @date    7/13/2011
         *
         * @return  a pointer to the locked data
         */

        virtual void* Lock() = 0;

        /**
         * @fn  virtual void D3D9Resource::Unlock() = 0;
         *
         * @brief   Unlocks the shared region of the Direct3D 9 object
         *
         * @author  Aharon
         * @date    7/13/2011
         */

        virtual void Unlock() = 0;

        /**
         * @fn  virtual void D3D9Resource::FillDimensions(const D3D9ResourceInfo& resourceInfo,
         *      size_t* const pszDims) const = 0;
         *
         * @brief   Fill a dimensions array.
         *
         * @author  Aharon
         * @date    7/19/2011
         *
         * @param   resourceInfo    Information describing the resource (we give this parameter to enable
         *                          calling this method before having called Initialize).
         * @param [in,out]  pszDims the dimensions array, which is guaranteed to have enough size to
         *                          accommodate all the dimensions.
         */

        virtual void FillDimensions(const D3D9ResourceInfo& resourceInfo, size_t* const pszDims) const = 0;

        /**
         * @fn  virtual bool D3D9Resource::ObtainPitches()
         *
         * @brief   Obtain pitches (this is often a costly operation, since it requires locking and
         *          unlocking the resource). This method is guaranteed to be called early enough to have
         *          the pitches ready for further use (for instance, for calculating the object's memory
         *          size).
         *
         * @author  Aharon
         * @date    7/24/2011
         *
         * @return  true if it succeeds, false if it fails.
         */

        virtual bool ObtainPitches() { return true; }

        /**
         * @fn  const size_t* D3D9Resource::GetPitches() const
         *
         * @brief   Gets the pitches.
         *
         * @author  Aharon
         * @date    7/24/2011
         *
         * @return  the pitches.
         */

        virtual const size_t* GetPitches() const { return NULL; }

        /**
         * @fn  void D3D9Resource::AcquireD3D9();
         *
         * @brief   Acquires this object from Direct3D 9
         *
         * @author  Aharon
         * @date    7/13/2011
         */

        void AcquireD3D9();

        /**
         * @fn  void D3D9Resource::Release();
         *
         * @brief   Releases this object to Direct3D 9
         *
         * @author  Aharon
         * @date    7/13/2011
         */

        void ReleaseD3D9();

        /**
         * @fn  bool D3D9Resource::IsAcquired() const
         *
         * @brief   Query if this object is acquired.
         *
         * @author  Aharon
         * @date    7/25/2011
         *
         * @return  true if acquired, false if not.
         */

        bool IsAcquired() const { return m_bAcquired; }

        /**
         * @fn  D3D9ResourceInfo& D3D9Resource::GetResourceInfo()
         *
         * @brief   Gets the resource information.
         *
         * @author  Aharon
         * @date    7/7/2011
         *
         * @return  The resource information.
         */

        const D3D9ResourceInfo& GetResourceInfo() const { return *m_pResourceInfo; }

        // inherited methods:

        virtual cl_err_code Initialize(cl_mem_flags	clMemFlags,
            const cl_image_format*	pclImageFormat, unsigned int dim_count,
            const size_t* dimension, const size_t* pitches, void* pHostPtr);

        virtual cl_err_code CreateSubBuffer(cl_mem_flags clFlags,
            cl_buffer_create_type buffer_create_type, const void* buffer_create_info,
            MemoryObject** ppBuffer);

        virtual cl_err_code ReadData(void* pOutData, const size_t* pszOrigin,
            const size_t* pszRegion, size_t szRowPitch, size_t szSlicePitch);

        virtual cl_err_code WriteData(const void* pOutData, const size_t* pszOrigin,
            const size_t* pszRegion, size_t szRowPitch, size_t szSlicePitch);

        virtual FissionableDevice* GetLocation() const;

        cl_err_code GetImageInfo(cl_image_info clParamName, size_t szParamValueSize, void* pParamValue, size_t* pszParamValueSizeRet);

    protected:

        /**
         * @fn  D3D9Resource::D3D9Resource(Context* pContext, ocl_entry_points* pOclEntryPoints)
         *
         * @brief   constructor.
         *
         * @author  Aharon
         * @date    7/6/2011
         */

        D3D9Resource(Context* pContext, ocl_entry_points* pOclEntryPoints, cl_mem_object_type clObjType) :
             GraphicsApiMemoryObject(pContext, pOclEntryPoints, clObjType), m_pResourceInfo(NULL),
                 m_bAcquired(false) { }

        /**
         * @fn  virtual cl_err_code D3D9Resource::GetImageInfoInternal(const cl_image_info clParamName,
         *      size_t& szSize, void* pParamValue, const size_t szParamValueSize) const;
         *
         * @brief   Gets an image information.
         *
         * @author  Aharon
         * @date    7/25/2011
         *
         * @param   clParamName         Name of the OpenCL parameter.
         * @param [in,out]  szSize      a reference to a variable in which to store the size of the
         *                              parameter.
         * @param [in,out]  pParamValue a pointer to a variable in which to store the value of the
         *                              parameter.
         * @param   szParamValueSize    Size of the variable pointed to by &lt;param&gt;pParamValue&lt;
         *                              param&gt;
         *
         * @return  CL_SUCCESS in case of success, error code in case of failure.
         */

        virtual cl_err_code GetImageInfoInternal(const cl_image_info clParamName, size_t& szSize,
            void* pParamValue, const size_t szParamValueSize) const;

        /**
         * @fn  cl_mem_object_type D3D9Resource::GetChildMemObjectType() const = 0;
         *
         * @author  Aharon
         * @date    7/7/2011
         *
         * @return  either CL_MEM_OBJECT_BUFFER, CL_MEM_OBJECT_IMAGE2D or CL_MEM_OBJECT_IMAGE3D
         */

        virtual cl_mem_object_type GetChildMemObjectType() const = 0;

        /**
         * @fn  size_t D3D9Resource::GetMemObjSize() const = 0;
         *
         * @author  Aharon
         * @date    7/7/2011
         *
         * @return  The memory object size in bytes
         */

        virtual size_t GetMemObjSize() const = 0;

        /**
         * @fn  DWORD D3D9Resource::GetD3D9Flags() const
         *
         * @brief   Gets the Direct3D 9 flags.
         *
         * @author  Aharon
         * @date    7/14/2011
         *
         * @return  The Direct3D 9 flags.
         */

        DWORD GetD3D9Flags() const
        {
            if (m_clFlags & CL_MEM_READ_ONLY)
                return D3DLOCK_READONLY;
            return 0;
        }

        // inherited methods:

        virtual cl_err_code CreateChildObject()
        {
            return CL_SUCCESS;
        }

    private:

        // do not implement
        D3D9Resource(const D3D9Resource&);
        D3D9Resource& operator=(const D3D9Resource&);

    };

    /**
     * @class   D3D9Buffer
     *
     * @brief   This class provides the common implementation for Direct3D 9 buffers.
     *
     * @author  Aharon
     * @date    7/20/2011
     *
     * @sa  Intel::OpenCL::Framework::D3D9Resource
     *
     * ### param    RESOURCE_TYPE   the concrete sub-type of IDirect3DResource9 of the underlying
     *                              Direct3D 9 resource.
     * ### param    DESC_TYPE       the type of the output parameter of the resource's GetDesc
     *                              methods.
     */

    template<typename RESOURCE_TYPE, typename DESC_TYPE>
    class D3D9Buffer : public D3D9Resource
    {

    public:

        /**
         * @fn  D3D9Buffer::D3D9Buffer(Context* pContext, ocl_entry_points* pOclEntryPoints)
         *
         * @brief   Constructor.
         *
         * @author  Aharon
         * @date    7/20/2011
         */

        D3D9Buffer(Context* pContext, ocl_entry_points* pOclEntryPoints, cl_mem_object_type clObjType) :
          D3D9Resource(pContext, pOclEntryPoints, clObjType) { }

        // inherited methods:

        bool IsCreatedInD3DPoolDefault(D3D9ResourceInfo& resourceInfo) const;

        virtual size_t GetRowPitchSize() const { return 0; }

        virtual size_t GetSlicePitchSize() const { return 0; }

        virtual size_t GetPixelSize() const { return 0; }

        virtual cl_err_code CheckBounds(const size_t* pszOrigin, const size_t* pszRegion) const;

    protected:

        cl_mem_object_type GetChildMemObjectType() const { return CL_MEM_OBJECT_BUFFER; }

        size_t GetMemObjSize() const;

        void* Lock();

        void Unlock();

        void FillDimensions(const D3D9ResourceInfo& resourceInfo, size_t* const pszDims) const;

        virtual cl_err_code GetImageInfoInternal(const cl_image_info clParamName, size_t& szSize,
            void* pParamValue, const size_t szParamValueSize) const
        {
            return CL_INVALID_MEM_OBJECT;
        }

    };

    /**
     * @class   D3D9VertexBuffer
     *
     * @brief   This class represents a Direct3D 9 vertex buffer
     *
     * @author  Aharon
     * @date    7/6/2011
     *
     * @sa  Intel::OpenCL::Framework::D3D9Resource
     */

    typedef D3D9Buffer<IDirect3DVertexBuffer9, D3DVERTEXBUFFER_DESC> D3D9VertexBuffer;

    /**
     * @class   D3D9IndexBuffer
     *
     * @brief   This class represents a Direct3D 9 index buffer
     *
     * @author  Aharon
     * @date    7/18/2011
     *
     * @sa  Intel::OpenCL::Framework::D3D9Resource
     */

    typedef D3D9Buffer<IDirect3DIndexBuffer9, D3DINDEXBUFFER_DESC> D3D9IndexBuffer;

    /**
     * @class   D3D9Image2D
     *
     * @brief   This class provides the common implementation for Direct3D 9 2D images.
     *
     * @author  Aharon
     * @date    7/20/2011
     *
     * @sa  Intel::OpenCL::Framework::D3D9Resource
     *
     */

    class D3D9Image2D : public D3D9Resource
    {

        size_t m_szPitch;

    public:

        /**
         * @fn  D3D9Image2D::D3D9Image2D(Context* pContext, ocl_entry_points* pOclEntryPoints)
         *
         * @brief   Constructor.
         *
         * @author  Aharon
         * @date    7/20/2011
         */

        D3D9Image2D(Context* pContext, ocl_entry_points* pOclEntryPoints, cl_mem_object_type clObjType) :
          D3D9Resource(pContext, pOclEntryPoints, clObjType) { }

        // inherited methods:

        bool IsCreatedInD3DPoolDefault(D3D9ResourceInfo& resourceInfo) const;

        virtual size_t GetRowPitchSize() const { return m_szPitch; }

        virtual size_t GetSlicePitchSize() const { return 0; }

        virtual size_t GetPixelSize() const;

        virtual cl_err_code CheckBounds(const size_t* pszOrigin, const size_t* pszRegion) const;

    protected:

        /**
         * @fn  virtual bool D3D9Image2D::ObtainPitch(size_t& szPitch) = 0;
         *
         * @brief   Obtains the pitch (by locking and unlocking the resource)
         *
         * @author  Aharon
         * @date    7/20/2011
         *
         * @param [in,out]  szPitch  The pitch.
         *
         * @return  whether the operation has succeeded
         */

        virtual bool ObtainPitch(size_t& szPitch) = 0;

        /**
         * @fn  DESC_TYPE D3D9Image2D::GetDesc(const D3D9ResourceInfo& resourceInfo) const = 0;
         *
         * @brief   Gets a description.
         *
         * @author  Aharon
         * @date    7/20/2011
         *
         * @param   resourceInfo    Information describing the resource.
         *
         * @return  The description.
         */

        virtual D3DSURFACE_DESC GetDesc(const D3D9ResourceInfo& resourceInfo) const = 0;

        // inherited methods:

        virtual cl_err_code GetImageInfoInternal(const cl_image_info clParamName, size_t& szSize,
            void* pParamValue, const size_t szParamValueSize) const;

        virtual bool ObtainPitches();

        virtual const size_t* GetPitches() const { return &m_szPitch; }

        cl_mem_object_type GetChildMemObjectType() const { return CL_MEM_OBJECT_IMAGE2D; }

        size_t GetMemObjSize() const;

        void FillDimensions(const D3D9ResourceInfo& resourceInfo, size_t* const pszDims) const;

    };

    /**
     * @class   D3D9Surface
     *
     * @brief   This class represents a Direct3D 9 surface
     *
     * @author  Aharon
     * @date    7/19/2011
     *
     * @sa  Intel::OpenCL::Framework::D3D9Resource
     */

    class D3D9Surface : public D3D9Image2D
    {

    public:

        /**
         * @fn  D3D9Surface::D3D9Surface(Context* pContext, ocl_entry_points* pOclEntryPoints)
         *
         * @brief   Constructor.
         *
         * @author  Aharon
         * @date    7/19/2011
         */

        D3D9Surface(Context* pContext, ocl_entry_points* pOclEntryPoints, cl_mem_object_type clObjType) :
          D3D9Image2D(pContext, pOclEntryPoints, clObjType) { }

    protected:

        bool ObtainPitch(size_t& szPitch);

        void* Lock();

        void Unlock();

        D3DSURFACE_DESC GetDesc(const D3D9ResourceInfo& resourceInfo) const;

    };

    /**
     * @class   D3D9Texture
     *
     * @brief   This class represents a Direct3D 9 texture
     *
     * @author  Aharon
     * @date    7/20/2011
     *
     * @sa  Intel::OpenCL::Framework::D3D9Resource
     */

    class D3D9Texture : public D3D9Image2D
    {

    public:

        /**
         * @fn  D3D9Texture::D3D9Texture(Context* pContext, ocl_entry_points* pOclEntryPoints)
         *
         * @brief   Constructor.
         *
         * @author  Aharon
         * @date    7/20/2011
         */

        D3D9Texture(Context* pContext, ocl_entry_points* pOclEntryPoints, cl_mem_object_type clObjType) :
          D3D9Image2D(pContext, pOclEntryPoints, clObjType) { }

    protected:

        bool ObtainPitch(size_t& szPitch);

        void* Lock();

        void Unlock();

        D3DSURFACE_DESC GetDesc(const D3D9ResourceInfo& resourceInfo) const;

        virtual cl_err_code GetImageInfoInternal(const cl_image_info clParamName, size_t& szSize,
            void* pParamValue, const size_t szParamValueSize) const;

    };

    /**
     * @class   D3D9CubeTexture
     *
     * @brief   This class represents a Direct3D 9 cube texture
     *
     * @author  Aharon
     * @date    7/24/2011
     *
     * @sa  Intel::OpenCL::Framework::D3D9Image2D
     */

    class D3D9CubeTexture : public D3D9Image2D
    {

    public:

        D3D9CubeTexture(Context* pContext, ocl_entry_points* pOclEntryPoints, cl_mem_object_type clObjType) :
          D3D9Image2D(pContext, pOclEntryPoints, clObjType) { }

    protected:

        bool ObtainPitch(size_t& szPitch);

        void* Lock();

        void Unlock();

        D3DSURFACE_DESC GetDesc(const D3D9ResourceInfo& resourceInfo) const;

        virtual cl_err_code GetImageInfoInternal(const cl_image_info clParamName, size_t& szSize,
            void* pParamValue, const size_t szParamValueSize) const;

    };

    /**
     * @class   D3D9VolumeTexture
     *
     * @brief   This class represents a Direct3D 9 volume texture.
     *
     * @author  Aharon
     * @date    7/24/2011
     *
     * @sa  Intel::OpenCL::Framework::D3D9Resource
     */

    class D3D9VolumeTexture : public D3D9Resource
    {
        enum Pitches { ROW_PITCH , SLICE_PITCH };

        size_t m_szPitches[2];

    public:

        /**
         * @fn  D3D9VolumeTexture::D3D9VolumeTexture(Context* pContext, ocl_entry_points* pOclEntryPoints)
         *
         * @brief   Constructor.
         *
         * @author  Aharon
         * @date    7/24/2011
         */

        D3D9VolumeTexture(Context* pContext, ocl_entry_points* pOclEntryPoints, cl_mem_object_type clObjType) :
            D3D9Resource(pContext, pOclEntryPoints, clObjType) { }

        // inherited methods:

        virtual bool IsCreatedInD3DPoolDefault(D3D9ResourceInfo& resourceInfo) const;

        virtual void* Lock();

        virtual void Unlock();

        virtual void FillDimensions(const D3D9ResourceInfo& resourceInfo, size_t* const pszDims) const;

        virtual size_t GetRowPitchSize() const { return m_szPitches[ROW_PITCH]; }

        virtual size_t GetSlicePitchSize() const { return m_szPitches[SLICE_PITCH]; }

        virtual size_t GetPixelSize() const;

        virtual cl_err_code CheckBounds(const size_t* pszOrigin, const size_t* pszRegion) const;

        virtual cl_err_code GetImageInfoInternal(const cl_image_info clParamName, size_t& szSize,
            void* pParamValue, const size_t szParamValueSize) const;

    protected:

        virtual cl_mem_object_type GetChildMemObjectType() const { return CL_MEM_OBJECT_IMAGE3D; }

        virtual size_t GetMemObjSize() const;

        virtual bool ObtainPitches();

        virtual const size_t* GetPitches() const { return m_szPitches; }

    private:

        D3DVOLUME_DESC GetDesc(const D3D9ResourceInfo& resourceInfo) const;

    };

}}}
