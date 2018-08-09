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

#include <d3d11.h>
#include "CL\cl.h"
#include "cl_logger.h"
#include "Logger.h"

using Intel::OpenCL::Utils::Logger;

namespace Intel { namespace OpenCL { namespace Framework
{

/**
 * This class is responsible for mapping and unmapping Direct3D 11 objects
 * @param RESOURCE_TYPE the super-type of all Direct 3D resources that can be created by this context
 * @param DESC_TYPE the type of the descriptor returned by GetDesc method of RESOURCE_TYPE
 */
template<typename RESOURCE_TYPE, typename DESC_TYPE>
class D3d11Mapper
{

public:

    /**
     * @param memFlags cl_mem_flags of the OpenCL shared object
     * @return the D3D11_MAP value corresponding to memFlags
     */
    static D3D11_MAP GetD3d11Map(cl_mem_flags memFlags);

    /**
     * Destructor
     */
    virtual ~D3d11Mapper();

    /**
     * map the Direct3D 11 object
     * @return a pointer to the object's data
     */
    void* Map();

    /**
     * unmap the Direct3D 11 object
     */
    void Unmap();    

protected:

    DECLARE_LOGGER_CLIENT;

    /**
     * Constructor
     * @param pResource the Direct3D 11 object to map
     * @param mapType a D3D11_MAP-typed value that specifies the CPU's read and write permissions for a resource
     * @param uiSubresource index number of the subresource
     */
    D3d11Mapper(RESOURCE_TYPE* pResource, D3D11_MAP mapType, UINT uiSubresource = 0);

    /**
     * @param a DESC_type structure that describes a resource
     * @return a pointer to a the created resource
     */
    virtual RESOURCE_TYPE* CreateResource(const DESC_TYPE& desc) = 0;

    /**
     * @return the ID3D11Device that create the resource
     */
    ID3D11Device& GetDevice() { return *m_pDevice; }

    /**
     * @return a D3D11_MAPPED_SUBRESOURCE of the resource. In case of an error an all-zero structure is returned.
     */
    D3D11_MAPPED_SUBRESOURCE GetMappedSubresource() const { return m_mappedSubresource; }

protected:

	/**
	 * index number of the subresource
	 */
	const UINT m_uiSubresource;

private:

    RESOURCE_TYPE& m_resource;    
    const D3D11_MAP m_mapType;
    
    ID3D11Device* m_pDevice;
    ID3D11DeviceContext* m_pImmediateContext;
    RESOURCE_TYPE* m_pStagingResource;
    D3D11_MAPPED_SUBRESOURCE m_mappedSubresource;

    void* Map(RESOURCE_TYPE& resource, UINT uiSubresource);

    bool CopyToStagingResource();

};

/**
 * This class extends D3d11Mapper for ID3D11Buffer
 */
class D3d11BufferMapper : public D3d11Mapper<ID3D11Buffer, D3D11_BUFFER_DESC>
{

public:

    /**
     * Constructor
     * @param pResource the Direct3D 11 object to map
     * @param mapType a D3D11_MAP-typed value that specifies the CPU's read and write permissions for a resource
     */
    D3d11BufferMapper(ID3D11Buffer* pResource, D3D11_MAP mapType) : D3d11Mapper<ID3D11Buffer, D3D11_BUFFER_DESC>(pResource, mapType)
    {
        Map();
        Unmap();
    }

    /**
     * Destructor
     */
    virtual ~D3d11BufferMapper(){};

protected:

    virtual ID3D11Buffer* CreateResource(const D3D11_BUFFER_DESC& desc);

};

/**
 * This class extends D3d11Mapper for ID3D11Texture2D
 */
class D3d11Texture2DMapper : public D3d11Mapper<ID3D11Texture2D, D3D11_TEXTURE2D_DESC>
{

public:

    /**
     * Constructor
     * @param pResource the Direct3D 11 object to map
     * @param mapType a D3D11_MAP-typed value that specifies the CPU's read and write permissions for a resource
     * @param uiSubresource index number of the subresource
     */
    D3d11Texture2DMapper(ID3D11Texture2D* pResource, D3D11_MAP mapType, UINT uiSubresource) :
      D3d11Mapper<ID3D11Texture2D, D3D11_TEXTURE2D_DESC>(pResource, mapType, uiSubresource)
    {
        Map();
        Unmap();
        m_uiRowPitch = GetMappedSubresource().RowPitch;
    }

