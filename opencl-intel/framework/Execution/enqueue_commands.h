// INTEL CONFIDENTIAL
//
// Copyright 2008-2019 Intel Corporation.
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

#include "Device.h"
#include "MemoryObject.h"
#include "cl_object.h"
#include "command_name.h"
#include "kernel.h"
#include "observer.h"
#include "queue_event.h"
#include "task_executor.h"

#include <CL/cl.h>
#include <Logger.h>
#include <cl_device_api.h>
#include <cl_types.h>
#include <list>
#include <ocl_itt.h>

namespace Intel {
namespace OpenCL {
namespace Framework {

// Forward declarations
class QueueEvent;
class MemoryObject;
class Kernel;
class IOclCommandQueueBase;
class ContextModule;
class OclCommandQueue;

/******************************************************************
 * This enumeration is used to identify if a command is going to be
 * executed on a device, or whether it is used by runtime only, mainly
 * for synch objects such as Barriers and Markers.
 ******************************************************************/
enum ECommandExecutionType {
  RUNTIME_EXECUTION_TYPE, // Command is executed in the runtime only
  DEVICE_EXECUTION_TYPE // The command is expected to be executed on the device
};

/******************************************************************
 *
 ******************************************************************/
class Command : public ICmdStatusChangedObserver {

public:
  Command(const SharedPtr<IOclCommandQueueBase> &cmdQueue);
  virtual ~Command();

  //
  // Use this function to initiate the command local data set by a command
  // constructor
  //
  virtual cl_err_code Init() = 0;

  //
  // This function is called when CL_COMPLETED is notified.
  // The command use this function to update its data respectively such as the
  // buffers.
  //
  virtual cl_err_code CommandDone() = 0;

  //
  // The function that is called when the command is popped out from the queue
  // and ready for the device Each command implements its local logic within
  // this function.
  //
  virtual cl_err_code Execute() = 0;

  //
  // The function that is called when the command is popped out from the queue
  // and ready for the device INSTEAD of Execute. Default implementation does
  // the generic part - if more is required override it and add more.
  //
  virtual cl_err_code Cancel();

  //
  // Returns the command type for GetInfo requests and execution needs
  //
  virtual cl_command_type GetCommandType() const { return m_commandType; }

  //
  // set command type (some commands like ReadBuffer/ReadImage change to MARKER
  // on their Execute() if decided not to go to device)
  //
  void SetCommandType(cl_command_type newCmdType) {
    m_commandType = newCmdType;
  }
  // Returns True if command is: Marker || Barrier || WaitForEvents
  //
  virtual bool isControlCommand() const { return false; }
  //
  // Returns the return code of the command
  //
  virtual cl_int GetReturnCode() const { return m_returnCode; }
  //
  // Sets the return code of the command
  //
  virtual void SetReturnCode(cl_int returnCode) { m_returnCode = returnCode; }
  //
  // Returns whether a command is going to be executed on the device or not.
  //
  virtual ECommandExecutionType GetExecutionType() const = 0;

  virtual const SharedPtr<IOclCommandQueueBase> &GetCommandQueue() {
    return m_pCommandQueue;
  }

  // ICmdStatusChangedObserver function
  cl_err_code NotifyCmdStatusChanged(cl_int iCmdStatus,
                                     cl_int iCompletionResult,
                                     cl_ulong ulTimer) override;

  // Command general functions
  const SharedPtr<QueueEvent> &GetEvent() { return m_Event; }
  void SetDevCmdListId(cl_dev_cmd_list clDevCmdListId) {
    m_clDevCmdListId = clDevCmdListId;
  }
  cl_dev_cmd_list GetDevCmdListId() const { return m_clDevCmdListId; }
  void SetDevice(const SharedPtr<FissionableDevice> &pDevice) {
    m_pDevice = pDevice;
  }
  const SharedPtr<FissionableDevice> &GetDevice() const { return m_pDevice; }

  void SetUsmPtrList(const std::vector<const void *> &usmPtrs) {
    std::lock_guard<std::mutex> lock(m_UsmPtrsMutex);
    m_UsmPtrs = usmPtrs;
  }

  cl_dev_cmd_desc *GetDeviceCommandDescriptor();

  // wrapper above Enqueue command to allow pre/post-fix commands
  // pEvent is an external user pointer that will point to the user-wisible
  // command which completion means user command completion Note: this may
  // disapper during Enqueue if it was successful!
  virtual cl_err_code EnqueueSelf(cl_bool bBlocking,
                                  cl_uint uNumEventsInWaitList,
                                  const cl_event *cpEeventWaitList,
                                  cl_event *pEvent, ApiLogger *apiLogger);

  // Prefix and Postfix Runtime commands
  // Each command may schedule prefix and postfix runtime commands for itself.
  // Such commands are invisible for users and are logical part of the main
  // command that should be executed by RunTime.
  //   Prefix command is executed Before main command is scheduled to device
  //   agent Postfix command is executed After main command signals completion
  // This commands may be long and are executed by task executor
  virtual cl_err_code PrefixExecute() { return CL_SUCCESS; }
  virtual cl_err_code PostfixExecute() { return CL_SUCCESS; }

  // Returns whether this command has been created dependent on events that need
  // to complete before it can be executed
  virtual bool IsDependentOnEvents() const { return false; }

  // Debug functions
  virtual const char *GetCommandName() const {
    return getCommandName(m_commandType);
  }

  // GPA related functions
  virtual ocl_gpa_command *GPA_GetCommand() { return m_pGpaCommand; }
  virtual void GPA_InitCommand();
  virtual void GPA_DestroyCommand();
  virtual void GPA_WriteCommandMetadata() {}
  virtual const char *GPA_GetCommandName() const {
    return getCommandNameGPA(m_commandType);
  }

  /**
   * @return whether this Command is already being deleted (useful for m_Event,
   * which when destroyed deletes its Command, which is usually the one that
   * contains it)
   */
  bool IsBeingDeleted() const { return m_bIsBeingDeleted; }

protected:
  // call this to break event<>command sharedPtr loop and initiate command
  // deletion
  void DetachEventSharedPtr();

  void UnregisterUSMFreeWaitEvent();

  // retrieve device specific descriptor of the memory object.
  // If descriptor is not ready on a device:
  //  1. The descriptor value will be set with NULL
  //  2. additional event will be added to dependency list
  //  3. On resolution the provided memory location will be update with device
  //  descriptor value
  cl_err_code GetMemObjectDescriptor(const SharedPtr<MemoryObject> &pMemObj,
                                     IOCLDevMemoryObject **ppDevMemObj);

  // AcquireMemoryObjects() brings required memory objects to the target device
  // and lock them there Must be called from Execute() and accompanied by call
  // to RelinquishMemoryObjects during CommandDone(). If memory objects are not
  // ready, adds new events to dependency and returns CL_NOT_READY Subsequent
  // calls to AcquireMemoryObjects() will do nothing and always return
  // CL_SUCCESS
  struct MemoryObjectArg {
    SharedPtr<MemoryObject> pMemObj;
    MemoryObject::MemObjUsage access_rights;
    MemoryObject::MemObjUsage access_rights_realy_used;

