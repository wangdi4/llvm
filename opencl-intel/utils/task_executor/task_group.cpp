// Copyright (c) 2006-2014 Intel Corporation
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

#ifdef _WIN32
#include <Windows.h>
#else
#include <sched.h>
#endif
#include "task_group.hpp"
#include "arena_handler.h"
#include "tbb_executor.h"
#include "cl_user_logger.h"

using namespace Intel::OpenCL::TaskExecutor;
using Intel::OpenCL::Utils::ApiLogger;

// TaskGroup methods:

void TaskGroup::WaitForAll()
{
    if (m_pRootTask->ref_count() > 1)
    {
        ArenaFunctorWaiter waiter(m_pRootTask);
        m_device->Execute(waiter);
        // at this point ref_count() might return again a value greater than 1, since another task might have already been enqueued after waiter was executed
    }
}

// SpawningTaskGroup methods

void SpawningTaskGroup::WaitForAll()
{
    if (!m_device->IsCurrentThreadInArena() || m_device->GetTaskExecutor().IsMaster())
    {
        ArenaFunctorWaiter func(m_pRootTask);
        m_device->Execute(func);    
    }
}
