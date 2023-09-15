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
#include "cl_object.h"
#include "cl_types.h"
#include "ocl_event.h"
#include "ocl_itt.h"
#include "queue_event.h"
#include <mutex>

namespace Intel {
namespace OpenCL {
namespace Framework {

class Command;

/*******************************************************************************
 * Class name:    OclEvent
 *
 * Description:
 *      TODO
 *
 ******************************************************************************/
class QueueEvent : public OclEvent {

public:
  PREPARE_SHARED_PTR(QueueEvent)
  friend class Command;
  static SharedPtr<QueueEvent>
  Allocate(const SharedPtr<IOclCommandQueueBase> &cmdQueue) {
    return new QueueEvent(cmdQueue);
  }

  ~QueueEvent();

  const SharedPtr<IOclCommandQueueBase> GetEventQueue() const;

  cl_command_queue GetEventQueueHandle() const { return m_pEventQueueHandle; }
  cl_int GetEventQueueId() const { return m_pEventQueueId; }

  cl_int GetReturnCode() const override;
  // OCLObject implementation
  cl_err_code GetInfo(cl_int iParamName, size_t szParamValueSize,
                      void *paramValue,
                      size_t *szParamValueSizeRet) const override;

  // Override to notify my command about failed events it depended on
  virtual cl_err_code
  ObservedEventStateChanged(const SharedPtr<OclEvent> &pEvent,
                            cl_int returnCode) override;

  // profiling support
  cl_err_code GetProfilingInfo(cl_profiling_info clParamName,
                               size_t szParamValueSize, void *pParamValue,
                               size_t *pszParamValueSizeRet);
  void SetProfilingInfo(cl_profiling_info clParamName, cl_ulong ulData);

  // include times from other command into me
  void IncludeProfilingInfo(const SharedPtr<QueueEvent> &other);

  void SetCommand(Command *cmd) { m_pCommand = cmd; }
  Command *GetCommand() const { return m_pCommand; }

  OclEventState
  SetEventState(OclEventState newColor); // returns the previous color

  // return true is command was ever executed
  bool EverIssuedToDevice() const { return m_bEverIssuedToDevice; }

  // Add notification to ITT library when event state change occurs
  void AddProfilerMarker(const char *szMarkerName, int iMarkerMask);
  bool GetVisibleToUser() const { return m_bVisibleToUser; };
  void SetVisibleToUser() { m_bVisibleToUser = true; };

#if defined(USE_ITT) && defined(USE_ITT_INTERNAL)
  // Override wait to track it in VTune. Need to track it here and not in parent
  // class as parent class does not contain required data and what we need now
  // is only a Queue Wait. Calls to parent Wait to do the real work.
  virtual void Wait();
#endif

protected:
  QueueEvent(const SharedPtr<IOclCommandQueueBase> &cmdQueue);

  // overrides from OclEvent
  virtual void DoneWithDependencies(const SharedPtr<OclEvent> &pEvent) override;
  virtual void NotifyComplete(cl_int returnCode = CL_SUCCESS) override;

  SProfilingInfo m_sProfilingInfo;
  bool m_bProfilingEnabled;
  bool m_bCommandQueuedValid;
  bool m_bCommandSubmitValid;
  bool m_bCommandStartValid;
  bool m_bCommandEndValid;
  bool m_bCommandCompleteValid;
  bool m_bVisibleToUser;
  Command *m_pCommand; // Pointer to the command represented by this event

  mutable std::recursive_mutex m_queueLock;
  SharedPtr<IOclCommandQueueBase>
      m_pEventQueue; // Pointer to the queue that this event was enqueued on
  cl_command_queue m_pEventQueueHandle; // A cached copy of m_pEventQueue's
                                        // handle to use in QueueEvent::GetInfo
  cl_int m_pEventQueueId;               // A cached copy of m_pEventQueue's id
  bool m_pEventQueueIsOOO; // A cached copy of m_pEventQueue's OOO status

private:
  bool m_bEverIssuedToDevice;
  void operator delete(void *p);

  ocl_gpa_data *m_pGPAData;
#if defined(USE_ITT)
  __itt_id m_ittID = __itt_null;
#endif
};

} // namespace Framework
} // namespace OpenCL
} // namespace Intel