    MemoryObjectArg(const SharedPtr<MemoryObject> &a,
                    MemoryObject::MemObjUsage b)
        : pMemObj(a), access_rights(b), access_rights_realy_used(b){};
    MemoryObjectArg()
        : pMemObj(nullptr), access_rights(MemoryObject::MEMOBJ_USAGES_COUNT),
          access_rights_realy_used(MemoryObject::MEMOBJ_USAGES_COUNT){};
    MemoryObjectArg(const MemoryObjectArg &other)
        : pMemObj(other.pMemObj), access_rights(other.access_rights),
          access_rights_realy_used(other.access_rights_realy_used) {}
  };

  typedef vector<MemoryObjectArg> MemoryObjectArgList;

  static void
  AddToMemoryObjectArgList(MemoryObjectArgList &argList, MemoryObject *pMemObj,
                           MemoryObject::MemObjUsage access_rights) {
    argList.resize(argList.size() + 1);
    MemoryObjectArg &arg = argList.back();
    arg.pMemObj = pMemObj;
    arg.access_rights = access_rights;
  }

  static void
  AddToMemoryObjectArgList(MemoryObjectArgList &argList,
                           const SharedPtr<MemoryObject> &pMemObj,
                           MemoryObject::MemObjUsage access_rights) {
    AddToMemoryObjectArgList(argList, pMemObj.GetPtr(), access_rights);
  }

  cl_err_code
  AcquireMemoryObjects(MemoryObjectArgList &argList,
                       const SharedPtr<FissionableDevice> &pDev = nullptr);

  void
  RelinquishMemoryObjects(MemoryObjectArgList &argList,
                          const SharedPtr<FissionableDevice> &pDev = nullptr);

  void prepare_command_descriptor(cl_dev_cmd_type type, void *params,
                                  size_t params_size);

  SharedPtr<QueueEvent> m_Event; // An associated event object

  cl_dev_cmd_desc m_DevCmd;         // Device command descriptor struct
  cl_dev_cmd_list m_clDevCmdListId; // An handle of the device command list that
                                    // this command should be queued on
  SharedPtr<FissionableDevice>
      m_pDevice; // A pointer to the device executing the command
  SharedPtr<IOclCommandQueueBase>
      m_pCommandQueue; // A pointer to the command queue on which the command
                       // resides
  cl_int m_returnCode; // The result of the completed command. Can be CL_SUCCESS
                       // or one of the errors defined by the spec.
  cl_command_type m_commandType; // Command type

  ocl_gpa_command *m_pGpaCommand;
  bool m_bIsBeingDeleted; // Command destructor is active - to be check during
                          // internal event destructor
  volatile bool m_bEventDetached; // event already detached from the command

  // Intermediate data
  MemoryObjectArgList m_MemOclObjects;

  ContextModule *m_pContextModule;
  // A list of pointers to USM whose free may be blocked by this command.
  std::vector<const void *> m_UsmPtrs;
  std::mutex m_UsmPtrsMutex;

  DECLARE_LOGGER_CLIENT;

private:
  // disable possibility to create two instances of Command with the same logger
  // pointer.
  Command(const Command &s);
  Command &operator=(const Command &s);
  // return CL_SUCCESS if ready and succeeded, CL_NOT_READY if not ready yet and
  // succeeded, other error code in case of error
  cl_err_code
  AcquireSingleMemoryObject(MemoryObjectArg &arg,
                            const SharedPtr<FissionableDevice> &pDev);

  bool m_memory_objects_acquired;
};

/**
 * This class represents a shared pointer for Command objects. It exposes an
 * interface of SmartPtr<COMMAND_TYPE>, but performs reference of its
 * QueueEvent.
 *
 * @param COMMAND_TYPE the type of pointed to Command
 */
template <typename COMMAND_TYPE = Command>
class CommandSharedPtr : public SmartPtr<COMMAND_TYPE> {

public:
  /**
   * Constructor
   * @param pCommand a pointer to the Command
   */
  CommandSharedPtr(COMMAND_TYPE *pCommand)
      : SmartPtr<COMMAND_TYPE>(pCommand),
        m_pQueueEvent(pCommand ? pCommand->GetEvent().GetPtr() : nullptr) {}
  /**
   * Copy constructor
   */
  CommandSharedPtr(const CommandSharedPtr &command)
      : SmartPtr<COMMAND_TYPE>(command.GetPtr()),
        m_pQueueEvent(command ? command->GetEvent().GetPtr() : nullptr) {}

  /**
   * Assignment operator
   */
  CommandSharedPtr &operator=(const CommandSharedPtr &command) {
    this->m_ptr = command.GetPtr();
    m_pQueueEvent = (command ? command->GetEvent().GetPtr() : nullptr);
    return *this;
  }

private:
  SharedPtr<QueueEvent> m_pQueueEvent;
};

class ReadGVCommand : public Command {
public:
  ReadGVCommand(const SharedPtr<IOclCommandQueueBase> &cmdQueue, void *pDst,
                const void *pSrc, size_t size);

  ~ReadGVCommand();

  cl_err_code Init() override { return CL_SUCCESS; }

  cl_err_code Execute() override;

  cl_err_code CommandDone() override { return CL_SUCCESS; }

  ECommandExecutionType GetExecutionType() const override {
    return DEVICE_EXECUTION_TYPE;
  }

  const char *GetCommandName() const override {
    return "CL_COMMAND_READ_GLOBAL_VARIABLE_INTEL";
  }

protected:
  void *m_pDst;
  const void *m_pSrc;
  size_t m_size;
};

class WriteGVCommand : public Command {
public:
  WriteGVCommand(const SharedPtr<IOclCommandQueueBase> &cmdQueue, void *pDst,
                 const void *pSrc, size_t size);

  ~WriteGVCommand();

  cl_err_code Init() override { return CL_SUCCESS; }

  cl_err_code Execute() override;

  cl_err_code CommandDone() override { return CL_SUCCESS; }

  ECommandExecutionType GetExecutionType() const override {
    return DEVICE_EXECUTION_TYPE;
  }

  const char *GetCommandName() const override {
    return "CL_COMMAND_WRITE_GLOBAL_VARIABLE_INTEL";
  }

protected:
  void *m_pDst;
  const void *m_pSrc;
  size_t m_size;
};

class MemoryCommand : public Command {
public:
  MemoryCommand(const SharedPtr<IOclCommandQueueBase> &cmdQueue)
      : Command(cmdQueue) {}

protected:
  cl_dev_cmd_param_rw m_rwParams;

