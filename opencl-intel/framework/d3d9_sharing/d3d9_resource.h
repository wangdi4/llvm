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

#pragma once

#include <d3d9.h>
#include "GraphicsApiMemoryObject.h"
#include "d3d9_context.h"
#include "cl_synch_objects.h"
#include "ocl_event.h"
#include "d3d11_mapper.h"

namespace Intel { namespace OpenCL { namespace Framework
{
/**
 * @class   D3DResource
 *
 * @brief   This class represents a Direct3D resource
 *
 * @param RESOURCE_TYPE the super-type of all Direct 3D resources that can be created by this context
 * @param DEV_TYPE the type of the Direct 3D device
 *
 * @author  Aharon
 * @date    7/6/2011
 *
 * @sa  GraphicsApiMemoryObject
 */

template<typename RESOURCE_TYPE, typename DEV_TYPE>
class D3DResource : public GraphicsApiMemoryObject
{

    D3DResourceInfo<RESOURCE_TYPE>* m_pResourceInfo;
    size_t m_szDimensions[3];
    bool m_bAcquired;

public:

    PREPARE_SHARED_PTR(D3DResource)

    /**
     * @brief   Finaliser.
     *
     * @author  Aharon
     * @date    7/6/2011
     */

    virtual ~D3DResource();

    /**
     * @brief   Query if this object is has been created in a manner valid for sharing.
     *
     * @author  Aharon
     * @date    7/7/2011
     *
     * @param   resourceInfo    Information describing the resource (we give this parameter to
     * 							enable calling this method before having called Initialize).
     *
     * @return  true if created in a manner valid for sharing, false if not.
     */

    virtual bool IsValidlyCreated(D3DResourceInfo<RESOURCE_TYPE>& resourceInfo) const = 0;

    /**
     * @brief   Locks the shared region of the Direct3D object
     *
     * @author  Aharon
     * @date    7/13/2011
     *
     * @return  a pointer to the locked data
     */

    virtual void* Lock() = 0;

    /**
     * @brief   Unlocks the shared region of the Direct3D object
     *
     * @author  Aharon
     * @date    7/13/2011
     */

    virtual void Unlock() = 0;

    /**
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

    virtual void FillDimensions(const D3DResourceInfo<RESOURCE_TYPE>& resourceInfo, size_t* const pszDims) const = 0;

    /**
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
     * @brief   Gets the pitches.
     *
     * @author  Aharon
     * @date    7/24/2011
     *
     * @return  the pitches.
     */

    virtual const size_t* GetPitches() const { return nullptr; }

    /**
     * @brief   Acquires this object from Direct3D
     *
     * @author  Aharon
     * @date    7/13/2011
	 *
	 * @return whether the acquisition has succeeded
     */

    bool AcquireD3D();

    /**
     * @brief   Releases this object to Direct3D
     *
     * @author  Aharon
     * @date    7/13/2011
     */

    void ReleaseD3D();

    /**
     * @brief   Query if this object is acquired.
     *
     * @author  Aharon
     * @date    7/25/2011
     *
     * @return  true if acquired, false if not.
     */

    bool IsAcquired() const { return m_bAcquired; }

    /**
     * @brief   Set whether this object is acquired
     * 			
     * @param   bAcquired   whether this object is acquired			
     */
    void SetAcquired(bool bAcquired) { m_bAcquired = bAcquired; }

    /**
     * @brief   Gets the resource information.
     *
     * @author  Aharon
     * @date    7/7/2011
     *
     * @return  The resource information.
     */

    const D3DResourceInfo<RESOURCE_TYPE>* GetResourceInfo() const { return m_pResourceInfo; }

    /**
     * @param resourceInfo the information about the resource
     * @return the cl_image_format corresponding to the Direct3D image format of this D3DResource
     */
    virtual cl_image_format GetClImageFormat(const D3DResourceInfo<RESOURCE_TYPE>& resourceInfo) const
    {
        const cl_image_format clImgFormat = {0};
        return clImgFormat;
    }

