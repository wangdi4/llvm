#include "pragmas.h"
#include "memory_pool.h"

#include <assert.h>

using namespace Intel::OpenCL::MicDevice;

MemoryPool::MemoryPool()
{
	m_startAddress = nullptr;
	m_numOfItems = 0;
	m_itemSize = 0;
	m_numOfFreeItems = 0;

	pthread_mutex_init(&m_mutex, nullptr);
}

MemoryPool::~MemoryPool()
{
	assert(m_allocatedAddresses.empty() && "Call release() before freeing all memory allocated");
	release();
	pthread_mutex_destroy(&m_mutex);
}

bool MemoryPool::init(void* startAddress, unsigned int numOfItems, unsigned int itemSize)
{
	assert(startAddress && "startAddress is NULL pointer");
	assert(numOfItems > 0 && "numOfItems = 0");
	assert(itemSize > 0 && "itemSize = 0");

	// if already initialized
	if (m_startAddress)
	{
		return false;
	}

	pthread_mutex_lock(&m_mutex);
	if (m_startAddress)
	{
		pthread_mutex_unlock(&m_mutex);
		return false;
	}
	NodeData tNodeData = {startAddress, numOfItems};
	// insert new memory node to m_freeMemList
	OclList<NodeData>::Node* memNode = m_freeMemList.pushFront(tNodeData);
	// insert to m_freeMemMap the startAddress as key and the created node pointer as value
	m_freeMemMap.insert( pair<void*, OclList<NodeData>::Node*> (startAddress, memNode) );

	m_numOfItems = numOfItems;
	m_itemSize = itemSize;
	m_numOfFreeItems = m_numOfItems;
	m_startAddress = startAddress;
	pthread_mutex_unlock(&m_mutex);

	return true;
}

bool MemoryPool::allocate(unsigned int numOfItems, void** address)
{
	assert(m_startAddress && "MemoryPool not initialized yet");
	// If num of free items is smaller than numOfItems return false
	if (m_numOfFreeItems < numOfItems)
	{
		return false;
	}
	bool result = false;
	void* pAddress = nullptr;
	OclListIterator<NodeData> iter;

	pthread_mutex_lock(&m_mutex);

	OclListIterator<NodeData> iterEnd = m_freeMemList.end();
	// traverse the free list from the beginning and trying to find node with size > "numOfItems". if found iter will point this node at the end of the "for" loop.
	for (iter = m_freeMemList.begin(); iter != iterEnd; ++iter)
	{
		if ((*iter).size >= numOfItems)
		{
			result = true;
			break;
		}
	}
	// If not found enouth memory in the pool.
	if (result == false)
	{
		pthread_mutex_unlock(&m_mutex);
		return false;
	}
	pAddress = (*iter).address;
	map<void*, OclList<NodeData>::Node*>::iterator freeMemMapIter = m_freeMemMap.find(pAddress);
	assert(freeMemMapIter != m_freeMemMap.end());
	// if the size of the memory pointed by iter is equal to "numOfItems" delete the node from m_freeMemList and from the map - m_freeMemMap
	if ((*iter).size == numOfItems)
	{
		// remove the node from m_freeMemList
		m_freeMemList.removeNode(iter);
	}
	else  // The node pointing by iter contain memory larger than "numOfItems" - shall update its content (size and address)
	{
		(*iter).address = (void*)((char*)((*iter).address) + numOfItems * m_itemSize);
		(*iter).size = (*iter).size - numOfItems;
		// enter the updated node to m_freeMemMap with new address (Going to remove the old one in the next instruction)
		m_freeMemMap.insert( pair<void*, OclList<NodeData>::Node*> ((*iter).address, freeMemMapIter->second) );
	}
	// remove the old item from m_freeMemMap (should removed because deleted from the list or because its key (address) changed)
	m_freeMemMap.erase(freeMemMapIter);
	// insert to m_allocatedAddresses the address allocated and its size
	m_allocatedAddresses.insert( pair<void*, unsigned int> (pAddress, numOfItems) );
	assert(m_numOfFreeItems >= numOfItems && "ERROR m_numOfFreeItems < 0");
	m_numOfFreeItems = m_numOfFreeItems - numOfItems;
	pthread_mutex_unlock(&m_mutex);

	// set the allocated address to out parameter.
	*address = pAddress;
	return true;
}


