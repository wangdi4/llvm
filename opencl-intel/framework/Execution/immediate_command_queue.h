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

#include "Logger.h"
#include "cl_synch_objects.h"
#include "cl_types.h"
#include "command_queue.h"
#include "ocl_command_queue.h"

namespace Intel {
namespace OpenCL {
namespace Framework {

class Device;
class EventsManager;
class Context;

/************************************************************************
 * InOrderCommandQueue is an ICommandQueue that implements the InOrder queue
 *policy as it defined in the openCL spec. This implementation include
 *Flush/Finish support
 *
 * The queue gets notification whenever a command in the queue changes its
 *state. As a result, the queue update the state of its queues.
 *
 * The list are ordered. Hence, there is no implicit dependency between
 *consecutive commands.
 ************************************************************************/
class ImmediateCommandQueue : public IOclCommandQueueBase {
public:
  PREPARE_SHARED_PTR(ImmediateCommandQueue)

  static SharedPtr<ImmediateCommandQueue>
  Allocate(SharedPtr<Context> pContext, cl_device_id clDefaultDeviceID,
           cl_command_queue_properties clProperties,
           EventsManager *pEventManager) {
    return SharedPtr<ImmediateCommandQueue>(new ImmediateCommandQueue(
        pContext, clDefaultDeviceID, clProperties, pEventManager));
  }

  virtual ~ImmediateCommandQueue();

  virtual cl_err_code Initialize() override;

  virtual cl_err_code EnqueueCommand(Command *pCommand, cl_bool bBlocking,
                                     cl_uint uNumEventsInWaitList,
                                     const cl_event *cpEeventWaitList,
                                     cl_event *pEvent,
                                     Utils::ApiLogger *apiLogger) override;
  virtual cl_err_code Enqueue(Command *cmd) override;
  virtual cl_err_code EnqueueWaitForEvents(Command *cmd) override {
    return Enqueue(cmd);
  }
  virtual cl_err_code EnqueueMarkerWaitForEvents(Command *marker) override;
  virtual cl_err_code EnqueueBarrierWaitForEvents(Command *barrier) override;

  virtual cl_err_code Flush(bool bBlocking) override;
  virtual cl_err_code NotifyStateChange(const SharedPtr<QueueEvent> &pEvent,
                                        OclEventState prevColor,
                                        OclEventState newColor) override;
  virtual cl_err_code SendCommandsToDevice() override;

  virtual void AddFloatingDependence(
      const SharedPtr<QueueEvent> & /*pCmdEvent*/) const override {}
  virtual void RemoveFloatingDependence(
      const SharedPtr<QueueEvent> & /*pCmdEvent*/) const override {}

protected:
  ImmediateCommandQueue(SharedPtr<Context> pContext,
                        cl_device_id clDefaultDeviceID,
                        cl_command_queue_properties clProperties,
                        EventsManager *pEventManager);

  std::mutex m_CS;
};

} // namespace Framework
} // namespace OpenCL
} // namespace Intel
