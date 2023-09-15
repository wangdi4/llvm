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

#include "cl_shared_ptr.h"
#include "task_executor.h"
#include <atomic>
#include <mutex>
#include <vector>

using Intel::OpenCL::TaskExecutor::ITask;
using Intel::OpenCL::TaskExecutor::ITaskBase;
using Intel::OpenCL::TaskExecutor::ITaskList;
using Intel::OpenCL::TaskExecutor::TASK_PRIORITY;
using Intel::OpenCL::Utils::ReferenceCountedObject;
using Intel::OpenCL::Utils::SharedPtr;

namespace Intel {
namespace OpenCL {
namespace DeviceCommands {

/**
 * This class represents any command that is enqueued to a device, either by the
 * host or a kernel.
 */
class DeviceCommand : virtual public ReferenceCountedObject {
public:
  PREPARE_SHARED_PTR(DeviceCommand)

  /**
   * Add dependencies according to an event wait list
   * @param pEventWaitList    an event wait list
   * @param uiNumEventsInWaitList size of the wait list
   * @return whether all events have completed
   */
  bool AddWaitListDependencies(const clk_event_t *pEventWaitList,
                               cl_uint uiNumEventsInWaitList);

  /**
   * Notify that a DeviceCommand for which this DeviceCommand is waiting has
   * finished (completed for event wait list or finished execution for parent
   * kernel/work group)
   * @param err the error code of the completed DeviceCommand
   */
  void NotifyCommandFinished(cl_dev_err_code err);

  /**
   * @return the error code of this DeviceCommand
   */
  cl_dev_err_code GetError() const { return m_err; }

  /**
   * add dependency to this DeviceCommand for another DeviceCommand
   */
  void AddDependency() { m_numDependencies++; }

  /**
   * @return the ITaskList that implements the command-queue on which this
   * DeviceCommand executes
   */
  const SharedPtr<ITaskList> &GetList() { return m_list; }

  /**
   * @return whether this DeviceCommand is a user command
   */
  virtual bool IsUserCommand() const { return false; }

  /**
   * @return this DeviceCommand's execution time
   */
  unsigned long long GetExecutionTime() const { return m_ulExecTime; }

  /**
   * @return this DeviceCommand's complete time
   */
  unsigned long long GetCompleteTime() const { return m_ulCompleteTime; }

  /**
   * Set the user pointer where the execution time value will be stored when it
   * is available
   * @return whether the pointer has been set (if not, then the information is
   * already available when calling GetExecutionTime())
   */
  bool SetExecTimeUserPtr(volatile void *pExecTimeUserPtr);

  /**
   * @return whether this DeviceCommand's state is CL_COMPLETE
   */
  bool IsCompleted() const { return m_bCompleted; }

  /**
   * @return a pointer to this DeviceCommand as an ITaskBase object or NULL if
   * it does not inherit from ITaskBase
   */
  ITaskBase *GetMyTaskBase() { return m_pMyTaskBase; };

  /**
   * Launch this DeviceCommand for execution
   */
  virtual void Launch() = 0;

  /**
   * Check memory stamp of the object
   */
  bool CheckStamp() const {
    if (m_CMD_STAMP != CMD_STAMP) {
      return false;
    }
    return true;
  }

protected:
  /**
   * Constructor
   * @param list      the ITaskList that implements the
   * command-queue on which this DeviceCommand executes
   * @param pMyTaskBase  a pointer to itself as ITaskBase or NULL if the
   * concrete class does not inherit from ITaskBase
   */
  DeviceCommand(ITaskList *list, ITaskBase *pMyTaskBase)
      : m_err(CL_DEV_SUCCESS), m_bCompleted(false), m_ulStartExecTime(0),
        m_ulExecTime(0), m_pExecTimeUserPtr(nullptr), m_list(list),
        m_pMyTaskBase(pMyTaskBase),
        m_bIsProfilingEnabled(nullptr != list ? list->IsProfilingEnabled()
                                              : false),
        m_CMD_STAMP(CMD_STAMP) {
    m_event.Init();
  }

  /**
   * Signal that this DeviceCommand is changing its state to CL_COMPLETE
   * @param err the error code of the DeviceCommand
   */
  void SignalComplete(cl_dev_err_code err);

  void Wait() const;

  /**
   * Set an error code for this DeviceCommand
   * @param err the error code to set
   */
  void SetError(cl_dev_err_code err);

