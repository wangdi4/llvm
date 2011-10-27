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

File Name:  main.cpp

\*****************************************************************************/

#include "cl_device_api.h"
#include "ICLDevBackendServiceFactory.h"
#include "ICLDevBackendOptions.h"
#include "MICDeviceServiceFactory.h"
#include "MICSerializationService.h"
#include <new>

namespace Intel { namespace OpenCL { namespace DeviceBackend {

extern "C" cl_dev_err_code InitDeviceBackend(const ICLDevBackendOptions* pBackendOptions)
{
    try
    {
        MICDeviceServiceFactory::Init();
        DefaultJITMemoryManager::Init();
    }
    catch( std::bad_alloc& )
    {
        return CL_DEV_OUT_OF_MEMORY;
    }
    return CL_DEV_SUCCESS;
}

extern "C" void TerminateDeviceBackend()
{
    MICDeviceServiceFactory::Terminate();
    DefaultJITMemoryManager::Terminate();
}

extern "C" ICLDevBackendServiceFactory* GetDeviceBackendFactory()
{
    return MICDeviceServiceFactory::GetInstance();
}

}}}

