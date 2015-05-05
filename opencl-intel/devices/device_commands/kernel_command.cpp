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

#include "kernel_command.h"
#include "cl_shared_ptr.hpp"

using namespace Intel::OpenCL::DeviceCommands;
using Intel::OpenCL::Utils::OclAutoMutex;
using Intel::OpenCL::TaskExecutor::IThreadLibTaskGroup;

cl_dev_err_code KernelCommand::AddChildKernelToLists(const SharedPtr<KernelCommand>& child, kernel_enqueue_flags_t flags, CommandSubmitionLists* pChildLists)
{
	ASSERT_RET_VAL(CLK_ENQUEUE_FLAGS_NO_WAIT == flags || CLK_ENQUEUE_FLAGS_WAIT_KERNEL == flags || CLK_ENQUEUE_FLAGS_WAIT_WORK_GROUP == flags,
		"CLK_ENQUEUE_FLAGS_NO_WAIT == flags || CLK_ENQUEUE_FLAGS_WAIT_KERNEL == flags || CLK_ENQUEUE_FLAGS_WAIT_WORK_GROUP == flags",
        CL_DEV_ERROR_FAIL);

    // TODO: Use tbb::scalable_allocator
    CommandToExecuteList_t* pNewKernel = new CommandToExecuteList_t; 
    if ( NULL == pNewKernel )
    {
        return CL_DEV_OUT_OF_MEMORY;
    }
	child->AddDependency();
    pNewKernel->command = child;

    switch ( flags ) {
    case CLK_ENQUEUE_FLAGS_WAIT_KERNEL:
		// we'll move the children in this list to m_waitingChildrenForKernel when the work-group finishes execution - this saves us a lot of contention
        // Add kernels to the head, saves one memory write
        pNewKernel->next = pChildLists->waitingChildrenForKernelLocalHead;
        pChildLists->waitingChildrenForKernelLocalHead = pNewKernel;
        if ( NULL == pChildLists->waitingChildrenForKernelLocalTail )
        {
            pChildLists->waitingChildrenForKernelLocalTail = pNewKernel;
            pNewKernel->next = NULL;
        }
        break;
    case CLK_ENQUEUE_FLAGS_WAIT_WORK_GROUP:
        pNewKernel->next = pChildLists->waitingChildrenForWorkGroup;
        pChildLists->waitingChildrenForWorkGroup = pNewKernel;
        break;
    default:
        assert(false && "invalid flag");
        return CL_DEV_INVALID_VALUE;
	}

    return CL_DEV_SUCCESS;
}

void KernelCommand::WaitForChildrenCompletion()
{ 
    while ( NULL != m_waitingChildrenForKernelGlobal )
    {
        CommandToExecuteList_t* pNextCommand = reinterpret_cast<CommandToExecuteList_t*>(m_waitingChildrenForKernelGlobal->next);
        m_waitingChildrenForKernelGlobal->command->NotifyCommandFinished(GetError());
        // Use to tbb::allocatore here
        delete m_waitingChildrenForKernelGlobal;
        m_waitingChildrenForKernelGlobal = pNextCommand;
    }
#ifdef _DEBUG
    IThreadLibTaskGroup::TaskGroupStatus status = 
#endif
    m_childrenTaskGroup->Wait();
#ifdef _DEBUG
    assert(IThreadLibTaskGroup::COMPLETE == status && "IThreadLibTaskGroup::COMPLETE != status");
#endif
}

void KernelCommand::SubmitCommands(CommandSubmitionLists* pNewCommands)
{
	// Submit children waiting to work-group completiom
    while ( NULL != pNewCommands->waitingChildrenForWorkGroup )
    {
        CommandToExecuteList_t* pNextCommand = reinterpret_cast<CommandToExecuteList_t*>(pNewCommands->waitingChildrenForWorkGroup->next);
        pNewCommands->waitingChildrenForWorkGroup->command->NotifyCommandFinished(GetError());
        // Use to tbb::allocatore here
        delete pNewCommands->waitingChildrenForWorkGroup;
        pNewCommands->waitingChildrenForWorkGroup = pNextCommand;
    }

    // move waiting children for parent from WG list to the global list
	if ( NULL != pNewCommands->waitingChildrenForKernelLocalHead )
	{
        // Make current local list as kernel global list
        CommandToExecuteList_t* prev_start = m_waitingChildrenForKernelGlobal.exchange(pNewCommands->waitingChildrenForKernelLocalHead);
        // Move previous global list to the end of current local list
        pNewCommands->waitingChildrenForKernelLocalTail->next = prev_start;
		// Reset local list
		pNewCommands->waitingChildrenForKernelLocalHead = NULL;
		pNewCommands->waitingChildrenForKernelLocalTail = NULL;
	}
}

