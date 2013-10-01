// Copyright (c) 2006-2013 Intel Corporation
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
class TEDevice;

using Intel::OpenCL::Utils::SharedPtr;
using Intel::OpenCL::Utils::ConstSharedPtr;
using Intel::OpenCL::Utils::ReferenceCountedObject;
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
    void    Cancel() { Execute(); }

    // ITask interface
    bool	        SetAsSyncPoint() {return false;}
    bool	        CompleteAndCheckSyncPoint() { return m_bFired;}
    bool	        IsCompleted() const {return m_bFired;}
    bool	        Execute() { m_bFired = true; return true;}
    long	        Release() { return 0; }	//Persistent member, don't release
    TASK_PRIORITY	GetPriority() const { return TASK_PRIORITY_MEDIUM;}
    Intel::OpenCL::TaskExecutor::ITaskGroup* GetNDRangeChildrenTaskGroup() { return NULL; }

protected:
    SyncTask() {m_bFired = false;}

    volatile bool	m_bFired;
};

/**
 * This class represents a group of tasks for which one can wait to their completion. It replaces tbb::task_group, which can't be used the way we need: parallel_for from one task_group might
 * steal the task which starts another parallel_for.
 */
class TaskGroup : public ITaskGroup
{
public:
    PREPARE_SHARED_PTR(TaskGroup)

    /**
     * @param device the TEDevice for enqueuing and executing tasks
     * @return a new TaskGroup
     */
    static SharedPtr<TaskGroup> Allocate(TEDevice& device)
    {
      return new TaskGroup(device);
    }

    /**
     * Destructor
     */
    ~TaskGroup()
    {
        assert(m_rootTask.ref_count() == 1);    // steady state of reference count when no functor is pending execution is 1
        tbb::task::destroy(m_rootTask);
    }

    tbb::task_group_context& GetContext() {return m_taskGroupContext; }
    /**
     * Enqueue a functor
     * @param F the functor's type
     * @param f the functor object			
     */
    template<typename F>
    void EnqueueFunc(const F& f);

    // overrriden methods:

    virtual void WaitForAll();
private:

	/**
     * Constructor
     * @param device the TEDevice for enqueuing and executing tasks
     */
    TaskGroup(TEDevice& device) :
        m_taskGroupContext(tbb::task_group_context::bound, tbb::task_group_context::default_traits | tbb::task_group_context::concurrent_wait),
        m_rootTask(*new(tbb::task::allocate_root(m_taskGroupContext)) tbb::empty_task()), m_device(device)
    {
        m_rootTask.increment_ref_count();
    }

    tbb::task_group_context m_taskGroupContext;
    tbb::empty_task& m_rootTask;  // an empty task whose reference count represents the number of enqueued tasks that haven't been completed
    TEDevice& m_device;

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

    unsigned int Enqueue(const Intel::OpenCL::Utils::SharedPtr<ITaskBase>& pTask);

    te_wait_result WaitForCompletion(const Intel::OpenCL::Utils::SharedPtr<ITaskBase>& pTaskToWait);

    bool Flush();

    ConcurrentTaskQueue* GetExecutingContainer()
    {
        return &m_quIncomingWork;
    }

	/**
     * Wait for all tasks (commands and executor tasks) to be completed
     */
    virtual void WaitForIdle()
    {
        m_taskGroup->WaitForAll();
    }

    TEDevice& GetTEDevice() { return *m_device; }

    virtual TE_CMD_LIST_PREFERRED_SCHEDULING GetPreferredScheduler() const { return m_scheduling;}
	
    SharedPtr<ITEDevice> GetDevice() { return m_device; }

    ConstSharedPtr<ITEDevice> GetDevice() const { return (const ITEDevice*)m_device.GetPtr(); }
    
    virtual tbb::affinity_partitioner& GetAffinityPartitioner() { return m_part; }
    virtual tbb::task_group_context&   GetTBBContext() { return m_taskGroup->GetContext(); }

