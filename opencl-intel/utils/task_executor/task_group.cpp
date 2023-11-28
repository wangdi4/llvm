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

#include "task_group.hpp"

#include "arena_handler.h"
#include "cl_shared_ptr.hpp"
#include "tbb_executor.h"

using namespace Intel::OpenCL::TaskExecutor;

// TaskGroup methods:

void TaskGroup::WaitForAll() {
  if (ref_count() > 0) {
    ArenaFunctorWaiter waiter(this);
    m_device->Execute(waiter);
    // at this point ref_count() might return again a value greater than 1,
    // since another task might have already been enqueued after waiter was
    // executed
  }
}

// SpawningTaskGroup methods

void SpawningTaskGroup::WaitForAll() {
  if (!m_device->IsCurrentThreadInArena() ||
      m_device->GetTaskExecutor().IsMaster()) {
    ArenaFunctorWaiter waiter(this);
    m_device->Execute(waiter);
  }
}
