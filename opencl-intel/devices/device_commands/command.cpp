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

#include "command.h"
#include "cl_shared_ptr.hpp"
#include "cl_sys_info.h"

using namespace Intel::OpenCL::DeviceCommands;
using Intel::OpenCL::Utils::AccurateHostTime;

void DeviceCommand::SetError(cl_dev_err_code err) {
  // if some child has nofitied us of some failure, we don't care if other
  // children or ourselves have completed successfully
  if (CL_DEV_SUCCEEDED(m_err)) {
    m_err = err;
  }
}

bool DeviceCommand::AddWaitListDependencies(const clk_event_t *pEventWaitList,
                                            cl_uint uiNumEventsInWaitList) {
  // this method is called once for a DeviceCommand right after it has been
  // created and before it is enqueued
  bool bAllEventsCompleted = true;
  m_numDependencies.fetch_add(uiNumEventsInWaitList);

  m_commandsThisIsWaitingFor.resize(uiNumEventsInWaitList);
  for (cl_uint i = 0; i < uiNumEventsInWaitList; i++) {
    DeviceCommand &waitingForCmd = *(DeviceCommand *)pEventWaitList[i];
    std::lock_guard<std::recursive_mutex> mutex(waitingForCmd.m_mutex);
    // we must protect from a race between waitingForCmd.m_bCompleted becoming
    // true and adding this to its m_waitingCommandsForThis
    if (!waitingForCmd.m_bCompleted) {
      bAllEventsCompleted = false;
    }
    waitingForCmd.m_waitingCommandsForThis.push_back(this);
    m_commandsThisIsWaitingFor[i] = &waitingForCmd;
  }
  return bAllEventsCompleted;
}

void DeviceCommand::NotifyCommandFinished(cl_dev_err_code err) {
  ASSERT_RET(m_numDependencies > 0, "m_numDependencies > 0");
  const long lCurrentDependencies = --m_numDependencies;
  if (CL_DEV_FAILED(err)) {
    SetError(err);
  }
  if (0 == lCurrentDependencies) {
    m_commandsThisIsWaitingFor.clear();
    if (CL_DEV_SUCCEEDED(GetError())) {
      Launch();
    } else {
      SignalComplete(GetError());
    }
  }
}

void DeviceCommand::SignalComplete(cl_dev_err_code err) {
  if (m_bIsProfilingEnabled) {
    const unsigned long long ulCompleteTime = AccurateHostTime();
    const long long lStartExecTime = (long long)m_ulStartExecTime,
                    lCompleteTime = (long long)ulCompleteTime;
    /* Because TSC values in different cores are slightly different, we can get
       negative time slices. Therefore we check this (but we are careful to
       identify a wrap-around) and assign, which is the only reasonable value in
       this case. */
    if (ulCompleteTime <= m_ulStartExecTime &&
        !(lCompleteTime >= 0 && lStartExecTime < 0)) {
      m_ulCompleteTime = 1;
    } else {
      m_ulCompleteTime = ulCompleteTime - m_ulStartExecTime;
    }
    if (nullptr != m_pExecTimeUserPtr) {
      ((volatile cl_long *)m_pExecTimeUserPtr)[1] = m_ulCompleteTime;
    }
  }

  SetError(err);
  std::lock_guard<std::recursive_mutex> mutex(m_mutex);
  // m_bCompleted and m_waitingCommandsForThis are protected together (see
  // AddWaitListDependencies)
  m_bCompleted = true;

  for (std::vector<SharedPtr<DeviceCommand>>::iterator iter =
           m_waitingCommandsForThis.begin();
       iter != m_waitingCommandsForThis.end(); iter++) {
    (*iter)->NotifyCommandFinished(GetError());
  }
  m_waitingCommandsForThis.clear();
  m_event.Signal();
}

void DeviceCommand::Wait() const { m_event.Wait(); }

void DeviceCommand::StartExecutionProfiling() {
  if (m_bIsProfilingEnabled) {
    m_ulStartExecTime = AccurateHostTime();
  }
}

void DeviceCommand::StopExecutionProfiling() {
  if (m_bIsProfilingEnabled) {
    const unsigned long long ulEndExecTime = AccurateHostTime();
    m_ulExecTime = ulEndExecTime - m_ulStartExecTime;
    if (nullptr != m_pExecTimeUserPtr) {
      *(volatile cl_long *)m_pExecTimeUserPtr = m_ulExecTime;
    }
  }
}

bool DeviceCommand::SetExecTimeUserPtr(volatile void *pExecTimeUserPtr) {
  std::lock_guard<std::recursive_mutex> mutex(m_mutex);
  if (m_bCompleted) {
    return false;
  }
  m_pExecTimeUserPtr = pExecTimeUserPtr;
  return true;
}
