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

File Name:  MICDeviceServiceFactory.cpp

\*****************************************************************************/

#include "MICDeviceServiceFactory.h"
#include "MICSerializationService.h"
#include "MICExecutionService.h"

namespace Intel { namespace OpenCL { namespace DeviceBackend {

MICDeviceServiceFactory* MICDeviceServiceFactory::s_pInstance = NULL;

MICDeviceServiceFactory::MICDeviceServiceFactory()
{}

MICDeviceServiceFactory::~MICDeviceServiceFactory()
{}

void MICDeviceServiceFactory::Init() 
{ 
    assert(!s_pInstance);
    s_pInstance = new MICDeviceServiceFactory(); 
}

void MICDeviceServiceFactory::Terminate() 
{
    if( NULL != s_pInstance)
    {
        delete s_pInstance;
        s_pInstance = NULL;
    }
}

ICLDevBackendServiceFactory* MICDeviceServiceFactory::GetInstance() 
{
    assert(s_pInstance);
    return s_pInstance; 
}

cl_dev_err_code MICDeviceServiceFactory::GetCompilationService( 
    const ICLDevBackendOptions* pBackendOptions,
    ICLDevBackendCompilationService** pBackendCompilationService)
{
    assert(false && "Compilation Service Not Supported in the Device");
    *pBackendCompilationService = NULL;
    return CL_DEV_ERROR_FAIL;
}

cl_dev_err_code MICDeviceServiceFactory::GetExecutionService(
    const ICLDevBackendOptions* pBackendOptions, 
    ICLDevBackendExecutionService** pBackendExecutionService)
{
    try
    {
        *pBackendExecutionService = new MICExecutionService();
    }
    catch( std::bad_alloc& )
    {
        return CL_DEV_OUT_OF_MEMORY; 
    }
    return CL_DEV_SUCCESS;
}

cl_dev_err_code MICDeviceServiceFactory::GetSerializationService(
    const ICLDevBackendOptions* pBackendOptions, 
    ICLDevBackendSerializationService** ppBackendSerializationService)
{
    try
    {
        *ppBackendSerializationService = new MICSerializationService(pBackendOptions);
    }
    catch( std::bad_alloc& )
    {
        return CL_DEV_OUT_OF_MEMORY; 
    }
    return CL_DEV_SUCCESS;
}

}}} // namespace