void KernelCommand::Launch()
{
    assert(GetParentTaskGroup() != NULL && "Launch shouldn't be called for KernelCommands enqueued from the host");
    GetList()->Spawn(GetMyTaskBase(), *GetParentTaskGroup());
}

int KernelCommand::EnqueueKernel(queue_t queue, kernel_enqueue_flags_t flags, cl_uint uiNumEventsInWaitList, const clk_event_t* pEventWaitList, clk_event_t* pEventRet,
		const Intel::OpenCL::DeviceBackend::ICLDevBackendKernel_* pKernel,
        const void* pBlockLiteral, size_t stBlockSize, const size_t* pLocalSizes, size_t stLocalSizeCount,
        const _ndrange_t* pNDRange, const void* pHandle)
{    
    // verify parameterss
    ASSERT_RET_VAL(pKernel != NULL, "Trying to enqueue with NULL kernel", CL_INVALID_KERNEL);
    if (NULL == queue || (NULL == pEventWaitList && uiNumEventsInWaitList > 0) || (NULL != pEventWaitList && 0 == uiNumEventsInWaitList) ||
        (flags != CLK_ENQUEUE_FLAGS_NO_WAIT && flags != CLK_ENQUEUE_FLAGS_WAIT_KERNEL && flags != CLK_ENQUEUE_FLAGS_WAIT_WORK_GROUP))
    {
        return CL_ENQUEUE_FAILURE;
    }
        
    ITaskList* pList = (ITaskList*)queue;
    if (!pList->DoesSupportDeviceSideCommandEnqueue())
    {
        return CL_INVALID_OPERATION;
    }

    if(!pKernel->GetKernelProporties()->IsNonUniformWGSizeSupported()) {
      // Check for non-uniform work-group size in all dimensions.
      for(unsigned int dim = 0; dim < pNDRange->workDimension; ++dim) {
        if(pNDRange->localWorkSize[dim] != 0 && pNDRange->globalWorkSize[dim] % pNDRange->localWorkSize[dim] != 0) { // Dimension with non-uniform work-group size detected.
          // TODO: return CLK_INVALID_NDRANGE if the -g compiler option was specified.
          return CL_ENQUEUE_FAILURE;
        }
      }
    }
    // debug mode: make sure all child kernels start after the parent ends its execution and then run in-order    
    ITaskList* const pDebugList = pList->GetDebugInOrderDeviceQueue();
    assert(pDebugList != NULL);
    m_bIsDebugMode = pKernel->GetKernelProporties()->HasDebugInfo();
    if (m_bIsDebugMode)
    {
        // indeed BE is supposed to force this flag, but we force it ourselves just to make sure
        flags = CLK_ENQUEUE_FLAGS_WAIT_KERNEL;
        m_childrenTaskGroup = pDebugList->GetNDRangeChildrenTaskGroup();
    }
    
    SharedPtr<KernelCommand> pChild;
#if __USE_TBB_ALLOCATOR__
        DeviceNDRange* const pChildAddress = m_deviceNDRangeAllocator.allocate(sizeof(DeviceNDRange));    // currently we ignore bad_alloc
        pChild = ::new(pChildAddress) DeviceNDRange(m_pTaskDispatcher, pList, pParent, pKernel, pContext, szContextSize, pNdrange, m_deviceNDRangeAllocator, m_deviceNDRangeContextAllocator);
#else
        pChild = AllocateChildCommand(m_bIsDebugMode ? pDebugList : pList, pKernel, pBlockLiteral, stBlockSize, pLocalSizes, stLocalSizeCount, pNDRange);
#endif
    const bool bAllEventsCompleted = pChild->AddWaitListDependencies(pEventWaitList, uiNumEventsInWaitList);    

    // if no need to wait, enqueue and flush
    if (CLK_ENQUEUE_FLAGS_NO_WAIT == flags )
    {
        if ( bAllEventsCompleted )
        {
            pChild->Launch();
        }
    }
    else
    {
        cl_dev_err_code err = AddChildKernelToLists(pChild, flags, (CommandSubmitionLists*)pHandle);
        if ( CL_DEV_FAILED(err) )
        {
            return CL_INVALID_ARG_VALUE;
        }
    }

    // update pEventRet
    if (pEventRet != NULL)
    {
        *pEventRet = pChild.GetPtr();
        pChild.IncRefCnt();    // it will be decremented in release_event
    }
    return CL_SUCCESS;
}

