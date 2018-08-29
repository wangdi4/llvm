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

#include <set>
#include "Context.h"
#include "d3d9_sharing.h"
#include <basetsd.h>
#include "surface_locker.h"
#include "cl_shared_ptr.h"
#include "Device.h"

namespace Intel { namespace OpenCL { namespace Framework
{

/**
 * @class   D3DContext
 *
 * @brief   This class represents a context that can create shared memory objects shared with Direct 3D
 *
 * @param RESOURCE_TYPE the super-type of all Direct 3D resources that can be created by this context
 * @param DEV_TYPE the type of the Direct 3D device
 *
 * @author  Aharon
 * @date    6/30/2011
 */
template<typename RESOURCE_TYPE, typename DEV_TYPE>
class D3DContext : public Context
{
    /**
     * @class   D3DResourceInfoComparator
     *
     * @brief   Direct3D 9 resource information comparator. 
     *
     * @author  Aharon
     * @date    7/25/2011
     */

    class D3DResourceInfoComparator {

    public:

        /**
         * @brief   predicate method
         *
         * @author  Aharon
         * @date    7/25/2011
         */

        bool operator()(const D3DResourceInfo<RESOURCE_TYPE>* left, const D3DResourceInfo<RESOURCE_TYPE>* right) const;

    };
    
    set<const D3DResourceInfo<RESOURCE_TYPE>*, D3DResourceInfoComparator> m_resourceInfoSet;
    Intel::OpenCL::Utils::OclMutex m_mutex;        
    const ID3DSharingDefinitions* const m_pd3dDefinitions;

public:

    PREPARE_SHARED_PTR(D3DContext)

    static SharedPtr<D3DContext> Allocate(const cl_context_properties* clProperties, cl_uint uiNumDevices,
        cl_uint uiNumRootDevices, SharedPtr<FissionableDevice>* ppDevices, logging_fn pfnNotify,
        void* pUserData, cl_err_code* pclErr, ocl_entry_points* pOclEntryPoints,
        ocl_gpa_data* pGPAData, IUnknown* const pD3DDevice, cl_context_properties iDevType, const ID3DSharingDefinitions* pd3dDefinitions,
        bool bIsInteropUserSync = false)
    {
        return SharedPtr<D3DContext>(new D3DContext(clProperties, uiNumDevices, uiNumRootDevices, ppDevices, pfnNotify, pUserData, pclErr, pOclEntryPoints, pGPAData, pD3DDevice, iDevType,
            pd3dDefinitions, bIsInteropUserSync));
    }

    /**
     * type of device: CL_CONTEXT_D3D9_DEVICE_INTEL, CL_CONTEXT_D3D9EX_DEVICE_INTEL or
     * CL_CONTEXT_DXVA9_DEVICE_INTEL
     */
    const cl_context_properties m_iDeviceType;

    /**
     * whether the context was created with CL_INTEROP_USER_SYNC property
     */
    const bool m_bIsInteropUserSync;    

    /**
     * @brief   Finaliser.
     *
     * @author  Aharon
     * @date    7/5/2011
     */

    virtual ~D3DContext();

    /**
     * @brief   D3D9Device getter
     *
     * @author  Aharon
     * @date    7/6/2011
     *
     * @return  the IDirect3DDevice9* against which this D3DContext was created
     */

    const IUnknown* GetD3DDevice() const { return m_pD3DDevice; }

    /**
     * @brief   Removes the resource information described by resourceInfo.
     *
     * @author  Aharon
     * @date    7/7/2011
     *
     * @param   resourceInfo    Information describing the resource.
     */

    void RemoveResourceInfo(const D3DResourceInfo<RESOURCE_TYPE>& resourceInfo)
    { 
        m_mutex.Lock();
        const size_t numErased = m_resourceInfoSet.erase(&resourceInfo);
        assert(1 == numErased);
        m_mutex.Unlock();
    }

    /**
     * @brief   Creates a Direct3D 9 resource.
     *
     * @author  Aharon
     * @date    7/7/2011
     *
     * @param   clFlags                 The OpenCL memory flags.
     * @param [in,out]  pResourceInfo   Information describing the resource.
     * @param [in,out]  ppMemObj        a pointer to a pointer to the created MemoryObject.
     * @param   clObjType               Type of the OpenCL object.
     * @param   uiDimCnt                number of dimensions of the resource.
     * @param   plane                   the plane of pResource to share, for planar surface formats
     * 									(otherwise MAXUINT)
     *
     * @return  CL_SUCCESS upon success, error code otherwise.
     */

