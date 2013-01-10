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

#include <set>
#include "task_executor.h"
#include <tbb/tbb.h>

#include "cl_synch_objects.h"
#include "cl_dynamic_lib.h"
#include "arena_handler.h"
#include "cl_shared_ptr.h"
#include "cl_synch_objects.h"
#include "base_command_list.h"

using Intel::OpenCL::Utils::SharedPtr;
using Intel::OpenCL::Utils::AtomicPointer;
using Intel::OpenCL::Utils::OclMutex;

namespace Intel { namespace OpenCL { namespace TaskExecutor {

    /**
     * a global flag indicating whether the program has called function exit
     */
    extern volatile bool gIsExiting;

	#define MAX_BATCH_SIZE			128 

	class TBBTaskExecutor : public ITaskExecutor
	{
	public:
		TBBTaskExecutor();
		virtual ~TBBTaskExecutor();
		int	Init(unsigned int uiNumThreads, ocl_gpa_data * pGPAData);        

		bool Activate();
		void Deactivate();

		unsigned int GetNumWorkingThreads() const;
		SharedPtr<ITaskList> CreateTaskList(CommandListCreationParam* param, void* pSubdevTaskExecData);
		unsigned int	Execute(const SharedPtr<ITaskBase>& pTask, void* pSubdevTaskExecData = NULL);
		te_wait_result	WaitForCompletion(ITaskBase * pTask, void* pSubdevTaskExecData = NULL);

		bool AllocateResources();
		void ReleaseResources();

		ocl_gpa_data* GetGPAData() const;

        virtual void* CreateSubdevice(unsigned int uiNumSubdevComputeUnits, const unsigned int* pLegalCores, IAffinityChangeObserver& observer);

        virtual void  ReleaseSubdevice(void* pSubdevData);

        virtual void  WaitUntilEmpty(void* pSubdevData);

        void SetWGContextPool(IWGContextPool* pWgContextPool);

        WGContextBase* GetWGContext(bool bBelongsToMasterThread);

        void ReleaseWorkerWGContext(WGContextBase* wgContext);

	protected:
		// Load TBB library explicitly
		bool LoadTBBLibrary();

		mutable OclMutex m_mutex;

		long								m_lActivateCount;


		// Independent tasks will be executed by this task group
        SharedPtr<ITaskList> m_pExecutorList;

		Intel::OpenCL::Utils::OclDynamicLib	m_dllTBBLib;

        ArenaHandler*                       m_pGlobalArenaHandler;

        /* We need this because of a bug Anton has reported: we should initialize the task_scheduler_init to P+1 threads, instead of P. Apparently, if we explicitly create a task_scheduler_init
           in a certain master thread, TBB creates a global task_scheduler_init object that future created task_arenas will use. Once they fix this bug, we can remove this attribute.
           They seem to have another bug in ~task_scheduler_init(), so we work around it by allocating and not deleting it. */
        tbb::task_scheduler_init*           m_pScheduler;
        IWGContextPool* m_pWgContextPool;

	private:
		TBBTaskExecutor(const TBBTaskExecutor&);
		TBBTaskExecutor& operator=(const TBBTaskExecutor&);
	};

    class MyObserver;

    class TBBThreadPoolPartitioner : public IThreadPoolPartitioner
    {
    public:
		TBBThreadPoolPartitioner(int numThreads, unsigned int* legalCoreIDs, IAffinityChangeObserver* pObserver, void* pSubdevTaskExecData);
        virtual ~TBBThreadPoolPartitioner();
        bool Activate();
        void Deactivate();

    protected:
        MyObserver* m_observer;
	
	private:
		TBBThreadPoolPartitioner(const TBBThreadPoolPartitioner&);
		TBBThreadPoolPartitioner& operator=(const TBBThreadPoolPartitioner&);
    };    
    
    class in_order_executor_task
    {
    public:
	    in_order_executor_task(in_order_command_list* list) : m_list(list){}
	
	    void operator()();

    protected:
	    in_order_command_list* m_list;

	    void FreeCommandBatch(TaskVector* pCmdBatch);

    };

    class out_of_order_executor_task
    {
    public:
	    out_of_order_executor_task(out_of_order_command_list* list) : m_list(list) {}

	    void operator()();

    private:

        SharedPtr<ITaskBase> GetTask();

	    out_of_order_command_list* m_list;
    };

}}}
