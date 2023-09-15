// INTEL CONFIDENTIAL
//
// Copyright 2006 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

#ifdef __THREAD_EXECUTOR__

#include "thread_executor.h"

#include "cl_shared_ptr.hpp"

#include <cassert>
#include <process.h>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

using namespace std;
using namespace Intel::OpenCL::TaskExecutor;

//////////////////////////////////////////////////////////////////////////
// GLOBAL VARIABLES
static volatile int g_iThreadPoolSize =
    0; // number of actual working threads in the system
static thread_pool g_obThreadPool; // working threads pool

// Counts the number of fragments that are waiting for execution
static volatile LONG g_plFragmentsWaiting[MAX_WORKING_THREADS_COUNT] = {0};

// Counts the number of fragments are not executed (waiting or in execution)
static volatile LONG g_plFragmentsNotFinished[MAX_WORKING_THREADS_COUNT] = {0};

// events that will be use to notify on task sets
static HANDLE g_pevTaskSetEvents[MAX_WORKING_THREADS_COUNT] = {nullptr};

// When new fragments are ready for execution in thread i, the relevant bit will
// be set to 1
static volatile LONG g_lNotifyMask = 0;

#ifdef _DEBUG
// For MT debug, the value is 1 when specific queue initializes a TaskSet
static volatile LONG g_lTaskSetInit[MAX_WORKING_THREADS_COUNT] = {0};
static volatile LONG g_lTaskSetExecute[MAX_WORKING_THREADS_COUNT] = {0};
#endif

//////////////////////////////////////////////////////////////////////////
// Worker Thread implementation
WorkerThread::WorkerThread(int iQueueId, bool bUseTaskalyzer) {
  // Initialize members. Then create the thread.
  m_dwThreadID = 0;
  m_hThread = 0;
  m_iNumTasks = 0;
  m_hEventComplete = 0;
  m_iQueueId = iQueueId;
  m_pCurrentTaskSet = nullptr;
  m_bUseTaskalyzer = bUseTaskalyzer;

  InitializeCriticalSectionAndSpinCount(&m_QueueLock, 4000);

  m_bIsBusy = false;
  m_evQueueEvent = CreateEvent(0, TRUE, FALSE, 0);

  for (int i = 0; i < g_iThreadPoolSize; ++i) {
    if (i == iQueueId) {
      m_pNotifyEvents[i] = m_evQueueEvent;
    } else {
      m_pNotifyEvents[i] = g_pevTaskSetEvents[i];
    }
  }

  m_hEvent = CreateEvent(0, FALSE, FALSE, 0);

  // NOTE:  CreateThread returns NULL in case of failure.
  //      In production code, you MUST handle failures, and pass
  //      it on according to whatever error handling mechanisms
  // you follow
  m_hThread = (HANDLE)_beginthreadex(0, 0, ThreadFunc, this, 0, &m_dwThreadID);

  // Wait for the thread to initialize and get ready ...
  // WaitForSingleObject( m_hEvent, INFINITE );
}

WorkerThread::~WorkerThread() {
  // The destructor signals the thread to complete
  // and then waits for it to complete
  if (m_hThread) {
    EnqueueTask(nullptr);
    // m_pTaskQueue->Enqueue(NULL);

    // Wait on the thread, until it completes
    ::WaitForSingleObject(m_hThread, INFINITE);
    m_hThread = 0;

    ::CloseHandle(m_hEvent);
    m_hEvent = 0;

    ::CloseHandle(m_evQueueEvent);
  }

  // Free critical sections
  DeleteCriticalSection(&m_QueueLock);
}

void WorkerThread::EnqueueTask(ITaskWrapper *pTaskWrapper) {
  EnterCriticalSection(&m_QueueLock);
  m_qTasks.push(pTaskWrapper);

  if (!m_bIsBusy) {
    // printf("Set Event\n");
    SetEvent(m_evQueueEvent);
  }

  // printf("NOT Set Event\n");
  LeaveCriticalSection(&m_QueueLock);

  ::InterlockedIncrement(&(m_iNumTasks));
}

