// INTEL CONFIDENTIAL
//
// Copyright 2009 Intel Corporation.
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

///////////////////////////////////////////////////////////
//  dispatcher_commands.h
//  Declaration of internal task dispatcher commands
////////////////////////////////////////////////////////////

#pragma once

#include "cl_device_api.h"
#include "cl_sys_defines.h"
#include "cl_thread.h"
#include "cpu_dev_limits.h"
#include "kernel_command.h"
#include "memory_allocator.h"
#include "ocl_itt.h"
#include "program_service.h"
#include "task_executor.h"
#ifdef __USE_TBB_SCALABLE_ALLOCATOR__
#include "tbb/scalable_allocator.h"
#endif
#include <atomic>

#define COLOR_TABLE_SIZE 64

namespace Intel {
namespace OpenCL {
namespace BuiltInKernels {
class IBuiltInKernel;
}
} // namespace OpenCL
} // namespace Intel

namespace Intel {
namespace OpenCL {
namespace CPUDevice {

class TaskDispatcher;

typedef cl_dev_err_code
fnDispatcherCommandCreate_t(TaskDispatcher *pTD, cl_dev_cmd_desc *pCmd,
                            SharedPtr<ITaskBase> *pTask,
                            const SharedPtr<ITaskList> &pList);

// Base class for handling dispatcher command execution
// All Commands will be implement this interface
class DispatcherCommand {
public:
  DispatcherCommand(TaskDispatcher *pTD, cl_dev_cmd_desc *pCmd);
  virtual ~DispatcherCommand();

  /**
   * @return the DispatcherCommand's TaskDispatcher
   */
  TaskDispatcher *GetTaskDispatcher() { return m_pTaskDispatcher; }

protected:
  virtual void NotifyCommandStatusChanged(cl_dev_cmd_desc *cmd,
                                          unsigned uStatus, int iErr);

  cl_dev_err_code ExtractNDRangeParams(
      void *pTargetTaskParam, const llvm::KernelArgument *pParams,
      const unsigned int *pMemObjectIndx, unsigned int uiMemObjCount,
      std::vector<cl_mem_obj_descriptor *> *devMemObjects,
      std::vector<char> *kernelParamsVec);

  TaskDispatcher *m_pTaskDispatcher;
  IOCLDevLogDescriptor *m_pLogDescriptor;
  cl_int m_iLogHandle;
  cl_dev_cmd_desc *m_pCmd;
  ocl_gpa_data *m_pGPAData;
#if defined(USE_ITT)
  __itt_id m_ittID;
#endif

  volatile bool m_bCompleted;
};

template <class ITaskClass>
class CommandBaseClass : public ITaskClass, public DispatcherCommand {
public:
  CommandBaseClass(TaskDispatcher *pTD, cl_dev_cmd_desc *pCmd)
      : DispatcherCommand(pTD, pCmd), m_isSyncPoint(false),
        m_completedSync(false) {}

  ~CommandBaseClass() {}

  // ITaskBase
  bool SetAsSyncPoint() override;
  bool CompleteAndCheckSyncPoint() override;
  bool IsCompleted() const override { return m_bCompleted; }
  long Release() override { return 0; }
  TASK_PRIORITY GetPriority() const override {
    return TaskExecutor::TASK_PRIORITY_MEDIUM;
  }

  void Cancel() override {
    NotifyCommandStatusChanged(m_pCmd, CL_COMPLETE,
                               cl_int(CL_DEV_COMMAND_CANCELLED));
  }
  Intel::OpenCL::TaskExecutor::IThreadLibTaskGroup *
  GetNDRangeChildrenTaskGroup() override {
    return nullptr;
  }

protected:
  std::atomic<bool> m_isSyncPoint;
  std::atomic<bool> m_completedSync;
};

// OCL Read/Write buffer execution
class ReadWriteMemObject : public CommandBaseClass<ITask> {
public:
  static cl_dev_err_code Create(TaskDispatcher *pTD, cl_dev_cmd_desc *pCmd,
                                SharedPtr<ITaskBase> *pTask,
                                const SharedPtr<ITaskList> &pList);

