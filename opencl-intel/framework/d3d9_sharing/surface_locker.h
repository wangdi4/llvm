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
#include "cl_synch_objects.h"
#include "Logger.h"

namespace Intel { namespace OpenCL { namespace Framework
{

    /**
     * @class   SurfaceLocker
     *
     * @brief   This class is responsible for the centralized control over the locking and
     * 			unlocking of each IDirect3DSurface9 for all planes of planar surfaces.
     *
     * @author  Aharon
     */

    class SurfaceLocker
    {

        Intel::OpenCL::Utils::OclMutex m_muAcquireRelease;  // provides synchronization for all non-const operations
        int m_lockRefCnt;  // reference counter for number of locks
        Intel::OpenCL::Utils::AtomicCounter m_objRefCnt;   // reference counter for number of D3D9Surface objects
        void* m_data;
        IDirect3DSurface9* const m_pSurface;
        INT m_pitch;
        const DWORD m_flags;
        DECLARE_LOGGER_CLIENT;

    public:

        /**
         * @brief Constructor
         * 
         * @param  pSurface a pointer to the IDirect3DSurface9 this SurfaceLocker controls
         * @param  flags    Combination of zero or more locking flags that describe the type of
         * 					lock to perform. If the surface is already locked, every further call
         * 					to this method must specify the same flags as the ones in the call that
         * 					first locked the surface.
         */
        SurfaceLocker(IDirect3DSurface9* pSurface, DWORD flags);

        /**
         * @brief adds an IDirect3DSurface9 object to this SurfaceLocker
         */
        void AddObject();

        /**
         * @brief removes an IDirect3DSurface9 object from this SurfaceLocker
         */
        void RemoveObject();

        /**
         * @return the number of IDirect3DSurface9 object dependent on this SurfaceLocker
         */
        long GetNumObjects() const
        {
            return m_objRefCnt;
        }

        /**
         * @brief locks the whole IDirect3DSurface9
         * 		  
         * @return the data of locked surface. It is D3D9Surface's responsibility to handle just
         * 		   its own plane's data. If flags is different than the ones in the previous calls,
         * 		   NULL is returned.
         */
        void* Lock();

        /**
         * @brief unlocks the whole IDirect3DSurface9
         */
        void Unlock();

        /**
         * @return the surface's pitch
         */

        INT GetPitch() const { return m_pitch; }

        /**
         * @return the flags
         */
        DWORD GetFlags() const { return m_flags; }

    private:

        bool Invariant() const
        {
            return 0 <= m_objRefCnt && m_objRefCnt >= m_lockRefCnt &&
                (0 == m_lockRefCnt || nullptr != m_data);
        }

    };

}}}