  void create_dev_cmd_rw(cl_uint uiDimCount, void *pPtr,
                         const size_t *pszMemObjOrigin,
                         const size_t *pszPtrOrigin, const size_t *pszRegion,
                         size_t szPtrRowPitch, size_t szPtrSlicePitch,
                         size_t szMemObjRowPitch, size_t szMemObjSlicePitch,
                         cl_dev_cmd_type clCmdType);
};

/******************************************************************
 *
 ******************************************************************/
class ReadMemObjCommand : public MemoryCommand {
public:
  ReadMemObjCommand(const SharedPtr<IOclCommandQueueBase> &cmdQueue,
                    ocl_entry_points *pOclEntryPoints,
                    const SharedPtr<MemoryObject> &pMemObj,
                    const size_t *pszOrigin, const size_t *pszRegion,
                    size_t szRowPitch, size_t szSlicePitch, void *pDst,
                    const size_t *pszDstOrigin = nullptr,
                    const size_t szDstRowPitch = 0,
                    const size_t szDstSlicePitch = 0);

  virtual ~ReadMemObjCommand();

  virtual ECommandExecutionType GetExecutionType() const override {
    return m_commandType != CL_COMMAND_MARKER ? DEVICE_EXECUTION_TYPE
                                              : RUNTIME_EXECUTION_TYPE;
  }

  virtual cl_err_code Init() override;
  virtual cl_err_code Execute() override;
  virtual cl_err_code CommandDone() override;

protected:
  size_t m_szOrigin[MAX_WORK_DIM];
  size_t m_szRegion[MAX_WORK_DIM];
  size_t m_szMemObjRowPitch;
  size_t m_szMemObjSlicePitch;
  void *m_pDst;
  size_t m_szDstOrigin[MAX_WORK_DIM];
  size_t m_szDstRowPitch;
  size_t m_szDstSlicePitch;
};

/******************************************************************
 *
 ******************************************************************/
class ReadBufferRectCommand : public ReadMemObjCommand {
public:
  ReadBufferRectCommand(const SharedPtr<IOclCommandQueueBase> &cmdQueue,
                        ocl_entry_points *pOclEntryPoints,
                        const SharedPtr<MemoryObject> &pBuffer,
                        const size_t szBufferOrigin[MAX_WORK_DIM],
                        const size_t szDstOrigin[MAX_WORK_DIM],
                        const size_t szRegion[MAX_WORK_DIM],
                        const size_t szBufferRowPitch,
                        const size_t szBufferSlicePitch,
                        const size_t szDstRowPitch,
                        const size_t szDstSlicePitch, void *pDst);
  virtual ~ReadBufferRectCommand();
};

class ReadBufferCommand : public ReadMemObjCommand {
public:
  ReadBufferCommand(const SharedPtr<IOclCommandQueueBase> &cmdQueue,
                    ocl_entry_points *pOclEntryPoints,
                    const SharedPtr<MemoryObject> &pBuffer,
                    const size_t pszOffset[MAX_WORK_DIM],
                    const size_t pszCb[MAX_WORK_DIM], void *pDst);
  virtual ~ReadBufferCommand();
};

class ReadSvmBufferCommand : public ReadBufferCommand {
public:
  ReadSvmBufferCommand(const SharedPtr<IOclCommandQueueBase> &cmdQueue,
                       ocl_entry_points *pOclEntryPoints,
                       const SharedPtr<MemoryObject> &pBuffer,
                       const size_t pszOffset[MAX_WORK_DIM],
                       const size_t pszCb[MAX_WORK_DIM], void *pDst)
      : ReadBufferCommand(cmdQueue, pOclEntryPoints, pBuffer, pszOffset, pszCb,
                          pDst) {
    m_commandType = CL_COMMAND_SVM_MEMCPY;
  }
};

class ReadUsmBufferCommand : public ReadBufferCommand {
public:
  ReadUsmBufferCommand(const SharedPtr<IOclCommandQueueBase> &cmdQueue,
                       ocl_entry_points *pOclEntryPoints,
                       const SharedPtr<MemoryObject> &pBuffer,
                       const size_t pszOffset[MAX_WORK_DIM],
                       const size_t pszCb[MAX_WORK_DIM], void *pDst)
      : ReadBufferCommand(cmdQueue, pOclEntryPoints, pBuffer, pszOffset, pszCb,
                          pDst) {
    m_commandType = CL_COMMAND_MEMCPY_INTEL;
  }
};

/******************************************************************
 *
 ******************************************************************/
class ReadImageCommand : public ReadMemObjCommand {

public:
  ReadImageCommand(const SharedPtr<IOclCommandQueueBase> &cmdQueue,
                   ocl_entry_points *pOclEntryPoints,
                   const SharedPtr<MemoryObject> &pImage,
                   const size_t *pszOrigin, const size_t *pszRegion,
                   size_t szRowPitch, size_t szSlicePitch, void *pDst);
  virtual ~ReadImageCommand();
};

/******************************************************************
 *
 ******************************************************************/
class WriteMemObjCommand : public MemoryCommand {

public:
  WriteMemObjCommand(const SharedPtr<IOclCommandQueueBase> &cmdQueue,
                     ocl_entry_points *pOclEntryPoints, cl_bool bBlocking,
                     const SharedPtr<MemoryObject> &pMemObj,
                     const size_t *pszOrigin, const size_t *pszRegion,
                     size_t szMemObjRowPitch, size_t szMemObjSlicePitch,
                     const void *cpSrc, const size_t *pszSrcOrigin = nullptr,
                     const size_t szSrcRowPitch = 0,
                     const size_t szSrcSlicePitch = 0);

  virtual ~WriteMemObjCommand();

  virtual ECommandExecutionType GetExecutionType() const override {
    return DEVICE_EXECUTION_TYPE;
  }

  virtual cl_err_code Init() override;
  virtual cl_err_code Execute() override;
  virtual cl_err_code CommandDone() override;

private:
  size_t m_szOrigin[MAX_WORK_DIM];
  size_t m_szRegion[MAX_WORK_DIM];
  cl_bool m_bBlocking;
  size_t m_szMemObjRowPitch;
  size_t m_szMemObjSlicePitch;
  const void *m_cpSrc;
  size_t m_szSrcOrigin[MAX_WORK_DIM];
  size_t m_szSrcRowPitch;
  size_t m_szSrcSlicePitch;
  void *m_pTempBuffer; // This buffer is used when command is blocking
};

/******************************************************************
 *
 ******************************************************************/
class FillMemObjCommand : public Command {

public:
  /**
   * Multi-dimmensional CTOR, for images.
   *
   * @param cmdQueue
   * @param pOclEntryPoints
   * @param pMemObj
   * @param pszOffset
   * @param pszRegion
   * @param numOfDimms
   * @param pattern
   * @param pattern_size
   */
  FillMemObjCommand(const SharedPtr<IOclCommandQueueBase> &cmdQueue,
                    ocl_entry_points *pOclEntryPoints,
                    const SharedPtr<MemoryObject> &pMemObj,
                    const size_t *pszOffset, const size_t *pszRegion,
                    const cl_uint numOfDimms, const void *pattern,
                    const size_t pattern_size);

