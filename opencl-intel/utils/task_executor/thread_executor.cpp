// Copyright (c) 2006-2009 Intel Corporation
// All rights reserved.
// 
// WARRANTY DISCLAIMER
// 
// THESE MATERIALS ARE PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL INTEL OR ITS
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THESE
// MATERIALS, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// 
// Intel Corporation is the author of the Materials, and requests that all
// problem reports or change requests be submitted to it directly

/*
*
* File thread_executor.cpp
*		Implementnation of Task system on Threads
*
*/
#include "stdafx.h"
#include "thread_executor.h"

#include <process.h>
#include <assert.h>

using namespace std;
using namespace Intel::OpenCL::TaskExecutor;

//////////////////////////////////////////////////////////////////////////
// GLOBAL VARIABLES
static volatile int			g_iThreadPoolSize = 0;	// number of actual working threads in the system
static thread_pool	g_obThreadPool;			// working threads pool

// Counts the number of fragments that are waiting for execution
static volatile LONG			g_plFragmentsWaiting[MAX_WORKING_THREADS_COUNT] = {0};

// Counts the number of fragments are not executed (waiting or in execution)
static volatile LONG			g_plFragmentsNotFinished[MAX_WORKING_THREADS_COUNT] = {0};

// events that will be use to notify on task sets
static HANDLE		g_pevTaskSetEvents[MAX_WORKING_THREADS_COUNT] = {NULL};

// When new fragments are ready for execution in thread i, the relevant bit will be set to 1
static volatile LONG			g_lNotifyMask = 0;

#ifdef _DEBUG
// For MT debug, the value is 1 when specific queue initializes a TaskSet
static volatile LONG			g_lTaskSetInit[MAX_WORKING_THREADS_COUNT] = {0};
static volatile LONG			g_lTaskSetExecute[MAX_WORKING_THREADS_COUNT] = {0};
#endif

//////////////////////////////////////////////////////////////////////////
// Worker Thread implementation
WorkerThread::WorkerThread(int iQueueId)
{
	// Initialize members. Then create the thread.
	m_dwThreadID	 = 0;
	m_hThread		 = 0;
	m_iNumTasks		 = 0;
	m_hEventComplete = 0;
	m_iQueueId       = iQueueId;
	m_pCurrentTaskSet = NULL;

	InitializeCriticalSectionAndSpinCount(&m_QueueLock, 4000);

	m_bIsBusy = false;
	m_evQueueEvent = CreateEvent(0, TRUE, FALSE, 0);

	for (int i=0; i<g_iThreadPoolSize; ++i)
	{
		if (i == iQueueId)
		{
			m_pNotifyEvents[i] = m_evQueueEvent;
		}
		else
		{
			m_pNotifyEvents[i] = g_pevTaskSetEvents[i];
		}
	}

	m_hEvent = CreateEvent( 0, FALSE, FALSE, 0 );

	// NOTE:	CreateThread returns NULL in case of failure.
	//			In production code, you MUST handle failures, and pass
	//			it on according to whatever error handling mechanisms you follow
	m_hThread = (HANDLE)_beginthreadex(0,
		0,
		ThreadFunc,
		this,
		0,
		&m_dwThreadID);

	// Wait for the thread to initialize and get ready ...
	//WaitForSingleObject( m_hEvent, INFINITE );
}

WorkerThread::~WorkerThread()
{
	// The destructor signals the thread to complete 
	// and then waits for it to complete
	if ( m_hThread ) 
	{
		EnqueueTask(NULL);
		//m_pTaskQueue->Enqueue(NULL);

		// Wait on the thread, until it completes
		::WaitForSingleObject( m_hThread, INFINITE );
		m_hThread = 0;

		::CloseHandle( m_hEvent );
		m_hEvent = 0;

		::CloseHandle(m_evQueueEvent);
	}

	// Free critical sections
	DeleteCriticalSection(&m_QueueLock);
}

