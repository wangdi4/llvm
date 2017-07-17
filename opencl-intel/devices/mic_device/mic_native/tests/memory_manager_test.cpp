#include "memory_manager.h"
#include "program_manager.h"
#include "ocl_list.h"

#include <stdio.h>
#include <cstdlib>
#include <algorithm>
#include <time.h>
#include <vector>
#include <map>
#include <pthread.h>
#include <assert.h>

using namespace Intel::OpenCL::MicDevice;
using namespace std;

#define MEMORY_ITEM_SIZE 4096
#define MINIMUM_MEM_ITEMS_IN_POOL 256
#define MINIMUM_BYTE_IN_POOL MEMORY_ITEM_SIZE * MINIMUM_MEM_ITEMS_IN_POOL

#define RESIZE_MEM_MUL 1.5

bool reservationTest()
{
	printf("reservationTest");
	bool result = true;
	ProgramManager prog[10];
	void* allocations[10];
	result &= prog[0].reserveExecutableMemory(((MINIMUM_BYTE_IN_POOL / 4) / 4) / RESIZE_MEM_MUL, 4);
	result &= prog[1].reserveExecutableMemory(((MINIMUM_BYTE_IN_POOL / 2) / 8) / RESIZE_MEM_MUL, 8);
	result &= prog[2].reserveExecutableMemory(((MINIMUM_BYTE_IN_POOL * 1.5) / 4) / RESIZE_MEM_MUL, 4);
	result &= prog[3].reserveExecutableMemory(((MINIMUM_BYTE_IN_POOL * 2.5) / 4) / RESIZE_MEM_MUL, 4);
	result &= prog[4].reserveExecutableMemory(((MINIMUM_BYTE_IN_POOL / 4) / 4) / RESIZE_MEM_MUL, 4);
	result &= prog[5].reserveExecutableMemory(((MINIMUM_BYTE_IN_POOL / 4) / 4) / RESIZE_MEM_MUL, 4);
	result &= prog[6].reserveExecutableMemory(((MINIMUM_BYTE_IN_POOL / 4) / 4) / RESIZE_MEM_MUL, 4);
	result &= prog[7].reserveExecutableMemory(((MINIMUM_BYTE_IN_POOL / 4) / 4) / RESIZE_MEM_MUL, 4);
	result &= prog[8].reserveExecutableMemory(((MINIMUM_BYTE_IN_POOL / 4) / 4) / RESIZE_MEM_MUL, 4);
	result &= prog[9].reserveExecutableMemory(((MINIMUM_BYTE_IN_POOL * 1.5) / 4) / RESIZE_MEM_MUL, 4);

	for (unsigned int i = 0; i < 10; i++)
	{
		result &= prog[i].allocate(1, &(allocations[i]));
	}
	if (result == false)
	{
		return false;
	}
	void* address = (void*)((char*)allocations[0] + (MINIMUM_BYTE_IN_POOL / 4));
	if (allocations[1] != address)
	{
		return false;
	}
	address = (void*)((char*)allocations[0] + (MINIMUM_BYTE_IN_POOL / 4) + (MINIMUM_BYTE_IN_POOL / 2));
	if (allocations[4] != address)
	{
		return false;
	}
	address = (void*)((char*)allocations[5] + (MINIMUM_BYTE_IN_POOL / 4));
	if (allocations[6] != address)
	{
		return false;
	}
	address = (void*)((char*)allocations[5] + (MINIMUM_BYTE_IN_POOL / 4) + (MINIMUM_BYTE_IN_POOL / 4));
	if (allocations[7] != address)
	{
		return false;
	}
	address = (void*)((char*)allocations[5] + (MINIMUM_BYTE_IN_POOL / 4) + (MINIMUM_BYTE_IN_POOL / 4) + (MINIMUM_BYTE_IN_POOL / 4));
	if (allocations[8] != address)
	{
		return false;
	}
	prog[0].freeReservedMemory();
	prog[4].freeReservedMemory();

	prog[3].freeReservedMemory();

	prog[6].freeReservedMemory();
	prog[7].freeReservedMemory();

	result &= prog[0].reserveExecutableMemory(((MINIMUM_BYTE_IN_POOL / 2) / 8) / RESIZE_MEM_MUL, 8);
	result &= prog[4].reserveExecutableMemory(((MINIMUM_BYTE_IN_POOL / 4) / 8) / RESIZE_MEM_MUL, 8);
	result &= prog[3].reserveExecutableMemory(((MINIMUM_BYTE_IN_POOL / 4) / 8) / RESIZE_MEM_MUL, 8);
	result &= prog[6].reserveExecutableMemory(((MINIMUM_BYTE_IN_POOL / 4) / 8) / RESIZE_MEM_MUL, 8);
	result &= prog[7].reserveExecutableMemory(((MINIMUM_BYTE_IN_POOL) / 8) / RESIZE_MEM_MUL, 8);

	void* oldP0Address = allocations[0];
	void* oldP3Address = allocations[3];

	prog[0].allocate(1, &(allocations[0]));
	prog[4].allocate(1, &(allocations[4]));
	prog[3].allocate(1, &(allocations[3]));
	prog[6].allocate(1, &(allocations[6]));
	
	address = (void*)((char*)allocations[5] + (MINIMUM_BYTE_IN_POOL / 4));
	if (allocations[0] != address)
	{
		return false;
	}
	address = (void*)((char*)oldP0Address + (MINIMUM_BYTE_IN_POOL / 4) + (MINIMUM_BYTE_IN_POOL / 2));
	if (allocations[4] != address)
	{
		return false;
	}
	address = oldP0Address;
	if (allocations[3] != address)
	{
		return false;
	}
	address = oldP3Address;
	if (allocations[6] != address)
	{
		return false;
	}

	prog[1].freeReservedMemory();
	prog[2].freeReservedMemory();

	prog[5].freeReservedMemory();

	prog[8].freeReservedMemory();
	prog[9].freeReservedMemory();

	return result;
}

