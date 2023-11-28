// INTEL CONFIDENTIAL
//
// Copyright 2008 Intel Corporation.
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

#include "in_order_command_queue.h"
#include "Context.h"
#include "Device.h"
#include "cl_shared_ptr.hpp"
#include "enqueue_commands.h"
#include "events_manager.h"
#include "ocl_command_queue.h"
#include "ocl_event.h"
#include "llvm/Support/Compiler.h" // LLVM_FALLTHROUGH
#include <assert.h>

using namespace Intel::OpenCL::Framework;

InOrderCommandQueue::InOrderCommandQueue(
    SharedPtr<Context> pContext, cl_device_id clDefaultDeviceID,
    cl_command_queue_properties clProperties, EventsManager *pEventManager)
    : IOclCommandQueueBase(pContext, clDefaultDeviceID, clProperties,
                           pEventManager) {}
InOrderCommandQueue::~InOrderCommandQueue() {
  assert(m_submittedQueue.IsEmpty());
}

cl_err_code InOrderCommandQueue::Enqueue(Command *cmd) {
#if defined(USE_ITT) && defined(USE_ITT_INTERNAL)
  if ((nullptr != GetGPAData()) && GetGPAData()->bUseGPA) {
    static __thread __itt_string_handle *pTaskName = nullptr;
    if (nullptr == pTaskName) {
      pTaskName = __itt_string_handle_create("InOrderCommandQueue::Enqueue()");
    }
    __itt_task_begin(GetGPAData()->pAPIDomain, __itt_null, __itt_null,
                     pTaskName);
  }
#endif

  QueueEvent *pEvent = cmd->GetEvent().GetPtr();
  if (m_bProfilingEnabled) {
    pEvent->SetProfilingInfo(
        CL_PROFILING_COMMAND_SUBMIT,
        m_pDefaultDevice->GetDeviceAgent()->clDevGetPerformanceCounter());
  }
  pEvent->AddProfilerMarker("SUBMITTED", ITT_SHOW_SUBMITTED_MARKER);

  m_submittedQueue.PushBack(cmd);

  // Not sure need this code. The last submitted command has floating dependency
  // that prevents from sending to the device When floating dependency reduces,
  // command is sent for execution
#if 0
    Flush(false);    // Solves the issue of event state reported to user
                    // On this stage event has CL_SUBMITTED state;
                    // However, w/o the flush commands is not submitted to device
                    // and this violates the spec.
                    // The SUBMITTED states identifies that command was submitted to device for execution
#endif

#if defined(USE_ITT) && defined(USE_ITT_INTERNAL)
  if ((nullptr != GetGPAData()) && GetGPAData()->bUseGPA) {
    __itt_task_end(GetGPAData()->pAPIDomain);
  }
#endif

  return CL_SUCCESS;
}

cl_err_code InOrderCommandQueue::Flush(bool /*bBlocking*/) {
#if defined(USE_ITT) && defined(USE_ITT_INTERNAL)
  if ((nullptr != GetGPAData()) && GetGPAData()->bUseGPA) {
    static __thread __itt_string_handle *pTaskName = nullptr;
    if (nullptr == pTaskName) {
      pTaskName = __itt_string_handle_create("InOrderCommandQueue::Flush()");
    }
    __itt_task_begin(GetGPAData()->pAPIDomain, __itt_null, __itt_null,
                     pTaskName);
  }
#endif
  SendCommandsToDevice();
#if defined(USE_ITT) && defined(USE_ITT_INTERNAL)
  if ((nullptr != GetGPAData()) && GetGPAData()->bUseGPA) {
    __itt_task_end(GetGPAData()->pAPIDomain);
  }
#endif
  return CL_SUCCESS;
}

cl_err_code
InOrderCommandQueue::NotifyStateChange(const SharedPtr<QueueEvent> &pEvent,
                                       OclEventState /*prevColor*/,
                                       OclEventState newColor) {
#if defined(USE_ITT) && defined(USE_ITT_INTERNAL)
  if ((nullptr != GetGPAData()) && GetGPAData()->bUseGPA) {
    static __thread __itt_string_handle *pTaskName = nullptr;
    if (nullptr == pTaskName) {
      pTaskName = __itt_string_handle_create(
          "InOrderCommandQueue::NotifyStateChange()");
    }
    __itt_task_begin(GetGPAData()->pAPIDomain, __itt_null, __itt_null,
                     pTaskName);
  }
#endif
  cl_err_code ret = CL_SUCCESS;
  switch (newColor) {
  case EVENT_STATE_DONE:
    if (pEvent->EverIssuedToDevice()) {
      --m_commandsInExecution;
    }
    LLVM_FALLTHROUGH;
  case EVENT_STATE_READY_TO_EXECUTE:
    // one of the commands became ready. Trigger a SendToDevice as it may be on
    // top of the queue. Todo: this can probably be improved
    SendCommandsToDevice();
    break;

  default:
    assert(0 &&
           "InOrderCommandQueue::NotifyStateChange called with wrong color");
    ret = CL_INVALID_VALUE;
  }

#if defined(USE_ITT) && defined(USE_ITT_INTERNAL)
  if ((nullptr != GetGPAData()) && GetGPAData()->bUseGPA) {
    __itt_task_end(GetGPAData()->pAPIDomain);
  }
#endif
  return ret;
}