void WorkerThread::EnqueueTask(ITaskWrapper * pTaskWrapper)
{
	EnterCriticalSection( &m_QueueLock );
	m_qTasks.push(pTaskWrapper); 

	if (!m_bIsBusy)
	{
		//printf("Set Event\n");
		SetEvent(m_evQueueEvent);
	}

	//printf("NOT Set Event\n");
	LeaveCriticalSection( &m_QueueLock );

	::InterlockedIncrement( &( m_iNumTasks ) );

}

bool WorkerThread::ExecutionRoutine()
{
	ITaskWrapper *pTaskBase = NULL;
	long lNumTasks = 0;
	lNumTasks = ::InterlockedCompareExchange(&m_iNumTasks, 0, 0);
	while (lNumTasks)
	{
		// get the next task for execution
		::EnterCriticalSection( &m_QueueLock );
		if (m_qTasks.size())
		{
			pTaskBase = m_qTasks.front();
			m_qTasks.pop(); 
		}
		::LeaveCriticalSection( &m_QueueLock );

		if (NULL == pTaskBase)
		{
			return true;
		}

		// execute task
		pTaskBase->Execute((void**)&m_pCurrentTaskSet);

		// release task
		pTaskBase->Release();

		lNumTasks = ::InterlockedDecrement(&m_iNumTasks);
	}
	return false;
}
unsigned int WorkerThread::ThreadFunc( LPVOID lpvThreadParam )
{
	DWORD dwRetVal = 0;

	// Typecast the thread param to a CWorkerThread*
	WorkerThread* pWorkerThread = static_cast<WorkerThread*>( lpvThreadParam );

	if ( !pWorkerThread ) 
	{
		// Invalid worker thread pointer, so quit
		dwRetVal ++;
	}
	else 
	{
		// Signal the pool manager that we are ready ...
		SetEvent( pWorkerThread->m_hEvent );

		ITaskWrapper * pTaskBase = NULL;
		CTaskSetFragment * pFragment = NULL;

		DWORD dwNotifyIndex = 0;
		bool bExitLoop = false;
		bool bIsEmpty = true;
		while (TRUE)
		{
			dwNotifyIndex = pWorkerThread->m_iQueueId;
			// before waiting for new tasks, check if the thread's queue is empty or not. If the queue is not empty
			// skip the WaitForMultipleObjects and do the work
			::EnterCriticalSection( &pWorkerThread->m_QueueLock );
			bIsEmpty = pWorkerThread->m_qTasks.empty();
			::LeaveCriticalSection( &pWorkerThread->m_QueueLock );

			if (bIsEmpty)
			{
				dwNotifyIndex = WaitForMultipleObjects(g_iThreadPoolSize, pWorkerThread->m_pNotifyEvents, FALSE, INFINITE);
				assert(dwNotifyIndex != 0xFFFFFFFF);
			}
			
			::EnterCriticalSection( &pWorkerThread->m_QueueLock );
			pWorkerThread->m_bIsBusy = true;
			::LeaveCriticalSection( &pWorkerThread->m_QueueLock );

			// the caller was the tasks queue to modify that new tasks are available at my queue
			if (dwNotifyIndex == pWorkerThread->m_iQueueId)	
			{
				if (pWorkerThread->ExecutionRoutine()) // the return value indicates if we need to break out from the thread routine
				{
					break;
				}
			}
			else
			{
				// help the thread that wake me up
				pFragment = g_obThreadPool[dwNotifyIndex]->GetNextFragment();
				if (pFragment)
				{
					pFragment->AttachToThread(pWorkerThread->m_iQueueId);
					do 
					{
						pFragment->Execute(pWorkerThread->m_iQueueId);
					} 
					while (pFragment = g_obThreadPool[dwNotifyIndex]->GetNextFragment());
				}
			}

			// in any case, go through the other threads (including mine) and search for work to do. we prefer to look for other
			// work instead to returning to the WaitFor...

			// run as long as there are threads that look for help
			LONG lNotify = ::InterlockedCompareExchange(&g_lNotifyMask, 0, 0);
			while ( lNotify )
			{
				long lNotifyMask = 1;
				for (int i=0; i<g_iThreadPoolSize && lNotify; ++i)
				{
					if ( (lNotifyMask & lNotify) && (i != pWorkerThread->m_iQueueId) )
					{
						// we've found the first thread. now, let's start executing its tasks
						pFragment = g_obThreadPool[i]->GetNextFragment();
						if (pFragment)
						{
							pFragment->AttachToThread(pWorkerThread->m_iQueueId);
							do {
								pFragment->Execute(pWorkerThread->m_iQueueId);
								// Update notification flags
								lNotify = ::InterlockedCompareExchange(&g_lNotifyMask, 0, 0);
							} while( (lNotifyMask & lNotify) && (pFragment = g_obThreadPool[i]->GetNextFragment()) );
						}
					}
					lNotifyMask <<= 1;
					lNotify = ::InterlockedCompareExchange(&g_lNotifyMask, 0, 0);
				}
			}

			if (pWorkerThread->ExecutionRoutine())
			{
				break;
			}

			::EnterCriticalSection( &(pWorkerThread->m_QueueLock) );
			pWorkerThread->m_bIsBusy = false;
			ResetEvent(pWorkerThread->m_evQueueEvent);
			::LeaveCriticalSection( &(pWorkerThread->m_QueueLock) );
		}
	}
	return dwRetVal;
}

