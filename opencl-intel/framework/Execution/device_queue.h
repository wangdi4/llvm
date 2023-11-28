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

#include "command_queue.h"

namespace Intel {
namespace OpenCL {
namespace Framework {

/**
 * This class represents an on-device command-queue
 */
class DeviceQueue : public OclCommandQueue {
public:
  PREPARE_SHARED_PTR(DeviceQueue)

  static SharedPtr<DeviceQueue>
  Allocate(const SharedPtr<Context> &pContext, cl_device_id clDefaultDeviceID,
           cl_command_queue_properties clProperties,
           EventsManager *pEventManager, bool bEnableProfiling, bool bIsDefault,
           cl_uint uiSize) {
    return new DeviceQueue(pContext, clDefaultDeviceID, clProperties,
                           pEventManager, bEnableProfiling, bIsDefault, uiSize);
  }

  // overriden methods:

  virtual cl_err_code Initialize() override;

  virtual cl_int GetInfoInternal(cl_int iParamName, void *pBuf, size_t szBuf,
                                 size_t *szOutput) const override;

  cl_err_code SetDefaultOnDevice(SharedPtr<FissionableDevice> pDevice) {
    return pDevice->SetDefaultDeviceQueue(this, m_clDevCmdListId);
  }

protected:
  virtual ~DeviceQueue() {
    // If the queue is a default device queue
    // the following call will unset it
    m_pDefaultDevice->UnsetDefaultQueueIfEqual(this);
  }

private:
  /**
   * Constructor
   * @param pContext        the context in which the
   * command-queue is to be created
   * @param clDefaultDeviceID    the device ID of the device in which the
   * command-queue is to be created
   * @param clProperties      the command-queue's properties
   * @param pEventManager      a pointer to the EventManager
   * @param bEnableProfiling    whether to enable profiling of commands
   * in the command-queue
   * @param bIsDefault      whether this command-queue is the
   * default device queue
   * @param uiSize        size of the device queue in
   * bytes
   */
  DeviceQueue(const SharedPtr<Context> &pContext,
              cl_device_id clDefaultDeviceID,
              cl_command_queue_properties clProperties,
              EventsManager *pEventManager, bool /*bEnableProfiling*/,
              bool bIsDefault, cl_uint uiSize)
      : OclCommandQueue(pContext, clDefaultDeviceID, clProperties,
                        pEventManager),
        m_bIsDefault(bIsDefault), m_uiSize(uiSize) {}

  const bool m_bIsDefault;
  const cl_uint m_uiSize;
};

} // namespace Framework
} // namespace OpenCL
} // namespace Intel