bool WorkerThread::ExecutionRoutine() {
  ITaskWrapper *pTaskBase = nullptr;
  long lNumTasks = 0;
  lNumTasks = ::InterlockedCompareExchange(&m_iNumTasks, 0, 0);
  while (lNumTasks) {
    // get the next task for execution
    ::EnterCriticalSection(&m_QueueLock);
    if (m_qTasks.size()) {
      pTaskBase = m_qTasks.front();
      m_qTasks.pop();
    }
    ::LeaveCriticalSection(&m_QueueLock);

    if (nullptr == pTaskBase) {
      return true;
    }

    // execute task
    pTaskBase->Execute((void **)&m_pCurrentTaskSet);

    // release task
    pTaskBase->Release();

    lNumTasks = ::InterlockedDecrement(&m_iNumTasks);
  }
  return false;
}
unsigned int WorkerThread::ThreadFunc(LPVOID lpvThreadParam) {
  DWORD dwRetVal = 0;

  // Typecast the thread param to a CWorkerThread*
  WorkerThread *pWorkerThread = static_cast<WorkerThread *>(lpvThreadParam);

  if (!pWorkerThread) {
    // Invalid worker thread pointer, so quit
    dwRetVal++;
  } else {
    // Signal the pool manager that we are ready ...
    SetEvent(pWorkerThread->m_hEvent);

    ITaskWrapper *pTaskBase = nullptr;
    CTaskSetFragment *pFragment = nullptr;

    DWORD dwNotifyIndex = 0;
    bool bExitLoop = false;
    bool bIsEmpty = true;
    while (TRUE) {
      dwNotifyIndex = pWorkerThread->m_iQueueId;
      // before waiting for new tasks, check if the thread's queue is empty or
      // not. If the queue is not empty skip the WaitForMultipleObjects and do
      // the work
      ::EnterCriticalSection(&pWorkerThread->m_QueueLock);
      bIsEmpty = pWorkerThread->m_qTasks.empty();
      ::LeaveCriticalSection(&pWorkerThread->m_QueueLock);

      if (bIsEmpty) {
        dwNotifyIndex = WaitForMultipleObjects(
            g_iThreadPoolSize, pWorkerThread->m_pNotifyEvents, FALSE, INFINITE);
        assert(dwNotifyIndex != 0xFFFFFFFF);
      }

      ::EnterCriticalSection(&pWorkerThread->m_QueueLock);
      pWorkerThread->m_bIsBusy = true;
      ::LeaveCriticalSection(&pWorkerThread->m_QueueLock);

      // the caller was the tasks queue to modify that new tasks are available
      // at my queue
      if (dwNotifyIndex == pWorkerThread->m_iQueueId) {
        if (pWorkerThread
                ->ExecutionRoutine()) // the return value indicates if we need
                                      // to break out from the thread routine
        {
          break;
        }
      } else {
        // help the thread that wake me up
        pFragment = g_obThreadPool[dwNotifyIndex]->GetNextFragment();
        if (pFragment) {
          pFragment->AttachToThread(pWorkerThread->m_iQueueId);
          do {
            // Detach from thread executed inside execution when no additional
            // Fragment exists
            pFragment = pFragment->ExecuteAndGetNext(pWorkerThread->m_iQueueId);
          } while (pFragment != nullptr);
        }
      }

      // in any case, go through the other threads (including mine) and search
      // for work to do. we prefer to look for other work instead to returning
      // to the WaitFor...

      // run as long as there are threads that look for help
      LONG lNotify = ::InterlockedCompareExchange(&g_lNotifyMask, 0, 0);
      while (lNotify) {
        long lNotifyMask = 1;
        for (int i = 0; i < g_iThreadPoolSize && lNotify; ++i) {
          if ((lNotifyMask & lNotify) && (i != pWorkerThread->m_iQueueId)) {
            // we've found the first thread. now, let's start executing its
            // tasks
            pFragment = g_obThreadPool[i]->GetNextFragment();
            if (pFragment) {
              pFragment->AttachToThread(pWorkerThread->m_iQueueId);
              do {
                // Detach from thread executed inside execution when no
                // additional Fragment exists
                pFragment =
                    pFragment->ExecuteAndGetNext(pWorkerThread->m_iQueueId);
              } while (pFragment != nullptr);
            }
          }
          lNotifyMask <<= 1;
          lNotify = ::InterlockedCompareExchange(&g_lNotifyMask, 0, 0);
        }
      }

      if (pWorkerThread->ExecutionRoutine()) {
        break;
      }

      ::EnterCriticalSection(&(pWorkerThread->m_QueueLock));
      pWorkerThread->m_bIsBusy = false;
      ResetEvent(pWorkerThread->m_evQueueEvent);
      ::LeaveCriticalSection(&(pWorkerThread->m_QueueLock));
    }
  }
  return dwRetVal;
}

