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
* File xn_executor.h
*		Implements interface required for task execution on XNTask sub-system
*
*/
#pragma once

#ifdef _WIN32
#include <Windows.h>
#endif

#include "task_executor.h"
#include "cl_synch_objects.h"
#include <list>
#include <queue>
#include <map>
using namespace Intel::OpenCL::Utils;

#define	MAX_WORKING_THREADS_COUNT	128		// We're assuming that there won't be more than 128 working
											// threads in the execution model

#define TASK_SET_FRAGMENT_SIZE		512	// number of items in a single fragment 

namespace Intel { namespace OpenCL { namespace TaskExecutor {

	class CTaskSet;
	class CTaskSetFragment;
	class WorkerThread;
	typedef std::vector<WorkerThread*>	thread_pool;

	//////////////////////////////////////////////////////////////////////////
	// ITaskWrapper
	class ITaskWrapper
	{
	public:

		virtual void Execute(void** pCurrentSet) = 0;

		virtual void Release() = 0;
	};


	//////////////////////////////////////////////////////////////////////////
	// WorkerThread
	class WorkerThread  
	{
	public:
		
		WorkerThread(int iQueueId);	// Each working thread gets its own id. the id will be use to
									// notify other threads on task set submission
		virtual ~WorkerThread();

		// Enqueue new task into the working thread's queue
		void EnqueueTask(ITaskWrapper * pTaskWrapper);
		
		// Ask the thread to notify on completion
		void NotifyDone(){ SetEvent( m_hEventComplete ); }

		// wait until thread's queue is empty
		int WaitForCompletion();

		bool ExecutionRoutine();

		// get the next task set fragment for execution. when there are no fragments left the 
		// function should return NULL
		CTaskSetFragment * GetNextFragment();

		// main thread's processing function
		static unsigned int WINAPI ThreadFunc( LPVOID lpvThreadParam );

		long		m_iNumTasks;	// Current number of jobs in the thread's queue
		int			m_iQueueId;		// thread's queue id

		bool				m_bIsBusy;			// modify if worker thread is currently busy (true) of not
												// this flag will be used by the task list to decide wether to
												// set the thread's event on new tasks
		
		HANDLE				m_evQueueEvent;		// task ready event

		HANDLE				m_pNotifyEvents[MAX_WORKING_THREADS_COUNT];

	private:

		HANDLE						m_hThread;		// The embedded thread handle
		unsigned int				m_dwThreadID;	// The embedded thread id
		HANDLE						m_hEvent;		// Event used to signal readiness of thread ...

		HANDLE						m_hEventComplete; // Event used to signal queue completion ...

		std::queue<ITaskWrapper*>	m_qTasks;		// thread's tasks queue

		CRITICAL_SECTION		m_QueueLock;		

		CTaskSet *				m_pCurrentTaskSet;
	};
	
	//////////////////////////////////////////////////////////////////////////
	// CTask - represent single task wrapper
	class CTask : public ITaskWrapper
	{
	public:
		CTask(ITask * pTask) : m_pTask(pTask){}

		void Execute(void** pCurrentSet){ m_pTask->Execute(); }
		void Release(){ m_pTask->Release(); delete this; }

	private:
		ITask * m_pTask;	// handle to the actual task
	};

	//////////////////////////////////////////////////////////////////////////
	// CTaskSetFragment - represent task-set fragment
	class CTaskSetFragment
	{
	public:
		CTaskSetFragment(int iQueueId, unsigned int iStart[3], unsigned int iEnd[3], ITaskSet * pTaskSet) : 
		  m_iQueueId(iQueueId), m_iStartX(iStart[0]), m_iEndX(iEnd[0]), m_iStartY(iStart[1]),
			  m_iEndY(iEnd[1]), m_iStartZ(iStart[2]), m_iEndZ(iEnd[2]), m_pTaskSet(pTaskSet){}

		inline void		Execute(unsigned int uiWorkerId);
		inline int		AttachToThread(unsigned int uiWorkerId);

