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

#pragma once

#include "command.h"
#include <IDeviceCommandManager.h>
#include <cl_dev_backend_api.h>

#include <cl_sys_defines.h>

namespace Intel { namespace OpenCL { namespace DeviceCommands {

/**
 * This class represents an ND-range kernel command enqueued either by the host or a parent kernel
 */
class KernelCommand : public DeviceCommand, public IDeviceCommandManager
{
public:
    PREPARE_SHARED_PTR(KernelCommand)

    /**
     * A child kernel has completed
     * @param err the child's error code
     */
    void ChildCompleted(cl_dev_err_code err)
    {
        if (CL_DEV_FAILED(err))
        {
          SetError(err);
        }
    }

    /**
     * @return ITaskGroup of the KernelCommand's parent (used to let the parent wait for its children)
     */
    SharedPtr<Intel::OpenCL::TaskExecutor::IThreadLibTaskGroup> GetParentTaskGroup()
    {
      if (m_parent != NULL)
      {
        return m_parent->m_childrenTaskGroup;
      }
      return NULL;
    }

    virtual KernelCommand* AllocateChildCommand(ITaskList* pList, const Intel::OpenCL::DeviceBackend::ICLDevBackendKernel_* pKernel,
        const void* pBlockLiteral, size_t stBlockSize, const size_t* pLocalSizes, size_t stLocalSizeCount,
        const _ndrange_t* pNDRange) const = 0;

    // inherited methods:
    virtual void Launch();

    // IDeviceCommandManager interface
    int EnqueueKernel(queue_t queue, kernel_enqueue_flags_t flags, cl_uint uiNumEventsInWaitList, const clk_event_t* pEventWaitList, clk_event_t* pEventRet,
		const Intel::OpenCL::DeviceBackend::ICLDevBackendKernel_* pKernel,
        const void* pBlockLiteral, size_t stBlockSize, const size_t* pLocalSize, size_t stLocalSizeCount,
        const _ndrange_t* pNDRange, const void* pHandle);

    int EnqueueMarker(queue_t queue, cl_uint uiNumEventsInWaitList, const clk_event_t* pEventWaitList, clk_event_t* pEventRet);

    int RetainEvent(clk_event_t event);

    int ReleaseEvent(clk_event_t event);

    clk_event_t CreateUserEvent(int* piErrcodeRet);

    virtual bool IsValidEvent(clk_event_t event) const;

    int SetEventStatus(clk_event_t event, int iStatus);

    void CaptureEventProfilingInfo(clk_event_t event, clk_profiling_info name, volatile void* pValue);

    queue_t GetDefaultQueueForDevice() const;

protected:

    /**
     * Constructor
     * @param list				the ITaskList that implements the command-queue on which this DeviceCommand executes
     * @param parent			the parent KernelCommand or NULL if this KernelCommand has been enqueued from the host
     * @param pMyTaskBase		a pointer to itself as ITaskBase or NULL if the concrete class does not inherit from ITaskBase
     */
    KernelCommand(ITaskList* pList, KernelCommand* parent, ITaskBase* pMyTaskBase) :
         DeviceCommand(pList, pMyTaskBase), m_parent(parent), m_childrenTaskGroup(NULL != pList ? pList->GetNDRangeChildrenTaskGroup().GetPtr() : NULL),
         m_bIsDebugMode(false)
       { m_waitingChildrenForKernelGlobal = NULL; }

    // Enqueued command list definition
    struct CommandToExecuteList_t {
        SharedPtr<KernelCommand> command;
        CommandToExecuteList_t*  next;
    };

    // Structure defined child kernel submission for each thread
    struct CommandSubmitionLists {
        CommandSubmitionLists() : waitingChildrenForKernelLocalHead(NULL), waitingChildrenForKernelLocalTail(NULL), waitingChildrenForWorkGroup(NULL) {}
        CommandToExecuteList_t*  waitingChildrenForKernelLocalHead;   // A pointer to the head of the thread local list of kernel depentent children
        CommandToExecuteList_t*  waitingChildrenForKernelLocalTail;   // A pointer to the tail of the thread local list of kernel depentent children
        CommandToExecuteList_t*  waitingChildrenForWorkGroup;         // A pointer to the thread local list of work-group depentent children
    };

    /**
     * Add child kernel to this KernelCommand
     * @param child		the child KernelCommand
     * @param flags		flags that specify when the child kernel will begin its execution
     */
    static cl_dev_err_code AddChildKernelToLists(const SharedPtr<KernelCommand>& child, kernel_enqueue_flags_t flags, CommandSubmitionLists* pChildLists);

    /**
     * Wait for the children to complete (not thread safe)
     */
    void WaitForChildrenCompletion();

    /**
     * Signal that the current work group has finished its execution
     */
    void SubmitCommands(CommandSubmitionLists* pNewCommands);

    /**
     * @return this KernelCommand's parent or NULL if it was enqueued from the host
     */
    const SharedPtr<KernelCommand>& GetParent() { return m_parent; }

    AtomicCounter                  m_numUserDependecies;
    SharedPtr<KernelCommand>       m_parent;
    SharedPtr<Intel::OpenCL::TaskExecutor::IThreadLibTaskGroup> m_childrenTaskGroup;
    Intel::OpenCL::Utils::AtomicPointer<CommandToExecuteList_t> m_waitingChildrenForKernelGlobal;
    volatile bool                  m_bIsDebugMode;
};

}}}