int WorkerThread::WaitForCompletion() {
  m_hEventComplete = ::CreateEvent(0, FALSE, FALSE, 0);

  // Enqueue a synch tasks and wait for the event to rais up
  EnqueueTask(new CTaskSynch(this));

  ::WaitForSingleObject(m_hEventComplete, INFINITE);

  ::CloseHandle(m_hEventComplete);
  m_hEventComplete = nullptr;

  return 0;
}

CTaskSetFragment *WorkerThread::GetNextFragment() {
  long lWaitingCount = InterlockedDecrement(&g_plFragmentsWaiting[m_iQueueId]);
  if (lWaitingCount < 0) {
    return nullptr;
  }
#ifdef _DEBUG
  long lInit = InterlockedCompareExchange(&g_lTaskSetInit[m_iQueueId], 0, 0);
  assert(lInit == 0);

  long lExecute =
      InterlockedCompareExchange(&g_lTaskSetExecute[m_iQueueId], 1, 1);
  assert(lExecute == 1);

  long lCount =
      ::InterlockedCompareExchange(&g_plFragmentsNotFinished[m_iQueueId], 0, 0);
  assert(lCount > 0);
#endif
  CTaskSet *pTaskSet = (CTaskSet *)InterlockedCompareExchangePointer(
      (void **)&m_pCurrentTaskSet, nullptr, nullptr);
  if (nullptr != pTaskSet) {
    return pTaskSet->GetFragment(lWaitingCount);
  }

  printf("--->>>> !!!! Error: m_pCurrentTaskSet=NULL, lWaitingCount=%d\n",
         lWaitingCount);
  return nullptr;
}

//////////////////////////////////////////////////////////////////////////
// CTaskSet implementation
struct sDim {
  size_t dim[3];
};
CTaskSet::CTaskSet(ITaskSet *pTaskSet, int iQueueId)
    : m_pTaskSet(pTaskSet), m_iQueueId(iQueueId) {}

