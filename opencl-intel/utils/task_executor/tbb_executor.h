// INTEL CONFIDENTIAL
//
// Copyright 2006-2020 Intel Corporation.
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

#include "arena_handler.h"
#include "base_command_list.h"
#include "cl_dynamic_lib.h"
#include "cl_shared_ptr.h"
#include "cl_synch_objects.h"
#include "task_executor.h"
#include "tbb_thread_manager.h"

#include <tbb/global_control.h>

#ifdef DEVICE_NATIVE
// no logger on discrete device
#define LOG_ERROR(...)
#define LOG_INFO(...)

#define DECLARE_LOGGER_CLIENT
#define INIT_LOGGER_CLIENT(...)
#define RELEASE_LOGGER_CLIENT
#else
#include "Logger.h"
#endif // DEVICE_NATIVE

using Intel::OpenCL::Utils::AtomicPointer;
using Intel::OpenCL::Utils::OclMutex;
using Intel::OpenCL::Utils::SharedPtr;

namespace Intel {
namespace OpenCL {
namespace TaskExecutor {

/**
 * a global flag indicating whether the program has called function exit
 */
class TBBTaskExecutor : public ITaskExecutor {
public:
  TBBTaskExecutor();
  virtual ~TBBTaskExecutor();

  int Init(unsigned int uiNumOfThreads = TE_AUTO_THREADS,
           ocl_gpa_data *pGPAData = nullptr,
           size_t ulAdditionalRequiredStackSize = 0,
           DeviceMode deviceMode = CPU_DEVICE) override;
  void Finalize() override;

  // Initialize TBB NUMA and check if it is supported
  void InitTBBNuma();

  Intel::OpenCL::Utils::SharedPtr<ITEDevice> CreateRootDevice(
      const RootDeviceCreationParam &device_desc = RootDeviceCreationParam(),
      void *user_data = nullptr,
      ITaskExecutorObserver *my_observer = nullptr) override;

  unsigned int GetMaxNumOfConcurrentThreads() const override;

  ocl_gpa_data *GetGPAData() const override;

  virtual DeviceHandleStruct GetCurrentDevice() const override;
  virtual bool IsMaster() const override;
  virtual unsigned int GetPosition(unsigned int level = 0) const override;

  virtual bool IsTBBNumaEnabled() const override { return m_tbbNumaEnabled; }
  virtual unsigned int GetTBBNumaNodesCount() const override {
    return (unsigned int)m_tbbNumaNodes.size();
  }
  virtual const std::vector<int> &GetTBBNumaNodes() const override {
    return m_tbbNumaNodes;
  }

  typedef TBB_ThreadManager<TBB_PerActiveThreadData> ThreadManager;
  ThreadManager &GetThreadManager() { return m_threadManager; }
  const ThreadManager &GetThreadManager() const { return m_threadManager; }

  virtual SharedPtr<IThreadLibTaskGroup>
  CreateTaskGroup(const SharedPtr<ITEDevice> &device) override;

  int GetErrorCode() override { return m_err; }

protected:
  // Load TBB library explicitly
  bool LoadTBBLibrary();

  // TBB global_control variables. Impact of a variable ends with its
  // lifetime.
  std::unique_ptr<tbb::global_control> m_tbbMaxParallelism;
  std::unique_ptr<tbb::global_control> m_tbbStackSize;

  ThreadManager m_threadManager;
  Intel::OpenCL::Utils::OclDynamicLib m_dllTBBLib;

  // whether TBB NUMA API is enabled
  bool m_tbbNumaEnabled;
  std::vector<int> m_tbbNumaNodes;

  // Logger
  DECLARE_LOGGER_CLIENT;

  int m_err; // error code

private:
  TBBTaskExecutor(const TBBTaskExecutor &);
  TBBTaskExecutor &operator=(const TBBTaskExecutor &);
};

class in_order_executor_task {
public:
  in_order_executor_task(const SharedPtr<base_command_list> &list)
      : m_list(list) {}

  void operator()() const;

protected:
  SharedPtr<base_command_list> m_list;

  void FreeCommandBatch(TaskVector *pCmdBatch);
};

class out_of_order_executor_task {
public:
  out_of_order_executor_task(const SharedPtr<base_command_list> &list)
      : m_list(list.StaticCast<out_of_order_command_list>()) {
    assert(m_list != 0);
  }

  void operator()() const;

private:
  SharedPtr<ITaskBase> GetTask() const;
  SharedPtr<out_of_order_command_list> m_list;
};

class immediate_executor_task {
public:
  immediate_executor_task(immediate_command_list *list,
                          const SharedPtr<ITaskBase> &pTask)
      : m_list(list), m_pTask(pTask) {}

  void operator()() const;

protected:
  immediate_command_list *m_list;
  SharedPtr<ITaskBase> m_pTask;
};

} // namespace TaskExecutor
} // namespace OpenCL
} // namespace Intel
