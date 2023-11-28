/*
    Copyright 2005 Intel Corporation.  All Rights Reserved.

    The source code contained or described herein and all documents related
    to the source code ("Material") are owned by Intel Corporation or its
    suppliers or licensors.  Title to the Material remains with Intel
    Corporation or its suppliers and licensors.  The Material is protected
    by worldwide copyright laws and treaty provisions.  No part of the
    Material may be used, copied, reproduced, modified, published, uploaded,
    posted, transmitted, distributed, or disclosed in any way without
    Intel's prior express written permission.

    No license under any patent, copyright, trade secret or other
    intellectual property right is granted to or conferred upon you by
    disclosure or delivery of the Materials, either expressly, by
    implication, inducement, estoppel or otherwise.  Any license under such
    intellectual property rights must be express and approved by Intel in
    writing.
*/

#ifndef HARNESS_TRAPPER_H
#include "spin_barrier.h"
#include "task_group_with_reference.h"
#include "tbb/task_arena.h"
#include "tbb/task_group.h"

#ifndef ASSERT
#define ASSERT __TBB_ASSERT
#define __TBB_UNDEF_ASSERT
#endif

namespace tbb {
namespace Harness {

class TbbWorkersTrapper {
  tbb::task_arena *m_arena;
  task_group_with_reference m_group;
  utils::SpinBarrier *my_barrier;

  class TrapperTaskRunner {
    task_group_with_reference *m_group;
    utils::SpinBarrier *m_barrier;

  public:
    TrapperTaskRunner(task_group_with_reference *group,
                      utils::SpinBarrier *barrier)
        : m_group(group), m_barrier(barrier) {}

    void operator()() const {
      m_barrier->wait(); // Wait until all workers are ready
      m_group->wait();
      m_barrier->signalNoWait();
    }
  };

  class TrapperReleaseRunner {
    utils::SpinBarrier *barrier;

  public:
    TrapperReleaseRunner(TbbWorkersTrapper &owner)
        : barrier(owner.my_barrier) {}
    void operator()() const {
      barrier->wait(); // Make sure no tasks are referencing us
      delete barrier;
    }
  };

  int num_threads;
  bool is_async;
  volatile bool is_trapped;

public:
  TbbWorkersTrapper(int _num_threads, bool _is_async, tbb::task_arena *arena)
      : m_arena(arena), num_threads(_num_threads), is_async(_is_async),
        is_trapped(false) {
    my_barrier = new utils::SpinBarrier;
    my_barrier->initialize(num_threads + (is_async ? 1 : 0));
  }

  ~TbbWorkersTrapper() {
    if (!is_trapped)
      return;
    m_group.release_wait();
    if (tbb::this_task_arena::current_thread_index() > 0) {
      // executing by a worker, so we must enqueue a task that will destroy the
      // barrier
      TrapperReleaseRunner f(*this);
      m_arena->enqueue(f);
      return;
    } else {
      my_barrier->wait(); // Make sure no tasks are referencing us
      delete my_barrier;
    }
  }

  // Delete copy & move constructor
  TbbWorkersTrapper(const TbbWorkersTrapper &) = delete;
  TbbWorkersTrapper(TbbWorkersTrapper &&) = delete;

  // Delete assignment operator
  TbbWorkersTrapper &operator=(const TbbWorkersTrapper &) = delete;
  TbbWorkersTrapper &operator=(TbbWorkersTrapper &&) = delete;

  bool IsTrapped() const { return is_trapped; }

  int GetTrappedThreadCount() const { return num_threads; }

  void Wait() { my_barrier->wait(); }

  void operator()(void) {
    assert((tbb::this_task_arena::current_thread_index() == 0 || is_async) &&
           "Trapper must be executed from the master slot or be async");
    m_group.reserve_wait();
    for (int i = 1; i < num_threads; ++i) {
      TrapperTaskRunner f(&m_group, my_barrier);
      m_group.run(f);
    }

    my_barrier->wait(); // Wait until all workers are ready
    is_trapped = true;
    // For async we need to trap this task as well
    if (is_async) {
      m_group.wait();
      my_barrier->wait();
    }
  }

}; // TbbWorkersTrapper

} // namespace Harness
} // namespace tbb

#ifdef __TBB_UNDEF_ASSERT
#undef ASSERT
#endif

#endif // #ifndef HARNESS_TRAPPER_H
