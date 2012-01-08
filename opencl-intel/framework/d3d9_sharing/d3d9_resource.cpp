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

#include "d3d9_resource.h"
#include "cl_logger.h"

using namespace Intel::OpenCL::Utils;

namespace Intel { namespace OpenCL { namespace Framework
{

    // explicit instantiation of template classes

    template class D3D9Buffer<IDirect3DVertexBuffer9, D3DVERTEXBUFFER_DESC>;
    template class D3D9Buffer<IDirect3DIndexBuffer9, D3DINDEXBUFFER_DESC>;

    /**
     * @fn  D3D9Resource::~D3D9Resource()
     */

    D3D9Resource::~D3D9Resource()
    {
        if (m_bAcquired)
        {
            m_pChildObject->Release();
        }
        if (NULL != m_pResourceInfo)
        {
            (dynamic_cast<D3D9Context*>(m_pContext))->RemoveResourceInfo(*m_pResourceInfo);
            m_pResourceInfo->m_pResource->Release();
            delete m_pResourceInfo;
        }        
    }

    /**
     * @fn  cl_err_code D3D9Resource::Initialize(cl_mem_flags clMemFlags,
     *      const cl_image_format* pclImageFormat, unsigned int dim_count, const size_t* dimension,
     *      const size_t* pitches, void* pHostPtr)
     */

    cl_err_code D3D9Resource::Initialize(cl_mem_flags clMemFlags,
        const cl_image_format* pclImageFormat, unsigned int dim_count,
        const size_t* dimension, const size_t* pitches, void* pHostPtr)
    {
        m_pResourceInfo = (D3D9ResourceInfo*)pHostPtr;
        m_pResourceInfo->m_pResource->AddRef();
        m_clMemObjectType = GetChildMemObjectType();
        if (!ObtainPitches())
        {
            /* We obtain the pitches by locking and unlocking the resource. It also sounds
            reasonable that we shouldn't be able to create an OpenCL memory object from a Direct3D
            9 resource that has already been locked, otherwise we would need to call
            clEnqueueReleaseD3D9ObjectsIntel for it, but it would fail according to spec, because we
            haven't called clEnqueueAcquireD3D9ObjectsIntel for it. We should clarify this point in
            the spec. */
            return CL_DX9_RESOURCE_ALREADY_ACQUIRED_INTEL;
        }
        m_stMemObjSize = GetMemObjSize();
        m_clFlags = clMemFlags;
        m_uiNumDim = dim_count;
        if (NULL != pclImageFormat)
        {
            m_clImageFormat = *pclImageFormat;
        }
        memcpy_s(m_szDimensions, sizeof(m_szDimensions), dimension, dim_count * sizeof(m_szDimensions[0]));
        return CL_SUCCESS;
    }

    /**
     * @fn  cl_err_code D3D9Resource::ReadData(void* pOutData, const size_t* pszOrigin,
     *      const size_t* pszRegion, size_t szRowPitch, size_t szSlicePitch)
     */

    cl_err_code D3D9Resource::ReadData(void* pOutData, const size_t* pszOrigin,
        const size_t* pszRegion, size_t szRowPitch, size_t szSlicePitch)
    {
        if (NULL == m_pChildObject)
        {
            return CL_INVALID_VALUE;
        }
        return m_pChildObject->ReadData(pOutData, pszOrigin, pszRegion, szRowPitch, szSlicePitch);
    }

    /**
     * @fn  cl_err_code D3D9Resource::WriteData(const void* pOutData, const size_t* pszOrigin,
     *      const size_t* pszRegion, size_t szRowPitch, size_t szSlicePitch)
     */

    cl_err_code D3D9Resource::WriteData(const void* pOutData, const size_t* pszOrigin,
        const size_t* pszRegion, size_t szRowPitch, size_t szSlicePitch)
    {
        if (NULL == m_pChildObject)
        {
            return CL_INVALID_VALUE;
        }
        return m_pChildObject->WriteData(pOutData, pszOrigin, pszRegion, szRowPitch, szSlicePitch);
    }

    /**
     * @fn  cl_err_code D3D9Resource::CreateSubBuffer(cl_mem_flags clFlags,
     *      cl_buffer_create_type buffer_create_type, const void* buffer_create_info,
     *      MemoryObject** ppBuffer)
     */

    cl_err_code D3D9Resource::CreateSubBuffer(cl_mem_flags clFlags,
        cl_buffer_create_type buffer_create_type, const void* buffer_create_info,
        MemoryObject** ppBuffer)
    {
        if (NULL == m_pChildObject)
        {
            return CL_INVALID_VALUE;
        }
        return m_pChildObject->CreateSubBuffer(clFlags, buffer_create_type, buffer_create_info, ppBuffer);
    }

    /**
     * @fn  FissionableDevice* D3D9Resource::GetLocation() const
     */

    FissionableDevice* D3D9Resource::GetLocation() const
    {
        if (NULL == m_pChildObject)
        {
            return NULL;
        }
        return m_pChildObject->GetLocation();
    }

    /**
     * @fn  void D3D9Resource::AcquireD3D9()
     */

