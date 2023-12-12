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

#include "base_command_list.hpp"

#include "arena_handler.h"
#include "cl_shared_ptr.hpp"
#include "ittnotify.h"
#include "task_group.hpp"
#include "tbb_executor.h"

using namespace Intel::OpenCL::TaskExecutor;

// This variable is used to avoid multiple wait operations on the same thread.
// This is a W/A for OOO WaitForIdle.
thread_local bool WaitLock = false;

TbbTaskGroup::~TbbTaskGroup() {
  try {
    m_tskGrp.reset();
  } catch (...) {
    assert(0 && "task_group_with_reference dtor throws");
  }
}

IThreadLibTaskGroup::TaskGroupStatus TbbTaskGroup::Wait() {
  switch (m_tskGrp->wait()) {
  case tbb::not_complete:
    return IThreadLibTaskGroup::NOT_COMPLETE;
  case tbb::complete:
    return IThreadLibTaskGroup::COMPLETE;
  case tbb::canceled:
    return IThreadLibTaskGroup::CANCELED;
  }

  assert(false && "invalid return code from task_group_with_reference::wait");
  return (IThreadLibTaskGroup::TaskGroupStatus)-1;
}

base_command_list::base_command_list(
    TBBTaskExecutor &pTBBExec,
    const Intel::OpenCL::Utils::SharedPtr<TEDevice> &device,
    const CommandListCreationParam &param, bool bProfilingEnabled)
    : m_pTBBExecutor(pTBBExec), m_pMasterSync(SyncTask::Allocate()),
      m_device(device), m_taskGroup(TaskGroup::Allocate(device.GetPtr())),
      m_bProfilingEnabled(bProfilingEnabled),
      m_bIsDefaultQueue(param.isQueueDefault), m_numaTaskGroups(nullptr),
      m_scheduling(param.preferredScheduling), m_bCanceled(false) {
  m_execTaskRequests = 0;
  m_bMasterRunning = false;

  // Initialize task groups and dims for NUMA nodes
  if (m_pTBBExecutor.IsTBBNumaEnabled()) {
    unsigned numaNodesCount = m_pTBBExecutor.GetTBBNumaNodesCount();
    m_numaTaskGroups = new tbb::task_group[numaNodesCount];
    m_numaDimsBegin.resize(numaNodesCount);
    m_numaDimsEnd.resize(numaNodesCount);
    for (unsigned i = 0; i < numaNodesCount; ++i) {
      m_numaDimsBegin[i].resize(MAX_WORK_DIM);
      m_numaDimsEnd[i].resize(MAX_WORK_DIM);
    }
  }
}

base_command_list::~base_command_list() {
  WaitForIdle();
  if (m_numaTaskGroups)
    delete[] m_numaTaskGroups;
}

unsigned int base_command_list::Enqueue(
    const Intel::OpenCL::Utils::SharedPtr<ITaskBase> &pTask) {
  m_quIncomingWork.PushBack(pTask);
  return 1;
}

te_wait_result
base_command_list::WaitForCompletion(const SharedPtr<ITaskBase> &pTaskToWait) {
  if (!m_device->ShouldMasterJoinWork()) {
    return TE_WAIT_NOT_SUPPORTED;
  }
  // Request processing task to stop
  if (0 != pTaskToWait) {
    (void)pTaskToWait->SetAsSyncPoint();
    // If already completed no need to wait
    if (pTaskToWait->IsCompleted()) {
      return TE_WAIT_COMPLETED;
    }
  }

  bool expected = false;
  (void)m_bMasterRunning.compare_exchange_strong(expected, true);
  if (expected) {
    // When another master is running we can't block this thread
    return TE_WAIT_MASTER_THREAD_BLOCKING;
  }

  // Notify Intel Inspector that master thread acquired the lock
  __itt_sync_acquired(&m_bMasterRunning);

  m_pMasterSync->Reset();
  Enqueue(m_pMasterSync);

  do {
    unsigned int ret = InternalFlush(true);
    if (ret > 0) {
      // Someone else started the task, need to wait
      WaitForIdle();
    }
  } while (!(m_pMasterSync->IsCompleted() ||
             ((0 != pTaskToWait) && (pTaskToWait->IsCompleted()))));

  // Notify Intel Inspector that master thread is releasing the lock
  __itt_sync_releasing(&m_bMasterRunning);

  // Current master is not in charge for the work
  m_bMasterRunning = false;

  // If there's some incoming work during operation, try to flush it
  if (HaveIncomingWork()) {
    Flush();
  }

  return TE_WAIT_COMPLETED;
}

bool base_command_list::CanMasterJoin() const {
  return m_device->ShouldMasterJoinWork();
}