    /**
     * @author  Aharon
     * @date    7/7/2011
     *
     * @return  either CL_MEM_OBJECT_BUFFER, CL_MEM_OBJECT_IMAGE2D or CL_MEM_OBJECT_IMAGE3D
     */

    virtual cl_mem_object_type GetChildMemObjectType() const = 0;

    // inherited methods:

    virtual cl_err_code Initialize(cl_mem_flags	clMemFlags,
        const cl_image_format*	pclImageFormat, unsigned int dim_count,
        const size_t* dimension, const size_t* pitches, void* pHostPtr, cl_rt_memobj_creation_flags	creation_flags);

    virtual cl_err_code CreateSubBuffer(cl_mem_flags clFlags,
        cl_buffer_create_type buffer_create_type, const void* buffer_create_info,
        SharedPtr<MemoryObject>* ppBuffer);

    virtual cl_err_code ReadData(void* pOutData, const size_t* pszOrigin,
        const size_t* pszRegion, size_t szRowPitch, size_t szSlicePitch);

    virtual cl_err_code WriteData(const void* pOutData, const size_t* pszOrigin,
        const size_t* pszRegion, size_t szRowPitch, size_t szSlicePitch);        

    cl_err_code GetImageInfo(cl_image_info clParamName, size_t szParamValueSize, void* pParamValue, size_t* pszParamValueSizeRet) const;

    cl_err_code GetDimensionSizes(size_t* pszRegion) const;

    virtual void GetLayout(size_t* dimensions, size_t* rowPitch, size_t* slicePitch) const;

protected:

    /**
     * @brief   constructor.
     *
     * @author  Aharon
     * @date    7/6/2011
     */

    D3DResource(SharedPtr<Context> pContext) :
         GraphicsApiMemoryObject(pContext), m_pResourceInfo(nullptr),
             m_bAcquired(false) { }              

    /**
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
     * @author  Aharon
     * @date    7/7/2011
     *
     * @return  The memory object size in bytes
     */

    virtual size_t GetMemObjSize() const = 0;        

    // inherited methods:

    virtual cl_err_code CreateChildObject()
    {
        return CL_SUCCESS;
    }

private:

    // do not implement
    D3DResource(const D3DResource&);
    D3DResource& operator=(const D3DResource&);

};

/**
 * @class   D3DImage2D
 *
 * @brief   This class provides the common implementation for Direct3D 2D images.
 *
 * @param RESOURCE_TYPE the super-type of all Direct 3D resources that can be created by this context
 * @param DEV_TYPE the type of the Direct 3D device
 * @param DESC_TYPE the type of the descriptor returned by GetDesc method of RESOURCE_TYPE
 *
 * @author  Aharon
 * @date    7/20/2011
 *
 * @sa  Intel::OpenCL::Framework::D3DResource
 *
 */

template<typename RESOURCE_TYPE, typename DEV_TYPE, typename DESC_TYPE>
class D3DImage2D : public D3DResource<RESOURCE_TYPE, DEV_TYPE>
{

    size_t m_szPitch;

public:

    // inherited methods:

    virtual size_t GetRowPitchSize() const { return m_szPitch; }

    virtual size_t GetSlicePitchSize() const { return 0; }

    virtual size_t GetPixelSize() const;

    virtual cl_err_code CheckBounds(const size_t* pszOrigin, const size_t* pszRegion) const;

    cl_mem_object_type GetChildMemObjectType() const { return CL_MEM_OBJECT_IMAGE2D; }

protected:

    /**
     * @brief   Constructor.
     *
     * @author  Aharon
     * @date    7/20/2011
     */

    D3DImage2D(SharedPtr<Context> pContext) : D3DResource<RESOURCE_TYPE, DEV_TYPE>(pContext) { }

    /**
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
     * @param resourceInfo D3DResourceInfo describing the resource
     * @return the width of the 2D image
     */
    virtual UINT GetWidth(const D3DResourceInfo<RESOURCE_TYPE>& resourceInfo) const = 0;

    /**
     * @param resourceInfo D3DResourceInfo describing the resource
     * @return the height of the 2D image
     */
    virtual UINT GetHeight(const D3DResourceInfo<RESOURCE_TYPE>& resourceInfo) const = 0;