    void D3D9Resource::AcquireD3D9()
    {
        Intel::OpenCL::Utils::OclAutoMutex mtx(&m_muAcquireRelease, false);
        
        if (NULL != m_pChildObject && CL_SUCCEEDED(GetAcquireState()))
        {
            // We have already acquired an object
            return;
        }
        m_muAcquireRelease.Lock();
        void* const pData = Lock();
        if (NULL == pData)
        {
            SetAcquireState(CL_INVALID_OPERATION);
            return;
        }
        // Now we need to create child object
        MemoryObject* pChild;
        cl_err_code res =
            MemoryObjectFactory::GetInstance()->CreateMemoryObject(CL_DEVICE_TYPE_CPU,
            GetChildMemObjectType(), CL_MEMOBJ_GFX_SHARE_NONE, m_pContext, &pChild);
        if (CL_FAILED(res))
        {
            Unlock();
            SetAcquireState(res);
            return;
        }
        res = pChild->Initialize(m_clFlags, &m_clImageFormat, m_uiNumDim, m_szDimensions, GetPitches(), pData);
        if (CL_FAILED(res))
        {
            Unlock();
            SetAcquireState(CL_OUT_OF_RESOURCES);
            return;
        }
        m_pChildObject.exchange(pChild);
        pChild->AddPendency(this);
    }

    /**
     * @fn  void D3D9Resource::ReleaseD3D9()
     */

    void D3D9Resource::ReleaseD3D9()
    {   
        m_muAcquireRelease.Lock();
        MemoryObject* const pChild = m_pChildObject.exchange(NULL);
        assert(NULL != pChild);
        pChild->RemovePendency(this);
        pChild->Release();
        Unlock();
        m_muAcquireRelease.Unlock();
    }

    /**
     * @fn  cl_image_format D3D9Resource::MapD3DFormat2OclFormat(const D3DFORMAT d3dFormat,
     *      unsigned int uiPlane)
     */

    cl_image_format D3D9Resource::MapD3DFormat2OclFormat(const D3DFORMAT d3dFormat, unsigned int uiPlane)
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
#if defined DX9_SHARING
        case D3DFMT_V16U16:
            clFormat.image_channel_order = CL_RG;
            clFormat.image_channel_data_type = CL_SNORM_INT16;
            break;
#endif
        case D3DFMT_A8L8:
            clFormat.image_channel_order = CL_RG;
            clFormat.image_channel_data_type = CL_UNORM_INT8;
            break;
#if defined DX9_SHARING
        case D3DFMT_V8U8:
            clFormat.image_channel_order = CL_RG;
            clFormat.image_channel_data_type = CL_SNORM_INT8;
            break;
#endif
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
#if defined DX9_SHARING
        case D3DFMT_Q16W16V16U16:
            clFormat.image_channel_order = CL_RGBA;
            clFormat.image_channel_data_type = CL_SNORM_INT16;
            break;
#endif
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
#if defined DX9_SHARING
        case D3DFMT_Q8W8V8U8:
            clFormat.image_channel_order = CL_RGBA;
            clFormat.image_channel_data_type = CL_SNORM_INT8;
            break;
#endif
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
        }
        return clFormat;
    }

    /**
     * @fn  cl_err_code D3D9Resource::GetImageInfo(cl_image_info clParamName, size_t szParamValueSize,
     *      void* pParamValue, size_t* pszParamValueSizeRet)
     */

    cl_err_code D3D9Resource::GetImageInfo(cl_image_info clParamName, size_t szParamValueSize, void* pParamValue, size_t* pszParamValueSizeRet)
    {
        if (NULL == pParamValue && NULL == pszParamValueSizeRet)
        {
            return CL_INVALID_VALUE;
        }
        size_t szSize = 0;
        const cl_err_code clErrCode = GetImageInfoInternal(clParamName, szSize, pParamValue, szParamValueSize);
        if (CL_SUCCESS != clErrCode)
        {
            return clErrCode;
        }
        if (NULL != pParamValue && szParamValueSize < szSize)
        {
            return CL_INVALID_VALUE;
        }
        if (NULL != pszParamValueSizeRet)
        {
            *pszParamValueSizeRet = szSize;
        }
        // value has already been copied by GetImageInfoInternal
        return CL_SUCCESS;
    }

    /**
     * @fn  cl_err_code D3D9Resource::GetImageInfoInternal(const cl_image_info clParamName, size_t& szSize,
     *      void* pParamValue, const size_t szParamValueSize) const
     */

    cl_err_code D3D9Resource::GetImageInfoInternal(const cl_image_info clParamName, size_t& szSize,
        void* pParamValue, const size_t szParamValueSize) const
    {
        size_t szSizeTVar;
        const void* pValue;

        switch (clParamName)
        {
        case CL_IMAGE_FORMAT:
            szSize = sizeof(cl_image_format);
            pValue = &m_clImageFormat;
            break;
        case CL_IMAGE_ELEMENT_SIZE:
                szSize = sizeof(size_t);
                szSizeTVar = GetPixelSize();
                pValue = &szSizeTVar;
                break;
        default:
            return CL_INVALID_VALUE;
        }
        if (NULL != pParamValue && szSize > 0)
        {
            MEMCPY_S(pParamValue, szParamValueSize, pValue, szSize);
        }
        return CL_SUCCESS;
    }

    /**
     * @fn  template<typename RESOURCE_TYPE, typename DESC_TYPE> size_t D3D9Buffer<RESOURCE_TYPE, DESC_TYPE>::GetMemObjSize() const
     */