void CTaskSet::Execute(void **pCurrentSet) {
#ifdef _DEBUG
  // Test for initialization process
  long lInit = InterlockedCompareExchange(&g_lTaskSetInit[m_iQueueId], 1, 0);
  assert(lInit == 0);
#endif

  // Prepare execution parameters
  sDim start = {0, 0, 0};
  sDim range;
  unsigned int dimCount;
  int res = m_pTaskSet->Init((size_t *)&range, dimCount);
  if (res) {
#ifdef _DEBUG
    lInit = InterlockedCompareExchange(&g_lTaskSetInit[m_iQueueId], 0, 1);
    assert(lInit == 1);
#endif
    m_pTaskSet->Finish(FINISH_INIT_FAILED);
    assert(0);
    return;
  }
  unsigned __int64 n = 1;
  for (unsigned int i = 0; i < dimCount; ++i) {
    n *= range.dim[i];
  }
  for (unsigned int i = dimCount; i < 3; ++i) {
    range.dim[i] = 1;
  }
  if (n > ULONG_MAX) {
#ifdef _DEBUG
    lInit = InterlockedCompareExchange(&g_lTaskSetInit[m_iQueueId], 0, 1);
    assert(lInit == 1);
#endif
    // Too many instances to execute
    m_pTaskSet->Finish(FINISH_EXECUTION_FAILED);
    assert(0);
    return;
  }
  unsigned int uiGranSize = ((((unsigned int)n) + g_iThreadPoolSize * 16 - 1) /
                             (g_iThreadPoolSize * 16));

  list<pair<sDim, sDim>> ranges;
  ranges.push_back(make_pair(start, range));
  while (!ranges.empty()) {
    pair<sDim, sDim> range = ranges.front();
    ranges.pop_front();

    size_t pages_size = range.second.dim[2] - range.first.dim[2];
    size_t rows_size = range.second.dim[1] - range.first.dim[1];
    size_t cols_size = range.second.dim[0] - range.first.dim[0];
    if (pages_size * rows_size * cols_size <= uiGranSize) {
      assert(range.first.dim[0] <= MAXUINT32);
      assert(range.first.dim[1] <= MAXUINT32);
      assert(range.first.dim[2] <= MAXUINT32);
      assert(range.second.dim[0] <= MAXUINT32);
      assert(range.second.dim[1] <= MAXUINT32);
      assert(range.second.dim[2] <= MAXUINT32);
      m_vectFragments.push_back(new CTaskSetFragment(
          m_iQueueId, (unsigned int)range.first.dim[0],
          (unsigned int)range.first.dim[1], (unsigned int)range.first.dim[2],
          (unsigned int)range.second.dim[0], (unsigned int)range.second.dim[1],
          (unsigned int)range.second.dim[2], m_pTaskSet));
      continue;
    }

    pair<sDim, sDim> newRange = range;
    if (pages_size < rows_size) {
      if (rows_size < cols_size) {
        size_t middle = range.first.dim[0] +
                        (range.second.dim[0] - range.first.dim[0]) / 2u;
        range.second.dim[0] = middle;
        newRange.first.dim[0] = middle;
      } else {
        size_t middle = range.first.dim[1] +
                        (range.second.dim[1] - range.first.dim[1]) / 2u;
        range.second.dim[1] = middle;
        newRange.first.dim[1] = middle;
      }
    } else {
      if (pages_size < cols_size) {
        size_t middle = range.first.dim[0] +
                        (range.second.dim[0] - range.first.dim[0]) / 2u;
        range.second.dim[0] = middle;
        newRange.first.dim[0] = middle;
      } else {
        size_t middle = range.first.dim[2] +
                        (range.second.dim[2] - range.first.dim[2]) / 2u;
        range.second.dim[2] = middle;
        newRange.first.dim[2] = middle;
      }
    }
    ranges.push_back(range);
    ranges.push_back(newRange);
  }

#ifdef _DEBUG
  lInit = InterlockedCompareExchange(&g_lTaskSetInit[m_iQueueId], 0, 1);
  assert(lInit == 1);
#endif

  // Set the number of waiting fragments
  assert(m_vectFragments.size() <= MAXINT32);
  long lNumFragemtns = (long)m_vectFragments.size();
  g_plFragmentsNotFinished[m_iQueueId] = lNumFragemtns;

  // Set current task set
  InterlockedExchangePointer(pCurrentSet, this);

  InterlockedExchange(&g_plFragmentsWaiting[m_iQueueId], lNumFragemtns);

#ifdef _DEBUG
  InterlockedExchange(&g_lTaskSetExecute[m_iQueueId], 1);
#endif
  // notify other threads to join the effort...
  // we need to make sure the global notification flag is set and the global
  // sub-tasks indicator is initialized to the current number of sub-tasks
  ::InterlockedBitTestAndSet(&g_lNotifyMask, m_iQueueId);
  SetEvent(g_pevTaskSetEvents[m_iQueueId]);

  // start executing fragmented tasks
  CTaskSetFragment *pFragment = GetNext();
  bool bExecuted = false;
  if (pFragment) {
    size_t firstWGID[3];
    size_t lastWGID[3];
    pFragment->getFirstWGID(firstWGID);
    pFragment->getLastWGID(lastWGID);
    // TODO: pass something meaningful to AttachTo/DetachFromThread
    m_pTaskSet->AttachToThread(nullptr, pFragment->Size(), firstWGID, lastWGID);
    bExecuted = true;
  }
  while (pFragment) {
    pFragment->Execute(m_iQueueId);
    pFragment = GetNext();
  }

  if (bExecuted) {
    m_pTaskSet->DetachFromThread(nullptr);
  }

  // the job has been finished, we need to modify the global notification flag
  ResetEvent(g_pevTaskSetEvents[m_iQueueId]);
  ::InterlockedBitTestAndReset(&g_lNotifyMask, m_iQueueId);

  // the queue is empty, wait until all fragments were completed
  while (
      ::InterlockedCompareExchange(&g_plFragmentsNotFinished[m_iQueueId], 0, 0))
    ;
  {
    // Just do busy loop, don't go to kernel
    // Sleep(0);
  }

#ifdef _DEBUG
  InterlockedExchange(&g_lTaskSetExecute[m_iQueueId], 0);
#endif

  InterlockedExchangePointer(pCurrentSet, nullptr);

  // The job has finished!
  m_pTaskSet->Finish(FINISH_COMPLETED);
}