  /**
   * 1D CTOR, for buffers.
   *
   * @param cmdQueue
   * @param pOclEntryPoints
   * @param pMemObj
   * @param pszOffset
   * @param pszRegion
   * @param pattern
   * @param pattern_size
   */
  FillMemObjCommand(const SharedPtr<IOclCommandQueueBase> &cmdQueue,
                    ocl_entry_points *pOclEntryPoints,
                    const SharedPtr<MemoryObject> &pMemObj,
                    const size_t pszOffset, const size_t pszRegion,
                    const void *pattern, const size_t pattern_size);

  virtual ~FillMemObjCommand();

  virtual ECommandExecutionType GetExecutionType() const override {
    return DEVICE_EXECUTION_TYPE;
  }

  virtual cl_err_code Init() override;
  virtual cl_err_code Execute() override;
  virtual cl_err_code CommandDone() override;

protected:
  size_t m_szOffset[MAX_WORK_DIM];
  size_t m_szRegion[MAX_WORK_DIM];
  cl_uint m_numOfDimms;

  char m_pattern[MAX_PATTERN_SIZE]; /* pattern for fill */
  size_t m_pattern_size;            /* fill pattern size in bytes */

  cl_dev_cmd_param_fill m_fillCmdParams;

private:
  cl_err_code m_internalErr; /* error logger for CTOR */
};

/******************************************************************
 *
 ******************************************************************/
class WriteBufferCommand : public WriteMemObjCommand {

public:
  WriteBufferCommand(const SharedPtr<IOclCommandQueueBase> &cmdQueue,
                     ocl_entry_points *pOclEntryPoints, cl_bool bBlocking,
                     const SharedPtr<MemoryObject> &pBuffer,
                     const size_t pszOffset[MAX_WORK_DIM],
                     const size_t pszCb[MAX_WORK_DIM], const void *cpSrc);

  virtual ~WriteBufferCommand();
};

class WriteSvmBufferCommand : public WriteBufferCommand {
public:
  WriteSvmBufferCommand(const SharedPtr<IOclCommandQueueBase> &cmdQueue,
                        ocl_entry_points *pOclEntryPoints, cl_bool bBlocking,
                        const SharedPtr<MemoryObject> &pBuffer,
                        const size_t pszOffset[MAX_WORK_DIM],
                        const size_t pszCb[MAX_WORK_DIM], const void *cpSrc)
      : WriteBufferCommand(cmdQueue, pOclEntryPoints, bBlocking, pBuffer,
                           pszOffset, pszCb, cpSrc) {
    m_commandType = CL_COMMAND_SVM_MEMCPY;
  }
};

class WriteUsmBufferCommand : public WriteBufferCommand {
public:
  WriteUsmBufferCommand(const SharedPtr<IOclCommandQueueBase> &cmdQueue,
                        ocl_entry_points *pOclEntryPoints, cl_bool bBlocking,
                        const SharedPtr<MemoryObject> &pBuffer,
                        const size_t pszOffset[MAX_WORK_DIM],
                        const size_t pszCb[MAX_WORK_DIM], const void *cpSrc)
      : WriteBufferCommand(cmdQueue, pOclEntryPoints, bBlocking, pBuffer,
                           pszOffset, pszCb, cpSrc) {
    m_commandType = CL_COMMAND_MEMCPY_INTEL;
  }
};

class FillBufferCommand : public FillMemObjCommand {
public:
  FillBufferCommand(const SharedPtr<IOclCommandQueueBase> &cmdQueue,
                    ocl_entry_points *pOclEntryPoints,
                    const SharedPtr<MemoryObject> &pBuffer, const void *pattern,
                    size_t pattern_size, size_t offset, size_t size);

  virtual ~FillBufferCommand();
};

class FillSvmBufferCommand : public FillBufferCommand {
public:
  FillSvmBufferCommand(const SharedPtr<IOclCommandQueueBase> &cmdQueue,
                       ocl_entry_points *pOclEntryPoints,
                       const SharedPtr<MemoryObject> &pBuffer,
                       const void *pattern, size_t pattern_size, size_t offset,
                       size_t size)
      : FillBufferCommand(cmdQueue, pOclEntryPoints, pBuffer, pattern,
                          pattern_size, offset, size) {
    m_commandType = CL_COMMAND_SVM_MEMFILL;
  }
};

class MemFillUsmBufferCommand : public FillBufferCommand {
public:
  MemFillUsmBufferCommand(const SharedPtr<IOclCommandQueueBase> &cmdQueue,
                          ocl_entry_points *pOclEntryPoints,
                          const SharedPtr<MemoryObject> &pBuffer,
                          const void *pattern, size_t pattern_size,
                          size_t offset, size_t size)
      : FillBufferCommand(cmdQueue, pOclEntryPoints, pBuffer, pattern,
                          pattern_size, offset, size) {
    m_commandType = CL_COMMAND_MEMFILL_INTEL;
  }
};

class WriteBufferRectCommand : public WriteMemObjCommand {
public:
  WriteBufferRectCommand(const SharedPtr<IOclCommandQueueBase> &cmdQueue,
                         ocl_entry_points *pOclEntryPoints, cl_bool bBlocking,
                         const SharedPtr<MemoryObject> &pBuffer,
                         const size_t szBufferOrigin[MAX_WORK_DIM],
                         const size_t szSrcOrigin[MAX_WORK_DIM],
                         const size_t szRegion[MAX_WORK_DIM],
                         const size_t szBufferRowPitch,
                         const size_t szBufferSlicePitch,
                         const size_t szDstRowPitch,
                         const size_t szDstSlicePitch, const void *pDst);
  virtual ~WriteBufferRectCommand();
};

/******************************************************************
 *
 ******************************************************************/
class WriteImageCommand : public WriteMemObjCommand {

public:
  WriteImageCommand(const SharedPtr<IOclCommandQueueBase> &cmdQueue,
                    ocl_entry_points *pOclEntryPoints, cl_bool bBlocking,
                    const SharedPtr<MemoryObject> &pImage,
                    const size_t *pszOrigin, const size_t *pszRegion,
                    size_t szRowPitch, size_t szSlicePitch, const void *cpSrc);

  virtual ~WriteImageCommand();
};

class FillImageCommand : public FillMemObjCommand {
public:
  FillImageCommand(const SharedPtr<IOclCommandQueueBase> &cmdQueue,
                   ocl_entry_points *pOclEntryPoints,
                   const SharedPtr<MemoryObject> &pImg, const void *pattern,
                   size_t pattern_size, const cl_uint num_of_dimms,
                   const size_t *offset, const size_t *size);

  virtual ~FillImageCommand();
};

/******************************************************************
 * This is an abstrct class that is used for all copy memory object commands
 ******************************************************************/
class CopyMemObjCommand : public MemoryCommand {

public:
  CopyMemObjCommand(const SharedPtr<IOclCommandQueueBase> &cmdQueue,
                    ocl_entry_points *pOclEntryPoints,
                    const SharedPtr<MemoryObject> &pSrcMemObj,
                    const SharedPtr<MemoryObject> &pDstMemObj,
                    const size_t *szSrcOrigin, const size_t *szDstOrigin,
                    const size_t *szRegion, const size_t szSrcRowPitch,
                    const size_t szSrcSlicePitch, const size_t szDstRowPitch,
                    const size_t szDstSlicePitch);
  virtual ~CopyMemObjCommand();

