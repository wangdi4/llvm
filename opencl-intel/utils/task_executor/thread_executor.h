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

#pragma once

#ifdef __THREAD_EXECUTOR__

#include "cl_synch_objects.h"
#include "task_executor.h"

#include <list>
#include <queue>

#ifdef _WIN32
#include <Windows.h>
#endif

#define MAX_WORKING_THREADS_COUNT                                              \
  128 // We're assuming that there won't be more than 128 working
      // threads in the execution model

#define TASK_SET_FRAGMENT_SIZE 512 // number of items in a single fragment

namespace Intel {
namespace OpenCL {
namespace TaskExecutor {

class CTaskSet;
class CTaskSetFragment;
class WorkerThread;
typedef std::vector<WorkerThread *> thread_pool;

//////////////////////////////////////////////////////////////////////////
// ITaskWrapper
class ITaskWrapper {
public:
  virtual void Execute(void **pCurrentSet) = 0;

  virtual void Release() = 0;
};

//////////////////////////////////////////////////////////////////////////
// WorkerThread
class WorkerThread {
public:
  WorkerThread(int iQueueId,
               bool bUseTaskalyzer); // Each working thread gets its own id. the
                                     // id will be use to notify other threads
                                     // on task set submission
  virtual ~WorkerThread();

  // Enqueue new task into the working thread's queue
  void EnqueueTask(ITaskWrapper *pTaskWrapper);

  // Ask the thread to notify on completion
  void NotifyDone() { SetEvent(m_hEventComplete); }

  // wait until thread's queue is empty
  int WaitForCompletion();

  bool ExecutionRoutine();

  // get the next task set fragment for execution. when there are no fragments
  // left the function should return NULL
  CTaskSetFragment *GetNextFragment();

  // main thread's processing function
  static unsigned int WINAPI ThreadFunc(LPVOID lpvThreadParam);

  long m_iNumTasks; // Current number of jobs in the thread's queue
  int m_iQueueId;   // thread's queue id

  bool m_bIsBusy; // modify if worker thread is currently busy (true) of not
                  // this flag will be used by the task list to decide wether to
                  // set the thread's event on new tasks

  HANDLE m_evQueueEvent; // task ready event

  HANDLE m_pNotifyEvents[MAX_WORKING_THREADS_COUNT];

private:
  HANDLE m_hThread;          // The embedded thread handle
  unsigned int m_dwThreadID; // The embedded thread id
  HANDLE m_hEvent;           // Event used to signal readiness of thread ...

  HANDLE m_hEventComplete; // Event used to signal queue completion ...

  std::queue<ITaskWrapper *> m_qTasks; // thread's tasks queue

  CRITICAL_SECTION m_QueueLock;

  CTaskSet *m_pCurrentTaskSet;
  bool m_bUseTaskalyzer; // TAL usage flag
};

//////////////////////////////////////////////////////////////////////////
// CTask - represent single task wrapper
class CTask : public ITaskWrapper {
public:
  CTask(const SharedPtr<ITask> &pTask) : m_pTask(pTask) {}

  void Execute(void **pCurrentSet) { m_pTask->Execute(); }
  void Release() {
    m_pTask->Release();
    delete this;
  }

private:
  SharedPtr<ITask> m_pTask; // handle to the actual task
};

//////////////////////////////////////////////////////////////////////////
// CTaskSetFragment - represent task-set fragment
class CTaskSetFragment {
public:
  CTaskSetFragment(int iQueueId, unsigned int iStartX, unsigned int iStartY,
                   unsigned int iStartZ, unsigned int iEndX, unsigned int iEndY,
                   unsigned int iEndZ, ITaskSet *pTaskSet)
      : m_iQueueId(iQueueId), m_iStartX(iStartX), m_iEndX(iEndX),
        m_iStartY(iStartY), m_iEndY(iEndY), m_iStartZ(iStartZ), m_iEndZ(iEndZ),
        m_pTaskSet(pTaskSet) {}

  inline void Execute(unsigned int uiWorkerId);
  // Also returns the next fragment to Execute if such exists
  inline CTaskSetFragment *ExecuteAndGetNext(unsigned int uiWorkerId);
  inline int AttachToThread(unsigned int uiWorkerId);
  inline size_t Size() const;
  inline void getFirstWGID(size_t firstWGID[3]) const;
  inline void getLastWGID(size_t lastWGID[3]) const;

protected:
  int m_iStartX;
  int m_iEndX;
  int m_iStartY;
  int m_iEndY;
  int m_iStartZ;
  int m_iEndZ;

