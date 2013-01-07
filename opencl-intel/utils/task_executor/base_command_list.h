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
#include "tbb/tbb.h"
#include "tbb/task_group.h"

namespace Intel { namespace OpenCL { namespace TaskExecutor {

typedef Intel::OpenCL::Utils::OclNaiveConcurrentQueue<Intel::OpenCL::Utils::SharedPtr<ITaskBase> > ConcurrentTaskQueue;
typedef std::vector<Intel::OpenCL::Utils::SharedPtr<ITaskBase> >             TaskVector;

class TBBTaskExecutor;
class ArenaHandler;

using Intel::OpenCL::Utils::SharedPtr;
using Intel::OpenCL::Utils::OclOsDependentEvent;
using Intel::OpenCL::Utils::OclMutex;

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

/**
 * This class represents a group of tasks for which one can wait to their completion. It replaces tbb::task_group, which can't be used the way we need: parallel_for from one task_group might
 * steal the task which starts another parallel_for.
 */
class TaskGroup
{
public:

    /**
     * Constructor
     * @param arenaHandler the ArenaHandler for enqueuing and executing tasks
     */
    TaskGroup(ArenaHandler& arenaHandler) : m_taskGroupContext(tbb::task_group_context::bound, tbb::task_group_context::default_traits | tbb::task_group_context::concurrent_wait),
        m_rootTask(*new(tbb::task::allocate_root(m_taskGroupContext)) tbb::empty_task()), m_arenaHandler(arenaHandler)
    {
        m_rootTask.increment_ref_count();
    }

    /**
     * Destructor
     */
    ~TaskGroup()
    {
        assert(m_rootTask.ref_count() == 1);    // steady state of reference count when no functor is pending execution is 1
        tbb::task::destroy(m_rootTask);
    }

    /**
     * Enqueue a functor
     * @param F the functor's type
     * @param f the functor object			
     */
    template<typename F>
    void EnqueueFunc(const F& f);

    /**
     * Wait for all functor objects enqueued up to this point to complete
     */
    void WaitForAll();

private:

    tbb::task_group_context m_taskGroupContext;
    tbb::empty_task& m_rootTask;  // an empty task whose reference count represents the number of enqueued tasks that haven't been completed    
    ArenaHandler& m_arenaHandler;

    // auxiliary functor classes to be replaced by lambda functions
    class ArenaFunctor
    {
    protected:

        ArenaFunctor(tbb::task& rootTask) : m_rootTask(rootTask) { }

        tbb::task& m_rootTask;
    };    

    template<typename F>
    class ArenaFunctorRunner : public ArenaFunctor
    {
    public:

        ArenaFunctorRunner<F>(tbb::task& rootTask, const F& func) : ArenaFunctor(rootTask), m_func(func) { }

        void operator()()
        {
            m_func();
            m_rootTask.decrement_ref_count();
        }

    private:

        F m_func;

    };

    class ArenaFunctorWaiter : public ArenaFunctor
    {
    public:

        ArenaFunctorWaiter(tbb::task& rootTask) : ArenaFunctor(rootTask) { }

        void operator()()
        {
            m_rootTask.wait_for_all();  // joins the work in current arena  WARNING: consumes stack, need to reduce number of simultaneously run calls to avoid memory blow-up            
        }

    };

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

	/**
     * Wait for all tasks (commands and executor tasks) to be completed
     */
    virtual void Wait()
	{
		m_taskGroup.WaitForAll();
	}

    ArenaHandler& GetDevArenaHandler() { return m_devArenaHandler; }

    virtual TE_CMD_LIST_PREFERRED_SCHEDULING GetPreferredScheduler() { return TE_CMD_LIST_PREFERRED_SCHEDULING_DYNAMIC; }
    
    virtual tbb::affinity_partitioner* GetAffinityPartitioner() { return NULL; }
   
    std::string GetTypeName() const { return "base_command_list"; }

    /**
     * Enqueue a functor to be run on the device's arena
     * @param F type of functor
     * @param func the functor object to be enqueued
     */
    template<typename F>
    void EnqueueFunc(const F& func)
    {
        m_taskGroup.EnqueueFunc(func);
    }
	friend class immediate_executor_task;

    bool HaveIncomingWork() const
	{
		return !m_quIncomingWork.IsEmpty();
	}

protected:
	friend class in_order_executor_task;
	friend class out_of_order_executor_task;

    base_command_list( TBBTaskExecutor* pTBBExec, ArenaHandler& devArenaHandler);	    	

