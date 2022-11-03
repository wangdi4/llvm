// INTEL CONFIDENTIAL
//
// Copyright 2009-2018 Intel Corporation.
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
#include "cpu_config.h"
#include "dispatcher_commands.h"
#include "handle_allocator.h"
#include "memory_allocator.h"
#include "program_service.h"
#include "task_executor.h"
#include <builtin_kernels.h>
#include <cl_synch_objects.h>
#include <cl_thread.h>

#include <atomic>
#include <list>
#include <map> //should be hash_map but cant compile #include <hash_map>

using namespace Intel::OpenCL::TaskExecutor;

#ifdef __INLCUDE_MKL__
#ifndef __OMP2TBB__
using Intel::OpenCL::BuiltInKernels::OMPExecutorThread;
#else
extern "C" void __kmpc_begin(void *loc, int flags);
extern "C" void __kmpc_end(void *loc);
extern "C" void
__omp2tbb_set_thread_max_concurency(unsigned int max_concurency);
extern "C" void
__omp2tbb_set_current_arena_concurrency(unsigned int num_threads);
#endif
#endif

namespace Intel {
namespace OpenCL {
namespace CPUDevice {

typedef struct _cl_dev_internal_subdevice_id {
  // Arch. data
  cl_uint num_compute_units;
  bool is_by_names;
  bool is_by_numa;
  cl_uint *legal_core_ids;

  // Task dispatcher for this sub-device
  Intel::OpenCL::Utils::AtomicCounter ref_count;
  volatile bool is_acquired;
  SharedPtr<ITEDevice> pSubDevice;
} cl_dev_internal_subdevice_id;

class IAffinityChangeObserver {
public:
  /**
   * Notify affinity change
   * @param tid - Thread id
   * @param core_index - Thread position (index) in tbb arena.
   * @param relocate - Whether the previous thread pinned at core_index should
   *                   be relocated to previous core of current thread.
   * @param need_mutex - Whether mutex is needed.
   */
  virtual void NotifyAffinity(threadid_t tid, unsigned int core_index,
                              bool relocate, bool need_mutex) = 0;

  /**
   * Check if master thread will be pinned.
   * @return true if master thread will be pinned.
   */
  virtual bool IsPinMasterAllowed() const = 0;
  virtual ~IAffinityChangeObserver() {}
};

class TaskDispatcher
    : public Intel::OpenCL::TaskExecutor::ITaskExecutorObserver {
  friend class DispatcherCommand;
  friend class AffinitizeThreads;

public:
  TaskDispatcher(cl_int devId, IOCLFrameworkCallbacks *pDevCallbacks,
                 ProgramService *programService, MemoryAllocator *memAlloc,
                 IOCLDevLogDescriptor *logDesc,
                 CPUDeviceConfig *cpuDeviceConfig,
                 IAffinityChangeObserver *pObsserver);
  virtual ~TaskDispatcher();

  virtual cl_dev_err_code init();

  virtual cl_dev_err_code
  SetDefaultCommandList(const SharedPtr<ITaskList> IN list);
  virtual cl_dev_err_code createCommandList(cl_dev_cmd_list_props IN props,
                                            ITEDevice *IN pDevice,
                                            SharedPtr<ITaskList> *OUT list);
  virtual cl_dev_err_code
  commandListExecute(const SharedPtr<ITaskList> &IN list,
                     cl_dev_cmd_desc *IN *cmds, cl_uint IN count);

  virtual ProgramService *getProgramService() { return m_pProgramService; }

  virtual bool isDestributedAllocationRequired();
  virtual bool isThreadAffinityRequired();

  bool isPredictablePartitioningAllowed() {
    cl_dev_internal_subdevice_id *pSubDevID =
        reinterpret_cast<cl_dev_internal_subdevice_id *>(
            m_pTaskExecutor->GetCurrentDevice().user_handle);
    return ((nullptr != pSubDevID) && pSubDevID->is_by_names);
  }

  void waitUntilEmpty(ITEDevice *pSubdev);

  ITaskExecutor &getTaskExecutor() { return *m_pTaskExecutor; }

  queue_t GetDefaultQueue() { return m_pDefaultQueue.GetPtr(); }

  queue_t GetTaskSeqQueue() {
    if (!m_pTaskSeqQueue) {
      OclAutoMutex lock(&TaskSeqQueueMutex);
      if (!m_pTaskSeqQueue) {
        cl_dev_err_code err = createCommandList(
            CL_DEV_LIST_ENABLE_OOO, GetRootDevice(), &m_pTaskSeqQueue);
        (void)err;
        assert(err == CL_DEV_SUCCESS &&
               "Failed to create command queue for task_sequence.");
      }
    }
    return m_pTaskSeqQueue.GetPtr();
  }

  unsigned int GetNumThreads() const { return m_uiNumThreads; }

  ITEDevice *GetRootDevice() { return m_pRootDevice.GetPtr(); }
  // ITaskExecutorObserver
  void *OnThreadEntry(bool registerThread) override;
  void OnThreadExit(void *currentThreadData) override;
  TE_BOOLEAN_ANSWER MayThreadLeaveDevice(void *currentThreadData) override;

  CPUDeviceConfig *getCPUDeviceConfig() { return m_pCPUDeviceConfig; }

#if defined(__INCLUDE_MKL__) && !defined(__OMP2TBB__)
  OMPExecutorThread *getOmpExecutionThread() const {
    return m_pOMPExecutionThread;
  }
#endif
protected:
  cl_int m_iDevId;
  IOCLDevLogDescriptor *m_pLogDescriptor;
  cl_int m_iLogHandle;
  ocl_gpa_data *m_pGPAData;
  IOCLFrameworkCallbacks *m_pFrameworkCallBacks;
  ProgramService *m_pProgramService;
  MemoryAllocator *m_pMemoryAllocator;
  CPUDeviceConfig *m_pCPUDeviceConfig;
  ITaskExecutor *m_pTaskExecutor;
  SharedPtr<ITEDevice> m_pRootDevice;
  unsigned int m_uiNumThreads;
  bool m_bTEActivated;

  Intel::OpenCL::Utils::SharedPtr<ITaskList> m_pDefaultQueue;
  Intel::OpenCL::Utils::SharedPtr<ITaskList> m_pTaskSeqQueue;

  IAffinityChangeObserver *m_pObserver;

  // Internal implementation of functions
  static fnDispatcherCommandCreate_t *m_vCommands[CL_DEV_CMD_MAX_COMMAND_TYPE];

  cl_dev_err_code SubmitTaskArray(ITaskList *pList, cl_dev_cmd_desc **cmds,
                                  cl_uint count);

  void NotifyCommandStatusChange(const cl_dev_cmd_desc *pCmd, unsigned uStatus,
                                 int iErr);

  // Get preferred scheduling, i.e. TBB partitioner
  TE_CMD_LIST_PREFERRED_SCHEDULING getPreferredScheduling();

  // Task failure notification
  class TaskFailureNotification : public ITask {
  public:
    PREPARE_SHARED_PTR(TaskFailureNotification)

    static SharedPtr<TaskFailureNotification>
    Allocate(TaskDispatcher *_this, const cl_dev_cmd_desc *pCmd,
             cl_int retCode) {
      return SharedPtr<TaskFailureNotification>(
          new TaskFailureNotification(_this, pCmd, retCode));
    }

    virtual ~TaskFailureNotification(){};

    // ITask interface
    bool CompleteAndCheckSyncPoint() override { return false; }
    bool SetAsSyncPoint() override {
      assert(0 && "Should not be called");
      return false;
    }
    bool IsCompleted() const override {
      assert(0 && "Should not be called");
      return true;
    }
    bool Execute() override { return Shoot(CL_DEV_ERROR_FAIL); }
    void Cancel() override { Shoot(CL_DEV_COMMAND_CANCELLED); }
    long Release() override { return 0; }
    TASK_PRIORITY GetPriority() const override { return TASK_PRIORITY_MEDIUM; }
    Intel::OpenCL::TaskExecutor::IThreadLibTaskGroup *
    GetNDRangeChildrenTaskGroup() override {
      return nullptr;
    }

  protected:
    TaskDispatcher *m_pTaskDispatcher;
    const cl_dev_cmd_desc *m_pCmd;
    cl_int m_retCode;

    bool Shoot(cl_dev_err_code err);

    TaskFailureNotification(TaskDispatcher *_this, const cl_dev_cmd_desc *pCmd,
                            cl_int retCode)
        : m_pTaskDispatcher(_this), m_pCmd(pCmd), m_retCode(retCode) {}
  };

  cl_dev_err_code NotifyFailure(ITaskList *pList, cl_dev_cmd_desc *cmd,
                                cl_int iRetCode);

#if defined(__INCLUDE_MKL__) && !defined(__OMP2TBB__)
  OMPExecutorThread *m_pOMPExecutionThread;
#endif

private:
  TaskDispatcher(const TaskDispatcher &);
  TaskDispatcher &operator=(const TaskDispatcher &);
  Intel::OpenCL::Utils::OclSpinMutex TaskSeqQueueMutex;
};

class AffinitizeThreads : public ITaskSet {
public:
  PREPARE_SHARED_PTR(AffinitizeThreads)

