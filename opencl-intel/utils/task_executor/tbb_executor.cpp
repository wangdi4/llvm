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

#include "tbb_executor.h"

#include "base_command_list.hpp"
#include "cl_config.h"
#include "cl_shared_ptr.hpp"
#include "cl_user_logger.h"
#include "cl_utils.h"
#include "cpu_dev_limits.h"
#include "task_group.hpp"
#include "tbb_execution_schedulers.h"

#include <algorithm>
#include <cassert>
#include <cl_env.h>
#include <cl_shutdown.h>
#include <cl_sys_defines.h>
#include <cl_sys_info.h>
#include <string>
#include <tbb/blocked_range.h>
#include <tbb/concurrent_queue.h>
#include <tbb/enumerable_thread_specific.h>
#include <tbb/scalable_allocator.h>
#include <tbb/task.h>
#include <vector>

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

// no local atexit handler - only global
USE_SHUTDOWN_HANDLER(NULL);

using namespace Intel::OpenCL::Utils;

// #define _EXTENDED_LOG
#if !VECTOR_RESERVE
#define VECTOR_RESERVE 16
#endif

#define MAX_BATCH_SIZE 128
#define SPARE_STATIC_DATA 8

namespace Intel {
namespace OpenCL {
namespace Utils {

FrameworkUserLogger *g_pUserLogger = nullptr;

}
} // namespace OpenCL
} // namespace Intel