    template<typename RESOURCE_TYPE, typename DESC_TYPE>
    size_t D3D9Buffer<RESOURCE_TYPE, DESC_TYPE>::GetMemObjSize() const
    {
        DESC_TYPE desc;
        RESOURCE_TYPE* const pVertexBuffer =
            static_cast<RESOURCE_TYPE*>(GetResourceInfo()->m_pResource);
        const HRESULT res = pVertexBuffer->GetDesc(&desc);
        assert(D3D_OK == res);
        return desc.Size;
    }

    /**
     * @fn  template<typename RESOURCE_TYPE, typename DESC_TYPE> bool D3D9Buffer<RESOURCE_TYPE, DESC_TYPE>::IsCreatedInD3DPoolDefault(D3D9ResourceInfo& resourceInfo) const
     */

    template<typename RESOURCE_TYPE, typename DESC_TYPE>
    bool D3D9Buffer<RESOURCE_TYPE, DESC_TYPE>::IsCreatedInD3DPoolDefault(D3D9ResourceInfo& resourceInfo) const
    {
        DESC_TYPE desc;
        RESOURCE_TYPE* const pVertexBuffer =
            static_cast<RESOURCE_TYPE*>(resourceInfo.m_pResource);
        const HRESULT res = pVertexBuffer->GetDesc(&desc);
        assert(D3D_OK == res);
        return D3DPOOL_DEFAULT == desc.Pool;
    }

    /**
     * @fn  template<typename RESOURCE_TYPE, typename DESC_TYPE> void* D3D9Buffer<RESOURCE_TYPE, DESC_TYPE>::Lock()
     */

    template<typename RESOURCE_TYPE, typename DESC_TYPE>
    void* D3D9Buffer<RESOURCE_TYPE, DESC_TYPE>::Lock()
    {
        void* pData;
        RESOURCE_TYPE* const pVertexBuffer =
            static_cast<RESOURCE_TYPE*>(GetResourceInfo()->m_pResource);
        const HRESULT res = pVertexBuffer->Lock(0, m_stMemObjSize, &pData, GetD3D9Flags());
        if (D3D_OK != res)
        {
            LOG_ERROR(TEXT("D3D9Buffer<RESOURCE_TYPE, DESC_TYPE>::Lock() failed with error code %d"), res);
            return NULL;
        }
        return pData;
    }

    /**
     * @fn  template<typename RESOURCE_TYPE, typename DESC_TYPE> void D3D9Buffer<RESOURCE_TYPE, DESC_TYPE>::Unlock()
     */

    template<typename RESOURCE_TYPE, typename DESC_TYPE>
    void D3D9Buffer<RESOURCE_TYPE, DESC_TYPE>::Unlock()
    {
        RESOURCE_TYPE* const pVertexBuffer =
            static_cast<RESOURCE_TYPE*>(GetResourceInfo()->m_pResource);
        const HRESULT res = pVertexBuffer->Unlock();
        if (D3D_OK != res)
        {
            LOG_ERROR(TEXT("D3D9Buffer<RESOURCE_TYPE, DESC_TYPE>::Unlock() failed with error code %d"), res);
        }
    }

    /**
     * @fn  template<typename RESOURCE_TYPE, typename DESC_TYPE> cl_err_code D3D9Buffer<RESOURCE_TYPE, DESC_TYPE>::CheckBounds(const size_t* pszOrigin,
     *      const size_t* pszRegion) const
     */

    template<typename RESOURCE_TYPE, typename DESC_TYPE>
    cl_err_code D3D9Buffer<RESOURCE_TYPE, DESC_TYPE>::CheckBounds(const size_t* pszOrigin, const size_t* pszRegion) const
    {
        return *pszOrigin + *pszRegion <= m_stMemObjSize ? CL_SUCCESS : CL_INVALID_VALUE ;
    }

    /**
     * @fn  template<typename RESOURCE_TYPE, typename DESC_TYPE> void D3D9Buffer::FillDimensions(const D3D9ResourceInfo& resourceInfo,
     *      size_t* const pszDims) const
     */

    template<typename RESOURCE_TYPE, typename DESC_TYPE>
    void D3D9Buffer<RESOURCE_TYPE, DESC_TYPE>::FillDimensions(const D3D9ResourceInfo& resourceInfo, size_t* const pszDims) const
    {
        RESOURCE_TYPE* const pVertexBuffer =
            static_cast<RESOURCE_TYPE*>(resourceInfo.m_pResource);
        DESC_TYPE desc;
        const HRESULT res = pVertexBuffer->GetDesc(&desc);
        assert(D3D_OK == res);
        pszDims[0] = desc.Size;            
    }

    /**
     * @fn  bool D3D9Image2D::IsCreatedInD3DPoolDefault(D3D9ResourceInfo& resourceInfo) const
     */

    bool D3D9Image2D::IsCreatedInD3DPoolDefault(D3D9ResourceInfo& resourceInfo) const
    {
        return D3DPOOL_DEFAULT == GetDesc(resourceInfo).Pool;
    }

    /**
     * @fn  size_t D3D9Image2D::GetPixelSize() const
     */

    size_t D3D9Image2D::GetPixelSize() const
    {
        const cl_image_format clFormat = MapD3DFormat2OclFormat(GetDesc(*GetResourceInfo()).Format);
        return m_pContext->GetPixelBytesCount(&clFormat);
    }    

