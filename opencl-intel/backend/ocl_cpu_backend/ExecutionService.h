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

File Name:  ExecutionService.h

\*****************************************************************************/
#pragma once

#include "cl_dev_backend_api.h"
#include "IAbstractBackendFactory.h"
#ifdef OCL_DEV_BACKEND_PLUGINS
#include "plugin_manager.h"
#endif

namespace Intel { namespace OpenCL { namespace DeviceBackend {

class Kernel;
class KernelProperties;

class ExecutionService: public ICLDevBackendExecutionService
{
public:
    ExecutionService(const ICLDevBackendOptions *pOptions);
    /**
     * @returns the target machine description size in bytes
     */
    virtual size_t GetTargetMachineDescriptionSize() const;

    /**
     * Gets the target machine description in the already allocated buffer
     *
     * @param pTargetDescription pointer to the allocated buffer to be filled with the
     *  target machine description
     * @param descriptionSize the size of the allocated buffer
     *
     * @returns CL_DEV_SUCCESS and the pTargetDescription will be filled with the
     *  description in case of success; otherwise:
     *  CL_DEV_INVALID_VALUE in case pTargetDescription == NULL
     *  CL_DEV_ERROR_FAIL in any other error
     */
    virtual cl_dev_err_code GetTargetMachineDescription(
        void* pTargetDescription,
        size_t descriptionSize) const;

    /**
     * Releases the Execution Service
     */
    virtual void Release();

protected:
    // pointer to the Backend Factory, not owned by this class
    IAbstractBackendFactory* m_pBackendFactory;

    #ifdef OCL_DEV_BACKEND_PLUGINS
    mutable Intel::OpenCL::PluginManager m_pluginManager;
    #endif
};

}}}
