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

#include "Logger.h"
#include "cl_shutdown.h"
#include "cl_thread.h"
#include "cl_types.h"
#include "context_module.h"
#include "execution_module.h"
#include "ocl_config.h"
#include "ocl_itt.h"
#include "platform_module.h"
#include "task_executor.h"

// system
#include <mutex>
#include <set>

namespace Intel {
namespace OpenCL {
namespace Framework {

/*******************************************************************************
 * Class name:    FrameworkProxy
 *
 * Description: the framework proxy class design to pass the OpenCL api calls to
 *              the framework's modules
 ******************************************************************************/
class FrameworkProxy {
public:
  /*****************************************************************************
   * Function:     Instance
   * Description: Get the instance of the framework proxy module.
   * Arguments:
   * Return value:    instance to the framework factory
   ****************************************************************************/
  static FrameworkProxy *Instance();

  static void Destroy();

  /*****************************************************************************
   * Function:     GetContextModule
   * Description:    Get handle to the context module
   * Arguments:
   * Return value: pointer to the ContextModule class. NULL if context module
   *               wasn't initialized successfully
   ****************************************************************************/
  ContextModule *GetContextModule() const { return m_pContextModule; }

  /*****************************************************************************
   * Function:     GetExecutionModule
   * Description:    Get handle to the execution module
   * Arguments:
   * Return value: pointer to the ExecutionModule class. NULL if module wasn't
   *               initialized successfully
   ****************************************************************************/
  ExecutionModule *GetExecutionModule() const { return m_pExecutionModule; }

  /*****************************************************************************
   * Function:     GetPlatformModule
   * Description:    Get handle to the platform module
   * Arguments:
   * Return value: pointer to the PlatformModule class. NULL if module wasn't
   *               initialized successfully
   ****************************************************************************/
  PlatformModule *GetPlatformModule() const { return m_pPlatformModule; }

  /*****************************************************************************
   * Function:     GetTaskExecutor
   * Description:    Get global TaskExecutor Interface for Framework
   * Arguments:
   * Return value:
   ****************************************************************************/
  Intel::OpenCL::TaskExecutor::ITaskExecutor *GetTaskExecutor() const;

  /*****************************************************************************
   * Function:     Activate
   * Description:    Simple TaskExecutor Interface for Framework
   * Arguments:
   * Return value:    false on error
   ****************************************************************************/
  bool ActivateTaskExecutor();

  /*****************************************************************************
   * Function:     CancelAllTasks()
   * Description:    Simple TaskExecutor Interface for Framework
   * Arguments:
   * Return value:
   ****************************************************************************/
  void CancelAllTasks(bool wait_for_finish) const;

  /*****************************************************************************
   * Function:     NeedToDisableAPIsAtShutdown
   * Description:    Considers if necessary to disable API at shutdown
   *
   * Arguments:
   * Return value:    true for WIN32 and FPGA, false for
   ****************************************************************************/
  bool NeedToDisableAPIsAtShutdown() const;

  /*****************************************************************************
   * Function:     Execute task on TaskExecutor
   * Description:    Simple
   * TaskExecutor Interface for Framework
   * Arguments:
   * Return
   * value:    false on error
   ****************************************************************************/
  bool Execute(const Intel::OpenCL::Utils::SharedPtr<
               Intel::OpenCL::TaskExecutor::ITaskBase> &pTask) const;

  /*****************************************************************************
   * Function:     Execute task on TaskExecutor immediate
   *
   * Description:    Simple TaskExecutor Interface for Framework
   *
   * Arguments:
   * Return value:    false on error
   ****************************************************************************/
  bool ExecuteImmediate(const Intel::OpenCL::Utils::SharedPtr<
                        Intel::OpenCL::TaskExecutor::ITaskBase> &pTask) const;

  bool API_Disabled() const { return Intel::OpenCL::Utils::IsShuttingDown(); }

  const OCLConfig *GetOCLConfig() { return m_pConfig; };

private:
  /*****************************************************************************
   * Function:     FrameworkProxy
   * Description:    The FrameworkProxy class constructor
   * Arguments:
   ****************************************************************************/
  FrameworkProxy();

  /*****************************************************************************
   * Function:     ~FrameworkProxy
   * Description:    The FrameworkProxy class destructor
   * Arguments:
   ****************************************************************************/
  virtual ~FrameworkProxy();

  void Initialize();
  void Release(bool bTerminate);

  static cl_icd_dispatch ICDDispatchTable;
  static SOCLCRTDispatchTable CRTDispatchTable;
  static ocl_entry_points OclEntryPoints;

  static void InitOCLEntryPoints();

  // handle to the platform module
  PlatformModule *m_pPlatformModule;

  // handle to the context module
  ContextModule *m_pContextModule;

  // handle to the execution module
  ExecutionModule *m_pExecutionModule;

  // handle to the file log handler
  Intel::OpenCL::Utils::FileLogHandler *m_pFileLogHandler;

  // handle to the configuration object
  OCLConfig *m_pConfig;

  mutable ocl_gpa_data m_GPAData;

  // handle to TaskExecutor
  // During shutdown task_executor dll may finish before current dll and destroy
  // all internal objects We can discover this case but we cannot access any
  // task_executor object at that time point because it may be already
  // destroyed. As SharedPtr accesses the object itself to manage counters, we
  // cannot use SharedPointers at all.
  mutable Intel::OpenCL::TaskExecutor::ITaskExecutor *m_pTaskExecutor;
  SharedPtr<Intel::OpenCL::TaskExecutor::ITaskList> m_pTaskList;
  SharedPtr<Intel::OpenCL::TaskExecutor::ITaskList> m_pTaskList_immediate;

  static FrameworkProxy *m_instance;

  // handle to the logger client
  DECLARE_LOGGER_CLIENT;
};

} // namespace Framework
} // namespace OpenCL
} // namespace Intel