    /**
     * @return the pitch	   
     */

    size_t GetPitch() const { return m_szPitch; }

    // inherited methods:

    virtual cl_err_code GetImageInfoInternal(const cl_image_info clParamName, size_t& szSize,
        void* pParamValue, const size_t szParamValueSize) const;

    virtual bool ObtainPitches();

    virtual const size_t* GetPitches() const { return &m_szPitch; }

    size_t GetMemObjSize() const;

    void FillDimensions(const D3DResourceInfo<RESOURCE_TYPE>& resourceInfo, size_t* const pszDims) const;

};

/**
 * @class   D3D9Surface
 *
 * @brief   This class represents a Direct3D 9 surface
 *
 * @author  Aharon
 * @date    7/19/2011
 *
 * @sa  Intel::OpenCL::Framework::D3DResource
 */

class D3D9Surface : public D3DImage2D<IDirect3DResource9, IDirect3DDevice9, D3DSURFACE_DESC>
{

public:

    PREPARE_SHARED_PTR(D3D9Surface)

    static SharedPtr<D3D9Surface> Allocate(SharedPtr<Context> pContext, cl_mem_object_type clObjType)
    {
        return SharedPtr<D3D9Surface>(new D3D9Surface(pContext, clObjType));
    }

    /**
     * @brief   Gets the Direct3D 9 flags.
     *
     * @param   clFlags the cl_mem_flags
     * 
     * @return  The Direct3D 9 flags.
     */

    static DWORD GetD3D9Flags(cl_mem_flags clFlags)
    {
        if (clFlags & CL_MEM_READ_ONLY)
            return D3DLOCK_READONLY;
        return 0;
    }

    /**
     * @brief   Gets the Direct3D 9 flags.
     *
     * @author  Aharon
     * @date    7/14/2011
     *
     * @return  The Direct3D 9 flags.
     */

    DWORD GetD3D9Flags() const
    {
        return GetD3D9Flags(m_clFlags);
    }

    /**
     * @brief   Destructor
     */
    ~D3D9Surface();

    // overriden methods:

    cl_image_format GetClImageFormat(const D3DResourceInfo<IDirect3DResource9>& resourceInfo) const;

    bool IsValidlyCreated(D3DResourceInfo<IDirect3DResource9>& resourceInfo) const;

protected:

    bool ObtainPitch(size_t& szPitch);

    void* Lock();

    void Unlock();

    UINT GetWidth(const D3DResourceInfo<IDirect3DResource9>& resourceInfo) const;

    UINT GetHeight(const D3DResourceInfo<IDirect3DResource9>& resourceInfo) const;

    virtual cl_err_code GetImageInfoInternal(const cl_image_info clParamName, size_t& szSize,
        void* pParamValue, const size_t szParamValueSize) const;

    virtual size_t GetPixelSize() const;

    size_t GetMemObjSize() const;

    cl_err_code CheckBounds(const size_t* pszOrigin, const size_t* pszRegion) const;

    void FillDimensions(const D3DResourceInfo<IDirect3DResource9>& resourceInfo, size_t* const pszDims) const;

private:

    /**
     * @brief   Constructor.
     *
     * @author  Aharon
     * @date    7/19/2011
     */

    D3D9Surface(SharedPtr<Context> pContext, cl_mem_object_type clObjType) :
      D3DImage2D<IDirect3DResource9, IDirect3DDevice9, D3DSURFACE_DESC>(pContext) { }

    D3DSURFACE_DESC GetDesc(const D3DResourceInfo<IDirect3DResource9>& resourceInfo) const;

};

/**
 * @class   D3D11Buffer
 *
 * @brief   This class provides the common implementation for Direct3D 11 buffers.
 *
 * @author  Aharon
 * @date    7/20/2011
 *
 * @sa  Intel::OpenCL::Framework::D3DResource
 */  

class D3D11Buffer : public D3DResource<ID3D11Resource, ID3D11Device>
{

public:

    PREPARE_SHARED_PTR(D3D11Buffer)

