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

#include <cassert>
#include "surface_locker.h"

namespace Intel { namespace OpenCL { namespace Framework
{

    /**
     * @fn SurfaceLocker::SurfaceLocker(IDirect3DSurface9* pSurface, DWORD flags)
     */
    SurfaceLocker::SurfaceLocker(IDirect3DSurface9* pSurface, DWORD flags) : m_lockRefCnt(0), m_data(nullptr),
        m_pSurface(pSurface), m_flags(flags)
    {
        D3DLOCKED_RECT lockedRect;

        HRESULT res = pSurface->LockRect(&lockedRect, nullptr, m_flags);
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
            const HRESULT res = m_pSurface->LockRect(&lockedRect, nullptr, m_flags);
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
            m_data = nullptr;
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
