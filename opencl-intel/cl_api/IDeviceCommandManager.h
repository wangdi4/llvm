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

#include "cl_device_api.h"
#include "cl_types.h"

namespace Intel {
namespace OpenCL {
namespace DeviceBackend {
class ICLDevBackendKernel_;
class ICLDevBackendBinary_;
} // namespace DeviceBackend
} // namespace OpenCL
} // namespace Intel

/**
 * This interface class represents a component that manages commands enqueued on
 * an on-device queue
 */

class IDeviceCommandManager {
public:
  /**
   * Enqueue a kernel on an on-device queue
   * @param queue          a valid on-device
   * command-queue. The kernel will be queued for execution on the device
   * associated with queue.
   * @param flags          flags that specify when
   * the child kernel begins execution
   * @param uiNumEventsInWaitList events that need to complete before this
   * particular command can be executed
   * @param pEventWaitList    number of events in
   * uiNumEventsInWaitList
   * @param pEventRet        an optional pointer for the
   * returned event for enqueued kernel
   * @param pKernel               a pointer to a ICLDevBackendKernel_ object
   * that represents the enqueued kernel
   * @param pBlockLiteral      a pointer to the block_literal
   * of the enqueued kernel. This memory must be copied before the method
   * returns.
   * @param stBlockSize      the size of pContext
   * @param pLocalSize      a pointer to an array of local sizes.
   * This memory must be copied before the method returns.
   * @param stLocalSizeCount    the number of local buffer sizes
   * @param pNdrange        a
   * cl_wor_ndrange_tk_description_type that contains the ND range parameters
   * @param pHandle        a RuntimeHandle provided to
   * RunWG() function
   * @return CL_SUCCESS if the kernel is enqueued successfully or an error
   * eitherwise
   */
  virtual int EnqueueKernel(
      queue_t queue, kernel_enqueue_flags_t flags,
      cl_uint uiNumEventsInWaitList, const clk_event_t *pEventWaitList,
      clk_event_t *pEventRet,
      const Intel::OpenCL::DeviceBackend::ICLDevBackendKernel_ *pKernel,
      const void *pBlockLiteral, size_t stBlockSize, const size_t *pLocalSize,
      size_t stLocalSizeCount, const _ndrange_t *pNDRange,
      const void *pHandle) = 0;

  /**
   * Enqueue a marker command on an on-device queue
   * @param queue          a valid on-device
   * command-queue. The marker will be queued for execution on the device
   * associated with queue.
   * @param uiNumEventsInWaitList events that need to complete before this
   * particular command can be executed
   * @param pEventWaitList    number of events in
   * uiNumEventsInWaitList
   * @param pEventRet        an optional pointer for the
   * returned event for enqueued marker
   */
  virtual int EnqueueMarker(queue_t queue, cl_uint uiNumEventsInWaitList,
                            const clk_event_t *pEventWaitList,
                            clk_event_t *pEventRet) = 0;

  /**
   * Increment the event reference count
   * @param event an event returned by EnqueueKernel or EnqueueMarker
   * @return CL_SUCCESS in case of success or an error code
   */
  virtual int RetainEvent(clk_event_t event) = 0;

  /**
   * Decrement the event reference count
   * @param event the event whose reference count is to be decremeneted
   * @retrun CL_SUCCESS if the method has been executed successfully or an error
   * eitherwise
   */
  virtual int ReleaseEvent(clk_event_t event) = 0;

  /**
   * Is event valid
   * @param event the event whose status is to be checked
   * @retrun true if the event is valid or an false otherwise
   */
  virtual bool IsValidEvent(clk_event_t event) const = 0;

  /**
   * Create a user event
   * @param piErrcodeRet if it is not NULL, the completion status of this method
   * will be assigned to this int
   * @return a valid non-zero event object
   */
  virtual clk_event_t CreateUserEvent(int *piErrcodeRet) = 0;

  /**
   * Set the execution status of a user event
   * @param event    a user event
   * @param iStatus  CL_COMPLETE or a negative integer value indicating an
   * error
   * @return CL_SUCCESS if the method was executed successfully or an error code
   * otherwise
   */
  virtual int SetEventStatus(clk_event_t event, int iStatus) = 0;

  virtual int WaitForEvents(cl_uint num_events,
                            const clk_event_t *event_list) = 0;

  /**
   * Capture the profiling information for a command
   * @param event the clk_event_t with which the command is associated
   * @param name  idetifies which profiling information is to be queried
   * @param value  a pointer to a buffer in which to store the profiling
   * information
   */
  virtual void CaptureEventProfilingInfo(clk_event_t event,
                                         clk_profiling_info name,
                                         volatile void *pValue) = 0;

  /**
   * @return the default queue for the device on which the kernel is executing
   */
  virtual queue_t GetDefaultQueueForDevice() const = 0;

  virtual queue_t GetTaskSeqQueueForDevice() const = 0;

  virtual ~IDeviceCommandManager() {}
};
