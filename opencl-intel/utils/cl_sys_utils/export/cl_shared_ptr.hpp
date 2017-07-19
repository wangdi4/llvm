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

#ifdef _DEBUG
extern OclMutex* allocatedObjectsMapMutex;
typedef std::map<const void*, long>                 AllocatedObjectsMap;
typedef std::map<const void*, long>::iterator       AllocatedObjectsMapIterator;
extern std::map<std::string, AllocatedObjectsMap >* allocatedObjectsMap;
#endif

template<typename T>
void SharedPtrBase<T>::IncRefCnt()
{
#ifdef _DEBUG
    const long lRefCnt = 
#endif
        this->m_ptr->IncRefCnt();
#ifdef _DEBUG
    // TODO: In some DLLs (like task_executor.dll) we always get NULL for the mutex and map - we need to fix this.
    std::string name = (nullptr != this->m_ptr->GetTypeName()) ? this->m_ptr->GetTypeName() : "";
    void*       p    = (void*)(this->m_ptr->GetThis());
    if (lRefCnt >= 0 && nullptr != allocatedObjectsMapMutex && nullptr != allocatedObjectsMap && name != "")   // otherwise the object isn't reference counted
    {
        allocatedObjectsMapMutex->Lock();
        if (1 == lRefCnt)
        {
            (*allocatedObjectsMap)[name][p] = 1;
        }
        else
        {   
            ++((*allocatedObjectsMap)[name][p]);
        }
        allocatedObjectsMapMutex->Unlock();        
    }
#endif
}

template<typename T>
void SharedPtrBase<T>::DecRefCntInt(T* ptr)
{
    if (nullptr != ptr)
    {
#ifdef _DEBUG
        std::string name = (nullptr != ptr->GetTypeName()) ? ptr->GetTypeName() : "";
        void*       p    = (void*)(ptr->GetThis());

        // This isn't thread safe, but these object are freed when the library is unloaded, so there is just one thread at this point.
        const bool bIsAllocationDbNull = nullptr == allocatedObjectsMap || nullptr == allocatedObjectsMapMutex || name == "";
#endif
        const long lNewVal = ptr->DecRefCnt();
#ifdef _DEBUG
        if (!bIsAllocationDbNull)
        {
            allocatedObjectsMapMutex->Lock();

            AllocatedObjectsMap& internal_map = (*allocatedObjectsMap)[name];
            AllocatedObjectsMapIterator it = internal_map.find(p);
            AllocatedObjectsMapIterator it_end = internal_map.end();

            if (it_end != it)
            {
                --(it->second);
                if (0 == it->second)
                {
                    internal_map.erase( it );
                }
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

template<typename T>
void ConstSharedPtr<T>::HandleRefCnt0(const T* ptr)
{
    (const_cast<T*>(ptr))->Cleanup();
}

template<typename T>
void LifetimeObjectContainer<T>::add( const SharedPtr<T>& ptr )
{
    if (!isZombie( ptr.GetPtr() ))
    {
        OclAutoMutex lock( &m_lock );
        if (!isZombie( ptr.GetPtr() ))
        {
            if (true == m_set.insert( ptr ).second)
            {
                // inserted a new element
                IncZombieCnt( ptr.GetPtr() );
            }
        }
    }
    else
    {
        assert( false && "Cannot add object to LifetimeObjectContainer in Zombie state" );
    }
}

template<typename T>
void LifetimeObjectContainer<T>::remove( const SharedPtr<T>& ptr )
{
    if (isZombie( ptr.GetPtr()))
    {
        OclAutoMutex lock( &m_lock );
        m_set.erase( ptr );
    }
    else
    {
        assert( false && "Cannot remove object from LifetimeObjectContainer in non-Zombie state" );
    }
}

template<typename T>
template<class T1>
void LifetimeObjectContainer<T>::getObjects( T1& containerToFill )
{
    OclAutoMutex lock( &m_lock );
    containerToFill.insert( containerToFill.end(), m_set.begin(), m_set.end() );
}


}}}