  int m_iQueueId;       // queue id
  ITaskSet *m_pTaskSet; // handle to the actual task set
};

//////////////////////////////////////////////////////////////////////////
// CTaskSet - represent a task-set wrapper
class CTaskSet : public ITaskWrapper {
public:
  CTaskSet(ITaskSet *pTaskSet, int iQueueId);
  void Execute(void **pCurrentSet);

  void Release() {
    m_pTaskSet->Release();
    delete this;
  }
  inline CTaskSetFragment *GetNext();
  CTaskSetFragment *GetFragment(unsigned int uiFrag) {
    return m_vectFragments[uiFrag];
  }

private:
  std::vector<CTaskSetFragment *>
      m_vectFragments; // holds the task set fragements

  ITaskSet *m_pTaskSet; // handle to the actual task set
  int m_iQueueId;       // queue id
};

//////////////////////////////////////////////////////////////////////////
// CTaskSynch
class CTaskSynch : public ITaskWrapper {
public:
  CTaskSynch(WorkerThread *pWorkerThread) : m_pWorkerThread(pWorkerThread) {}
  void Execute(void **pCurrentSet) { m_pWorkerThread->NotifyDone(); }
  void Release() {
    m_pWorkerThread = nullptr;
    delete this;
  }

private:
  WorkerThread *m_pWorkerThread;
};

//////////////////////////////////////////////////////////////////////////
// ThreadTaskListOrderedImp - implementing the ITaskList interface
class ThreadTaskListOrderedImpl : public ITaskList {
public:
  ThreadTaskListOrderedImpl() { m_pSelectedWorkerThread = nullptr; }

  // ITaskList interface
  unsigned int Enqueue(const SharedPtr<ITaskBase> &pTaskBase);
  te_wait_result WaitForCompletion(const SharedPtr<ITaskBase> &pTaskToWait) {
    return TE_WAIT_NOT_SUPPORTED;
  }
  bool Flush() { return true; }

  void Retain() { m_refCount++; }

  void Release() {
    long prevVal = m_refCount--;
    if (1 == prevVal) {
      delete this;
    }
  }

  std::string GetTypeName() const { return "ThreadTaskListOrderedImpl"; }

protected:
  virtual ~ThreadTaskListOrderedImpl(){};

  WorkerThread *m_pSelectedWorkerThread;
  std::atomic<long> m_refCount{1};
};

//////////////////////////////////////////////////////////////////////////
// ThreadTaskListOrderedImp - implementing the ITaskList interface
class ThreadTaskListUnOrderedImpl : public ITaskList {
  friend class ThreadTaskExecutor;

public:
  // ITaskList interface
  unsigned int Enqueue(const SharedPtr<ITaskBase> &pTaskBase);
  te_wait_result WaitForCompletion(const SharedPtr<ITaskBase> &pTaskToWait) {
    return TE_WAIT_NOT_SUPPORTED;
  }
  bool Flush() { return true; }

  void Retain() { m_refCount++; }

  void Release() {
    long prevVal = m_refCount--;
    if (1 == prevVal) {
      delete this;
    }
  }

  std::string GetTypeName() const { return "ThreadTaskListUnOrderedImpl"; }

protected:
  virtual ~ThreadTaskListUnOrderedImpl(){};
  std::atomic<long> m_refCount{1};
};

/////////////////////////////////////////////////////////////////////////////
// ThreadTaskExecutorImpl - implementing the ITaskExecuter interface
class ThreadTaskExecutor : public ITaskExecutor {
public:
  // Constructor
  ThreadTaskExecutor() : m_uiNumWorkingThreads(0), m_lRefCount(0) {}
  virtual ~ThreadTaskExecutor() {}

  // ITaskExecutor interface
  int Init(unsigned int uiNumThreads, ocl_gpa_data *pGPAData);
  unsigned int GetMaxNumOfConcurrentThreads() const {
    return m_uiNumWorkingThreads;
  }
  ITaskList *CreateTaskList(bool OOO = false);
  unsigned int Execute(const SharedPtr<ITaskBase> &pTask,
                       void *pSubdevTaskExecData = nullptr);
  te_wait_result WaitForCompletion() { return TE_WAIT_NOT_SUPPORTED; }
  void Close(bool bCancel);
  ocl_gpa_data *GetGPAData() const;
  int GetErrorCode() override { return 0; }

protected:
  long m_lRefCount;

  int InitThreadPool(unsigned int uiNumThreads); // Initialize the thread pool
  bool ClearThreadPool(); // Destroy all threads in the pool

  unsigned int m_uiNumWorkingThreads;

  HANDLE m_hEvent; // Event used to signal thread completion

  bool m_bIsPoolValid; // Is the thread pool shutting down?

  ThreadTaskListUnOrderedImpl m_MyList;
};
} // namespace TaskExecutor
} // namespace OpenCL
} // namespace Intel

#endif // #ifdef __THREAD_EXECUTOR__
