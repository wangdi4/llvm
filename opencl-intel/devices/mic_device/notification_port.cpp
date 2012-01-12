#include "notification_port.h"

#include <common/COIEvent_common.h>
#include <source/COIEvent_source.h>

#include <malloc.h>
#include <cstring>
#include <algorithm>
#include <signal.h>
#include <assert.h>

using namespace Intel::OpenCL::MICDevice;

#define CALL_BACKS_ARRAY_RESIZE_AMOUNT 1024

set<pthread_t> NotificationPort::m_NotificationThreadsSet;
OclMutex      NotificationPort::m_notificationThreadsMutex;

void NotificationPort::registerNotificationPortThread(pthread_t threadHandler)
{
	// Add the new thread to the live threads set.
	OclAutoMutex lock(&m_notificationThreadsMutex);
	m_NotificationThreadsSet.insert(threadHandler);
}

void NotificationPort::unregisterNotificationPortThread(pthread_t threadHandler)
{
	// Remove the thread from the live threads set.
	OclAutoMutex lock(&m_notificationThreadsMutex);
	m_NotificationThreadsSet.erase(threadHandler);
}

void NotificationPort::waitForAllNotificationPortThreads()
{
	vector<pthread_t> liveThreadsSnapshot;
	{
		// Get snapshot of the current threads alive.
		OclAutoMutex lock(&m_notificationThreadsMutex);
		liveThreadsSnapshot.insert(liveThreadsSnapshot.begin(), m_NotificationThreadsSet.begin(), m_NotificationThreadsSet.end());
	}
	while (liveThreadsSnapshot.size() > 0)
	{
		// Waits for all threads.
		for (unsigned int i = 0; i < liveThreadsSnapshot.size(); i++)
		{
			while (ESRCH != pthread_kill(liveThreadsSnapshot[i], 0))
			{
				pthread_yield();
			}
		}

		liveThreadsSnapshot.clear();
		
		{
			// Get snapshot of the current threads alive.
			OclAutoMutex lock(&m_notificationThreadsMutex);
			liveThreadsSnapshot.insert(liveThreadsSnapshot.begin(), m_NotificationThreadsSet.begin(), m_NotificationThreadsSet.end());
		}
	}
}

NotificationPort::NotificationPort(void) : m_poc(0), m_barriers(NULL), m_notificationsPackages(NULL), m_maxBarriers(0), m_waitingSize(0), m_lastCallBackAge(0), m_workerState(CREATED)
{
	// initialize client critical section object (mutex)
	pthread_mutex_init(&m_mutex, NULL);
	// initialize client condition variable
	pthread_cond_init(&m_clientCond, NULL);
}

NotificationPort::~NotificationPort(void)
{
	// destroy condition variables
	pthread_cond_destroy(&m_clientCond);
	// destroy mutex
	pthread_mutex_destroy(&m_mutex);
}

NotificationPort* NotificationPort::notificationPortFactory(uint16_t maxBarriers)
{
	NotificationPort* retNotificationPort = new NotificationPort();
	if (NULL == retNotificationPort)
	{
		return NULL;
	}

	int err = retNotificationPort->initialize(maxBarriers);
	if (SUCCESS != err)
	{
		delete retNotificationPort;
		retNotificationPort = NULL;
	}

	return retNotificationPort;
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
	// Reserve "maxBarriers" to "m_pendingNotificationArr". (The size of the vector is at least "maxBarriers")
	m_pendingNotificationArr.reserve(maxBarriers);
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

	// waiting size = 1 becuase currently it waits only on the main barrier.
	m_waitingSize = 1;

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

	// Register the new thread
	NotificationPort::registerNotificationPortThread(m_workerThread);

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

	// Set the new pending notification
	EventNotificationPackagePair tNotification;
	tNotification.first = barrier;
	tNotification.second.callBack = callBack;
	tNotification.second.arg = arg;
	tNotification.second.age = m_lastCallBackAge;

	// Push the new notification to the pending list
	m_pendingNotificationArr.push_back(tNotification);

	m_lastCallBackAge ++;

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

	// wait for the termination of worker thread (If the calling thread is the worker thread)
	size_t currPoc = m_poc;
	while ((m_workerState != FINISHED) && (currPoc == m_poc) && (0 == pthread_equal(m_workerThread, pthread_self())))
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
				size_t pendingNotificationsAmount = thisWorker->m_pendingNotificationArr.size();
				// If there is no enougth space, should resize the buffers "m_barriers" and "m_notificationsPackages".
				if ((thisWorker->m_waitingSize + pendingNotificationsAmount) >= thisWorker->m_maxBarriers)
				{
					thisWorker->resizeBuffers(&fireCallBacksArr, &firedIndicesArr, pendingNotificationsAmount);
				}
				assert((thisWorker->m_waitingSize + thisWorker->m_pendingNotificationArr.size()) < thisWorker->m_maxBarriers);
				for (unsigned int i = 0; i < pendingNotificationsAmount; i++)
				{
					// Add the pending notification to the real waiting list.
					thisWorker->m_barriers[thisWorker->m_waitingSize] = ((thisWorker->m_pendingNotificationArr)[i]).first;
					thisWorker->m_notificationsPackages[thisWorker->m_waitingSize] = ((thisWorker->m_pendingNotificationArr)[i]).second;
					thisWorker->m_waitingSize ++;
				}
				thisWorker->m_pendingNotificationArr.clear();
				thisWorker->m_operationMask[ADD] = false;
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

	// Unregister this thread.
	NotificationPort::unregisterNotificationPortThread(thisWorker->m_workerThread);
	// delete the NotificationPort instance.
	delete(thisWorker);

	return NULL;
}

void NotificationPort::getFiredCallBacks(unsigned int numSignaled, unsigned int* signaledIndices, vector<notificationPackage>& callBacksRet, bool* workerThreadSignaled)
{
	unsigned int notSignaledIndex = m_waitingSize - 1;

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

			notSignaledIndex --;
		}

		m_waitingSize --;
	}
}


void NotificationPort::resizeBuffers(vector<notificationPackage>* fireCallBacksArr, unsigned int** firedIndicesArr, size_t minimumResize)
{
	m_maxBarriers =   + (((uint16_t)(minimumResize / CALL_BACKS_ARRAY_RESIZE_AMOUNT) + 1) * CALL_BACKS_ARRAY_RESIZE_AMOUNT);
	assert(m_maxBarriers <= INT16_MAX && "Resize failed overflow max barriers size");
	m_barriers = (COIEVENT*)realloc(m_barriers, m_maxBarriers * sizeof(COIEVENT));
	assert(m_barriers && "memory allocation failed for m_barriers");
	m_notificationsPackages = (notificationPackage*)realloc(m_notificationsPackages, m_maxBarriers * sizeof(notificationPackage));
	assert(m_notificationsPackages && "memory allocation failed for m_notificationsPackages");
	fireCallBacksArr->reserve(m_maxBarriers);
	*firedIndicesArr = (unsigned int*)realloc(*firedIndicesArr, sizeof(unsigned int) * m_maxBarriers);
	assert(*firedIndicesArr && "memory allocation failed for *firedIndicesArr");
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

	m_workerState = FINISHED;
	pthread_cond_broadcast(&m_clientCond);

	pthread_mutex_unlock(&m_mutex);
}