    cl_err_code CreateD3DResource(cl_mem_flags clFlags, D3DResourceInfo<RESOURCE_TYPE>* const pResourceInfo, SharedPtr<MemoryObject>* const ppMemObj, cl_mem_object_type clObjType,
        cl_uint uiDimCnt, UINT plane = MAXUINT);

    /**
     * @return the ID3DSharingDefinitions of the version of the extension used
     */
    const ID3DSharingDefinitions& GetD3dDefinitions() const { return *m_pd3dDefinitions; }

    /**
     * @param pResource a pointer to a Direct 3D resource
     * @return a pointer to the device that created pResource or NULL if it cannot be gotten. NOTE: remember that the device has to be release after its use.
     */
    virtual DEV_TYPE* GetDevice(RESOURCE_TYPE* pResource) const = 0;

protected:

    /**
     * @brief   Constructor.
     *
     * @author  Aharon
     * @date    6/30/2011
     *
     * @param   clProperties            context's properties.
     * @param   uiNumDevices            number of devices associated to the context
     * @param   uiNumRootDevices        The user interface number root devices.
     * @param   ppDevices               list of devices.
     * @param   pfnNotify               error notification function's pointer.
     * @param   pUserData               user data
     * @param [in,out]  pclErr          If non-null, the pcl error.
     * @param   pOclEntryPoints 
     * @param   pGPAData                If non-null, information describing the gpa.
     * @param   pD3D9Device             to use for Direct3D interoperability.
     * @param   iDevType                type of device
     * @param   pd3d9Definitions         a pointer to ID3DSharingDefinitions of the version of the extension used
     * @param   bIsInteropUserSync      whether CL_INTEROP_USER_SYNC context property is set to true (option in OpenCL 1.2)
     */
    D3DContext(const cl_context_properties* clProperties, cl_uint uiNumDevices,
        cl_uint uiNumRootDevices, SharedPtr<FissionableDevice>* ppDevices, logging_fn pfnNotify,
        void* pUserData, cl_err_code* pclErr, ocl_entry_points* pOclEntryPoints,
        ocl_gpa_data* pGPAData, IUnknown* const pD3DDevice, cl_context_properties iDevType, const ID3DSharingDefinitions* pd3dDefinitions,
        ContextModule& contextModule, bool bIsInteropUserSync = false);

    
    IUnknown* const m_pD3DDevice;
    /**
     * lock the internal mutex
     */
    void LockMutex() { m_mutex.Lock(); }

    /**
     * unlock the internal mutex
     */
    void UnlockMutex() { m_mutex.Unlock(); }

    /**
     * handle planar surfaces
     * @param pResourceInfo information for the surface
     * @param flags    Combination of zero or more locking flags that describe the type of
     * 					lock to perform. If the surface is already locked, every further call
     * 					to this method must specify the same flags as the ones in the call that
     * 					first locked the surface.
     * @return CL_SUCCESS or error code in case of an error 
     */
    virtual cl_err_code HandlePlanarSurface(D3DResourceInfo<RESOURCE_TYPE>* pResourceInfo, cl_mem_flags clFlags) = 0;        

private:        

    // do not implement

    D3DContext<RESOURCE_TYPE, DEV_TYPE>(const D3DContext<RESOURCE_TYPE, DEV_TYPE>&);
    D3DContext<RESOURCE_TYPE, DEV_TYPE>& operator=(const D3DContext<RESOURCE_TYPE, DEV_TYPE>&);

};

/**
 * This class extends D3DContext for Direct 3D 9
 */
class D3D9Context : public D3DContext<IDirect3DResource9, IDirect3DDevice9>
{

public:

    PREPARE_SHARED_PTR(D3D9Context)