  virtual cl_err_code Init() override;
  virtual cl_err_code Execute() override;
  virtual cl_err_code CommandDone() override;

  virtual ECommandExecutionType GetExecutionType() const override {
    return m_commandType != CL_COMMAND_MARKER ? DEVICE_EXECUTION_TYPE
                                              : RUNTIME_EXECUTION_TYPE;
  }

protected:
  SharedPtr<MemoryObject> m_pSrcMemObj;
  SharedPtr<MemoryObject> m_pDstMemObj;
  size_t m_szSrcOrigin[MAX_WORK_DIM];
  size_t m_szDstOrigin[MAX_WORK_DIM];
  size_t m_szRegion[MAX_WORK_DIM];
  cl_uint
      m_uiSrcNumDims; // The dimensions represent the memory object type, 1,2,3
  // respectively are BUFFER/2D/3D. The private member is used only for ease of
  // use.
  cl_uint m_uiDstNumDims;
  cl_dev_cmd_param_copy m_copyParams;

  size_t m_szSrcRowPitch;
  size_t m_szSrcSlicePitch;
  size_t m_szDstRowPitch;
  size_t m_szDstSlicePitch;

  // Private functions
  cl_err_code CopyOnDevice(const SharedPtr<FissionableDevice> &pDevice);
};

/******************************************************************
 *
 ******************************************************************/
class CopyBufferCommand : public CopyMemObjCommand {

public:
  CopyBufferCommand(const SharedPtr<IOclCommandQueueBase> &cmdQueue,
                    ocl_entry_points *pOclEntryPoints,
                    const SharedPtr<MemoryObject> &pSrcBuffer,
                    const SharedPtr<MemoryObject> &pDstBuffer,
                    const size_t szSrcOrigin[MAX_WORK_DIM],
                    const size_t szDstOrigin[MAX_WORK_DIM],
                    const size_t szRegion[MAX_WORK_DIM]);
  virtual ~CopyBufferCommand();
};

class CopySvmBufferCommand : public CopyBufferCommand {
public:
  CopySvmBufferCommand(const SharedPtr<IOclCommandQueueBase> &cmdQueue,
                       ocl_entry_points *pOclEntryPoints,
                       const SharedPtr<MemoryObject> &pSrcBuffer,
                       const SharedPtr<MemoryObject> &pDstBuffer,
                       const size_t szSrcOrigin[MAX_WORK_DIM],
                       const size_t szDstOrigin[MAX_WORK_DIM],
                       const size_t szRegion[MAX_WORK_DIM])
      : CopyBufferCommand(cmdQueue, pOclEntryPoints, pSrcBuffer, pDstBuffer,
                          szSrcOrigin, szDstOrigin, szRegion) {
    m_commandType = CL_COMMAND_SVM_MEMCPY;
  }
};

class CopyUsmBufferCommand : public CopyBufferCommand {
public:
  CopyUsmBufferCommand(const SharedPtr<IOclCommandQueueBase> &cmdQueue,
                       ocl_entry_points *pOclEntryPoints,
                       const SharedPtr<MemoryObject> &pSrcBuffer,
                       const SharedPtr<MemoryObject> &pDstBuffer,
                       const size_t szSrcOrigin[MAX_WORK_DIM],
                       const size_t szDstOrigin[MAX_WORK_DIM],
                       const size_t szRegion[MAX_WORK_DIM])
      : CopyBufferCommand(cmdQueue, pOclEntryPoints, pSrcBuffer, pDstBuffer,
                          szSrcOrigin, szDstOrigin, szRegion) {
    m_commandType = CL_COMMAND_MEMCPY_INTEL;
  }
};

/******************************************************************
 *
 ******************************************************************/
class CopyBufferRectCommand : public CopyMemObjCommand {

public:
  CopyBufferRectCommand(const SharedPtr<IOclCommandQueueBase> &cmdQueue,
                        ocl_entry_points *pOclEntryPoints,
                        const SharedPtr<MemoryObject> &pSrcBuffer,
                        const SharedPtr<MemoryObject> &pDstBuffer,
                        const size_t szSrcOrigin[MAX_WORK_DIM],
                        const size_t szDstOrigin[MAX_WORK_DIM],
                        const size_t szRegion[MAX_WORK_DIM],
                        const size_t szSrcRowPitch,
                        const size_t szSrcSlicePitch,
                        const size_t szDstRowPitch,
                        const size_t szDstSlicePitch);
  virtual ~CopyBufferRectCommand();
};

/******************************************************************
 *
 ******************************************************************/
class CopyImageCommand : public CopyMemObjCommand {

public:
  CopyImageCommand(const SharedPtr<IOclCommandQueueBase> &cmdQueue,
                   ocl_entry_points *pOclEntryPoints,
                   const SharedPtr<MemoryObject> &pSrcImage,
                   const SharedPtr<MemoryObject> &pDstImage,
                   const size_t *pszSrcOrigin, const size_t *pszDstOrigin,
                   const size_t *pszRegion);
  virtual ~CopyImageCommand();
};

/******************************************************************
 *
 ******************************************************************/
class CopyImageToBufferCommand : public CopyMemObjCommand {
public:
  CopyImageToBufferCommand(const SharedPtr<IOclCommandQueueBase> &cmdQueue,
                           ocl_entry_points *pOclEntryPoints,
                           const SharedPtr<MemoryObject> &pSrcImage,
                           const SharedPtr<MemoryObject> &pDstBuffer,
                           const size_t *pszSrcOrigin,
                           const size_t *pszSrcRegion,
                           size_t pszDstOffset[MAX_WORK_DIM]);
  virtual ~CopyImageToBufferCommand();
};

/******************************************************************
 *
 ******************************************************************/
class CopyBufferToImageCommand : public CopyMemObjCommand {

public:
  CopyBufferToImageCommand(const SharedPtr<IOclCommandQueueBase> &cmdQueue,
                           ocl_entry_points *pOclEntryPoints,
                           const SharedPtr<MemoryObject> &pSrcBuffer,
                           const SharedPtr<MemoryObject> &pDstImage,
                           size_t pszSrcOffset[MAX_WORK_DIM],
                           const size_t *pszDstOrigin,
                           const size_t *pszDstRegion);

  virtual ~CopyBufferToImageCommand();
};

/******************************************************************
 *
 ******************************************************************/
class PrePostFixRuntimeCommand;
class MapMemObjCommand : public Command {
public:
  MapMemObjCommand(const SharedPtr<IOclCommandQueueBase> &cmdQueue,
                   ocl_entry_points *pOclEntryPoints,
                   const SharedPtr<MemoryObject> &pMemObj,
                   cl_map_flags clMapFlags, const size_t *pOrigin,
                   const size_t *pRegion, size_t *pszImageRowPitch,
                   size_t *pszImageSlicePitch);
  virtual ~MapMemObjCommand();