    static SharedPtr<D3D11Buffer> Allocate(SharedPtr<Context> pContext, cl_mem_object_type clObjType)
    {
        return SharedPtr<D3D11Buffer>(new D3D11Buffer(pContext, clObjType));
    }    

    // inherited methods:

    ~D3D11Buffer()
    {
        if (m_pBufferMapper)
        {
            delete m_pBufferMapper;
        }
    }

    virtual cl_err_code Initialize(cl_mem_flags	clMemFlags, const cl_image_format* pclImageFormat, unsigned int dim_count, const size_t* dimension,
        const size_t* pitches, void* pHostPtr, cl_rt_memobj_creation_flags	creation_flags);

    bool IsValidlyCreated(D3DResourceInfo<ID3D11Resource>& resourceInfo) const;

    virtual size_t GetRowPitchSize() const { return 0; }

    virtual size_t GetSlicePitchSize() const { return 0; }

    virtual size_t GetPixelSize() const { return 0; }

    virtual cl_err_code CheckBounds(const size_t* pszOrigin, const size_t* pszRegion) const;

    cl_mem_object_type GetChildMemObjectType() const { return CL_MEM_OBJECT_BUFFER; }

protected:        

    size_t GetMemObjSize() const;

    void* Lock();

    void Unlock();

    void FillDimensions(const D3DResourceInfo<ID3D11Resource>& resourceInfo, size_t* const pszDims) const;

    virtual cl_err_code GetImageInfoInternal(const cl_image_info clParamName, size_t& szSize,
        void* pParamValue, const size_t szParamValueSize) const
    {
        return CL_INVALID_MEM_OBJECT;
    }

private:

    /**
     * @brief   Constructor.
     *
     * @author  Aharon
     * @date    7/20/2011
     */

    D3D11Buffer(SharedPtr<Context> pContext, cl_mem_object_type clObjType) :
      D3DResource<ID3D11Resource, ID3D11Device>(pContext), m_pBufferMapper(nullptr) { }

    D3d11BufferMapper* m_pBufferMapper;
    // disable possibility to create two instances of D3D11Buffer with the same m_pBufferMapper pointer.
    D3D11Buffer(const D3D11Buffer& s);
    D3D11Buffer& operator=(const D3D11Buffer& s);

};    

/**
 * This class represents a Direct3D 11 2D texture
 */
class D3D11Texture2D : public D3DImage2D<ID3D11Resource, ID3D11Device, D3D11_TEXTURE2D_DESC>
{

public:

    PREPARE_SHARED_PTR(D3D11Texture2D)

    static SharedPtr<D3D11Texture2D> Allocate(SharedPtr<Context> pContext, cl_mem_object_type clObjType)
    {
        return SharedPtr<D3D11Texture2D>(new D3D11Texture2D(pContext, clObjType));
    }    

    ~D3D11Texture2D()
    {
        if (m_pTexture2DMapper)
        {
            delete m_pTexture2DMapper;
        }
    }

    // overriden methods:

    virtual cl_err_code Initialize(cl_mem_flags	clMemFlags, const cl_image_format* pclImageFormat, unsigned int dim_count, const size_t* dimension,
        const size_t* pitches, void* pHostPtr, cl_rt_memobj_creation_flags	creation_flags);

    bool IsValidlyCreated(D3DResourceInfo<ID3D11Resource>& resourceInfo) const;

    virtual cl_image_format GetClImageFormat(const D3DResourceInfo<ID3D11Resource>& resourceInfo) const;

protected:

    bool ObtainPitch(size_t& szPitch);

    void* Lock();

    void Unlock();

    UINT GetWidth(const D3DResourceInfo<ID3D11Resource>& resourceInfo) const;

    UINT GetHeight(const D3DResourceInfo<ID3D11Resource>& resourceInfo) const;

    virtual cl_err_code GetImageInfoInternal(const cl_image_info clParamName, size_t& szSize, void* pParamValue, const size_t szParamValueSize) const;

private:

    /**
     * @brief   Constructor.
     *
     * @author  Aharon
     * @date    7/20/2011
     */