    /**
     * @brief   Constructor.
     *
     * @param   clProperties            context's properties.
     * @param   uiNumDevices            number of devices associated to the context
     * @param   uiNumRootDevices        The user interface number root devices.
     * @param   ppDevices               list of devices.
     * @param   pfnNotify               error notification function's pointer.
     * @param   pUserData               user data
     * @param [in,out]  pclErr          If non-null, the pcl error.
     * @param   pOclEntryPoints 
     * @param   pGPAData                If non-null, information describing the gpa.
     * @param   pD3D9Device             to use for Direct3D 9 interoperability.
     * @param   iDevType                type of device
     * @param   pd3d9Definitions         a pointer to ID3DSharingDefinitions of the version of the extension used
     * @param   bIsInteropUserSync      whether CL_INTEROP_USER_SYNC context property is set to true (option in OpenCL 1.2)
     */
    static SharedPtr<D3D9Context> Allocate(const cl_context_properties* clProperties, cl_uint uiNumDevices,
        cl_uint uiNumRootDevices, SharedPtr<FissionableDevice>* ppDevices, logging_fn pfnNotify,
        void* pUserData, cl_err_code* pclErr, ocl_entry_points* pOclEntryPoints,
        ocl_gpa_data* pGPAData, IUnknown* const pD3D9Device, cl_context_properties iDevType, const ID3DSharingDefinitions* pd3d9Definitions,
        ContextModule& contextModule, bool bIsInteropUserSync = false)
    {
        return SharedPtr<D3D9Context>(new D3D9Context(clProperties, uiNumDevices, uiNumRootDevices, ppDevices, pfnNotify, pUserData, pclErr, pOclEntryPoints, pGPAData,
            pD3D9Device, iDevType, pd3d9Definitions, contextModule, bIsInteropUserSync));
    }

    /**
     * @param   pSurface    a pointer to IDirect3DSurface9
     * 
     * @return  a pointer to the SurfaceLocker that controls pSurface or NULL if no such
     * 			SurfaceLocker exists
     */
    SurfaceLocker* GetSurfaceLocker(const IDirect3DSurface9* pSurface)
    {
        LockMutex();
        map<const IDirect3DSurface9*, SurfaceLocker*>::iterator iter = m_surfaceLockers.find(pSurface);
        if (m_surfaceLockers.end() == iter)
        {
            UnlockMutex();
            return nullptr;
        }
        UnlockMutex();
        return iter->second;
    }

    /**
     * @param   pSurface    a pointer to IDirect3DSurface9
     * 
     * @return  a pointer to the SurfaceLocker that controls pSurface or NULL if no such
     * 			SurfaceLocker exists
     */
    const SurfaceLocker* GetSurfaceLocker(const IDirect3DSurface9* pSurface) const
    {
        map<const IDirect3DSurface9*, SurfaceLocker*>::const_iterator iter = m_surfaceLockers.find(pSurface);
        if (m_surfaceLockers.end() == iter)
        {
            return nullptr;
        }
        return iter->second;
    }

    /**
     * @brief releases the SurfaceLocker associated with a particular IDirect3DSurface9
     * 		  
     * @param pSurface  a pointer to the IDirect3DSurface9 to be released
     */
    void ReleaseSurfaceLocker(const IDirect3DSurface9* pSurface);

    // overridden methods

    IDirect3DDevice9* GetDevice(IDirect3DResource9* pResource) const;

protected:

    cl_err_code HandlePlanarSurface(D3DResourceInfo<IDirect3DResource9>* pResourceInfo, cl_mem_flags clFlags);

private:

    /**
     * @brief   Constructor.
     *
     * @param   clProperties            context's properties.
     * @param   uiNumDevices            number of devices associated to the context
     * @param   uiNumRootDevices        The user interface number root devices.
     * @param   ppDevices               list of devices.
     * @param   pfnNotify               error notification function's pointer.
     * @param   pUserData               user data
     * @param [in,out]  pclErr          If non-null, the pcl error.
     * @param   pOclEntryPoints 
     * @param   pGPAData                If non-null, information describing the gpa.
     * @param   pD3D9Device             to use for Direct3D 9 interoperability.
     * @param   iDevType                type of device
     * @param   pd3d9Definitions         a pointer to ID3DSharingDefinitions of the version of the extension used
     * @param   bIsInteropUserSync      whether CL_INTEROP_USER_SYNC context property is set to true (option in OpenCL 1.2)
     */
    D3D9Context(const cl_context_properties* clProperties, cl_uint uiNumDevices,
        cl_uint uiNumRootDevices, SharedPtr<FissionableDevice>* ppDevices, logging_fn pfnNotify,
        void* pUserData, cl_err_code* pclErr, ocl_entry_points* pOclEntryPoints,
        ocl_gpa_data* pGPAData, IUnknown* const pD3D9Device, cl_context_properties iDevType, const ID3DSharingDefinitions* pd3d9Definitions,
        ContextModule& contextModule, bool bIsInteropUserSync = false) :
    D3DContext<IDirect3DResource9, IDirect3DDevice9>(clProperties, uiNumDevices, uiNumRootDevices, ppDevices, pfnNotify, pUserData, pclErr, pOclEntryPoints, pGPAData,
        pD3D9Device, iDevType, pd3d9Definitions, contextModule, bIsInteropUserSync) { }

