#include "notification_port.h"

#include <common/COIEvent_common.h>
#include <source/COIEvent_source.h>

#include <malloc.h>
#include <cstring>
#include <algorithm>
#include <assert.h>

using namespace Intel::OpenCL::MICDevice;

#define CALL_BACKS_ARRAY_RESIZE_AMOUNT 1024

NotificationPort::NotificationPort(void) : m_poc(0), m_barriers(NULL), m_notificationsPackages(NULL), m_maxBarriers(0), m_waitingSize(0), m_realSize(0), m_lastCallBackAge(0), m_workerState(CREATED)
{
	// initialize client critical section object (mutex)
	pthread_mutex_init(&m_mutex, NULL);
	// initialize client condition variable
	pthread_cond_init(&m_clientCond, NULL);
	// initialize resize condition variable
	pthread_cond_init(&m_resizeCond, NULL);
}

NotificationPort::~NotificationPort(void)
{
	release();
	// destroy condition variables
	pthread_cond_destroy(&m_resizeCond);
	pthread_cond_destroy(&m_clientCond);
	// destroy mutex
	pthread_mutex_destroy(&m_mutex);
}

int NotificationPort::initialize(uint16_t maxBarriers)
{
	assert(maxBarriers < INT16_MAX && "maxBarriers must be smaller than INT16_MAX");
	pthread_mutex_lock(&m_mutex);
	// if initialize already called after constructor or release operation
	if (m_maxBarriers > 0)
	{
		pthread_mutex_unlock(&m_mutex);
		return ALREADY_INITIALIZE_FAILURE;
	}
	// Add 1 barrier for the main barrier (The first barrier)
	m_maxBarriers = maxBarriers + 1;

	m_barriers = NULL;
	m_notificationsPackages = NULL;

	m_barriers = (COIEVENT*)malloc(m_maxBarriers * sizeof(COIEVENT));
	if (!m_barriers)
	{
		releaseResources();
		pthread_mutex_unlock(&m_mutex);
		return MEM_OBJECT_ALLOCATION_FAILURE;
	}

	m_notificationsPackages = (notificationPackage*)malloc(m_maxBarriers * sizeof(notificationPackage));
	if (!m_notificationsPackages)
	{
		releaseResources();
		pthread_mutex_unlock(&m_mutex);
		return MEM_OBJECT_ALLOCATION_FAILURE;
	}

	// reset the operation mask
	memset(m_operationMask, 0, sizeof(bool) * AVAILABLE_OPERATIONS_LEN);

	// create the main thread barrier
	COIRESULT result = COIEventRegisterUserEvent(m_barriers);

	if (result != COI_SUCCESS)
	{
		releaseResources();
		pthread_mutex_unlock(&m_mutex);
		return CREATE_BARRIER_FAILURE;
	}

	m_waitingSize = 1;
	m_realSize = 1;

	pthread_attr_t tattr;
	pthread_attr_init(&tattr);
	pthread_attr_setdetachstate(&tattr, PTHREAD_CREATE_DETACHED);

	m_poc ++;

	m_workerState = BEGINNING;

	if (0 != pthread_create(&m_workerThread, &tattr, ThreadEntryPoint, (void*)this))
	{
		pthread_attr_destroy(&tattr);
		releaseResources();
		pthread_mutex_unlock(&m_mutex);
		return CREATE_WORKER_THREAD_FAILURE;
	}
	pthread_attr_destroy(&tattr);

	// wait until the worker thread will start execution
	while (BEGINNING == m_workerState)
	{
	    pthread_cond_wait(&m_clientCond, &m_mutex);
	}
	pthread_mutex_unlock(&m_mutex);

	return SUCCESS;
}