struct MemHandle
{
	void* address;
	unsigned int numOfItems;
};

bool ProgramMemoryAllocationTest()
{
	printf("ProgramMemoryAllocationTest\n");
	unsigned int numOfFree = 0;
	unsigned int numOfAlloc = 0;
	bool result = true;
	ProgramManager prog;
	result &= prog.reserveExecutableMemory(MEMORY_ITEM_SIZE , 4);
	void* startAddress = nullptr;
	void* address = nullptr;
	for (unsigned int i = 0; i < MEMORY_ITEM_SIZE * RESIZE_MEM_MUL; i++)
	{
		result &= prog.allocate(1,  &address);
		if (i == 0)
		{
			startAddress = address;
		}
	}

	if (prog.allocate(1,  &address))
	{
		return false;
	}
	address = startAddress;
	if (prog.free((void*)((char*)startAddress - 1)))
	{
		return false;
	}
	for (unsigned int i = 0; i < MEMORY_ITEM_SIZE * RESIZE_MEM_MUL; i++)
	{
		result &= prog.free(address);
		address = (char*)address + 4;
	}

	if (prog.free(startAddress))
	{
		return false;
	}

	const unsigned int rounds = 10000000;
	unsigned int numOfItems = 1;
	unsigned int sumAllocSize = 0;
	vector<MemHandle> allocAddress;
	srand(time(nullptr));
	for (unsigned int i = 0; i < rounds; i++)
	{
		if ((rand() % 2 == 0) || (allocAddress.size() == 0))
		{
			numOfItems = rand() % (min(32,MEMORY_ITEM_SIZE / 2)) + 1;
			while (sumAllocSize + numOfItems > MEMORY_ITEM_SIZE)
			{
				unsigned int index = rand() % allocAddress.size();
				if (prog.free(allocAddress[index].address) == false)
				{
					return false;
				}
				numOfFree ++;
				sumAllocSize = sumAllocSize - allocAddress[index].numOfItems;
				allocAddress.erase(allocAddress.begin() + index);
			}
			if (prog.allocate(numOfItems, &address) == false)
			{
				return false;
			}
			numOfAlloc ++;
			sumAllocSize = sumAllocSize + numOfItems;
			MemHandle m = {address, numOfItems};
			allocAddress.push_back(m);
		}
		else
		{
			unsigned int index = rand() % allocAddress.size();
			if (prog.free(allocAddress[index].address) == false)
			{
				return false;
			}
			numOfFree ++;
			sumAllocSize = sumAllocSize - allocAddress[index].numOfItems;
			allocAddress.erase(allocAddress.begin() + index);
		}
	}

	prog.freeReservedMemory();
	printf("\tNum of allocations = %d, Num of free = %d\n", numOfAlloc, numOfFree);
	return result;
}

struct thread_data
{
	ProgramManager* prog;
	map<void*, MemHandle>* allocationsMap;
	pthread_mutex_t* mutex;
	unsigned int maxSizeToAllocate;
	unsigned int numThreads;
	volatile bool result;
};