    map<const IDirect3DSurface9*, SurfaceLocker*> m_surfaceLockers;        

};

/**
 * This class extends D3DContext for Direct 3D 11
 */
class D3D11Context : public D3DContext<ID3D11Resource, ID3D11Device>
{
public:

    PREPARE_SHARED_PTR(D3D11Context)

    /**
     * @param   clProperties            context's properties.
     * @param   uiNumDevices            number of devices associated to the context
     * @param   uiNumRootDevices        The user interface number root devices.
     * @param   ppDevices               list of devices.
     * @param   pfnNotify               error notification function's pointer.
     * @param   pUserData               user data
     * @param [in,out]  pclErr          If non-null, the pcl error.
     * @param   pOclEntryPoints 
     * @param   pGPAData                If non-null, information describing the gpa.
     * @param   pD3D9Device             to use for Direct3D 11 interoperability.
     * @param   iDevType                type of device
     * @param   pd3d9Definitions         a pointer to ID3DSharingDefinitions of the version of the extension used
     * @param   bIsInteropUserSync      whether CL_INTEROP_USER_SYNC context property is set to true (option in OpenCL 1.2)
     */
    static SharedPtr<D3D11Context> Allocate(const cl_context_properties* clProperties, cl_uint uiNumDevices,
        cl_uint uiNumRootDevices, SharedPtr<FissionableDevice>* ppDevices, logging_fn pfnNotify,
        void* pUserData, cl_err_code* pclErr, ocl_entry_points* pOclEntryPoints,
        ocl_gpa_data* pGPAData, IUnknown* const pD3D9Device, cl_context_properties iDevType, const ID3DSharingDefinitions* pd3d9Definitions,
        ContextModule& contextModule, bool bIsInteropUserSync = false)
    {        
        return SharedPtr<D3D11Context>(new D3D11Context(clProperties, uiNumDevices, uiNumRootDevices, ppDevices, pfnNotify, pUserData, pclErr, pOclEntryPoints, pGPAData,
            pD3D9Device, iDevType, pd3d9Definitions, contextModule, bIsInteropUserSync));
    }

    ~D3D11Context()
    {
        m_pD3DDevice->Release();
    }

    // overridden methods

    ID3D11Device* GetDevice(ID3D11Resource* pResource) const;

    cl_err_code	GetInfo(cl_int param_name, size_t param_value_size, void * param_value, size_t * param_value_size_ret) const;

protected:

    cl_err_code HandlePlanarSurface(D3DResourceInfo<ID3D11Resource>* pResourceInfo, cl_mem_flags clFlags) { return CL_SUCCESS; }

private:

    /**
     * @brief   Constructor.
     *
     * @param   clProperties            context's properties.
     * @param   uiNumDevices            number of devices associated to the context
     * @param   uiNumRootDevices        The user interface number root devices.
     * @param   ppDevices               list of devices.
     * @param   pfnNotify               error notification function's pointer.
     * @param   pUserData               user data
     * @param [in,out]  pclErr          If non-null, the pcl error.
     * @param   pOclEntryPoints 
     * @param   pGPAData                If non-null, information describing the gpa.
     * @param   pD3D9Device             to use for Direct3D 11 interoperability.
     * @param   iDevType                type of device
     * @param   pd3d9Definitions         a pointer to ID3DSharingDefinitions of the version of the extension used
     * @param   bIsInteropUserSync      whether CL_INTEROP_USER_SYNC context property is set to true (option in OpenCL 1.2)
     */
    D3D11Context(const cl_context_properties* clProperties, cl_uint uiNumDevices,
        cl_uint uiNumRootDevices, SharedPtr<FissionableDevice>* ppDevices, logging_fn pfnNotify,
        void* pUserData, cl_err_code* pclErr, ocl_entry_points* pOclEntryPoints,
        ocl_gpa_data* pGPAData, IUnknown* const pD3D9Device, cl_context_properties iDevType, const ID3DSharingDefinitions* pd3d9Definitions,
         ContextModule& contextModule, bool bIsInteropUserSync = false) :
    D3DContext<ID3D11Resource, ID3D11Device>(clProperties, uiNumDevices, uiNumRootDevices, ppDevices, pfnNotify, pUserData, pclErr, pOclEntryPoints, pGPAData,
        pD3D9Device, iDevType, pd3d9Definitions, contextModule, bIsInteropUserSync) 
    { 
        pD3D9Device->AddRef();
    }

};
    
}}}
