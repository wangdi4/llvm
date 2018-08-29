// INTEL CONFIDENTIAL
//
// Copyright 2006-2018 Intel Corporation.
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

#include <cassert>
#include "tbb/task.h"
#include "task_executor.h"
#include "cl_shared_ptr.h"

namespace Intel { namespace OpenCL { namespace TaskExecutor {

class TEDevice;

/**
 * Base class of all clases that implement ITaskGroup
 */
class TaskGroupBase : public ITaskGroup
{
public:

    /**
     * Destructor
     */
    virtual ~TaskGroupBase()
    {
        if (m_pRootTask->decrement_ref_count() == 0)
        {
            tbb::task::destroy(*m_pRootTask);
        }
    }

    /**
     * @return the task_group_context of this TaskGroup object
     */
    tbb::task_group_context& GetContext() { return m_taskGroupContext; }

protected:

    /**
     * the task_group_context passed to tbb::task::allocate_root in order to create m_pRootTask
     */
    tbb::task_group_context m_taskGroupContext;
    /**
     * an empty task whose reference count represents the number of enqueued tasks that haven't been completed
     */
    tbb::empty_task* m_pRootTask;
    /**
     * the TEDevice for enqueuing and executing tasks
     */
    TEDevice* m_device;

    /**
     * @param device the TEDevice for enqueuing and executing tasks
     */
    explicit TaskGroupBase(TEDevice* device) :
        m_taskGroupContext(tbb::task_group_context::bound, tbb::task_group_context::default_traits | tbb::task_group_context::concurrent_wait),
        m_pRootTask(new(tbb::task::allocate_root(m_taskGroupContext)) tbb::empty_task()), m_device(device)
    {
        m_pRootTask->set_ref_count(1);
    }
    
    // auxiliary functor classes to be replaced by lambda functions
    class ArenaFunctor
    {
    protected:

        ArenaFunctor(tbb::task* pRootTask) : m_pRootTask(pRootTask) { }

        tbb::task* m_pRootTask;
    };    

    class ArenaFunctorWaiter : public ArenaFunctor
    {
    public:
        ArenaFunctorWaiter(tbb::task* pRootTask) : ArenaFunctor(pRootTask) { }

        void operator()()
        {
            m_pRootTask->wait_for_all();  // joins the work in current arena  WARNING: consumes stack, need to reduce number of simultaneously run calls to avoid memory blow-up            
        }

    };

};

/**
 * This class represents a group of tasks for which one can wait to their completion. It replaces tbb::task_group, which can't be used the way we need: parallel_for from one task_group might
 * steal the task which starts another parallel_for.
 */
class TaskGroup : public TaskGroupBase
{ 
public:
    PREPARE_SHARED_PTR(TaskGroup)

    /**
     * @param device the TEDevice for enqueuing and executing tasks
     * @return a new TaskGroup
     */
    static Intel::OpenCL::Utils::SharedPtr<TaskGroup> Allocate(TEDevice* device)
    {
      return new TaskGroup(device);
    }   
    
    /**
     * Enqueue a functor
     * @param F the functor's type
     * @param f the functor object			
     */
    template<typename F>
    void EnqueueFunc(const F& f);

    // overriden methods:

    void WaitForAll();

private:    

    TaskGroup(TEDevice* device) : TaskGroupBase(device) { }       

    template<typename F>
    class ArenaFunctorRunner : public ArenaFunctor
    {
    public:
        ArenaFunctorRunner<F>(tbb::task* pRootTask, const F& func) : ArenaFunctor(pRootTask), m_func(func) { }

        void operator()()
        {
            m_func();          
            if (m_pRootTask->decrement_ref_count() == 0)
            {
                tbb::task::destroy(*m_pRootTask);
            }
        }

    private:
        F m_func;
    };    

};

/**
 * Implements ITaskGroup and executing a functor by wrapping it with a task that is allocated a child of the root task and then spawned. It is used instead of tbb::task_group, because if a
 * task T is run by a tbb::task_group causes this tbb::task_group's destructor to be called up the stack, there is a deadlock, because tbb::~task_group waits for all its tasks to finish,
 * but task T has not finished yet. The implemetation of SpawningTaskGroup prevents this.
 * This class is used for grouping tasks executing the commands themselves in OOO command list.
 */
class SpawningTaskGroup : public TaskGroupBase
{
public:
    PREPARE_SHARED_PTR(SpawningTaskGroup)

    /**
     * @param pDevice the TEDevice on which that tasks are executed
     * @return a new SpawningTaskGroup
     */
    static Intel::OpenCL::Utils::SharedPtr<SpawningTaskGroup> Allocate(TEDevice* pDevice)
    {
        return new SpawningTaskGroup(pDevice);
    }

    /**
     * Spawn a task executing a functor
     * @param F the type of the functor
     * @param f the functor to be execution
     */
    template<typename F>
    void Spawn(const F& f);

    virtual void WaitForAll();

private:

    SpawningTaskGroup(TEDevice* pDevice) : TaskGroupBase(pDevice) { }

    template<typename F>
    class TaskGroupTask : public tbb::task
    {
    public:

        TaskGroupTask(const F& func) : m_func(func) { }

        // overriden methods:

        virtual task* execute()
        {
            m_func();
            return nullptr;
        }

    private:

        F m_func;

    };

};

}}}