  virtual cl_err_code Init() override;
  virtual cl_err_code Execute() override;
  virtual cl_err_code CommandDone() override;

  ECommandExecutionType GetExecutionType() const override {
    return m_ExecutionType;
  }

  virtual cl_err_code EnqueueSelf(cl_bool bBlocking,
                                  cl_uint uNumEventsInWaitList,
                                  const cl_event *cpEeventWaitList,
                                  cl_event *pEvent,
                                  ApiLogger *apiLogger) override;
  virtual cl_err_code PostfixExecute() override;

  // Object only function
  void *GetMappedPtr() const { return m_pHostDataPtr; }

protected:
  cl_map_flags m_clMapFlags;
  size_t m_szOrigin[MAX_WORK_DIM];
  size_t m_szRegion[MAX_WORK_DIM];
  size_t *m_pszImageRowPitch;
  size_t *m_pszImageSlicePitch;
  cl_dev_cmd_param_map *m_pMappedRegion;
  void *m_pHostDataPtr;
  SharedPtr<FissionableDevice> m_pActualMappingDevice;
  ECommandExecutionType m_ExecutionType;

  // postfix-related. Created in init, pointer zeroed at enqueue.
  ocl_entry_points *m_pOclEntryPoints;
  PrePostFixRuntimeCommand *m_pPostfixCommand;
  bool m_bResourcesAllocated;

private:
  MapMemObjCommand(const MapMemObjCommand &);
  MapMemObjCommand &operator=(const MapMemObjCommand &);
};

/******************************************************************
 *
 ******************************************************************/
class MapBufferCommand : public MapMemObjCommand {

public:
  MapBufferCommand(const SharedPtr<IOclCommandQueueBase> &cmdQueue,
                   ocl_entry_points *pOclEntryPoints,
                   const SharedPtr<MemoryObject> &pBuffer,
                   cl_map_flags clMapFlags, size_t szOffset, size_t szCb);
  virtual ~MapBufferCommand();
};

class MapSvmBufferCommand : public MapBufferCommand {
public:
  MapSvmBufferCommand(const SharedPtr<IOclCommandQueueBase> &cmdQueue,
                      ocl_entry_points *pOclEntryPoints,
                      const SharedPtr<MemoryObject> &pBuffer,
                      cl_map_flags clMapFlags, size_t szOffset, size_t szCb)
      : MapBufferCommand(cmdQueue, pOclEntryPoints, pBuffer, clMapFlags,
                         szOffset, szCb) {
    m_commandType = CL_COMMAND_SVM_MAP;
  }
};

/******************************************************************
 *
 ******************************************************************/
class MapImageCommand : public MapMemObjCommand {
public:
  MapImageCommand(const SharedPtr<IOclCommandQueueBase> &cmdQueue,
                  ocl_entry_points *pOclEntryPoints,
                  const SharedPtr<MemoryObject> &pImage,
                  cl_map_flags clMapFlags, const size_t *pOrigin,
                  const size_t *pRegion, size_t *pszImageRowPitch,
                  size_t *pszImageSlicePitch);
  virtual ~MapImageCommand();
};

/******************************************************************
 *
 ******************************************************************/
class UnmapMemObjectCommand : public Command {
public:
  UnmapMemObjectCommand(const SharedPtr<IOclCommandQueueBase> &cmdQueue,
                        ocl_entry_points *pOclEntryPoints,
                        const SharedPtr<MemoryObject> &pMemObject,
                        void *pMappedRegion);
  virtual ~UnmapMemObjectCommand();

  cl_err_code Init() override;
  cl_err_code Execute() override;
  cl_err_code CommandDone() override;

  cl_err_code EnqueueSelf(cl_bool bBlocking, cl_uint uNumEventsInWaitList,
                          const cl_event *cpEeventWaitList, cl_event *pEvent,
                          ApiLogger *apiLogger) override;
  cl_err_code PrefixExecute() override;

  ECommandExecutionType GetExecutionType() const override {
    return m_ExecutionType;
  }

private:
  void *m_pMappedPtr;
  cl_dev_cmd_param_map *m_pMappedRegion;
  SharedPtr<FissionableDevice> m_pActualMappingDevice;
  ECommandExecutionType m_ExecutionType;

  // prefix-related. Created in init, pointer zeroed at enqueue.
  PrePostFixRuntimeCommand *m_pPrefixCommand;
  ocl_entry_points *m_pOclEntryPoints;
  bool m_bResourcesAllocated;
  UnmapMemObjectCommand(const UnmapMemObjectCommand &);
  UnmapMemObjectCommand &operator=(const UnmapMemObjectCommand &);
};

class UnmapSvmBufferCommand : public UnmapMemObjectCommand {
public:
  UnmapSvmBufferCommand(const SharedPtr<IOclCommandQueueBase> &cmdQueue,
                        ocl_entry_points *pOclEntryPoints,
                        const SharedPtr<MemoryObject> &pMemObject,
                        void *pMappedRegion)
      : UnmapMemObjectCommand(cmdQueue, pOclEntryPoints, pMemObject,
                              pMappedRegion) {
    m_commandType = CL_COMMAND_SVM_UNMAP;
  }
};

/******************************************************************
 *
 ******************************************************************/
class NDRangeKernelCommand : public Command {
public:
  NDRangeKernelCommand(const SharedPtr<IOclCommandQueueBase> &cmdQueue,
                       ocl_entry_points *pOclEntryPoints,
                       const SharedPtr<Kernel> &pKernel, cl_uint uWorkDim,
                       const size_t *szGlobalWorkOffset,
                       const size_t *szGlobalWorkSize,
                       const size_t *szLocalWorkSize);
  virtual ~NDRangeKernelCommand();

  virtual cl_err_code Init() override;
  virtual cl_err_code Execute() override;
  virtual cl_err_code CommandDone() override;
  ECommandExecutionType GetExecutionType() const override {
    return DEVICE_EXECUTION_TYPE;
  }

  // GPA related functions
  virtual const char *GPA_GetCommandName() const override {
    return (CL_COMMAND_NDRANGE_KERNEL == m_commandType)
               ? m_pKernel->GetName()
               : Command::GPA_GetCommandName();
  }
  virtual void GPA_WriteCommandMetadata() override;

  using CommandCallBackFn = std::function<void(cl_event)>;

  /// Set a callback function which will be called when FPGA pipe kernel
  /// command (requires kernel serialization) is done.
  void SetFPGASerializeCompleteCallBack(CommandCallBackFn F) {
    FPGASerializeCompleteCallBackFunc = F;
  }