cl_err_code InOrderCommandQueue::SendCommandsToDevice() {
#if defined(USE_ITT) && defined(USE_ITT_INTERNAL)
  if ((nullptr != GetGPAData()) && GetGPAData()->bUseGPA) {
    static __thread __itt_string_handle *pTaskName = nullptr;
    if (nullptr == pTaskName) {
      pTaskName = __itt_string_handle_create(
          "InOrderCommandQueue::SendCommandsToDevice()");
    }
    __itt_task_begin(GetGPAData()->pAPIDomain, __itt_null, __itt_null,
                     pTaskName);
  }
#endif

  long prev_val = m_submittedQueueGuard++;
  if (0 == prev_val) {
    // use my own Queue ID as command list ID for the device
    do // loop on all requests
    {
      cl_uint cmdListLength = 0;
      cl_err_code res;
      while (!m_submittedQueue.IsEmpty()) {
        const CommandSharedPtr<> &cmd = m_submittedQueue.Top();
        OclEventState color = cmd->GetEvent()->GetEventState();
        if (EVENT_STATE_READY_TO_EXECUTE == color) {
          if (RUNTIME_EXECUTION_TYPE == cmd->GetExecutionType()) {
            if (m_commandsInExecution > 0) {
              break; // Need to wait all executing commands to complete
            }
            ++m_commandsInExecution;
            cmd->GetEvent()->SetEventState(EVENT_STATE_ISSUED_TO_DEVICE);
            res = (m_bCancelAll) ? cmd->Cancel() : cmd->Execute();
            if (CL_SUCCEEDED(res)) {
              m_submittedQueue.PopFront();
              continue;
            }
            if (res != CL_NOT_READY) {
              assert(0);
              // keep in the queue
              m_submittedQueue.PopFront();
              m_commandsInExecution--;
              continue;
            }
            break;
          }
          // Ready for execution, schedule it
          assert(m_pDefaultDevice == cmd->GetDevice());
          cmd->SetDevCmdListId(m_clDevCmdListId);
          cmd->GetEvent()->SetEventState(EVENT_STATE_ISSUED_TO_DEVICE);
          res = cmd->Execute();
          if (CL_SUCCEEDED(res)) {
            m_submittedQueue.PopFront();
            ++m_commandsInExecution;
            ++cmdListLength;
          } else if (res == CL_NOT_READY) {
            // keep in the queue
          } else if (res == CL_DONE_ON_RUNTIME) {
            // We have additional commands in execution
            cmd->SetCommandType(CL_COMMAND_MARKER);
            cmd->GetEvent()->SetEventState(EVENT_STATE_READY_TO_EXECUTE);
            if (0 == m_commandsInExecution) {
              continue;
            }
            break;
          } else {
            // there has been an error, remove from queue
            NotifyCommandFailed(res, cmd);
            m_commandsInExecution++;
            m_submittedQueue.PopFront();
          }
        } else if (EVENT_STATE_HAS_DEPENDENCIES == color) {
          break;
        } else if (((EVENT_STATE_ISSUED_TO_DEVICE == color) ||
                    (EVENT_STATE_EXECUTING_ON_DEVICE == color) ||
                    (EVENT_STATE_DONE_EXECUTING_ON_DEVICE == color)) &&
                   (RUNTIME_EXECUTION_TYPE == cmd->GetExecutionType())) {
          break; // Runtime command is still executing
        } else if (EVENT_STATE_DONE == color) {
          // We have completed command in the queue
          // There is two reasons: 1. runtime command, 2. failed command(one of
          // the dependencies failed)
          m_submittedQueue.PopFront();
          continue;
        } else {
          // Todo:
          assert(0);
        }
      }
      if (cmdListLength > 0) // any commands were submitted to the device
      {
        m_pDefaultDevice->GetDeviceAgent()->clDevFlushCommandList(
            m_clDevCmdListId);
      }
    } while (--m_submittedQueueGuard > 0);
  }
  // else return immediately
#if defined(USE_ITT) && defined(USE_ITT_INTERNAL)
  if ((nullptr != GetGPAData()) && GetGPAData()->bUseGPA) {
    __itt_task_end(GetGPAData()->pAPIDomain);
  }
#endif

  return CL_SUCCESS;
}
