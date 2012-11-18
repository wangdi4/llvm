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

#pragma once

#include "cl_synch_objects.h"
#include "cl_shared_ptr.h"
#include "task_executor.h"
#include "tbb/task_group.h"

namespace Intel { namespace OpenCL { namespace TaskExecutor {

typedef Intel::OpenCL::Utils::OclNaiveConcurrentQueue<Intel::OpenCL::Utils::SharedPtr<ITaskBase> > ConcurrentTaskQueue;
typedef std::vector<Intel::OpenCL::Utils::SharedPtr<ITaskBase> >             TaskVector;

class TBBTaskExecutor;
class ArenaHandler;

using Intel::OpenCL::Utils::SharedPtr;

// Master thread syncronization task
// This task used to mark when any master thread requested synchronization point
// Internal variable is set when queue reached the sync. point
// Execute return false, due to execution should be interrupted
class SyncTask : public ITask
{
public:

    PREPARE_SHARED_PTR(SyncTask)
	
    static Intel::OpenCL::Utils::SharedPtr<SyncTask> Allocate() { return Intel::OpenCL::Utils::SharedPtr<SyncTask>(new SyncTask()); }

	void	Reset() { m_bFired = false;}

	// ITask interface
	bool	SetAsSyncPoint() {return false;}
	bool	CompleteAndCheckSyncPoint() { return m_bFired;}
	bool	IsCompleted() const {return m_bFired;}
	bool	Execute() { m_bFired = true; return true;}
	long	Release() { return 0; }	//Persistent member, don't release

protected:

    SyncTask() {m_bFired = false;}

	volatile bool	m_bFired;
};

class base_command_list : public ITaskList
{
public:	    

    PREPARE_SHARED_PTR(base_command_list)

    ~base_command_list();    

    unsigned int Enqueue(const Intel::OpenCL::Utils::SharedPtr<ITaskBase>& pTask)
	{
		m_quIncomingWork.PushBack(pTask);        
		return 0;
	}

	te_wait_result WaitForCompletion(const Intel::OpenCL::Utils::SharedPtr<ITaskBase>& pTaskToWait);

	bool Flush();

	ConcurrentTaskQueue* GetExecutingContainer()
	{
		return &m_quIncomingWork;
	}    

    template<typename F>
    void ExecuteFunction(F& f);

    void Wait();

    ArenaHandler& GetDevArenaHandler() { return m_devArenaHandler; }

    std::string GetTypeName() const { return "base_command_list"; }

protected:
	friend class in_order_executor_task;
	friend class out_of_order_executor_task;

    base_command_list(bool subdevice, TBBTaskExecutor* pTBBExec, ArenaHandler& devArenaHandler);	    
	
	bool HaveIncomingWork() const
	{
		return !m_quIncomingWork.IsEmpty();
	}

	virtual unsigned int LaunchExecutorTask(bool blocking) = 0;

	inline unsigned int InternalFlush(bool blocking);

	TBBTaskExecutor*		m_pTBBExecutor;

	ConcurrentTaskQueue		m_quIncomingWork;

	tbb::atomic<unsigned int>	m_execTaskRequests;
	Intel::OpenCL::Utils::SharedPtr<SyncTask> m_pMasterSync;

	// Only single muster thread can join the execution on specific queue
	// this mutex will block others. The atomic prevents wait on
	// the master if another master is running
	tbb::atomic<bool>		m_bMasterRunning;

    ArenaHandler&           m_devArenaHandler;
    tbb::task_group         m_taskGroup;

    // auxiliary functor classes to be replaced by lambda functions
    class TaskGroupFunctor
    {
    protected:

        TaskGroupFunctor(tbb::task_group& taskGroup) : m_taskGroup(taskGroup) { }

        tbb::task_group& m_taskGroup;
    };

    template<typename F>
    class TaskGroupRunner : public TaskGroupFunctor
    {
    public:

        TaskGroupRunner<F>(tbb::task_group& taskGroup, F& func) : TaskGroupFunctor(taskGroup), m_func(func) {}

        void operator()()
        {
            m_taskGroup.run(m_func);
        }

    private:

        F m_func;

    };

    class TaskGroupWaiter : public TaskGroupFunctor
    {
    public:

        TaskGroupWaiter(tbb::task_group& taskGroup) : TaskGroupFunctor(taskGroup) { }

        void operator()()
        {
            TaskGroupFunctor::m_taskGroup.wait();
        }

    };


private:
	//Disallow copy constructor
	base_command_list(const base_command_list& l);
	
};

class in_order_command_list : public base_command_list
{
public:

    PREPARE_SHARED_PTR(in_order_command_list)

    static SharedPtr<in_order_command_list> Allocate(bool subdevice, TBBTaskExecutor* pTBBExec, ArenaHandler& devArenaHandler)
    {
        return new in_order_command_list(subdevice, pTBBExec, devArenaHandler);
    }

protected:

    virtual unsigned int LaunchExecutorTask(bool blocking);

private:

    in_order_command_list(bool subdevice, TBBTaskExecutor* pTBBExec, ArenaHandler& devArenaHandler) : base_command_list(subdevice, pTBBExec, devArenaHandler) { }
};

class out_of_order_command_list : public base_command_list
{
public:

    PREPARE_SHARED_PTR(out_of_order_command_list)

    static SharedPtr<out_of_order_command_list> Allocate(bool subdevice, TBBTaskExecutor* pTBBExec, ArenaHandler& devArenaHandler)
    {
        return new out_of_order_command_list(subdevice, pTBBExec, devArenaHandler);
    }

    ~out_of_order_command_list()
    {
        m_oooTaskGroup.wait();
    }

    template<typename F>
    void EnqueueFunction(F& f);

    void WaitOnOOOTaskGroup();

protected:

    virtual unsigned int LaunchExecutorTask(bool blocking);

private:

    /* It is not allowed to call task_group::wait() from a worker thread that belongs to the same task_group, therefore we need a task_group for running the
       OOO tasks that is different than the one used to launch the executor task. */
    tbb::task_group m_oooTaskGroup;

    out_of_order_command_list(bool subdevice, TBBTaskExecutor* pTBBExec, ArenaHandler& devArenaHandler) : base_command_list(subdevice, pTBBExec, devArenaHandler) { }
};

}}}