    /**
     * Destructor
     */
    virtual ~D3d11Texture2DMapper(){};

    /**
     * @return the row pitch of the 2D texture
     */
    UINT GetRowPitch() const { return m_uiRowPitch; }

protected:

    virtual ID3D11Texture2D* CreateResource(const D3D11_TEXTURE2D_DESC& desc);

private:

    UINT m_uiRowPitch;

};

/**
 * This class extends D3d11Mapper for ID3D11Texture3D
 */
class D3d11Texture3DMapper : public D3d11Mapper<ID3D11Texture3D, D3D11_TEXTURE3D_DESC>
{

public:

    /**
     * Constructor
     * @param pResource the Direct3D 11 object to map
     * @param mapType a D3D11_MAP-typed value that specifies the CPU's read and write permissions for a resource
     * @param uiSubresource index number of the subresource
     */
    D3d11Texture3DMapper(ID3D11Texture3D* pResource, D3D11_MAP mapType, UINT uiSubresource) :
      D3d11Mapper<ID3D11Texture3D, D3D11_TEXTURE3D_DESC>(pResource, mapType, uiSubresource)
    {
        Map();
        Unmap();
        const D3D11_MAPPED_SUBRESOURCE d3d11MappedSubresource = GetMappedSubresource();
        m_uiRowPitch = d3d11MappedSubresource.RowPitch;
        m_uiDepthPitch = d3d11MappedSubresource.DepthPitch;
    }

    /**
     * Destructor
     */
    virtual ~D3d11Texture3DMapper(){};

    /**
     * @return the row pitch of the 3D texture
     */
    UINT GetRowPitch() const { return m_uiRowPitch; }

    /**
     * @return the depth pitch of the 3D texture
     */
    UINT GetDepthPitch() const { return m_uiDepthPitch; }

protected:

    virtual ID3D11Texture3D* CreateResource(const D3D11_TEXTURE3D_DESC& desc);

private:

    UINT m_uiRowPitch;
    UINT m_uiDepthPitch;

};

template<typename RESOURCE_TYPE, typename DESC_TYPE>
D3d11Mapper<RESOURCE_TYPE, DESC_TYPE>::D3d11Mapper(RESOURCE_TYPE* pResource, D3D11_MAP mapType, UINT uiSubresource = 0) :
m_resource(*pResource), m_mapType(mapType), m_uiSubresource(uiSubresource), m_pDevice(nullptr), m_pImmediateContext(nullptr), m_pStagingResource(nullptr)
{
    INIT_LOGGER_CLIENT("D3d11Mapper", Intel::OpenCL::Utils::LL_ERROR);
    m_mappedSubresource.RowPitch = m_mappedSubresource.DepthPitch = 0;
    /* Constructors of sub-classes should call Map() and Unmap() to update m_mappedSubresource (they can't be called here, since Map() itself may call a pure virtual method,
       which isn't yet initialized at this stage). */
}

template<typename RESOURCE_TYPE, typename DESC_TYPE>
D3d11Mapper<RESOURCE_TYPE, DESC_TYPE>::~D3d11Mapper()
{
    RELEASE_LOGGER_CLIENT;
}

template<typename RESOURCE_TYPE, typename DESC_TYPE>
D3D11_MAP D3d11Mapper<RESOURCE_TYPE, DESC_TYPE>::GetD3d11Map(cl_mem_flags memFlags)
{
    switch (memFlags)
    {
    case CL_MEM_READ_ONLY:
        return D3D11_MAP_READ;
    case CL_MEM_WRITE_ONLY:
        return D3D11_MAP_WRITE;
    case CL_MEM_READ_WRITE:
    case 0: // the default is CL_MEM_READ_WRITE (this is specified for core object types, but we can conclude that this is the meaning here also)
        return D3D11_MAP_READ_WRITE;
    default:
        assert(false);
        return (D3D11_MAP)0;
    }
}

template<typename RESOURCE_TYPE, typename DESC_TYPE>
void* D3d11Mapper<RESOURCE_TYPE, DESC_TYPE>::Map()
{          
    assert(!m_pDevice && !m_pImmediateContext);
    m_resource.GetDevice(&m_pDevice);
    m_pDevice->GetImmediateContext(&m_pImmediateContext);

    DESC_TYPE desc;
    m_resource.GetDesc(&desc);    
    assert(D3D11_USAGE_IMMUTABLE != desc.Usage);
    // dynamic resources can only be written by CPU
    if (D3D11_USAGE_DYNAMIC == desc.Usage && D3D11_MAP_WRITE == m_mapType || D3D11_USAGE_STAGING == desc.Usage)
    {
        return Map(m_resource, m_uiSubresource);
    }
    else    // otherwise create a staging resource, copy the data of the original resource to it and map the staging resource
    {
        if (!CopyToStagingResource())
        {
            return nullptr;
        }
        return Map(*m_pStagingResource, 0);
    }    
}

template<typename RESOURCE_TYPE, typename DESC_TYPE>
void D3d11Mapper<RESOURCE_TYPE, DESC_TYPE>::Unmap()
{
    DESC_TYPE desc;
    m_resource.GetDesc(&desc);
    assert(D3D11_USAGE_IMMUTABLE != desc.Usage);    
    if (D3D11_USAGE_DYNAMIC == desc.Usage && D3D11_MAP_WRITE == m_mapType || D3D11_USAGE_STAGING == desc.Usage)
    {
        m_pImmediateContext->Unmap(&m_resource, 0);
    }
    else
    {
        m_pImmediateContext->Unmap(m_pStagingResource, 0);
        /* This action is asynchronous and we can't wait for its completion. If the user tries to read from the resource after this, the driver detects the read-after-write
            hazard and takes care of the synchronization. */
        m_pImmediateContext->CopySubresourceRegion(&m_resource, m_uiSubresource, 0, 0, 0, m_pStagingResource, 0, nullptr);
        m_pStagingResource->Release();
        m_pStagingResource = nullptr;
    }
    m_pImmediateContext->Release();
    m_pImmediateContext = nullptr;
    m_pDevice->Release();
    m_pDevice = nullptr;
}

template<typename RESOURCE_TYPE, typename DESC_TYPE>
void* D3d11Mapper<RESOURCE_TYPE, DESC_TYPE>::Map(RESOURCE_TYPE& resource, UINT uiSubresource)
{
    const HRESULT res = m_pImmediateContext->Map(&resource, uiSubresource, m_mapType, 0, &m_mappedSubresource);

    if (DXGI_ERROR_DEVICE_REMOVED == res)
    {
        LOG_ERROR(TEXT("the video card has been removed"));
        return nullptr;
    }
    assert(S_OK == res);
    return m_mappedSubresource.pData;
}

template<typename RESOURCE_TYPE, typename DESC_TYPE>
bool D3d11Mapper<RESOURCE_TYPE, DESC_TYPE>::CopyToStagingResource()
{
    // prepare descriptor for staging resource
    DESC_TYPE stagingResourceDesc;
    m_resource.GetDesc(&stagingResourceDesc);
    stagingResourceDesc.Usage = D3D11_USAGE_STAGING;
    stagingResourceDesc.BindFlags = 0;
    switch (m_mapType)
    {
    case D3D11_MAP_READ:
        stagingResourceDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
        break;
    case D3D11_MAP_WRITE:
        stagingResourceDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        break;
    case D3D11_MAP_READ_WRITE:
        stagingResourceDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
        break;
    default:
        assert(false);
    }

    assert(!m_pStagingResource);
    m_pStagingResource = CreateResource(stagingResourceDesc);
    if (nullptr == m_pStagingResource)
    {
        LOG_ERROR(TEXT("cannot create staging resource to copy from default one"));
        return false;
    }
    // this action is asynchronous, but the map we do after it will wait for it to complete
    m_pImmediateContext->CopySubresourceRegion(m_pStagingResource, 0, 0, 0, 0, &m_resource, m_uiSubresource, nullptr);
    return true;
}

}}}