    /**
     * @fn  cl_err_code D3D9Image2D::CheckBounds(const size_t* pszOrigin,
     *      const size_t* pszRegion) const
     */

    cl_err_code D3D9Image2D::CheckBounds(const size_t* pszOrigin, const size_t* pszRegion) const
    {
        const D3DSURFACE_DESC desc = GetDesc(*GetResourceInfo());
        if (pszOrigin[0] + pszRegion[0] > desc.Width)
        {
            return CL_INVALID_VALUE;
        }
        if (pszOrigin[1] + pszRegion[1] > desc.Height)
        {
            return CL_INVALID_VALUE;
        }
        return CL_SUCCESS;            
    }    

    /**
     * @fn  size_t D3D9Image2D::GetMemObjSize() const
     */

    size_t D3D9Image2D::GetMemObjSize() const
    {
        const D3DSURFACE_DESC desc = GetDesc(*GetResourceInfo());
        const cl_image_format clFormat = MapD3DFormat2OclFormat(desc.Format);
        return m_pContext->GetPixelBytesCount(&clFormat) * m_szPitch * desc.Height;
    }    

    /**
     * @fn  bool D3D9Image2D::ObtainPitches()
     */

    bool D3D9Image2D::ObtainPitches()
    {
        return ObtainPitch(m_szPitch);
    }

    /**
     * @fn  void D3D9Image2D::FillDimensions(const D3D9ResourceInfo& resourceInfo,
     *      size_t* const pszDims) const
     */

    void D3D9Image2D::FillDimensions(const D3D9ResourceInfo& resourceInfo, size_t* const pszDims) const
    {
        const D3DSURFACE_DESC desc = GetDesc(resourceInfo);        
        pszDims[0] = desc.Width;
        pszDims[1] = desc.Height;    
    }    

    /**
     * @fn  cl_err_code D3D9Image2D::GetImageInfoInternal(const cl_image_info clParamName, size_t& szSize,
     *      void* pParamValue, const size_t szParamValueSize) const
     */

    cl_err_code D3D9Image2D::GetImageInfoInternal(const cl_image_info clParamName, size_t& szSize,
        void* pParamValue, const size_t szParamValueSize) const
    {        
        size_t szSizeTVar;
        const void* pValue;

        switch (clParamName)
        {        
        case CL_IMAGE_ROW_PITCH:
                szSize = sizeof(size_t);
                pValue = &m_szPitch;
                break;            
        case CL_IMAGE_WIDTH:
                szSize = sizeof(size_t);
                szSizeTVar = GetDesc(*GetResourceInfo()).Width;
                pValue = &szSizeTVar;
                break;            
        case CL_IMAGE_HEIGHT:
                szSize = sizeof(size_t);
                szSizeTVar = GetDesc(*GetResourceInfo()).Height;
                pValue = &szSizeTVar;
                break;            
        case CL_IMAGE_DEPTH:
        case CL_IMAGE_SLICE_PITCH:
                szSize = sizeof(size_t);
                szSizeTVar = 0;
                pValue = &szSizeTVar;
                break;
#if defined DX9_SHARING
        case CL_IMAGE_D3D9_FACE_INTEL:
        case CL_IMAGE_D3D9_LEVEL_INTEL:
            // if sub-classes can handle this parameter, they should do it.
            return CL_INVALID_DX9_RESOURCE_INTEL;
#endif
        default:
            return D3D9Resource::GetImageInfoInternal(clParamName, szSize, pParamValue, szParamValueSize);
        }
        if (NULL != pParamValue && szSize > 0)
        {
            MEMCPY_S(pParamValue, szParamValueSize, pValue, szSize);
        }
        return CL_SUCCESS;
    }

    /**
     * @fn D3D9Surface::~D3D9Surface()
     */
    D3D9Surface::~D3D9Surface()
    {
        if (GetResourceInfo() != NULL)
        {
            IDirect3DSurface9* const pSurface = static_cast<IDirect3DSurface9*>(GetResourceInfo()->m_pResource);
            D3D9Context& context = *dynamic_cast<D3D9Context*>(m_pContext);
            if (NULL != context.GetSurfaceLocker(pSurface))
            {
                context.ReleaseSurfaceLocker(pSurface);
            }        
        }
    }

    /**
     * @fn  void* D3D9Surface::Lock()
     */

