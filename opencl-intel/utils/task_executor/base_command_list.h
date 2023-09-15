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

#pragma once

#include "cl_shared_ptr.h"
#include "cl_synch_objects.h"
#include "task_executor.h"
#include "task_group.h"
#include "tbb/partitioner.h"

namespace Intel {
namespace OpenCL {
namespace TaskExecutor {

typedef Intel::OpenCL::Utils::OclNaiveConcurrentQueue<
    Intel::OpenCL::Utils::SharedPtr<ITaskBase>>
    ConcurrentTaskQueue;
typedef std::vector<Intel::OpenCL::Utils::SharedPtr<ITaskBase>> TaskVector;

class TBBTaskExecutor;
class TEDevice;

using Intel::OpenCL::Utils::ConstSharedPtr;
using Intel::OpenCL::Utils::OclOsDependentEvent;
using Intel::OpenCL::Utils::ReferenceCountedObject;
using Intel::OpenCL::Utils::SharedPtr;

// Master thread syncronization task
// This task used to mark when any master thread requested synchronization point
// Internal variable is set when queue reached the sync. point
// Execute return false, due to execution should be interrupted
class SyncTask : public ITask {
public:
  PREPARE_SHARED_PTR(SyncTask)

  static Intel::OpenCL::Utils::SharedPtr<SyncTask> Allocate() {
    return Intel::OpenCL::Utils::SharedPtr<SyncTask>(new SyncTask());
  }

  void Reset() { m_bFired = false; }
  void Cancel() override { Execute(); }

  // ITask interface
  bool SetAsSyncPoint() override { return false; }
  bool CompleteAndCheckSyncPoint() override { return m_bFired; }
  bool IsCompleted() const override { return m_bFired; }
  bool Execute() override {
    m_bFired = true;
    return true;
  }
  long Release() override { return 0; } // Persistent member, don't release
  TASK_PRIORITY GetPriority() const override { return TASK_PRIORITY_MEDIUM; }
  Intel::OpenCL::TaskExecutor::IThreadLibTaskGroup *
  GetNDRangeChildrenTaskGroup() override {
    return nullptr;
  }

protected:
  SyncTask() { m_bFired = false; }

  volatile bool m_bFired;
};

/**
 * This class implements IThreadLibTaskGroup for TBB
 */
class TbbTaskGroup : public IThreadLibTaskGroup {
public:
  TbbTaskGroup() : m_tskGrp(new task_group_with_reference()) {}

  PREPARE_SHARED_PTR(TbbTaskGroup)

  static SharedPtr<TbbTaskGroup> Allocate() { return new TbbTaskGroup(); }

  /**
   * run a functor on this TbbTaskGroup
   * @param Func the class of the functor
   * @param f the functor object
   */
  template <typename Func> void Run(Func &f);

  // overriden methods

  virtual TaskGroupStatus Wait() override;

  virtual ~TbbTaskGroup();

private:
  std::unique_ptr<task_group_with_reference> m_tskGrp;
};

class base_command_list : public ITaskList {
public:
  PREPARE_SHARED_PTR(base_command_list)

  ~base_command_list();

  base_command_list(const base_command_list &l) = delete;
  base_command_list &operator=(const base_command_list &) = delete;
  base_command_list &operator=(base_command_list &&) = delete;

  unsigned int
  Enqueue(const Intel::OpenCL::Utils::SharedPtr<ITaskBase> &pTask) override;

  te_wait_result WaitForCompletion(
      const Intel::OpenCL::Utils::SharedPtr<ITaskBase> &pTaskToWait) override;

  bool CanMasterJoin() const override;

  void DisableMasterJoin() override;

  int GetDeviceConcurency() const override;

  bool Flush() override;

  ConcurrentTaskQueue *GetExecutingContainer() { return &m_quIncomingWork; }

  /**
   * Wait for all tasks (commands and executor tasks) to be completed
   */
  virtual void WaitForIdle() { m_taskGroup->WaitForAll(); }

  TEDevice *GetTEDevice() { return m_device.GetPtr(); }

  virtual TE_CMD_LIST_PREFERRED_SCHEDULING GetPreferredScheduler() const {
    return m_scheduling;
  }

  SharedPtr<ITEDevice> GetDevice() override { return m_device; }

  ConstSharedPtr<ITEDevice> GetDevice() const override {
    return (const ITEDevice *)m_device.GetPtr();
  }

  tbb::task_group *GetNumaTaskGroups() const { return m_numaTaskGroups; }
  std::vector<std::vector<size_t>> &GetNumaDimsBegin() {
    return m_numaDimsBegin;
  }
  std::vector<std::vector<size_t>> &GetNumaDimsEnd() { return m_numaDimsEnd; }

