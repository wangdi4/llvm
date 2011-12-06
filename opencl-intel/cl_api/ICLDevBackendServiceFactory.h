#ifndef CLDevBackendManagerFactory_H
#define CLDevBackendManagerFactory_H

#include "cl_device_api.h"
#include "ICLDevBackendOptions.h"
#include "ICLDevBackendCompilationService.h"
#include "ICLDevBackendSerializationService.h"
#include "ICLDevBackendExecutionService.h"
#include "ICLDevBackendImageService.h"

namespace Intel { namespace OpenCL { namespace DeviceBackend {

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
class ICLDevBackendServiceFactory
{
public:
    /**
     * Creates Compilation Service object
     *
     * @param pBackendOptions pointer to class which will contain the backend compilation
     *  options (target arch, target description , etc ..)
     * @param pBackendCompilationService [OUT] will be modified to contain the generated object
     *
     * @returns 
     *  CL_DEV_SUCCESS in case of success, otherwise:
     *  CL_DEV_OUT_OF_MEMORY in case of lack of memory
     *  CL_DEV_ERROR_FAIL in any other failure
     */
    virtual cl_dev_err_code GetCompilationService(
        const ICLDevBackendOptions* pBackendOptions, 
        ICLDevBackendCompilationService** pBackendCompilationService) = 0;

    /**
     * Creates Execution Service object
     *
     * @param pBackendOptions pointer to class which will contain the backend execution
     *  options (like printf handler address, alloc\dealloc program JIT handler address, etc ..)
     * @param pBackendExecutionService [OUT] will be modified to contain the generated object
     *
     * @returns 
     *  CL_DEV_SUCCESS in case of success, otherwise:
     *  CL_DEV_OUT_OF_MEMORY in case of lack of memory
     *  CL_DEV_ERROR_FAIL in any other failure
     */
    virtual cl_dev_err_code GetExecutionService(
        const ICLDevBackendOptions* pBackendOptions, 
        ICLDevBackendExecutionService** pBackendExecutionService) = 0;

    /**
     * Creates Serialization Service object
     *
     * @param pBackendOptions pointer to class which will contain the backend compilation
     *  options (alloc\dealloc program JIT handler address, etc ..)
     * @param pBackendSerializationService [OUT] will be modified to contain the generated object
     *
     * @returns 
     *  CL_DEV_SUCCESS in case of success, otherwise:
     *  CL_DEV_OUT_OF_MEMORY in case of lack of memory
     *  CL_DEV_ERROR_FAIL in any other failure
     */
    virtual cl_dev_err_code GetSerializationService(
        const ICLDevBackendOptions* pBackendOptions, 
        ICLDevBackendSerializationService** pBackendSerializationService) = 0;

    /**
     * Creates Image Service object
     *
     * @param pBackendOptions pointer to class which will contain the backend compilation
     *  options (alloc\dealloc program JIT handler address, etc ..)
     * @param pBackendImageService [OUT] will be modified to contain the generated object
     *
     * @returns 
     *  CL_DEV_SUCCESS in case of success, otherwise:
     *  CL_DEV_OUT_OF_MEMORY in case of lack of memory
     *  CL_DEV_ERROR_FAIL in any other failure
     */
    virtual cl_dev_err_code GetImageService(
      const ICLDevBackendOptions* pBackendOptions, 
      ICLDevBackendImageService** ppBackendImageService) = 0;
};

}}} // namespace

// Defines the exported functions for the DLL application.
#ifdef __cplusplus
extern "C" 
{
#endif
    using namespace Intel::OpenCL::DeviceBackend;
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
    //extern "C" cl_dev_err_code InitBackend(
    //	const ICLDevBackendOptions* pBackendOptions);
    LLVM_BACKEND_API cl_dev_err_code InitDeviceBackend(const ICLDevBackendOptions* pBackendOptions);
    /**
     * @returns the backend manager factory instance in success, NULL otherwise
     *  NOTICE: that the factory is singelton
     */
    LLVM_BACKEND_API ICLDevBackendServiceFactory* GetDeviceBackendFactory();

    /**
     * Terminates the backend. Frees internal structures
     */
    LLVM_BACKEND_API void TerminateDeviceBackend();

    /*
     * Function pointer types
     */
    typedef cl_dev_err_code (*BACKEND_INIT_FUNCPTR)(const ICLDevBackendOptions* pBackendOptions);
    typedef void (*BACKEND_TERMINATE_FUNCPTR)();
    typedef ICLDevBackendServiceFactory*  (*BACKEND_GETFACTORY_FUNCPTR)();

#ifdef __cplusplus
}
#endif

#endif // CLDevBackendManagerFactory_H
