// INTEL CONFIDENTIAL
//
// Copyright 2012-2023 Intel Corporation.
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

#include "context_module.h"
#include "Context.hpp"
#include "GenericMemObj.h"
#include "MemoryAllocator/MemoryObject.h"
#include "cl_shared_ptr.hpp"
#include "cl_sys_info.h"
#include "command_queue.h"
#include "events_manager.h"
#include "framework_proxy.h"
#include "kernel.h"
#include "ocl_itt.h"
#include "pipe.h"
#include "program.h"
#include "program_with_library_kernels.h"
#include "sampler.h"
#include "svm_buffer.h"
#include "user_event.h"
#include "usm_buffer.h"
#include "llvm/Support/Compiler.h" // LLVM_FALLTHROUGH
#include <CL/cl_fpga_ext.h>
#include <Device.h>
#include <algorithm>
#include <assert.h>
#include <cl_objects_map.h>
#include <cl_utils.h>
#include <platform_module.h>
#include <set>

using namespace Intel::OpenCL::Utils;
using namespace Intel::OpenCL::Framework;

//////////////////////////////////////////////////////////////////////////
// ContextModule C'tor
//////////////////////////////////////////////////////////////////////////
ContextModule::ContextModule(PlatformModule *pPlatformModule)
    : m_pOclEntryPoints(nullptr), m_pGPAData(nullptr), m_bIsTerminating(false) {
  INIT_LOGGER_CLIENT(TEXT("ContextModule"), LL_DEBUG);

  LOG_INFO(TEXT("%s"), TEXT("ContextModule constructor enter"));

  m_pPlatformModule = pPlatformModule;
}
//////////////////////////////////////////////////////////////////////////
// ContextModule D'tor
//////////////////////////////////////////////////////////////////////////
ContextModule::~ContextModule() { RELEASE_LOGGER_CLIENT; }

//////////////////////////////////////////////////////////////////////////
// ContextModule::Initialize
//////////////////////////////////////////////////////////////////////////
cl_err_code ContextModule::Initialize(ocl_entry_points *pOclEntryPoints,
                                      ocl_gpa_data *pGPAData) {
  LOG_INFO(TEXT("%s"), TEXT("ContextModule::Initialize enter"));

  m_pOclEntryPoints = pOclEntryPoints;
  m_pGPAData = pGPAData;

  return CL_SUCCESS;
}
//////////////////////////////////////////////////////////////////////////
// ContextModule::Release
//////////////////////////////////////////////////////////////////////////
cl_err_code ContextModule::Release(bool bTerminate) {
  m_bIsTerminating = bTerminate;
  return CL_SUCCESS;
}

namespace Intel {
namespace OpenCL {
namespace Framework {

//////////////////////////////////////////////////////////////////////////
// Get All Objects from map
//////////////////////////////////////////////////////////////////////////
template <class Object> class AllObjectsFilter {
public:
  typedef std::vector<SharedPtr<Object>> ObjectsList;

  AllObjectsFilter(ObjectsList &out_list) : m_out_list(out_list) {}

  bool operator()(
      const SharedPtr<OCLObject<typename Object::OCLObjectHandleType>> &obj) {
    Object *pObj = dynamic_cast<Object *>(obj.GetPtr());

    if (nullptr != pObj) {
      m_out_list.push_back(pObj);
    }

    return true;
  }

private:
  ObjectsList &m_out_list;
};

template <class Object>
static void
GetAllObjects(OCLObjectsMap<typename Object::OCLObjectHandleType> &mapObjects,
              typename AllObjectsFilter<Object>::ObjectsList &to_remove) {
  to_remove.reserve(mapObjects.Count());

  AllObjectsFilter<Object> filter(to_remove);
  mapObjects.ForEach(filter);
}
} // namespace Framework
} // namespace OpenCL
} // namespace Intel

/******************************************************************
 *
 * Forcibly shutdown all contextes
 *
 ******************************************************************/
void ContextModule::ShutDown(bool wait_for_finish) {
  FrameworkProxy *framework_proxy = FrameworkProxy::Instance();
  ExecutionModule *execution_module = framework_proxy->GetExecutionModule();
  EventsManager *eventsManager = execution_module->GetEventsManager();

  // 1. Cancel all build tasks
  framework_proxy->CancelAllTasks(wait_for_finish);

  // 2. Switch all active command queues to a cancel state
  // If a command is enqueued just before this, there is a race condition
  // between 'RuntimeCommandTask::Cancel' and 'TaskExecutor::execute_command'.
  // Since we will try to finish all active queues below, this behavior try to
  // cancel all active queues is not necessary. So we intentionally disable this
  // code here.
#if 0
  execution_module->CancelAllActiveQueues();
#endif

  // 3. Signal all non-completed user events to push queues forward
  //    Release all non-released user events
  execution_module->ReleaseAllUserEvents(true);

  // 4. clFinish() of all queueus
  // FIXME: Some fpga tests intentionally write a kernel with infinite loop that
  // will cause hang at this line. So we temporarily allow FPGA emulator not to
  // wait for command queues finish.
  if (wait_for_finish &&
      framework_proxy->GetOCLConfig()->GetDeviceMode() != FPGA_EMU_DEVICE) {
    execution_module->FinishAllActiveQueues();
  }

  // 5. Delete all active queues
  execution_module->DeleteAllActiveQueues(true);

  // 6. Emulate Release of all objects maintained by user
  RemoveAllMemObjects(true);
  RemoveAllSamplers(true);
  RemoveAllKernels(true);
  RemoveAllPrograms(true);

  eventsManager->DisableNewEvents();
  m_mapContexts.DisableAdding();

  eventsManager->ReleaseAllEvents(false);

  m_mapContexts.SetPreserveUserHandles();
  m_mapContexts.ReleaseAllObjects(false);

  m_pPlatformModule->RemoveAllDevices(true);

// Intentionally disable this code due to shutdown issue
#if 0
  // FIXME: Autorun kernels will hold some internal objects so that devices
  // can't be closed during shutdown process. This is a workaround that we
  // don't wait for devices to close in FPGA emulator device mode. And the right
  // way is to refine execution model for autorun kernel so that we can decide
  // when to turn it off.
  if (framework_proxy->GetOCLConfig()->GetDeviceMode() != FPGA_EMU_DEVICE) {
    // 7. Ensure that all devices really closed
#ifdef _DEBUG
    const unsigned long long TIMEOUT = 100 * 1000000000LL; // 100 sec
    const unsigned long long endTime = HostTime() + TIMEOUT;
    while (0 < m_pPlatformModule->GetActiveDeviceCount()) {
      if (HostTime() > endTime) {
        DumpSharedPts(
            "ContextModule::ShutDown - Device Agents cannot be closed, "
            "time out. Only SharedPtrs local to intelocl DLL",
            true);
        break;
      }
    }
#else
    m_pPlatformModule->WaitForAllDevices();
#endif
  }
#endif

  // At that point still some internal threads in different DLLs may handle
  // SharedPtr's destruction We need to wait until all of them will end their
  // work. We will do this in the TerminateProcess() function by calling
  // shutdown callbacks of all DLLs to ensure their full shutdown
}

//////////////////////////////////////////////////////////////////////////
// ContextModule::CreateContext
//////////////////////////////////////////////////////////////////////////
cl_context
ContextModule::CreateContext(const cl_context_properties *clProperties,
                             cl_uint uiNumDevices, const cl_device_id *pDevices,
                             logging_fn pfnNotify, void *pUserData,
                             cl_err_code *pRrrcodeRet) {
  // cl_start;

  LOG_INFO(TEXT("Enter ContextModule::CreateContext (clProperties=%p, "
                "uiNumDevices=%u, pDevices=%p)"),
           clProperties, uiNumDevices, pDevices);

  if (nullptr == pDevices) {
    LOG_ERROR(TEXT("(!pDevices); return CL_INVALID_VALUE"));
    if (nullptr != pRrrcodeRet) {
      *pRrrcodeRet = CL_INVALID_VALUE;
    }
    return CL_INVALID_HANDLE;
  }

  if (!pfnNotify && pUserData) {
    LOG_ERROR(TEXT("(!pfnNotify && pUserData); return CL_INVALID_VALUE"));
    if (nullptr != pRrrcodeRet) {
      *pRrrcodeRet = CL_INVALID_VALUE;
    }
    return CL_INVALID_HANDLE;
  }

  if (nullptr != pRrrcodeRet) {
    *pRrrcodeRet = CL_SUCCESS;
  }

  SharedPtr<FissionableDevice> *ppDevices =
      new SharedPtr<FissionableDevice>[uiNumDevices];
  if (nullptr == ppDevices) {
    LOG_ERROR(TEXT("Failed to allocate memory for devices: new "
                   "Device[uiNumDevices] = NULL"));
    if (nullptr != pRrrcodeRet) {
      *pRrrcodeRet = CL_OUT_OF_HOST_MEMORY;
    }
    return CL_INVALID_HANDLE;
  }

  cl_err_code clErrRet = GetDevices(uiNumDevices, pDevices, ppDevices);
  if (CL_FAILED(clErrRet)) {
    delete[] ppDevices;
    if (nullptr != pRrrcodeRet) {
      *pRrrcodeRet = CL_INVALID_DEVICE;
    }
    return CL_INVALID_HANDLE;
  }

  cl_uint numRootDevices = 0;
  for (cl_uint i = 0; i < uiNumDevices; ++i) {
    if (ppDevices[i]->IsRootLevelDevice()) {
      ++numRootDevices;
    }
  }

  SharedPtr<Context> pContext;
  // check properties
  std::map<cl_context_properties, cl_context_properties> propertyMap;
  if (nullptr != clProperties) {
    size_t i = 0;

    while (0 != clProperties[i]) {
      if (CL_CONTEXT_PLATFORM == clProperties[i] &&
          !m_pPlatformModule->CheckPlatformId(
              (cl_platform_id)clProperties[i + 1])) {
        LOG_ERROR(TEXT("platform value specified in properties is "
                       "not a valid platform"));
        delete[] ppDevices;
        if (nullptr != pRrrcodeRet) {
          *pRrrcodeRet = CL_INVALID_PLATFORM;
        }
        return CL_INVALID_HANDLE;
      }
      if (propertyMap.find(clProperties[i]) != propertyMap.end()) {
        LOG_ERROR(TEXT("the same property name is specified more than once"));
        delete[] ppDevices;
        if (nullptr != pRrrcodeRet) {
          *pRrrcodeRet = CL_INVALID_PROPERTY;
        }
        return CL_INVALID_HANDLE;
      }

      std::vector<cl_context_properties> legalProperties = {
          CL_CONTEXT_PLATFORM, CL_CONTEXT_INTEROP_USER_SYNC};

      cl_context_properties currentProperty = clProperties[i];
      if (std::none_of(legalProperties.begin(), legalProperties.end(),
                       [currentProperty](const cl_context_properties &prop) {
                         return prop == currentProperty;
                       })) {
        LOG_ERROR(TEXT("context property name in properties is not "
                       "a supported property name"));
        delete[] ppDevices;
        if (nullptr != pRrrcodeRet) {
          *pRrrcodeRet = CL_INVALID_PROPERTY;
        }
        return CL_INVALID_HANDLE;
      }
      propertyMap[clProperties[i]] = clProperties[i + 1];
      i += 2;
    }
  }

  // Default error in case new() will fail
  clErrRet = CL_OUT_OF_HOST_MEMORY;
  {
    pContext = Context::Allocate(clProperties, uiNumDevices, numRootDevices,
                                 ppDevices, pfnNotify, pUserData, &clErrRet,
                                 m_pOclEntryPoints, m_pGPAData, *this);
  }
  if (CL_FAILED(clErrRet)) {
    LOG_ERROR(TEXT("Create context failed"));
    if (nullptr != pRrrcodeRet) {
      *pRrrcodeRet = clErrRet;
    }
    delete[] ppDevices;
    if (NULL != pContext.GetPtr()) {
      pContext->Release();
    }
    return CL_INVALID_HANDLE;
  }

  if (FrameworkProxy::Instance()->GetOCLConfig()->EnableParallelCopy())
    clErrRet = initializeLibraryProgram(pContext, uiNumDevices, ppDevices);

  delete[] ppDevices;

  if (CL_FAILED(clErrRet))
    return CL_INVALID_HANDLE;

  cl_context clContextId = (cl_context)m_mapContexts.AddObject(pContext);
  LOG_INFO(TEXT("CONTEXT_TEST: New context created. (id = %p)"), clContextId);

  return clContextId;
}
cl_context ContextModule::CreateContextFromType(
    const cl_context_properties *clProperties, cl_device_type clDeviceType,
    logging_fn pfnNotify, void *pUserData, cl_int *pErrcodeRet) {
  // cl_start;
  LOG_INFO(
      TEXT("Enter ContextModule::CreateContextFromType (clProperties=%p, "
           "clDeviceType=%llu, pfnNotify=%p, pUserData=%p, pErrcodeRet=%p)"),
      clProperties, (unsigned long long)clDeviceType, pfnNotify, pUserData,
      pErrcodeRet);

#ifdef _DEBUG
  assert(nullptr != m_pPlatformModule);
#endif
  cl_uint uiNumDevices = 0;

  // TODO: Handle new spec
  cl_err_code clErrRet = m_pPlatformModule->GetDeviceIDs(
      nullptr, clDeviceType, 0, nullptr, &uiNumDevices);
  if (CL_FAILED(clErrRet)) {
    LOG_ERROR(TEXT("GetDeviceIDs(%llu, 0, NULL, %p) = %s"),
              (unsigned long long)clDeviceType, &uiNumDevices,
              ClErrTxt(clErrRet));
    if (nullptr != pErrcodeRet) {
      *pErrcodeRet = clErrRet;
    }
    return CL_INVALID_HANDLE;
  }

  cl_device_id *pDevices = new cl_device_id[uiNumDevices];
  if (nullptr == pDevices) {
    LOG_ERROR(TEXT("new cl_device_id[%u] = NULL"), uiNumDevices);
    if (nullptr != pErrcodeRet) {
      *pErrcodeRet = CL_OUT_OF_HOST_MEMORY;
    }
    return CL_INVALID_HANDLE;
  }

  // TODO: Handle new spec
  clErrRet = m_pPlatformModule->GetDeviceIDs(nullptr, clDeviceType,
                                             uiNumDevices, pDevices, nullptr);
  if (CL_FAILED(clErrRet)) {
    LOG_ERROR(TEXT("GetDeviceIDs(%llu, %u, %p, NULL) = %s"),
              (unsigned long long)clDeviceType, uiNumDevices, pDevices,
              ClErrTxt(clErrRet));
    if (nullptr != pErrcodeRet) {
      *pErrcodeRet = clErrRet;
    }
    delete[] pDevices;
    return CL_INVALID_HANDLE;
  }
  cl_context clContext = CreateContext(clProperties, uiNumDevices, pDevices,
                                       pfnNotify, pUserData, pErrcodeRet);
  delete[] pDevices;
  // cl_return clContext;
  return clContext;
}

//////////////////////////////////////////////////////////////////////////
// ContextModule::SetContextDestructorCallback
//////////////////////////////////////////////////////////////////////////
cl_err_code ContextModule::SetContextDestructorCallback(
    cl_context clContext,
    void(CL_CALLBACK *pfnNotify)(cl_context context, void *userData),
    void *pUserData) {
  LOG_DEBUG(
      TEXT("Enter SetContextDestructorCallback(clContext=%p, pUserData=%p)"),
      clContext, pUserData);

  cl_err_code clErrRet = CL_SUCCESS;
  SharedPtr<Context> pContext =
      m_mapContexts.GetOCLObject((_cl_context_int *)clContext)
          .DynamicCast<Context>();
  // Return CL_INVALID_CONTEXT if context is not a valid context.
  if (!pContext.GetPtr()) {
    LOG_DEBUG(TEXT("m_mapContexts.GetOCLObject(%p, %p) = nullptr"), clContext,
              &pContext);
    return CL_INVALID_CONTEXT;
  }

  // Return CL_INVALID_VALUE if pfn_notify is NULL.
  if (!pfnNotify) {
    LOG_DEBUG(TEXT("%s"), TEXT("(!pfnNotify); return CL_INVALID_VALUE"));
    return CL_INVALID_VALUE;
  }

  clErrRet = pContext->setDestructorCallback(clContext, pfnNotify, pUserData);
  if (CL_FAILED(clErrRet))
    pContext->NotifyError("clSetContextDestructorCallback failed", &clErrRet,
                          sizeof(cl_int));
  return clErrRet;
}

//////////////////////////////////////////////////////////////////////////
// ContextModule::CheckDevices
//////////////////////////////////////////////////////////////////////////

cl_err_code ContextModule::GetRootDevices(cl_uint uiNumDevices,
                                          const cl_device_id *pclDeviceIds,
                                          SharedPtr<Device> *ppDevices) {
  LOG_DEBUG(TEXT("ContextModule::GetRootDevices enter. uiNumDevices=%u, "
                 "pclDeviceIds=%p, ppDevices=%p"),
            uiNumDevices, pclDeviceIds, ppDevices);
#ifdef _DEBUG
  assert(nullptr != m_pPlatformModule);
  assert((nullptr != ppDevices) && (0 != uiNumDevices));
#endif

  cl_uint rootId = 0;
  // go through device ids and get the device from the platform module

  for (cl_uint ui = 0; ui < uiNumDevices; ++ui) {
    SharedPtr<FissionableDevice> pDevice =
        m_pPlatformModule->GetDevice(pclDeviceIds[ui]);
    if (NULL == pDevice.GetPtr()) {
      LOG_ERROR(TEXT("m_pPlatformModule->GetDevice(%p) = NULL"),
                pclDeviceIds[ui]);
      return CL_ERR_KEY_NOT_FOUND;
    }
    if (pDevice->IsRootLevelDevice()) {
      ppDevices[rootId++] = pDevice->GetRootDevice();
    }
  }
  return CL_SUCCESS;
}

cl_err_code ContextModule::GetDevices(cl_uint uiNumDevices,
                                      const cl_device_id *pclDeviceIds,
                                      SharedPtr<FissionableDevice> *ppDevices) {
  LOG_DEBUG(TEXT("ContextModule::GetRootDevices enter. uiNumDevices=%u, "
                 "pclDeviceIds=%p, ppDevices=%p"),
            uiNumDevices, pclDeviceIds, ppDevices);
#ifdef _DEBUG
  assert(nullptr != m_pPlatformModule);
  assert((nullptr != ppDevices) && (0 != uiNumDevices));
#endif

  // go through device ids and get the device from the platform module

  for (cl_uint ui = 0; ui < uiNumDevices; ++ui) {
    SharedPtr<FissionableDevice> pDevice =
        m_pPlatformModule->GetDevice(pclDeviceIds[ui]);
    if (NULL == pDevice.GetPtr()) {
      LOG_ERROR(TEXT("m_pPlatformModule->GetDevice(%p) = NULL"),
                pclDeviceIds[ui]);
      return CL_ERR_KEY_NOT_FOUND;
    }
    ppDevices[ui] = pDevice;
  }
  return CL_SUCCESS;
}
//////////////////////////////////////////////////////////////////////////
// ContextModule::RetainContext
//////////////////////////////////////////////////////////////////////////
cl_err_code ContextModule::RetainContext(cl_context context) {
  LOG_INFO(TEXT("ContextModule::RetainContext enter. context=%p"), context);
  cl_err_code clErrRet = CL_SUCCESS;
  (void)clErrRet;
  SharedPtr<Context> pContext =
      m_mapContexts.GetOCLObject((_cl_context_int *)context)
          .DynamicCast<Context>();
  if (NULL == pContext.GetPtr()) {
    LOG_ERROR(TEXT("m_mapContexts.GetOCLObject(%p, %p) = %s"), context,
              &pContext, ClErrTxt(clErrRet));
    return CL_INVALID_CONTEXT;
  }
  return pContext->Retain();
}