  static SharedPtr<AffinitizeThreads> Allocate(unsigned int numThreads,
                                               cl_ulong timeOutInTicks) {
    return SharedPtr<AffinitizeThreads>(
        new AffinitizeThreads(numThreads, timeOutInTicks));
  }

  virtual ~AffinitizeThreads();

  // ITaskSet interface
  bool SetAsSyncPoint() override { return false; }
  bool CompleteAndCheckSyncPoint() override { return true; }
  bool IsCompleted() const override { return true; }
  int Init(size_t region[], unsigned int &regCount,
           size_t numberOfThreads) override;
  void *AttachToThread(void *tls, size_t uiNumberOfWorkGroups,
                       size_t firstWGID[], size_t lastWGID[]) override;
  void DetachFromThread(void *data) override;
  bool ExecuteIteration(size_t x, size_t y, size_t z, void *data) override;
  bool Finish(FINISH_REASON /*reason*/) override {
    ++m_endBarrier;
    return false;
  }
  long Release() override { return 0; }
  void Cancel() override { Finish(FINISH_EXECUTION_FAILED); };
  Intel::OpenCL::TaskExecutor::IThreadLibTaskGroup *
  GetNDRangeChildrenTaskGroup() override {
    return nullptr;
  }

  TASK_PRIORITY GetPriority() const override { return TASK_PRIORITY_MEDIUM; }
  TASK_SET_OPTIMIZATION OptimizeBy() const override {
    return TASK_SET_OPTIMIZE_DEFAULT;
  }
  size_t PreferredSequentialItemsPerThread() const override { return 1; }
  bool PreferNumaNodes() const override { return false; }

  void WaitForEndOfTask() const;

protected:
  unsigned int m_numThreads;
  cl_ulong m_startTime; // start time in nanoseconds
  cl_ulong m_timeOut;   // time out in nanoseconds
  std::atomic<unsigned> m_barrier;
  std::atomic<bool> m_failed;

  Intel::OpenCL::Utils::AtomicCounter m_endBarrier;

  AffinitizeThreads(unsigned int numThreads, cl_ulong timeOutInTicks);
};

} // namespace CPUDevice
} // namespace OpenCL
} // namespace Intel
