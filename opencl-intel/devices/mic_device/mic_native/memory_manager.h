#pragma once

#include "pragmas.h"
#include "memory_pool.h"
#include "ocl_list.h"
#include "native_synch_objects.h"

#include <map>

using namespace Intel::OpenCL::UtilsNative;

namespace Intel { namespace OpenCL { namespace MICDeviceNative {

/* This class manage the memory pools and reserve memory for Programs.
   The implementation is thread safe.
   This class is singleton class. Access the singleton object by static method getInstance(). */
class MemoryManager
{

public:

    /* Return the singleton instance of MemoryManager */
    static MemoryManager& getInstance( void )
    {
        assert( m_pSingleMemoryManager && "SINK: MemoryManager was not initialized" );
        return *m_pSingleMemoryManager;
    }

    /* Create and Release the singleton object */
    static void createMemoryManager( void );
    static void releaseMemoryManager( void );

    /* Reserve memory to program from MemoryPool. (create new MemoryPool if needed and allocate executable memory by mmap).
    numOfItems - The amount of items to allocate.
    itemSize - The granularity of each memory item. (its size)
    reserveMemAddress - Out parameter that will include the start address of the memory reserved.
    Return true if succeeded and false otherwise */
    bool reserveMem(unsigned int numOfItems, unsigned int itemSize, void** reserveMemAddress);

    /* Free already reserved memory. (delete MemoryPool if needed and free the executable memory by munma if needed).
    reserveMemAddress - the start address of the memory reserved. */
    void freeReserveMem(void* reserveMemAddress);

protected:

    virtual ~MemoryManager();

private:

    /* Private constructor in order to ensure that only one instance exist */
    MemoryManager();

    /* unmap the executable memory that is pointed by pMemPool, and delete the object */
    void unmapAndDeleteMemPool(MemoryPool* pMemPool);

    /* Release the content of the data structures and release memory.
       Call it from destructor only. */
    void release();

    // Th single MemoryManager object
    static MemoryManager* m_pSingleMemoryManager;

    // List of memory pools.
    OclList<MemoryPool*> m_memoryPoolsList;
    // map key is start address of memory pool and the value is OclList::Node* that contain the MemoryPool object.
    map<void*, OclList<MemoryPool*>::Node*> m_startAddressToMemPool;

    // back up memory pool
    MemoryPool* m_backupPool;

    OclMutexNative m_mutex;

};

}}}