  // ITask interface
  bool Execute() override;

protected:
  ReadWriteMemObject(TaskDispatcher *pTD, cl_dev_cmd_desc *pCmd);
  cl_dev_err_code CheckCommandParams(cl_dev_cmd_desc *cmd);
};

// OCL Copy Mem Obj Command
class CopyMemObject : public CommandBaseClass<ITask> {
public:
  static cl_dev_err_code Create(TaskDispatcher *pTD, cl_dev_cmd_desc *pCmd,
                                SharedPtr<ITaskBase> *pTask,
                                const SharedPtr<ITaskList> &pList);

  // DispatcherCommand interface
  cl_dev_err_code CheckCommandParams(cl_dev_cmd_desc *cmd);

  // ITask interface
  bool Execute() override;

protected:
  CopyMemObject(TaskDispatcher *pTD, cl_dev_cmd_desc *pCmd);
};

// OCL Native function execution
class NativeFunction : public CommandBaseClass<ITask> {
public:
  static cl_dev_err_code Create(TaskDispatcher *pTD, cl_dev_cmd_desc *pCmd,
                                SharedPtr<ITaskBase> *pTask,
                                const SharedPtr<ITaskList> &pList);

  // DispatcherCommand interface
  cl_dev_err_code CheckCommandParams(cl_dev_cmd_desc *cmd);

  // ITask interface
  bool Execute() override;

protected:
  NativeFunction(TaskDispatcher *pTD, cl_dev_cmd_desc *pCmd);

  char *m_pArgV;
};

// OCL Map function execution
class MapMemObject : public CommandBaseClass<ITask> {
public:
  static cl_dev_err_code Create(TaskDispatcher *pTD, cl_dev_cmd_desc *pCmd,
                                SharedPtr<ITaskBase> *pTask,
                                const SharedPtr<ITaskList> &pList);

  // DispatcherCommand interface
  cl_dev_err_code CheckCommandParams(cl_dev_cmd_desc *cmd);

  // ITask interface
  bool Execute() override;

protected:
  MapMemObject(TaskDispatcher *pTD, cl_dev_cmd_desc *pCmd);
};

// OCL UnMap function execution
class UnmapMemObject : public CommandBaseClass<ITask> {
public:
  static cl_dev_err_code Create(TaskDispatcher *pTD, cl_dev_cmd_desc *pCmd,
                                SharedPtr<ITaskBase> *pTask,
                                const SharedPtr<ITaskList> &pList);

  // DispatcherCommand interface
  cl_dev_err_code CheckCommandParams(cl_dev_cmd_desc *cmd);

