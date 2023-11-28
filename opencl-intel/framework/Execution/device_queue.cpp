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

#include "device_queue.h"
#include "Context.h"
#include "context_module.h"

using namespace Intel::OpenCL::Framework;

cl_err_code DeviceQueue::Initialize() {
  if (m_bIsDefault) {
    m_pDefaultDevice->SetOrReturnDefaultQueue(this);
  }
  const cl_dev_subdevice_id subdevice_id =
      m_pContext->GetSubdeviceId(m_clDefaultDeviceHandle);
  const int props = CL_DEV_LIST_ENABLE_OOO |
                    (m_bProfilingEnabled ? CL_DEV_LIST_PROFILING : 0) |
                    (m_bIsDefault ? CL_DEV_LIST_QUEUE_DEFAULT : 0);
  // currently we don't make use of the CL_QUEUE_SIZE value
  const cl_dev_err_code retDev =
      m_pDefaultDevice->GetDeviceAgent()->clDevCreateCommandList(
          (cl_dev_cmd_list_props)props, subdevice_id, &m_clDevCmdListId);
  if (CL_DEV_FAILED(retDev)) {
    m_clDevCmdListId = 0;
    if (m_bIsDefault) {
      m_pDefaultDevice->UnsetDefaultQueueIfEqual(this);
    }
    return CL_OUT_OF_RESOURCES;
  }
  return CL_SUCCESS;
}

cl_int DeviceQueue::GetInfoInternal(cl_int iParamName, void *pBuf, size_t szBuf,
                                    size_t *szOuput) const {
  switch (iParamName) {
  case CL_QUEUE_SIZE:
    if (szBuf < sizeof(cl_uint))
      return CL_INVALID_VALUE;
    *(cl_uint *)pBuf = m_uiSize;
    *szOuput = sizeof(cl_uint);
    break;
  case CL_QUEUE_PROPERTIES: {
    cl_int iErrCode =
        OclCommandQueue::GetInfoInternal(iParamName, pBuf, szBuf, szOuput);
    if (CL_FAILED(iErrCode))
      return CL_INVALID_VALUE;
    *(cl_command_queue_properties *)pBuf |= CL_QUEUE_ON_DEVICE;
    if (m_bIsDefault) {
      *(cl_command_queue_properties *)pBuf |= CL_QUEUE_ON_DEVICE_DEFAULT;
    }
    break;
  }
  default:
    return OclCommandQueue::GetInfoInternal(iParamName, pBuf, szBuf, szOuput);
  }
  return CL_SUCCESS;
}