    D3D11Texture2D(SharedPtr<Context> pContext, cl_mem_object_type clObjType) :
      D3DImage2D<ID3D11Resource, ID3D11Device, D3D11_TEXTURE2D_DESC>(pContext), m_pTexture2DMapper(nullptr) { }

    D3d11Texture2DMapper* m_pTexture2DMapper;
    // disable possibility to create two instances of D3D11Texture2D with the same m_pTexture2DMapper pointer.
    D3D11Texture2D(const D3D11Texture2D& s);
    D3D11Texture2D& operator=(const D3D11Texture2D& s);

};

/**
 * This class represents a Direct3D 11 3D texture
 */
class D3D11Texture3D :  public D3DResource<ID3D11Resource, ID3D11Device>
{

public:

    PREPARE_SHARED_PTR(D3D11Texture3D)

    static SharedPtr<D3D11Texture3D> Allocate(SharedPtr<Context> pContext, cl_mem_object_type clObjType)
    {
        return SharedPtr<D3D11Texture3D>(new D3D11Texture3D(pContext, clObjType));
    }    

    /**
     * Destructor
     */
    ~D3D11Texture3D()
    {
        if (m_pTexture3DMapper)
        {
            delete m_pTexture3DMapper;
        }
    }

    // overridden methods:

    virtual cl_err_code Initialize(cl_mem_flags	clMemFlags, const cl_image_format* pclImageFormat, unsigned int dim_count, const size_t* dimension,
        const size_t* pitches, void* pHostPtr, cl_rt_memobj_creation_flags	creation_flags);

    bool IsValidlyCreated(D3DResourceInfo<ID3D11Resource>& resourceInfo) const;        

    virtual void* Lock();

    virtual void Unlock();

    virtual void FillDimensions(const D3DResourceInfo<ID3D11Resource>& resourceInfo, size_t* const pszDims) const;

    virtual bool ObtainPitches();

    virtual const size_t* GetPitches() const;

    virtual cl_image_format GetClImageFormat(const D3DResourceInfo<ID3D11Resource>& resourceInfo) const;

    virtual size_t GetRowPitchSize() const { return m_szPitch[0]; }

    virtual size_t GetSlicePitchSize() const { return m_szPitch[1]; }

    virtual size_t GetPixelSize() const
    {
        const cl_image_format imgFormat = GetClImageFormat(*GetResourceInfo());
        return clGetPixelBytesCount(&imgFormat);
    }

    virtual cl_err_code CheckBounds( const size_t* pszOrigin, const size_t* pszRegion) const;

    virtual cl_mem_object_type GetChildMemObjectType() const { return CL_MEM_OBJECT_IMAGE3D; }

protected:        

    virtual size_t GetMemObjSize() const;

    cl_err_code GetImageInfoInternal(const cl_image_info clParamName, size_t& szSize, void* pParamValue,
        const size_t szParamValueSize) const;

private:

    D3d11Texture3DMapper* m_pTexture3DMapper;
    size_t m_szPitch[2];

    /**
     * Constructor
     */
    D3D11Texture3D(SharedPtr<Context> pContext, cl_mem_object_type clObjType) :
      D3DResource<ID3D11Resource, ID3D11Device>(pContext), m_pTexture3DMapper(nullptr) { }

    D3D11Texture3D(const D3D11Texture3D& s);

    D3D11Texture3D& operator=(const D3D11Texture3D& s);

    UINT GetWidth(const D3DResourceInfo<ID3D11Resource>& resourceInfo) const;

    UINT GetHeight(const D3DResourceInfo<ID3D11Resource>& resourceInfo) const;

