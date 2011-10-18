#include "pragmas.h"
#include "memory_manager.h"
#include "native_common_macros.h"

#include <sys/mman.h>
#include <assert.h>

using namespace Intel::OpenCL::MICDeviceNative;

#define MEMORY_ITEM_SIZE 4096
#define MINIMUM_MEM_ITEMS_IN_POOL 256

MemoryManager* MemoryManager::m_pSingleMemoryManager = NULL;

MemoryManager::MemoryManager()
{
    m_backupPool = NULL;
}

MemoryManager::~MemoryManager()
{
    release();
}

void MemoryManager::createMemoryManager( void )
{
    // if already initialized return it.
    if (m_pSingleMemoryManager)
    {
        return;
    }

    // create new MemoryManager
    m_pSingleMemoryManager = new MemoryManager();

    assert( m_pSingleMemoryManager && "SINK: Cannot create MemoryManager" );
}

void MemoryManager::releaseMemoryManager( void )
{
    // if not created or already released
    if (m_pSingleMemoryManager == NULL)
    {
        return;
    }
    MemoryManager* tMemoryManager = m_pSingleMemoryManager;
    // If this thread is the first thread that change the value of m_pSingleMemoryManager to NULL, can delete it
    if ((m_pSingleMemoryManager) && (__sync_bool_compare_and_swap(&m_pSingleMemoryManager, (size_t)tMemoryManager, (size_t)NULL)))
    {
        delete(tMemoryManager);
    }
}

bool MemoryManager::reserveMem(unsigned int numOfItems, unsigned int itemSize, void** reserveMemAddress)
{
    assert(numOfItems > 0 && "numOfItems == 0");
    assert(itemSize > 0 && "itemSize == 0");
    assert(reserveMemAddress && "reserveMemAddress == NULL");

    void* tAddress = NULL;
    OclListIterator<MemoryPool*> memPoolListIter;
    // convert the amount of item to allocate according to MEMORY_ITEM_SIZE
    unsigned int numOfItemsToAllocate = numOfItems * itemSize / MEMORY_ITEM_SIZE;
    if (numOfItems * itemSize % MEMORY_ITEM_SIZE != 0)
    {
        numOfItemsToAllocate ++;
    }

    OclAutoMutexNative auto_lock(&m_mutex);

    OclListIterator<MemoryPool*> memPoolListIterEnd = m_memoryPoolsList.end();
    // trying to find MemoryPool with at least numOfItemsToAllocate free space.
    for (memPoolListIter = m_memoryPoolsList.begin(); memPoolListIter != memPoolListIterEnd; ++memPoolListIter)
    {
        if ((*memPoolListIter)->allocate(numOfItemsToAllocate, &tAddress))
        {
            break;
        }
    }

    // If not found Memory pool with free space.
    if (tAddress == NULL)
    {
        // Check if there is enough space in the backup pool
        MemoryPool* pMemPool = m_backupPool;
        if ((pMemPool == NULL) || (pMemPool->getTotalAmountOfItems() < numOfItemsToAllocate))
        {
            pMemPool = new MemoryPool();
            assert(pMemPool && "new operation failed");
            unsigned int numMemItems = (numOfItemsToAllocate > MINIMUM_MEM_ITEMS_IN_POOL) ? numOfItemsToAllocate : MINIMUM_MEM_ITEMS_IN_POOL;

            void* exeMemStartAddress = mmap(NULL, numMemItems * MEMORY_ITEM_SIZE, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANONYMOUS | MAP_PRIVATE, 0, 0);
			assert(IS_ALIGN(exeMemStartAddress, MEMORY_ITEM_SIZE));
            // the map operation failed
            if (exeMemStartAddress == MAP_FAILED)
            {
                delete(pMemPool);
                return false;
            }
            // init the new MemoryPool.
            bool ok = pMemPool->init(exeMemStartAddress, numMemItems, MEMORY_ITEM_SIZE);
            assert(ok && "MemoryPool init failed");
        }

        // Enter the new MemoryPool to the data structures
        OclList<MemoryPool*>::Node* pMemPoolNode = m_memoryPoolsList.pushBack(pMemPool);
        m_startAddressToMemPool.insert( pair<void*, OclList<MemoryPool*>::Node*> (pMemPool->getStartAddress(), pMemPoolNode) );
        // If used the backup pool, set the backup as NULL
        if (pMemPool == m_backupPool)
        {
            m_backupPool = NULL;
        }

        // allocate the reserved memory from the Pool
        bool res = pMemPool->allocate(numOfItemsToAllocate, &tAddress);
        assert(res && "ERROR");
    }
    assert(tAddress && "ERROR");

    *reserveMemAddress = tAddress;
	assert(IS_ALIGN(*reserveMemAddress, MEMORY_ITEM_SIZE));
    return true;
}


