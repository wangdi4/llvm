// Copyright (c) 2006-2012 Intel Corporation
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

#include <cassert>
#include <map>
#include "cl_synch_objects.h"
#include "cl_shared_ptr.h"

namespace Intel { namespace OpenCL { namespace Utils {

#if _DEBUG
extern OclMutex* allocatedObjectsMapMutex;
extern std::map<std::string, std::map<const void*, long> >* allocatedObjectsMap;
#endif

template<typename T>
void SharedPtr<T>::IncRefCnt()
{
#if _DEBUG
    const long lRefCnt = 
#endif
        this->m_ptr->IncRefCnt();
#if _DEBUG
    if (lRefCnt >= 0 && NULL != allocatedObjectsMapMutex && NULL != allocatedObjectsMap)   // otherwise the object isn't reference counted
    {
        allocatedObjectsMapMutex->Lock();
        (*allocatedObjectsMap)[this->m_ptr->GetTypeName()][this->m_ptr] = lRefCnt;
        allocatedObjectsMapMutex->Unlock();        
    }
#endif
}

template<typename T>
void SharedPtr<T>::DecRefCntInt(T* ptr)
{
    if (NULL != ptr)
    {
#if _DEBUG
        // This isn't thread safe, but these object are freed when the library is unloaded, so there is just one thread at this point.
        const bool bIsAllocationDbNull = NULL == allocatedObjectsMap || NULL == allocatedObjectsMapMutex;

        if (!bIsAllocationDbNull)
        {
            allocatedObjectsMapMutex->Lock();
        }
#endif
        const long lNewVal = ptr->DecRefCnt();
#if _DEBUG
        if (!bIsAllocationDbNull)
        {
            if (lNewVal > 0)
            {
                (*allocatedObjectsMap)[ptr->GetTypeName()][ptr] = lNewVal;
            }
            else if (0 == lNewVal)
            {
                (*allocatedObjectsMap)[ptr->GetTypeName()].erase(ptr);
            }
            allocatedObjectsMapMutex->Unlock();
        }
#endif
        if (0 == lNewVal)
        {
            ptr->Cleanup();
            delete ptr;
        }
    }
}

template<typename T>
long SharedPtr<T>::GetRefCnt() const
{
    if (this->m_ptr)
    {
        return this->m_ptr->GetRefCnt();
    }
    else
    {
        return 0;
    }
}

template<typename T>
SharedPtr<T>::operator ConstSharedPtr<T>() const
{
    return ConstSharedPtr<T>(this->m_ptr);
}

}}}