int WorkerThread::WaitForCompletion()
{
	m_hEventComplete = ::CreateEvent( 0, FALSE, FALSE, 0 );

	// Enqueue a synch tasks and wait for the event to rais up
	EnqueueTask(new CTaskSynch(this));

	::WaitForSingleObject( m_hEventComplete, INFINITE );

	::CloseHandle( m_hEventComplete );
	m_hEventComplete = NULL;

	return 0;
}

CTaskSetFragment * WorkerThread::GetNextFragment()
{
	long lWaitingCount = InterlockedDecrement(&g_plFragmentsWaiting[m_iQueueId]);
	if ( lWaitingCount < 0 )
	{
		return NULL;
	}
#ifdef _DEBUG
	long lInit = InterlockedCompareExchange( &g_lTaskSetInit[m_iQueueId], 0, 0);
	assert(lInit==0);

	long lExecute = InterlockedCompareExchange( &g_lTaskSetExecute[m_iQueueId], 1, 1);
	assert(lExecute==1);

	long lCount = ::InterlockedCompareExchange(&g_plFragmentsNotFinished[m_iQueueId], 0, 0);
	assert(lCount>0);
#endif
	CTaskSet* pTaskSet = (CTaskSet*)InterlockedCompareExchangePointer((void**)&m_pCurrentTaskSet, NULL, NULL);
	if ( NULL != pTaskSet )
	{
		return pTaskSet->GetFragment(lWaitingCount);
	}

	printf("--->>>> !!!! Error: m_pCurrentTaskSet=NULL, lWaitingCount=%d\n", lWaitingCount);
	return  NULL;
}

//////////////////////////////////////////////////////////////////////////
// CTaskSet implementation
struct sDim 
{
	unsigned int dim[3];
};
CTaskSet::CTaskSet(ITaskSet * pTaskSet, int iQueueId) :
m_pTaskSet(pTaskSet), m_iQueueId(iQueueId)
{
}

