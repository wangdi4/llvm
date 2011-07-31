#pragma once

#include "ocl_list.h"

#include <pthread.h>
#include <map>

using namespace std;

namespace Intel { namespace OpenCL { namespace MicDevice {

/* Class that represent memory pool with operations such as allocate and free.
   It is thread safe implementation */
class MemoryPool
{

public:

	/* Default constructor, the MemoryPool is not ready until init() will call */
	MemoryPool();

	/* Destructor */
	virtual ~MemoryPool();

	/* Initialize the memory pool.
	   If other thread is already initialize the startAddress, return false.
	   startAddress - The start address of executable memory already allocated. (The client is responsible to allocate and free the memory pointed by startAddress).
	   numOfItems - The amount of memory items allocated.
	   itemSize - The granularity (size in bytes) of each memory item. 
	   return true if succeeded and false otherwise */
	virtual bool init(void* startAddress, unsigned int numOfItems, unsigned int itemSize);

	/* Allocate memory for the client.
	   numOfItems - the memory size.
	   address - out parameter the address allocated.
	   return true if succeeded and false otherwise */
	virtual bool allocate(unsigned int numOfItems, void** address);

	/* Free allocated memory.
	   address - the address to free.
	   return true if succeeded and false othrewise, for example if address does not exist (which mean does not allocated before) */
	virtual bool free(void* address);

	/* Return true if the pool is empty.
	   Must protect the MemoryPool object with external lock in order to be sure that the object state is not changed after the internal lock released*/
	bool isNothingAllocated() const;

	/* Return MemoryPool start address */
	virtual void* getStartAddress() { return m_startAddress; };

	/* Return MemoryPool m_numOfItems */
	unsigned int getTotalAmountOfItems() { return m_numOfItems; };

	/*Return memoryPool m_itemSize */
	unsigned int getItemSize() { return m_itemSize; };

protected:

	/* Clear the content of the pool and return the state of the object as before init state. 
	   Return true if succeeded and false otherwise. */
	bool release();

private:

	struct NodeData
	{
		void* address;
		unsigned int size;
	};

	/* Add released memory to the data structures - m_freeMemList, m_freeMemMap
	   address - the address of the memory
	   numOfItems - the amount of items the memory contained. */
	void addFreeMemory(void* address, unsigned int numOfItems);

	// The start address of the executable memory
	void* m_startAddress;

	// The amount of items allocated
	unsigned int m_numOfItems;

	// The granularity of each item (size in bytes)
	unsigned int m_itemSize;

	// The amount of currently free items
	unsigned int m_numOfFreeItems;

	// free memory list contain free addresses and their size
	OclList<NodeData> m_freeMemList;

	// map from free memory address to FreeMemoryList node (Need for merge continues memory chunks)
	map<void*, OclList<NodeData>::Node*> m_freeMemMap;

	// map from allocated memory address to its size
	map<void*, unsigned int> m_allocatedAddresses;
	
	mutable pthread_mutex_t m_mutex;

};

}}}