    void* D3D9Surface::Lock()
    {
        IDirect3DSurface9* const pSurface =
            static_cast<IDirect3DSurface9*>(GetResourceInfo()->m_pResource);
        D3DLOCKED_RECT lockedRect;
        HRESULT res;
        const D3DFORMAT format = GetDesc(*GetResourceInfo()).Format;
        if (MAKEFOURCC('N', 'V', '1', '2') != format && MAKEFOURCC('Y', 'V', '1', '2') != format)
        {
            res = pSurface->LockRect(&lockedRect, NULL, GetD3D9Flags());            
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
        SurfaceLocker* const pSurfaceLocker = dynamic_cast<D3D9Context*>(m_pContext)->GetSurfaceLocker(pSurface);
        void* const pData = pSurfaceLocker->Lock();
        assert(NULL != pData);
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

    /**
     * @fn  bool D3D9Surface::ObtainPitch(size_t& szPitch)
     */

    bool D3D9Surface::ObtainPitch(size_t& szPitch)
    {
        IDirect3DSurface9* const pSurface =
            static_cast<IDirect3DSurface9*>(GetResourceInfo()->m_pResource);
        const SurfaceLocker* const pSurfaceLocker = dynamic_cast<const D3D9Context*>(m_pContext)->GetSurfaceLocker(pSurface);
        if (NULL != pSurfaceLocker)
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
        HRESULT res = pSurface->LockRect(&lockedRect, NULL, GetD3D9Flags());
        if (D3D_OK != res)
        {
            return false;
        }
        szPitch = lockedRect.Pitch;
        res = pSurface->UnlockRect();
        assert(D3D_OK == res);
        return true;
    }

    /**
     * @fn  void D3D9Surface::Unlock()
     */

    void D3D9Surface::Unlock()
    {
        IDirect3DSurface9* const pSurface =
            static_cast<IDirect3DSurface9*>(GetResourceInfo()->m_pResource);
        SurfaceLocker* const pSurfaceLocker = dynamic_cast<D3D9Context*>(m_pContext)->GetSurfaceLocker(pSurface);
        if (NULL != pSurfaceLocker)
        {
            pSurfaceLocker->Unlock();
        }
        else
        {
            const HRESULT res = pSurface->UnlockRect();            
            assert(D3D_OK == res);
        }        
    }

    /**
     * @fn  D3DSURFACE_DESC GetDesc(D3D9ResourceInfo& resourceInfo) const
     */

    D3DSURFACE_DESC D3D9Surface::GetDesc(const D3D9ResourceInfo& resourceInfo) const
    {
        IDirect3DSurface9* const pSurface =
            static_cast<IDirect3DSurface9*>(resourceInfo.m_pResource);
        D3DSURFACE_DESC desc;
        const HRESULT res = pSurface->GetDesc(&desc);
        assert(D3D_OK == res);
        return desc;
    }

    /**
     * @fn cl_err_code D3D9Surface::GetImageInfoInternal(const cl_image_info clParamName, size_t& szSize,
     *       void* pParamValue, const size_t szParamValueSize) const
     */

    cl_err_code D3D9Surface::GetImageInfoInternal(const cl_image_info clParamName, size_t& szSize,
        void* pParamValue, const size_t szParamValueSize) const
    {
        const void* pValue;
        const UINT plane = dynamic_cast<const D3D9SurfaceResourceInfo*>(GetResourceInfo())->m_plane;
        const D3DFORMAT format = GetDesc(*GetResourceInfo()).Format;
        assert(0 == plane || 1 == plane || 2 == plane);
        size_t szHeight, szWidth;

        switch (clParamName)
        {
        case CL_IMAGE_DX9_PLANE_INTEL:
            szSize = sizeof(UINT);
            pValue = &plane;
            break;
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
            return D3D9Image2D::GetImageInfoInternal(clParamName, szSize, pParamValue, szParamValueSize);
        }            
        if (NULL != pParamValue && szSize > 0)
        {
            MEMCPY_S(pParamValue, szParamValueSize, pValue, szSize);
        }
        return CL_SUCCESS;
    }

    /**
     * @fn size_t D3D9Surface::GetPixelSize() const
     */

    size_t D3D9Surface::GetPixelSize() const
    {
        const cl_image_format clFormat = MapD3DFormat2OclFormat(GetDesc(*GetResourceInfo()).Format,
            dynamic_cast<const D3D9SurfaceResourceInfo*>(GetResourceInfo())->m_plane);
        return m_pContext->GetPixelBytesCount(&clFormat);
    }

    /**
     * @fn size_t D3D9Surface::GetMemObjSize() const
     */

    size_t D3D9Surface::GetMemObjSize() const
    {
        const D3DSURFACE_DESC desc = GetDesc(*GetResourceInfo());
        const UINT plane = dynamic_cast<const D3D9SurfaceResourceInfo*>(GetResourceInfo())->m_plane;
        const cl_image_format clFormat = MapD3DFormat2OclFormat(desc.Format, plane);
        switch (desc.Format)
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
            return m_pContext->GetPixelBytesCount(&clFormat) * GetPitch() * desc.Height;
        }        
    }

    /**
     * @fn cl_err_code D3D9Surface::CheckBounds(const size_t* pszOrigin, const size_t* pszRegion) const
     */
    
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
        return D3D9Image2D::CheckBounds(pszOrigin, pszRegion);
    }

    /**
     * @fn void D3D9Surface::FillDimensions(const D3D9ResourceInfo& resourceInfo, size_t* const pszDims) const
     */

    void D3D9Surface::FillDimensions(const D3D9ResourceInfo& resourceInfo, size_t* const pszDims) const
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

    /**
     * @fn  cl_err_code D3D9Texture::GetImageInfoInternal(const cl_image_info clParamName, size_t& szSize,
     *      void* pParamValue, const size_t szParamValueSize) const
     */

    cl_err_code D3D9Texture::GetImageInfoInternal(const cl_image_info clParamName, size_t& szSize,
        void* pParamValue, const size_t szParamValueSize) const
    {
        const void* pValue;

#if defined DX9_SHARING
        switch (clParamName)
        {
        case CL_IMAGE_D3D9_LEVEL_INTEL:
            szSize = sizeof(UINT);
            pValue = &dynamic_cast<const D3D9TextureResourceInfo&>(GetResourceInfo()).m_uiMipLevel;
            break;
        default:
#endif
            return D3D9Image2D::GetImageInfoInternal(clParamName, szSize, pParamValue, szParamValueSize);
#if defined DX9_SHARING
        }
#endif
        if (NULL != pParamValue && szSize > 0)
        {
            MEMCPY_S(pParamValue, szParamValueSize, pValue, szSize);
        }
        return CL_SUCCESS;
    }

