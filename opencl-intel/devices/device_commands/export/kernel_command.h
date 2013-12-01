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

using Intel::OpenCL::TaskExecutor::ITaskGroup;

namespace Intel { namespace OpenCL { namespace DeviceCommands {

/**
 * This class represents an ND-range kernel command enqueued either by the host or a parent kernel
 */
class KernelCommand : public DeviceCommand
{
public:
    PREPARE_SHARED_PTR(KernelCommand)

    /**
     * Add child kernel to this KernelCommand
     * @param child		the child KernelCommand
     * @param flags		flags that specify when the child kernel will begin its execution
     */
    void AddChildKernel(const SharedPtr<KernelCommand>& child, kernel_enqueue_flags_t flags);

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
    SharedPtr<ITaskGroup> GetParentTaskGroup()
    {
      if (m_parent != NULL)
      {
        return m_parent->m_childrenTaskGroup;
      }
      return NULL;
    }

    // inherited methods:
    virtual void Launch();

protected:

    /**
     * Constructor
     * @param list				the ITaskList that implements the command-queue on which this DeviceCommand executes
     * @param parent			the parent KernelCommand or NULL if this KernelCommand has been enqueued from the host
     * @param childrenTaskGroup ITaskGroup for waiting for children kernels
     * @param pMyTaskBase		a pointer to itself as ITaskBase or NULL if the concrete class does not inherit from ITaskBase
     */
    KernelCommand(const SharedPtr<ITaskList>& list, const SharedPtr<KernelCommand>& parent, const SharedPtr<ITaskGroup>& childrenTaskGroup, ITaskBase* pMyTaskBase) :
       DeviceCommand(list, pMyTaskBase), m_parent(parent), m_childrenTaskGroup(childrenTaskGroup) { }

    /**
     * @return a vector of child KernelCommands that are waiting for the current work-group
     */
    virtual std::vector<SharedPtr<KernelCommand> >& GetWaitingChildrenForWG() = 0;

    /**
     * @return a vector of child KernelCommands that are waiting for the entire parent in the current work-group
     */
    virtual std::vector<SharedPtr<KernelCommand> >& GetWaitingChildrenForParentInWg() = 0;

    /**
     * Wait for the children to complete (not thread safe)
     */
    void WaitForChildrenCompletion();

    /**
     * Is any waiting child exist (not thread safe)
     */
    bool IsWaitingChildExist() { return (m_waitingChildrenForKernel.size() > 0); }

    /**
     * Signal that the current work group has finished its execution
     */
    void WgFinishedExecution();

    /**
     * @return this KernelCommand's parent or NULL if it was enqueued from the host
     */
    const SharedPtr<KernelCommand>& GetParent() { return m_parent; }

  private:
    AtomicCounter m_numUserDependecies;
    const SharedPtr<KernelCommand> m_parent;
    Intel::OpenCL::Utils::OclSpinMutex m_muChildrenForKernel;
    std::vector<SharedPtr<KernelCommand> > m_waitingChildrenForKernel;
    SharedPtr<ITaskGroup> m_childrenTaskGroup;
};

}}}
