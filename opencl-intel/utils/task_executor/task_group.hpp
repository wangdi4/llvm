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

#include "task_group.h"
#include "arena_handler.h"

namespace Intel { namespace OpenCL { namespace TaskExecutor {

// TaskGroup methods:

template<typename F>
void TaskGroup::EnqueueFunc(const F& f)
{
    m_pRootTask->increment_ref_count();   // Increment the reference count here. It will be decremented inside runner's function after f() has been called.
    ArenaFunctorRunner<F> runner(m_pRootTask, f);
    m_device->Enqueue(runner);    
}

// SpawningTaskGroup methods:

template<typename F>
void SpawningTaskGroup::Spawn(const F& f)
{
    tbb::task* t = new(tbb::task::allocate_additional_child_of(*m_pRootTask)) TaskGroupTask<F>(f);
    tbb::task::spawn(*t);
}

}}}
