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
void SharedPtrBase<T>::IncRefCnt()
{
#if _DEBUG
    const long lRefCnt = 
#endif
        this->m_ptr->IncRefCnt();
#if _DEBUG
    // TODO: In some DLLs (like task_executor.dll) we always get NULL for the mutex and map - we need to fix this.
    if (lRefCnt >= 0 && NULL != allocatedObjectsMapMutex && NULL != allocatedObjectsMap && NULL != this->m_ptr->GetTypeName())   // otherwise the object isn't reference counted
    {
        allocatedObjectsMapMutex->Lock();
        (*allocatedObjectsMap)[std::string(this->m_ptr->GetTypeName())][this->m_ptr->GetThis()] = lRefCnt;
        allocatedObjectsMapMutex->Unlock();        
    }
#endif
}

template<typename T>
void SharedPtrBase<T>::DecRefCntInt(T* ptr)
{
    if (NULL != ptr)
    {
#if _DEBUG
        // This isn't thread safe, but these object are freed when the library is unloaded, so there is just one thread at this point.
        const bool bIsAllocationDbNull = NULL == allocatedObjectsMap || NULL == allocatedObjectsMapMutex || NULL == ptr->GetTypeName();

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
                (*allocatedObjectsMap)[std::string(ptr->GetTypeName())][ptr->GetThis()] = lNewVal;
            }
            else if (0 == lNewVal)
            {
                (*allocatedObjectsMap)[std::string(ptr->GetTypeName())].erase(ptr->GetThis());
            }
            allocatedObjectsMapMutex->Unlock();
        }
#endif
        if (0 == lNewVal)
        {
            HandleRefCnt0(ptr);
        }
    }
}

template<typename T>
long SharedPtrBase<T>::GetRefCnt() const
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

template<typename T>
void SharedPtr<T>::HandleRefCnt0(T* ptr)
{
    ptr->Cleanup();
}

}}}
