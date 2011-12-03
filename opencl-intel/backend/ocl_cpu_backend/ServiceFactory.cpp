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

File Name:  ServiceFactory.cpp

\*****************************************************************************/

#include "ServiceFactory.h"
#include "BackendConfiguration.h"
#include "CPUCompileService.h"
#include "MICCompileService.h"
#include "MICSerializationService.h"
#include "ExecutionService.h"
#include "ImageCallbackServices.h"
#include "exceptions.h"
#include "CPUDetect.h"
#include "CPUExecutionService.h"

namespace Intel { namespace OpenCL { namespace DeviceBackend {

ServiceFactory* ServiceFactory::s_pInstance = NULL;

ServiceFactory::ServiceFactory()
{}

ServiceFactory::~ServiceFactory()
{}

void ServiceFactory::Init()
{
    assert(!s_pInstance);
    s_pInstance = new ServiceFactory();
}

void ServiceFactory::Terminate()
{
    if( NULL != s_pInstance)
    {
        delete s_pInstance;
        s_pInstance = NULL;
    }
}

ICLDevBackendServiceFactory* ServiceFactory::GetInstance()
{
    assert(s_pInstance);
    return s_pInstance;
}

cl_dev_err_code ServiceFactory::GetCompilationService( 
    const ICLDevBackendOptions* pBackendOptions,
    ICLDevBackendCompilationService** ppBackendCompilationService)
{
    try
    {
        // TODO: (later) need to remove these lines select operation mode should get the operation from the options
        CompilerConfiguration config(*BackendConfiguration::GetInstance()->GetCompilerConfig());
        config.ApplyRuntimeOptions(pBackendOptions);
        OPERATION_MODE mode = config.GetOperationMode();

        if(MIC_MODE == mode)
        {
            *ppBackendCompilationService = new MICCompileService(config);
            return CL_DEV_SUCCESS;
        }
        //if(CPU_MODE == mode)
        *ppBackendCompilationService = new CPUCompileService(config);
        return CL_DEV_SUCCESS;
    }
    catch( Exceptions::DeviceBackendExceptionBase& e )
    {
        return e.GetErrorCode();
    }
    catch( std::bad_alloc& )
    {
        return CL_DEV_OUT_OF_MEMORY; 
    }
    catch( std::runtime_error& )
    {
        return CL_DEV_ERROR_FAIL;
    }
}

cl_dev_err_code ServiceFactory::GetExecutionService(
    const ICLDevBackendOptions* pBackendOptions, 
    ICLDevBackendExecutionService** ppBackendExecutionService)
{
    try
    {
        // TODO: maybe need to remove these lines select operation mode should get the operation from the options
        CompilerConfiguration config(*BackendConfiguration::GetInstance()->GetCompilerConfig());
        config.ApplyRuntimeOptions(pBackendOptions);
        OPERATION_MODE mode = config.GetOperationMode();

        if(MIC_MODE == mode)
        {
            *ppBackendExecutionService = NULL;
            //assert(false && "Execution Service Not Implemented for MIC Device on Host Lib");
            return CL_DEV_ERROR_FAIL;
        }
        //if(CPU_MODE == mode)
        *ppBackendExecutionService = new CPUExecutionService();
        return CL_DEV_SUCCESS;
    }
    catch( Exceptions::DeviceBackendExceptionBase& e )
    {
        return e.GetErrorCode();
    }
    catch( std::bad_alloc& )
    {
        return CL_DEV_OUT_OF_MEMORY; 
    }
}

cl_dev_err_code ServiceFactory::GetSerializationService(
    const ICLDevBackendOptions* pBackendOptions, 
    ICLDevBackendSerializationService** pBackendSerializationService)
{
    try
    {
        // TODO: maybe need to remove these lines select operation mode should get the operation from the options
        CompilerConfiguration config(*BackendConfiguration::GetInstance()->GetCompilerConfig());
        config.ApplyRuntimeOptions(pBackendOptions);
        OPERATION_MODE mode = config.GetOperationMode();

        if(MIC_MODE == mode)
        {
            *pBackendSerializationService = new MICSerializationService(pBackendOptions);
            return CL_DEV_SUCCESS;
        }
        //if(CPU_MODE == mode)
        assert(false && "Serialization Service Not Implemented for CPU Device");
        return CL_DEV_ERROR_FAIL;
    }
    catch( Exceptions::DeviceBackendExceptionBase& e )
    {
        return e.GetErrorCode();
    }
    catch( std::bad_alloc& )
    {
        return CL_DEV_OUT_OF_MEMORY; 
    }
}

cl_dev_err_code ServiceFactory::GetImageService(
    const ICLDevBackendOptions* pBackendOptions, 
    ICLDevBackendImageService** ppBackendImageService)
{
    try
    {
        CompilerConfiguration config(*BackendConfiguration::GetInstance()->GetCompilerConfig());
        config.ApplyRuntimeOptions(pBackendOptions);
        /// WORKAROUND!! Wee need to skip built-in module load for
        /// Image compiler instance
        config.SkipBuiltins();
        OPERATION_MODE mode = config.GetOperationMode();
        *ppBackendImageService = new ImageCallbackService(config, mode == CPU_MODE);
        return CL_DEV_SUCCESS;
    }
    catch( Exceptions::DeviceBackendExceptionBase& e )
    {
        return e.GetErrorCode();
    }
    catch( std::bad_alloc& )
    {
        return CL_DEV_OUT_OF_MEMORY; 
    }
}

}}}