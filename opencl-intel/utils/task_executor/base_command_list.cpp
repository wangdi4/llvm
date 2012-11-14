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

#include "arena_handler.h"
#include "base_command_list.hpp"
#include "cl_shared_ptr.hpp"

using namespace Intel::OpenCL::TaskExecutor;

base_command_list::base_command_list(bool subdevice, TBBTaskExecutor* pTBBExec, ArenaHandler& devArenaHandler) :
    m_pTBBExecutor(pTBBExec), m_pMasterSync(SyncTask::Allocate()), m_devArenaHandler(devArenaHandler)
{
	m_execTaskRequests = 0;
	m_bMasterRunning = false;
    m_devArenaHandler.AddCommandList(this);
}

base_command_list::~base_command_list()
{
    Wait();
    m_devArenaHandler.RemoveCommandList(this);
}

te_wait_result base_command_list::WaitForCompletion(const SharedPtr<ITaskBase>& pTaskToWait)
{
    if (m_devArenaHandler.GetNumSubdevComputeUnits() == 1)
    {
        return TE_WAIT_NOT_SUPPORTED;   // master thread can't join a sub-device with size 1
    }
	// Request processing task to stop
	if ( NULL != pTaskToWait )
	{
		bool completed = pTaskToWait->SetAsSyncPoint();
		// If already completed no need to wait
		if ( completed )
		{
			return TE_WAIT_COMPLETED;
		}
	}	

	bool bVal = m_bMasterRunning.compare_and_swap(true, false);
	if (bVal)
	{
		// When another master is running we can't block this thread
		return TE_WAIT_MASTER_THREAD_BLOCKING;
	}

	if ( NULL == pTaskToWait )
	{
		m_pMasterSync->Reset();
		Enqueue(m_pMasterSync);
	}

	do
	{
		unsigned int ret = InternalFlush(true);
		if ( ret > 0)
		{
			// Someone else started the task, need to wait
            Wait();
		}
	} while ( !(m_pMasterSync->IsCompleted() || ((NULL != pTaskToWait) && (pTaskToWait->IsCompleted()))) );
		
	// Current master is not in charge for the work
	m_bMasterRunning = false;

	// If there's some incoming work during operation, try to flush it
	if ( HaveIncomingWork() )
	{
		Flush();
	}

	return TE_WAIT_COMPLETED;
}

inline unsigned int base_command_list::InternalFlush(bool blocking)
{    
	unsigned int runningTaskRequests = m_execTaskRequests++;    
	if ( 0 == runningTaskRequests )
	{
		//We need to launch the task to handle the existing input
		return LaunchExecutorTask(blocking);
	}
	//Otherwise, the task is already running and will see our input in the next epoch
	return runningTaskRequests;
}

bool base_command_list::Flush()
{
	InternalFlush(false);
	return true;
}

void base_command_list::Wait()
{
	TaskGroupWaiter functor(m_taskGroup);
	m_devArenaHandler.Execute(functor);
}
