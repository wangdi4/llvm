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

#include "cl_shared_ptr.hpp"
#include <stdio.h>

using namespace std;

namespace Intel { namespace OpenCL { namespace Utils {

#ifdef _DEBUG
/* These are defined as pointers, since in Linux the order of unloading shared libraries in the exit flow is undefined and objects in data segment might be destroyed before they are used
   in a shared library that has not yet been unloaded. */
OclMutex* allocatedObjectsMapMutex;
map<string, AllocatedObjectsMap >* allocatedObjectsMap;
#endif

void InitSharedPtrs()
{
#ifdef _DEBUG
    if (nullptr == allocatedObjectsMapMutex)
    {
        allocatedObjectsMapMutex = new OclMutex(DEFAULT_SPIN_COUNT, SUPPORT_RECURSIVE_LOCK);
    }
    if (nullptr == allocatedObjectsMap)
    {
        allocatedObjectsMap = new map<string, map<const void*, long> >();
    }
#endif
}

void FiniSharedPts()
{
    // we don't delete the objects, because this method maybe be called during an non-clean shutdown, when some threads are still using them.
}

void ReferenceCountedObject::IncZombieCnt() const
{
    OclAutoMutex lock(&m_zombieLock);
    ++m_zombieLevelCnt; 
    m_bCheckZombie = true; 
}


long ReferenceCountedObject::DriveEnterZombieState() const
{
    long new_val;
    bool trigger_action = false;
    
    // do everything inside a lock
    // in the most common case each DecRefCount operation for objects with lifetime control will
    // require 3 lock operations:
    //  1. Acquire m_zombieLock
    //  2. Decrement atomic counter
    //  3. Release m_zombieLock
    {
        OclAutoMutex lock(&m_zombieLock);
        new_val = --m_refCnt;  // this cannot result in object deletion as done inside lock
        
        if ((new_val == m_zombieLevelCnt) && (ZOMBIE != m_state))
        {
            ++m_refCnt;       // ensure object will not disapper during callbacks
            m_state = ZOMBIE; 
            trigger_action = true;
        }
    }

    if (true == trigger_action)
    {
        // call callbacks autside of lonk to avoid deadlocks
        const_cast<ReferenceCountedObject*>(this)->EnterZombieState( TOP_LEVEL_CALL );
#ifdef _DEBUG
        assert( (true == m_bEnterZombieStateCalled) && "EnterZombieState() must be propagated up to the parent" );
#endif
        // now decrement counter inside the same lock as above to avoid object deappearence 
        // inside the above critical section in another thread
        OclAutoMutex lock(&m_zombieLock);
        new_val = --m_refCnt;
    }

    return new_val;
}

#ifdef _DEBUG

static void DumpSharedPtsHeader( const char* map_title )
{
    printf("\nBEGIN SHARED POINTERS MAP: %s\n", (nullptr != map_title) ? map_title : "" );
}

static void DumpSharedPtsFooter( const char* map_title )
{
    printf("\nEND SHARED POINTERS MAP: %s\n", (nullptr != map_title) ? map_title : "" );
    fflush(0);
}

void DumpSharedPts(const char* map_title, bool if_non_empty)
{
    bool header_printed = false;
    
    if ((nullptr != allocatedObjectsMapMutex) && (nullptr != allocatedObjectsMap))
    {
        allocatedObjectsMapMutex->Lock();

        if (!if_non_empty)
        {
            header_printed = true;
            DumpSharedPtsHeader( map_title );
        }

        map<string, AllocatedObjectsMap >::iterator name_it     = allocatedObjectsMap->begin();
        map<string, AllocatedObjectsMap >::iterator name_it_end = allocatedObjectsMap->end();

        for (; name_it != name_it_end; ++name_it)
        {
            AllocatedObjectsMap& internal_map = name_it->second;
            if (internal_map.size() > 0)
            {
                if (!header_printed)
                {
                    header_printed = true;
                    DumpSharedPtsHeader( map_title );                                        
                }
                printf("\n%s:\n", name_it->first.c_str());
                AllocatedObjectsMapIterator it      = internal_map.begin();
                AllocatedObjectsMapIterator it_end  = internal_map.end();

                for (; it != it_end; ++it)
                {
                    printf("\t%p  %ld\n", it->first, it->second);
                }
                
            }
            
        }

        if (header_printed)
        {
            DumpSharedPtsFooter( map_title );
        }

        allocatedObjectsMapMutex->Unlock();
    }
}

#endif

}}}