void CTaskSet::Execute(void** pCurrentSet)
{ 
#ifdef _DEBUG
	// Test for initialization process
	long lInit = InterlockedCompareExchange(&g_lTaskSetInit[m_iQueueId], 1, 0);
	assert(lInit==0);
#endif

	// Prepare execution parameters
	sDim start = {0, 0, 0};
	sDim range;

	int res = m_pTaskSet->Init((unsigned int *)&range);
	if ( res )
	{
		m_pTaskSet->Finish(FINISH_INIT_FAILED);
		assert(0);
		return;
	}
	unsigned __int64 n = 1;
	for(int i =0; i<3; ++i)
	{
		n *= range.dim[i];
	}
	if ( n > ULONG_MAX)
	{
		// Too many instances to execute
		m_pTaskSet->Finish(FINISH_EXECUTION_FAILED);
		return;
	}
	unsigned int uiGranSize = (( ((unsigned int)n)+g_iThreadPoolSize*16-1)/(g_iThreadPoolSize*16));

	list<pair<sDim,sDim>> ranges;
	ranges.push_back(make_pair(start, range));
	while ( !ranges.empty() )
	{
		pair<sDim,sDim> range = ranges.front();
		ranges.pop_front();

		unsigned int pages_size = range.second.dim[2]-range.first.dim[2];
		unsigned int rows_size = range.second.dim[1]-range.first.dim[1];
		unsigned int cols_size = range.second.dim[0]-range.first.dim[0];
		if ( pages_size*rows_size*cols_size <= uiGranSize)
		{
			m_vectFragments.push_back(new CTaskSetFragment(m_iQueueId,
				(unsigned int*)(&range.first),
				(unsigned int*)(&range.second),
				m_pTaskSet));
			continue;
		}

		pair<sDim,sDim> newRange = range;
		if( pages_size < rows_size ) {
			if ( rows_size < cols_size ) {
				unsigned int middle = range.first.dim[0] + (range.second.dim[0]-range.first.dim[0])/2u;
				range.second.dim[0] = middle;
				newRange.first.dim[0] = middle;
			} else {
				unsigned int middle = range.first.dim[1] + (range.second.dim[1]-range.first.dim[1])/2u;
				range.second.dim[1] = middle;
				newRange.first.dim[1] = middle;
			}
		} else {
			if ( pages_size < cols_size ) {
				unsigned int middle = range.first.dim[0] + (range.second.dim[0]-range.first.dim[0])/2u;
				range.second.dim[0] = middle;
				newRange.first.dim[0] = middle;
			} else {
				unsigned int middle = range.first.dim[2] + (range.second.dim[2]-range.first.dim[2])/2u;
				range.second.dim[2] = middle;
				newRange.first.dim[2] = middle;
			}
		}
		ranges.push_back(range);
		ranges.push_back(newRange);
	}

#ifdef _DEBUG
	lInit = InterlockedCompareExchange(&g_lTaskSetInit[m_iQueueId], 0, 1);
	assert(lInit==1);
#endif

	// Set the number of waiting fragments
	long lNumFragemtns = m_vectFragments.size();
	g_plFragmentsNotFinished[m_iQueueId] = lNumFragemtns;
	
	// Set current task set
	InterlockedExchangePointer(pCurrentSet, this);

	InterlockedExchange( &g_plFragmentsWaiting[m_iQueueId], lNumFragemtns );

#ifdef _DEBUG
	InterlockedExchange(&g_lTaskSetExecute[m_iQueueId], 1);
#endif
	// notify other threads to join the effort...
	// we need to make sure the global notification flag is set and the global sub-tasks
	// indicator is initialized to the current number of sub-tasks
	::InterlockedBitTestAndSet(&g_lNotifyMask, m_iQueueId);
	SetEvent(g_pevTaskSetEvents[m_iQueueId]);

	// start executing fragmented tasks
	CTaskSetFragment * pFragment = GetNext();
	if (pFragment)
	{
		m_pTaskSet->AttachToThread(m_iQueueId);
	}
	while(pFragment) 
	{
		pFragment->Execute(m_iQueueId);
		pFragment = GetNext();
	}

	// the job has been finished, we need to modify the global notification flag
	ResetEvent(g_pevTaskSetEvents[m_iQueueId]);
	::InterlockedBitTestAndReset(&g_lNotifyMask, m_iQueueId);

	//the queue is empty, wait until all fragments were completed
	while ( ::InterlockedCompareExchange(&g_plFragmentsNotFinished[m_iQueueId], 0, 0) );
	{
		// Just do busy loop, don't go to kernel
		//Sleep(0);
	}
	
#ifdef _DEBUG
	InterlockedExchange(&g_lTaskSetExecute[m_iQueueId], 0);
#endif

	InterlockedExchangePointer(pCurrentSet, NULL);

	// The job has finished!
	m_pTaskSet->Finish(FINISH_COMPLETED);
}

