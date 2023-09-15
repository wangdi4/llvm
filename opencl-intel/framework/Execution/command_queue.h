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

#pragma once

#include "Context.h"
#include "cl_types.h"
#include "enqueue_commands.h"
#include "ocl_command_queue.h"
#include "queue_event.h"

namespace Intel {
namespace OpenCL {
namespace Framework {

// Forward declaration
class Command;

/**
 *
 *
 */
class ICommandQueue {
public:
  virtual cl_err_code Enqueue(Command *command) = 0;
  virtual cl_err_code EnqueueWaitForEvents(Command *wfe) = 0;
  virtual cl_err_code EnqueueMarkerWaitForEvents(Command *marker) = 0;
  virtual cl_err_code EnqueueBarrierWaitForEvents(Command *barrier) = 0;
  virtual cl_err_code
  WaitForCompletion(const SharedPtr<QueueEvent> &pEvent) = 0;

  virtual cl_err_code Flush(bool bBlocking) = 0;
  virtual cl_err_code SendCommandsToDevice() = 0;
  virtual cl_err_code NotifyStateChange(const SharedPtr<QueueEvent> &pEvent,
                                        OclEventState prevColor,
                                        OclEventState newColor) = 0;
  virtual ~ICommandQueue() {}
};

class IOclCommandQueueBase : public OclCommandQueue, public ICommandQueue {
public:
  PREPARE_SHARED_PTR(IOclCommandQueueBase)

  enum RUNTIME_COMMAND_TYPE { JUST_WAIT = 0, MARKER, BARRIER };

  virtual cl_err_code EnqueueCommand(Command *pCommand, cl_bool bBlocking,
                                     cl_uint uNumEventsInWaitList,
                                     const cl_event *cpEeventWaitList,
                                     cl_event *pEvent,
                                     Utils::ApiLogger *apiLogger);
  virtual cl_err_code EnqueueRuntimeCommandWaitEvents(
      RUNTIME_COMMAND_TYPE type, Command *pCommand,
      cl_uint uNumEventsInWaitList, const cl_event *pEventWaitList,
      cl_event *pEvent, Utils::ApiLogger *pApiLogger);
  virtual cl_err_code
  WaitForCompletion(const SharedPtr<QueueEvent> &pEvent) override;
  virtual ocl_gpa_data *GetGPAData() const override {
    return m_pContext->GetGPAData();
  }

  virtual void
  AddFloatingDependence(const SharedPtr<QueueEvent> &pCmdEvent) const {
    pCmdEvent->AddFloatingDependence();
  }
  virtual void
  RemoveFloatingDependence(const SharedPtr<QueueEvent> &pCmdEvent) const {
    pCmdEvent->RemoveFloatingDependence();
  }

  void NotifyCommandFailed(cl_err_code err,
                           const CommandSharedPtr<> &command) const;

protected:
  IOclCommandQueueBase(SharedPtr<Context> pContext,
                       cl_device_id clDefaultDeviceID,
                       cl_command_queue_properties clProperties,
                       EventsManager *pEventManager)
      : OclCommandQueue(pContext, clDefaultDeviceID, clProperties,
                        pEventManager) {}

  virtual ~IOclCommandQueueBase() {}

private:
  cl_err_code EnqueueWaitEventsProlog(const SharedPtr<QueueEvent> &pEvent,
                                      cl_uint uNumEventsInWaitList,
                                      const cl_event *pEventWaitList);
};
} // namespace Framework
} // namespace OpenCL
} // namespace Intel