void* threadStartPoint(void* arg)
{
	thread_data* threadData = (thread_data*)arg;
	ProgramManager* prog = threadData->prog;
	map<void*, MemHandle>* allocationsMap = threadData->allocationsMap;
	vector<MemHandle> myAllocations;
	pthread_mutex_t* mutex = threadData->mutex;
	unsigned int maxSizeToAllocate = threadData->maxSizeToAllocate;
	unsigned int numThreads = threadData->numThreads;
	const unsigned int rounds = 1000000;
	void* address = nullptr;
	unsigned int numOfItems = 1;
	unsigned int sumAllocSize = 0;
	srand(time(nullptr));
	for (unsigned int i = 0; i < rounds; i++)
	{
		if ((rand() % 2 > 0) || (myAllocations.size() == 0))
		{
			numOfItems = rand() % (maxSizeToAllocate) + 1;
			while (sumAllocSize + numOfItems > (max((unsigned int)1,MEMORY_ITEM_SIZE / numThreads)))
			{
				unsigned int index = rand() % myAllocations.size();
				pthread_mutex_lock(mutex);
				allocationsMap->erase(myAllocations[index].address);
				pthread_mutex_unlock(mutex);
				if (prog->free(myAllocations[index].address) == false)
				{
					printf("FAIL\n");
					__sync_bool_compare_and_swap(&(threadData->result), true, false);
					return nullptr;
				}
				sumAllocSize = sumAllocSize - myAllocations[index].numOfItems;
				myAllocations.erase(myAllocations.begin() + index);
			}
			if (prog->allocate(numOfItems, &address) == false)
			{
				printf("FAIL\n");
				__sync_bool_compare_and_swap(&(threadData->result), true, false);
				return nullptr;
			}
			sumAllocSize = sumAllocSize + numOfItems;
			MemHandle m = {address, numOfItems};
			myAllocations.push_back(m);
			pthread_mutex_lock(mutex);
//			printf("map size = %d\n", allocationsMap->size());
			map<void*, MemHandle>::iterator iter = allocationsMap->lower_bound(address);
			if (iter != allocationsMap->end())
			{
//				printf("allocated from %p to %p, next address is %p\n", address, (void*)((char*)address + numOfItems * 4), iter->first);
				if ((void*)((char*)address + numOfItems * 4) > iter->first)
				{
					printf("FAIL\n");
					__sync_bool_compare_and_swap(&(threadData->result), true, false);
					return nullptr;
				}
			}
			if (iter != allocationsMap->begin())
			{
				iter --;
//				printf("allocated from %p, previous address is from %p to %p\n", address, iter->first, (void*)((char*)(iter->first) + iter->second.numOfItems * 4));
				if ((void*)((char*)(iter->first) + iter->second.numOfItems * 4) > address)
				{
					printf("FAIL\n");
					__sync_bool_compare_and_swap(&(threadData->result), true, false);
					return nullptr;
				}
			}
			allocationsMap->insert( pair<void*, MemHandle> (address, m) );
			pthread_mutex_unlock(mutex);
		}
		else
		{
			unsigned int index = rand() % myAllocations.size();
			pthread_mutex_lock(mutex);
			allocationsMap->erase(myAllocations[index].address);
			pthread_mutex_unlock(mutex);
			if (prog->free(myAllocations[index].address) == false)
			{
				printf("FAIL\n");
				__sync_bool_compare_and_swap(&(threadData->result), true, false);
				return nullptr;
			}
			sumAllocSize = sumAllocSize - myAllocations[index].numOfItems;
			myAllocations.erase(myAllocations.begin() + index);
		}
	}


	return nullptr;
}

bool ProgramMemoryAllocationMultiThreadedTest(unsigned int numThreads)
{
	printf("ProgramMemoryAllocationMultiThreadedTest");
	map<void*, MemHandle> allocationsMap;
	pthread_mutex_t mutex;
	pthread_mutex_init(&mutex, nullptr);
	pthread_t* threads = (pthread_t*)malloc(sizeof(pthread_t) * numThreads);
	thread_data threadsData;
	unsigned int maxItemsSize = max((const unsigned int)1, min(32 / numThreads ,(MEMORY_ITEM_SIZE / 2) / numThreads));
	
	ProgramManager prog;
	bool result = prog.reserveExecutableMemory(MEMORY_ITEM_SIZE , 4);

	threadsData.prog = &prog;
	threadsData.allocationsMap = &allocationsMap;
	threadsData.mutex = &mutex;
	threadsData.maxSizeToAllocate = maxItemsSize;
	threadsData.numThreads = numThreads;
	threadsData.result = true;
	
	for (unsigned int i = 0; i < numThreads; i++)
	{
		pthread_create(&(threads[i]), nullptr, threadStartPoint, &threadsData);
	}
	for (unsigned int i = 0; i < numThreads; i++)
	{
		pthread_join(threads[i], nullptr);
	}
	
	pthread_mutex_destroy(&mutex);
	free(threads);
	return threadsData.result;
}

int main()
{
	bool result = true;
	bool currResult = reservationTest();
	result &= currResult;
	if (currResult)
	{
		printf(" - Pass\n");
	}
	currResult &= ProgramMemoryAllocationTest();
	result &= currResult;
	if (currResult)
	{
		printf("ProgramMemoryAllocationTest - Pass\n");
	}
	currResult &= ProgramMemoryAllocationMultiThreadedTest(8);
	result &= currResult;
	if (currResult)
	{
		printf(" - Pass\n");
	}
	MemoryManager::releaseMemoryManager();
	printf("ByeBye Result = %s\n", (result ? "True" : "False"));
	return 0;
}