void MemoryManager::freeReserveMem(void* reserveMemAddress)
{
    assert(reserveMemAddress && "reserveMemAddress == NULL");
    m_mutex.Lock();

    // reserveMemAddress suppose to be >= to startAddress of MemoryPool trying to find this Pool.
    map<void*, OclList<MemoryPool*>::Node*>::iterator addressToMemPoolNodeIter = m_startAddressToMemPool.lower_bound(reserveMemAddress);
    // if reserveMemAddress is not equal to MemoryPool start address that change iter position to one location before current location
    if ((addressToMemPoolNodeIter != m_startAddressToMemPool.begin()) && (reserveMemAddress != addressToMemPoolNodeIter->first))
    {
        --addressToMemPoolNodeIter;
    }
    assert(addressToMemPoolNodeIter != m_startAddressToMemPool.end() && "lower address than reserveMemAddress doesnt found");

    OclList<MemoryPool*>::Node* pMemPoolNode = addressToMemPoolNodeIter->second;
    // get the MemoryPool found
    MemoryPool* pMemPool = pMemPoolNode->data;

    // check correctness of the input
    assert((char*)reserveMemAddress >= (char*)pMemPool->getStartAddress() &&
        (char*)reserveMemAddress < (char*)pMemPool->getStartAddress() + (pMemPool->getTotalAmountOfItems() * pMemPool->getItemSize()));

    // free reserved memory from pool
    bool result = pMemPool->free(reserveMemAddress);
    assert(result);

    // If Nothing is Allocated in the MemoryPool and it is not the only pool in the list
    if ((pMemPool->isNothingAllocated()) && (m_startAddressToMemPool.size() > 1))
    {
        m_startAddressToMemPool.erase(addressToMemPoolNodeIter);
        m_memoryPoolsList.removeNode(pMemPoolNode);
        MemoryPool* poolToDelete = pMemPool;
        // If there is no backup pool or the backup size is smaller than this pool size that set this pool as backup pool
        if ((m_backupPool == NULL) || (pMemPool->getTotalAmountOfItems() > m_backupPool->getTotalAmountOfItems()))
        {
            poolToDelete = m_backupPool;
            m_backupPool = pMemPool;
        }

        m_mutex.Unlock();

        // unmap the executable memory and delete the MemmoryPool object (poolToDelete can be NULL if m_backupPool was NULL)
        if (poolToDelete)
        {
            unmapAndDeleteMemPool(poolToDelete);
        }
        return;
    }

    m_mutex.Unlock();
}

void MemoryManager::unmapAndDeleteMemPool(MemoryPool* pMemPool)
{
    int err = munmap(pMemPool->getStartAddress(), pMemPool->getTotalAmountOfItems() * pMemPool->getItemSize());
    assert(err == 0 && "munmap failed");
    delete(pMemPool);
}

void MemoryManager::release()
{
    // If exist backup MemoryPool than release its resources
    if (m_backupPool)
    {
        unmapAndDeleteMemPool(m_backupPool);
    }

    m_startAddressToMemPool.clear();

    OclListIterator<MemoryPool*> iter;
    OclListIterator<MemoryPool*> iterEnd = m_memoryPoolsList.end();
    // release the executable memory and resources of each MemoryPool
    for (iter = m_memoryPoolsList.begin(); iter != iterEnd; ++iter)
    {
        unmapAndDeleteMemPool(*iter);
    }
    m_memoryPoolsList.clear();
}

