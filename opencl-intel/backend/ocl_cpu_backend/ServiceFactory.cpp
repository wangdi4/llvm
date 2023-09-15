// INTEL CONFIDENTIAL
//
// Copyright 2010 Intel Corporation.
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

#include "ServiceFactory.h"
#include "BackendConfiguration.h"
#include "CPUCompileService.h"
#include "CPUExecutionService.h"
#include "ImageCallbackServices.h"
#include "cl_cpu_detect.h"
#include "debuggingservicewrapper.h"
#include "exceptions.h"

namespace Intel {
namespace OpenCL {
namespace DeviceBackend {

ServiceFactory *ServiceFactory::s_pInstance = nullptr;

ServiceFactory::ServiceFactory() {}

ServiceFactory::~ServiceFactory() {}

void ServiceFactory::Init() {
  assert(!s_pInstance);
  s_pInstance = new ServiceFactory();
}

void ServiceFactory::Terminate() {
  if (nullptr != s_pInstance) {
    delete s_pInstance;
    s_pInstance = nullptr;
  }
}

ICLDevBackendServiceFactory *ServiceFactory::GetInstance() {
  assert(s_pInstance);
  return s_pInstance;
}

ICLDevBackendServiceFactoryInternal *ServiceFactory::GetInstanceInternal() {
  assert(s_pInstance);
  return s_pInstance;
}

cl_dev_err_code ServiceFactory::GetCompilationService(
    const ICLDevBackendOptions *pBackendOptions,
    ICLDevBackendCompilationService **ppBackendCompilationService) {
  try {
    if (nullptr == ppBackendCompilationService) {
      return CL_DEV_INVALID_VALUE;
    }

    if (nullptr != pBackendOptions) {
      size_t device = pBackendOptions->GetIntValue(
          (int)CL_DEV_BACKEND_OPTION_DEVICE, CPU_DEVICE);
      if (CPU_DEVICE != device && FPGA_EMU_DEVICE != device) {
        throw Exceptions::DeviceBackendExceptionBase(
            "Unsupported device", CL_DEV_INVALID_OPERATION_MODE);
      }
    }

    std::unique_ptr<ICompilerConfig> config =
        BackendConfiguration::GetInstance().GetCPUCompilerConfig(
            pBackendOptions);
    *ppBackendCompilationService = new CPUCompileService(std::move(config));
    return CL_DEV_SUCCESS;
  } catch (Exceptions::DeviceBackendExceptionBase &e) {
    return e.GetErrorCode();
  } catch (std::bad_alloc &) {
    return CL_DEV_OUT_OF_MEMORY;
  } catch (std::runtime_error &) {
    return CL_DEV_ERROR_FAIL;
  }
}

cl_dev_err_code ServiceFactory::GetExecutionService(
    const ICLDevBackendOptions *pBackendOptions,
    ICLDevBackendExecutionService **ppBackendExecutionService) {
  try {
    if (nullptr == ppBackendExecutionService) {
      return CL_DEV_INVALID_VALUE;
    }

    if (nullptr != pBackendOptions) {
      size_t device = pBackendOptions->GetIntValue(
          (int)CL_DEV_BACKEND_OPTION_DEVICE, CPU_DEVICE);
      if (CPU_DEVICE != device && FPGA_EMU_DEVICE != device) {
        throw Exceptions::DeviceBackendExceptionBase(
            "Unsupported device", CL_DEV_INVALID_OPERATION_MODE);
      }
    }

    *ppBackendExecutionService = new CPUExecutionService(pBackendOptions);
    return CL_DEV_SUCCESS;
  } catch (Exceptions::DeviceBackendExceptionBase &e) {
    return e.GetErrorCode();
  } catch (std::bad_alloc &) {
    return CL_DEV_OUT_OF_MEMORY;
  }
}

cl_dev_err_code ServiceFactory::GetSerializationService(
    const ICLDevBackendOptions *pBackendOptions,
    ICLDevBackendSerializationService **pBackendSerializationService) {
  try {
    if (nullptr == pBackendSerializationService) {
      return CL_DEV_INVALID_VALUE;
    }

    if (nullptr != pBackendOptions) {
      size_t device = pBackendOptions->GetIntValue(
          (int)CL_DEV_BACKEND_OPTION_DEVICE, CPU_DEVICE);
      if (CPU_DEVICE != device && FPGA_EMU_DEVICE != device) {
        throw Exceptions::DeviceBackendExceptionBase(
            "Unsupported device", CL_DEV_INVALID_OPERATION_MODE);
      }
    }

    throw Exceptions::DeviceBackendExceptionBase(
        "Serialization Service Not Implemented for CPU Device",
        CL_DEV_INVALID_OPERATION_MODE);
  } catch (Exceptions::DeviceBackendExceptionBase &e) {
    return e.GetErrorCode();
  } catch (std::bad_alloc &) {
    return CL_DEV_OUT_OF_MEMORY;
  }
}

cl_dev_err_code
ServiceFactory::GetDebuggingService(ICLDebuggingService **pDebuggingService) {
  ICLDebuggingService *instance =
      DebuggingServiceWrapper::GetInstance().GetDebuggingService();
  *pDebuggingService = instance;
  return instance == nullptr ? CL_DEV_ERROR_FAIL : CL_DEV_SUCCESS;
}

cl_dev_err_code ServiceFactory::GetImageService(
    const ICLDevBackendOptions *pBackendOptions,
    ICLDevBackendImageService **ppBackendImageService) {
  try {
    if (nullptr != pBackendOptions) {
      size_t device = pBackendOptions->GetIntValue(
          (int)CL_DEV_BACKEND_OPTION_DEVICE, CPU_DEVICE);
      if (CPU_DEVICE != device && FPGA_EMU_DEVICE != device) {
        throw Exceptions::DeviceBackendExceptionBase(
            "Unsupported device", CL_DEV_INVALID_OPERATION_MODE);
      }
    }

    /// WORKAROUND!! Wee need to skip built-in module load for
    /// Image compiler instance
    std::unique_ptr<ICompilerConfig> config =
        BackendConfiguration::GetInstance().GetCPUCompilerConfig(
            pBackendOptions, /*SkipBuiltins*/ true);
    *ppBackendImageService = new ImageCallbackService(*config, true);
    return CL_DEV_SUCCESS;
  } catch (Exceptions::DeviceBackendExceptionBase &e) {
    return e.GetErrorCode();
  } catch (std::bad_alloc &) {
    return CL_DEV_OUT_OF_MEMORY;
  }
}

} // namespace DeviceBackend
} // namespace OpenCL
} // namespace Intel