    /**
     * @fn  bool D3D9Texture::ObtainPitch(size_t& szPitch)
     */

    bool D3D9Texture::ObtainPitch(size_t& szPitch)
    {
        const D3D9TextureResourceInfo& resourceInfo =
            dynamic_cast<const D3D9TextureResourceInfo&>(*GetResourceInfo());
        IDirect3DTexture9* const pTexture =
            static_cast<IDirect3DTexture9*>(resourceInfo.m_pResource);
        D3DLOCKED_RECT lockedRect;
        HRESULT res = pTexture->LockRect(resourceInfo.m_uiMipLevel, &lockedRect, NULL, GetD3D9Flags());
        if (D3D_OK != res)
        {
            return false;
        }
        szPitch = lockedRect.Pitch;
        res = pTexture->UnlockRect(resourceInfo.m_uiMipLevel);
        assert(D3D_OK == res);
        return true;
    }

    /**
     * @fn  void* D3D9Texture::Lock()
     */

    void* D3D9Texture::Lock()
    {
        const D3D9TextureResourceInfo& resourceInfo =
            dynamic_cast<const D3D9TextureResourceInfo&>(*GetResourceInfo());
        IDirect3DTexture9* const pTexture =
            static_cast<IDirect3DTexture9*>(resourceInfo.m_pResource);
        D3DLOCKED_RECT lockedRect;
        const HRESULT res = pTexture->LockRect(resourceInfo.m_uiMipLevel, &lockedRect, NULL, GetD3D9Flags());
        assert(D3D_OK == res);
        return lockedRect.pBits;
    }

    /**
     * @fn  void D3D9Texture::Unlock()
     */

    void D3D9Texture::Unlock()
    {
        const D3D9TextureResourceInfo& resourceInfo =
            dynamic_cast<const D3D9TextureResourceInfo&>(*GetResourceInfo());
        IDirect3DTexture9* const pTexture =
            static_cast<IDirect3DTexture9*>(resourceInfo.m_pResource);
        const HRESULT res = pTexture->UnlockRect(resourceInfo.m_uiMipLevel);
        assert(D3D_OK == res);
    }

    /**
     * @fn  D3DSURFACE_DESC D3D9Texture::GetDesc(const D3D9ResourceInfo& resourceInfo) const }}}
     */

    D3DSURFACE_DESC D3D9Texture::GetDesc(const D3D9ResourceInfo& resourceInfo) const
    {
        const D3D9TextureResourceInfo& textureResourceInfo =
            dynamic_cast<const D3D9TextureResourceInfo&>(resourceInfo);
        IDirect3DTexture9* const pTexture =
            static_cast<IDirect3DTexture9*>(resourceInfo.m_pResource);
        D3DSURFACE_DESC desc;
        const HRESULT res = pTexture->GetLevelDesc(textureResourceInfo.m_uiMipLevel, &desc);
        assert(D3D_OK == res);
        return desc;
    }

    /**
     * @fn  cl_err_code D3D9CubeTexture::GetImageInfoInternal(const cl_image_info clParamName, size_t& szSize,
     *      void* pParamValue, const size_t szParamValueSize) const
     */

    cl_err_code D3D9CubeTexture::GetImageInfoInternal(const cl_image_info clParamName, size_t& szSize,
        void* pParamValue, const size_t szParamValueSize) const
    {
        const void* pValue;

#if defined DX9_SHARING
        switch (clParamName)
        {
        case CL_IMAGE_D3D9_LEVEL_INTEL:
            szSize = sizeof(UINT);
            pValue = &dynamic_cast<const D3D9TextureResourceInfo&>(GetResourceInfo()).m_uiMipLevel;
            break;
        case CL_IMAGE_D3D9_FACE_INTEL:
            szSize = sizeof(D3DCUBEMAP_FACES);
            pValue = &dynamic_cast<const D3D9CubeTextureResourceInfo&>(GetResourceInfo()).m_facetype;
            break;
        default:
#endif
            return D3D9Image2D::GetImageInfoInternal(clParamName, szSize, pParamValue, szParamValueSize);
#if defined DX9_SHARING
        }
#endif
        if (NULL != pParamValue && szSize > 0)
        {
            MEMCPY_S(pParamValue, szParamValueSize, pValue, szSize);
        }
        return CL_SUCCESS;
    }

    /**
     * @fn  bool D3D9CubeTexture::ObtainPitch(size_t& szPitch)
     */

    bool D3D9CubeTexture::ObtainPitch(size_t& szPitch)
    {
        const D3D9CubeTextureResourceInfo& resourceInfo =
            dynamic_cast<const D3D9CubeTextureResourceInfo&>(*GetResourceInfo());
        IDirect3DCubeTexture9* const pCubeTexture =
            static_cast<IDirect3DCubeTexture9*>(resourceInfo.m_pResource);
        D3DLOCKED_RECT lockedRect;
        HRESULT res = pCubeTexture->LockRect(resourceInfo.m_facetype, resourceInfo.m_uiMipLevel,
            &lockedRect, NULL, GetD3D9Flags());
        if (D3D_OK != res)
        {
            return false;
        }
        szPitch = lockedRect.Pitch;
        res = pCubeTexture->UnlockRect(resourceInfo.m_facetype, resourceInfo.m_uiMipLevel);
        assert(D3D_OK == res);
        return true;
    }