//////////////////////////////////////////////////////////////////////////
// ContextModule::ReleaseContext
//////////////////////////////////////////////////////////////////////////
cl_err_code ContextModule::ReleaseContext(cl_context context) {
  LOG_INFO(TEXT("ContextModule::ReleaseContext enter. context=%p"), context);

  // Release library program on the first attempt of ReleaseContext. Library
  // program should be released at the last attempt, however, it is tricky to
  // track reference count of context within library program.
  cl_err_code err = releaseLibraryProgram(context);
  if (CL_FAILED(err)) {
    LOG_ERROR(TEXT("releaseLibraryProgram failed, err %d"), err);
    return CL_OUT_OF_RESOURCES;
  }

  SharedPtr<OCLObject<_cl_context_int>> pContext =
      m_mapContexts.GetOCLObject((_cl_context_int *)context);
  if (pContext.GetRefCnt() > 2) {
    LOG_INFO(
        TEXT("Warning: context %p will not have been deleted after this call - "
             "the user might have forgotten to release some objects"),
        context);
  }

  err = m_mapContexts.ReleaseObject((_cl_context_int *)context);
  return ((CL_ERR_KEY_NOT_FOUND == err) ? CL_INVALID_CONTEXT : err);
}

//////////////////////////////////////////////////////////////////////////
// ContextModule::GetContextInfo
//////////////////////////////////////////////////////////////////////////
cl_err_code ContextModule::GetContextInfo(cl_context context,
                                          cl_context_info param_name,
                                          size_t param_value_size,
                                          void *param_value,
                                          size_t *param_value_size_ret) {
  LOG_DEBUG(
      TEXT("ContextModule::GetContextInfo enter. context=%p, param_name=%u, "
           "param_value_size=%zu, param_value=%p, param_value_size_ret=%p"),
      context, param_name, param_value_size, param_value, param_value_size_ret);

  cl_err_code clErrRet = CL_SUCCESS;
  SharedPtr<Context> pContext =
      m_mapContexts.GetOCLObject((_cl_context_int *)context)
          .DynamicCast<Context>();
  if (NULL == pContext.GetPtr()) {
    LOG_ERROR(TEXT("m_mapContexts.GetOCLObject(%p, %p) = %s"), context,
              &pContext, ClErrTxt(clErrRet));
    return CL_INVALID_CONTEXT;
  }
  clErrRet = pContext->GetInfo((cl_int)param_name, param_value_size,
                               param_value, param_value_size_ret);
  if (CL_FAILED(clErrRet)) {
    pContext->NotifyError("clGetContextInfo failed", &clErrRet, sizeof(cl_int));
  }
  return clErrRet;
}
cl_program ContextModule::CreateProgramWithIL(cl_context clContext,
                                              const unsigned char *pIL,
                                              size_t length,
                                              cl_int *pErrcodeRet) {
  LOG_INFO(
      TEXT("CreateProgramWithIL enter. clContext=%p, pIL=%p, szLength=%zu, "
           "pErrcodeRet=%p"),
      clContext, pIL, length, pErrcodeRet);

  if (0 == length || nullptr == pIL) {
    // invalid value
    LOG_ERROR(TEXT("%zu == length || %p == pIL"), length, pIL);
    if (nullptr != pErrcodeRet) {
      *pErrcodeRet = CL_INVALID_VALUE;
    }
    return CL_INVALID_HANDLE;
  }
  cl_err_code clErrRet = CL_SUCCESS;
  // get the context from the contexts map list
  SharedPtr<Context> pContext =
      m_mapContexts.GetOCLObject((_cl_context_int *)clContext)
          .DynamicCast<Context>();
  if (NULL == pContext.GetPtr()) {
    LOG_ERROR(TEXT("m_mapContexts.GetOCLObject(%p, %p) = %s"), clContext,
              &pContext, ClErrTxt(clErrRet));
    if (NULL != pErrcodeRet) {
      *pErrcodeRet = CL_INVALID_CONTEXT;
    }
    return CL_INVALID_HANDLE;
  }
  SharedPtr<Program> pProgram;
  clErrRet = pContext->CreateProgramWithIL(pIL, length, &pProgram);
  if (CL_FAILED(clErrRet)) {
    if (NULL != pErrcodeRet) {
      *pErrcodeRet = clErrRet;
    }
    pContext->NotifyError("clCreateProgramWithIL failed", &clErrRet,
                          sizeof(cl_int));
    if (pProgram) {
      pContext->RemoveProgram(pProgram->GetHandle());
      pProgram->Release();
    }
    return CL_INVALID_HANDLE;
  }
  clErrRet = m_mapPrograms.AddObject(pProgram, false);
  if (CL_FAILED(clErrRet)) {
    if (NULL != pErrcodeRet) {
      *pErrcodeRet = clErrRet;
    }
    pContext->NotifyError("clCreateProgramWithIL failed", &clErrRet,
                          sizeof(cl_int));
    pContext->RemoveProgram(pProgram->GetHandle());
    pProgram->Release();
    return CL_INVALID_HANDLE;
  }
  if (NULL != pErrcodeRet) {
    *pErrcodeRet = CL_SUCCESS;
  }
  return pProgram->GetHandle();
}
//////////////////////////////////////////////////////////////////////////
// ContextModule::CreateProgramWithSource
//////////////////////////////////////////////////////////////////////////
cl_program ContextModule::CreateProgramWithSource(cl_context clContext,
                                                  cl_uint uiCount,
                                                  const char **ppcStrings,
                                                  const size_t *szLengths,
                                                  cl_int *pErrcodeRet) {
  LOG_INFO(TEXT("CreateProgramWithSource enter. clContext=%p, uiCount=%u, "
                "ppcStrings=%p, szLengths=%p, pErrcodeRet=%p"),
           clContext, uiCount, ppcStrings, szLengths, pErrcodeRet);

  cl_err_code clErrRet = CL_SUCCESS;
  // get the context from the contexts map list
  SharedPtr<Context> pContext =
      m_mapContexts.GetOCLObject((_cl_context_int *)clContext)
          .DynamicCast<Context>();
  if (NULL == pContext.GetPtr()) {
    LOG_ERROR(TEXT("m_mapContexts.GetOCLObject(%p, %p) = %s"), clContext,
              &pContext, ClErrTxt(clErrRet));
    if (NULL != pErrcodeRet) {
      *pErrcodeRet = CL_INVALID_CONTEXT;
    }
    return CL_INVALID_HANDLE;
  }
  SharedPtr<Program> pProgram;
  clErrRet = pContext->CreateProgramWithSource(uiCount, ppcStrings, szLengths,
                                               &pProgram);
  if (CL_FAILED(clErrRet)) {
    if (NULL != pErrcodeRet) {
      *pErrcodeRet = clErrRet;
    }
    pContext->NotifyError("clCreateProgramWithSource failed", &clErrRet,
                          sizeof(cl_int));
    if (pProgram) {
      pContext->RemoveProgram(pProgram->GetHandle());
      pProgram->Release();
    }
    return CL_INVALID_HANDLE;
  }
  clErrRet = m_mapPrograms.AddObject(pProgram, false);
  if (CL_FAILED(clErrRet)) {
    if (NULL != pErrcodeRet) {
      *pErrcodeRet = clErrRet;
    }
    pContext->NotifyError("clCreateProgramWithSource failed", &clErrRet,
                          sizeof(cl_int));
    pContext->RemoveProgram(pProgram->GetHandle());
    pProgram->Release();
    return CL_INVALID_HANDLE;
  }
  if (NULL != pErrcodeRet) {
    *pErrcodeRet = CL_SUCCESS;
  }
  return pProgram->GetHandle();
}

//////////////////////////////////////////////////////////////////////////
// ContextModule::CreateProgramWithBinary
//////////////////////////////////////////////////////////////////////////
cl_program ContextModule::CreateProgramWithBinary(
    cl_context clContext, cl_uint uiNumDevices,
    const cl_device_id *pclDeviceList, const size_t *pszLengths,
    const unsigned char **ppBinaries, cl_int *piBinaryStatus, cl_int *pErrRet) {
  LOG_INFO(
      TEXT("CreateProgramWithBinary enter. clContext=%p, uiNumDevices=%u, "
           "pclDeviceList=%p, pszLengths=%p, ppBinaries=%p, piBinaryStatus=%p"),
      clContext, uiNumDevices, pclDeviceList, pszLengths, ppBinaries,
      piBinaryStatus);
  if (nullptr == pclDeviceList || 0 == uiNumDevices || nullptr == pszLengths ||
      nullptr == ppBinaries) {
    // invalid value
    LOG_ERROR(TEXT("NULL == pclDeviceList || 0 == uiNumDevices || "
                   "NULL == pszLengths || NULL == ppBinaries"));
    if (NULL != pErrRet) {
      *pErrRet = CL_INVALID_VALUE;
    }
    return CL_INVALID_HANDLE;
  }
  // get the context from the contexts map list
  SharedPtr<Context> pContext =
      m_mapContexts.GetOCLObject((_cl_context_int *)clContext)
          .DynamicCast<Context>();
  if (NULL == pContext.GetPtr()) {
    LOG_ERROR(TEXT("m_mapContexts.GetOCLObject(%p, %p) = NULL"), clContext,
              &pContext);
    if (NULL != pErrRet) {
      *pErrRet = CL_INVALID_CONTEXT;
    }
    return CL_INVALID_HANDLE;
  }
  SharedPtr<Program> pProgram;
  cl_err_code clErrRet =
      pContext->CreateProgramWithBinary(uiNumDevices, pclDeviceList, pszLengths,
                                        ppBinaries, piBinaryStatus, &pProgram);
  if (CL_FAILED(clErrRet)) {
    if (NULL != pErrRet) {
      *pErrRet = clErrRet;
    }
    pContext->NotifyError("clCreateProgramWithBinary failed", &clErrRet,
                          sizeof(cl_int));
    if (pProgram) {
      pContext->RemoveProgram(pProgram->GetHandle());
      pProgram->Release();
    }
    return CL_INVALID_HANDLE;
  }
  clErrRet = m_mapPrograms.AddObject(pProgram, false);
  if (CL_FAILED(clErrRet)) {
    if (NULL != pErrRet) {
      *pErrRet = CL_OUT_OF_HOST_MEMORY;
    }
    pContext->NotifyError("clCreateProgramWithBinary failed", &clErrRet,
                          sizeof(cl_int));
    pContext->RemoveProgram(pProgram->GetHandle());
    pProgram->Release();
    return CL_INVALID_HANDLE;
  }
  if (NULL != pErrRet) {
    *pErrRet = CL_SUCCESS;
  }
  return pProgram->GetHandle();
}

//////////////////////////////////////////////////////////////////////////
// ContextModule::CreateProgramWithBuiltInKernels
//////////////////////////////////////////////////////////////////////////
cl_program ContextModule::CreateProgramWithBuiltInKernels(
    cl_context clContext, cl_uint uiNumDevices,
    const cl_device_id *pclDeviceList, const char *szKernelNames,
    cl_int *pErrcodeRet) {
  LOG_INFO(TEXT("CreateProgramWithBinary enter. clContext=%p, uiNumDevices=%u"),
           clContext, uiNumDevices);
  if (nullptr == pclDeviceList || 0 == uiNumDevices ||
      nullptr == szKernelNames) {
    // invalid value
    LOG_ERROR(TEXT("%S"), TEXT("NULL == pclDeviceList || 0 == uiNumDevices || "
                               "NULL == szKernelNames"));
    if (NULL != pErrcodeRet) {
      *pErrcodeRet = CL_INVALID_VALUE;
    }
    return CL_INVALID_HANDLE;
  }
  // get the context from the contexts map list
  SharedPtr<Context> pContext =
      m_mapContexts.GetOCLObject((_cl_context_int *)clContext)
          .DynamicCast<Context>();
  if (NULL == pContext.GetPtr()) {
    LOG_ERROR(TEXT("m_mapContexts.GetOCLObject(%p) = NULL"), clContext);
    if (nullptr != pErrcodeRet) {
      *pErrcodeRet = CL_INVALID_CONTEXT;
    }
    return CL_INVALID_HANDLE;
  }
  SharedPtr<Program> pProgram = nullptr;
  cl_err_code clErrRet = pContext->CreateProgramWithBuiltInKernels(
      uiNumDevices, pclDeviceList, szKernelNames, &pProgram);
  if (CL_FAILED(clErrRet)) {
    if (nullptr != pErrcodeRet) {
      *pErrcodeRet = clErrRet;
    }
    pContext->NotifyError("CreateProgramWithBuiltInKernels failed", &clErrRet,
                          sizeof(cl_int));
    if (pProgram) {
      pContext->RemoveProgram(pProgram->GetHandle());
      pProgram->Release();
    }
    return CL_INVALID_HANDLE;
  }
  clErrRet = m_mapPrograms.AddObject(pProgram, false);
  if (CL_FAILED(clErrRet)) {
    if (nullptr != pErrcodeRet) {
      *pErrcodeRet = CL_OUT_OF_HOST_MEMORY;
    }
    pContext->NotifyError("CreateProgramWithBuiltInKernels failed", &clErrRet,
                          sizeof(cl_int));
    pContext->RemoveProgram(pProgram->GetHandle());
    pProgram->Release();
    return CL_INVALID_HANDLE;
  }
  if (nullptr != pErrcodeRet) {
    *pErrcodeRet = CL_SUCCESS;
  }
  return pProgram->GetHandle();
}

