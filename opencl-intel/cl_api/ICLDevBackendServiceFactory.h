// INTEL CONFIDENTIAL
//
// Copyright 2012 Intel Corporation.
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

#ifndef CLDevBackendManagerFactory_H
#define CLDevBackendManagerFactory_H

#include "ICLDevBackendCompilationService.h"
#include "ICLDevBackendExecutionService.h"
#include "ICLDevBackendImageService.h"
#include "ICLDevBackendOptions.h"
#include "ICLDevBackendSerializationService.h"
#include "cl_device_api.h"

namespace Intel {
namespace OpenCL {
namespace DeviceBackend {

#ifndef LLVM_BACKEND_API
#if defined(_WIN32)
#ifdef OclCpuBackEnd_EXPORTS
#define LLVM_BACKEND_API __declspec(dllexport)
#else
#define LLVM_BACKEND_API __declspec(dllimport)
#endif
#else
#define LLVM_BACKEND_API
#endif
#endif

/**
 * Factory which is responsible for service objects creation
 */
class ICLDevBackendServiceFactory {
public:
  virtual ~ICLDevBackendServiceFactory() {}

  /**
   * Creates Compilation Service object
   *
   * @param pBackendOptions pointer to class which will contain the backend
   * compilation options (target arch, target description , etc ..)
   * @param pBackendCompilationService [OUT] will be modified to contain the
   * generated object
   *
   * @returns
   *  CL_DEV_SUCCESS in case of success, otherwise:
   *  CL_DEV_OUT_OF_MEMORY in case of lack of memory
   *  CL_DEV_ERROR_FAIL in any other failure
   */
  virtual cl_dev_err_code GetCompilationService(
      const ICLDevBackendOptions *pBackendOptions,
      ICLDevBackendCompilationService **pBackendCompilationService) = 0;

  /**
   * Creates Execution Service object
   *
   * @param pBackendOptions pointer to class which will contain the backend
   * execution options (like printf handler address, alloc\dealloc program JIT
   * handler address, etc ..)
   * @param pBackendExecutionService [OUT] will be modified to contain the
   * generated object
   *
   * @returns
   *  CL_DEV_SUCCESS in case of success, otherwise:
   *  CL_DEV_OUT_OF_MEMORY in case of lack of memory
   *  CL_DEV_ERROR_FAIL in any other failure
   */
  virtual cl_dev_err_code GetExecutionService(
      const ICLDevBackendOptions *pBackendOptions,
      ICLDevBackendExecutionService **pBackendExecutionService) = 0;

  /**
   * Creates Serialization Service object
   *
   * @param pBackendOptions pointer to class which will contain the backend
   * compilation options (alloc\dealloc program JIT handler address, etc ..)
   * @param pBackendSerializationService [OUT] will be modified to contain the
   * generated object
   *
   * @returns
   *  CL_DEV_SUCCESS in case of success, otherwise:
   *  CL_DEV_OUT_OF_MEMORY in case of lack of memory
   *  CL_DEV_ERROR_FAIL in any other failure
   */
  virtual cl_dev_err_code GetSerializationService(
      const ICLDevBackendOptions *pBackendOptions,
      ICLDevBackendSerializationService **pBackendSerializationService) = 0;

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
                  ICLDevBackendImageService **ppBackendImageService) = 0;
};

} // namespace DeviceBackend
} // namespace OpenCL
} // namespace Intel

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize the backend for use
 *
 * @param pBackendOptions options to be passed to the backend
 *
 * @returns
 *  CL_DEV_SUCCESS in case of success, otherwise:
 *  CL_DEV_OUT_OF_MEMORY in case of lack of memory
 *  CL_DEV_ERROR_FAIL in any other failure
 */
// extern "C" cl_dev_err_code InitBackend(
//  const ICLDevBackendOptions* pBackendOptions);
cl_dev_err_code InitDeviceBackend(
    const Intel::OpenCL::DeviceBackend::ICLDevBackendOptions *pBackendOptions);
/**
 * @returns the backend manager factory instance in success, NULL otherwise
 *  NOTICE: that the factory is singelton
 */
Intel::OpenCL::DeviceBackend::ICLDevBackendServiceFactory *
GetDeviceBackendFactory();

/**
 * Terminates the backend. Frees internal structures
 */
void TerminateDeviceBackend();

/*
 * Function pointer types
 */
typedef cl_dev_err_code (*BACKEND_INIT_FUNCPTR)(
    const Intel::OpenCL::DeviceBackend::ICLDevBackendOptions *pBackendOptions);
typedef void (*BACKEND_TERMINATE_FUNCPTR)();
typedef Intel::OpenCL::DeviceBackend::ICLDevBackendServiceFactory *(
    *BACKEND_GETFACTORY_FUNCPTR)();

#ifdef __cplusplus
}
#endif

#endif // CLDevBackendManagerFactory_H
