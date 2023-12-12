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
#include "task_executor.h"
#include "task_group_with_reference.h"
#include "tbb/task_group.h"

extern thread_local bool WaitLock;

namespace Intel {
namespace OpenCL {
namespace TaskExecutor {

class TEDevice;

/**
 * Base class of all clases that implement ITaskGroup
 */
class TaskGroupBase : public ITaskGroup, public task_group_with_reference {
public:
  /**
   * Destructor
   */
  virtual ~TaskGroupBase() {}

protected:
  /**
   * the TEDevice for enqueuing and executing tasks
   */
  TEDevice *m_device;

  /**
   * @param device the TEDevice for enqueuing and executing tasks
   */
  explicit TaskGroupBase(TEDevice *device) : m_device(device) {}
};

/**
 * This class represents a group of tasks for which one can wait to their
 * completion. It replaces tbb::task_group, which can't be used the way we need:
 * parallel_for from one task_group might steal the task which starts another
 * parallel_for.
 */
class TaskGroup : public TaskGroupBase {
public:
  PREPARE_SHARED_PTR(TaskGroup)

  /**
   * @param device the TEDevice for enqueuing and executing tasks
   * @return a new TaskGroup
   */
  static Intel::OpenCL::Utils::SharedPtr<TaskGroup> Allocate(TEDevice *device) {
    return new TaskGroup(device);
  }

  /**
   * Enqueue a functor.
   * @param f the functor object.
   */
  template <typename F> void EnqueueFunc(const F &f);

  /**
   * Execute a functor.
   * @param f the functor object.
   */
  template <typename F> void ExecuteFunc(const F &f);

  /**
   * Wait for all tasks.
   */
  void WaitForAll() override;

private:
  TaskGroup(TEDevice *device) : TaskGroupBase(device) {}
};

/// auxiliary functor classes to be replaced by lambda functions
class ArenaFunctor {
protected:
  ArenaFunctor(task_group_with_reference *taskGroup) : m_taskGroup(taskGroup) {}

  task_group_with_reference *m_taskGroup;
};

/// Run task_group::wait inside task_arena.
class ArenaFunctorWaiter : public ArenaFunctor {
public:
  ArenaFunctorWaiter(task_group_with_reference *taskGroup)
      : ArenaFunctor(taskGroup) {}

  void operator()() const {
    // joins the work.
    m_taskGroup->wait();
  }
};

/// Run a task inside task_arena
template <typename F> class ArenaFunctorRunner : public ArenaFunctor {
public:
  ArenaFunctorRunner<F>(task_group_with_reference *taskGroup, const F &func)
      : ArenaFunctor(taskGroup), m_func(func) {}

  void operator()() const {
    m_func();
    m_taskGroup->release_wait();
  }

protected:
  F m_func;
};

/**
 * Implements ITaskGroup and executing a functor by wrapping it with a task that
 * is run in task_group. This class is used for grouping tasks executing the
 * commands themselves in OOO command list.
 */
class SpawningTaskGroup : public TaskGroupBase {
public:
  PREPARE_SHARED_PTR(SpawningTaskGroup)

  /**
   * @param pDevice the TEDevice on which that tasks are executed.
   * @return a new SpawningTaskGroup.
   */
  static Intel::OpenCL::Utils::SharedPtr<SpawningTaskGroup>
  Allocate(TEDevice *pDevice) {
    return new SpawningTaskGroup(pDevice);
  }

  /**
   * Spawn a task executing a functor.
   * @param F the type of the functor.
   * @param f the functor to be execution.
   */
  template <typename F> void Spawn(const F &f);

  virtual void WaitForAll() override;

  ~SpawningTaskGroup() {
    if (WaitLock) {
      assert(ref_count() == 1 &&
             "The wait context of SpawningTaskGroup should be 1");
      release_wait();
    }
  }

private:
  SpawningTaskGroup(TEDevice *pDevice) : TaskGroupBase(pDevice) {}
};

} // namespace TaskExecutor
} // namespace OpenCL
} // namespace Intel