CTaskSetFragment *CTaskSet::GetNext() {
  long lWaitingCount = InterlockedDecrement(&g_plFragmentsWaiting[m_iQueueId]);
  if (lWaitingCount < 0) {
    return nullptr;
  }

#ifdef _DEBUG
  long lInit = InterlockedCompareExchange(&g_lTaskSetInit[m_iQueueId], 0, 0);
  assert(lInit == 0);

  long lExecute =
      InterlockedCompareExchange(&g_lTaskSetExecute[m_iQueueId], 1, 1);
  assert(lExecute == 1);
#endif

  return m_vectFragments[lWaitingCount];
}

//////////////////////////////////////////////////////////////////////////
// CTaskSet implementation
CTaskSetFragment *CTaskSetFragment::ExecuteAndGetNext(unsigned int uiWorkerId) {
#ifdef _DEBUG
  long lInit = InterlockedCompareExchange(&g_lTaskSetInit[m_iQueueId], 0, 0);
  assert(lInit == 0);

  long lExecute =
      InterlockedCompareExchange(&g_lTaskSetExecute[m_iQueueId], 1, 1);
  assert(lExecute == 1);
#endif

  // execute fragment iteration
  for (int i = m_iStartZ; i < m_iEndZ; ++i) {
    for (int j = m_iStartY; j < m_iEndY; ++j) {
      for (int k = m_iStartX; k < m_iEndX; ++k) {
        // TODO: pass real WGContext
        m_pTaskSet->ExecuteIteration(k, j, i, nullptr);
      }
    }
  }
  CTaskSetFragment *pNext = g_obThreadPool[m_iQueueId]->GetNextFragment();

  if (nullptr == pNext) {
    // This thread finished execution
    // TODO: pass real WGContext
    m_pTaskSet->DetachFromThread(nullptr);
  }

  // modify the relevant queue's fragments counter
  ::InterlockedDecrement(&g_plFragmentsNotFinished[m_iQueueId]);
  delete this;
  return pNext;
}
void CTaskSetFragment::Execute(unsigned int uiWorkerId) {
#ifdef _DEBUG
  long lInit = InterlockedCompareExchange(&g_lTaskSetInit[m_iQueueId], 0, 0);
  assert(lInit == 0);

  long lExecute =
      InterlockedCompareExchange(&g_lTaskSetExecute[m_iQueueId], 1, 1);
  assert(lExecute == 1);
#endif

  // execute fragment iteration
  for (int i = m_iStartZ; i < m_iEndZ; ++i) {
    for (int j = m_iStartY; j < m_iEndY; ++j) {
      for (int k = m_iStartX; k < m_iEndX; ++k) {
        // TODO: pass real WGContext
        m_pTaskSet->ExecuteIteration(k, j, i, nullptr);
      }
    }
  }
  // modify the relevant queue's fragments counter
  ::InterlockedDecrement(&g_plFragmentsNotFinished[m_iQueueId]);
  delete this;
}