    virtual void Cancel() { m_bCanceled = true; }
    bool         Is_canceled() const { return m_bCanceled; }
   
    /**
     * Enqueue a functor to be run on the device's arena
     * @param F type of functor
     * @param func the functor object to be enqueued
     */
    template<typename F>
    void EnqueueFunc(const F& func)
    {
        m_taskGroup->EnqueueFunc(func);
    }
    friend class immediate_executor_task;

    bool HaveIncomingWork() const
    {
        return !m_quIncomingWork.IsEmpty();
    }

    virtual bool IsProfilingEnabled() const { return m_bProfilingEnabled; }

    void Spawn(const Intel::OpenCL::Utils::SharedPtr<ITaskBase>& pTask, ITaskGroup& taskGroup);

protected:
    friend class in_order_executor_task;
    friend class out_of_order_executor_task;

    base_command_list( TBBTaskExecutor& pTBBExec, const Intel::OpenCL::Utils::SharedPtr<TEDevice>& device, const CommandListCreationParam& param, bool bProfilingEnabled = false);

    virtual unsigned int LaunchExecutorTask(bool blocking,
                                              const Intel::OpenCL::Utils::SharedPtr<ITaskBase>& pTask = NULL) = 0;

    unsigned int InternalFlush(bool blocking);

    TBBTaskExecutor&		m_pTBBExecutor;

    ConcurrentTaskQueue		m_quIncomingWork;

    tbb::atomic<unsigned int>	m_execTaskRequests;
    Intel::OpenCL::Utils::SharedPtr<SyncTask> m_pMasterSync;

    // Only single muster thread can join the execution on specific queue
    // this mutex will block others. The atomic prevents wait on
    // the master if another master is running
    tbb::atomic<bool>		m_bMasterRunning;

    // In most cases m_device should be a shared pointer, but in the case of default command list this will create a cycle. As default command list is 
    // invisible from outside, we need it not to cointain pointer to TEDevice.
    SharedPtr<TEDevice>     m_device;
    SharedPtr<TaskGroup>    m_taskGroup;	
    const bool				m_bProfilingEnabled;
    const bool				m_bIsDefaultQueue;

    // Affinity partitioner used in execution
    tbb::affinity_partitioner	m_part;

    TE_CMD_LIST_PREFERRED_SCHEDULING m_scheduling;

    volatile bool           m_bCanceled;

private:
    //Disallow copy constructor
    base_command_list(const base_command_list& l);
};

class in_order_command_list : public base_command_list
{
public:

    PREPARE_SHARED_PTR(in_order_command_list)

    static SharedPtr<in_order_command_list> Allocate( TBBTaskExecutor& pTBBExec, 
                                                      const Intel::OpenCL::Utils::SharedPtr<TEDevice>& device, 
                                                      const CommandListCreationParam& param )
    {
        return new in_order_command_list(pTBBExec, device, param);
    }

    // This is an optimization: since only one NDRange command can Simultaneously run, all NDRange commands can share the same TaskGroup, without the need to allocate a new one for each of them.
    virtual SharedPtr<ITaskGroup> GetNDRangeChildrenTaskGroup() { return m_ndrangeChildrenTaskGroup; }

    virtual bool DoesSupportDeviceSideCommandEnqueue() const { return true; }

    virtual bool IsProfilingEnabled() const { return false; }

protected:

    virtual unsigned int LaunchExecutorTask(bool blocking, const Intel::OpenCL::Utils::SharedPtr<ITaskBase>& pTask = NULL );

private:
    SharedPtr<TaskGroup>			 m_ndrangeChildrenTaskGroup;

    in_order_command_list(TBBTaskExecutor& pTBBExec, const Intel::OpenCL::Utils::SharedPtr<TEDevice>& device, const CommandListCreationParam& param) :
		base_command_list(pTBBExec, device, param), m_ndrangeChildrenTaskGroup(TaskGroup::Allocate(*device))  {}

};

