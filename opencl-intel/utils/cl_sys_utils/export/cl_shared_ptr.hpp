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
