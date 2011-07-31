#pragma once

#include "memory_pool.h"

namespace Intel { namespace OpenCL { namespace MICDeviceNative {

/* Class that manage the Program executable memory.
   Assume that if reserving memory for this program, than this program destructore will call before calling MemoryManager::releaseMemoryManager(). */
class ProgramMemoryManager : private MemoryPool
{

public:

    ProgramMemoryManager();

    virtual ~ProgramMemoryManager();

    /* Reserve executable memory for this program.
       numOfItems - The amount of memory items.
       itemSize - The granularity of each memory item.
       Return true if succeeded and false otherwise.
       This method is thread safe if the implementation of MemoryManager and MemoryPool is thread safe. */
    bool reserveExecutableMemory(unsigned int numOfItems, unsigned int itemSize);

    /* Free already reserved memory.
       This method is thread safe if the implementation of MemoryManager and MemoryPool is thread safe. */
    void freeReservedMemory();

    /* Inherit from MemoryPool.
       This method is thread safe if the implementation of MemoryPool is thread safe. */
    using MemoryPool::allocate;

    /* Inherit from MemoryPool.
       This method is thread safe if the implementation of MemoryPool is thread safe. */
    using MemoryPool::free;

};

}}}
