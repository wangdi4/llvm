#include "program_memory_manager.h"
#include "memory_manager.h"

using namespace Intel::OpenCL::MICDeviceNative;

#define RESIZE_MEM_MUL 1.5

ProgramMemoryManager::ProgramMemoryManager() : MemoryPool() {}

ProgramMemoryManager::~ProgramMemoryManager()
{
    freeReservedMemory();
}

bool ProgramMemoryManager::reserveExecutableMemory(unsigned int numOfItems, unsigned int itemSize)
{
    // if already reserved return false;
    if (getStartAddress())
    {
        return false;
    }
    void* tReserveMemAddress = nullptr;

    // enlarge the allocated memory size by multiply of RESIZE_MEM_MUL
    unsigned int tResizeNumOfItems = numOfItems * RESIZE_MEM_MUL;
    // reserve memory
    bool result = MemoryManager::getInstance().reserveMem(tResizeNumOfItems, itemSize, &tReserveMemAddress);
    // if reserve operation succeeded
    if (result)
    {
        // If init operation failed, it means that other thread already initialize the memory
        if (init(tReserveMemAddress, tResizeNumOfItems, itemSize) == false)
        {
            MemoryManager::getInstance().freeReserveMem(tReserveMemAddress);
            result = false;
        }
    }
    return result;
}

void ProgramMemoryManager::freeReservedMemory()
{
    // get the reserved memory address
    void* tReserveAddress = getStartAddress();
    // if not initialized or already released
    if (tReserveAddress == nullptr)
    {
        return;
    }
    // If executable memory initialized
    if (release())
    {
        // release this program reserved memory
        MemoryManager::getInstance().freeReserveMem(tReserveAddress);
    }
}