    /**
     * @fn  void* D3D9CubeTexture::Lock()
     */

    void* D3D9CubeTexture::Lock()
    {
        const D3D9CubeTextureResourceInfo& resourceInfo =
            dynamic_cast<const D3D9CubeTextureResourceInfo&>(*GetResourceInfo());
        IDirect3DCubeTexture9* const pCubeTexture =
            static_cast<IDirect3DCubeTexture9*>(resourceInfo.m_pResource);
        D3DLOCKED_RECT lockedRect;
        const HRESULT res = pCubeTexture->LockRect(resourceInfo.m_facetype, resourceInfo.m_uiMipLevel,
            &lockedRect, NULL, GetD3D9Flags());
        assert(D3D_OK == res);
        return lockedRect.pBits;
    }

    /**
     * @fn  void D3D9CubeTexture::Unlock()
     */

    void D3D9CubeTexture::Unlock()
    {
        const D3D9CubeTextureResourceInfo& resourceInfo =
            dynamic_cast<const D3D9CubeTextureResourceInfo&>(*GetResourceInfo());
        IDirect3DCubeTexture9* const pCubeTexture =
            static_cast<IDirect3DCubeTexture9*>(resourceInfo.m_pResource);
        const HRESULT res = pCubeTexture->UnlockRect(resourceInfo.m_facetype, resourceInfo.m_uiMipLevel);
        assert(D3D_OK == res);
    }

    /**
     * @fn  D3DSURFACE_DESC D3D9CubeTexture::GetDesc(const D3D9ResourceInfo& resourceInfo) const
     */

    D3DSURFACE_DESC D3D9CubeTexture::GetDesc(const D3D9ResourceInfo& resourceInfo) const
    {
        const D3D9CubeTextureResourceInfo& cubeTextureInfo =
            dynamic_cast<const D3D9CubeTextureResourceInfo&>(resourceInfo);
        IDirect3DCubeTexture9* const pCubeTexture =
            static_cast<IDirect3DCubeTexture9*>(resourceInfo.m_pResource);
        D3DSURFACE_DESC desc;
        const HRESULT res = pCubeTexture->GetLevelDesc(cubeTextureInfo.m_uiMipLevel, &desc);
        assert(D3D_OK == res);
        return desc;
    }

    /**
     * @fn  bool D3D9VolumeTexture::IsCreatedInD3DPoolDefault(D3D9ResourceInfo& resourceInfo) const
     */

    bool D3D9VolumeTexture::IsCreatedInD3DPoolDefault(D3D9ResourceInfo& resourceInfo) const
    {
        return D3DPOOL_DEFAULT == GetDesc(resourceInfo).Pool;
    }

    /**
     * @fn  void* D3D9VolumeTexture::Lock()
     */

    void* D3D9VolumeTexture::Lock()
    {
        const D3D9TextureResourceInfo& textureInfo =
            dynamic_cast<const D3D9TextureResourceInfo&>(*GetResourceInfo());
        IDirect3DVolumeTexture9* const pVolumeTexture =
            static_cast<IDirect3DVolumeTexture9*>(textureInfo.m_pResource);
        D3DLOCKED_BOX lockedVolume;
        const HRESULT res = pVolumeTexture->LockBox(textureInfo.m_uiMipLevel, &lockedVolume, NULL, GetD3D9Flags());
        assert(D3D_OK == res);
        return lockedVolume.pBits;
    }

    /**
     * @fn  void D3D9VolumeTexture::Unlock()
     */

    void D3D9VolumeTexture::Unlock()
    {
        const D3D9TextureResourceInfo& textureInfo =
            dynamic_cast<const D3D9TextureResourceInfo&>(*GetResourceInfo());
        IDirect3DVolumeTexture9* const pVolumeTexture =
            static_cast<IDirect3DVolumeTexture9*>(textureInfo.m_pResource);
        const HRESULT res = pVolumeTexture->UnlockBox(textureInfo.m_uiMipLevel);
        assert(D3D_OK == res);
    }

    /**
     * @fn  void D3D9VolumeTexture::FillDimensions(const D3D9ResourceInfo& resourceInfo,
     *      size_t* const pszDims) const
     */

    void D3D9VolumeTexture::FillDimensions(const D3D9ResourceInfo& resourceInfo, size_t* const pszDims) const
    {
        const D3DVOLUME_DESC desc = GetDesc(resourceInfo);
        pszDims[0] = desc.Width;
        pszDims[1] = desc.Height;
        pszDims[2] = desc.Depth;
    }

    /**
     * @fn  size_t D3D9VolumeTexture::GetMemObjSize() const
     */

    size_t D3D9VolumeTexture::GetMemObjSize() const
    {
        const D3DVOLUME_DESC desc = GetDesc(*GetResourceInfo());
        const cl_image_format clImageFormat = MapD3DFormat2OclFormat(desc.Format);
        const size_t szPixelByteCnt = m_pContext->GetPixelBytesCount(&clImageFormat);
        return szPixelByteCnt * m_szPitches[0] * m_szPitches[1] * desc.Depth;
    }