int NotificationPort::addBarrier(const COIEVENT &barrier, NotificationPort::CallBack *callBack, void *arg)
{
    COIRESULT result = COI_SUCCESS;

	assert(callBack && "Error - callBack must be non-NULL pointer");
	pthread_mutex_lock(&m_mutex);

	// The object does not initialize
	if (m_maxBarriers == 0)
	{
		pthread_mutex_unlock(&m_mutex);
		return NOT_INITIASLIZE_FAILURE;
	}

	// Exceed the barrier array size
	while (m_realSize >= m_maxBarriers)
	{
		m_operationMask[RESIZE] = true;
		result = COIEventSignalUserEvent(m_barriers[0]);
		assert(result == COI_SUCCESS && "Signal main barrier failed");

		// wait for the allocation
	    pthread_cond_wait(&m_resizeCond, &m_mutex);

		// The object does not initialize
		if (m_maxBarriers == 0)
		{
			pthread_mutex_unlock(&m_mutex);
			return NOT_INITIASLIZE_FAILURE;
		}
	}

	m_barriers[m_realSize] = barrier;
	m_notificationsPackages[m_realSize].callBack = callBack;
	m_notificationsPackages[m_realSize].arg = arg;
	m_notificationsPackages[m_realSize].age = m_lastCallBackAge;
	m_lastCallBackAge ++;
	m_realSize ++;

	m_operationMask[ADD] = true;

	result = COIEventSignalUserEvent(m_barriers[0]);
	assert(result == COI_SUCCESS && "Signal main barrier failed");

	pthread_mutex_unlock(&m_mutex);

	return SUCCESS;
}

int NotificationPort::release()
{
	COIRESULT result = COI_SUCCESS;

    pthread_mutex_lock(&m_mutex);

	// The object does not initialize
	if (m_maxBarriers == 0)
	{
		pthread_mutex_unlock(&m_mutex);
		return NOT_INITIASLIZE_FAILURE;
	}

	m_operationMask[RELEASE] = true;

	result = COIEventSignalUserEvent(m_barriers[0]);
	assert(result == COI_SUCCESS && "Signal main barrier failed");

	// wait for the termination of worker thread
	size_t currPoc = m_poc;
	while ((m_workerState != FINISHED) && (currPoc == m_poc))
	{
	    pthread_cond_wait(&m_clientCond, &m_mutex);
	}

	pthread_mutex_unlock(&m_mutex);

	return 0;
}

void* NotificationPort::ThreadEntryPoint(void *threadObject)
{
	bool keepWork = true;
	NotificationPort* thisWorker = (NotificationPort*)threadObject;

	vector<notificationPackage> fireCallBacksArr;
	fireCallBacksArr.reserve(thisWorker->m_maxBarriers);
	unsigned int* firedIndicesArr = (unsigned int*)malloc(sizeof(unsigned int) * thisWorker->m_maxBarriers);
	assert(firedIndicesArr && "memory allocation for firedIndicesArr failed");
	unsigned int firedAmount;
	bool workerThreadSigaled;

	COIRESULT result;

	// Block the main thread until this point in order to ensure that the thread begin it's life before initialize() method completed
	pthread_mutex_lock(&thisWorker->m_mutex);
	thisWorker->m_workerState = RUNNING;
	pthread_cond_signal(&thisWorker->m_clientCond);
	pthread_mutex_unlock(&thisWorker->m_mutex);

	while (keepWork)
	{
		firedAmount = 0;
		workerThreadSigaled = false;

		// wait for barrier(s) signal(s)
		result = COIEventWait(thisWorker->m_waitingSize, thisWorker->m_barriers, -1, false, &firedAmount, firedIndicesArr);
		assert(result == COI_SUCCESS && "COIBarrierWait failed for some reason");

		pthread_mutex_lock(&(thisWorker->m_mutex));

		// get all the signaled barriers.
		thisWorker->getFiredCallBacks(firedAmount, firedIndicesArr, fireCallBacksArr, &workerThreadSigaled);

		// If the main thread signaled
		if (workerThreadSigaled)
		{
		    result = COIEventUnregisterUserEvent(thisWorker->m_barriers[0]);
			assert(result == COI_SUCCESS && "UnRegister main barrier failed");
			result = COIEventRegisterUserEvent(&(thisWorker->m_barriers[0]));
			assert(result == COI_SUCCESS && "Register main barrier failed");
			firedAmount --;
			// If Add barrier operation
			if (thisWorker->m_operationMask[ADD] == true)
			{
				thisWorker->m_waitingSize = thisWorker->m_realSize;
				thisWorker->m_operationMask[ADD] = false;
			}
			// If Resize operation
			if (thisWorker->m_operationMask[RESIZE] == true)
			{
				thisWorker->resizeBuffers(&fireCallBacksArr, &firedIndicesArr);
				thisWorker->m_operationMask[RESIZE] = false;
			}
			// If Release operation
			if (thisWorker->m_operationMask[RELEASE] == true)
			{
				keepWork = false;
			}
		}

		pthread_mutex_unlock(&(thisWorker->m_mutex));

		// If more than one element fired, sort the fired element according to their age
		if (firedAmount > 1)
		{
			sort(fireCallBacksArr.begin(), fireCallBacksArr.begin() + firedAmount, notificationPackage::compare);
		}

		for (unsigned int i = 0; i < firedAmount; i++)
		{
			fireCallBacksArr[i].callBack->fireCallBack(fireCallBacksArr[i].arg);
		}

	}

	thisWorker->releaseResources();

	free(firedIndicesArr);

	return 0;
}