	virtual unsigned int LaunchExecutorTask(bool blocking,
                                            const Intel::OpenCL::Utils::SharedPtr<ITaskBase>* pTask = NULL) = 0;

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
    TaskGroup               m_taskGroup;

private:
	//Disallow copy constructor
	base_command_list(const base_command_list& l);
	
};

class in_order_command_list : public base_command_list
{
public:

    PREPARE_SHARED_PTR(in_order_command_list)

    static SharedPtr<in_order_command_list> Allocate( TBBTaskExecutor* pTBBExec, 
                                                      ArenaHandler& devArenaHandler, 
                                                      const CommandListCreationParam* param = NULL )
    {
        return new in_order_command_list(pTBBExec, devArenaHandler, param);
    }

protected:

    virtual unsigned int LaunchExecutorTask(bool blocking, const Intel::OpenCL::Utils::SharedPtr<ITaskBase>* pTask = NULL );

private:
    tbb::affinity_partitioner        m_ap;
    TE_CMD_LIST_PREFERRED_SCHEDULING m_scheduling;

    in_order_command_list(TBBTaskExecutor* pTBBExec, ArenaHandler& devArenaHandler, const CommandListCreationParam* param) : 
        base_command_list(pTBBExec, devArenaHandler) 
    {
        m_scheduling = ( NULL != param ) ? param->preferredScheduling : TE_CMD_LIST_PREFERRED_SCHEDULING_DYNAMIC;
    }
};

class out_of_order_command_list : public base_command_list
{
public:

    PREPARE_SHARED_PTR(out_of_order_command_list)

    static SharedPtr<out_of_order_command_list> Allocate( TBBTaskExecutor* pTBBExec, 
                                                          ArenaHandler& devArenaHandler, 
                                                          const CommandListCreationParam* param = NULL )
    {
        return new out_of_order_command_list(pTBBExec, devArenaHandler, param);
    }

    /**
     * Enqueue a task to execute a functor for OOO execution
     * @param F the functor's type
     * @param f the functor's object			
     */
    template<typename F>
    void EnqueueOOOFunc(const F& f)
    {
        m_oooTaskGroup.EnqueueFunc(f);
    }

	/**
     * Wait for all the enqueued commands to be completed
     */
	void WaitForAllCommands()
    {
        m_oooTaskGroup.WaitForAll();
    }

    // overriden methods:    

    void Wait()
    {
        base_command_list::Wait();
        m_oooTaskGroup.WaitForAll();
    }
        
private:

    virtual unsigned int LaunchExecutorTask(bool blocking, const Intel::OpenCL::Utils::SharedPtr<ITaskBase>* pTask = NULL);

    TaskGroup m_oooTaskGroup;

    out_of_order_command_list(TBBTaskExecutor* pTBBExec, ArenaHandler& devArenaHandler, const CommandListCreationParam* param) : 
        base_command_list(pTBBExec, devArenaHandler), m_oooTaskGroup(devArenaHandler) { }
};

class immediate_command_list : public base_command_list
{
public:

    PREPARE_SHARED_PTR(immediate_command_list)

    static SharedPtr<immediate_command_list> Allocate( TBBTaskExecutor* pTBBExec, 
                                                       ArenaHandler& devArenaHandler, 
                                                       const CommandListCreationParam* param )
    {
        return new immediate_command_list(pTBBExec, devArenaHandler, param);
    }

    unsigned int Enqueue(const Intel::OpenCL::Utils::SharedPtr<ITaskBase>& pTask)
    {        
        return LaunchExecutorTask( true, &pTask );
    }

    te_wait_result WaitForCompletion(const Intel::OpenCL::Utils::SharedPtr<ITaskBase>& pTaskToWait)
    {
        return TE_WAIT_NOT_SUPPORTED;
    }

    bool Flush() { return true; }

protected:

    virtual unsigned int LaunchExecutorTask(bool blocking, const Intel::OpenCL::Utils::SharedPtr<ITaskBase>* pTask = NULL);

private:
    tbb::affinity_partitioner m_ap;
    TE_CMD_LIST_PREFERRED_SCHEDULING m_scheduling;
    
    immediate_command_list(TBBTaskExecutor* pTBBExec, ArenaHandler& devArenaHandler, const CommandListCreationParam* param) : 
        base_command_list(pTBBExec, devArenaHandler) 
    {
        m_scheduling = ( NULL != param ) ? param->preferredScheduling : TE_CMD_LIST_PREFERRED_SCHEDULING_DYNAMIC;
    }
};

}}}