//////////////////////////////////////////////////////////////////////////
// ContextModule::RetainProgram
//////////////////////////////////////////////////////////////////////////
cl_err_code ContextModule::RetainProgram(cl_program clProgram) {
  LOG_INFO(TEXT("Enter RetainProgram (clProgram=%p)"), clProgram);

  SharedPtr<Program> pProgram =
      m_mapPrograms.GetOCLObject((_cl_program_int *)clProgram)
          .DynamicCast<Program>();
  if (NULL == pProgram.GetPtr()) {
    LOG_ERROR(TEXT("program %p is invalid program"), clProgram);
    return CL_INVALID_PROGRAM;
  }
  return pProgram->Retain();
}
//////////////////////////////////////////////////////////////////////////
// ContextModule::ReleaseProgram
//////////////////////////////////////////////////////////////////////////
cl_err_code ContextModule::ReleaseProgram(cl_program clProgram) {
  LOG_INFO(TEXT("Enter ReleaseProgram (clProgram=%p)"), (void *)clProgram);

  SharedPtr<Program> pProgram =
      m_mapPrograms.GetOCLObject((_cl_program_int *)clProgram)
          .DynamicCast<Program>();
  if (NULL == pProgram.GetPtr()) {
    LOG_ERROR(TEXT("program %p is invalid program"), clProgram);
    return CL_INVALID_PROGRAM;
  }
  SharedPtr<Context> pContext = pProgram->GetContext();
  if (NULL == pContext.GetPtr()) {
    LOG_ERROR(TEXT("pProgram->GetContext returned NULL"));
    return CL_INVALID_PROGRAM;
  }

  long newRef = pProgram->Release();
  if (newRef < 0) {
    LOG_ERROR(TEXT("pProgram->Release() failed, newRef %ld"), newRef);
    return CL_INVALID_PROGRAM;
  } else if (0 == newRef) {
    cl_err_code clErrRet = pContext->RemoveProgram(clProgram);
    if (CL_FAILED(clErrRet)) {
      LOG_ERROR(TEXT("pContext->RemoveProgram failed, err %d"), clErrRet);
      return CL_ERR_OUT(clErrRet);
    }

    clErrRet = m_mapPrograms.RemoveObject((_cl_program_int *)clProgram);
    if (CL_FAILED(clErrRet)) {
      LOG_ERROR(TEXT("m_mapPrograms.RemoveObject failed, err %d"), clErrRet);
      return CL_ERR_OUT(clErrRet);
    }
  }
  return CL_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////
// RemoveAllPrograms
//////////////////////////////////////////////////////////////////////////
void ContextModule::RemoveAllPrograms(bool preserve_user_handles) {
  m_mapPrograms.DisableAdding();

  AllObjectsFilter<Program>::ObjectsList to_remove;
  GetAllObjects<Program>(m_mapPrograms, to_remove);

  if (preserve_user_handles) {
    m_mapPrograms.SetPreserveUserHandles();
  }

  for (AllObjectsFilter<Program>::ObjectsList::iterator it = to_remove.begin();
       it != to_remove.end(); ++it) {
    SharedPtr<Program> &pObj = *it;

    cl_program handle = pObj->GetHandle();
    pObj->GetContext()->RemoveProgram(handle);
    m_mapPrograms.RemoveObject((_cl_program_int *)handle);
  }

  to_remove.clear();
  m_mapPrograms.ReleaseAllObjects(false);
}

//////////////////////////////////////////////////////////////////////////
// ContextModule::CompileProgram
//////////////////////////////////////////////////////////////////////////
cl_int ContextModule::CompileProgram(
    cl_program clProgram, cl_uint uiNumDevices,
    const cl_device_id *pclDeviceList, const char *pcOptions,
    cl_uint num_input_headers, const cl_program *pclInputHeaders,
    const char **header_include_names,
    void(CL_CALLBACK *pfn_notify)(cl_program program, void *user_data),
    void *pUserData) {
  LOG_INFO(TEXT("CompileProgram enter. clProgram=%p, uiNumDevices=%u, "
                "pclDeviceList=%p, pcOptions=%p, num_input_headers=%u, "
                "pclInputHeaders=%p, header_include_names=%p, pUserData=%p"),
           clProgram, uiNumDevices, pclDeviceList, pcOptions, num_input_headers,
           pclInputHeaders, header_include_names, pUserData);

  if ((0 == num_input_headers) &&
      ((nullptr != pclInputHeaders) || (nullptr != header_include_names))) {
    return CL_INVALID_VALUE;
  }

  if ((0 != num_input_headers) &&
      ((nullptr == pclInputHeaders) || (nullptr == header_include_names))) {
    return CL_INVALID_VALUE;
  }

  if ((nullptr == pfn_notify) && (nullptr != pUserData)) {
    return CL_INVALID_VALUE;
  }

  // get program from programs map list
  SharedPtr<Program> pProgram =
      m_mapPrograms.GetOCLObject((_cl_program_int *)clProgram)
          .DynamicCast<Program>();
  if (NULL == pProgram.GetPtr()) {
    LOG_ERROR(TEXT("program %p isn't valid program"), clProgram);
    return CL_INVALID_PROGRAM;
  }

  SharedPtr<Context> pContext = pProgram->GetContext();
  cl_int clErr = pContext->CompileProgram(
      clProgram, uiNumDevices, pclDeviceList, num_input_headers,
      pclInputHeaders, header_include_names, pcOptions, pfn_notify, pUserData);

  return clErr;
}
//////////////////////////////////////////////////////////////////////////
// ContextModule::LinkProgram
//////////////////////////////////////////////////////////////////////////
cl_program ContextModule::LinkProgram(
    cl_context clContext, cl_uint uiNumDevices,
    const cl_device_id *pclDeviceList, const char *pcOptions,
    cl_uint uiNumInputPrograms, const cl_program *pclInputPrograms,
    void(CL_CALLBACK *pfn_notify)(cl_program program, void *user_data),
    void *pUserData, cl_int *pErrcodeRet) {
  LOG_INFO(TEXT("LinkProgram enter. clContext=%p, uiNumDevices=%u, "
                "pclDeviceList=%p, pcOptions=%p, uiNumInputPrograms=%u, "
                "pclInputPrograms=%p, pUserData=%p"),
           clContext, uiNumDevices, pclDeviceList, pcOptions,
           uiNumInputPrograms, pclInputPrograms, pUserData);

  if ((nullptr == pfn_notify) && (nullptr != pUserData)) {
    if (NULL != pErrcodeRet) {
      *pErrcodeRet = CL_INVALID_VALUE;
    }
    return CL_INVALID_HANDLE;
  }

  if ((0 == uiNumInputPrograms) || (nullptr == pclInputPrograms)) {
    if (NULL != pErrcodeRet) {
      *pErrcodeRet = CL_INVALID_VALUE;
    }
    return CL_INVALID_HANDLE;
  }

  if ((nullptr == pclDeviceList) && (0 < uiNumDevices)) {
    if (NULL != pErrcodeRet) {
      *pErrcodeRet = CL_INVALID_VALUE;
    }
    return CL_INVALID_HANDLE;
  }

  if ((nullptr != pclDeviceList) && (0 == uiNumDevices)) {
    if (NULL != pErrcodeRet) {
      *pErrcodeRet = CL_INVALID_VALUE;
    }
    return CL_INVALID_HANDLE;
  }

  // get context from contexts map list
  SharedPtr<Context> pContext =
      m_mapContexts.GetOCLObject((_cl_context_int *)clContext)
          .DynamicCast<Context>();
  if (NULL == pContext.GetPtr()) {
    LOG_ERROR(TEXT("context %p isn't valid context"), clContext);
    if (NULL != pErrcodeRet) {
      *pErrcodeRet = CL_INVALID_CONTEXT;
    }
    return CL_INVALID_HANDLE;
  }

  SharedPtr<Program> pProgram;

  if (0 < uiNumDevices) {
    cl_err_code clErrRet =
        pContext->CreateProgramForLink(uiNumDevices, pclDeviceList, &pProgram);
    if (CL_FAILED(clErrRet)) {
      if (NULL != pErrcodeRet) {
        *pErrcodeRet = clErrRet;
      }
      if (pProgram) {
        pContext->RemoveProgram(pProgram->GetHandle());
        pProgram->Release();
      }
      return CL_INVALID_HANDLE;
    }
  } else {
    cl_uint uiNumContextDevices = 0;
    cl_device_id *pContextDevices =
        pContext->GetDeviceIds(&uiNumContextDevices);

    cl_err_code clErrRet = pContext->CreateProgramForLink(
        (cl_uint)uiNumContextDevices, pContextDevices, &pProgram);
    if (CL_FAILED(clErrRet)) {
      if (NULL != pErrcodeRet) {
        *pErrcodeRet = clErrRet;
      }
      if (pProgram) {
        pContext->RemoveProgram(pProgram->GetHandle());
        pProgram->Release();
      }
      return CL_INVALID_HANDLE;
    }
  }

  cl_err_code clErrRet = m_mapPrograms.AddObject(pProgram, false);
  if (CL_FAILED(clErrRet)) {
    if (NULL != pErrcodeRet) {
      *pErrcodeRet = clErrRet;
    }
    pContext->RemoveProgram(pProgram->GetHandle());
    pProgram->Release();
    return CL_INVALID_HANDLE;
  }

  clErrRet = pContext->LinkProgram(
      pProgram->GetHandle(), uiNumDevices, pclDeviceList, uiNumInputPrograms,
      pclInputPrograms, pcOptions, pfn_notify, pUserData);

  if (NULL != pErrcodeRet) {
    *pErrcodeRet = clErrRet;
  }
  // we should return a valid program handle even if the linking has failed
  return pProgram->GetHandle();
}
//////////////////////////////////////////////////////////////////////////
// ContextModule::BuildProgram
//////////////////////////////////////////////////////////////////////////
cl_int ContextModule::BuildProgram(
    cl_program clProgram, cl_uint uiNumDevices,
    const cl_device_id *pclDeviceList, const char *pcOptions,
    void(CL_CALLBACK *pfn_notify)(cl_program program, void *user_data),
    void *pUserData) {
  LOG_INFO(TEXT("BuildProgram enter. clProgram=%p, uiNumDevices=%u, "
                "pclDeviceList=%p, pcOptions=%p, pUserData=%p"),
           clProgram, uiNumDevices, pclDeviceList, pcOptions, pUserData);

  if ((nullptr == pfn_notify) && (NULL != pUserData)) {
    return CL_INVALID_VALUE;
  }

  // get program from programs map list
  SharedPtr<Program> pProgram =
      m_mapPrograms.GetOCLObject((_cl_program_int *)clProgram)
          .DynamicCast<Program>();
  if (NULL == pProgram.GetPtr()) {
    LOG_ERROR(TEXT("program %p isn't valid program"), clProgram);
    return CL_INVALID_PROGRAM;
  }

  SharedPtr<Context> pContext = pProgram->GetContext();
  cl_int clErr = pContext->BuildProgram(clProgram, uiNumDevices, pclDeviceList,
                                        pcOptions, pfn_notify, pUserData);

  return clErr;
}
//////////////////////////////////////////////////////////////////////////
// ContextModule::GetProgramInfo
//////////////////////////////////////////////////////////////////////////
cl_int ContextModule::GetProgramInfo(cl_program clProgram,
                                     cl_program_info clParamName,
                                     size_t szParamValueSize, void *pParamValue,
                                     size_t *pszParamValueSizeRet) {
  LOG_INFO(
      TEXT("GetProgramInfo enter. clProgram=%p, clParamName=%u, "
           "szParamValueSize=%zu, pParamValue=%p, pszParamValueSizeRet=%p"),
      clProgram, clParamName, szParamValueSize, pParamValue,
      pszParamValueSizeRet);

  cl_err_code clErrRet = CL_SUCCESS;
  (void)clErrRet;
  SharedPtr<Program> pProgram =
      m_mapPrograms.GetOCLObject((_cl_program_int *)clProgram)
          .DynamicCast<Program>();
  if (NULL == pProgram.GetPtr()) {
    LOG_ERROR(TEXT("m_mapPrograms.GetOCLObject(%p, %p) = %s"), clProgram,
              &pProgram, ClErrTxt(clErrRet));
    return CL_INVALID_PROGRAM;
  }
  return pProgram->GetInfo((cl_int)clParamName, szParamValueSize, pParamValue,
                           pszParamValueSizeRet);
}

//////////////////////////////////////////////////////////////////////////
// ContextModule::clSetProgramSpecializationConstant
//////////////////////////////////////////////////////////////////////////
cl_int ContextModule::clSetProgramSpecializationConstant(
    cl_program clProgram, cl_uint uiSpecId, size_t szSpecSize,
    const void *pSpecValue) {
  LOG_INFO(TEXT("clSetProgramSpecializationConstant enter. program=%p, "
                "spec_id=%u, spec_size=%zu, spec_value=%p"),
           clProgram, uiSpecId, szSpecSize, pSpecValue);
  SharedPtr<Program> pProgram =
      m_mapPrograms.GetOCLObject((_cl_program_int *)clProgram)
          .DynamicCast<Program>();
  if (NULL == pProgram.GetPtr()) {
    LOG_ERROR(TEXT("program %p isn't valid program"), clProgram);
    return CL_INVALID_PROGRAM;
  }

  SharedPtr<Context> pContext = pProgram->GetContext();
  return pContext->SetSpecializationConstant(pProgram, uiSpecId, szSpecSize,
                                             pSpecValue);
}

//////////////////////////////////////////////////////////////////////////
// ContextModule::GetProgramBuildInfo
//////////////////////////////////////////////////////////////////////////
cl_int ContextModule::GetProgramBuildInfo(
    cl_program clProgram, cl_device_id clDevice, cl_program_info clParamName,
    size_t szParamValueSize, void *pParamValue, size_t *pszParamValueSizeRet) {
  LOG_INFO(TEXT("GetProgramBuildInfo enter. clProgram=%p, clDevice=%p, "
                "clParamName=%u, szParamValueSize=%zu, pParamValue=%p, "
                "pszParamValueSizeRet=%p"),
           clProgram, clDevice, clParamName, szParamValueSize, pParamValue,
           pszParamValueSizeRet);

  cl_err_code clErrRet = CL_SUCCESS;
  (void)clErrRet;
  SharedPtr<Program> pProgram =
      m_mapPrograms.GetOCLObject((_cl_program_int *)clProgram)
          .DynamicCast<Program>();
  if (NULL == pProgram.GetPtr()) {
    LOG_ERROR(TEXT("m_mapPrograms.GetOCLObject(%p, %p) = %s"), clProgram,
              &pProgram, ClErrTxt(clErrRet));
    return CL_INVALID_PROGRAM;
  }
  return pProgram->GetBuildInfo(clDevice, clParamName, szParamValueSize,
                                pParamValue, pszParamValueSizeRet);
}
//////////////////////////////////////////////////////////////////////////
// ContextModule::CreateKernel
//////////////////////////////////////////////////////////////////////////
cl_kernel ContextModule::CreateKernel(cl_program clProgram,
                                      const char *pscKernelName,
                                      cl_int *piErr) {
  LOG_INFO(TEXT("CreateKernel enter. clProgram=%p, pscKernelName=%p, piErr=%p"),
           clProgram, pscKernelName, piErr);

  // get program object
  SharedPtr<Program> pProgram =
      m_mapPrograms.GetOCLObject((_cl_program_int *)clProgram)
          .DynamicCast<Program>();
  if (NULL == pProgram.GetPtr()) {
    LOG_ERROR(TEXT("clProgram is invalid program"));
    if (nullptr != piErr) {
      *piErr = CL_INVALID_PROGRAM;
    }
    return CL_INVALID_HANDLE;
  }

  // create new kernel
  SharedPtr<Kernel> pKernel = nullptr;
  cl_err_code clErrRet = pProgram->CreateKernel(pscKernelName, &pKernel);
  if (nullptr != piErr) {
    *piErr = CL_ERR_OUT(clErrRet);
  }

  if (NULL != pKernel.GetPtr()) {
    // add new kernel to the context module's kernels list
    m_mapKernels.AddObject(pKernel, false);
    if (nullptr != piErr) {
      *piErr = CL_SUCCESS;
    }
    // return handle
    return pKernel->GetHandle();
  }

  return CL_INVALID_HANDLE;
}
//////////////////////////////////////////////////////////////////////////
// ContextModule::CloneKernel
//////////////////////////////////////////////////////////////////////////
cl_kernel ContextModule::CloneKernel(cl_kernel source_kernel, cl_int *pErr) {
  LOG_INFO(TEXT("ContextModule::CloneKernel enter. source_kernel=%p"),
           source_kernel);
  SharedPtr<Kernel> pSrcKernel =
      m_mapKernels.GetOCLObject((_cl_kernel_int *)source_kernel)
          .DynamicCast<Kernel>();
  if (NULL == pSrcKernel.GetPtr()) {
    LOG_ERROR(TEXT("GetOCLObject(%p, %p) = NULL"), source_kernel, &pSrcKernel);
    if (NULL != pErr)
      *pErr = CL_INVALID_KERNEL;
    return CL_INVALID_HANDLE;
  }

  SharedPtr<Program> pProgram = pSrcKernel->GetProgram();
  // create new kernel
  SharedPtr<Kernel> pNewKernel = nullptr;
  cl_err_code clErrRet =
      pProgram->CreateKernel(pSrcKernel->GetName(), &pNewKernel);
  if (CL_SUCCESS != clErrRet) {
    if (NULL != pErr)
      *pErr = CL_ERR_OUT(clErrRet);
    return CL_INVALID_HANDLE;
  }

  size_t numOfArgs = pSrcKernel->GetKernelArgsCount();
  for (size_t i = 0; i < numOfArgs; ++i) {
    const KernelArg *src_arg = pSrcKernel->GetKernelArg(i);
    if (src_arg->IsValid())
      if (CL_SUCCESS != pNewKernel->SetKernelArgInternal(i, src_arg)) {
        assert(false && "Unhandled type?");
        if (NULL != pErr)
          *pErr = CL_INVALID_VALUE;
        return CL_INVALID_HANDLE;
      }
  }

  pNewKernel->SetSvmFineGrainSystem(pSrcKernel->IsSvmFineGrainSystem());
  std::vector<SharedPtr<SVMBuffer>> svmBuffers;
  pSrcKernel->GetNonArgSvmBuffers(svmBuffers);
  pNewKernel->SetNonArgSvmBuffers(svmBuffers);

  // Unified shared memory
  pNewKernel->SetUsmIndirectHost(pSrcKernel->IsUsmIndirectHost());
  pNewKernel->SetUsmIndirectDevice(pSrcKernel->IsUsmIndirectDevice());
  pNewKernel->SetUsmIndirectShared(pSrcKernel->IsUsmIndirectShared());
  std::vector<SharedPtr<USMBuffer>> usmBuffers;
  pSrcKernel->GetNonArgUsmBuffers(usmBuffers);
  pNewKernel->SetNonArgUsmBuffers(usmBuffers);

  if (NULL != pNewKernel.GetPtr()) {
    // add new kernel to the context module's kernels list
    m_mapKernels.AddObject(pNewKernel, false);
    if (NULL != pErr) {
      *pErr = CL_SUCCESS;
    }
    // return handle
    return pNewKernel->GetHandle();
  }
  assert(0);

  return CL_INVALID_HANDLE;
}

//////////////////////////////////////////////////////////////////////////
// ContextModule::CreateKernelsInProgram
//////////////////////////////////////////////////////////////////////////
cl_int ContextModule::CreateKernelsInProgram(cl_program clProgram,
                                             cl_uint uiNumKernels,
                                             cl_kernel *pclKernels,
                                             cl_uint *puiNumKernelsRet) {
  LOG_INFO(TEXT("CreateKernelsInProgram enter. clProgram=%p, uiNumKernels=%u, "
                "pclKernels=%p, puiNumKernelsRet=%p"),
           clProgram, uiNumKernels, pclKernels, puiNumKernelsRet);

  // get the program object
  SharedPtr<Program> pProgram =
      m_mapPrograms.GetOCLObject((_cl_program_int *)clProgram)
          .DynamicCast<Program>();
  if (NULL == pProgram.GetPtr()) {
    LOG_ERROR(TEXT("clProgram is invalid program"));
    return CL_INVALID_PROGRAM;
  }

  // create all kernels for the program
  cl_err_code clErrRet =
      pProgram->CreateAllKernels(uiNumKernels, pclKernels, puiNumKernelsRet);
  if (CL_FAILED(clErrRet)) {
    return CL_ERR_OUT(clErrRet);
  }
  // No point in creating user-invisible kernels
  if (nullptr == pclKernels) {
    return CL_SUCCESS;
  }

  // get kernels and add them to the context module's map list
  std::vector<SharedPtr<Kernel>> Kernels;
  pProgram->GetKernels(Kernels);
  for (const auto &Kern : Kernels) {
    m_mapKernels.AddObject(Kern, false);
  }
  return CL_SUCCESS;
}
//////////////////////////////////////////////////////////////////////////
// ContextModule::RetainKernel
//////////////////////////////////////////////////////////////////////////
cl_int ContextModule::RetainKernel(cl_kernel clKernel) {
  LOG_INFO(TEXT("Enter RetainKernel (clKernel=%p)"), clKernel);

  cl_err_code clErr = CL_SUCCESS;
  SharedPtr<Kernel> pKernel =
      m_mapKernels.GetOCLObject((_cl_kernel_int *)clKernel)
          .DynamicCast<Kernel>();
  if (NULL == pKernel.GetPtr()) {
    LOG_ERROR(TEXT("GetOCLObject(%p, %p) returned %s"), clKernel, &pKernel,
              ClErrTxt(clErr));
    return CL_INVALID_KERNEL;
  }

  clErr = pKernel->Retain();

  return CL_ERR_OUT(clErr);
}
//////////////////////////////////////////////////////////////////////////
// ContextModule::ReleaseKernel
//////////////////////////////////////////////////////////////////////////
cl_int ContextModule::ReleaseKernel(cl_kernel clKernel) {
  LOG_INFO(TEXT("Enter ReleaseKernel (clKernel=%p)"), clKernel);

  cl_err_code clErr = CL_SUCCESS;
  SharedPtr<Kernel> pKernel =
      m_mapKernels.GetOCLObject((_cl_kernel_int *)clKernel)
          .DynamicCast<Kernel>();
  if (NULL == pKernel.GetPtr()) {
    LOG_ERROR(TEXT("GetOCLObject(%p, %p) returned %s"), clKernel, &pKernel,
              ClErrTxt(clErr));
    return CL_INVALID_KERNEL;
  }

  SharedPtr<Program> pProgram = pKernel->GetProgram();
  if (NULL == pProgram.GetPtr()) {
    LOG_ERROR(TEXT("pKernel->GetProgram returned nullptr"));
    return CL_INVALID_KERNEL;
  }

  long newRef = pKernel->Release();
  cl_int err = CL_SUCCESS;
  if (newRef < 0) {
    LOG_ERROR(TEXT("pKernel->Release() returned %ld"), newRef);
    err = CL_INVALID_KERNEL;
  } else if (0 == newRef) {
    clErr = pProgram->RemoveKernel(clKernel);
    if (CL_FAILED(clErr)) {
      LOG_ERROR(TEXT("pProgram->RemoveKernel returned %ld"), clErr);
      return CL_ERR_OUT(clErr);
    }
    // remove kernel form kernels list and add it to the dirty kernels list
    clErr = m_mapKernels.RemoveObject((_cl_kernel_int *)clKernel);
    if (CL_FAILED(clErr)) {
      LOG_ERROR(TEXT("m_mapKernels.RemoveObject returned %ld"), clErr);
      return CL_ERR_OUT(clErr);
    }
  }
  return err;
}

//////////////////////////////////////////////////////////////////////////
// RemoveAllKernels
//////////////////////////////////////////////////////////////////////////
void ContextModule::RemoveAllKernels(bool preserve_user_handles) {
  m_mapKernels.DisableAdding();

  AllObjectsFilter<Kernel>::ObjectsList to_remove;
  GetAllObjects<Kernel>(m_mapKernels, to_remove);

  if (preserve_user_handles) {
    m_mapKernels.SetPreserveUserHandles();
  }

  for (AllObjectsFilter<Kernel>::ObjectsList::iterator it = to_remove.begin();
       it != to_remove.end(); ++it) {
    SharedPtr<Kernel> &pObj = *it;

    cl_kernel handle = pObj->GetHandle();
    pObj->GetProgram()->RemoveKernel(handle);
    m_mapKernels.RemoveObject((_cl_kernel_int *)handle);
  }

  to_remove.clear();
  m_mapKernels.ReleaseAllObjects(false);
}

//////////////////////////////////////////////////////////////////////////
// ContextModule::SetKernelArg
//////////////////////////////////////////////////////////////////////////
cl_int ContextModule::SetKernelArg(cl_kernel clKernel, cl_uint uiArgIndex,
                                   size_t szArgSize, const void *pArgValue) {
  LOG_DEBUG(TEXT("Enter SetKernelArg (clKernel=%p, uiArgIndex=%u, "
                 "szArgSize=%zu, pszArgValue=%p)"),
            clKernel, uiArgIndex, szArgSize, pArgValue);

  cl_err_code clErr = CL_SUCCESS;
  SharedPtr<Kernel> pKernel =
      m_mapKernels.GetOCLObject((_cl_kernel_int *)clKernel)
          .StaticCast<Kernel>();
  if (NULL == pKernel.GetPtr()) {
    LOG_ERROR(TEXT("GetOCLObject(%p, %p) returned NULL Object"), clKernel,
              &pKernel);
    return CL_INVALID_KERNEL;
  }
  clErr = pKernel->SetKernelArg(uiArgIndex, szArgSize, pArgValue);
  return CL_ERR_OUT(clErr);
}

//////////////////////////////////////////////////////////////////////////
// ContextModule::SetKernelArgSVMPointer
//////////////////////////////////////////////////////////////////////////

cl_int ContextModule::SetKernelArgSVMPointer(cl_kernel clKernel,
                                             cl_uint uiArgIndex,
                                             const void *pArgValue) {
  SharedPtr<Kernel> pKernel =
      m_mapKernels.GetOCLObject((_cl_kernel_int *)clKernel)
          .StaticCast<Kernel>();
  if (NULL == pKernel.GetPtr()) {
    LOG_ERROR(TEXT("GetOCLObject(%p, %p) returned NULL"), clKernel, &pKernel);
    return CL_INVALID_KERNEL;
  }

  cl_err_code err =
      pKernel->SetKernelArg(uiArgIndex, sizeof(pArgValue), pArgValue, true);
  return CL_ERR_OUT(err);
}

cl_int ContextModule::SetKernelExecInfo(cl_kernel clKernel,
                                        cl_kernel_exec_info paramName,
                                        size_t szParamValueSize,
                                        const void *pParamValue) {
  SharedPtr<Kernel> pKernel =
      m_mapKernels.GetOCLObject((_cl_kernel_int *)clKernel)
          .StaticCast<Kernel>();
  if (NULL == pKernel.GetPtr()) {
    LOG_ERROR(TEXT("GetOCLObject(%p, %p) returned NULL"), clKernel, &pKernel);
    return CL_INVALID_KERNEL;
  }
  if (NULL == pParamValue) {
    return CL_INVALID_VALUE;
  }
  return pKernel->GetContext()->SetKernelExecInfo(
      pKernel, paramName, szParamValueSize, pParamValue);
}

cl_mem ContextModule::CreatePipe(cl_context context, cl_mem_flags flags,
                                 cl_uint uiPipePacketSize,
                                 cl_uint uiPipeMaxPackets,
                                 const cl_pipe_properties *pProperties,
                                 void *pHostPtr, size_t *pSizeRet,
                                 cl_int *piErrcodeRet) {
  cl_err_code err;
  SharedPtr<Context> pContext = GetContext(context);
  if (NULL == pContext.GetPtr()) {
    if (nullptr != piErrcodeRet) {
      *piErrcodeRet = CL_INVALID_CONTEXT;
    }
    return CL_INVALID_HANDLE;
  }
  err = CheckMemObjectParameters(flags, nullptr, CL_MEM_OBJECT_PIPE, 0, 0, 0, 0,
                                 0, 0, nullptr, pContext);
  if (CL_FAILED(err) || NULL != pProperties) {
    if (nullptr != piErrcodeRet) {
      *piErrcodeRet = CL_INVALID_VALUE;
    }
    return CL_INVALID_HANDLE;
  }

  cl_uint uiMinPipeMaxPacketSize = 0;
  const tSetOfDevices *pDevs = pContext->GetAllRootDevices();
  for (tSetOfDevices::const_iterator iter = pDevs->begin();
       iter != pDevs->end(); iter++) {
    cl_uint uiPipeMaxPacketSize;
    const cl_err_code err = (*iter)->GetInfo(CL_DEVICE_PIPE_MAX_PACKET_SIZE,
                                             sizeof(uiPipeMaxPacketSize),
                                             &uiPipeMaxPacketSize, nullptr);
    if (CL_FAILED(err)) {
      if (nullptr != piErrcodeRet) {
        *piErrcodeRet = err;
      }
      return CL_INVALID_HANDLE;
    }
    if (0 == uiMinPipeMaxPacketSize ||
        uiPipeMaxPacketSize < uiMinPipeMaxPacketSize) {
      uiMinPipeMaxPacketSize = uiPipeMaxPacketSize;
    }
  }
  if (0 == uiPipePacketSize || 0 == uiPipeMaxPackets ||
      uiPipePacketSize > uiMinPipeMaxPacketSize) {
    if (nullptr != piErrcodeRet) {
      *piErrcodeRet = CL_INVALID_PIPE_SIZE;
    }
    return CL_INVALID_HANDLE;
  }

  // handling INTEL extension for CRT
  if (NULL != pSizeRet) {
    if (NULL == pHostPtr) {
      *pSizeRet = Pipe::CalcPipeSize(uiPipePacketSize, uiPipeMaxPackets);
      if (nullptr != piErrcodeRet) {
        *piErrcodeRet = CL_SUCCESS;
      }
      return CL_INVALID_HANDLE;
    } else {
      if (*pSizeRet != Pipe::CalcPipeSize(uiPipePacketSize, uiPipeMaxPackets)) {
        if (nullptr != piErrcodeRet) {
          *piErrcodeRet = CL_OUT_OF_RESOURCES;
        }
        return CL_INVALID_HANDLE;
      }
    }
  } else {
    ASSERT_RET_VAL(NULL == pHostPtr, "this combination isn't expected from CRT",
                   CL_INVALID_HANDLE);
  }

  SharedPtr<MemoryObject> pPipe;
  err = pContext->CreatePipe(flags, uiPipePacketSize, uiPipeMaxPackets, pPipe,
                             pHostPtr);
  if (CL_FAILED(err)) {
    if (nullptr != piErrcodeRet) {
      *piErrcodeRet = err;
    }
    return CL_INVALID_HANDLE;
  }
  err = m_mapMemObjects.AddObject(pPipe, false);
  if (CL_FAILED(err)) {
    if (nullptr != piErrcodeRet) {
      *piErrcodeRet = err;
    }
    return CL_INVALID_HANDLE;
  }
  if (nullptr != piErrcodeRet) {
    *piErrcodeRet = CL_SUCCESS;
  }
  return pPipe->GetHandle();
}

cl_int ContextModule::GetPipeInfo(cl_mem pipe, cl_pipe_info paramName,
                                  size_t szParamValueSize, void *pParamValue,
                                  size_t *pszParamValueSizeRet) {
  SharedPtr<Pipe> pPipe =
      m_mapMemObjects.GetOCLObject((_cl_mem_int *)pipe).StaticCast<Pipe>();
  if (NULL == pPipe.GetPtr()) {
    return CL_INVALID_MEM_OBJECT;
  }
  return pPipe->GetPipeInfo(paramName, szParamValueSize, pParamValue,
                            pszParamValueSizeRet);
}

//////////////////////////////////////////////////////////////////////////
// ContextModule::GetKernelSubGroupInfo
//////////////////////////////////////////////////////////////////////////
cl_int ContextModule::GetKernelSubGroupInfo(
    cl_kernel kernel, cl_device_id device, cl_kernel_sub_group_info param_name,
    size_t input_value_size, const void *input_value, size_t param_value_size,
    void *param_value, size_t *param_value_size_ret) {
  LOG_INFO(
      TEXT(
          "Enter GetKernelSubGroupInfo (kernel=%p, device=%p, param_name=%u, input_value_size=%zu,\
                                                input_value=%p, param_value_size=%zu, param_value=%p,\
                                                param_value_size_ret=%p)"),
      kernel, device, param_name, input_value_size, input_value,
      param_value_size, param_value, param_value_size_ret);

  SharedPtr<Kernel> pKernel =
      m_mapKernels.GetOCLObject((_cl_kernel_int *)kernel).DynamicCast<Kernel>();
  if (NULL == pKernel.GetPtr()) {
    LOG_ERROR(TEXT("GetOCLObject(%p) returned NULL"), kernel);
    return CL_INVALID_KERNEL;
  }
  SharedPtr<FissionableDevice> pDevice;
  if (nullptr != device) {
    pDevice = m_pPlatformModule->GetDevice(device);
    if (NULL == pDevice.GetPtr()) {
      LOG_ERROR(TEXT("GetDevice(%p) returned NULL"), device);
      return CL_INVALID_DEVICE;
    }
  }
  return pKernel->GetSubGroupInfo(pDevice, param_name, param_value_size,
                                  input_value_size, input_value, param_value,
                                  param_value_size_ret);
}

//////////////////////////////////////////////////////////////////////////
// ContextModule::GetKernelInfo
//////////////////////////////////////////////////////////////////////////
cl_int ContextModule::GetKernelInfo(cl_kernel clKernel,
                                    cl_kernel_info clParamName,
                                    size_t szParamValueSize, void *pParamValue,
                                    size_t *pszParamValueSizeRet) {
  LOG_INFO(
      TEXT("Enter GetKernelInfo (clKernel=%p, clParamName=%u, "
           "szParamValueSize=%zu, pParamValue=%p, pszParamValueSizeRet=%p)"),
      clKernel, clParamName, szParamValueSize, pParamValue,
      pszParamValueSizeRet);

  cl_err_code clErr = CL_SUCCESS;
  (void)clErr;
  SharedPtr<Kernel> pKernel =
      m_mapKernels.GetOCLObject((_cl_kernel_int *)clKernel)
          .DynamicCast<Kernel>();
  if (NULL == pKernel.GetPtr()) {
    LOG_ERROR(TEXT("GetOCLObject(%p, %p) returned %s"), clKernel, &pKernel,
              ClErrTxt(clErr));
    return CL_INVALID_KERNEL;
  }

  return pKernel->GetInfo(clParamName, szParamValueSize, pParamValue,
                          pszParamValueSizeRet);
}

//////////////////////////////////////////////////////////////////////////
// ContextModule::GetKernelWorkGroupInfo
//////////////////////////////////////////////////////////////////////////
cl_int ContextModule::GetKernelWorkGroupInfo(
    cl_kernel clKernel, cl_device_id deviceId,
    cl_kernel_work_group_info clParamName, size_t szParamValueSize,
    void *pParamValue, size_t *pszParamValueSizeRet) {
  LOG_INFO(TEXT("Enter GetKernelWorkGroupInfo (clKernel=%p, clDevice=%p, "
                "clParamName=%u, szParamValueSize=%zu, pParamValue=%p, "
                "pszParamValueSizeRet=%p)"),
           clKernel, deviceId, clParamName, szParamValueSize, pParamValue,
           pszParamValueSizeRet);
  SharedPtr<Kernel> pKernel =
      m_mapKernels.GetOCLObject((_cl_kernel_int *)clKernel)
          .DynamicCast<Kernel>();
  if (NULL == pKernel.GetPtr()) {
    LOG_ERROR(TEXT("GetOCLObject(%p) returned NULL"), clKernel);
    return CL_INVALID_KERNEL;
  }
  SharedPtr<FissionableDevice> pDevice;
  if (nullptr != deviceId) // When deviceId is NULL, we should pass this
                           // paramter to kernel. In case of single device, it's
                           // data should be returned
  {
    pDevice = m_pPlatformModule->GetDevice(deviceId);
    if (NULL == pDevice.GetPtr()) {
      LOG_ERROR(TEXT("GetDevice(%p) returned NULL"), deviceId);
      return CL_INVALID_DEVICE;
    }
  }
  return pKernel->GetWorkGroupInfo(pDevice, clParamName, szParamValueSize,
                                   pParamValue, pszParamValueSizeRet);
}

//////////////////////////////////////////////////////////////////////////
// ContextModule::CreateBuffer
//////////////////////////////////////////////////////////////////////////
cl_mem ContextModule::CreateBuffer(cl_context clContext, cl_mem_flags clFlags,
                                   size_t szSize, void *pHostPtr,
                                   cl_int *pErrcodeRet) {
  LOG_DEBUG(TEXT("Enter CreateBuffer (clContext=%p, clFlags=%llu, szSize=%zu, "
                 "pHostPtr=%p, pErrcodeRet=%p)"),
            clContext, (unsigned long long)clFlags, szSize, pHostPtr,
            pErrcodeRet);

  cl_mem bufHandle =
      CreateBufferImpl(clContext, clFlags, szSize, pHostPtr, pErrcodeRet);

  if (bufHandle != CL_INVALID_HANDLE) {
    LOG_DEBUG(TEXT("CreateBuffer return handle %p"), bufHandle);
  }
  return bufHandle;
}

//////////////////////////////////////////////////////////////////////////
// ContextModule::CreateBufferWithPropertiesINTEL
//////////////////////////////////////////////////////////////////////////
cl_mem ContextModule::CreateBufferWithPropertiesINTEL(
    cl_context clContext, const cl_mem_properties_intel *properties,
    cl_mem_flags clFlags, size_t szSize, void *pHostPtr, cl_int *pErrcodeRet) {
  LOG_DEBUG(TEXT("Enter CreateBufferWithPropertiesINTEL(clContext=%p, "
                 "properties=%p, clFlags=%llu,"
                 " szSize=%zu, pHostPtr=%p, pErrcodeRet=%p)"),
            clContext, properties, (unsigned long long)clFlags, szSize,
            pHostPtr, pErrcodeRet);

  // check the properties.
  // Currently only CL_MEM_CHANNEL_INTEL is supported.
  if (properties) {
    for (int i = 0; properties[i] != 0; i += 2) {
      switch (properties[i]) {
      case CL_MEM_CHANNEL_INTEL:
        continue;
      default: {
        if (NULL != pErrcodeRet) {
          *pErrcodeRet = CL_INVALID_PROPERTY;
        }
        return CL_INVALID_HANDLE;
      }
      }
    }
  }
  cl_mem bufHandle =
      CreateBufferImpl(clContext, clFlags, szSize, pHostPtr, pErrcodeRet);

  if (bufHandle != CL_INVALID_HANDLE) {
    LOG_DEBUG(TEXT("CreateBufferWithPropertiesINTEL return handle %p"),
              bufHandle);
  }
  return bufHandle;
}

cl_mem ContextModule::CreateBufferImpl(cl_context clContext,
                                       cl_mem_flags clFlags, size_t szSize,
                                       void *pHostPtr, cl_int *pErrcodeRet) {
  SharedPtr<Context> pContext =
      m_mapContexts.GetOCLObject((_cl_context_int *)clContext)
          .DynamicCast<Context>();
  if (NULL == pContext.GetPtr()) {
    LOG_ERROR(TEXT("m_pContexts->GetOCLObject(%p) = NULL , pContext = %p"),
              clContext, pContext.GetPtr())
    if (NULL != pErrcodeRet) {
      *pErrcodeRet = CL_INVALID_CONTEXT;
    }
    return CL_INVALID_HANDLE;
  }

  cl_err_code clErr =
      CheckMemObjectParameters(clFlags, nullptr, CL_MEM_OBJECT_BUFFER, 0, 0, 0,
                               0, 0, 0, pHostPtr, pContext);
  if (!((CL_INVALID_IMAGE_FORMAT_DESCRIPTOR == clErr) ||
        (CL_SUCCESS == clErr))) {
    if (NULL != pErrcodeRet) {
      *pErrcodeRet = clErr;
    }
    return CL_INVALID_HANDLE;
  }

  SharedPtr<MemoryObject> pBuffer;
  SharedPtr<SVMBuffer> pSvmBuf = pContext->GetSVMBufferContainingAddr(
      pHostPtr); // we assume that the cl_mem and SVM buffer share the same
                 // context
  if (pSvmBuf.GetPtr() != NULL &&
      (clFlags & (CL_MEM_USE_HOST_PTR | CL_MEM_COPY_HOST_PTR))) {
    if (!pSvmBuf->IsContainedInBuffer(pHostPtr, szSize)) {
      if (NULL != pErrcodeRet) {
        *pErrcodeRet = CL_INVALID_BUFFER_SIZE; // this error code isn't
                                               // specified in the spec
      }
      return CL_INVALID_HANDLE;
    }
  }

  if (pSvmBuf.GetPtr() != NULL && (clFlags & CL_MEM_USE_HOST_PTR)) {
    cl_buffer_region bufRegion;
    bufRegion.origin = (char *)pHostPtr - (char *)pSvmBuf->GetAddr();
    bufRegion.size = szSize;
    clErr = pContext->CreateSubBuffer(
        pSvmBuf, clFlags, CL_BUFFER_CREATE_TYPE_REGION, &bufRegion, &pBuffer,
        /*RequireAlign*/ false);
    if (CL_SUCCEEDED(clErr)) {
      pBuffer->UpdateHostPtr(pBuffer->GetFlags(), pHostPtr);
    }
  } else {
    clErr = pContext->CreateBuffer(clFlags, szSize, pHostPtr, &pBuffer);
  }

  if (CL_FAILED(clErr)) {
    LOG_ERROR(TEXT("pContext->CreateBuffer(%llu, %zu, %p, %p) = %s"),
              (unsigned long long)clFlags, szSize, pHostPtr, &pBuffer,
              ClErrTxt(clErr))
    if (NULL != pErrcodeRet) {
      *pErrcodeRet = CL_ERR_OUT(clErr);
    }
    return CL_INVALID_HANDLE;
  }
  clErr = m_mapMemObjects.AddObject(pBuffer, false);
  if (CL_FAILED(clErr)) {
    LOG_ERROR(TEXT("m_mapMemObjects.AddObject(%p, %p, false) = %s"),
              pBuffer.GetPtr(), pBuffer->GetHandle(), ClErrTxt(clErr))
    if (nullptr != pErrcodeRet) {
      *pErrcodeRet = CL_ERR_OUT(clErr);
    }
    return CL_INVALID_HANDLE;
  }
  if (nullptr != pErrcodeRet) {
    *pErrcodeRet = CL_SUCCESS;
  }

  return pBuffer->GetHandle();
}

//////////////////////////////////////////////////////////////////////////
// ContextModule::CreateBufferWithProperties
//////////////////////////////////////////////////////////////////////////
cl_mem ContextModule::CreateBufferWithProperties(
    cl_context clContext, const cl_mem_properties *properties,
    cl_mem_flags clFlags, size_t szSize, void *pHostPtr, cl_int *pErrcodeRet) {
  LOG_DEBUG(TEXT("Enter CreateBufferWithProperties(clContext=%p, "
                 "properties=%p, clFlags=%llu,"
                 " szSize=%zu, pHostPtr=%p, pErrcodeRet=%p)"),
            clContext, properties, (unsigned long long)clFlags, szSize,
            pHostPtr, pErrcodeRet);
  cl_int iErrCode = CL_SUCCESS;
  std::vector<cl_mem_properties> bufferPropsArray;

  while (nullptr != properties && 0 != *properties && CL_SUCCEEDED(iErrCode)) {
    const cl_mem_properties name = *(properties++);
    if (std::find(bufferPropsArray.begin(), bufferPropsArray.end(), name) !=
        bufferPropsArray.end()) {
      iErrCode = CL_INVALID_VALUE;
      break;
    }
    const cl_mem_properties value = *(properties++);
    bufferPropsArray.push_back(name);
    bufferPropsArray.push_back(value);

    // OpenCL 3.0 does not define any optional properties for buffers.
  }

  // Add a terminator
  if (nullptr != properties)
    bufferPropsArray.push_back(0);

  if (CL_SUCCEEDED(iErrCode)) {
    cl_mem bufHandle =
        CreateBufferImpl(clContext, clFlags, szSize, pHostPtr, pErrcodeRet);

    if (bufHandle != CL_INVALID_HANDLE) {
      LOG_DEBUG(TEXT("CreateBufferWithProperties return handle %p"), bufHandle);
      SharedPtr<MemoryObject> pBuffer =
          m_mapMemObjects.GetOCLObject((_cl_mem_int *)bufHandle)
              .DynamicCast<MemoryObject>();
      if (pBuffer)
        pBuffer->SetProperties(bufferPropsArray);
    }
    return bufHandle;
  } else {
    if (nullptr != pErrcodeRet)
      *pErrcodeRet = iErrCode;
    return CL_INVALID_HANDLE;
  }
}

//////////////////////////////////////////////////////////////////////////
// ContextModule::CreateSubBuffer
//////////////////////////////////////////////////////////////////////////
cl_mem ContextModule::CreateSubBuffer(cl_mem clBuffer, cl_mem_flags clFlags,
                                      cl_buffer_create_type buffer_create_type,
                                      const void *buffer_create_info,
                                      cl_int *pErrcodeRet) {
  LOG_INFO(TEXT("Enter CreateSubBuffer (clBuffer=%p, clFlags=%llu, "
                "cl_buffer_create_type=%u, pErrcodeRet=%p)"),
           clBuffer, (unsigned long long)clFlags, buffer_create_type,
           pErrcodeRet);

  cl_int iNullErr;
  cl_int &iErr = pErrcodeRet ? *pErrcodeRet : iNullErr;

  if (!clBuffer) {
    iErr = CL_INVALID_MEM_OBJECT;
    return CL_INVALID_HANDLE;
  }

  SharedPtr<MemoryObject> pMemObj =
      m_mapMemObjects.GetOCLObject((_cl_mem_int *)clBuffer)
          .DynamicCast<MemoryObject>();
  if (NULL == pMemObj.GetPtr()) {
    LOG_ERROR(TEXT("GetOCLObject(%p, %p) returned NULL"), clBuffer, &pMemObj);
    iErr = CL_INVALID_MEM_OBJECT;
    return CL_INVALID_HANDLE;
  }

  SharedPtr<Context> pContext = pMemObj->GetContext();

  iErr = CheckMemObjectParameters(clFlags, nullptr, CL_MEM_OBJECT_BUFFER, 0, 0,
                                  0, 0, 0, 0, nullptr, pContext);
  if (CL_FAILED(iErr)) {
    return CL_INVALID_HANDLE;
  }

  // check memory object is a Buffer not Image2D/3D
  if (pMemObj->GetType() != CL_MEM_OBJECT_BUFFER) {
    iErr = CL_INVALID_MEM_OBJECT;
    return CL_INVALID_HANDLE;
  }

  if (NULL != pMemObj->GetParent().GetPtr()) {
    if (pMemObj->GetParent().DynamicCast<SVMBuffer>().GetPtr() == NULL) {
      iErr = CL_INVALID_MEM_OBJECT;
      return CL_INVALID_HANDLE;
    }
    /* When creating a cl_mem buffer from an SVM buffer, we return a
       sub-buffer of SVMBuffer.m_memObj. However, if the user creates a
       sub-buffer of this cl_mem buffer, we can't create a sub-buffer of a
       sub-buffer. So the solution is to create a sub-buffer of the SVMBuffer
       itself. */
    pMemObj = pMemObj->GetParent();
  }

  SharedPtr<MemoryObject> pBuffer = nullptr;
  cl_err_code clErr = pContext->CreateSubBuffer(
      pMemObj, clFlags, buffer_create_type, buffer_create_info, &pBuffer,
      /*RequireAlign*/ true);
  if (CL_FAILED(clErr)) {
    iErr = CL_ERR_OUT(clErr);
    return CL_INVALID_HANDLE;
  }

  clErr = m_mapMemObjects.AddObject(pBuffer, false);
  if (CL_FAILED(clErr)) {
    LOG_ERROR(TEXT("m_mapMemObjects.AddObject(%p, %p, false) = %s"),
              pBuffer.GetPtr(), pBuffer->GetHandle(), ClErrTxt(clErr))
    if (nullptr != pErrcodeRet) {
      iErr = CL_ERR_OUT(clErr);
    }
    return CL_INVALID_HANDLE;
  }
  if (nullptr != pErrcodeRet) {
    iErr = CL_SUCCESS;
  }

  LOG_DEBUG(TEXT("CreateSubBuffer returned handle = %p"), pBuffer->GetHandle());

  return pBuffer->GetHandle();
}

//////////////////////////////////////////////////////////////////////////
// ContextModule::CreateImage2D
//////////////////////////////////////////////////////////////////////////
cl_mem ContextModule::CreateImage2D(cl_context clContext, cl_mem_flags clFlags,
                                    const cl_image_format *clImageFormat,
                                    size_t szImageWidth, size_t szImageHeight,
                                    size_t szImageRowPitch, void *pHostPtr,
                                    cl_int *pErrcodeRet) {
  const cl_mem image = CreateScalarImage<2, CL_MEM_OBJECT_IMAGE2D>(
      clContext, clFlags, clImageFormat, szImageWidth, szImageHeight, 0,
      szImageRowPitch, 0, pHostPtr, pErrcodeRet);

  LOG_DEBUG(TEXT("CreateImage2D returned handle = %p"), image);

  return image;
}

//////////////////////////////////////////////////////////////////////////
// ContextModule::CreateImage3D
//////////////////////////////////////////////////////////////////////////
cl_mem ContextModule::CreateImage3D(cl_context clContext, cl_mem_flags clFlags,
                                    const cl_image_format *clImageFormat,
                                    size_t szImageWidth, size_t szImageHeight,
                                    size_t szImageDepth, size_t szImageRowPitch,
                                    size_t szImageSlicePitch, void *pHostPtr,
                                    cl_int *pErrcodeRet) {
  const cl_mem image = CreateScalarImage<3, CL_MEM_OBJECT_IMAGE3D>(
      clContext, clFlags, clImageFormat, szImageWidth, szImageHeight,
      szImageDepth, szImageRowPitch, szImageSlicePitch, pHostPtr, pErrcodeRet);

  LOG_DEBUG(TEXT("CreateImage3D returned handle = %p"), image);

  return image;
}

// Move main logic of CreateImage function to this helper function.
cl_mem ContextModule::CreateImageImpl(cl_context context, cl_mem_flags flags,
                                      const cl_image_format *image_format,
                                      const cl_image_desc *image_desc,
                                      void *host_ptr, cl_int *errcode_ret) {
  cl_mem clMemObj = CL_INVALID_HANDLE;

  SharedPtr<Context> pContext =
      m_mapContexts.GetOCLObject((_cl_context_int *)context)
          .DynamicCast<Context>();
  if (NULL == pContext.GetPtr()) {
    if (errcode_ret) {
      *errcode_ret = CL_INVALID_CONTEXT;
    }
    return CL_INVALID_HANDLE;
  }

  if (pContext->IsFPGAEmulator()) {
    if (errcode_ret) {
      *errcode_ret = CL_INVALID_OPERATION;
    }
    return CL_INVALID_HANDLE;
  }

  if (!image_desc || 0 != image_desc->num_mip_levels ||
      0 != image_desc->num_samples ||
      (CL_MEM_OBJECT_IMAGE1D_BUFFER != image_desc->image_type &&
       CL_MEM_OBJECT_IMAGE2D != image_desc->image_type &&
       nullptr != image_desc->mem_object)) {
    if (errcode_ret) {
      *errcode_ret = CL_INVALID_IMAGE_DESCRIPTOR;
    }
    return CL_INVALID_HANDLE;
  }
  switch (image_desc->image_type) {
  case CL_MEM_OBJECT_IMAGE1D:
    clMemObj = CreateScalarImage<1, CL_MEM_OBJECT_IMAGE1D>(
        context, flags, image_format, image_desc->image_width, 0, 0, 0, 0,
        host_ptr, errcode_ret);
    break;
  case CL_MEM_OBJECT_IMAGE1D_BUFFER:
    clMemObj = CreateImageBuffer<1, CL_MEM_OBJECT_IMAGE1D_BUFFER>(
        context, flags, image_format, *image_desc, image_desc->mem_object,
        errcode_ret);
    break;
  case CL_MEM_OBJECT_IMAGE2D:
    if (nullptr == image_desc->mem_object) {
      clMemObj = CreateScalarImage<2, CL_MEM_OBJECT_IMAGE2D>(
          context, flags, image_format, image_desc->image_width,
          image_desc->image_height, 0, image_desc->image_row_pitch, 0, host_ptr,
          errcode_ret);
    } else {
      cl_mem_object_type objType;
      if (CL_FAILED(GetMemObjectInfo(image_desc->mem_object, CL_MEM_TYPE,
                                     sizeof(objType), &objType, nullptr)) ||
          (objType != CL_MEM_OBJECT_BUFFER &&
           objType != CL_MEM_OBJECT_IMAGE2D)) {
        if (errcode_ret != nullptr) {
          *errcode_ret = CL_INVALID_IMAGE_DESCRIPTOR;
        }
        return CL_INVALID_HANDLE;
      }
      if (CL_MEM_OBJECT_BUFFER == objType) {
        clMemObj = CreateImageBuffer<2, CL_MEM_OBJECT_IMAGE2D>(
            context, flags, image_format, *image_desc, image_desc->mem_object,
            errcode_ret);
      } else {
        clMemObj =
            Create2DImageFromImage(context, flags, image_format, image_desc,
                                   image_desc->mem_object, errcode_ret);
      }
    }
    break;
  case CL_MEM_OBJECT_IMAGE3D:
    clMemObj = CreateScalarImage<3, CL_MEM_OBJECT_IMAGE3D>(
        context, flags, image_format, image_desc->image_width,
        image_desc->image_height, image_desc->image_depth,
        image_desc->image_row_pitch, image_desc->image_slice_pitch, host_ptr,
        errcode_ret);
    break;
  case CL_MEM_OBJECT_IMAGE1D_ARRAY:
  case CL_MEM_OBJECT_IMAGE2D_ARRAY:
    clMemObj = CreateImageArray(context, flags, image_format, image_desc,
                                host_ptr, errcode_ret);
    break;
  default:
    LOG_ERROR(TEXT("unsupported image type (%u)"), image_desc->image_type);
    if (errcode_ret) {
      *errcode_ret = CL_INVALID_IMAGE_DESCRIPTOR;
    }
  }
  return clMemObj;
}

/************************************************************************/
/* ContextModule::CreateImage                                              */
/************************************************************************/
cl_mem ContextModule::CreateImage(cl_context context, cl_mem_flags flags,
                                  const cl_image_format *image_format,
                                  const cl_image_desc *image_desc,
                                  void *host_ptr, cl_int *errcode_ret) {
  LOG_DEBUG(TEXT("Enter CreateImage(context=%p, flags=%llu, image_format=%p, "
                 "image_desc=%p, host_ptr=%p, errcode_ret=%p"),
            context, (unsigned long long)flags, image_format, image_desc,
            host_ptr, errcode_ret);

  cl_mem clMemObj = CreateImageImpl(context, flags, image_format, image_desc,
                                    host_ptr, errcode_ret);

  LOG_DEBUG(TEXT("CreateImage returned handle = %p"), clMemObj);

  return clMemObj;
}

/************************************************************************/
/* ContextModule::CreateImageWithProperties                                */
/************************************************************************/
cl_mem ContextModule::CreateImageWithProperties(
    cl_context context, const cl_mem_properties *properties, cl_mem_flags flags,
    const cl_image_format *image_format, const cl_image_desc *image_desc,
    void *host_ptr, cl_int *errcode_ret) {
  LOG_DEBUG(TEXT("Enter CreateImageWithProperties(context=%p, properties=%p, "
                 "flags=%llu, image_format=%p, image_desc=%p, host_ptr=%p, "
                 "errcode_ret=%p"),
            context, properties, (unsigned long long)flags, image_format,
            image_desc, host_ptr, errcode_ret);
  cl_int iErrCode = CL_SUCCESS;
  std::vector<cl_mem_properties> imagePropsArray;

  while (nullptr != properties && 0 != *properties && CL_SUCCEEDED(iErrCode)) {
    const cl_mem_properties name = *(properties++);
    if (std::find(imagePropsArray.begin(), imagePropsArray.end(), name) !=
        imagePropsArray.end()) {
      iErrCode = CL_INVALID_VALUE;
      break;
    }
    const cl_mem_properties value = *(properties++);
    imagePropsArray.push_back(name);
    imagePropsArray.push_back(value);

    // OpenCL 3.0 does not define any optional properties for image.
  }

  if (nullptr != properties)
    imagePropsArray.push_back(0);

  if (CL_SUCCEEDED(iErrCode)) {
    cl_mem clMemObj = CreateImageImpl(context, flags, image_format, image_desc,
                                      host_ptr, errcode_ret);
    if (clMemObj != CL_INVALID_HANDLE) {
      LOG_DEBUG(TEXT("CreateImageWithProperties returned handle = %p"),
                clMemObj);
      SharedPtr<MemoryObject> pImage =
          m_mapMemObjects.GetOCLObject((_cl_mem_int *)clMemObj)
              .DynamicCast<MemoryObject>();
      if (pImage)
        pImage->SetProperties(imagePropsArray);
    }
    return clMemObj;
  } else {
    if (nullptr != errcode_ret)
      *errcode_ret = iErrCode;
    return CL_INVALID_HANDLE;
  }
}

bool ContextModule::Check2DImageFromBufferPitch(
    const ConstSharedPtr<GenericMemObject> &pBuffer, const cl_image_desc &desc,
    const cl_image_format &format) const {
  const tSetOfDevices &devices = *pBuffer->GetContext()->GetAllRootDevices();

  cl_uint uiMaxImgPitchAlign = 0;
  for (tSetOfDevices::const_iterator iter = devices.begin();
       iter != devices.end(); iter++) {
    cl_uint uiImgPitchAlign;
    (*iter)->GetInfo(CL_DEVICE_IMAGE_PITCH_ALIGNMENT, sizeof(uiImgPitchAlign),
                     &uiImgPitchAlign, nullptr);
    if (uiImgPitchAlign > uiMaxImgPitchAlign) {
      uiMaxImgPitchAlign = uiImgPitchAlign;
    }
  }
  if (0 == uiMaxImgPitchAlign) {
    return false;
  }

  const size_t szRowPitch =
      desc.image_row_pitch > 0
          ? desc.image_row_pitch
          : desc.image_width * clGetPixelBytesCount(&format);
  return szRowPitch % uiMaxImgPitchAlign == 0;
}

cl_mem ContextModule::CreateImageArray(cl_context clContext,
                                       cl_mem_flags clFlags,
                                       const cl_image_format *clImageFormat,
                                       const cl_image_desc *pClImageDesc,
                                       void *pHostPtr, cl_int *pErrcodeRet) {
  SharedPtr<Context> pContext =
      m_mapContexts.GetOCLObject((_cl_context_int *)clContext)
          .DynamicCast<Context>();
  if (NULL == pContext.GetPtr()) {
    LOG_ERROR(TEXT("m_pContexts->GetOCLObject(%p) = NULL"), clContext);
    if (nullptr != pErrcodeRet) {
      *pErrcodeRet = CL_INVALID_CONTEXT;
    }
    return CL_INVALID_HANDLE;
  }

  // Do some initial (not context specific) parameter checking
  // check input memory flags
  cl_err_code clErr = CheckMemObjectParameters(
      clFlags, clImageFormat, pClImageDesc->image_type,
      pClImageDesc->image_width, pClImageDesc->image_height, 0,
      pClImageDesc->image_row_pitch, pClImageDesc->image_slice_pitch,
      pClImageDesc->image_array_size, pHostPtr, pContext);
  if (CL_FAILED(clErr)) {
    LOG_ERROR(TEXT("Parameter check failed"));
    if (nullptr != pErrcodeRet) {
      *pErrcodeRet = clErr;
    }
    return CL_INVALID_HANDLE;
  }
  SharedPtr<MemoryObject> pImageArr;
  // Do some context specific checks
  if (CL_MEM_OBJECT_IMAGE1D_ARRAY == pClImageDesc->image_type) {
    clErr = CheckContextSpecificParameters(pContext, pClImageDesc->image_type,
                                           pClImageDesc->image_width, 0, 0,
                                           pClImageDesc->image_array_size);
  } else if (CL_MEM_OBJECT_IMAGE2D_ARRAY == pClImageDesc->image_type) {
    clErr = CheckContextSpecificParameters(
        pContext, pClImageDesc->image_type, pClImageDesc->image_width,
        pClImageDesc->image_height, 0, pClImageDesc->image_array_size);
  } else {
    assert(0 && "Inside CreateImageArray with non array type.");
  }
  if (CL_FAILED(clErr)) {
    LOG_ERROR(TEXT("Context specific parameter check failed"));
    if (nullptr != pErrcodeRet) {
      *pErrcodeRet = clErr;
    }
    return CL_INVALID_HANDLE;
  }

  // Do some context specific checks
  if (CL_MEM_OBJECT_IMAGE1D_ARRAY == pClImageDesc->image_type) {
    clErr = CheckContextSpecificParameters(pContext, pClImageDesc->image_type,
                                           pClImageDesc->image_width, 0, 0,
                                           pClImageDesc->image_array_size);
  } else if (CL_MEM_OBJECT_IMAGE2D_ARRAY == pClImageDesc->image_type) {
    clErr = CheckContextSpecificParameters(
        pContext, pClImageDesc->image_type, pClImageDesc->image_width,
        pClImageDesc->image_height, 0, pClImageDesc->image_array_size);
  } else {
    assert(0 && "Inside CreateImageArray with non array type.");
  }
  if (CL_FAILED(clErr)) {
    LOG_ERROR(TEXT("Context specific parameter check failed"));
    if (nullptr != pErrcodeRet) {
      *pErrcodeRet = clErr;
    }
    return CL_INVALID_HANDLE;
  }

  clErr = pContext->CreateImageArray(clFlags, clImageFormat, pHostPtr,
                                     pClImageDesc, &pImageArr);
  if (CL_FAILED(clErr) || NULL == pImageArr.GetPtr()) {
    LOG_ERROR(TEXT("pContext->CreateImage2DArray(%llu, %p, %p, %p, %p) = %s"),
              (unsigned long long)clFlags, clImageFormat, pHostPtr,
              pClImageDesc, &pImageArr, ClErrTxt(clErr));
    if (nullptr != pErrcodeRet) {
      *pErrcodeRet = CL_ERR_OUT(clErr);
    }
    return CL_INVALID_HANDLE;
  }
  clErr = m_mapMemObjects.AddObject(pImageArr, false);
  if (CL_FAILED(clErr)) {
    LOG_ERROR(TEXT("m_mapMemObjects.AddObject(%p, %p, false) = %s"),
              pImageArr.GetPtr(), pImageArr->GetHandle(), ClErrTxt(clErr))
    if (NULL != pErrcodeRet) {
      *pErrcodeRet = CL_ERR_OUT(clErr);
    }
    return CL_INVALID_HANDLE;
  }
  if (nullptr != pErrcodeRet) {
    *pErrcodeRet = CL_SUCCESS;
  }
  return pImageArr->GetHandle();
}

cl_mem ContextModule::Create2DImageFromImage(
    cl_context context, cl_mem_flags flags, const cl_image_format *pImageFormat,
    const cl_image_desc *pImageDesc, cl_mem otherImgHandle,
    cl_int *piErrcodeRet) {
  SharedPtr<Context> pContext =
      m_mapContexts.GetOCLObject((_cl_context_int *)context)
          .DynamicCast<Context>();
  assert((pContext.GetPtr() != NULL) &&
         "Wrong context passed to Create2DImageFromImage.");
  if (pContext.GetPtr() == NULL) {
    if (piErrcodeRet != nullptr) {
      *piErrcodeRet = CL_INVALID_CONTEXT;
    }
    return CL_INVALID_HANDLE;
  }
  SharedPtr<MemoryObject> pOtherImg = pContext->GetMemObject(otherImgHandle);
  assert(pOtherImg.GetPtr() != NULL &&
         "Wrong image descriptor passed to Create2DImageFromImage");
  if (pOtherImg.GetPtr() == NULL) {
    if (piErrcodeRet != nullptr) {
      *piErrcodeRet = CL_INVALID_IMAGE_DESCRIPTOR;
    }
    return CL_INVALID_HANDLE;
  }
  size_t szOtherWidth, szOtherHeight, szOtherRowPitch;
  cl_image_format otherImgFormat;

  pOtherImg->GetImageInfo(CL_IMAGE_WIDTH, sizeof(szOtherWidth), &szOtherWidth,
                          nullptr);
  pOtherImg->GetImageInfo(CL_IMAGE_HEIGHT, sizeof(szOtherHeight),
                          &szOtherHeight, nullptr);
  pOtherImg->GetImageInfo(CL_IMAGE_ROW_PITCH, sizeof(szOtherRowPitch),
                          &szOtherRowPitch, nullptr);
  pOtherImg->GetImageInfo(CL_IMAGE_FORMAT, sizeof(otherImgFormat),
                          &otherImgFormat, nullptr);

  if (pImageDesc->image_width != szOtherWidth ||
      pImageDesc->image_height != szOtherHeight ||
      (pImageDesc->image_row_pitch != 0 &&
       pImageDesc->image_row_pitch != szOtherRowPitch) ||
      (pImageDesc->image_row_pitch == 0 &&
       clGetPixelBytesCount(pImageFormat) * pImageDesc->image_width !=
           szOtherRowPitch) ||
      !((pImageFormat->image_channel_order == CL_sBGRA &&
         otherImgFormat.image_channel_order == CL_BGRA) ||
        (pImageFormat->image_channel_order == CL_BGRA &&
         otherImgFormat.image_channel_order == CL_sBGRA) ||
        (pImageFormat->image_channel_order == CL_sRGBA &&
         otherImgFormat.image_channel_order == CL_RGBA) ||
        (pImageFormat->image_channel_order == CL_RGBA &&
         otherImgFormat.image_channel_order == CL_sRGBA) ||
        (pImageFormat->image_channel_order == CL_sRGB &&
         otherImgFormat.image_channel_order == CL_RGB) ||
        (pImageFormat->image_channel_order == CL_RGB &&
         otherImgFormat.image_channel_order == CL_sRGB) ||
        (pImageFormat->image_channel_order == CL_sRGBx &&
         otherImgFormat.image_channel_order == CL_RGBx) ||
        (pImageFormat->image_channel_order == CL_RGBx &&
         otherImgFormat.image_channel_order == CL_sRGBx))) {
    if (piErrcodeRet != nullptr) {
      *piErrcodeRet = CL_INVALID_IMAGE_FORMAT_DESCRIPTOR;
    }
    return CL_INVALID_HANDLE;
  }
  void *const pData = pOtherImg->GetBackingStoreData();
  cl_mem clImg = CreateScalarImage<2, CL_MEM_OBJECT_IMAGE2D>(
      context, flags, pImageFormat, pImageDesc->image_width,
      pImageDesc->image_height, 1, pImageDesc->image_row_pitch, 0, pData,
      piErrcodeRet, true);
  SharedPtr<MemoryObject> pImg =
      m_mapMemObjects.GetOCLObject((_cl_mem_int *)clImg)
          .DynamicCast<MemoryObject>();
  pImg->SetParent(pOtherImg);
  return clImg;
}

//////////////////////////////////////////////////////////////////////////
// ContextModule::RetainMemObject
//////////////////////////////////////////////////////////////////////////
cl_int ContextModule::RetainMemObject(cl_mem clMemObj) {
  LOG_DEBUG(TEXT("Enter RetainMemObject (clMemObj=%p)"), clMemObj);

  cl_err_code clErr = CL_SUCCESS;
  (void)clErr;
  SharedPtr<MemoryObject> pMemObj =
      m_mapMemObjects.GetOCLObject((_cl_mem_int *)clMemObj)
          .DynamicCast<MemoryObject>();
  if (NULL == pMemObj.GetPtr()) {
    LOG_ERROR(TEXT("GetOCLObject(%p, %p) returned %s"), clMemObj, &pMemObj,
              ClErrTxt(clErr));
    return CL_INVALID_MEM_OBJECT;
  }
  return pMemObj->Retain();
}
//////////////////////////////////////////////////////////////////////////
// ContextModule::ReleaseMemObject
//////////////////////////////////////////////////////////////////////////
cl_int ContextModule::ReleaseMemObject(cl_mem clMemObj) {
  LOG_DEBUG(TEXT("Enter ReleaseMemObject (clMemObj=%p)"), clMemObj);

  cl_err_code clErr = CL_SUCCESS;
  SharedPtr<MemoryObject> pMemObj =
      m_mapMemObjects.GetOCLObject((_cl_mem_int *)clMemObj)
          .DynamicCast<MemoryObject>();
  if (NULL == pMemObj.GetPtr()) {
    LOG_ERROR(TEXT("GetOCLObject(%p, %p) returned %s"), clMemObj, &pMemObj,
              ClErrTxt(clErr));
    return CL_INVALID_MEM_OBJECT;
  }
  SharedPtr<Context> pContext = pMemObj->GetContext();
  if (NULL == pContext.GetPtr()) {
    return CL_INVALID_MEM_OBJECT;
  }

  long newRef = pMemObj->Release();

  cl_int res = CL_SUCCESS;
  if (newRef < 0) {
    return CL_INVALID_MEM_OBJECT;
  } else if (0 == newRef) {
    // TODO: handle release memory object
    clErr = pContext->RemoveMemObject(clMemObj);
    if (CL_FAILED(clErr)) {
      res = CL_ERR_OUT(clErr);
    }
    clErr = m_mapMemObjects.RemoveObject((_cl_mem_int *)clMemObj);
    if (CL_FAILED(clErr)) {
      res = CL_ERR_OUT(clErr);
    }
  }
  return res;
}

//////////////////////////////////////////////////////////////////////////
// RemoveAllMemObjects
//////////////////////////////////////////////////////////////////////////
typedef std::list<SharedPtr<MemoryObject>> MemObjListType;
void ContextModule::RemoveAllMemObjects(bool preserve_user_handles) {
  m_mapMemObjects.DisableAdding();

  AllObjectsFilter<MemoryObject>::ObjectsList to_remove;
  GetAllObjects<MemoryObject>(m_mapMemObjects, to_remove);

  if (preserve_user_handles) {
    m_mapMemObjects.SetPreserveUserHandles();
  }

  for (AllObjectsFilter<MemoryObject>::ObjectsList::iterator it =
           to_remove.begin();
       it != to_remove.end(); ++it) {
    SharedPtr<MemoryObject> &pObj = *it;

    cl_mem handle = pObj->GetHandle();
    pObj->GetContext()->RemoveMemObject(handle);
    m_mapMemObjects.RemoveObject((_cl_mem_int *)handle);
  }

  to_remove.clear();
  m_mapMemObjects.ReleaseAllObjects(false);

  for (std::map<void *, SharedPtr<Context>>::iterator it =
           m_mapSVMBuffers.begin();
       it != m_mapSVMBuffers.end(); ++it) {
    it->second->SVMFree(it->first);
  }
  m_mapSVMBuffers.clear();

  // Free unified shared memory
  for (std::pair<void *const, SharedPtr<Context>> &p : m_mapUSMBuffers)
    p.second->USMFree(p.first);
  m_mapUSMBuffers.clear();

  // Actually, all tracker events should be unregistered when related command
  // done as expected. But we push tracker event into wait list after command
  // has been enqueued, sometimes command has already completed by this time.
  // Then the tracker event will never be unregistered. So we need to manually
  // clear wait list here.
  m_mapUSMFreeWaitList.clear();

  // Remove all mapped regions
  MemObjListType mapped_list;
  m_setMappedMemObjects.getObjects(mapped_list);

  for (MemObjListType::iterator it = mapped_list.begin();
       it != mapped_list.end(); ++it) {
    SharedPtr<MemoryObject> obj = *it;
    obj->ReleaseAllMappedRegions();
  }
  mapped_list.clear();
}

//////////////////////////////////////////////////////////////////////////
// ContextModule::GetSupportedImageFormats
//////////////////////////////////////////////////////////////////////////
cl_int ContextModule::GetSupportedImageFormats(cl_context clContext,
                                               cl_mem_flags clFlags,
                                               cl_mem_object_type clImageType,
                                               cl_uint uiNumEntries,
                                               cl_image_format *pclImageFormats,
                                               cl_uint *puiNumImageFormats) {
  LOG_INFO(TEXT("Enter GetSupportedImageFormats (clContext=%p, clFlags=%llu, "
                "clImageType=%u, uiNumEntries=%u, pclImageFormats=%p, "
                "puiNumImageFormats=%p)"),
           clContext, (unsigned long long)clFlags, clImageType, uiNumEntries,
           pclImageFormats, puiNumImageFormats);

  SharedPtr<Context> pContext =
      m_mapContexts.GetOCLObject((_cl_context_int *)clContext)
          .DynamicCast<Context>();
  if (NULL == pContext.GetPtr()) {
    LOG_ERROR(TEXT("m_pContexts->GetOCLObject(%p) = NULL"), clContext);
    return CL_INVALID_CONTEXT;
  }
  return pContext->GetSupportedImageFormats(
      clFlags, clImageType, uiNumEntries, pclImageFormats, puiNumImageFormats);
}
//////////////////////////////////////////////////////////////////////////
// ContextModule::GetMemObjectInfo
//////////////////////////////////////////////////////////////////////////
cl_int ContextModule::GetMemObjectInfo(cl_mem clMemObj, cl_mem_info clParamName,
                                       size_t szParamValueSize,
                                       void *pParamValue,
                                       size_t *pszParamValueSizeRet) {
  LOG_INFO(
      TEXT("Enter GetMemObjectInfo (clMemObj=%p, clParamName=%u, "
           "szParamValueSize=%zu, pParamValue=%p, pszParamValueSizeRet=%p)"),
      clMemObj, clParamName, szParamValueSize, pParamValue,
      pszParamValueSizeRet);

  cl_err_code clErr = CL_SUCCESS;
  (void)clErr;
  SharedPtr<MemoryObject> pMemObj =
      m_mapMemObjects.GetOCLObject((_cl_mem_int *)clMemObj)
          .DynamicCast<MemoryObject>();
  if (NULL == pMemObj.GetPtr()) {
    LOG_ERROR(TEXT("GetOCLObject(%p, %p) returned %s"), clMemObj, &pMemObj,
              ClErrTxt(clErr));
    return CL_INVALID_MEM_OBJECT;
  }

  return pMemObj->GetInfo(clParamName, szParamValueSize, pParamValue,
                          pszParamValueSizeRet);
}
//////////////////////////////////////////////////////////////////////////
// ContextModule::GetImageInfo
//////////////////////////////////////////////////////////////////////////
cl_int ContextModule::GetImageInfo(cl_mem clImage, cl_image_info clParamName,
                                   size_t szParamValueSize, void *pParamValue,
                                   size_t *pszParamValueSizeRet) {
  LOG_INFO(
      TEXT("Enter GetImageInfo (clImage=%p, clParamName=%u, "
           "szParamValueSize=%zu, pParamValue=%p, pszParamValueSizeRet=%p)"),
      clImage, clParamName, szParamValueSize, pParamValue,
      pszParamValueSizeRet);

  cl_err_code clErr = CL_SUCCESS;
  (void)clErr;
  SharedPtr<MemoryObject> pMemObj =
      m_mapMemObjects.GetOCLObject((_cl_mem_int *)clImage)
          .DynamicCast<MemoryObject>();
  if (NULL == pMemObj.GetPtr()) {
    LOG_ERROR(TEXT("GetOCLObject(%p, %p) returned %s"), clImage, &pMemObj,
              ClErrTxt(clErr));
    return CL_INVALID_MEM_OBJECT;
  }

  // If memory object doesnt support this operation it retuns
  // CL_INVALID_MEM_OBJECT
  return pMemObj->GetImageInfo(clParamName, szParamValueSize, pParamValue,
                               pszParamValueSizeRet);
}

//////////////////////////////////////////////////////////////////////////
// ContextModule::CreateSampler
//////////////////////////////////////////////////////////////////////////
cl_int ContextModule::SetMemObjectDestructorCallback(cl_mem memObj,
                                                     mem_dtor_fn pfn_notify,
                                                     void *pUserData) {
  cl_err_code clErr = CL_SUCCESS;

  SharedPtr<MemoryObject> pMemObj =
      m_mapMemObjects.GetOCLObject((_cl_mem_int *)memObj)
          .DynamicCast<MemoryObject>();
  if (NULL == pMemObj.GetPtr()) {
    LOG_ERROR(TEXT("GetOCLObject(%p, %p) returned %s"), memObj, &pMemObj,
              ClErrTxt(clErr));
    return CL_INVALID_MEM_OBJECT;
  }

  // if pfn_notify is NULL. the following register function will return
  // CL_INVALID_VALUE
  clErr = pMemObj->registerDtorNotifierCallback(pfn_notify, pUserData);
  return clErr;
}

//////////////////////////////////////////////////////////////////////////
// ContextModule::CreateSamplerWithProperties
//////////////////////////////////////////////////////////////////////////
cl_sampler ContextModule::CreateSamplerWithProperties(
    cl_context clContext, const cl_sampler_properties *pSamplerProperties,
    cl_int *pErrcodeRet) {
  cl_bool bNormalizedCoords = CL_TRUE;
  cl_addressing_mode clAddressingMode = CL_ADDRESS_CLAMP;
  cl_filter_mode clFilterMode = CL_FILTER_NEAREST;
  std::set<cl_sampler_properties> specifiedNames;
  cl_int iErrCode = CL_SUCCESS;
  std::vector<cl_sampler_properties> samplerPropsArray;

  while (nullptr != pSamplerProperties && 0 != *pSamplerProperties &&
         CL_SUCCEEDED(iErrCode)) {
    const cl_sampler_properties name = *(pSamplerProperties++);
    if (specifiedNames.find(name) !=
        specifiedNames
            .end()) // the same property name cannot be specified more than once
    {
      iErrCode = CL_INVALID_VALUE;
      break;
    }
    specifiedNames.insert(name);
    const cl_sampler_properties value = *(pSamplerProperties++);
    samplerPropsArray.push_back(name);
    samplerPropsArray.push_back(value);

    switch (name) {
    case CL_SAMPLER_NORMALIZED_COORDS:
      if (CL_TRUE != value && CL_FALSE != value) {
        iErrCode = CL_INVALID_VALUE;
        break;
      }
      bNormalizedCoords = (cl_bool)value;
      break;
    case CL_SAMPLER_ADDRESSING_MODE:
      if (CL_ADDRESS_MIRRORED_REPEAT != value && CL_ADDRESS_REPEAT != value &&
          CL_ADDRESS_CLAMP_TO_EDGE != value && CL_ADDRESS_CLAMP != value &&
          CL_ADDRESS_NONE != value) {
        iErrCode = CL_INVALID_VALUE;
        break;
      }
      clAddressingMode = (cl_addressing_mode)value;
      break;
    case CL_SAMPLER_FILTER_MODE:
      if (CL_FILTER_NEAREST != value && CL_FILTER_LINEAR != value) {
        iErrCode = CL_INVALID_VALUE;
        break;
      }
      clFilterMode = (cl_filter_mode)value;
      break;
    default:
      iErrCode = CL_INVALID_VALUE;
    }
  }

  // Add a terminator
  if (nullptr != pSamplerProperties)
    samplerPropsArray.push_back(0);

  if (CL_SUCCEEDED(iErrCode)) {
    cl_sampler sampler =
        CreateSampler(clContext, bNormalizedCoords, clAddressingMode,
                      clFilterMode, pErrcodeRet);
    SharedPtr<Sampler> pSampler =
        m_mapSamplers.GetOCLObject((_cl_sampler_int *)sampler)
            .DynamicCast<Sampler>();
    // According to the specs of OCL3.0, the implementation of
    // CL_SAMPLER_PROPERTIES must return the values specified in the
    // properties argument in the same order, so we need to save a
    // copy the original properties
    if (pSampler)
      pSampler->SetProperties(samplerPropsArray);
    return sampler;
  } else {
    if (nullptr != pErrcodeRet) {
      *pErrcodeRet = iErrCode;
    }
    return CL_INVALID_HANDLE;
  }
}

//////////////////////////////////////////////////////////////////////////
// ContextModule::CreateSampler
//////////////////////////////////////////////////////////////////////////
cl_sampler ContextModule::CreateSampler(cl_context clContext,
                                        cl_bool bNormalizedCoords,
                                        cl_addressing_mode clAddressingMode,
                                        cl_filter_mode clFilterMode,
                                        cl_int *pErrcodeRet) {
  LOG_DEBUG(TEXT("Enter CreateSampler (clContext=%p, bNormalizedCoords=%d, "
                 "clAddressingMode=%u, clFilterMode=%u, pErrcodeRet=%p)"),
            clContext, bNormalizedCoords, clAddressingMode, clFilterMode,
            pErrcodeRet);

  SharedPtr<Context> pContext =
      m_mapContexts.GetOCLObject((_cl_context_int *)clContext)
          .DynamicCast<Context>();
  if (NULL == pContext.GetPtr()) {
    LOG_ERROR(TEXT("m_pContexts->GetOCLObject(%p) = NULL"), clContext);
    if (nullptr != pErrcodeRet) {
      *pErrcodeRet = CL_INVALID_CONTEXT;
    }
    return CL_INVALID_HANDLE;
  }

  if (pContext->IsFPGAEmulator()) {
    if (nullptr != pErrcodeRet) {
      *pErrcodeRet = CL_INVALID_OPERATION;
    }
    return CL_INVALID_HANDLE;
  }

  SharedPtr<Sampler> pSampler;
  cl_err_code clErr = pContext->CreateSampler(
      bNormalizedCoords, clAddressingMode, clFilterMode, &pSampler);
  if (CL_FAILED(clErr)) {
    LOG_ERROR(TEXT("pContext->CreateSampler(%d, %u, %u, %p) = %s"),
              bNormalizedCoords, clAddressingMode, clFilterMode, &pSampler,
              ClErrTxt(clErr))
    if (nullptr != pErrcodeRet) {
      *pErrcodeRet = CL_ERR_OUT(clErr);
    }
    return CL_INVALID_HANDLE;
  }
  clErr = m_mapSamplers.AddObject(pSampler, false);
  if (CL_FAILED(clErr)) {
    LOG_ERROR(TEXT("m_mapMemObjects.AddObject(%p, %p, false) = %s"),
              pSampler.GetPtr(), pSampler->GetHandle(), ClErrTxt(clErr))
    if (nullptr != pErrcodeRet) {
      *pErrcodeRet = CL_ERR_OUT(clErr);
    }
    return CL_INVALID_HANDLE;
  }
  if (nullptr != pErrcodeRet) {
    *pErrcodeRet = CL_SUCCESS;
  }
  return pSampler->GetHandle();
}

//////////////////////////////////////////////////////////////////////////
// ContextModule::RetainSampler
//////////////////////////////////////////////////////////////////////////
cl_int ContextModule::RetainSampler(cl_sampler clSampler) {
  LOG_DEBUG(TEXT("Enter RetainSampler (clSampler=%p)"), clSampler);

  cl_err_code clErr = CL_SUCCESS;
  SharedPtr<Sampler> pSampler =
      m_mapSamplers.GetOCLObject((_cl_sampler_int *)clSampler)
          .DynamicCast<Sampler>();
  if (CL_FAILED(clErr) || NULL == pSampler.GetPtr()) {
    LOG_ERROR(TEXT("GetOCLObject(%p, %p) returned %s"), clSampler, &pSampler,
              ClErrTxt(clErr));
    return CL_INVALID_SAMPLER;
  }
  return pSampler->Retain();
}
//////////////////////////////////////////////////////////////////////////
// ContextModule::ReleaseSampler
//////////////////////////////////////////////////////////////////////////
cl_int ContextModule::ReleaseSampler(cl_sampler clSampler) {
  LOG_DEBUG(TEXT("Enter RetainMemObject (clMemObj=%p)"), clSampler);

  cl_err_code clErr = CL_SUCCESS;
  SharedPtr<Sampler> pSampler =
      m_mapSamplers.GetOCLObject((_cl_sampler_int *)clSampler)
          .DynamicCast<Sampler>();
  if (NULL == pSampler.GetPtr()) {
    LOG_ERROR(TEXT("GetOCLObject(%p, %p) returned %s"), clSampler, &pSampler,
              ClErrTxt(clErr));
    return CL_INVALID_SAMPLER;
  }
  SharedPtr<Context> pContext = pSampler->GetContext();
  if (NULL == pContext.GetPtr()) {
    return CL_INVALID_SAMPLER;
  }

  long newRef = pSampler->Release();
  cl_int res = CL_SUCCESS;
  if (newRef < 0) {
    return CL_INVALID_SAMPLER;
  } else if (0 == newRef) {
    clErr = pContext->RemoveSampler(clSampler);
    if (CL_FAILED(clErr)) {
      res = CL_ERR_OUT(clErr);
    }
    clErr = m_mapSamplers.RemoveObject((_cl_sampler_int *)clSampler);
    if (CL_FAILED(clErr)) {
      res = CL_ERR_OUT(clErr);
    }
  }

  return res;
}

///////////////////////////////////////////////////////////////////////////
// RemoveAllSamplers
//////////////////////////////////////////////////////////////////////////
void ContextModule::RemoveAllSamplers(bool preserve_user_handles) {
  m_mapSamplers.DisableAdding();

  AllObjectsFilter<Sampler>::ObjectsList to_remove;
  GetAllObjects<Sampler>(m_mapSamplers, to_remove);

  if (preserve_user_handles) {
    m_mapSamplers.SetPreserveUserHandles();
  }

  for (AllObjectsFilter<Sampler>::ObjectsList::iterator it = to_remove.begin();
       it != to_remove.end(); ++it) {
    SharedPtr<Sampler> &pObj = *it;

    cl_sampler handle = pObj->GetHandle();
    pObj->GetContext()->RemoveSampler(handle);
    m_mapSamplers.RemoveObject((_cl_sampler_int *)handle);
  }

  to_remove.clear();
  m_mapSamplers.ReleaseAllObjects(false);
}

//////////////////////////////////////////////////////////////////////////
// ContextModule::GetSamplerInfo
//////////////////////////////////////////////////////////////////////////
cl_int ContextModule::GetSamplerInfo(cl_sampler clSampler,
                                     cl_sampler_info clParamName,
                                     size_t szParamValueSize, void *pParamValue,
                                     size_t *pszParamValueSizeRet) {
  LOG_INFO(
      TEXT("Enter GetSamplerInfo (clSampler=%p, clParamName=%u, "
           "szParamValueSize=%zu, pParamValue=%p, pszParamValueSizeRet=%p)"),
      clSampler, clParamName, szParamValueSize, pParamValue,
      pszParamValueSizeRet);

  cl_err_code clErr = CL_SUCCESS;
  SharedPtr<Sampler> pSampler =
      m_mapSamplers.GetOCLObject((_cl_sampler_int *)clSampler)
          .DynamicCast<Sampler>();
  if (CL_FAILED(clErr) || NULL == pSampler.GetPtr()) {
    LOG_ERROR(TEXT("GetOCLObject(%p, %p) returned %s"), clSampler, &pSampler,
              ClErrTxt(clErr));
    return CL_INVALID_SAMPLER;
  }

  clErr = pSampler->GetInfo((cl_int)clParamName, szParamValueSize, pParamValue,
                            pszParamValueSizeRet);
  return CL_ERR_OUT(clErr);
}

SharedPtr<Context> ContextModule::GetContext(cl_context clContext) {
  return m_mapContexts.GetOCLObject((_cl_context_int *)clContext)
      .DynamicCast<Context>();
}

SharedPtr<Kernel> ContextModule::GetKernel(cl_kernel clKernel) {
  return m_mapKernels.GetOCLObject((_cl_kernel_int *)clKernel)
      .DynamicCast<Kernel>();
}

SharedPtr<Program> ContextModule::GetProgram(const cl_program clProgram) {
  return m_mapPrograms.GetOCLObject((_cl_program_int *)clProgram)
      .DynamicCast<Program>();
}

SharedPtr<MemoryObject>
ContextModule::GetMemoryObject(const cl_mem clMemObjId) {
  return m_mapMemObjects.GetOCLObject((_cl_mem_int *)clMemObjId)
      .DynamicCast<MemoryObject>();
}

cl_err_code ContextModule::CheckMemObjectParameters(
    cl_mem_flags clMemFlags, const cl_image_format *clImageFormat,
    cl_mem_object_type clMemObjType, size_t szImageWidth, size_t szImageHeight,
    size_t, size_t szImageRowPitch, size_t szImageSlicePitch,
    size_t /*szArraySize*/, void *pHostPtr, SharedPtr<Context> pContext) {
  cl_mem_flags extensions_flags = 0;
  if (pContext->IsFPGAEmulator()) {
    extensions_flags = CL_CHANNEL_1_INTELFPGA | CL_CHANNEL_2_INTELFPGA |
                       CL_CHANNEL_3_INTELFPGA | CL_CHANNEL_4_INTELFPGA |
                       CL_CHANNEL_5_INTELFPGA | CL_CHANNEL_6_INTELFPGA |
                       CL_MEM_HETEROGENEOUS_INTELFPGA;
  }
  // check for illegal flags
  if ((clMemFlags &
       ~(CL_MEM_READ_WRITE | CL_MEM_WRITE_ONLY | CL_MEM_READ_ONLY |
         CL_MEM_USE_HOST_PTR | CL_MEM_ALLOC_HOST_PTR | CL_MEM_COPY_HOST_PTR |
         CL_MEM_HOST_WRITE_ONLY | CL_MEM_HOST_READ_ONLY |
         CL_MEM_HOST_NO_ACCESS | extensions_flags)) != 0) {
    return CL_INVALID_VALUE;
  }
  // check for illegal flag combinations
  if (((clMemFlags & CL_MEM_READ_ONLY) && (clMemFlags & CL_MEM_WRITE_ONLY)) ||
      ((clMemFlags & CL_MEM_READ_ONLY) && (clMemFlags & CL_MEM_READ_WRITE)) ||
      ((clMemFlags & CL_MEM_WRITE_ONLY) && (clMemFlags & CL_MEM_READ_WRITE)) ||
      ((clMemFlags & CL_MEM_USE_HOST_PTR) &&
       (clMemFlags & CL_MEM_ALLOC_HOST_PTR)) ||
      ((clMemFlags & CL_MEM_HOST_WRITE_ONLY) &&
       (clMemFlags & CL_MEM_HOST_READ_ONLY)) ||
      ((clMemFlags & CL_MEM_HOST_WRITE_ONLY) &&
       (clMemFlags & CL_MEM_HOST_NO_ACCESS)) ||
      ((clMemFlags & CL_MEM_HOST_READ_ONLY) &&
       (clMemFlags & CL_MEM_HOST_NO_ACCESS))) {
    return CL_INVALID_VALUE;
  }

  if ((NULL == pHostPtr) &&
      ((0 != szImageRowPitch) || (0 != szImageSlicePitch))) {
    return CL_INVALID_IMAGE_DESCRIPTOR;
  }

  if ((NULL == pHostPtr) &&
      ((CL_MEM_COPY_HOST_PTR | CL_MEM_USE_HOST_PTR) & clMemFlags)) {
    return CL_INVALID_HOST_PTR;
  }

  if (CL_MEM_OBJECT_IMAGE1D_BUFFER != clMemObjType &&
      CL_MEM_OBJECT_IMAGE2D != clMemObjType && (NULL != pHostPtr) &&
      !((CL_MEM_COPY_HOST_PTR | CL_MEM_USE_HOST_PTR) & clMemFlags)) {
    return CL_INVALID_HOST_PTR;
  }

  if (nullptr != clImageFormat) {
    // Check if channel order and data type are in the range of valid numbers,
    // see cl.h
    if (!(clImageFormat->image_channel_order >= CL_R &&
          clImageFormat->image_channel_order < CL_SNORM_INT8 &&
          clImageFormat->image_channel_data_type >= CL_SNORM_INT8 &&
          clImageFormat->image_channel_data_type < CL_MEM_OBJECT_BUFFER)) {
      return CL_INVALID_IMAGE_FORMAT_DESCRIPTOR;
    }

    size_t pixelBytesCnt = clGetPixelBytesCount(clImageFormat);
    if (0 == pixelBytesCnt) {
      return CL_IMAGE_FORMAT_NOT_SUPPORTED;
    }
    // Check minimum row pitch size
    size_t szMinRowPitchSize = szImageWidth * pixelBytesCnt;
    if ((NULL != pHostPtr) && (0 != szImageRowPitch) &&
        ((szImageRowPitch < szMinRowPitchSize) ||
         (szImageRowPitch % pixelBytesCnt))) {
      return CL_INVALID_IMAGE_DESCRIPTOR;
    }
    // in 1D image array there is no row pitch, just slice pitch
    const size_t szRealRowPitch =
        0 == szImageRowPitch || CL_MEM_OBJECT_IMAGE1D_ARRAY == clMemObjType
            ? szMinRowPitchSize
            : szImageRowPitch;
    const size_t szMinSlicePitchSize =
        CL_MEM_OBJECT_IMAGE1D_ARRAY == clMemObjType
            ? szRealRowPitch
            : szRealRowPitch * szImageHeight;
    if ((NULL != pHostPtr) && (0 != szImageSlicePitch) &&
        ((szImageSlicePitch < szMinSlicePitchSize) ||
         (szImageRowPitch != 0 && szImageSlicePitch % szImageRowPitch))) {
      return CL_INVALID_IMAGE_DESCRIPTOR;
    }
  }

  cl_mem_flags fpgaHostSideFlags = 0;
  if (pContext->IsFPGAEmulator()) {
    fpgaHostSideFlags = CL_MEM_HOST_READ_ONLY | CL_MEM_HOST_WRITE_ONLY;
  }
  if (CL_MEM_OBJECT_PIPE == clMemObjType &&
      (clMemFlags & ~(CL_MEM_READ_WRITE | CL_MEM_WRITE_ONLY | CL_MEM_READ_ONLY |
                      CL_MEM_HOST_NO_ACCESS | fpgaHostSideFlags)) != 0) {
    return CL_INVALID_VALUE;
  }

  return CL_SUCCESS;
}

cl_err_code ContextModule::CheckContextSpecificParameters(
    SharedPtr<Context> pContext, const cl_mem_object_type image_type,
    const size_t image_width, const size_t image_height,
    const size_t image_depth, const size_t array_size,
    const void *pImgBufferHostPtr, cl_mem_flags bufFlags) {
  size_t maxW = (size_t)-1;
  size_t maxH = (size_t)-1;
  size_t maxD = (size_t)-1;
  size_t maxArraySize = (size_t)-1;
  size_t max1dFromBuffer = (size_t)-1;
  bool isArray = (CL_MEM_OBJECT_IMAGE1D_ARRAY == image_type ||
                  CL_MEM_OBJECT_IMAGE2D_ARRAY == image_type);

  const tSetOfDevices *rootDevices = pContext->GetAllRootDevices();

  for (tSetOfDevices::const_iterator devIt = rootDevices->begin();
       devIt != rootDevices->end(); ++devIt) {
    size_t sz;
    SharedPtr<Device> dev = *devIt;

    if (CL_MEM_OBJECT_IMAGE3D == image_type) {
      dev->GetInfo(CL_DEVICE_IMAGE3D_MAX_WIDTH, sizeof(size_t), &sz, nullptr);
    } else {
      // also applies to 1D image width not created from buffer
      dev->GetInfo(CL_DEVICE_IMAGE2D_MAX_WIDTH, sizeof(size_t), &sz, nullptr);
    }
    maxW = maxW > sz ? sz : maxW;

    if (CL_MEM_OBJECT_IMAGE3D == image_type) {
      dev->GetInfo(CL_DEVICE_IMAGE3D_MAX_HEIGHT, sizeof(size_t), &sz, nullptr);
    } else {
      dev->GetInfo(CL_DEVICE_IMAGE2D_MAX_HEIGHT, sizeof(size_t), &sz, nullptr);
    }
    maxH = maxH > sz ? sz : maxH;

    if (CL_MEM_OBJECT_IMAGE3D == image_type) {
      dev->GetInfo(CL_DEVICE_IMAGE3D_MAX_DEPTH, sizeof(size_t), &sz, nullptr);
      maxD = maxD > sz ? sz : maxD;
    }

    if (CL_MEM_OBJECT_IMAGE1D_BUFFER == image_type) {
      dev->GetInfo(CL_DEVICE_IMAGE_MAX_BUFFER_SIZE, sizeof(size_t), &sz,
                   nullptr);
      max1dFromBuffer = max1dFromBuffer > sz ? sz : max1dFromBuffer;
    }

    if (isArray) {
      dev->GetInfo(CL_DEVICE_IMAGE_MAX_ARRAY_SIZE, sizeof(size_t), &sz,
                   nullptr);
      maxArraySize = maxArraySize > sz ? sz : maxArraySize;
    }

    if (NULL != pImgBufferHostPtr && CL_MEM_OBJECT_IMAGE2D == image_type &&
        (bufFlags & CL_MEM_USE_HOST_PTR)) {
      cl_uint uiImgBaseAddrAlign;
      dev->GetInfo(CL_DEVICE_IMAGE_BASE_ADDRESS_ALIGNMENT,
                   sizeof(uiImgBaseAddrAlign), &uiImgBaseAddrAlign, nullptr);
      if (!IS_ALIGNED_ON(pImgBufferHostPtr, uiImgBaseAddrAlign)) {
        return CL_INVALID_IMAGE_FORMAT_DESCRIPTOR;
      }
    }
  }

  // Check (the minimum of) maximum sizes and return CL_INVALID_IMAGE_SIZE if
  // exceeding it.
  switch (image_type) {
  case CL_MEM_OBJECT_IMAGE3D:
    if (image_depth > maxD)
      return CL_INVALID_IMAGE_SIZE;
    LLVM_FALLTHROUGH;
  case CL_MEM_OBJECT_IMAGE2D_ARRAY:
  case CL_MEM_OBJECT_IMAGE2D:
    if (image_height > maxH)
      return CL_INVALID_IMAGE_SIZE;
    LLVM_FALLTHROUGH;
  case CL_MEM_OBJECT_IMAGE1D:
    // for 1D NOT created from a buffer, use CL_DEVICE_IMAGE2D_MAX_WIDTH
    if (image_width > maxW)
      return CL_INVALID_IMAGE_SIZE;
    break;
  default:
    // all other cases covered outside the switch.
    break;
  }

  if (isArray) {
    if (array_size > maxArraySize)
      return CL_INVALID_IMAGE_SIZE;
  }

  if (CL_MEM_OBJECT_IMAGE1D_BUFFER == image_type) {
    // for 1D created from a buffer, use CL_DEVICE_IMAGE_MAX_BUFFER_SIZE
    if (image_width > max1dFromBuffer)
      return CL_INVALID_IMAGE_SIZE;
  }

  return CL_SUCCESS;
}

cl_int
ContextModule::initializeLibraryProgram(SharedPtr<Context> &Ctx,
                                        const cl_uint NumDevices,
                                        SharedPtr<FissionableDevice> *Devices) {
  LOG_INFO(TEXT("%s"), TEXT("initializeLibraryProgram enter"));

  // Create program.
  SharedPtr<Program> Prog;
  std::string KernelNames;
  cl_int Err = Ctx->CreateProgramWithLibraryKernels(NumDevices, Devices, &Prog,
                                                    KernelNames);
  if (CL_FAILED(Err)) {
    LOG_ERROR(TEXT("CreateProgramWithLibraryKernels failed, err %d"), Err);
    return Err;
  }

  Err = m_mapPrograms.AddObject(Prog, false);
  if (CL_FAILED(Err)) {
    LOG_ERROR(TEXT("m_mapPrograms.AddObject failed, err %d"), Err);
    Ctx->RemoveProgram(Prog->GetHandle());
    Prog->Release();
    return Err;
  }

  // Create Kernels for current thread.
  threadid_t TID = clMyThreadId();
  std::vector<std::string> KernelNamesVec = SplitString(KernelNames, ';');
  {
    std::lock_guard<std::mutex> mu(m_backendLibraryMutex);
    for (auto &KName : KernelNamesVec) {
      cl_kernel K = CreateLibraryKernelForThread(Ctx, TID, KName);
      if (!K) {
        LOG_ERROR(TEXT("Failed to create library kernel %s"), KName.c_str());
        return CL_OUT_OF_RESOURCES;
      }
    }
  }
  return Err;
}

cl_int ContextModule::releaseLibraryProgram(const cl_context Ctx) {
  LOG_INFO(TEXT("%s"), TEXT("releaseLibraryProgram enter"));
  std::lock_guard<std::mutex> mu(m_backendLibraryMutex);
  SharedPtr<Context> C =
      m_mapContexts.GetOCLObject((_cl_context_int *)Ctx).DynamicCast<Context>();
  if (!C) {
    // Ctx could be already released.
    return CL_SUCCESS;
  }
  cl_program P = C->GetLibraryProgram();
  if (!P) {
    // TODO replace with assert when context reference count issue is solved.
    return CL_SUCCESS;
  }
  // Release kernels.
  auto &Kernels = C->GetLibraryKernels();
  cl_int Err;
  for (auto I = Kernels.begin(), E = Kernels.end(); I != E; ++I) {
    for (auto &K : I->second) {
      Err = ReleaseKernel(K.second);
      if (CL_FAILED(Err))
        return Err;
    }
  }

  Err = ReleaseProgram(P);

  // TODO not needed when context reference count issue is solved.
  C->ResetLibraryProgramKernels();

  return Err;
}

cl_kernel ContextModule::CreateLibraryKernelForThread(SharedPtr<Context> &Ctx,
                                                      threadid_t TID,
                                                      const std::string &Name) {
  cl_program P = Ctx->GetLibraryProgram();
  if (!P) {
    // This could happen after CreateContext, RetainContext and then
    // ReleaseContext. TODO replace with assert when context reference count
    // issue is solved.
    return nullptr;
  }

  cl_int err;
  cl_kernel K = CreateKernel(P, Name.c_str(), &err);
  if (CL_FAILED(err)) {
    LOG_ERROR(TEXT("Failed to create library kernel, err %d"), err);
    return nullptr;
  }
  Ctx->SetLibraryKernel(TID, Name, K);
  return K;
}

SharedPtr<Kernel> ContextModule::GetLibraryKernel(SharedPtr<Context> &Ctx,
                                                  const std::string &Name) {
  std::lock_guard<std::mutex> mu(m_backendLibraryMutex);
  threadid_t TID = clMyThreadId();
  auto &Kernels = Ctx->GetLibraryKernels();
  cl_kernel K = (Kernels.count(TID) && Kernels[TID].count(Name))
                    ? Kernels[TID][Name]
                    : nullptr;
  if (!K) {
    K = CreateLibraryKernelForThread(Ctx, TID, Name);
    if (!K)
      return nullptr;
  }

  return GetKernel(K);
}

/////////////////////////////////////////////////////////////////////
// OpenCL 1.2 functions
/////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// ContextModule::GetKernelArgInfo
//////////////////////////////////////////////////////////////////////////
cl_int ContextModule::GetKernelArgInfo(cl_kernel clKernel, cl_uint argIndx,
                                       cl_kernel_arg_info paramName,
                                       size_t szParamValueSize,
                                       void *pParamValue,
                                       size_t *pszParamValueSizeRet) {
  LOG_INFO(
      TEXT("Enter clKernel=%p, argIndx=%u, clParamName=%u, "
           "szParamValueSize=%zu, pParamValue=%p, pszParamValueSizeRet=%p"),
      clKernel, argIndx, paramName, szParamValueSize, pParamValue,
      pszParamValueSizeRet);

  cl_err_code clErr = CL_SUCCESS;
  (void)clErr;
  SharedPtr<Kernel> pKernel =
      m_mapKernels.GetOCLObject((_cl_kernel_int *)clKernel)
          .DynamicCast<Kernel>();
  if (NULL == pKernel.GetPtr()) {
    LOG_ERROR(TEXT("GetOCLObject(%p, %p) returned %s"), clKernel, &pKernel,
              ClErrTxt(clErr));
    return CL_INVALID_KERNEL;
  }

  return pKernel->GetKernelArgInfo(argIndx, paramName, szParamValueSize,
                                   pParamValue, pszParamValueSizeRet);
}

void *ContextModule::SVMAlloc(cl_context context, cl_svm_mem_flags flags,
                              size_t size, unsigned int uiAlignment) {
  SharedPtr<Context> pContext = GetContext(context);
  if (pContext.GetPtr() == NULL) {
    LOG_ERROR(TEXT("context is not a valid context"));
    return nullptr;
  }
  if (flags & CL_MEM_SVM_ATOMICS && !(flags & CL_MEM_SVM_FINE_GRAIN_BUFFER)) {
    LOG_ERROR(TEXT("flags does not contain CL_MEM_SVM_FINE_GRAIN_BUFFER "
                   "but does contain CL_MEM_SVM_ATOMICS"));
    return nullptr;
  }
  if ((flags & ~(CL_MEM_READ_WRITE | CL_MEM_WRITE_ONLY | CL_MEM_READ_ONLY |
                 CL_MEM_SVM_FINE_GRAIN_BUFFER | CL_MEM_SVM_ATOMICS)) != 0) {
    LOG_ERROR(TEXT("The values specified in flags are not valid i.e. do "
                   "not match those defined in table 5.13"));
    return nullptr;
  }
  if (0 == size) {
    LOG_ERROR(TEXT("size is 0"));
    return nullptr;
  }
  if (uiAlignment > 0 &&
      (!IsPowerOf2(uiAlignment) || uiAlignment > sizeof(cl_long16))) {
    LOG_ERROR(TEXT("invalid alignment"));
    return nullptr;
  }
  void *pSvmBuf = pContext->SVMAlloc(flags, size, uiAlignment);
  if (pSvmBuf) {
    std::lock_guard<std::mutex> mu(m_SvmUsmMutex);
    m_mapSVMBuffers[pSvmBuf] = pContext;
  }
  return pSvmBuf;
}

void ContextModule::SVMFree(cl_context context, void *pSvmPtr) {
  SharedPtr<Context> pContext = GetContext(context);
  if (pContext.GetPtr() == NULL) {
    LOG_ERROR(TEXT("context is not a valid context"));
    return;
  }
  if (NULL == pSvmPtr) {
    LOG_INFO(TEXT("pSvmPtr is NULL"));
    return;
  }
  pContext->SVMFree(pSvmPtr);
  {
    std::lock_guard<std::mutex> mu(m_SvmUsmMutex);
    m_mapSVMBuffers.erase(pSvmPtr);
  }
}

void *ContextModule::MapHostPipeIntelFPGA(cl_mem pipe, cl_map_flags flags,
                                          size_t requestedSize,
                                          size_t *pMappedSize, cl_int *pError) {
  SharedPtr<Pipe> pPipe =
      m_mapMemObjects.GetOCLObject((_cl_mem_int *)pipe).StaticCast<Pipe>();

  if (!pPipe) {
    if (pError) {
      *pError = CL_INVALID_MEM_OBJECT;
    }
    return nullptr;
  }

  SharedPtr<Context> pContext = pPipe->GetContext();
  return pContext->MapPipe(pPipe, flags, requestedSize, pMappedSize, pError);
}

cl_int ContextModule::UnmapHostPipeIntelFPGA(cl_mem pipe, void *pMappedPtr,
                                             size_t sizeToUnmap,
                                             size_t *pUnmappedSize) {
  SharedPtr<Pipe> pPipe =
      m_mapMemObjects.GetOCLObject((_cl_mem_int *)pipe).StaticCast<Pipe>();

  if (!pPipe) {
    return CL_INVALID_MEM_OBJECT;
  }
  SharedPtr<Context> pContext = pPipe->GetContext();
  return pContext->UnmapPipe(pPipe, pMappedPtr, sizeToUnmap, pUnmappedSize);
}

cl_int ContextModule::ReadPipeIntelFPGA(cl_mem pipe, void *pDst) {
  SharedPtr<Pipe> pPipe =
      m_mapMemObjects.GetOCLObject((_cl_mem_int *)pipe).StaticCast<Pipe>();

  if (!pPipe) {
    return CL_INVALID_MEM_OBJECT;
  }
  SharedPtr<Context> pContext = pPipe->GetContext();
  return pContext->ReadPipe(pPipe, pDst);
}

cl_int ContextModule::WritePipeIntelFPGA(cl_mem pipe, const void *pSrc) {
  SharedPtr<Pipe> pPipe =
      m_mapMemObjects.GetOCLObject((_cl_mem_int *)pipe).StaticCast<Pipe>();

  if (!pPipe) {
    return CL_INVALID_MEM_OBJECT;
  }
  SharedPtr<Context> pContext = pPipe->GetContext();
  return pContext->WritePipe(pPipe, pSrc);
}

cl_int ContextModule::GetProfileDataDeviceIntelFPGA(
    cl_device_id /*device_id*/, cl_program program,
    cl_bool /*read_enqueue_kernels*/, cl_bool /*read_auto_enqueued*/,
    cl_bool /*clear_counters_after_readback*/, size_t /*param_value_size*/,
    void * /*param_value*/, size_t * /*param_value_size_ret*/,
    cl_int *errcode_ret) {
  if (program == nullptr) {
    if (errcode_ret != nullptr) {
      *errcode_ret = CL_INVALID_PROGRAM;
    }
    return CL_INVALID_PROGRAM;
  }

  // Yet this implementation is a stub, so the function is just returning
  // CL_INVALID_DEVICE for an FPGA emulator.
  if (errcode_ret != nullptr) {
    *errcode_ret = CL_INVALID_DEVICE;
  }
  return CL_INVALID_DEVICE;
}
//////////////////////////////////////////////////////////////////////////////
//
// cl_intel_function_pointers functions
//
//////////////////////////////////////////////////////////////////////////////

cl_int ContextModule::GetDeviceFunctionPointer(cl_device_id device,
                                               cl_program program,
                                               const char *func_name,
                                               cl_ulong *func_pointer_ret) {
  if (nullptr == func_name || nullptr == func_pointer_ret) {
    return CL_INVALID_VALUE;
  }

  // TODO: check for CL_INVALID_DEVICE

  SharedPtr<Program> pProgram =
      m_mapPrograms.GetOCLObject((_cl_program_int *)program)
          .DynamicCast<Program>();

  if (NULL == pProgram.GetPtr()) {
    return CL_INVALID_PROGRAM;
  }

  cl_int error =
      pProgram->GetDeviceFunctionPointer(device, func_name, func_pointer_ret);

  return error;
}

cl_int ContextModule::GetDeviceGlobalVariablePointer(
    cl_device_id device, cl_program program, const char *gv_name,
    size_t *gv_size_ret, void **gv_pointer_ret, cl_prog_gv *gv_ret) {
  if (nullptr == gv_name || (nullptr == gv_pointer_ret && nullptr == gv_ret))
    return CL_INVALID_VALUE;

  SharedPtr<Program> pProgram =
      m_mapPrograms.GetOCLObject((_cl_program_int *)program)
          .DynamicCast<Program>();
  if (nullptr == pProgram.GetPtr())
    return CL_INVALID_PROGRAM;

  cl_prog_gv gv;
  cl_int err = pProgram->GetDeviceGlobalVariablePointer(device, gv_name, &gv);
  if (CL_FAILED(err))
    return err;

  if (gv_ret) {
    *gv_ret = gv;
    return CL_SUCCESS;
  }

  if (gv_size_ret)
    *gv_size_ret = gv.size;
  *gv_pointer_ret = gv.pointer;
  return CL_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
//
// cl_intel_unified_shared_memory functions
//
//////////////////////////////////////////////////////////////////////////////

void *ContextModule::USMHostAlloc(cl_context context,
                                  const cl_mem_properties_intel *properties,
                                  size_t size, cl_uint alignment,
                                  cl_int *errcode_ret) {
  SharedPtr<Context> pContext = GetContext(context);
  if (nullptr == pContext.GetPtr()) {
    LOG_ERROR(TEXT("context is not a valid context"));
    if (errcode_ret)
      *errcode_ret = CL_INVALID_CONTEXT;
    return nullptr;
  }

  void *pUsmBuf =
      pContext->USMHostAlloc(properties, size, alignment, errcode_ret);
  if (pUsmBuf) {
    std::lock_guard<std::mutex> mu(m_SvmUsmMutex);
    m_mapUSMBuffers[pUsmBuf] = pContext;
  }
  return pUsmBuf;
}

void *ContextModule::USMDeviceAlloc(cl_context context, cl_device_id device,
                                    const cl_mem_properties_intel *properties,
                                    size_t size, cl_uint alignment,
                                    cl_int *errcode_ret) {
  SharedPtr<Context> pContext = GetContext(context);
  if (nullptr == pContext.GetPtr()) {
    LOG_ERROR(TEXT("context is not a valid context"));
    if (errcode_ret)
      *errcode_ret = CL_INVALID_CONTEXT;
    return nullptr;
  }

  void *pUsmBuf = pContext->USMDeviceAlloc(device, properties, size, alignment,
                                           errcode_ret);
  if (pUsmBuf) {
    std::lock_guard<std::mutex> mu(m_SvmUsmMutex);
    m_mapUSMBuffers[pUsmBuf] = pContext;
  }
  return pUsmBuf;
}

void *ContextModule::USMSharedAlloc(cl_context context, cl_device_id device,
                                    const cl_mem_properties_intel *properties,
                                    size_t size, cl_uint alignment,
                                    cl_int *errcode_ret) {
  SharedPtr<Context> pContext = GetContext(context);
  if (nullptr == pContext.GetPtr()) {
    LOG_ERROR(TEXT("context is not a valid context"));
    if (errcode_ret)
      *errcode_ret = CL_INVALID_CONTEXT;
    return nullptr;
  }

  void *pUsmBuf = pContext->USMSharedAlloc(device, properties, size, alignment,
                                           errcode_ret);
  if (pUsmBuf) {
    std::lock_guard<std::mutex> mu(m_SvmUsmMutex);
    m_mapUSMBuffers[pUsmBuf] = pContext;
  }
  return pUsmBuf;
}

cl_int ContextModule::USMFree(cl_context context, void *ptr) {
  SharedPtr<Context> pContext = GetContext(context);
  if (nullptr == pContext.GetPtr()) {
    LOG_ERROR(TEXT("context is not a valid context"));
    return CL_INVALID_CONTEXT;
  }
  if (nullptr == ptr) {
    LOG_INFO(TEXT("ptr is nullptr. No action occurs."));
    return CL_SUCCESS;
  }
  cl_err_code err = pContext->USMFree(ptr);
  if (CL_SUCCESS == err) {
    std::lock_guard<std::mutex> mu(m_SvmUsmMutex);
    m_mapUSMBuffers.erase(ptr);
  }
  return err;
}

cl_int ContextModule::USMBlockingFree(cl_context context, void *ptr) {
  SharedPtr<Context> pContext = GetContext(context);
  if (nullptr == pContext.GetPtr()) {
    LOG_ERROR(TEXT("context is not a valid context"));
    return CL_INVALID_CONTEXT;
  }

  if (nullptr == ptr) {
    LOG_INFO(TEXT("ptr is nullptr. No action occurs."));
    return CL_SUCCESS;
  }

  auto usmBufferIter = m_mapUSMBuffers.find(ptr);
  if (usmBufferIter == m_mapUSMBuffers.end()) {
    LOG_ERROR(TEXT("ptr isn't a USM buffer"));
    return CL_INVALID_VALUE;
  }

  std::vector<cl_event> waitList;
  cl_err_code err = CL_SUCCESS;
  {
    std::lock_guard<std::mutex> mu(m_SvmUsmMutex);
    auto waitListIter = m_mapUSMFreeWaitList.find(ptr);
    if (waitListIter != m_mapUSMFreeWaitList.end())
      for (const auto &eventSPtr : waitListIter->second)
        waitList.push_back(eventSPtr.get());
  }

  // Wait for all commands referring this USM buffer to complete.
  if (!waitList.empty()) {
    // After we get the waitList, we should skip invalid events in case some
    // user events are released before waiting on them.
    err = FrameworkProxy::Instance()
              ->GetExecutionModule()
              ->GetEventsManager()
              ->WaitForEvents(waitList.size(), waitList.data(),
                              /*skipInvalidEvents*/ true);

    if (CL_FAILED(err)) {
      // TODO: The document doesn't say which error code we should return
      // if some internal errors occurs.
      LOG_ERROR(TEXT("Failed to wait for related events done."));
      return err;
    }
  }

  err = pContext->USMFree(ptr);

  if (CL_SUCCESS == err) {
    std::lock_guard<std::mutex> mu(m_SvmUsmMutex);
    m_mapUSMBuffers.erase(usmBufferIter);
    auto waitListIter = m_mapUSMFreeWaitList.find(ptr);
    if (waitListIter != m_mapUSMFreeWaitList.end())
      m_mapUSMFreeWaitList.erase(waitListIter);
  }

  return err;
}

cl_int ContextModule::GetMemAllocInfoINTEL(cl_context context, const void *ptr,
                                           cl_mem_info_intel param_name,
                                           size_t param_value_size,
                                           void *param_value,
                                           size_t *param_value_size_ret) {
  SharedPtr<Context> pContext = GetContext(context);
  if (nullptr == pContext.GetPtr()) {
    LOG_ERROR(TEXT("context is not a valid context"));
    return CL_INVALID_CONTEXT;
  }
  cl_err_code err = pContext->GetMemAllocInfoINTEL(
      ptr, param_name, param_value_size, param_value, param_value_size_ret);
  return err;
}

cl_int ContextModule::SetKernelArgUSMPointer(cl_kernel clKernel,
                                             cl_uint uiArgIndex,
                                             const void *pArgValue) {
  SharedPtr<Kernel> pKernel =
      m_mapKernels.GetOCLObject((_cl_kernel_int *)clKernel)
          .StaticCast<Kernel>();
  if (nullptr == pKernel.GetPtr()) {
    LOG_ERROR(TEXT("GetOCLObject(%p, %p) returned nullptr"), clKernel,
              &pKernel);
    return CL_INVALID_KERNEL;
  }

  cl_err_code err = pKernel->SetKernelArg(uiArgIndex, sizeof(pArgValue),
                                          pArgValue, false, true);
  return CL_ERR_OUT(err);
}

//////////////////////////////////////////////////////////////////////////////
//
// Utility functions
//
/////////////////////////////////////////////////////////////////////////////
void ContextModule::RegisterMappedMemoryObject(MemoryObject *pMemObj) {
  m_setMappedMemObjects.add(pMemObj);
}

void ContextModule::UnRegisterMappedMemoryObject(MemoryObject *pMemObj) {
  m_setMappedMemObjects.remove(pMemObj);
}

void ContextModule::RegisterUSMFreeWaitEvent(
    const void *usmPtr, std::shared_ptr<_cl_event> eventSPtr) {
  assert(usmPtr != nullptr && "Not a valid USM pointer");
  std::lock_guard<std::mutex> mu(m_SvmUsmMutex);
  m_mapUSMFreeWaitList[usmPtr].push_back(eventSPtr);
}

void ContextModule::UnregisterUSMFreeWaitEvent(const void *usmPtr,
                                               cl_event evt) {
  assert(usmPtr != nullptr && "Not a valid USM pointer");
  std::lock_guard<std::mutex> mu(m_SvmUsmMutex);
  auto WaitListIter = m_mapUSMFreeWaitList.find(usmPtr);
  if (WaitListIter == m_mapUSMFreeWaitList.end())
    return;

  auto &WaitList = WaitListIter->second;
  auto It = std::find_if(WaitList.begin(), WaitList.end(),
                         [evt](auto &I) { return evt == I.get(); });
  if (It != WaitList.end())
    WaitList.erase(It);
}