CTaskSetFragment * CTaskSet::GetNext()
{
	long lWaitingCount = InterlockedDecrement(&g_plFragmentsWaiting[m_iQueueId]);
	if ( lWaitingCount < 0 )
	{
		return NULL;
	}

#ifdef _DEBUG
	long lInit = InterlockedCompareExchange(&g_lTaskSetInit[m_iQueueId], 0, 0);
	assert(lInit==0);

	long lExecute = InterlockedCompareExchange( &g_lTaskSetExecute[m_iQueueId], 1, 1);
	assert(lExecute==1);
#endif

	return m_vectFragments[lWaitingCount];
}

//////////////////////////////////////////////////////////////////////////
// CTaskSet implementation
void CTaskSetFragment::Execute(unsigned int uiWorkerId)
{
#ifdef _DEBUG
	long lInit = InterlockedCompareExchange(&g_lTaskSetInit[m_iQueueId], 0, 0);
	assert(lInit==0);

	long lExecute = InterlockedCompareExchange( &g_lTaskSetExecute[m_iQueueId], 1, 1);
	assert(lExecute==1);
#endif

	// execute fragment iteration
	for(int i=m_iStartZ; i<m_iEndZ; ++i)
	{
		for(int j=m_iStartY; j<m_iEndY; ++j)
		{
			for(int k=m_iStartX; k<m_iEndX; ++k)
			{
				m_pTaskSet->ExecuteIteration(k, j, i, uiWorkerId);
			}
		}
	}

	// modify the relevant queue's fragments counter
	::InterlockedDecrement(&g_plFragmentsNotFinished[m_iQueueId]);
	delete this;
}

int	CTaskSetFragment::AttachToThread(unsigned int uiWorkerId)
{
#ifdef _DEBUG
	long lInit = InterlockedCompareExchange(&g_lTaskSetInit[m_iQueueId], 0, 0);
	assert(lInit==0);

	long lExecute = InterlockedCompareExchange( &g_lTaskSetExecute[m_iQueueId], 1, 1);
	assert(lExecute==1);
#endif

	return m_pTaskSet->AttachToThread(uiWorkerId);
}

//////////////////////////////////////////////////////////////////////////
// ThreadTaskListOrderedImpl implementation
unsigned int ThreadTaskListOrderedImpl::Enqueue(ITaskBase* pTaskBase)
{
	if (NULL == m_pSelectedWorkerThread)
	{
		// find the list 
		m_pSelectedWorkerThread = g_obThreadPool.front();
		long lSelectedTaskNum = ::InterlockedCompareExchange(&(m_pSelectedWorkerThread->m_iNumTasks), 0, 0);

		// Locate a thread with the lowest number of jobs ...
		// NOTE 1:	This approach assumes all jobs are equal, 
		//			which is an obviously wrong assumption. 
		//			So we will need to think of weighting jobs - based on complexity,
		//			latency etc etc.
		// NOTE 2:	Each thread is hard at work processing jobs. 
		//			So by the time we reach the end of the list,
		//			chances are we have the most loaded thread, 
		//			instead of the least loaded thread.
		//			So you will also need to figure out a way for each 
		//			thread to return an absolute ETA, and then 
		//			use the min of those to assign the current job to.
		for ( int i=0; i < g_iThreadPoolSize; i++ ) 
		{
			long lNumTasks = ::InterlockedCompareExchange(&(g_obThreadPool[i]->m_iNumTasks), 0, 0);
			if ( 0 == lNumTasks )
			{
				m_pSelectedWorkerThread = g_obThreadPool[i];
				break;
			}
			if ( lSelectedTaskNum > lNumTasks ) 
			{
				m_pSelectedWorkerThread = g_obThreadPool[i];
				lSelectedTaskNum = lNumTasks;
			}
		}
	}
	if (pTaskBase->IsTaskSet())
	{
		m_pSelectedWorkerThread->EnqueueTask(new CTaskSet((ITaskSet*)pTaskBase, m_pSelectedWorkerThread->m_iQueueId));
	}
	else
	{
		m_pSelectedWorkerThread->EnqueueTask(new CTask((ITask*)pTaskBase));
	}
	return 0;
}

