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

#include <algorithm>
#include "arena_handler.h"
#include "tbb_executor.h"
#include "base_command_list.h"
#include "cl_sys_info.h"
#include "cl_shared_ptr.hpp"

namespace Intel { namespace OpenCL { namespace TaskExecutor {

using namespace Intel::OpenCL::Utils;

// DevArenaObserver's methods:

void DevArenaObserver::on_scheduler_entry(bool bIsWorker)
{
    /* If the user enqueues a command and then exits the program without having waited for the command to finish, we might get here while things are being folded beneath our feet. We have no way to
       protect ourselves, except by raising a flag in a function registered by atexit. This isn't a perfect protection, since exit can be called while the worker thread is in this method after having
       already checked the flag, but it significantly reduces the probability for it. This also applies to the same flag checking in other methods. */
    if (gIsExiting)
    {
        return;
    }
    const int iCurSlot = tbb::task_arena::current_slot();
    assert(iCurSlot >= 0);    
    WGContextBase* const pOldWgContext = m_pArenaHandler->GetWGContext(iCurSlot);
    if (NULL == pOldWgContext || (pOldWgContext != NULL && pOldWgContext->DoesBelongToMasterThread() == bIsWorker))
    {
        if (pOldWgContext != NULL && pOldWgContext->DoesBelongToMasterThread() == bIsWorker)
        {
            m_taskExecutor.ReleaseWorkerWGContext(pOldWgContext);
        }
        WGContextBase* const pNewWgContext = m_taskExecutor.GetWGContext(!bIsWorker);
        if (pNewWgContext != NULL)
        {
            m_pArenaHandler->SetWGContext(iCurSlot, pNewWgContext);
            if (m_pArenaHandler->GetNumSubdevComputeUnits() > 1)
            {
                pNewWgContext->SetThreadId(iCurSlot);
            }
            else
            {
                pNewWgContext->SetThreadId(0); // an arena of sub-device of size 1 has 2 slots, but just one is used
            }
        }
    }
}

void DevArenaObserver::on_scheduler_exit(bool bIsWorker)
{
    if (gIsExiting)
    {
        return;
    }
    const int iCurSlot = tbb::task_arena::current_slot();
    assert(iCurSlot >= 0);
    WGContextBase* const pWgContext = m_pArenaHandler->GetWGContext(iCurSlot);
    if (NULL != pWgContext)
    {
        m_taskExecutor.ReleaseWorkerWGContext(pWgContext);        
        m_pArenaHandler->SetWGContext(iCurSlot, NULL);
    }    
}

bool DevArenaObserver::on_scheduler_leaving()
{
    return !m_pArenaHandler->AreEnqueuedTasks();
}

// SubdevArenaObserver's methods:

SubdevArenaObserver::SubdevArenaObserver(tbb::task_arena& arena, TBBTaskExecutor& taskExecutor, const unsigned int* pLegalCores, size_t szNumlegalCores, IAffinityChangeObserver& observer) :
    DevArenaObserver(arena, taskExecutor), m_observer(observer)
{
    for (size_t i = 0; i < szNumlegalCores; i++)
    {
        m_legalCores.insert(pLegalCores[i]);
    }
}

void SubdevArenaObserver::on_scheduler_entry(bool bIsWorker)
{
    if (gIsExiting)
    {
        return;
    }
    DevArenaObserver::on_scheduler_entry(bIsWorker);
    if (bIsWorker)
    {
        OclAutoMutex autoMutex(&m_mutex);
        const int iCurSlot = tbb::task_arena::current_slot();
        assert(iCurSlot >= 0);
        assert(m_legalCores.size() > 0);
        const unsigned int uiCoreId = *m_legalCores.begin();
        m_legalCores.erase(uiCoreId);
        m_slots2Cores[iCurSlot] = uiCoreId;
        m_observer.NotifyAffinity(iCurSlot, uiCoreId);
    }
}

void SubdevArenaObserver::on_scheduler_exit(bool bIsWorker)
{
    if (gIsExiting)
    {
        return;
    }
    DevArenaObserver::on_scheduler_exit(bIsWorker);
    if (bIsWorker)
    {
        OclAutoMutex autoMutex(&m_mutex);
        const int iCurSlot = tbb::task_arena::current_slot();
        assert(m_slots2Cores.find(iCurSlot) != m_slots2Cores.end());
        m_legalCores.insert(m_slots2Cores[iCurSlot]);
        m_slots2Cores.erase(iCurSlot);
    }
}

// ArenaHandler's methods:

ArenaHandler::ArenaHandler(unsigned int uiNumComputeUnits, unsigned int uiNumTotalComputeUnits, TBBTaskExecutor& taskExecutor) :
// the global scheduler is initialized with P+1 threads because of a bug in TBB (see TBBTaskExecutor::m_pScheduler)
m_wgContexts(uiNumTotalComputeUnits + 1), m_taskExecutor(taskExecutor), m_uiNumSubdevComputeUnits(uiNumComputeUnits)
{
    for (std::vector<WGContextBase*>::iterator iter = m_wgContexts.begin(); iter != m_wgContexts.end(); iter++)
    {
        *iter = NULL;
    }
    /* If a TBB arena is created with maximum concurrency P, it will have P+1 slots, since one slot is reserved for the master thread. However, the steady state of the arena is that the
       maximum number of threads in it is P, because if a master joins the arena, a worker is forced to leave it after having finished its current task. However, there may be a period of time
       between the moment the master joins the arena and a worker leaves it that there are P+1 threads in the arena. In the future this process may be synchronized, so that no such period will
       exist.
       This isn't true in case the sub-device includes all the compute units in the system, because here there is no danger that P+1 threads would run simultenously.
       This is why if numComputeUnits is greater than 1, we pass numComputeUnits-1 as maximum concurrency and if it is 1, we pass 1. In case of a sub-device with size 1, we accept that most of
       the time just one thread will run in the arena, but there might be some periods of time where the work will overflow to another thread. Otherwise we will not be able to create such a
       sub-device. */
    if (uiNumComputeUnits > 1 && uiNumComputeUnits < uiNumTotalComputeUnits)
    {
        m_arena.initialize(uiNumComputeUnits - 1);  // apparently task_arena(int) doesn't do all the required initialization...
    }
    else
    {
        m_arena.initialize(uiNumComputeUnits);
    }
}

void ArenaHandler::Init(DevArenaObserver* pArenaObserver)
{
    m_pArenaObserver = pArenaObserver;
    m_pArenaObserver->SetArenaHandler(this);
    m_pArenaObserver->observe(true);
}

ArenaHandler::~ArenaHandler()
{
    m_isTerminating = true;
    /* This destructor is called in the finalization flow of task_executor, so it's not safe to call m_taskExecutor.GetWGContextPool(), which itself calls the device agent. This is why I don't
       take care for releasing the WG contexts in m_wgContexts. */
    m_cmdLists.clear();
}

void ArenaHandler::AddCommandList(const SharedPtr<base_command_list>& pCmdList)
{
    OclAutoMutex mu(&m_mutex);
    const std::pair<std::set<SharedPtr<base_command_list> >::iterator, bool> res = m_cmdLists.insert(pCmdList);
    assert(res.second);
}

void ArenaHandler::RemoveCommandList(const base_command_list* pCmdList)
{
    OclAutoMutex mu(&m_mutex);
    /* We can get here in from ~ArenaHandler(). In this case pCmdList won't find itself in the set anymore, so erase will return 0, but that's OK. We use a regular pointer rather than SharedPtr to prevent an infinite
       recursion of ~base_commad_list(). Try it and see what happens :) */
    for (std::set<SharedPtr<base_command_list> >::iterator iter = m_cmdLists.begin(); iter != m_cmdLists.end(); iter++)
    {
        if (iter->GetPtr() == pCmdList)
        {
            m_cmdLists.erase(iter);
            break;
        }
    }
}

WGContextBase* ArenaHandler::GetWGContext()
{
    if (gIsExiting)
    {
        return NULL;
    }
    const int iCurSlot = tbb::task_arena::current_slot();
    assert(iCurSlot >= 0 && iCurSlot < (int)m_wgContexts.size());
    if (NULL == m_wgContexts[iCurSlot])
    {        
        m_wgContexts[iCurSlot] = m_taskExecutor.GetWGContext(false);
    }
    return m_wgContexts[iCurSlot];
}

void ArenaHandler::WaitUntilEmpty()
{
    OclAutoMutex mu(&m_mutex);
    // Calling m_arena.wait_until_empty() is dangerous, because waiting from a worker thread that belongs to the arena causes a deadlock. Instead we wait on all the task_group, which is safe.
    for (std::set<SharedPtr<base_command_list> >::iterator iter = m_cmdLists.begin(); iter != m_cmdLists.end(); iter++)
    {
        (*iter)->Wait();
    }
}

// SubdevArenaHandler's methods:
SubdevArenaHandler::SubdevArenaHandler(unsigned int uiNumSubdevComputeUnits, unsigned int uiNumTotalComputeUnits, TBBTaskExecutor& taskExecutor, const unsigned int* pLegalCores,
    IAffinityChangeObserver& observer) :
    ArenaHandler(uiNumSubdevComputeUnits, uiNumTotalComputeUnits, taskExecutor), m_subdevArenaObserver(m_arena, taskExecutor, pLegalCores, uiNumSubdevComputeUnits, observer)    
{    
    m_pInternalCmdList = in_order_command_list::Allocate(true, &taskExecutor, *this);
    Init(&m_subdevArenaObserver);
}

}}}
