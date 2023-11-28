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

#pragma once

#include "IDeviceCommandManager.h"
#include "cl_dev_backend_api.h"
#include "cl_sys_defines.h"
#include "command.h"

namespace Intel {
namespace OpenCL {
namespace DeviceCommands {

/**
 * This class represents an ND-range kernel command enqueued either by the host
 * or a parent kernel
 */
class KernelCommand : public DeviceCommand, public IDeviceCommandManager {
public:
  PREPARE_SHARED_PTR(KernelCommand)

  /**
   * A child kernel has completed
   * @param err the child's error code
   */
  void ChildCompleted(cl_dev_err_code err) {
    if (CL_DEV_FAILED(err)) {
      SetError(err);
    }
  }

  /**
   * @return ITaskGroup of the KernelCommand's parent (used to let the parent
   * wait for its children)
   */
  SharedPtr<Intel::OpenCL::TaskExecutor::IThreadLibTaskGroup>
  GetParentTaskGroup() {
    if (m_parent != 0) {
      return m_parent->m_childrenTaskGroup;
    }
    return nullptr;
  }

  virtual KernelCommand *AllocateChildCommand(
      ITaskList *pList,
      const Intel::OpenCL::DeviceBackend::ICLDevBackendKernel_ *pKernel,
      const void *pBlockLiteral, size_t stBlockSize, const size_t *pLocalSizes,
      size_t stLocalSizeCount, const _ndrange_t *pNDRange) const = 0;

  // inherited methods:
  virtual void Launch() override;

  // IDeviceCommandManager interface
  int EnqueueKernel(
      queue_t queue, kernel_enqueue_flags_t flags,
      cl_uint uiNumEventsInWaitList, const clk_event_t *pEventWaitList,
      clk_event_t *pEventRet,
      const Intel::OpenCL::DeviceBackend::ICLDevBackendKernel_ *pKernel,
      const void *pBlockLiteral, size_t stBlockSize, const size_t *pLocalSize,
      size_t stLocalSizeCount, const _ndrange_t *pNDRange,
      const void *pHandle) override;

  int EnqueueMarker(queue_t queue, cl_uint uiNumEventsInWaitList,
                    const clk_event_t *pEventWaitList,
                    clk_event_t *pEventRet) override;

  int RetainEvent(clk_event_t event) override;

  int ReleaseEvent(clk_event_t event) override;

  clk_event_t CreateUserEvent(int *piErrcodeRet) override;

  virtual bool IsValidEvent(clk_event_t event) const override;

  int SetEventStatus(clk_event_t event, int iStatus) override;

  int WaitForEvents(cl_uint num_events, const clk_event_t *event_list) override;

  void CaptureEventProfilingInfo(clk_event_t event, clk_profiling_info name,
                                 volatile void *pValue) override;

  queue_t GetDefaultQueueForDevice() const override;

protected:
  /**
   * Constructor
   * @param list        the ITaskList that implements
   * the command-queue on which this DeviceCommand executes
   * @param parent      the parent KernelCommand or NULL if this
   * KernelCommand has been enqueued from the host
   * @param pMyTaskBase    a pointer to itself as ITaskBase or NULL if the
   * concrete class does not inherit from ITaskBase
   */
  KernelCommand(ITaskList *pList, KernelCommand *parent, ITaskBase *pMyTaskBase)
      : DeviceCommand(pList, pMyTaskBase), m_parent(parent),
        m_childrenTaskGroup(nullptr != pList
                                ? pList->GetNDRangeChildrenTaskGroup().GetPtr()
                                : nullptr) {
    m_waitingChildrenForKernelGlobal = nullptr;
  }

  // Enqueued command list definition
  struct CommandToExecuteList_t {
    SharedPtr<KernelCommand> command;
    CommandToExecuteList_t *next;
  };

  // Structure defined child kernel submission for each thread
  struct CommandSubmitionLists {
    CommandSubmitionLists()
        : waitingChildrenForKernelLocalHead(nullptr),
          waitingChildrenForKernelLocalTail(nullptr),
          waitingChildrenForWorkGroup(nullptr) {}
    CommandToExecuteList_t
        *waitingChildrenForKernelLocalHead; // A pointer to the head of the
                                            // thread local list of kernel
                                            // depentent children
    CommandToExecuteList_t
        *waitingChildrenForKernelLocalTail; // A pointer to the tail of the
                                            // thread local list of kernel
                                            // depentent children
    CommandToExecuteList_t
        *waitingChildrenForWorkGroup; // A pointer to the thread local list of
                                      // work-group depentent children
  };

  /**
   * Add child kernel to this KernelCommand
   * @param child    the child KernelCommand
   * @param flags    flags that specify when the child kernel will
   * begin its execution
   */
  static cl_dev_err_code
  AddChildKernelToLists(const SharedPtr<KernelCommand> &child,
                        kernel_enqueue_flags_t flags,
                        CommandSubmitionLists *pChildLists);

  /**
   * Wait for the children to complete (not thread safe)
   */
  void WaitForChildrenCompletion();

  /**
   * Signal that the current work group has finished its execution
   */
  void SubmitCommands(CommandSubmitionLists *pNewCommands);

  /**
   * @return this KernelCommand's parent or NULL if it was enqueued from the
   * host
   */
  const SharedPtr<KernelCommand> &GetParent() { return m_parent; }

  SharedPtr<KernelCommand> m_parent;
  SharedPtr<Intel::OpenCL::TaskExecutor::IThreadLibTaskGroup>
      m_childrenTaskGroup;
  Intel::OpenCL::Utils::AtomicPointer<CommandToExecuteList_t>
      m_waitingChildrenForKernelGlobal;
};

} // namespace DeviceCommands
} // namespace OpenCL
} // namespace Intel