  // ITask interface
  bool Execute() override;

protected:
  UnmapMemObject(TaskDispatcher *pTD, cl_dev_cmd_desc *pCmd);
};

// OCL Kernel execution
class NDRange : public CommandBaseClass<TaskExecutor::ITaskSet>,
                public DeviceCommands::KernelCommand {
public:
  PREPARE_SHARED_PTR(NDRange)

  static unsigned int RGBTable[COLOR_TABLE_SIZE];

  static cl_dev_err_code Create(TaskDispatcher *pTD, cl_dev_cmd_desc *pCmd,
                                SharedPtr<ITaskBase> *pTask,
                                const SharedPtr<ITaskList> &pList);

  // ITaskSet interface
  int Init(size_t region[], unsigned int &regCount,
           size_t numberOfThreads) override;
  void *AttachToThread(void *pWgContext, size_t uiNumberOfWorkGroups,
                       size_t firstWGID[], size_t lastWGID[]) override;
  void DetachFromThread(void *pWgContext) override;
  bool ExecuteIteration(size_t x, size_t y, size_t z,
                        void *pWgContext) override;
  bool Finish(TaskExecutor::FINISH_REASON reason) override;

  // Optimize By
  TaskExecutor::TASK_SET_OPTIMIZATION OptimizeBy() const override {
    return TaskExecutor::TASK_SET_OPTIMIZE_DEFAULT;
  }
  size_t PreferredSequentialItemsPerThread() const override;
  bool PreferNumaNodes() const override { return true; }

  bool IsCompleted() const override {
    return CommandBaseClass<ITaskSet>::IsCompleted();
  }

  TaskExecutor::IThreadLibTaskGroup *GetNDRangeChildrenTaskGroup() override {
    return GetParentTaskGroup().GetPtr();
  }
  char *GetParamsPtr() {
    return (char *)(((cl_dev_cmd_param_kernel *)m_pCmd->params)->arg_values);
  }

  KernelCommand *AllocateChildCommand(
      ITaskList *pList,
      const Intel::OpenCL::DeviceBackend::ICLDevBackendKernel_ *pKernel,
      const void *pBlockLiteral, size_t stBlockSize, const size_t *pLocalSizes,
      size_t stLocalSizeCount, const _ndrange_t *pNDRange) const override;

  // IDeviceCommandManager
  queue_t GetDefaultQueueForDevice() const override;
  queue_t GetTaskSeqQueueForDevice() const override;

protected:
  NDRange(TaskDispatcher *pTD, cl_dev_cmd_desc *pCmd, ITaskList *pList,
          KernelCommand *parent);

  cl_int m_lastError;
  const DeviceBackend::ICLDevBackendKernelRunner *m_pRunner = nullptr;
  llvm::UniformKernelArgs *m_pImplicitArgs = nullptr;
  void *m_pKernelArgs = nullptr;

  static THREAD_LOCAL
      DeviceBackend::ICLDevBackendKernelRunner::ICLDevExecutionState
          m_tExecState;

  // Information about the hardware and a potential override for work group to
  // thread mapping
  unsigned int m_numThreads;
  bool m_bEnablePredictablePartitioning;
  bool m_needSerializeWGs = false;

  // Used when running in "predictable partitioning" mode (i.e. 1:1 mapping
  // between threads and WGs when using fission) Ensures no work group is
  // executed twice, regardless of task stealing
  Intel::OpenCL::Utils::AtomicBitField m_bWGExecuted;

  // Unique ID of the NDRange command
  static std::atomic<long> s_lGlbNDRangeId;
  long m_lNDRangeId;

#ifdef _DEBUG
  // For debug
  std::atomic<long> m_lExecuting{0};
  std::atomic<long> m_lFinish{0};
  std::atomic<long> m_lAttaching{0};
#endif
};

/**
 * This class represents an NDRange command enqueued by a kernel
 */
class DeviceNDRange : public NDRange {
public:
  PREPARE_SHARED_PTR(DeviceNDRange)

  DeviceNDRange(
      TaskDispatcher *pTD, ITaskList *pList, KernelCommand *parent,
      const Intel::OpenCL::DeviceBackend::ICLDevBackendKernel_ *pKernel,
      const void *pBlockLiteral, size_t stBlockSize, const size_t *pLocalSizes,
      size_t stLocalSizeCount, const _ndrange_t *pNDRange
#ifdef __USE_TBB_SCALABLE_ALLOCATOR__
      ,
      tbb::scalable_allocator<DeviceNDRange> &deviceNDRangeAllocator,
      tbb::scalable_allocator<char> &deviceNDRangeContextAllocator
#endif
      )
      : NDRange(pTD, &m_cmdDesc, pList, parent)
#ifdef __USE_TBB_SCALABLE_ALLOCATOR__
        ,
        m_deviceNDRangeAllocator(deviceNDRangeAllocator),
        m_deviceNDRangeContextAllocator(deviceNDRangeContextAllocator)
#endif
  {
    InitBlockCmdDesc(pKernel, pBlockLiteral, stBlockSize, pLocalSizes,
                     stLocalSizeCount, pNDRange);
  }

  /**
   * @return the next available command ID for DeviceNDRange commands
   */
  static long GetNextCmdId() { return sm_cmdIdCnt++; }

  // inherited methods:
  void NotifyCommandStatusChanged(cl_dev_cmd_desc *cmd, unsigned uStatus,
                                  int iErr) override;
  void Cleanup(bool /*bIsTerminate*/ = false) override {
    const cl_dev_cmd_param_kernel &paramKerel =
        *((cl_dev_cmd_param_kernel *)m_cmdDesc.params);
#if 0
        m_deviceNDRangeContextAllocator.deallocate((char*)paramKerel.arg_values, paramKerel.arg_size);
        m_deviceNDRangeAllocator.deallocate(this, sizeof(DeviceNDRange));
#else
    ALIGNED_FREE(paramKerel.arg_values);
    delete this;
#endif
  }

private:
  void InitBlockCmdDesc(
      const Intel::OpenCL::DeviceBackend::ICLDevBackendKernel_ *pKernel,
      const void *pBlockLiteral, size_t stBlockSize, const size_t *pLocalSizes,
      size_t stLocalSizeCount, const _ndrange_t *pNDRange);

