// Copyright (c) 2006-2012 Intel Corporation
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
* File tbb_executor.h
*		Implements interface required for task execution on XNTask sub-system
*
*/
#pragma once

#include "task_executor.h"
#include <tbb/tbb.h>

#include "cl_synch_objects.h"

namespace Intel { namespace OpenCL { namespace TaskExecutor {

	#define INVALID_WORKER_ID		0xFFFFFFFF
	#define INVALID_SCHEDULER_ID	((tbb::task_scheduler_init*)(-1))

	class ThreadIDAllocator
	{
	public:
		ThreadIDAllocator(unsigned int uiNumOfThreads);

		unsigned int GetNextAvailbleId();	// Return available Id, and increase ref. count
											// -1 return is there is no available id's, ref. count is not increased
		void ReleaseId(unsigned int uiId);	// Release Available id and decreases reference count

		long GetRefCount() { return m_aRefCount;}

	protected:
		// can't be deleted
		~ThreadIDAllocator() {};

		unsigned int							m_uiNumThreads;
		//The variables below are used to ensure working threads have unique IDs in the range [0, numThreads - 1]
		//The idea is that as threads enter the pool they get a unique identifier (NextAvailableThreadId) and use the thread ID from gAvailableThreadIds in that index
		//When a thread leaves the pool, it decrements NextAvailableThreadId and writes his id to gAvailableThreadIds in the previous value
		Intel::OpenCL::Utils::AtomicBitField	m_aThreadAvailabilityMask;

		Intel::OpenCL::Utils::AtomicCounter		m_aRefCount; // Reference count is set to be 1
	};

	class ThreadIDAssigner : public tbb::task_scheduler_observer
	{
		struct thread_local_data
		{
			thread_local_data() : uiWorkerId(INVALID_WORKER_ID), pScheduler(INVALID_SCHEDULER_ID), pIDAllocator(NULL) {};
			unsigned int				uiWorkerId;
			tbb::task_scheduler_init*	pScheduler;
			ThreadIDAllocator*			pIDAllocator;
		};

		static tbb::enumerable_thread_specific<thread_local_data>  *t_pThreadLocalData;

		bool m_bUseTaskalyzer;
		Intel::OpenCL::Utils::AtomicPointer<ThreadIDAllocator> m_pIdAllocator;

	public:

		ThreadIDAssigner();
		~ThreadIDAssigner();

		static unsigned int GetWorkerID();
		static void SetWorkerID(unsigned int id);
		static void ReleaseWorkerID();
		static tbb::task_scheduler_init* GetScheduler();
		static void SetScheduler(tbb::task_scheduler_init* init);
		static bool IsWorkerScheduler();

		virtual void on_scheduler_entry( bool is_worker );
		virtual void on_scheduler_exit( bool is_worker );

		ThreadIDAllocator* SetThreadIdAllocator(ThreadIDAllocator* newIdAllocator);
	};

	class TBBTaskExecutor : public ITaskExecutor
	{
	public:
		TBBTaskExecutor();
		virtual ~TBBTaskExecutor();
		int	Init(unsigned int uiNumThreads, ocl_gpa_data * pGPAData);

		bool Activate();
		void Deactivate();

		unsigned int GetNumWorkingThreads() const;
		ITaskList* CreateTaskList(CommandListCreationParam* param);
		unsigned int	Execute(ITaskBase * pTask);
		te_wait_result	WaitForCompletion();

		void Close(bool bCancel);

		void ReleasePerThreadData();

		ocl_gpa_data* GetGPAData() const;
	protected:
		Intel::OpenCL::Utils::OclMutex		m_muActivate;

		friend class						base_command_list;
		tbb::task_group_context*			m_pGrpContext;

		long								m_lActivateCount;


		// Independent tasks will be executed by this task group
		ITaskList*							m_pExecutorList;
#if defined(USE_GPA)
		// When using GPA, keep an extra task_scheduler_init to keep the same worker pool even after CPU device shutdown
		// This is a deliberate memory leak and is never freed
		tbb::task_scheduler_init*			m_pGPAscheduler;
#endif
	private:
		ThreadIDAssigner* m_threadPoolChangeObserver;
	};

    class MyObserver;

    class TBBThreadPoolPartitioner : public IThreadPoolPartitioner
    {
    public:
		TBBThreadPoolPartitioner(int numThreads, unsigned int* legalCoreIDs, IAffinityChangeObserver* pObserver);
        virtual ~TBBThreadPoolPartitioner();
        bool Activate();
        void Deactivate();

    protected:
        MyObserver* m_observer;
    };
}}}
