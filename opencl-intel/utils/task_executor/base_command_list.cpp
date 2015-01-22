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

#include "arena_handler.h"
#include "tbb_executor.h"
#include "cl_shared_ptr.hpp"
#include "task_group.hpp"
#include "base_command_list.hpp"

using namespace Intel::OpenCL::TaskExecutor;

IThreadLibTaskGroup::TaskGroupStatus TbbTaskGroup::Wait()
{
    switch (m_tskGrp.wait())
    {
    case tbb::not_complete:
        return IThreadLibTaskGroup::NOT_COMPLETE;
    case tbb::complete:
        return IThreadLibTaskGroup::COMPLETE;
    case tbb::canceled:
        return IThreadLibTaskGroup::CANCELED;
    default:
        assert(false && "invalid return code from tbb::task_group::wait");
        return (IThreadLibTaskGroup::TaskGroupStatus)-1;
    }
}

base_command_list::base_command_list(TBBTaskExecutor& pTBBExec, const Intel::OpenCL::Utils::SharedPtr<TEDevice>& device, const CommandListCreationParam& param, bool bProfilingEnabled) :
	m_pTBBExecutor(pTBBExec),
	m_pMasterSync(SyncTask::Allocate()),
	m_device(device),
	m_taskGroup(TaskGroup::Allocate(device.GetPtr())),
	m_bProfilingEnabled(bProfilingEnabled),
	m_bIsDefaultQueue(param.isQueueDefault),
	m_scheduling(param.preferredScheduling),
	m_bCanceled(false)
{
    m_execTaskRequests = 0;
    m_bMasterRunning = false;
}

base_command_list::~base_command_list()
{
    WaitForIdle();
}

unsigned int base_command_list::Enqueue(const Intel::OpenCL::Utils::SharedPtr<ITaskBase>& pTask)
{
    m_quIncomingWork.PushBack(pTask);  
    return 1;
}

te_wait_result base_command_list::WaitForCompletion(const SharedPtr<ITaskBase>& pTaskToWait)
{
	  if (!m_device->ShouldMasterJoinWork())
    {
        return TE_WAIT_NOT_SUPPORTED;
    }
    // Request processing task to stop
    if ( NULL != pTaskToWait )
    {
        bool completed = pTaskToWait->SetAsSyncPoint();
        // If already completed no need to wait
        if ( completed )
        {
          return TE_WAIT_COMPLETED;
        }
    }

    bool bVal = m_bMasterRunning.compare_and_swap(true, false);
    if (bVal)
    {
      // When another master is running we can't block this thread
      return TE_WAIT_MASTER_THREAD_BLOCKING;
    }

    if ( NULL == pTaskToWait )
    {
      m_pMasterSync->Reset();
      Enqueue(m_pMasterSync);
    }

    do
    {
        unsigned int ret = InternalFlush(true);
        if ( ret > 0)
        {
            // Someone else started the task, need to wait
            WaitForIdle();
        }
    } while ( !(m_pMasterSync->IsCompleted() || ((NULL != pTaskToWait) && (pTaskToWait->IsCompleted()))) );
		
    // Current master is not in charge for the work
    m_bMasterRunning = false;

    // If there's some incoming work during operation, try to flush it
    if ( HaveIncomingWork() )
    {
        Flush();
    }

    return TE_WAIT_COMPLETED;
}

bool base_command_list::CanMasterJoin() const
{
    return m_device->ShouldMasterJoinWork();
}

int base_command_list::GetDeviceConcurency() const
{
    return m_device->GetConcurrency();
}

unsigned int base_command_list::InternalFlush(bool blocking)
{    
    unsigned int runningTaskRequests = m_execTaskRequests++;
    if ( 0 == runningTaskRequests )
    {
        //We need to launch the task to handle the existing input
        return LaunchExecutorTask(blocking);
    }
    //Otherwise, the task is already running and will see our input in the next epoch
    return runningTaskRequests;
}

bool base_command_list::Flush()
{
    InternalFlush(false);
    return true;
}

unsigned int in_order_command_list::LaunchExecutorTask(bool blocking, const Intel::OpenCL::Utils::SharedPtr<ITaskBase>& pTask )
{
    assert( NULL == pTask && "Target task must be NULL");
    
    in_order_executor_task functor(this);
    if (!blocking)
    {
        m_taskGroup->EnqueueFunc(functor);
        return 1;
    }
    else
    {
        m_device->Execute<in_order_executor_task>(functor);
        return 0;
    }
}

unsigned int out_of_order_command_list::LaunchExecutorTask(bool blocking, const Intel::OpenCL::Utils::SharedPtr<ITaskBase>& pTask)
{
    assert( NULL == pTask && "Target task must be NULL");
    
    out_of_order_executor_task functor(this);
    if (!blocking)
    {
        m_taskGroup->EnqueueFunc(functor);
        return 1;
    }
    else
    {
        m_device->Execute<out_of_order_executor_task>(functor);
        return 0;
    }
}

out_of_order_command_list::~out_of_order_command_list()
{
    /* Although in ~base_command_list we also wait for idle, we need to first wait here, otherwise m_oooTaskGroup might be destroyed before we make sure all tasks are completed in
       ~base_command_list */
	  WaitForIdle();
 }

class TaskGroupWaiter
{
public:
    TaskGroupWaiter(const SharedPtr<SpawningTaskGroup>& tbbTskGrp, TaskGroup& tskGrp, const TEDevice& dev) : m_spawnTskGrp(tbbTskGrp), m_tskGrp(tskGrp), m_dev(dev) { }

    void operator()()
    {
        m_spawnTskGrp->WaitForAll();
        m_tskGrp.WaitForAll();
    }

private:
    SharedPtr<SpawningTaskGroup> m_spawnTskGrp;
    TaskGroup& m_tskGrp;
    const TEDevice& m_dev;
};

void out_of_order_command_list::WaitForIdle()
{
    // we wait here for 2 things separately: commands and execution tasks
    TaskGroupWaiter waiter(m_oooTaskGroup, *m_taskGroup, *m_device);
    m_device->Execute(waiter);
}

ITaskList* base_command_list::GetDebugInOrderDeviceQueue()
{ 
    return m_pTBBExecutor.GetDebugInOrderDeviceQueue();
}

void out_of_order_command_list::Spawn(const Intel::OpenCL::Utils::SharedPtr<ITaskBase>& pTask, IThreadLibTaskGroup& taskGroup)
{
    ExecuteContainerBody functor(pTask, *this);
    static_cast<TbbTaskGroup&>(taskGroup).Run(functor);
}

void in_order_command_list::Spawn(const Intel::OpenCL::Utils::SharedPtr<ITaskBase>& pTask, IThreadLibTaskGroup& taskGroup)
{
    ExecuteContainerBody functor(pTask, *this);
    functor();
}

void immediate_command_list::Spawn(const Intel::OpenCL::Utils::SharedPtr<ITaskBase>& pTask, IThreadLibTaskGroup& taskGroup)
{
    ExecuteContainerBody functor(pTask, *this);
    functor();
}

unsigned int immediate_command_list::Enqueue(const Intel::OpenCL::Utils::SharedPtr<ITaskBase>& pTask)
{    
    return LaunchExecutorTask( true, pTask );
}

unsigned int immediate_command_list::LaunchExecutorTask(bool blocking, const Intel::OpenCL::Utils::SharedPtr<ITaskBase>& pTask )
{
    assert( NULL != pTask && "Target task is NULL");
    assert( blocking && "Must be called as blocking");
    
    immediate_executor_task functor(this, pTask);
    m_device->Execute<immediate_executor_task>(functor);
    return 0;
}
