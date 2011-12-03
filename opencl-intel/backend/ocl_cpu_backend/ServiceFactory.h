/*****************************************************************************\

Copyright (c) Intel Corporation (2010).

    INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
    LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
    ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
    PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
    DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
    PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
    including liability for infringement of any proprietary rights, relating to
    use of the code. No license, express or implied, by estoppels or otherwise,
    to any intellectual property rights is granted herein.

File Name:  ServiceFactory.h

\*****************************************************************************/
#pragma once

#include <assert.h>
#include "cl_dev_backend_api.h"

namespace Intel { namespace OpenCL { namespace DeviceBackend {

// Backend Operation Modes enumeration
enum OPERATION_MODE
{
    CPU_MODE = 0,
    MIC_MODE
};

class ServiceFactory: public ICLDevBackendServiceFactory
{
private:
    ServiceFactory();
    ~ServiceFactory();

public:
    static void Init();
    static void Terminate();
    static ICLDevBackendServiceFactory* GetInstance();

    /**
     * Creates Compilation Service object
     *
     * @param pBackendOptions pointer to class which will contain the backend compilation
     *  options (target arch, target description , etc ..)
     * @param pBackendCompilationService will be modified to contain the generated object
     *
     * @returns 
     *  CL_DEV_SUCCESS in case of success, otherwise:
     *  CL_DEV_OUT_OF_MEMORY in case of lack of memory
     *  CL_DEV_ERROR_FAIL in any other failure
     */
    virtual cl_dev_err_code GetCompilationService( 
        const ICLDevBackendOptions* pBackendOptions,
        ICLDevBackendCompilationService** pBackendCompilationService);

    /**
     * Creates Execution Service object
     *
     * @param pBackendOptions pointer to class which will contain the backend execution
     *  options (like printf handler address, alloc\dealloc program JIT handler address, etc ..)
     * @param pBackendExecutionService will be modified to contain the generated object
     *
     * @returns 
     *  CL_DEV_SUCCESS in case of success, otherwise:
     *  CL_DEV_OUT_OF_MEMORY in case of lack of memory
     *  CL_DEV_ERROR_FAIL in any other failure
     */
    virtual cl_dev_err_code GetExecutionService(
        const ICLDevBackendOptions* pBackendOptions, 
        ICLDevBackendExecutionService** pBackendExecutionService);

    /**
     * Creates Serialization Service object
     *
     * @param pBackendOptions pointer to class which will contain the backend compilation
     *  options (alloc\dealloc program JIT handler address, etc ..)
     * @param pBackendSerializationService will be modified to contain the generated object
     *
     * @returns 
     *  CL_DEV_SUCCESS in case of success, otherwise:
     *  CL_DEV_OUT_OF_MEMORY in case of lack of memory
     *  CL_DEV_ERROR_FAIL in any other failure
     */
    virtual cl_dev_err_code GetSerializationService(
        const ICLDevBackendOptions* pBackendOptions, 
        ICLDevBackendSerializationService** pBackendSerializationService);

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
      ICLDevBackendImageService** ppBackendImageService);
    
private:
    static ServiceFactory* s_pInstance;
};

}}}