  /// Return true if FPGA pipe kernel command complete callback is set.
  bool HasFPGASerializeCompleteCallBack() const {
    return (bool)FPGASerializeCompleteCallBackFunc;
  }

protected:
  cl_dev_cmd_param_kernel m_kernelParams;
  // Private members
  SharedPtr<Kernel> m_pKernel;
  const DeviceKernel *m_pDeviceKernel;
  cl_uint m_uiWorkDim;
  const size_t *m_cpszGlobalWorkOffset;
  const size_t *m_cpszGlobalWorkSize;
  const size_t *m_cpszLocalWorkSize;

  std::vector<IOCLDevMemoryObject *> m_nonArgSvmBuffersVec;
  std::vector<IOCLDevMemoryObject *> m_nonArgUsmBuffersVec;
  // Record device descriptor of buffers in order to release them
  std::vector<IOCLDevMemoryObject *> m_argDevDescMemObjects;
#if defined(USE_ITT)
  void GPA_WriteWorkMetadata(const size_t *pWorkMetadata,
                             __itt_string_handle *keyStrHandle) const;
#endif

  // Callback function that will be called when this command is done and
  // this command is restricted by kernel serialization due to FPGA pipe.
  // Refer to ExecutionModule::EnqueueNDRangeKernel for details.
  CommandCallBackFn FPGASerializeCompleteCallBackFunc;
};

/******************************************************************
 *
 ******************************************************************/
class TaskCommand : public NDRangeKernelCommand {

public:
  TaskCommand(const SharedPtr<IOclCommandQueueBase> &cmdQueue,
              ocl_entry_points *pOclEntryPoints,
              const SharedPtr<Kernel> &pKernel);
  virtual ~TaskCommand();

  // Override Init only to set a different device type
  cl_err_code Init() override;
  ECommandExecutionType GetExecutionType() const override {
    return DEVICE_EXECUTION_TYPE;
  }

private:
  size_t m_szStaticWorkSize; // Set to 1 to support NDRangeKernel execution with
                             // work_size=1
};

/******************************************************************
 *
 ******************************************************************/
class NativeKernelCommand : public Command {

public:
  typedef void(CL_CALLBACK *pUserFnc_t)(void *);
  NativeKernelCommand(const SharedPtr<IOclCommandQueueBase> &cmdQueue,
                      ocl_entry_points *pOclEntryPoints, pUserFnc_t pUserFnc,
                      void *pArgs, size_t szCbArgs, cl_uint uNumMemObjects,
                      SharedPtr<MemoryObject> *ppMemObjList,
                      const void **ppArgsMemLoc);
  virtual ~NativeKernelCommand();

  cl_err_code Init() override;
  cl_err_code Execute() override;
  cl_err_code CommandDone() override;
  ECommandExecutionType GetExecutionType() const override {
    return DEVICE_EXECUTION_TYPE;
  }

protected:
  cl_dev_cmd_param_native m_nativeParams;

private:
  pUserFnc_t m_pUserFnc;
  void *m_pArgs;
  size_t m_szCbArgs;
  cl_uint m_uNumMemObjects;
  SharedPtr<MemoryObject> *m_ppMemObjList;
  const void **m_ppArgsMemLoc;
};

class MigrateSVMMemCommand : public Command {

public:
  MigrateSVMMemCommand(const SharedPtr<IOclCommandQueueBase> &cmdQueue,
                       cl_mem_migration_flags clFlags, cl_uint uNumMemObjects,
                       const void **pMemObject, const size_t *sizes);

  virtual ~MigrateSVMMemCommand();

  virtual ECommandExecutionType GetExecutionType() const override {
    return DEVICE_EXECUTION_TYPE;
  }

  virtual cl_err_code Init() override;
  virtual cl_err_code Execute() override;
  virtual cl_err_code CommandDone() override;

protected:
  const void *
      *m_pMemObjects; // used temporary to pass info from contructor to init()
  const size_t *m_pSizes;

  cl_dev_cmd_param_migrate m_migrateCmdParams;
};

/******************************************************************
 *
 ******************************************************************/
class MigrateMemObjCommand : public Command {

public:
  MigrateMemObjCommand(const SharedPtr<IOclCommandQueueBase> &cmdQueue,
                       ocl_entry_points *pOclEntryPoints,
                       cl_mem_migration_flags clFlags, cl_uint uNumMemObjects,
                       const cl_mem *pMemObjects);

  virtual ~MigrateMemObjCommand();

  virtual ECommandExecutionType GetExecutionType() const override {
    return DEVICE_EXECUTION_TYPE;
  }

  virtual cl_err_code Init() override;
  virtual cl_err_code Execute() override;
  virtual cl_err_code CommandDone() override;

protected:
  const cl_mem
      *m_pMemObjects; // used temporary to pass info from contructor to init()

  cl_dev_cmd_param_migrate m_migrateCmdParams;
};

/******************************************************************
     *
 * MigrateUSMMemCommand explicitly migrates a region of a shared
     * USM
 * allocation to the device associated with the command_queue.

 * ******************************************************************/
class MigrateUSMMemCommand : public Command {

public:
  MigrateUSMMemCommand(const SharedPtr<IOclCommandQueueBase> &cmdQueue,
                       cl_mem_migration_flags_intel clFlags, const void *ptr,
                       size_t size);

  virtual ~MigrateUSMMemCommand() {}

  virtual ECommandExecutionType GetExecutionType() const override {
    return DEVICE_EXECUTION_TYPE;
  }

  virtual cl_err_code Init() override;
  virtual cl_err_code Execute() override;
  virtual cl_err_code CommandDone() override;

protected:
  const void *m_ptr;

  cl_dev_cmd_param_migrate_usm m_migrateCmdParams;
};

/******************************************************************
     *

 * ******************************************************************/
class AdviseUSMMemCommand : public Command {
public:
  AdviseUSMMemCommand(const SharedPtr<IOclCommandQueueBase> &cmdQueue,
                      const void *ptr, size_t size, cl_mem_advice_intel advice);

  virtual ~AdviseUSMMemCommand() {}

  virtual ECommandExecutionType GetExecutionType() const override {
    return DEVICE_EXECUTION_TYPE;
  }

  virtual cl_err_code Init() override;
  virtual cl_err_code Execute() override;
  virtual cl_err_code CommandDone() override;

protected:
  const void *m_ptr;