namespace Intel {
namespace OpenCL {
namespace TaskExecutor {

void RegisterReleaseSchedulerForMasterThread();

//! global TBB task scheduler objects
unsigned int gWorker_threads = 0;
AtomicCounter glTaskSchedCounter;

void TE_RegisterGlobalAtExitNotification(IAtExitCentralPoint *fn) {
  Intel::OpenCL::Utils::RegisterGlobalAtExitNotification(fn);
}

bool execute_command(const SharedPtr<ITaskBase> &pCmd,
                     base_command_list &cmdList) {
  bool runNextCommand = true;
  bool cancel = cmdList.Is_canceled();

  if (cancel) {
    pCmd->Cancel();
  } else if (pCmd->IsTaskSet()) {
    const SharedPtr<ITaskSet> &pTaskSet = pCmd.StaticCast<ITaskSet>();
    runNextCommand =
        TBB_ExecutionSchedulers::parallel_execute(cmdList, pTaskSet);
  } else {
    const SharedPtr<ITask> &pTask = pCmd.StaticCast<ITask>();
    runNextCommand = pTask->Execute();
  }

  runNextCommand &= !pCmd->CompleteAndCheckSyncPoint();
  return (cancel || runNextCommand);
}

void in_order_executor_task::operator()() const {
  assert(m_list);
  ConcurrentTaskQueue *work = m_list->GetExecutingContainer();
  assert(work);
  bool mustExit = false;

  while (true) {
    SharedPtr<ITaskBase> currentTask;
    // First check if we need to stop iterating, next get next available record
    while (!(mustExit && m_list->m_bMasterRunning) &&
           work->TryPop(currentTask)) {
      if (NULL == currentTask->GetNDRangeChildrenTaskGroup()) {
        // task enqueued by the host
        mustExit = !execute_command(currentTask, *m_list); // stop requested
      } else {
        // child task (GetNDRangeChildrenTaskGroup() returns the parent's
        // TaskGroup)
        ExecuteContainerBody functor(currentTask, *m_list);
        TbbTaskGroup &tbbTskGrp = *static_cast<TbbTaskGroup *>(
            currentTask->GetNDRangeChildrenTaskGroup());
        tbbTskGrp.Run(functor);
      }
      currentTask = NULL;
    }

    if (mustExit) {
      if (m_list->m_execTaskRequests.exchange(0) > 1) {
        m_list->InternalFlush(false);
      }
      break;
    }
    if (1 == m_list->m_execTaskRequests--) {
      break;
    }
  }
}

void out_of_order_executor_task::operator()() const {
  while (true) {
    /* We still have a proble in case the user calls clFinish from one thread
       and then enqueues a command from another thread - the second command will
       wait until the SyncTask of the clFinish is completed, although it could
       be executed without waiting. However, this is a rare case and we won't
       handle it for now. */
    SharedPtr<ITaskBase> pTask = GetTask();
    while (0 != pTask && NULL == dynamic_cast<SyncTask *>(pTask.GetPtr())) {
      ExecuteContainerBody functor(pTask, *m_list);
      if (pTask->GetNDRangeChildrenTaskGroup() != NULL) {
        // this is a child task - GetNDRangeChildrenTaskGroup() returns its
        // parent TaskGroup
        TbbTaskGroup &tbbTskGrp =
            *static_cast<TbbTaskGroup *>(pTask->GetNDRangeChildrenTaskGroup());
        tbbTskGrp.Run(functor);
      } else {
        // this is a parent task (enqueued by the host)
        m_list->ExecOOOFunc<ExecuteContainerBody>(functor);
      }
      pTask = GetTask();
    }
    if (0 != pTask && NULL != dynamic_cast<SyncTask *>(pTask.GetPtr())) {
      // synchronization point
      m_list->WaitForAllCommands();
      static_cast<SyncTask *>(pTask.GetPtr())->Execute();
      if (m_list->m_execTaskRequests.exchange(0) > 1) {
        m_list->InternalFlush(false);
      }
      break;
    }
    if (1 == m_list->m_execTaskRequests--) {
      break;
    }
  }
}

SharedPtr<ITaskBase> out_of_order_executor_task::GetTask() const {
  SharedPtr<ITaskBase> pTask;
  if (m_list->GetExecutingContainer()->TryPop(pTask)) {
    return pTask;
  } else {
    return NULL;
  }
}

void immediate_executor_task::operator()() const {
  assert(m_list);
  assert(m_pTask);

  execute_command(m_pTask, *m_list);
}

/////////////// TaskExecutor //////////////////////
TBBTaskExecutor::TBBTaskExecutor() : m_tbbNumaEnabled(false), m_err(0) {
  // we deliberately don't delete m_pScheduler (see comment above its
  // definition)
}

TBBTaskExecutor::~TBBTaskExecutor() {
  /* We don't delete m_pGlobalArenaHandler because of all kind of TBB issues in
     the shutdown sequence, but since this destructor is called when the whole
     library goes down and m_pGlobalArenaHandler is a singleton, it doesn't
     really matter. */
  // TBB seem to have a bug in ~task_scheduler_init(), so we work around it by
  // not deleting m_pScheduler (TBB bug #1955)
}

int TBBTaskExecutor::Init(unsigned int uiNumOfThreads, ocl_gpa_data *pGPAData,
                          size_t ulAdditionalRequiredStackSize,
                          DeviceMode deviceMode) {
  INIT_LOGGER_CLIENT("TBBTaskExecutor", LL_INFO);
  LOG_INFO(TEXT("Initialization request with %d threads"), uiNumOfThreads);
  if (0 != gWorker_threads) {
    assert(0 && "TBBExecutor already initialized");
    LOG_ERROR(TEXT("Already initialized with %d threads"), gWorker_threads);
    ;
    return gWorker_threads;
  }

  m_pGPAData = pGPAData;

  // Explicitly load TBB library
  if (!LoadTBBLibrary()) {
    LOG_ERROR(TEXT("%s"), "Failed to load TBB library");
    if (g_pUserLogger && g_pUserLogger->IsErrorLoggingEnabled())
      g_pUserLogger->PrintError("Failed to load TBB library.");
    return 0;
  }

  // Check TBB library version

  // Minimal TBB version that we support(7001 is TBB 4.2).
  const int MINIMAL_TBB_INTERFACE_VERSION = 7001;
  if (MINIMAL_TBB_INTERFACE_VERSION > TBB_runtime_interface_version()) {
    std::stringstream stream;
    stream << "TBB version doesn't match. Required "
           << __TBB_STRING(MINIMAL_TBB_INTERFACE_VERSION) << ", loaded "
           << TBB_runtime_interface_version() << "." << std::ends;
    LOG_ERROR(TEXT(stream.str().c_str()), "");
    if (nullptr != g_pUserLogger && g_pUserLogger->IsErrorLoggingEnabled()) {
      g_pUserLogger->PrintError(stream.str().c_str());
    }
    return 0;
  }

  gWorker_threads = uiNumOfThreads;
  unsigned activeThreads = (unsigned)tbb::global_control::active_value(
      tbb::global_control::max_allowed_parallelism);
  if (gWorker_threads == TE_AUTO_THREADS) {
    // Threads number should be inquired from the threads spawner (tbb).
    gWorker_threads =
        std::min((unsigned int)Intel::OpenCL::Utils::GetNumberOfProcessors(),
                 activeThreads);
  }

  unsigned hardwareThreads = std::min(
      gWorker_threads, (unsigned)Intel::OpenCL::Utils::GetNumberOfProcessors());
  // TBB restrictions. Magic number 256 is obtained from TBB team. It
  // means that TBB can create at least 256 workers, even on machines
  // with small number of hardware threads.
  unsigned maxThreads = std::max(4 * hardwareThreads, 256U);
  // 1 main thread + 1 tbb worker. If application limits
  // max_allowed_parallelism to 1, we can only set minThreads to 1.
  unsigned minThreads = 1;
  if (activeThreads > 1)
    minThreads += TE_MIN_WORKER_THREADS;

  if (FPGA_EMU_DEVICE == deviceMode) {
    if (uiNumOfThreads != TE_AUTO_THREADS) {
      if (uiNumOfThreads < minThreads) {
        LOG_ERROR(TEXT("TBBTaskExecutor cannot be constructed with %u threads. "
                       "Setting num threads to %u"),
                  uiNumOfThreads, minThreads);
        gWorker_threads = minThreads;
      } else if (uiNumOfThreads > maxThreads) {
        LOG_ERROR(TEXT("TBBTaskExecutor cannot be constructed with %u threads. "
                       "Setting num threads to %u"),
                  uiNumOfThreads, maxThreads);
        gWorker_threads = maxThreads;
      } else {
        gWorker_threads = uiNumOfThreads;
      }
    } else {
      // Adjust number of threads required for an 'average' FPGA program
      unsigned averageThreads = 32;

      if (hardwareThreads < averageThreads) {
        gWorker_threads = std::min(averageThreads, maxThreads);
      }
    }
  } else {
    gWorker_threads = std::max(hardwareThreads, minThreads);
  }

  if (gWorker_threads != activeThreads) {
    m_tbbMaxParallelism = std::make_unique<tbb::global_control>(
        tbb::global_control::max_allowed_parallelism, gWorker_threads);
    // Allow the setting of max_allowed_parallelism to fail since application
    // could set max_allowed_parallelism to a small value.
    activeThreads = (unsigned)tbb::global_control::active_value(
        tbb::global_control::max_allowed_parallelism);
    if (activeThreads != gWorker_threads) {
      LOG_ERROR(TEXT("Failed to set tbb global_control "
                     "max_allowed_parallelism: actual %u, expected %u"),
                activeThreads, gWorker_threads);
      gWorker_threads = activeThreads;
    }
  }

  // Set stack size.
  size_t stackSize = ulAdditionalRequiredStackSize;
  if (FPGA_EMU_DEVICE == deviceMode) {
    const size_t TBBDefaultStackSize = tbb::global_control::active_value(
        tbb::global_control::thread_stack_size);
    stackSize += TBBDefaultStackSize;
  }

  // Align stack size to 4 bytes.
  if ((stackSize & 3u) != 0) {
    // check that we can align stackSize without overflowing of size_t
    assert((((size_t)-1) - 4u >= (stackSize & (~3u))) &&
           "stackSize is too big");
    // if last 2 bits are non-zero clear it and add 4 bytes to cover the loss.
    stackSize = (stackSize & (~3u)) + 4u;
  }
  // We force stack size of TBB created threads to match required value.
  m_tbbStackSize = std::make_unique<tbb::global_control>(
      tbb::global_control::thread_stack_size, stackSize);
  size_t activeStackSize =
      tbb::global_control::active_value(tbb::global_control::thread_stack_size);
  if (activeStackSize != stackSize) {
    // This might not be successful if multiple instances of OCL RT are
    // created in the same process, only the first setting is successful.
    LOG_ERROR(
        TEXT("Failed to set tbb thread_stack_size, actual %zu, expected %zu"),
        activeStackSize, stackSize);
  } else {
    LOG_INFO(TEXT("Set tbb thread_stack_size to %zu"), stackSize);
  }

  m_threadManager.Init(gWorker_threads + SPARE_STATIC_DATA);
  // + SPARE to allow temporary oversubscription in flat mode and
  // additional root devices

  // Initialize TBB NUMA support
  if (gWorker_threads > 1)
    InitTBBNuma();

  LOG_INFO(TEXT("TBBTaskExecutor constructed to %d threads"), gWorker_threads);

  return gWorker_threads;
}

void TBBTaskExecutor::InitTBBNuma() {
  std::string envCPUPlaces;
  if (Intel::OpenCL::Utils::getEnvVar(envCPUPlaces, "DPCPP_CPU_PLACES") &&
      ("numa_domains" == StringRef(envCPUPlaces).lower())) {
    // Only call tbb::info::numa_nodes if env is set, otherwise there is perf
    // regression (CMPLRLLVM-20763) since this call take 45ms on 2-socket CLX.
    m_tbbNumaNodes = tbb::info::numa_nodes();
    int nodesCount = (int)m_tbbNumaNodes.size();
    // Only use TBB NUMA support if env is "numa_domains" and there are at least
    // two NUMA nodes.
    m_tbbNumaEnabled = nodesCount > 1;
  }
}

void TBBTaskExecutor::Finalize() {
  gWorker_threads = 0;
  LOG_INFO(TEXT("%s"), "TBBTaskExecutor Destroyed");
  RELEASE_LOGGER_CLIENT;
}

SharedPtr<ITEDevice>
TBBTaskExecutor::CreateRootDevice(const RootDeviceCreationParam &device_desc,
                                  void *user_data,
                                  ITaskExecutorObserver *my_observer) {
  LOG_INFO(TEXT("Creating RootDevice with %d threads"),
           device_desc.uiThreadsPerLevel[0]);
  assert((gWorker_threads > 0) && "TBB executor should be initialized first");

  RootDeviceCreationParam device(device_desc);

  if ((TE_AUTO_THREADS == device.uiThreadsPerLevel[0]) &&
      (1 == device.uiNumOfLevels)) {
    // tbb::task_scheduler_init creates "num_threads - 1" workers to account
    // the master thread and avoid possible oversubscription. If the number
    // workers is 0, we need to set uiThreadsPerLevel to 1.
    //
    // tbb::task_arena expects that tbb::task_scheduler is initialized by
    // "num_threads - num_of_masters" threads.
    //
    // We cannot just use gWorker_threads as "num_threads" for an arena when
    // joining of masters is disabled because it accounts the master thread.
    // So, we need to manually remove the master thread from an arena
    device.uiThreadsPerLevel[0] =
        (TE_DISABLE_MASTERS_JOIN == device.mastersJoining)
            ? std::max(1, (int)gWorker_threads -
                              (int)device.uiNumOfExecPlacesForMasters)
            : gWorker_threads;
  }

  assert((device.uiNumOfLevels <= TE_MAX_LEVELS_COUNT) &&
         "Too many levels of devices");
  if (TE_MAX_LEVELS_COUNT == device.uiNumOfLevels) {
    device.uiThreadsPerLevel[0] = GetTBBNumaNodesCount();
    device.uiThreadsPerLevel[1] =
        ((TE_DISABLE_MASTERS_JOIN == device.mastersJoining)
             ? gWorker_threads - device.uiNumOfExecPlacesForMasters
             : gWorker_threads) /
        device.uiThreadsPerLevel[0];
  }

  // check params
  if ((0 == device.uiNumOfLevels) ||
      (device.uiNumOfLevels > TE_MAX_LEVELS_COUNT)) {
    LOG_ERROR(
        TEXT("Wrong uiNumOfLevels parameter = %d, must be between 1 and %d"),
        device.uiNumOfLevels, TE_MAX_LEVELS_COUNT);
    return NULL;
  }

  // check for overall number of threads
  unsigned int overall_threads = 1;
  for (unsigned int i = 0; i < device.uiNumOfLevels; ++i) {
    unsigned int uiThreadsPerLevel = device.uiThreadsPerLevel[i];
    if ((0 == uiThreadsPerLevel) || (TE_AUTO_THREADS == uiThreadsPerLevel)) {
      assert(false && "Cannot specify 0 or TE_AUTO_THREADS threads per level");
      LOG_ERROR(
          TEXT("Wrong number of threads per level: %u, must not be 0 or %u"),
          uiThreadsPerLevel, TE_AUTO_THREADS);
      return NULL;
    }
    overall_threads *= device.uiThreadsPerLevel[i];
  }

  if ((overall_threads == 0) || (overall_threads > gWorker_threads)) {
    assert(false && "Too many threads requested - above maximum configured");
    LOG_ERROR(
        TEXT("Wrong number of threads specified per level. Amount of threads "
             "on each level should be above 0, overall number not exceed %d"),
        gWorker_threads);
    return NULL;
  }

  // Create root device
  SharedPtr<TEDevice> root =
      TEDevice::Allocate(device, user_data, my_observer, *this);

  if (0 == root) {
    LOG_ERROR(TEXT("%s"), "Root device allocation failed"); // make gcc happy
  } else {
    LOG_INFO(TEXT("Root device created with %d threads"), overall_threads);
  }
  return root;
}

unsigned int TBBTaskExecutor::GetMaxNumOfConcurrentThreads() const {
  return gWorker_threads;
}

ocl_gpa_data *TBBTaskExecutor::GetGPAData() const { return m_pGPAData; }

bool TBBTaskExecutor::LoadTBBLibrary() {
#ifdef WIN32
  // The loading on tbb.dll was delayed,
  // Need to load manually before default dll is loaded

  std::string tbbPath("tbb12");

#ifdef _DEBUG
  tbbPath += "_debug";
#endif // _DEBUG

  tbbPath += TBB_BINARIES_POSTFIX;
  tbbPath += ".dll";

  // check the current module folder first.
  std::string modulePath = std::string(MAX_PATH, '\0');

  Intel::OpenCL::Utils::GetModuleDirectory(&modulePath[0], MAX_PATH);
  modulePath.resize(modulePath.find_first_of('\0'));
  modulePath += tbbPath;

  m_err = m_dllTBBLib.Load(modulePath.c_str());
  if (m_err != 0) {
    char cszLibraryName[1024] = {0};
    BasicCLConfigWrapper basicConfig;
    basicConfig.Initialize(GetConfigFilePath());
    std::string tbbLocInReg =
        "SOFTWARE\\Intel\\oneAPI\\TBB\\" + basicConfig.GetTBBVersion();
    // Get TBB location defined under
    // "SOFTWARE\\Intel\\oneAPI\\TBB\\TBB_VERSION\\" location. The key name is
    // TBB_DLL_PATH
    bool keyret =
        GetStringValueFromRegistryOrETC(HKEY_LOCAL_MACHINE, tbbLocInReg.c_str(),
                                        "TBB_DLL_PATH", cszLibraryName, 1024);
    std::string tbbFullPath;
    if (keyret) {
      tbbFullPath = std::string(cszLibraryName) + "\\" + tbbPath;
      m_err = m_dllTBBLib.Load(tbbFullPath.c_str());
    }

    if (m_err != 0 && !basicConfig.GetTBBDLLPath().empty()) {
      // Suppose there is a field named TBB_DLL_PATH in OCL CPU RT
      // configuration file.
      tbbFullPath = basicConfig.GetTBBDLLPath() + "\\" + tbbPath;
      m_err = m_dllTBBLib.Load(tbbFullPath.c_str());
    }

    if (m_err != 0) {
      fprintf(
          stderr,
          "Cannot load TBB from neither Windows registry key nor CPU runtime "
          "configuration file (cl.cfg / cl.fpga_emu.cfg). Error: %s.\n"
          "You can ask your administrator to configure TBB library location "
          "in the configuration file.\n",
          m_dllTBBLib.GetError().c_str());
    }
  }
#endif

  return m_err == 0;
}

ITaskExecutor::DeviceHandleStruct TBBTaskExecutor::GetCurrentDevice() const {
  TBB_PerActiveThreadData *tls = m_threadManager.GetCurrentThreadDescriptor();

  if (NULL == tls) {
    return DeviceHandleStruct();
  } else {
    return DeviceHandleStruct(
        tls->device, (NULL != tls->device) ? tls->device->GetUserData() : NULL);
  }
}

bool TBBTaskExecutor::IsMaster() const {
  TBB_PerActiveThreadData *tls = m_threadManager.GetCurrentThreadDescriptor();
  return ((NULL != tls) ? tls->is_master : true);
}

unsigned int TBBTaskExecutor::GetPosition(unsigned int level) const {
  if (level >= TE_MAX_LEVELS_COUNT) {
    assert(false &&
           "Cannot return thread position for the level more then supported");
    return TE_UNKNOWN;
  }

  TBB_PerActiveThreadData *tls = m_threadManager.GetCurrentThreadDescriptor();

  return (((NULL != tls) && (NULL != tls->device) &&
           (level < tls->device->GetNumOfLevels()))
              ? tls->position[level]
              : TE_UNKNOWN);
}

SharedPtr<IThreadLibTaskGroup>
TBBTaskExecutor::CreateTaskGroup(const SharedPtr<ITEDevice> & /*device*/) {
  return TbbTaskGroup::Allocate();
}
} // namespace TaskExecutor
} // namespace OpenCL
} // namespace Intel