  /**
   * Signal that this DeviceCommand's execution has started
   */
  void StartExecutionProfiling();

  /**
   * Signal that this DeviceCommand's execution has ended
   */
  void StopExecutionProfiling();

private:
  std::atomic<long> m_numDependencies{0};
  cl_dev_err_code m_err;
  volatile bool m_bCompleted;
  unsigned long long m_ulStartExecTime;
  unsigned long long
      m_ulExecTime; // CL_PROFILING_COMMAND_END CL_PROFLING_COMMAND_START
  unsigned long long m_ulCompleteTime = 0; // CL_PROFILING_COMMAND_COMPLETE
                                           // CL_PROFILING_COMAMND_START
  volatile void *
      m_pExecTimeUserPtr; // a pointer to two 64-bit values: the first will hold
                          // m_ulExecTime and the second to m_ulCompleteTime
  std::vector<SharedPtr<DeviceCommand>>
      m_waitingCommandsForThis; // a list of DeviceCommands waiting for this
                                // DeviceCommand to finish
  // a list of DeviceCommands this DeviceCommand is waiting for (for holding a
  // SharedPtr to them, so they wouldn't be deleted until this DeviceCommand is
  // finished)
  std::vector<SharedPtr<DeviceCommand>> m_commandsThisIsWaitingFor;
  SharedPtr<ITaskList> m_list;
  mutable std::recursive_mutex m_mutex;
  mutable Utils::OclOsDependentEvent m_event;
  ITaskBase *const m_pMyTaskBase;
  const bool m_bIsProfilingEnabled;
  static const cl_long CMD_STAMP =
      0xEFEBDAEFEBDA7190; // memory stamp for detecting if the object belongs to
                          // the DeviceCommand class
  const cl_long m_CMD_STAMP;
};

/**
 * This class represents a device-side marker command
 */
class Marker : public DeviceCommand, public ITask {
public:
  PREPARE_SHARED_PTR(Marker)

  /**
   * Allocate a new Marker
   * @param list the ITaskList that implements the command-queue on which this
   * DeviceCommand executes
   */
  static SharedPtr<Marker> Allocate(ITaskList *list) {
    return new Marker(list);
  }

  // inherited methods:

  virtual bool Execute() override {
    StartExecutionProfiling();
    SignalComplete(CL_DEV_SUCCESS);
    StopExecutionProfiling();
    return true;
  }

  bool IsTaskSet() const override { return false; }

  bool IsCompleted() const override { return DeviceCommand::IsCompleted(); }

  virtual TASK_PRIORITY GetPriority() const override {
    return Intel::OpenCL::TaskExecutor::TASK_PRIORITY_MEDIUM;
  }

  virtual bool CompleteAndCheckSyncPoint() override { return false; }

  virtual bool SetAsSyncPoint() override {
    ASSERT_RET_VAL(false,
                   "Device commands can't be used as a synchronization point",
                   IsCompleted());
    return IsCompleted();
  }

  virtual long Release() override { return 0; }

  void Cancel() override { SetError(CL_DEV_COMMAND_CANCELLED); }

  virtual Intel::OpenCL::TaskExecutor::IThreadLibTaskGroup *
  GetNDRangeChildrenTaskGroup() override {
    return nullptr;
  }

  virtual void Launch() override { Execute(); }

private:
  Marker(ITaskList *list) : DeviceCommand(list, this) {}
};

/**
 * This class represents a device-side user event
 */
class UserEvent : public DeviceCommand {
public:
  /**
   * Allocate a new UserEvent
   */
  static UserEvent *Allocate() { return new UserEvent(); }

  /**
   * Set the execution status of the UserEvent
   * @param iStatus CL_COMPLETE or a negative integer value indicating an error
   */
  void SetStatus(int iStatus) {
    if (CL_COMPLETE == iStatus) {
      SetError(CL_DEV_SUCCESS);
    } else {
      SetError(CL_DEV_ERROR_FAIL);
    }
    SignalComplete(GetError());
  }

  // inherited methods

  bool IsUserCommand() const override { return true; }

  virtual void Launch() override {
    assert(false && "UserEvent shouldn't be launched");
  }

protected:
  UserEvent() : DeviceCommand(nullptr, nullptr) {}
};

} // namespace DeviceCommands
} // namespace OpenCL
} // namespace Intel
