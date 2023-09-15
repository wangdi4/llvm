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

#pragma once

#include "cl_dev_backend_api.h"
#include <assert.h>

namespace Intel {
namespace OpenCL {
namespace DeviceBackend {

class ICLDebuggingService;

// Internal backend service factory interface. Adds another method on top of
// the backend service factory interface - to get a debugging service.
// This interface is separated because it's only being used internally in the
// backend.
//
class ICLDevBackendServiceFactoryInternal : public ICLDevBackendServiceFactory {
public:
  virtual cl_dev_err_code
  GetDebuggingService(ICLDebuggingService **pDebuggingService) = 0;
};

class ServiceFactory : public ICLDevBackendServiceFactoryInternal {
private:
  ServiceFactory();
  ~ServiceFactory();

public:
  static void Init();
  static void Terminate();
  static ICLDevBackendServiceFactory *GetInstance();

  /**
   * Get an instance as a pointer to the internal service factory interface.
   * Required to be able to access methods added in the internal interface.
   */
  static ICLDevBackendServiceFactoryInternal *GetInstanceInternal();

  /**
   * Creates Compilation Service object
   *
   * @param pBackendOptions pointer to class which will contain the backend
   * compilation options (target arch, target description , etc ..)
   * @param pBackendCompilationService will be modified to contain the generated
   * object
   *
   * @returns
   *  CL_DEV_SUCCESS in case of success, otherwise:
   *  CL_DEV_OUT_OF_MEMORY in case of lack of memory
   *  CL_DEV_ERROR_FAIL in any other failure
   */
  virtual cl_dev_err_code GetCompilationService(
      const ICLDevBackendOptions *pBackendOptions,
      ICLDevBackendCompilationService **pBackendCompilationService) override;

  /**
   * Creates Execution Service object
   *
   * @param pBackendOptions pointer to class which will contain the backend
   * execution options (like printf handler address, alloc\dealloc program JIT
   * handler address, etc ..)
   * @param pBackendExecutionService will be modified to contain the generated
   * object
   *
   * @returns
   *  CL_DEV_SUCCESS in case of success, otherwise:
   *  CL_DEV_OUT_OF_MEMORY in case of lack of memory
   *  CL_DEV_NOT_SUPPORTED in case of requesting ACCELERATOR execution service
   * on the host. CL_DEV_ERROR_FAIL in any other failure
   */
  virtual cl_dev_err_code GetExecutionService(
      const ICLDevBackendOptions *pBackendOptions,
      ICLDevBackendExecutionService **pBackendExecutionService) override;

  /**
   * Creates Serialization Service object
   *
   * @param pBackendOptions pointer to class which will contain the backend
   * compilation options (alloc\dealloc program JIT handler address, etc ..)
   * @param pBackendSerializationService will be modified to contain the
   * generated object
   *
   * @returns
   *  CL_DEV_SUCCESS in case of success, otherwise:
   *  CL_DEV_OUT_OF_MEMORY in case of lack of memory
   *  CL_DEV_ERROR_FAIL in any other failure
   */
  virtual cl_dev_err_code
  GetSerializationService(const ICLDevBackendOptions *pBackendOptions,
                          ICLDevBackendSerializationService *
                              *pBackendSerializationService) override;

  /**
   * Creates Image Service object
   *
   * @param pBackendOptions pointer to class which will contain the backend
   * compilation options (alloc\dealloc program JIT handler address, etc ..)
   * @param pBackendImageService [OUT] will be modified to contain the generated
   * object
   *
   * @returns
   *  CL_DEV_SUCCESS in case of success, otherwise:
   *  CL_DEV_OUT_OF_MEMORY in case of lack of memory
   *  CL_DEV_ERROR_FAIL in any other failure
   */
  virtual cl_dev_err_code
  GetImageService(const ICLDevBackendOptions *pBackendOptions,
                  ICLDevBackendImageService **ppBackendImageService) override;

  /**
   * Creates a Debugging Service object
   *
   * @ param pDebuggingService will be modified to contain the generated
   *         object.
   *
   * @returns
   *  CL_DEV_SUCCESS in case of success, otherwise:
   *  CL_DEV_ERROR_FAIL in case of failure
   */
  virtual cl_dev_err_code
  GetDebuggingService(ICLDebuggingService **pDebuggingService) override;

private:
  static ServiceFactory *s_pInstance;
};

} // namespace DeviceBackend
} // namespace OpenCL
} // namespace Intel
