// Copyright (c) 2006-2012 Intel Corporation
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

#include "base_command_list.h"
#include "arena_handler.h"

namespace Intel { namespace OpenCL { namespace TaskExecutor {

template<typename F>
void TaskGroup::EnqueueFunc(const F& f)
{
    m_rootTask.increment_ref_count();   // Increment the reference count here. It will be decremented inside runner's function after f() has been called.
    ArenaFunctorRunner<F> runner(m_rootTask, f); 
    m_device.Enqueue(runner);
}

#ifdef __HARD_TRAPPING__
// Enqueue into device through execute function
// Required for trapped devices
template<typename F>
void TaskGroup::EnqueueFuncEx(const F& f)
{
    //m_rootTask.increment_ref_count();   // Increment the reference count here. It will be decremented inside runner's function after f() has been called.
    ArenaFunctorSpawner<F> spawner(m_rootTask, f);
    m_device.Execute(spawner);
    //printf("EnqueueFuncEx - root %p ref_cnt=%d, f=%p\n", (void*)&m_rootTask, m_rootTask.ref_count(), (void*)&f);fflush(0);
}
#endif

}}}