class out_of_order_command_list : public base_command_list
{
public:
    PREPARE_SHARED_PTR(out_of_order_command_list)

    static SharedPtr<out_of_order_command_list> Allocate( TBBTaskExecutor& pTBBExec, 
                                                          const Intel::OpenCL::Utils::SharedPtr<TEDevice>& device, 
                                                          const CommandListCreationParam& param )
    {
        return new out_of_order_command_list(pTBBExec, device, param);
    }

    /**
     * Destructor
     */
    ~out_of_order_command_list();

    /**
     * Execute a functor for OOO execution (this method should be called only when running inside the arena)
     * @param F the functor's type
     * @param f the functor's object			
     */
     template<typename F>
     void ExecOOOFunc(const F& f)
     {
         m_oooTaskGroup.run(f);
     }

	/**
     * Wait for all the enqueued commands to be completed
     */
     void WaitForAllCommands()
     {
         m_oooTaskGroup.wait();
     }

     // overriden methods:

     void WaitForIdle();

    bool DoesSupportDeviceSideCommandEnqueue() const { return true; }

    virtual SharedPtr<ITaskGroup> GetNDRangeChildrenTaskGroup() { return TaskGroup::Allocate(GetTEDevice()); }

private:
    virtual unsigned int LaunchExecutorTask(bool blocking, const Intel::OpenCL::Utils::SharedPtr<ITaskBase>& pTask = NULL);

    tbb::task_group m_oooTaskGroup;

    out_of_order_command_list(TBBTaskExecutor& pTBBExec, const Intel::OpenCL::Utils::SharedPtr<TEDevice>& device, const CommandListCreationParam& param) :
		base_command_list(pTBBExec, device, param, param.isProfilingEnabled) { }
};

bool execute_command(const SharedPtr<ITaskBase>& pCmd, base_command_list& cmdList);

struct ExecuteContainerBody
{
    const SharedPtr<ITaskBase> m_pTask;
    base_command_list& m_list;

    ExecuteContainerBody(const SharedPtr<ITaskBase>& pTask, base_command_list& list) :
			m_pTask(pTask), m_list(list) {}

    void operator()()
    {
        execute_command(m_pTask, m_list);
    }
};

class immediate_command_list : public base_command_list
{
public:

    PREPARE_SHARED_PTR(immediate_command_list)

    static SharedPtr<immediate_command_list> Allocate( TBBTaskExecutor& pTBBExec, 
                                                       const Intel::OpenCL::Utils::SharedPtr<TEDevice>& device, 
                                                       const CommandListCreationParam& param )
    {
        return new immediate_command_list(pTBBExec, device, param);
    }

    unsigned int Enqueue(const Intel::OpenCL::Utils::SharedPtr<ITaskBase>& pTask);

    te_wait_result WaitForCompletion(const Intel::OpenCL::Utils::SharedPtr<ITaskBase>& pTaskToWait)
    {
        return TE_WAIT_NOT_SUPPORTED;
    }

    bool Flush() { return true; }

    virtual SharedPtr<ITaskGroup> GetNDRangeChildrenTaskGroup() { return TaskGroup::Allocate(GetTEDevice()); }

    bool DoesSupportDeviceSideCommandEnqueue() const { return false; }

    virtual bool IsProfilingEnabled() const { return false; }

protected:

    virtual unsigned int LaunchExecutorTask(bool blocking, const Intel::OpenCL::Utils::SharedPtr<ITaskBase>& pTask = NULL);

private:
    tbb::affinity_partitioner m_ap;
    
    immediate_command_list(TBBTaskExecutor& pTBBExec, const Intel::OpenCL::Utils::SharedPtr<TEDevice>& device, const CommandListCreationParam& param) :
        base_command_list(pTBBExec, device, param) {}
};

}}}
