/*****************************************************************************\

Copyright (c) Intel Corporation (2011-2012).

    INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
    LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
    ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
    PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
    DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
    PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
    including liability for infringement of any proprietary rights, relating to
    use of the code. No license, express or implied, by estoppels or otherwise,
    to any intellectual property rights is granted herein.

File Name:  OpenCLBackendWrapper.h

\*****************************************************************************/
#ifndef OPENCL_BACKEND_WRAPPER_H
#define OPENCL_BACKEND_WRAPPER_H


#include "cl_device_api.h"
#include "cl_dev_backend_api.h"
#include "cl_dynamic_lib.h"

using namespace Intel::OpenCL::DeviceBackend;

namespace Intel{ namespace OpenCL { namespace CPUDevice 
{
    /**
     * @brief This class hides the internals of loading and calling OCL CPU Backend
     * 
     */  
    class OpenCLBackendWrapper
    {
    public:
        OpenCLBackendWrapper();
        /**
         * Calls the OCLCpuBackend Init function
         */
        cl_dev_err_code Init(const ICLDevBackendOptions* pBackendOptions);

        /**
         * Calls the OCLCpuBackend Terminate function
         */
        void Terminate();

        /**
         * Calls the OCLCpuBackend 'GetBackendFactor' method (returning the factory inteface)
         */
        ICLDevBackendServiceFactory* GetBackendFactory();


    private:
        /**
         * Explicitly loads the OCLCpuBackend dll
         */
        cl_dev_err_code LoadDll();

        /**
         * Explicityly unloads the OCLCpuBackend dll
         */
        void UnloadDll();

    private:
        Intel::OpenCL::Utils::OclDynamicLib m_dll;

        BACKEND_INIT_FUNCPTR       m_funcInit;
        BACKEND_TERMINATE_FUNCPTR  m_funcTerminate;
        BACKEND_GETFACTORY_FUNCPTR m_funcGetFactory;
    };
}}}

#endif // OPENCL_BACKEND_WRAPPER_H