  virtual tbb::affinity_partitioner &GetAffinityPartitioner() { return m_part; }
  virtual tbb::static_partitioner &GetStaticPartitioner() {
    return m_staticPartitioner;
  }

  virtual void Cancel() override { m_bCanceled = true; }
  bool Is_canceled() const { return m_bCanceled; }

  friend class immediate_executor_task;

  bool HaveIncomingWork() const { return !m_quIncomingWork.IsEmpty(); }

  virtual bool IsProfilingEnabled() const override {
    return m_bProfilingEnabled;
  }

protected:
  friend class in_order_executor_task;
  friend class out_of_order_executor_task;

  base_command_list(TBBTaskExecutor &pTBBExec,
                    const Intel::OpenCL::Utils::SharedPtr<TEDevice> &device,
                    const CommandListCreationParam &param,
                    bool bProfilingEnabled = false);

  virtual unsigned int LaunchExecutorTask(
      bool blocking,
      const Intel::OpenCL::Utils::SharedPtr<ITaskBase> &pTask = nullptr) = 0;

  unsigned int InternalFlush(bool blocking);

  TBBTaskExecutor &m_pTBBExecutor;

  ConcurrentTaskQueue m_quIncomingWork;

  std::atomic<unsigned int> m_execTaskRequests;
  Intel::OpenCL::Utils::SharedPtr<SyncTask> m_pMasterSync;

  // Only single muster thread can join the execution on specific queue
  // this mutex will block others. The atomic prevents wait on
  // the master if another master is running
  std::atomic<bool> m_bMasterRunning;

  // In most cases m_device should be a shared pointer, but in the case of
  // default command list this will create a cycle. As default command list is
  // invisible from outside, we need it not to cointain pointer to TEDevice.
  SharedPtr<TEDevice> m_device;
  SharedPtr<TaskGroup> m_taskGroup;
  const bool m_bProfilingEnabled;
  const bool m_bIsDefaultQueue;

  // TBB task groups used for arenas bound to NUMA nodes.
  tbb::task_group *m_numaTaskGroups;
  // Beginning of ranges for NUMA arenas.
  std::vector<std::vector<size_t>> m_numaDimsBegin;
  // End of ranges for NUMA arenas.
  std::vector<std::vector<size_t>> m_numaDimsEnd;

  // Affinity partitioner used in execution
  tbb::affinity_partitioner m_part;
  // Static partitioner
  tbb::static_partitioner m_staticPartitioner;

  TE_CMD_LIST_PREFERRED_SCHEDULING m_scheduling;

  volatile bool m_bCanceled;
};

class in_order_command_list : public base_command_list {
public:
  PREPARE_SHARED_PTR(in_order_command_list)

  static SharedPtr<in_order_command_list>
  Allocate(TBBTaskExecutor &pTBBExec,
           const Intel::OpenCL::Utils::SharedPtr<TEDevice> &device,
           const CommandListCreationParam &param,
           bool bProfilingEnabled = false) {
    return new in_order_command_list(pTBBExec, device, param,
                                     bProfilingEnabled);
  }

  // This is an optimization: since only one NDRange command can Simultaneously
  // run, all NDRange commands can share the same TaskGroup, without the need to
  // allocate a new one for each of them.
  virtual SharedPtr<IThreadLibTaskGroup>
  GetNDRangeChildrenTaskGroup() override {
    return m_ndrangeChildrenTaskGroup;
  }

  virtual bool DoesSupportDeviceSideCommandEnqueue() const override {
    return true;
  }

  virtual bool IsMasterJoined() const override { return m_bMasterRunning; }

  virtual void Spawn(const Intel::OpenCL::Utils::SharedPtr<ITaskBase> &pTask,
                     IThreadLibTaskGroup &taskGroup) override;

protected:
  virtual unsigned int
  LaunchExecutorTask(bool blocking,
                     const Intel::OpenCL::Utils::SharedPtr<ITaskBase> &pTask =
                         nullptr) override;

private:
  SharedPtr<IThreadLibTaskGroup> m_ndrangeChildrenTaskGroup;

  in_order_command_list(TBBTaskExecutor &pTBBExec,
                        const Intel::OpenCL::Utils::SharedPtr<TEDevice> &device,
                        const CommandListCreationParam &param,
                        bool bProfilingEnabled)
      : base_command_list(pTBBExec, device, param, bProfilingEnabled),
        m_ndrangeChildrenTaskGroup(TbbTaskGroup::Allocate()) {}
};

class out_of_order_command_list : public base_command_list {
public:
  PREPARE_SHARED_PTR(out_of_order_command_list)

