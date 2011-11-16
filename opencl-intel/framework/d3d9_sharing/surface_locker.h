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
                (0 == m_lockRefCnt || NULL != m_data);
        }

    };

}}}
