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

#include <cassert>
#include "surface_locker.h"

namespace Intel { namespace OpenCL { namespace Framework
{

    /**
     * @fn SurfaceLocker::SurfaceLocker(IDirect3DSurface9* pSurface, DWORD flags)
     */
    SurfaceLocker::SurfaceLocker(IDirect3DSurface9* pSurface, DWORD flags) : m_lockRefCnt(0), m_data(NULL),
        m_pSurface(pSurface), m_flags(flags)
    {
        D3DLOCKED_RECT lockedRect;

        HRESULT res = pSurface->LockRect(&lockedRect, NULL, 0);
        assert(D3D_OK == res);
        m_pitch = lockedRect.Pitch;
        res = pSurface->UnlockRect();
        assert(D3D_OK == res);
        assert(Invariant());
    }

    /**
     * @fn void* SurfaceLocker::Lock(DWORD flags)
     */
    void* SurfaceLocker::Lock()
    {
        assert(Invariant());
        Intel::OpenCL::Utils::OclAutoMutex mtx(&m_muAcquireRelease);
        if (0 == m_lockRefCnt)
        {
            D3DLOCKED_RECT lockedRect;
            const HRESULT res = m_pSurface->LockRect(&lockedRect, NULL, m_flags);
            assert(D3D_OK == res);
            m_data = lockedRect.pBits;
        }
        m_lockRefCnt++;
        assert(Invariant());
        return m_data;
    }

    /**
     * @fn void SurfaceLocker::Unlock()
     */
    void SurfaceLocker::Unlock()
    {
        assert(Invariant());
        Intel::OpenCL::Utils::OclAutoMutex mtx(&m_muAcquireRelease);
        assert(0 < m_lockRefCnt);
        m_lockRefCnt--;
        if (0 == m_lockRefCnt)
        {
            const HRESULT res = m_pSurface->UnlockRect();
            assert(D3D_OK == res);
            m_data = NULL;
        }
        assert(Invariant());
    }

    /**
     * @fn void SurfaceLocker::AddObject()
     */
    void SurfaceLocker::AddObject()
    {
        assert(Invariant());
        m_objRefCnt++;
        assert(Invariant());
    }

    /**
     * @fn void SurfaceLocker::RemoveObject()
     */
    void SurfaceLocker::RemoveObject()
    {
        assert(Invariant());
        m_objRefCnt--;
        assert(Invariant());
    }

}}}