int KernelCommand::EnqueueMarker(queue_t queue, cl_uint uiNumEventsInWaitList, const clk_event_t* pEventWaitList, clk_event_t* pEventRet)
{
    if (NULL == queue || NULL == pEventWaitList || 0 == uiNumEventsInWaitList)
    {
        return CL_ENQUEUE_FAILURE;
    }

    ITaskList* pList = (ITaskList*)queue;
    if (!pList->DoesSupportDeviceSideCommandEnqueue())
    {
        return CL_INVALID_OPERATION;
    }
    SharedPtr<Marker> marker = Marker::Allocate(pList);    // should we use a pool here too?
    const bool bAllEventsCompleted = marker->AddWaitListDependencies(pEventWaitList, uiNumEventsInWaitList);
    if (bAllEventsCompleted)
    {
        marker->Launch();
    }
    if (pEventRet != NULL)
    {
        *pEventRet = marker.GetPtr();
        marker.IncRefCnt();    // it will decremeneted in release_event
    }
    return CL_SUCCESS;
}

int KernelCommand::RetainEvent(clk_event_t event)
{
    if (NULL == event)
    {
        return CL_INVALID_EVENT;
    }
    DeviceCommand* pCmd = (DeviceCommand*)event;
    pCmd->IncRefCnt();
    return CL_SUCCESS;
}

int KernelCommand::ReleaseEvent(clk_event_t event)
{
    if (NULL == event)
    {
        return CL_INVALID_EVENT;
    }
    DeviceCommand* pEvent = reinterpret_cast<DeviceCommand*>(event);
    long val = pEvent->DecRefCnt();
    if ( 0 == val )
    {
        pEvent->Cleanup();
    }
    return CL_SUCCESS;
}

clk_event_t KernelCommand::CreateUserEvent(int* piErrcodeRet)
{
    UserEvent* pUserEvent = UserEvent::Allocate();
    if (NULL != pUserEvent)
    {
        pUserEvent->IncRefCnt();
    }
    if (NULL != piErrcodeRet)
    {
       *piErrcodeRet = (NULL != pUserEvent) ? CL_SUCCESS : CL_EVENT_ALLOCATION_FAILURE;
    }
    return (clk_event_t)(pUserEvent);
}
    
int KernelCommand::SetEventStatus(clk_event_t event, int iStatus)
{
    UserEvent* pEvent = reinterpret_cast<UserEvent*>(event);
    if ( (NULL == pEvent) || !pEvent->IsUserCommand() )
    {
        return CL_INVALID_EVENT;
    }
    if ( (CL_COMPLETE!=iStatus) && (iStatus>= 0) )
    {
        return CL_INVALID_VALUE;
    }

    pEvent->SetStatus(iStatus);
    return CL_SUCCESS;
}

void KernelCommand::CaptureEventProfilingInfo(clk_event_t event, clk_profiling_info name, volatile void* pValue)
{
    if (NULL == event || name != CLK_PROFILING_COMMAND_EXEC_TIME || NULL == pValue)
    {
        return;
    }    

    DeviceCommand* pCmd = reinterpret_cast<DeviceCommand*>(event);
    const SharedPtr<ITaskList>& pList = pCmd->GetList();
    if ( ((NULL!=pList)  && !pList->IsProfilingEnabled()) || pCmd->IsUserCommand() )
    {
        return;
    }
    if (!pCmd->SetExecTimeUserPtr(pValue))    // otherwise the information will be available when the command completes
    {
        ((cl_ulong*)pValue)[0] = pCmd->GetExecutionTime();
        ((cl_ulong*)pValue)[1] = pCmd->GetCompleteTime();
    }        
}

queue_t KernelCommand::GetDefaultQueueForDevice() const
{
    // Redirect to parenet
    if ( NULL == m_parent )
    {
        return NULL;
    }
    return m_parent->GetDefaultQueueForDevice();
}

bool KernelCommand::IsValidEvent(clk_event_t event) const
{
    DeviceCommand* pEvent = reinterpret_cast<DeviceCommand*>(event);
    if (NULL == pEvent)
    {
        return false;
    }

    if (!pEvent->CheckStamp())
    {
        return false;
    }

    if (pEvent->GetRefCnt() < 1)
    {
        return false;
    }
    return true;
}