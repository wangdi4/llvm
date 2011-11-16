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

#include <cstdlib>
#include "d3d9_context.h"
#include "CL/cl_d3d9.h"
#include "Device.h"
#include "cl_logger.h"
#include "d3d9_resource.h"

#pragma comment (lib, "d3dx9d.lib")
#pragma comment (lib, "d3d9.lib")

using namespace Intel::OpenCL::Utils;

namespace Intel { namespace OpenCL { namespace Framework
{
    /**
     * @fn  IDirect3DDevice9* ParseD3D9ContextOptions(const cl_context_properties* const pProperties)
     */

    IDirect3DDevice9* ParseD3D9ContextOptions(const cl_context_properties* const pProperties)
    {
        if (NULL == pProperties)
        {
            return NULL;
        }
        const cl_context_properties* pProprty = pProperties;
        while (0 != *pProprty)
        {
            if (CL_CONTEXT_D3D9_DEVICE_INTEL == *pProprty)
                return (IDirect3DDevice9*)(pProprty[1]);
            pProprty += 2;
        }
        return NULL;
    }

    /**
     * @fn  D3D9Context::D3D9Context(const cl_context_properties* clProperties, cl_uint uiNumDevices,
     *      cl_uint uiNumRootDevices, FissionableDevice** ppDevices, logging_fn pfnNotify,
     *      void* pUserData, cl_err_code* pclErr, ocl_entry_points* pOclEntryPoints,
     *      ocl_gpa_data* pGPAData, IDirect3DDevice9* const pD3D9Device)
     */

    D3D9Context::D3D9Context(const cl_context_properties* clProperties, cl_uint uiNumDevices,
        cl_uint uiNumRootDevices, FissionableDevice** ppDevices, logging_fn pfnNotify,
        void* pUserData, cl_err_code* pclErr, ocl_entry_points* pOclEntryPoints,
        ocl_gpa_data* pGPAData, IDirect3DDevice9* const pD3D9Device) :
    Context(clProperties, uiNumDevices, uiNumRootDevices, ppDevices, pfnNotify, pUserData,
        pclErr, pOclEntryPoints, pGPAData),
        m_pD3D9Device(pD3D9Device)
    {        
        /* The spec states that we should return "CL_INVALID_D3D9_DEVICE_Intel if the value of the
        property CL_CONTEXT_D3D9_DEVICE_Intel is non-NULL and does not specify a valid Direct3D 9
        device with which the cl_device_ids against which this context is to be created may
        interoperate." However, I don't know how to check the validity of the device. I guess a
        non-valid device will cause a segmentation fault if one of its methods are called and we
        don't want that. */

        m_pD3D9Device->AddRef();
        for (cl_uint i = 0; i < m_pOriginalNumDevices; i++)
        {
            m_ppAllDevices[i]->SetD3D9Device(pD3D9Device);
        }
    }

    /**
     * @fn  D3D9Context::~D3D9Context()
     */

    D3D9Context::~D3D9Context()
    {
        m_pD3D9Device->Release();
        for (cl_uint i = 0; i < m_pOriginalNumDevices; i++)
        {
            m_ppAllDevices[i]->SetD3D9Device(NULL);
        }
    }

    /**
     * @fn  cl_err_code D3D9Context::CreateD3D9Resource(cl_mem_flags clFlags,
     *      D3D9ResourceInfo* const pResourceInfo, MemoryObject** const ppMemObj,
     *      cl_mem_object_type clObjType, cl_uint uiDimCnt, const D3DFORMAT d3dFormat)
     */

    cl_err_code D3D9Context::CreateD3D9Resource(cl_mem_flags clFlags,
        D3D9ResourceInfo* const pResourceInfo, MemoryObject** const ppMemObj,
        cl_mem_object_type clObjType, cl_uint uiDimCnt, const D3DFORMAT d3dFormat)
    {
        Intel::OpenCL::Utils::OclAutoMutex mtx(&m_muAcquireRelease, false);
        m_muAcquireRelease.Lock();

        if (m_resourceInfoSet.find(pResourceInfo) != m_resourceInfoSet.end())
        {
            LOG_ERROR(TEXT("%s"), "A cl_mem from this D3D9ResourceInfo has already been created");
            delete pResourceInfo;
            return CL_INVALID_D3D9_RESOURCE_INTEL;
        }
        assert(NULL != ppMemObj);
        MemoryObject* pMemObj;

        cl_err_code clErr = MemoryObjectFactory::GetInstance()->CreateMemoryObject(m_devTypeMask,
            clObjType, CL_MEMOBJ_GFX_SHARE_DX9, this, &pMemObj);
        if (CL_FAILED(clErr))
        {
            LOG_ERROR(TEXT("Error creating new D3D9Resource, returned: %S"), ClErrTxt(clErr));
            delete pResourceInfo;
            return clErr;
        }
        // resourceInfo will be deleted in pD3D9Resource's destructor
        m_resourceInfoSet.insert(pResourceInfo);        
        D3D9Resource& d3d9Resource = *dynamic_cast<D3D9Resource*>(pMemObj);
        if (!d3d9Resource.IsCreatedInD3DPoolDefault(*pResourceInfo))
        {
            LOG_ERROR(TEXT("%s"), "resource is not a Direct3D 9 resource created in D3DPOOL_DEFAULT");
            d3d9Resource.Release();
            return CL_INVALID_D3D9_RESOURCE_INTEL;
        }        
        size_t dims[3];
        d3d9Resource.FillDimensions(*pResourceInfo, dims);
        const cl_image_format clFormat = D3D9Resource::MapD3DFormat2OclFormat(d3dFormat);
        clErr = d3d9Resource.Initialize(clFlags, D3DFMT_UNKNOWN != d3dFormat ? &clFormat : NULL,
            uiDimCnt, dims, NULL, pResourceInfo);
        if (CL_FAILED(clErr))
        {
            LOG_ERROR(TEXT("Failed to initialize data, pD3D9Resource->Initialize(pHostPtr = %S"),
                ClErrTxt(clErr));
            d3d9Resource.Release();
            return clErr;
        }        
        m_mapMemObjects.AddObject((OCLObject<_cl_mem_int>*)pMemObj);
        *ppMemObj = pMemObj;
        return CL_SUCCESS;
    }

    /**
     * @fn  bool D3D9Context::D3D9ResourceInfoComparator::operator()(const D3D9ResourceInfo* left,
     *      const D3D9ResourceInfo* right) const
     */

    bool D3D9Context::D3D9ResourceInfoComparator::operator()(const D3D9ResourceInfo* left, const D3D9ResourceInfo* right) const
    {
        if (typeid(*left) != typeid(*right))
            return &typeid(*left) < &typeid(*right);
        return *left < *right;
    }

}}}