bool MemoryPool::free(void* address)
{
	assert(m_startAddress && "MemoryPool not initialized yet");
	// if the address to free is not in the address space of the pool
	if (((char*)address < (char*)m_startAddress) || ((char*)address >= (char*)m_startAddress + m_numOfItems * m_itemSize))
	{
		return false;
	}
	unsigned int numOfItems = 0;
	pthread_mutex_lock(&m_mutex);
	// find if address exist in m_allocatedAddresses map
	map<void*, unsigned int>::iterator allocAddressesIter = m_allocatedAddresses.find(address);
	// if not exist return false;
	if (allocAddressesIter == m_allocatedAddresses.end())
	{
		pthread_mutex_unlock(&m_mutex);
		return false;
	}
	numOfItems = allocAddressesIter->second;
	m_allocatedAddresses.erase(allocAddressesIter);

	addFreeMemory(address, numOfItems);

	m_numOfFreeItems = m_numOfFreeItems + numOfItems;
	assert(m_numOfFreeItems <= m_numOfItems && "ERROR m_numOfFreeItems > m_numOfItems");
	pthread_mutex_unlock(&m_mutex);
	return true;
}

bool MemoryPool::isNothingAllocated() const
{
	assert(m_startAddress && "MemoryPool not initialized yet");
	pthread_mutex_lock(&m_mutex);
	bool result = m_allocatedAddresses.empty();
	pthread_mutex_unlock(&m_mutex);
	return result;
}

bool MemoryPool::release()
{
	if (m_startAddress == nullptr)
	{
		return false;
	}
	pthread_mutex_lock(&m_mutex);
	if (m_startAddress == nullptr)
	{
		pthread_mutex_unlock(&m_mutex);
		return false;
	}
	m_allocatedAddresses.clear();
	m_freeMemMap.clear();
	m_freeMemList.clear();
	m_numOfFreeItems = m_numOfItems;
	m_numOfItems = 0;
	m_itemSize = 0;
	m_numOfFreeItems = 0;
	m_startAddress = nullptr;
	pthread_mutex_unlock(&m_mutex);
	return true;
}

void MemoryPool::addFreeMemory(void* address, unsigned int numOfItems)
{
	// If m_freeMemMap is empty just insert a new item to m_freeMemList and m_freeMemMap
	if (m_freeMemMap.empty())
	{
		NodeData tNodeData = {address, numOfItems};
		OclList<NodeData>::Node* memNode = m_freeMemList.pushFront(tNodeData);
		m_freeMemMap.insert( pair<void*, OclList<NodeData>::Node*> (address, memNode) );
		return;
	}
	//Handle left memory node
	bool mergeToLeft = false;
	map<void*, OclList<NodeData>::Node*>::iterator freeMemMapIterator = m_freeMemMap.lower_bound(address);
	if (freeMemMapIterator != m_freeMemMap.begin())
	{
		freeMemMapIterator --;
	}
	OclList<NodeData>::Node* leftNode = freeMemMapIterator->second;
	// if the address pointed by freeMemMapIterator + its size * m_itemSize == address can merge to left memNode
	if ((void*)((char*)(leftNode->data.address) + leftNode->data.size * m_itemSize) == address)
	{
		leftNode->data.size = leftNode->data.size + numOfItems;
		mergeToLeft = true;
	}

	//Handle right memory node
	bool mergeToRight = false;
	freeMemMapIterator = m_freeMemMap.upper_bound(address);
	// if exist item with address larger than input address
	if (freeMemMapIterator != m_freeMemMap.end())
	{
		OclList<NodeData>::Node* rightNode = freeMemMapIterator->second;
		// if the input address + the input numOfItems * m_itemSize == address pointed by freeMemMapIterator can merge the right memNode
		if ((void*)((char*)address + numOfItems * m_itemSize) == rightNode->data.address)
		{
			// if also merged with left memNode can update left memNode and remove the right memNode
			if (mergeToLeft)
			{
				// update left node side (merge left and right nodes)
				leftNode->data.size = leftNode->data.size + rightNode->data.size;
				// remove the right item from m_freeMemList
				m_freeMemList.removeNode(rightNode);
			}
			else // merge only with right memNode
			{
				// update right memNode address to input address
				rightNode->data.address = address;
				// update right memNode size
				rightNode->data.size = rightNode->data.size + numOfItems;
				// update m_freeMemMap becuase the value of the key (address) changed
				m_freeMemMap.insert( pair<void*, OclList<NodeData>::Node*> (address, rightNode) );
			}
			// remove the right item from m_freeMemMap
			m_freeMemMap.erase(freeMemMapIterator);
			mergeToRight = true;
		}
	}
	// if already merged finish
	if ((mergeToLeft) || (mergeToRight))
	{
		return;
	}
	// didn't merge so have to create new memNode
	NodeData tNodeData = {address, numOfItems};
	OclList<NodeData>::Node* memNode = m_freeMemList.pushFront(tNodeData);
	m_freeMemMap.insert( pair<void*, OclList<NodeData>::Node*> (address, memNode) );
}
