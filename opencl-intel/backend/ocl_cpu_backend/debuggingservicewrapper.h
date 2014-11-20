/*****************************************************************************\

Copyright (c) Intel Corporation (2010-2014).

    INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
    LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
    ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
    PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
    DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
    PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
    including liability for infringement of any proprietary rights, relating to
    use of the code. No license, express or implied, by estoppels or otherwise,
    to any intellectual property rights is granted herein.

File Name:  debuggingservicewrapper.h

\*****************************************************************************/
#ifndef DEBUGGINGSERVICEWRAPPER_H
#define DEBUGGINGSERVICEWRAPPER_H

#include "icldebuggingservice.h"
#include "cl_device_api.h"


namespace Intel { namespace OpenCL { namespace DeviceBackend {


class DebuggingServiceWrapper
{
public:
    static DebuggingServiceWrapper& GetInstance() {return instance;}

    DebuggingServiceWrapper();

    // Initialize the service. This can return CL_DEV_SUCCESS also if the
    // DLL was not loaded voluntarily (due to a lacking environment var).
    // In this case all subsequent calls to GetDebuggingService will return
    // NULL and Terminate will be a no-op.
    //
    cl_dev_err_code Init();
    void Terminate();
    ICLDebuggingService* GetDebuggingService();

private:
    cl_dev_err_code LoadDll();
    void UnloadDll();

private:
    static DebuggingServiceWrapper instance;

    bool m_dll_loaded;

    DEBUGGING_SERVICE_INIT_FUNC m_init_func;
    DEBUGGING_SERVICE_TERMINATE_FUNC m_terminate_func;
    DEBUGGING_SERVICE_INSTANCE_FUNC m_instance_func;
};

}}}

#endif // DEBUGGINGSERVICEWRAPPER_H

