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

#include "arena_handler.h"
#include "task_group.h"

namespace Intel {
namespace OpenCL {
namespace TaskExecutor {

// TaskGroup methods:

template <typename F> void TaskGroup::EnqueueFunc(const F &f) {
  // Increment the reference count here. It will be decremented inside
  // runner's function after f() has been called.
  reserve_wait();
  ArenaFunctorRunner<F> runner(this, f);
  m_device->Enqueue(runner);
}

template <typename F> void TaskGroup::ExecuteFunc(const F &f) {
  reserve_wait();
  ArenaFunctorRunner<F> runner(this, f);
  m_device->Execute(runner);
}

// SpawningTaskGroup methods:

template <typename F> void SpawningTaskGroup::Spawn(const F &f) { run(f); }

} // namespace TaskExecutor
} // namespace OpenCL
} // namespace Intel