    /**
     * @fn  bool D3D9VolumeTexture::ObtainPitches()
     */

    bool D3D9VolumeTexture::ObtainPitches()
    {
        const D3D9TextureResourceInfo& textureInfo =
            dynamic_cast<const D3D9TextureResourceInfo&>(*GetResourceInfo());
        IDirect3DVolumeTexture9* const pVolumeTexture =
            static_cast<IDirect3DVolumeTexture9*>(textureInfo.m_pResource);
        D3DLOCKED_BOX lockedVolume;
        HRESULT res = pVolumeTexture->LockBox(textureInfo.m_uiMipLevel, &lockedVolume, NULL, GetD3D9Flags());
        if (D3D_OK != res)
        {
            return false;
        }
        res = pVolumeTexture->UnlockBox(textureInfo.m_uiMipLevel);
        assert(D3D_OK == res);
        m_szPitches[ROW_PITCH] = lockedVolume.RowPitch;
        m_szPitches[SLICE_PITCH] = lockedVolume.SlicePitch;
        return true;
    }

    /**
     * @fn  D3DVOLUME_DESC D3D9VolumeTexture::GetDesc(const D3D9ResourceInfo& resourceInfo) const
     */

    D3DVOLUME_DESC D3D9VolumeTexture::GetDesc(const D3D9ResourceInfo& resourceInfo) const
    {
        const D3D9TextureResourceInfo& textureInfo =
            dynamic_cast<const D3D9TextureResourceInfo&>(resourceInfo);
        IDirect3DVolumeTexture9* const pVolumeTexture =
            static_cast<IDirect3DVolumeTexture9*>(textureInfo.m_pResource);
        D3DVOLUME_DESC desc;
        const HRESULT res = pVolumeTexture->GetLevelDesc(textureInfo.m_uiMipLevel, &desc);
        assert(D3D_OK == res);
        return desc;
    }

    /**
     * @fn  size_t D3D9VolumeTexture::GetPixelSize() const
     */

    size_t D3D9VolumeTexture::GetPixelSize() const
    {
        const D3DVOLUME_DESC desc = GetDesc(*GetResourceInfo());
        const cl_image_format clImageFormat = MapD3DFormat2OclFormat(desc.Format);
        return m_pContext->GetPixelBytesCount(&clImageFormat);
    }

    /**
     * @fn  cl_err_code D3D9VolumeTexture::CheckBounds(const size_t* pszOrigin,
     *      const size_t* pszRegion) const
     */

    cl_err_code D3D9VolumeTexture::CheckBounds(const size_t* pszOrigin, const size_t* pszRegion) const
    {
        const D3DVOLUME_DESC desc = GetDesc(*GetResourceInfo());

        if (pszOrigin[0] + pszRegion[0] > desc.Width)
        {
            return CL_INVALID_VALUE;
        }
        if (pszOrigin[1] + pszRegion[1] > desc.Height)
        {
            return CL_INVALID_VALUE;
        }
        if (pszOrigin[2] + pszRegion[2] > desc.Depth)
        {
            return CL_INVALID_VALUE;
        }
        return CL_SUCCESS;
    }

    /**
     * @fn  cl_err_code D3D9VolumeTexture::GetImageInfoInternal(const cl_image_info clParamName,
     *      size_t& szSize, void* pParamValue, const size_t szParamValueSize) const
     */

    cl_err_code D3D9VolumeTexture::GetImageInfoInternal(const cl_image_info clParamName, size_t& szSize,
        void* pParamValue, const size_t szParamValueSize) const
    {
        size_t szSizeTVar;
        const void* pValue;

        switch (clParamName)
        {        
        case CL_IMAGE_ROW_PITCH:
            szSize = sizeof(size_t);
            pValue = &m_szPitches[ROW_PITCH];
            break;
        case CL_IMAGE_SLICE_PITCH:
            szSize = sizeof(size_t);
            pValue = &m_szPitches[SLICE_PITCH];
            break;
        case CL_IMAGE_WIDTH:
            szSize = sizeof(size_t);
            szSizeTVar = GetDesc(*GetResourceInfo()).Width;
            pValue = &szSizeTVar;
            break;
        case CL_IMAGE_HEIGHT:
            szSize = sizeof(size_t);
            szSizeTVar = GetDesc(*GetResourceInfo()).Height;
            pValue = &szSizeTVar;
            break;
        case CL_IMAGE_DEPTH:
            szSize = sizeof(size_t);
            szSizeTVar = GetDesc(*GetResourceInfo()).Depth;
            pValue = &szSizeTVar;
            break;
#if defined DX9_SHARING
        case CL_IMAGE_D3D9_LEVEL_INTEL:
            szSize = sizeof(UINT);
            pValue = &dynamic_cast<const D3D9TextureResourceInfo&>(GetResourceInfo()).m_uiMipLevel;
            break;
#endif
        default:
            return D3D9Resource::GetImageInfoInternal(clParamName, szSize, pParamValue, szParamValueSize);
        }
        if (NULL != pParamValue && szSize > 0)
        {
            MEMCPY_S(pParamValue, szParamValueSize, pValue, szSize);
        }
        return CL_SUCCESS;
    }

}}}