	protected:
		int				m_iStartX;
		int				m_iEndX; 
		int				m_iStartY;
		int				m_iEndY; 
		int				m_iStartZ;
		int				m_iEndZ;

		int				m_iQueueId;		// queue id
		ITaskSet *		m_pTaskSet;		// handle to the actual task set
	};

	//////////////////////////////////////////////////////////////////////////
	// CTaskSet - represent a task-set wrapper
	class CTaskSet : public ITaskWrapper
	{
	public:
		CTaskSet(ITaskSet * pTaskSet, int iQueueId);
		void Execute(void** pCurrentSet);

		void Release()
		{ 
			m_pTaskSet->Release();
			delete this; 
		}
		inline CTaskSetFragment * GetNext();
		CTaskSetFragment * GetFragment(unsigned int uiFrag) {return m_vectFragments[uiFrag];}

	private:

		std::vector<CTaskSetFragment*>	m_vectFragments; // holds the task set fragements
		
		ITaskSet *						m_pTaskSet;			// handle to the actual task set
		int								m_iQueueId;			// queue id
	};

	//////////////////////////////////////////////////////////////////////////
	// CTaskSynch
	class CTaskSynch : public ITaskWrapper
	{
	public:
		CTaskSynch(WorkerThread * pWorkerThread) : m_pWorkerThread(pWorkerThread){}
		void Execute(void** pCurrentSet)
		{ 
			m_pWorkerThread->NotifyDone(); 
		}
		void Release()
		{ 
			m_pWorkerThread = NULL;
			delete this; 
		}
	private:
		WorkerThread * m_pWorkerThread;
	};

	//////////////////////////////////////////////////////////////////////////
	// ThreadTaskListOrderedImp - implementing the ITaskList interface
	class ThreadTaskListOrderedImpl : public ITaskList
	{
	public:
		ThreadTaskListOrderedImpl()
		{
			m_pSelectedWorkerThread = NULL;
		}

		// ITaskList interface
		unsigned int Enqueue(ITaskBase* pTaskBase);
		void         Flush() {};
		void         Release()
		{ 
			delete this;
		}

	protected:
		virtual ~ThreadTaskListOrderedImpl() {};

		WorkerThread * m_pSelectedWorkerThread;
	};

	//////////////////////////////////////////////////////////////////////////
	// ThreadTaskListOrderedImp - implementing the ITaskList interface
	class ThreadTaskListUnOrderedImpl : public ITaskList
	{
		friend class ThreadTaskExecutor;
	public:
		ThreadTaskListUnOrderedImpl()
		{
		}

		// ITaskList interface
		unsigned int Enqueue(ITaskBase* pTaskBase);
		void         Flush() {};
		void         Release()
		{ 
			delete this;
		}

	protected:
		virtual ~ThreadTaskListUnOrderedImpl() {};
	};

	/////////////////////////////////////////////////////////////////////////////
	// ThreadTaskExecutorImpl - implementing the ITaskExecuter interface
	class ThreadTaskExecutor : public ITaskExecutor
	{
	public:
		// Constructor
		ThreadTaskExecutor() : m_uiNumWorkingThreads(0), m_lRefCount(0) {}
		virtual ~ThreadTaskExecutor(){}

		// ITaskExecutor interface
		int	Init(unsigned int uiNumThreads);
		unsigned int GetNumWorkingThreads() const
						{return m_uiNumWorkingThreads;}
		ITaskList* CreateTaskList(bool OOO = false);
		unsigned int	Execute(ITaskBase * pTask);
		void Close(bool bCancel);

	protected:
		long		m_lRefCount;

		int InitThreadPool(unsigned int uiNumThreads); // Initialize the thread pool
		bool ClearThreadPool(); // Destroy all threads in the pool

		unsigned int	m_uiNumWorkingThreads;

		HANDLE			m_hEvent; // Event used to signal thread completion
		
		bool				m_bIsPoolValid;		// Is the thread pool shutting down? 

		ThreadTaskListUnOrderedImpl	m_MyList;

	};
}}}