  static std::atomic<long> sm_cmdIdCnt;

  cl_dev_cmd_param_kernel m_paramKernel;
  cl_dev_cmd_desc m_cmdDesc;
  struct ProgramService::KernelMapEntry m_kernelMapEntry;
#ifdef __USE_TBB_SCALABLE_ALLOCATOR__
  tbb::scalable_allocator<DeviceNDRange> &m_deviceNDRangeAllocator;
  tbb::scalable_allocator<char> &m_deviceNDRangeContextAllocator;
#endif
};

// OCL fill buffer/image command
class FillMemObject : public CommandBaseClass<ITask> {
public:
  static cl_dev_err_code Create(TaskDispatcher *pTD, cl_dev_cmd_desc *pCmd,
                                SharedPtr<ITaskBase> *pTask,
                                const SharedPtr<ITaskList> &pList);

  // ITask interface
  bool Execute() override;

protected:
  FillMemObject(TaskDispatcher *pTD, cl_dev_cmd_desc *pCmd);
  cl_dev_err_code CheckCommandParams(cl_dev_cmd_desc *cmd);
};

// OCL migrate buffer/image command
class MigrateMemObject : public CommandBaseClass<ITask> {
public:
  static cl_dev_err_code Create(TaskDispatcher *pTD, cl_dev_cmd_desc *pCmd,
                                SharedPtr<ITaskBase> *pTask,
                                const SharedPtr<ITaskList> &pList);

  // ITask interface
  bool Execute() override;

protected:
  MigrateMemObject(TaskDispatcher *pTD, cl_dev_cmd_desc *pCmd);
  cl_dev_err_code CheckCommandParams(cl_dev_cmd_desc *cmd);
};

// OCL migrate USM buffer command
class MigrateUSMMemObject : public CommandBaseClass<ITask> {
public:
  static cl_dev_err_code Create(TaskDispatcher *pTD, cl_dev_cmd_desc *pCmd,
                                SharedPtr<ITaskBase> *pTask,
                                const SharedPtr<ITaskList> &pList);

  // ITask interface
  bool Execute() override;

protected:
  MigrateUSMMemObject(TaskDispatcher *pTD, cl_dev_cmd_desc *pCmd);
  cl_dev_err_code CheckCommandParams(cl_dev_cmd_desc *cmd);
};

#ifdef __INCLUDE_MKL__
// OCL Native function execution
class NativeKernelTask : public CommandBaseClass<ITask> {
public:
  static cl_dev_err_code Create(TaskDispatcher *pTD,
                                const SharedPtr<ITaskList> &pList,
                                cl_dev_cmd_desc *pCmd,
                                SharedPtr<ITaskBase> *pTask);

  // DispatcherCommand interface
  cl_dev_err_code CheckCommandParams(cl_dev_cmd_desc *cmd);

  // ITask interface
  bool Execute() override;

protected:
  NativeKernelTask(TaskDispatcher *pTD, const SharedPtr<ITaskList> &pList,
                   cl_dev_cmd_desc *pCmd);

  const Intel::OpenCL::BuiltInKernels::IBuiltInKernel *m_pBIKernel;
  ITaskList *m_pList;
};
#endif

// OCL advise USM mem command
class AdviseUSMMemObject : public CommandBaseClass<ITask> {
public:
  static cl_dev_err_code Create(TaskDispatcher *pTD, cl_dev_cmd_desc *pCmd,
                                SharedPtr<ITaskBase> *pTask,
                                const SharedPtr<ITaskList> &pList);

  // ITask interface
  bool Execute() override;

protected:
  AdviseUSMMemObject(TaskDispatcher *pTD, cl_dev_cmd_desc *pCmd);
  cl_dev_err_code CheckCommandParams(cl_dev_cmd_desc *cmd);
};

} // namespace CPUDevice
} // namespace OpenCL
} // namespace Intel