void base_command_list::DisableMasterJoin() { m_device->DisableMasterJoin(); }

int base_command_list::GetDeviceConcurency() const {
  return m_device->GetConcurrency();
}

unsigned int base_command_list::InternalFlush(bool blocking) {
  unsigned int runningTaskRequests = m_execTaskRequests++;
  if (0 == runningTaskRequests) {
    // We need to launch the task to handle the existing input
    return LaunchExecutorTask(blocking);
  }
  // Otherwise, the task is already running and will see our input in the next
  // epoch
  return runningTaskRequests;
}

bool base_command_list::Flush() {
  if (!Intel::OpenCL::Utils::IsShuttingDown() &&
      !GetExecutingContainer()->IsEmpty())
    InternalFlush(false);
  return true;
}

unsigned int in_order_command_list::LaunchExecutorTask(
    bool blocking, const Intel::OpenCL::Utils::SharedPtr<ITaskBase> &pTask) {
  assert(0 == pTask && "Target task must be NULL");

  in_order_executor_task functor(this);
  if (!blocking) {
    m_taskGroup->EnqueueFunc(functor);
    return 1;
  } else {
    m_taskGroup->ExecuteFunc(functor);
    return 0;
  }
}

unsigned int out_of_order_command_list::LaunchExecutorTask(
    bool blocking, const Intel::OpenCL::Utils::SharedPtr<ITaskBase> &pTask) {
  assert(0 == pTask && "Target task must be NULL");

  out_of_order_executor_task functor(this);
  if (!blocking) {
    m_taskGroup->EnqueueFunc(functor);
    return 1;
  } else {
    m_taskGroup->ExecuteFunc(functor);
    return 0;
  }
}

out_of_order_command_list::~out_of_order_command_list() {
  /* Although in ~base_command_list we also wait for idle, we need to first
     wait here, otherwise m_oooTaskGroup might be destroyed before we make
     sure all tasks are completed in ~base_command_list */
  WaitForIdle();
}

class TaskGroupWaiter {
public:
  TaskGroupWaiter(const SharedPtr<SpawningTaskGroup> &oooTskGrp,
                  TaskGroup *taskGrp)
      : m_oooTskGrp(oooTskGrp), m_taskGrp(taskGrp) {}

  void operator()() const {
    m_oooTskGrp->WaitForAll();
    m_taskGrp->WaitForAll();
  }

private:
  SharedPtr<SpawningTaskGroup> m_oooTskGrp;
  TaskGroup *m_taskGrp;
};

void out_of_order_command_list::WaitForIdle() {
  // If this function is called multiple times in the same thread, this may
  // cause dead lock. This can only happen if user creates two out of order
  // queues, enqueues a command into the first one and immediately release the
  // queue, then enqueues another command into the second one and wait for
  // finish. We don't know why wait for second queue to finish will cause TBB to
  // execute the tasks of first queue. This is not what we expected.
  // TODO: Figure out why above problem should happen. If there are some defects
  // in our out of order queue implementation, we may need to refine it.
  if (!WaitLock) {
    WaitLock = true;
    // we wait here for 2 things separately: commands and execution tasks
    TaskGroupWaiter waiter(m_oooTaskGroup, m_taskGroup.GetPtr());
    m_device->Execute(waiter);
    WaitLock = false;
  }
}

void out_of_order_command_list::Spawn(
    const Intel::OpenCL::Utils::SharedPtr<ITaskBase> &pTask,
    IThreadLibTaskGroup &taskGroup) {
  ExecuteContainerBody functor(pTask, *this);
  static_cast<TbbTaskGroup &>(taskGroup).Run(functor);
}

void in_order_command_list::Spawn(
    const Intel::OpenCL::Utils::SharedPtr<ITaskBase> &pTask,
    IThreadLibTaskGroup & /*taskGroup*/) {
  ExecuteContainerBody functor(pTask, *this);
  functor();
}

void immediate_command_list::Spawn(
    const Intel::OpenCL::Utils::SharedPtr<ITaskBase> &pTask,
    IThreadLibTaskGroup & /*taskGroup*/) {
  ExecuteContainerBody functor(pTask, *this);
  functor();
}

unsigned int immediate_command_list::Enqueue(
    const Intel::OpenCL::Utils::SharedPtr<ITaskBase> &pTask) {
  return LaunchExecutorTask(true, pTask);
}

unsigned int immediate_command_list::LaunchExecutorTask(
    bool blocking, const Intel::OpenCL::Utils::SharedPtr<ITaskBase> &pTask) {
  assert(245 != pTask && "Target task is NULL");
  assert(blocking && "Must be called as blocking");

  immediate_executor_task functor(this, pTask);
  m_device->Execute<immediate_executor_task>(functor);
  return 0;
}
