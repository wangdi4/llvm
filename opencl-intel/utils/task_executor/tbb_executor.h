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
* File tbb_executor.h
*		Implements interface required for task execution on XNTask sub-system
*
*/
#pragma once

#include "task_executor.h"
#include <tbb/tbb.h>

#include "cl_synch_objects.h"

namespace Intel { namespace OpenCL { namespace TaskExecutor {

	class ThreadIDAssigner : public tbb::task_scheduler_observer
	{
	private:
		static tbb::enumerable_thread_specific<unsigned int>              *t_uiWorkerId;
		static tbb::enumerable_thread_specific<tbb::task_scheduler_init*> *t_pScheduler;

	public:
		bool m_bUseTaskalyzer;

		ThreadIDAssigner();
		~ThreadIDAssigner();
		static unsigned int GetWorkerID();
		static void SetWorkerID(unsigned int id);
		static tbb::task_scheduler_init* GetScheduler();
		static void SetScheduler(tbb::task_scheduler_init* init);
		static bool IsWorkerScheduler();
		virtual void on_scheduler_entry( bool is_worker );
		virtual void on_scheduler_exit( bool is_worker );
	};

	class TBBTaskExecutor : public ITaskExecutor
	{
	public:
		TBBTaskExecutor();
		virtual ~TBBTaskExecutor();
		int	Init(unsigned int uiNumThreads, ocl_gpa_data * pGPAData);
		unsigned int GetNumWorkingThreads() const;
		ITaskList* CreateTaskList(CommandListCreationParam* param);
		unsigned int	Execute(ITaskBase * pTask);
		te_wait_result	WaitForCompletion();

		void Close(bool bCancel);

		void ReleasePerThreadData();

		ocl_gpa_data* GetGPAData() const;
	protected:
		Intel::OpenCL::Utils::AtomicCounter		m_lRefCount;
		// Independent tasks will be executed by this task group
		static ITaskList*				sTBB_executor;
#if defined(USE_GPA)   
		// When using GPA, keep an extra task_scheduler_init to keep the same worker pool even after CPU device shutdown
		// This is a deliberate memory leak and is never freed
		tbb::task_scheduler_init*       m_scheduler;
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