int CTaskSetFragment::AttachToThread(unsigned int uiWorkerId) {
  size_t firstWGID[3];
  size_t lastWGID[3];
#ifdef _DEBUG
  long lInit = InterlockedCompareExchange(&g_lTaskSetInit[m_iQueueId], 0, 0);
  assert(lInit == 0);

  long lExecute =
      InterlockedCompareExchange(&g_lTaskSetExecute[m_iQueueId], 1, 1);
  assert(lExecute == 1);
#endif
  getFirstWGID(firstWGID);
  getLastWGID(lastWGID);
  // TODO: pass real WGContext
  return m_pTaskSet->AttachToThread(nullptr, Size(), firstWGID, lastWGID);
}

size_t CTaskSetFragment::Size() const {
  return (size_t)((m_iEndZ - m_iStartZ) * (m_iEndY - m_iStartY) *
                  (m_iEndX - m_iStartX));
}

void CTaskSetFragment::getFirstWGID(size_t firstWGID[3]) const {
  firstWGID[0] = m_iStartX;
  firstWGID[1] = m_iStartY;
  firstWGID[2] = m_iStartZ;
  return;
}

void CTaskSetFragment::getLastWGID(size_t lastWGID[3]) const {
  lastWGID[0] = m_iEndX;
  lastWGID[1] = m_iEndY;
  lastWGID[2] = m_iEndZ;
  return;
}

//////////////////////////////////////////////////////////////////////////
// ThreadTaskListOrderedImpl implementation
unsigned int
ThreadTaskListOrderedImpl::Enqueue(const SharedPtr<ITaskBase> &pTaskBase) {
  if (nullptr == m_pSelectedWorkerThread) {
    // find the list
    m_pSelectedWorkerThread = g_obThreadPool.front();
    long lSelectedTaskNum = ::InterlockedCompareExchange(
        &(m_pSelectedWorkerThread->m_iNumTasks), 0, 0);

    // Locate a thread with the lowest number of jobs ...
    // NOTE 1:  This approach assumes all jobs are equal,
    //      which is an obviously wrong assumption.
    //      So we will need to think of weighting jobs - based on
    // complexity,       latency etc etc.
    // NOTE 2:  Each thread is hard at work processing jobs.
    //      So by the time we reach the end of the list,
    //      chances are we have the most loaded thread,
    //      instead of the least loaded thread.
    //      So you will also need to figure out a way for each
    //      thread to return an absolute ETA, and then
    //      use the min of those to assign the current job to.
    for (int i = 0; i < g_iThreadPoolSize; i++) {
      long lNumTasks =
          ::InterlockedCompareExchange(&(g_obThreadPool[i]->m_iNumTasks), 0, 0);
      if (0 == lNumTasks) {
        m_pSelectedWorkerThread = g_obThreadPool[i];
        break;
      }
      if (lSelectedTaskNum > lNumTasks) {
        m_pSelectedWorkerThread = g_obThreadPool[i];
        lSelectedTaskNum = lNumTasks;
      }
    }
  }
  if (pTaskBase->IsTaskSet()) {
    m_pSelectedWorkerThread->EnqueueTask(new CTaskSet(
        (ITaskSet *)pTaskBase.GetPtr(), m_pSelectedWorkerThread->m_iQueueId));
  } else {
    m_pSelectedWorkerThread->EnqueueTask(
        new CTask((ITask *)pTaskBase.GetPtr()));
  }
  return 0;
}