void NotificationPort::getFiredCallBacks(unsigned int numSignaled, unsigned int* signaledIndices, vector<notificationPackage>& callBacksRet, bool* workerThreadSignaled)
{
	unsigned int initialWaitingSize = m_waitingSize;
	unsigned int notSignaledIndex = m_realSize - 1;

	unsigned int index = 0;
	for (unsigned int i = 0; i < numSignaled; i++)
	{
		// if worker thread signaled
		if (signaledIndices[i] == 0)
		{
			*workerThreadSignaled = true;
			continue;
		}
		// save the notification package
		callBacksRet[index] = m_notificationsPackages[signaledIndices[i]];
		index ++;
		// signature which give hint that the barrier signaled. (hint for the swaps in the next loop)
		m_notificationsPackages[signaledIndices[i]].callBack = NULL;
	}

	// swap signaled barriers with non-signaled
	for (unsigned int i = 0; i < numSignaled; i++)
	{
		// if worker thread signaled
		if (signaledIndices[i] == 0)
		{
			continue;
		}
		// while barrier in location notSignaledIndex is fired and notSignaledIndex >= current barrier index
		while ((notSignaledIndex >= signaledIndices[i]) && (m_notificationsPackages[notSignaledIndex].callBack == NULL))
		{
			notSignaledIndex --;
		}
		if (notSignaledIndex > signaledIndices[i])
		{
			// switch between the current barrier and the last not signaled barrier.
			m_barriers[signaledIndices[i]] = m_barriers[notSignaledIndex];
			m_notificationsPackages[signaledIndices[i]] = m_notificationsPackages[notSignaledIndex];

			// if swap with index larger than initialWaitingSize increasing m_waitingSize by one because it will decrease by one at the end of the loop
			if (notSignaledIndex >= initialWaitingSize)
			{
				m_waitingSize ++;
			}
			notSignaledIndex --;
		}

		m_realSize --;
		m_waitingSize --;
	}
}


void NotificationPort::resizeBuffers(vector<notificationPackage>* fireCallBacksArr, unsigned int** firedIndicesArr)
{
	assert(m_maxBarriers + CALL_BACKS_ARRAY_RESIZE_AMOUNT <= INT16_MAX && "Resize failed overflow max barriers size");
	m_maxBarriers = m_maxBarriers + CALL_BACKS_ARRAY_RESIZE_AMOUNT;
	m_barriers = (COIEVENT*)realloc(m_barriers, m_maxBarriers * sizeof(COIEVENT));
	assert(m_barriers && "memory allocation failed for m_barriers");
	m_notificationsPackages = (notificationPackage*)realloc(m_notificationsPackages, m_maxBarriers * sizeof(notificationPackage));
	assert(m_notificationsPackages && "memory allocation failed for m_notificationsPackages");
	fireCallBacksArr->reserve(m_maxBarriers);
	*firedIndicesArr = (unsigned int*)realloc(*firedIndicesArr, sizeof(unsigned int) * m_maxBarriers);
	assert(*firedIndicesArr && "memory allocation failed for *firedIndicesArr");
	pthread_cond_broadcast(&m_resizeCond);
}


void NotificationPort::releaseResources()
{
    COIRESULT result = COI_SUCCESS;

	pthread_mutex_lock(&m_mutex);

	m_maxBarriers = 0;
	//release the worker barrier
	if (m_barriers)
	{
		result = COIEventUnregisterUserEvent(m_barriers[0]);
		assert(result == COI_SUCCESS && "Unregister main barrier failed");
		free(m_barriers);
		m_barriers = NULL;
	}
	if (m_notificationsPackages)
	{
		free(m_notificationsPackages);
		m_notificationsPackages = NULL;
	}
	m_waitingSize = 0;
	m_realSize = 0;

	pthread_cond_broadcast(&m_resizeCond);
	m_workerState = FINISHED;
	pthread_cond_broadcast(&m_clientCond);

	pthread_mutex_unlock(&m_mutex);
}