    UINT GetDepth(const D3DResourceInfo<ID3D11Resource>& resourceInfo) const;

};
    
template<typename RESOURCE_TYPE, typename DEV_TYPE>
D3DResource<RESOURCE_TYPE, DEV_TYPE>::~D3DResource()
{
    if (m_bAcquired)
    {
        m_itCurrentAcquriedObject->second->Release();
    }
    if (nullptr != m_pResourceInfo)
    {
        m_pContext.DynamicCast<D3DContext<RESOURCE_TYPE, DEV_TYPE>>()->RemoveResourceInfo(*m_pResourceInfo);
        m_pResourceInfo->m_pResource->Release();
        delete m_pResourceInfo;
    }        
}

template<typename RESOURCE_TYPE, typename DEV_TYPE>
cl_err_code D3DResource<RESOURCE_TYPE, DEV_TYPE>::Initialize(cl_mem_flags clMemFlags,
    const cl_image_format* pclImageFormat, unsigned int dim_count,
    const size_t* dimension, const size_t* pitches, void* pHostPtr, cl_rt_memobj_creation_flags	creation_flags)
{
    m_pResourceInfo = (D3DResourceInfo<RESOURCE_TYPE>*)pHostPtr;
    m_pResourceInfo->m_pResource->AddRef();
    m_clMemObjectType = GetChildMemObjectType();
    if (!ObtainPitches())
    {
        /* We obtain the pitches by locking and unlocking the resource. It also sounds
        reasonable that we shouldn't be able to create an OpenCL memory object from a Direct3D
        resource that has already been locked, otherwise we would need to call
        clEnqueueReleaseD3D for it, but it would fail according to spec, because we
        haven't called clEnqueueAcquireD3D for it. We should clarify this point in
        the spec. */
        return m_pContext.DynamicCast<D3DContext<RESOURCE_TYPE, DEV_TYPE>>()->GetD3dDefinitions().GetResourceAlreadyAcquired();
    }
    m_stMemObjSize = GetMemObjSize();
    m_clFlags = clMemFlags;
    m_uiNumDim = dim_count;
    if (nullptr != pclImageFormat)
    {
        m_clImageFormat = *pclImageFormat;
    }
    else
    {
        m_clImageFormat.image_channel_data_type = m_clImageFormat.image_channel_order = 0;
    }
    memcpy_s(m_szDimensions, sizeof(m_szDimensions), dimension, dim_count * sizeof(m_szDimensions[0]));
    return CL_SUCCESS;
}

template<typename RESOURCE_TYPE, typename DEV_TYPE>
cl_err_code D3DResource<RESOURCE_TYPE, DEV_TYPE>::ReadData(void* pOutData, const size_t* pszOrigin,
    const size_t* pszRegion, size_t szRowPitch, size_t szSlicePitch)
{
	Intel::OpenCL::Utils::OclAutoMutex mtx(&m_muAcquireRelease);
	if ( m_lstAcquiredObjectDescriptors.end() == m_itCurrentAcquriedObject )
	{
		return CL_INVALID_VALUE;
	}

	return m_itCurrentAcquriedObject->second->ReadData(pOutData, pszOrigin, pszRegion, szRowPitch, szSlicePitch);
}

template<typename RESOURCE_TYPE, typename DEV_TYPE>
cl_err_code D3DResource<RESOURCE_TYPE, DEV_TYPE>::WriteData(const void* pOutData, const size_t* pszOrigin,
    const size_t* pszRegion, size_t szRowPitch, size_t szSlicePitch)
{
	Intel::OpenCL::Utils::OclAutoMutex mtx(&m_muAcquireRelease);
	if ( m_lstAcquiredObjectDescriptors.end() == m_itCurrentAcquriedObject )
	{
		return CL_INVALID_VALUE;
	}

	return m_itCurrentAcquriedObject->second->WriteData(pOutData, pszOrigin, pszRegion, szRowPitch, szSlicePitch);
}

template<typename RESOURCE_TYPE, typename DEV_TYPE>
cl_err_code D3DResource<RESOURCE_TYPE, DEV_TYPE>::CreateSubBuffer(cl_mem_flags clFlags,
    cl_buffer_create_type buffer_create_type, const void* buffer_create_info,
    SharedPtr<MemoryObject>* ppBuffer)
{
	Intel::OpenCL::Utils::OclAutoMutex mtx(&m_muAcquireRelease);
	if ( m_lstAcquiredObjectDescriptors.end() == m_itCurrentAcquriedObject )
	{
		return CL_INVALID_VALUE;
	}

	return m_itCurrentAcquriedObject->second->CreateSubBuffer(clFlags, buffer_create_type, buffer_create_info, ppBuffer);
}

template<typename RESOURCE_TYPE, typename DEV_TYPE>
bool D3DResource<RESOURCE_TYPE, DEV_TYPE>::AcquireD3D()
{
    Intel::OpenCL::Utils::OclAutoMutex mtx(&m_muAcquireRelease);
    
	if ( m_lstAcquiredObjectDescriptors.end() != m_itCurrentAcquriedObject && 
        ( (CL_GFX_OBJECT_NOT_ACQUIRED != m_itCurrentAcquriedObject->second) &&
        (CL_GFX_OBJECT_NOT_READY != m_itCurrentAcquriedObject->second) &&
		    (CL_GFX_OBJECT_FAIL_IN_ACQUIRE != m_itCurrentAcquriedObject->second) )
		)
	{
        // We have already acquired an object
        return true;
	}

	void* const pData = Lock();
    if (nullptr == pData)
    {
		m_itCurrentAcquriedObject->second = CL_GFX_OBJECT_FAIL_IN_ACQUIRE;
        return false;
    }
    
	// Now we need to create child object
    SharedPtr<MemoryObject> pChild;
    cl_err_code res =
        MemoryObjectFactory::GetInstance()->CreateMemoryObject(CL_DEVICE_TYPE_CPU,
        GetChildMemObjectType(), CL_MEMOBJ_GFX_SHARE_NONE, m_pContext, &pChild);
    if (CL_FAILED(res))
    {
        m_itCurrentAcquriedObject->second = CL_GFX_OBJECT_FAIL_IN_ACQUIRE;
        return false;
    }

    if (m_clImageFormat.image_channel_data_type != 0 || m_clImageFormat.image_channel_order != 0)
    {
        res = pChild->Initialize(m_clFlags, &m_clImageFormat, m_uiNumDim, m_szDimensions, GetPitches(), pData, CL_RT_MEMOBJ_FORCE_BS);
    }
    else
    {
        res = pChild->Initialize(m_clFlags, nullptr, m_uiNumDim, m_szDimensions, GetPitches(), pData, CL_RT_MEMOBJ_FORCE_BS);
    }
    
    if (CL_FAILED(res))
    {
        m_itCurrentAcquriedObject->second = CL_GFX_OBJECT_FAIL_IN_ACQUIRE;
        return false;
    }

    m_itCurrentAcquriedObject->second = pChild;
	return true;
}

template<typename RESOURCE_TYPE, typename DEV_TYPE>
void D3DResource<RESOURCE_TYPE, DEV_TYPE>::ReleaseD3D()
{   
    Unlock();
}

template<typename RESOURCE_TYPE, typename DEV_TYPE>
cl_err_code D3DResource<RESOURCE_TYPE, DEV_TYPE>::GetImageInfo(cl_image_info clParamName, size_t szParamValueSize, void* pParamValue, size_t* pszParamValueSizeRet) const
{
    if (nullptr == pParamValue && nullptr == pszParamValueSizeRet)
    {
        return CL_INVALID_VALUE;
    }
    size_t szSize = 0;
    const cl_err_code clErrCode = GetImageInfoInternal(clParamName, szSize, pParamValue, szParamValueSize);
    if (CL_SUCCESS != clErrCode)
    {
        return clErrCode;
    }
    if (nullptr != pParamValue && szParamValueSize < szSize)
    {
        return CL_INVALID_VALUE;
    }
    if (nullptr != pszParamValueSizeRet)
    {
        *pszParamValueSizeRet = szSize;
    }
    // value has already been copied by GetImageInfoInternal
    return CL_SUCCESS;
}

template<typename RESOURCE_TYPE, typename DEV_TYPE>
cl_err_code D3DResource<RESOURCE_TYPE, DEV_TYPE>::GetDimensionSizes(size_t* pszRegion) const
{
    FillDimensions(*GetResourceInfo(), pszRegion);
    return CL_SUCCESS;
}

template<typename RESOURCE_TYPE, typename DEV_TYPE>
void D3DResource<RESOURCE_TYPE, DEV_TYPE>::GetLayout(size_t* dimensions, size_t* rowPitch, size_t* slicePitch) const
{
    if (nullptr != dimensions)
    {
        GetDimensionSizes(dimensions);
    }
    
    const size_t* const pitches = GetPitches();
    const size_t szNumPitches = GetNumDimensions() - 1;
    assert(szNumPitches <= 2);
    if (1 <= szNumPitches && nullptr != rowPitch)
    {
        *rowPitch = pitches[0];
    }
    if (2 == szNumPitches && nullptr != slicePitch)
    {
        *slicePitch = pitches[1];
    }
}

template<typename RESOURCE_TYPE, typename DEV_TYPE>
cl_err_code D3DResource<RESOURCE_TYPE, DEV_TYPE>::GetImageInfoInternal(const cl_image_info clParamName, size_t& szSize,
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
    if (nullptr != pParamValue && szSize > 0)
    {
        MEMCPY_S(pParamValue, szParamValueSize, pValue, szSize);
    }
    return CL_SUCCESS;
}

template<typename RESOURCE_TYPE, typename DEV_TYPE, typename DESC_TYPE>
size_t D3DImage2D<RESOURCE_TYPE, DEV_TYPE, DESC_TYPE>::GetPixelSize() const
{
    const cl_image_format clFormat = GetClImageFormat(*GetResourceInfo());
    return clGetPixelBytesCount(&clFormat);
}    

template<typename RESOURCE_TYPE, typename DEV_TYPE, typename DESC_TYPE>
cl_err_code D3DImage2D<RESOURCE_TYPE, DEV_TYPE, DESC_TYPE>::CheckBounds(const size_t* pszOrigin, const size_t* pszRegion) const
{
    if (pszOrigin[0] + pszRegion[0] > GetWidth(*GetResourceInfo()))
    {
        return CL_INVALID_VALUE;
    }
    if (pszOrigin[1] + pszRegion[1] > GetHeight(*GetResourceInfo()))
    {
        return CL_INVALID_VALUE;
    }
    return CL_SUCCESS;            
}    

template<typename RESOURCE_TYPE, typename DEV_TYPE, typename DESC_TYPE>
size_t D3DImage2D<RESOURCE_TYPE, DEV_TYPE, DESC_TYPE>::GetMemObjSize() const
{
    const cl_image_format clFormat = GetClImageFormat(*GetResourceInfo());
    return clGetPixelBytesCount(&clFormat) * m_szPitch * GetHeight(*GetResourceInfo());
}    

template<typename RESOURCE_TYPE, typename DEV_TYPE, typename DESC_TYPE>
bool D3DImage2D<RESOURCE_TYPE, DEV_TYPE, DESC_TYPE>::ObtainPitches()
{
    return ObtainPitch(m_szPitch);
}

template<typename RESOURCE_TYPE, typename DEV_TYPE, typename DESC_TYPE>
void D3DImage2D<RESOURCE_TYPE, DEV_TYPE, DESC_TYPE>::FillDimensions(const D3DResourceInfo<RESOURCE_TYPE>& resourceInfo, size_t* const pszDims) const
{
    pszDims[0] = GetWidth(resourceInfo);
    pszDims[1] = GetHeight(resourceInfo);
}    

template<typename RESOURCE_TYPE, typename DEV_TYPE, typename DESC_TYPE>
cl_err_code D3DImage2D<RESOURCE_TYPE, DEV_TYPE, DESC_TYPE>::GetImageInfoInternal(const cl_image_info clParamName, size_t& szSize,
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
            szSizeTVar = GetWidth(*GetResourceInfo());
            pValue = &szSizeTVar;
            break;            
    case CL_IMAGE_HEIGHT:
            szSize = sizeof(size_t);
            szSizeTVar = GetHeight(*GetResourceInfo());
            pValue = &szSizeTVar;
            break;            
    case CL_IMAGE_DEPTH:
    case CL_IMAGE_SLICE_PITCH:
            szSize = sizeof(size_t);
            szSizeTVar = 0;
            pValue = &szSizeTVar;
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

}}}