unsigned int ThreadTaskListUnOrderedImpl::Enqueue(ITaskBase* pTaskBase)
{
	WorkerThread * pWorkerThread = g_obThreadPool[0];
	int i=1;
	long lWorkerTaskNum = ::InterlockedCompareExchange(&(pWorkerThread->m_iNumTasks), 0, 0);
	while ( (i<g_iThreadPoolSize) && (0 != lWorkerTaskNum) ) 
	{
		long lTaskNum = ::InterlockedCompareExchange(&(g_obThreadPool[i]->m_iNumTasks), 0, 0);
		if ( lWorkerTaskNum > lTaskNum ) 
		{
			pWorkerThread = g_obThreadPool[i];
			lWorkerTaskNum = lTaskNum;
		}
		++i;
	}

	if (pTaskBase->IsTaskSet())
	{
		pWorkerThread->EnqueueTask(new CTaskSet((ITaskSet*)pTaskBase, pWorkerThread->m_iQueueId));
	}
	else
	{
		pWorkerThread->EnqueueTask(new CTask((ITask*)pTaskBase));
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
// ThreadTaskExecutor implementation
int	ThreadTaskExecutor::Init(unsigned int uiNumThreads)
{
	unsigned long ulNewVal = InterlockedIncrement(&m_lRefCount);
	if ( ulNewVal > 1 )
	{
		while ( 0 == g_iThreadPoolSize );	// spin loop until initialization completed
		return g_iThreadPoolSize;
	}

	if ( 0 == uiNumThreads)
	{
		SYSTEM_INFO siSysInfo;
		GetSystemInfo(&siSysInfo); 
		uiNumThreads = siSysInfo.dwNumberOfProcessors;
	}

	m_uiNumWorkingThreads = g_iThreadPoolSize = uiNumThreads;
	for ( unsigned int i = 0; i < uiNumThreads; i ++ ) 
	{
		g_pevTaskSetEvents[i] = CreateEvent(0, TRUE, FALSE, 0);
	}

	for ( unsigned int i = 0; i < uiNumThreads; i ++ ) 
	{
		g_obThreadPool.push_back( new WorkerThread(i) );
	}
	m_bIsPoolValid = true;

	return uiNumThreads;
}

void ThreadTaskExecutor::Close(bool bCancel)
{
	unsigned long ulNewVal = InterlockedDecrement(&m_lRefCount);
	if ( ulNewVal > 0)
	{
		return;
	}

	ClearThreadPool();
}

ITaskList* ThreadTaskExecutor::CreateTaskList(bool OOO)
{
	if ( OOO )
	{
		return new ThreadTaskListUnOrderedImpl();
	}
	return  new ThreadTaskListOrderedImpl();
}
unsigned int ThreadTaskExecutor::Execute(ITaskBase * pTask)
{
	int ret = m_MyList.Enqueue(pTask);
	m_MyList.Flush();
	return ret;
}

bool ThreadTaskExecutor::ClearThreadPool()
{
	// Signal that the pool will not accept any more jobs
	m_bIsPoolValid = false;

	for ( int i = 0; i < g_iThreadPoolSize; i ++ ) 
	{
		::CloseHandle(g_pevTaskSetEvents[i]);
	}

	// Start deleting all CWorkerThreads
	// NOTE:	The CWorkerThread destructor posts a WM_QUIT message to the embedded thread,
	//			and then waits on the thread to complete. So each destructor will return only
	//			after the embedded thread has processed all jobs in it's queue
	for ( int i = 0; i < g_iThreadPoolSize; i ++ ) 
	{
		delete g_obThreadPool[i];
		g_obThreadPool[i] = NULL;
	}
	g_obThreadPool.clear();

	return true;
}
