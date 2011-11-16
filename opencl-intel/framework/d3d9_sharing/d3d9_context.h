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

#include <set>
#include "Context.h"
#include "d3d9_sharing.h"
#include <basetsd.h>
#include "surface_locker.h"

namespace Intel { namespace OpenCL { namespace Framework
{

    /**
     * @class   D3D9Context
     *
     * @brief   This class represents a context created with the CL_CONTEXT_D3D9_DEVICE_Intel
     * 			property.
     *
     * @author  Aharon
     * @date    6/30/2011
     */

    class D3D9Context : public Context
    {
        /**
         * @class   D3D9ResourceInfoComparator
         *
         * @brief   Direct3D 9 resource information comparator. 
         *
         * @author  Aharon
         * @date    7/25/2011
         */

        class D3D9ResourceInfoComparator {

        public:

            /**
             * @fn  bool D3D9ResourceInfoComparator::operator()(const D3D9ResourceInfo* left,
             *      const D3D9ResourceInfo* right)
             *
             * @brief   predicate method
             *
             * @author  Aharon
             * @date    7/25/2011
             */

            bool operator()(const D3D9ResourceInfo* left, const D3D9ResourceInfo* right) const;

        };

        IUnknown* const m_pD3D9Device;  // this can be either IDirect3DDevice9, IDirect3DDevice9Ex or IDXVAHD_Device         
        set<const D3D9ResourceInfo*, D3D9ResourceInfoComparator> m_resourceInfoSet;
        Intel::OpenCL::Utils::OclMutex m_muAcquireRelease;
        map<const IDirect3DSurface9*, SurfaceLocker*> m_surfaceLockers;        

    public:

        /**
         * type of device: CL_CONTEXT_D3D9_DEVICE_INTEL, CL_CONTEXT_D3D9EX_DEVICE_INTEL or
         * CL_CONTEXT_DXVA9_DEVICE_INTEL
         */
        const int m_iDeviceType;

        /**
         * whether the context was created with CL_INTEROP_USER_SYNC property
         */
        const bool m_bIsInteropUserSync;

        /**
         * @fn  D3D9Context::D3D9Context(const cl_context_properties* clProperties, cl_uint uiNumDevices,
         *      cl_uint uiNumRootDevices, FissionableDevice** ppDevices, logging_fn pfnNotify,
         *      void* pUserData, cl_err_code* pclErr, ocl_entry_points* pOclEntryPoints,
         *      ocl_gpa_data* pGPAData, IUnknown* const pD3D9Device)
         *
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
         * @param   pD3D9Device             to use for Direct3D 9 interoperability.
         * @param   iDevType                type of device: CL_CONTEXT_D3D9_DEVICE_INTEL,
         * 									CL_CONTEXT_D3D9EX_DEVICE_INTEL or CL_CONTEXT_DXVA9_DEVICE_INTEL 									
         * @param   bIsInteropUserSync      whether CL_INTEROP_USER_SYNC context property is set to true (option in OpenCL 1.2)
         */

        D3D9Context(const cl_context_properties* clProperties, cl_uint uiNumDevices,
            cl_uint uiNumRootDevices, FissionableDevice** ppDevices, logging_fn pfnNotify,
            void* pUserData, cl_err_code* pclErr, ocl_entry_points* pOclEntryPoints,
            ocl_gpa_data* pGPAData, IUnknown* const pD3D9Device, int iDevType,
            bool bIsInteropUserSync = false);

        /**
         * @fn  virtual D3D9Context::~D3D9Context()
         *
         * @brief   Finaliser.
         *
         * @author  Aharon
         * @date    7/5/2011
         */

        virtual ~D3D9Context();

        /**
         * @fn  const IUnknown* D3D9Context::GetD3D9Device() const
         *
         * @brief   D3D9Device getter
         *
         * @author  Aharon
         * @date    7/6/2011
         *
         * @return  the IDirect3DDevice9* against which this D3D9Context was created
         */

        const IUnknown* GetD3D9Device() const { return m_pD3D9Device; }

        /**
         * @fn  void D3D9Context::RemoveResourceInfo(const D3D9ResourceInfo& resourceInfo)
         *
         * @brief   Removes the resource information described by resourceInfo.
         *
         * @author  Aharon
         * @date    7/7/2011
         *
         * @param   resourceInfo    Information describing the resource.
         */

        void RemoveResourceInfo(const D3D9ResourceInfo& resourceInfo)
        { 
            m_muAcquireRelease.Lock();
            const size_t numErased = m_resourceInfoSet.erase(&resourceInfo);
            assert(1 == numErased);
            m_muAcquireRelease.Unlock();
        }

        /**
         * @fn  cl_err_code D3D9Context::CreateD3D9Resource(cl_mem_flags clFlags,
         *      D3D9ResourceInfo* const pResourceInfo, MemoryObject** const ppMemObj,
         *      cl_mem_object_type clObjType, cl_uint uiDimCnt, const D3DFORMAT d3dFormat, UINT plane = 0);
         *
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
         * @param   d3dFormat               a D3DFORMAT specifying the image's format or D3DFMT_UNKNOWN
         *                                  in case the resource is not an image.         
         * @param   plane                   the plane of pResource to share, for planar surface formats
         * 									(otherwise MAXUINT)
         *
         * @return  CL_SUCCESS upon success, error code otherwise.
         */

        cl_err_code CreateD3D9Resource(cl_mem_flags clFlags,
            D3D9ResourceInfo* const pResourceInfo, MemoryObject** const ppMemObj,
            cl_mem_object_type clObjType, cl_uint uiDimCnt, const D3DFORMAT d3dFormat, UINT plane = MAXUINT);

        /**
         * @brief releases the SurfaceLocker associated with a particular IDirect3DSurface9
         * 		  
         * @param pSurface  a pointer to the IDirect3DSurface9 to be released
         */
        void ReleaseSurfaceLocker(const IDirect3DSurface9* pSurface);

        /**
         * @param   pSurface    a pointer to IDirect3DSurface9
         * 
         * @return  a pointer to the SurfaceLocker that controls pSurface or NULL if no such
         * 			SurfaceLocker exists
         */
        SurfaceLocker* GetSurfaceLocker(const IDirect3DSurface9* pSurface)
        {
            Intel::OpenCL::Utils::OclAutoMutex mtx(&m_muAcquireRelease);
            map<const IDirect3DSurface9*, SurfaceLocker*>::iterator iter = m_surfaceLockers.find(pSurface);
            if (m_surfaceLockers.end() == iter)
            {
                return NULL;
            }
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
                return NULL;
            }
            return iter->second;
        }

    private:

        cl_err_code HandlePlanarSurface(D3D9ResourceInfo* pResourceInfo, cl_mem_flags clFlags);

        // do not implement

        D3D9Context(const D3D9Context&);
        D3D9Context& operator=(const D3D9Context&);

    };

}}}