  cl_dev_cmd_param_advise_usm m_adviseCmdParams;
};

/******************************************************************
 * Runtime command is a command that was created by the runtime
 * and is used for sync within the runtime.
 * The command does nothing but keep the event mechanism and therefore can be
 *use for synch Implementation may use it for Flush or Finish commands or
 *marker/barrier etc.
 ******************************************************************/
class RuntimeCommand : public Command {
public:
  RuntimeCommand(const SharedPtr<IOclCommandQueueBase> &cmdQueue,
                 bool bIsDependentOnEvents = false)
      : Command(cmdQueue), m_bIsDependentOnEvents(bIsDependentOnEvents) {
    m_commandType = CL_COMMAND_RUNTIME;
  }
  virtual ~RuntimeCommand() {}
  virtual cl_err_code Init() override { return CL_SUCCESS; }
  virtual cl_err_code Execute() override;
  virtual cl_err_code CommandDone() override { return CL_SUCCESS; }
  virtual ECommandExecutionType GetExecutionType() const override {
    return RUNTIME_EXECUTION_TYPE;
  }
  virtual bool isControlCommand() const override { return true; }
  virtual bool IsDependentOnEvents() const override {
    return m_bIsDependentOnEvents;
  }

private:
  const bool m_bIsDependentOnEvents;
};

/******************************************************************
 *
 ******************************************************************/
class MarkerCommand : public RuntimeCommand {

public:
  MarkerCommand(const SharedPtr<IOclCommandQueueBase> &cmdQueue,
                bool bIsDependentOnEvents)
      : RuntimeCommand(cmdQueue, bIsDependentOnEvents) {
    m_commandType = CL_COMMAND_MARKER;
  }
  virtual ~MarkerCommand() {}
};

/******************************************************************
 *
 ******************************************************************/
class WaitForEventsCommand : public RuntimeCommand {

public:
  WaitForEventsCommand(const SharedPtr<IOclCommandQueueBase> &cmdQueue,
                       bool bIsDependentOnEvents)
      : RuntimeCommand(cmdQueue, bIsDependentOnEvents) {
    m_commandType = CL_COMMAND_WAIT_FOR_EVENTS;
  }

  virtual ~WaitForEventsCommand() {}
};

/******************************************************************
 *
 ******************************************************************/
class BarrierCommand : public RuntimeCommand {

public:
  BarrierCommand(const SharedPtr<IOclCommandQueueBase> &cmdQueue,
                 bool bIsDependentOnEvents)
      : RuntimeCommand(cmdQueue, bIsDependentOnEvents) {
    m_commandType = CL_COMMAND_BARRIER;
  }
  virtual ~BarrierCommand() {}
};

/******************************************************************
     * The
 * command is used when user map pointers which have not been
     * allocated
 * by clSVMAlloc, e.g. SVM_FINE_GRAIN_SYSTEM.
     * In this case we should just
 * handle event dependency.

 * ******************************************************************/
class SVMMAP_Command_NOOP : public RuntimeCommand {

public:
  SVMMAP_Command_NOOP(const SharedPtr<IOclCommandQueueBase> &cmdQueue,
                      bool bIsDependentOnEvents)
      : RuntimeCommand(cmdQueue, bIsDependentOnEvents) {
    m_commandType = CL_COMMAND_SVM_MAP;
  }
  virtual ~SVMMAP_Command_NOOP() {}
};

/******************************************************************
     * The
 * command is used when user map pointers which have not been
     * allocated
 * by clSVMAlloc, e.g. SVM_FINE_GRAIN_SYSTEM.
     * In this case we should just
 * handle event dependency.

 * ******************************************************************/
class SVMUNMAP_Command_NOOP : public RuntimeCommand {

public:
  SVMUNMAP_Command_NOOP(const SharedPtr<IOclCommandQueueBase> &cmdQueue,
                        bool bIsDependentOnEvents)
      : RuntimeCommand(cmdQueue, bIsDependentOnEvents) {
    m_commandType = CL_COMMAND_SVM_UNMAP;
  }
  virtual ~SVMUNMAP_Command_NOOP() {}
};

/******************************************************************
 *
 * Special internal Runtime commands to perform some async action before/after
 *normal command
 *
 ******************************************************************/
class ErrorQueueEvent : public OclEvent {
public:
  PREPARE_SHARED_PTR(ErrorQueueEvent)

  static SharedPtr<ErrorQueueEvent> Allocate(_cl_context_int *context) {
    return SharedPtr<ErrorQueueEvent>(new ErrorQueueEvent(context));
  }

  void Init(PrePostFixRuntimeCommand *owner) { m_owner = owner; }

  // Override to notify my command about failed events it depended on
  virtual cl_err_code
  ObservedEventStateChanged(const SharedPtr<OclEvent> &pEvent,
                            cl_int returnCode) override;

  // Get the return code of the command associated with the event.
  virtual cl_int GetReturnCode() const override;

  virtual cl_err_code GetInfo(cl_int iParamName, size_t szParamValueSize,
                              void *pParamValue,
                              size_t *pszParamValueSizeRet) const override;

private:
  ErrorQueueEvent(_cl_context_int *context)
      : OclEvent(context), m_owner(nullptr){};

  PrePostFixRuntimeCommand *m_owner;
};

class RuntimeCommandTask : public Intel::OpenCL::TaskExecutor::ITask {
public:
  PREPARE_SHARED_PTR(RuntimeCommandTask)

  static SharedPtr<RuntimeCommandTask> Allocate() {
    return SharedPtr<RuntimeCommandTask>(new RuntimeCommandTask());
  }

  void Init(PrePostFixRuntimeCommand *owner) { m_owner = owner; }

  // ITask interface
  bool SetAsSyncPoint() override;
  bool IsCompleted() const override { return m_bIsCompleted; }
  bool CompleteAndCheckSyncPoint() override;
  bool Execute() override;
  void Cancel() override;
  long Release() override;

  Intel::OpenCL::TaskExecutor::TASK_PRIORITY GetPriority() const override {
    return Intel::OpenCL::TaskExecutor::TASK_PRIORITY_MEDIUM;
  }

  virtual Intel::OpenCL::TaskExecutor::IThreadLibTaskGroup *
  GetNDRangeChildrenTaskGroup() override {
    return nullptr;
  }

private:
  RuntimeCommandTask() : m_owner(nullptr){};

  CommandSharedPtr<PrePostFixRuntimeCommand> m_owner;
  bool m_bIsCompleted;
};

class PrePostFixRuntimeCommand : public RuntimeCommand {
public:
  enum Mode { PREFIX_MODE = 0, POSTFIX_MODE };

  PrePostFixRuntimeCommand(Command *relatedUserCommand, Mode working_mode,
                           const SharedPtr<IOclCommandQueueBase> &cmdQueue);

  cl_err_code Init() override;
  cl_err_code Execute() override;
  cl_err_code CommandDone() override;

  // called possibly from another thread
  void DoAction();
  void CancelAction();

  // called by "related" command if enqueue was unsuccessful
  void ErrorDone();
  void ErrorEnqueue(cl_event *intermediate_pEvent, cl_event *user_pEvent,
                    cl_err_code err_to_force_return);
  cl_err_code GetForcedErrorCode() const { return m_force_error_return; };

  cl_command_type GetCommandType() const override {
    return m_relatedUserCommand->GetCommandType();
  };
  const char *GetCommandName() const override {
    if (m_working_mode == PREFIX_MODE) {
      return "PreFixRuntimeCommand";
    } else {
      return "PostFixRuntimeCommand";
    }
  };

private:
  CommandSharedPtr<> m_relatedUserCommand;
  Mode m_working_mode;
  cl_err_code m_force_error_return;
  SharedPtr<ErrorQueueEvent> m_error_event;
  SharedPtr<RuntimeCommandTask> m_task;
};

} // namespace Framework
} // namespace OpenCL
} // namespace Intel