unsigned int
ThreadTaskListUnOrderedImpl::Enqueue(const SharedPtr<ITaskBase> &pTaskBase) {
  WorkerThread *pWorkerThread = g_obThreadPool[0];
  int i = 1;
  long lWorkerTaskNum =
      ::InterlockedCompareExchange(&(pWorkerThread->m_iNumTasks), 0, 0);
  while ((i < g_iThreadPoolSize) && (0 != lWorkerTaskNum)) {
    long lTaskNum =
        ::InterlockedCompareExchange(&(g_obThreadPool[i]->m_iNumTasks), 0, 0);
    if (lWorkerTaskNum > lTaskNum) {
      pWorkerThread = g_obThreadPool[i];
      lWorkerTaskNum = lTaskNum;
    }
    ++i;
  }

  if (pTaskBase->IsTaskSet()) {
    pWorkerThread->EnqueueTask(new CTaskSet((ITaskSet *)pTaskBase.GetPtr(),
                                            pWorkerThread->m_iQueueId));
  } else {
    pWorkerThread->EnqueueTask(new CTask((ITask *)pTaskBase.GetPtr()));
  }
  return 0;
}

//////////////////////////////////////////////////////////////////////////
// ThreadTaskExecutor implementation
int ThreadTaskExecutor::Init(unsigned int uiNumThreads,
                             ocl_gpa_data *pGPAData) {
  unsigned long ulNewVal = InterlockedIncrement(&m_lRefCount);
  if (ulNewVal > 1) {
    while (0 == g_iThreadPoolSize)
      ; // spin loop until initialization completed
    return g_iThreadPoolSize;
  }

  if (AUTO_THREADS == uiNumThreads) {
    SYSTEM_INFO siSysInfo;
    GetSystemInfo(&siSysInfo);
    uiNumThreads = siSysInfo.dwNumberOfProcessors;
  }

  m_uiNumWorkingThreads = g_iThreadPoolSize = uiNumThreads;
  for (unsigned int i = 0; i < uiNumThreads; i++) {
    g_pevTaskSetEvents[i] = CreateEvent(0, TRUE, FALSE, 0);
  }

  for (unsigned int i = 0; i < uiNumThreads; i++) {
    // The instrumentation in WorkerThread is disabled in the initial
    // porting of TAL to GPA 4.0 and might be used in later stages
    g_obThreadPool.push_back(new WorkerThread(i, false));
  }
  m_bIsPoolValid = true;

  return uiNumThreads;
}

void ThreadTaskExecutor::Close(bool bCancel) {
  unsigned long ulNewVal = InterlockedDecrement(&m_lRefCount);
  if (ulNewVal > 0) {
    return;
  }

  ClearThreadPool();
}

ocl_gpa_data *ThreadTaskExecutor::GetGPAData() const { return m_pGPAData; }

ITaskList *ThreadTaskExecutor::CreateTaskList(bool OOO) {
  if (OOO) {
    return new ThreadTaskListUnOrderedImpl();
  }
  return new ThreadTaskListOrderedImpl();
}
unsigned int ThreadTaskExecutor::Execute(const SharedPtr<ITaskBase> &pTask,
                                         void *pSubdevTaskExecData) {
  assert(pSubdevTaskExecData !=
         nullptr); // executing on a sub-device isn't yet implemented
  int ret = m_MyList.Enqueue(pTask);
  m_MyList.Flush();
  return ret;
}

bool ThreadTaskExecutor::ClearThreadPool() {
  // Signal that the pool will not accept any more jobs
  m_bIsPoolValid = false;

  for (int i = 0; i < g_iThreadPoolSize; i++) {
    ::CloseHandle(g_pevTaskSetEvents[i]);
  }

  // Start deleting all CWorkerThreads
  // NOTE:  The CWorkerThread destructor posts a WM_QUIT message to the
  // embedded thread,
  //      and then waits on the thread to complete. So each
  // destructor
  // will return only       after the embedded thread has processed
  // all jobs in it's queue
  for (int i = 0; i < g_iThreadPoolSize; i++) {
    delete g_obThreadPool[i];
    g_obThreadPool[i] = nullptr;
  }
  g_obThreadPool.clear();

  return true;
}

#endif // #ifdef __THREAD_EXECUTOR__