  static SharedPtr<out_of_order_command_list>
  Allocate(TBBTaskExecutor &pTBBExec,
           const Intel::OpenCL::Utils::SharedPtr<TEDevice> &device,
           const CommandListCreationParam &param) {
    return new out_of_order_command_list(pTBBExec, device, param);
  }

  /**
   * Destructor
   */
  ~out_of_order_command_list();

  /**
   * Execute a functor for OOO execution (this method should be called only when
   * running inside the arena)
   * @param F the functor's type
   * @param f the functor's object
   */
  template <typename F> void ExecOOOFunc(const F &f) {
    m_oooTaskGroup->Spawn(f);
  }

  /**
   * Wait for all the enqueued commands to be completed
   */
  void WaitForAllCommands() { m_oooTaskGroup->WaitForAll(); }

  // overriden methods:

  bool IsMasterJoined() const override { return false; }
  void WaitForIdle() override;
  bool DoesSupportDeviceSideCommandEnqueue() const override { return true; }

  virtual SharedPtr<IThreadLibTaskGroup>
  GetNDRangeChildrenTaskGroup() override {
    return TbbTaskGroup::Allocate();
  }

  void Spawn(const Intel::OpenCL::Utils::SharedPtr<ITaskBase> &pTask,
             IThreadLibTaskGroup &taskGroup) override;

private:
  virtual unsigned int
  LaunchExecutorTask(bool blocking,
                     const Intel::OpenCL::Utils::SharedPtr<ITaskBase> &pTask =
                         nullptr) override;

  SharedPtr<SpawningTaskGroup> m_oooTaskGroup;

  out_of_order_command_list(
      TBBTaskExecutor &pTBBExec,
      const Intel::OpenCL::Utils::SharedPtr<TEDevice> &device,
      const CommandListCreationParam &param)
      : base_command_list(pTBBExec, device, param, param.isProfilingEnabled),
        m_oooTaskGroup(SpawningTaskGroup::Allocate(device.GetPtr())) {}
};

bool execute_command(const SharedPtr<ITaskBase> &pCmd,
                     base_command_list &cmdList);

struct ExecuteContainerBody {
  const SharedPtr<ITaskBase> m_pTask;
  base_command_list &m_list;

  ExecuteContainerBody(const SharedPtr<ITaskBase> &pTask,
                       base_command_list &list)
      : m_pTask(pTask), m_list(list) {}

  void operator()() const { execute_command(m_pTask, m_list); }
};

class immediate_command_list : public base_command_list {
public:
  PREPARE_SHARED_PTR(immediate_command_list)

  static SharedPtr<immediate_command_list>
  Allocate(TBBTaskExecutor &pTBBExec,
           const Intel::OpenCL::Utils::SharedPtr<TEDevice> &device,
           const CommandListCreationParam &param) {
    return new immediate_command_list(pTBBExec, device, param);
  }

  unsigned int
  Enqueue(const Intel::OpenCL::Utils::SharedPtr<ITaskBase> &pTask) override;

  te_wait_result WaitForCompletion(
      const Intel::OpenCL::Utils::SharedPtr<ITaskBase> & /*pTaskToWait*/)
      override {
    return TE_WAIT_NOT_SUPPORTED;
  }

  bool Flush() override { return true; }

  virtual SharedPtr<IThreadLibTaskGroup>
  GetNDRangeChildrenTaskGroup() override {
    return TbbTaskGroup::Allocate();
  }

  bool DoesSupportDeviceSideCommandEnqueue() const override { return false; }

  virtual bool IsProfilingEnabled() const override { return false; }

  bool IsMasterJoined() const override { return true; }

  virtual void Spawn(const Intel::OpenCL::Utils::SharedPtr<ITaskBase> &pTask,
                     IThreadLibTaskGroup &taskGroup) override;

protected:
  virtual unsigned int
  LaunchExecutorTask(bool blocking,
                     const Intel::OpenCL::Utils::SharedPtr<ITaskBase> &pTask =
                         nullptr) override;

private:
  tbb::affinity_partitioner m_ap;

  immediate_command_list(
      TBBTaskExecutor &pTBBExec,
      const Intel::OpenCL::Utils::SharedPtr<TEDevice> &device,
      const CommandListCreationParam &param)
      : base_command_list(pTBBExec, device, param) {}
};

} // namespace TaskExecutor
} // namespace OpenCL
} // namespace Intel
