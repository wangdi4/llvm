/*****************************************************************************\

Copyright (c) Intel Corporation (2010-2012).

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
#include "debuggingservicewrapper.h"

/**
 * Description of ENABLE_SDE mode for MIC:
 * In general we have dynamic library which compiles and execute the code for 
 * the CPU, but in MIC we have the compilation functionality done in this 
 * dynamic lib and the execution functionality in another static lib (executor)
 * which will be loaded in the device.
 *
 * The main reason for this seperation is because of that Compilation done in
 * the HOST (CPU regular x86 arch), and the execution will be done in the 
 * device (MIC native).
 *
 * In SDE we don't have the device but we still need to simulate the real flow. 
 * The solution is to enable "execution" for MIC JIT also in the dynamic 
 * library without need to simulate the executor (we will simulate the JIT 
 * execution only), this will work since SDE work as follows:
 *
 *   - if you see an instruction that the CPU can execute it:
 *       execute it in the hardware
 *   - if you see an instruction can't be executed (aka MIC instruction):
 *       simulate it
 *
 * so for SDE our execution driver (executor) will be compiled for the CPU (x86) 
 * and we assume that we will use this path only under SDE so it's okay.
 */
#ifdef ENABLE_SDE
#include "MICExecutionService.h"
#endif



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

ICLDevBackendServiceFactoryInternal* ServiceFactory::GetInstanceInternal()
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
        OPERATION_MODE mode = CPU_MODE;
        if(NULL != pBackendOptions)
        {
            std::string cpuArch = pBackendOptions->GetStringValue((int)CL_DEV_BACKEND_OPTION_CPU_ARCH, "auto");
            mode = Utils::SelectOperationMode(cpuArch.c_str());
        }

        if(MIC_MODE == mode)
        {
            MICCompilerConfiguration config(BackendConfiguration::GetInstance()->GetMICCompilerConfig());
            config.ApplyRuntimeOptions(pBackendOptions);
            *ppBackendCompilationService = new MICCompileService(config);
            return CL_DEV_SUCCESS;
        }
        //if(CPU_MODE == mode)
        CompilerConfiguration config(BackendConfiguration::GetInstance()->GetCPUCompilerConfig());
        config.ApplyRuntimeOptions(pBackendOptions);
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
        OPERATION_MODE mode = CPU_MODE;
        if(NULL != pBackendOptions)
        {
            std::string cpuArch = pBackendOptions->GetStringValue((int)CL_DEV_BACKEND_OPTION_CPU_ARCH, "auto");
            mode = Utils::SelectOperationMode(cpuArch.c_str());
        }

        if(MIC_MODE == mode)
        {
        #ifdef ENABLE_SDE
            *ppBackendExecutionService = new MICExecutionService();
            return CL_DEV_SUCCESS;
        #else 
            *ppBackendExecutionService = NULL;
            //assert(false && "Execution Service Not Implemented for MIC Device on Host Lib");
            return CL_DEV_ERROR_FAIL;
        #endif
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
        OPERATION_MODE mode = CPU_MODE;
        if(NULL != pBackendOptions)
        {
            std::string cpuArch = pBackendOptions->GetStringValue((int)CL_DEV_BACKEND_OPTION_CPU_ARCH, "auto");
            mode = Utils::SelectOperationMode(cpuArch.c_str());
        }

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

cl_dev_err_code ServiceFactory::GetDebuggingService(
    ICLDebuggingService** pDebuggingService)
{
    ICLDebuggingService* instance = 
        DebuggingServiceWrapper::GetInstance().GetDebuggingService();
    *pDebuggingService = instance;
    return instance == NULL ? CL_DEV_ERROR_FAIL : CL_DEV_SUCCESS;
}
cl_dev_err_code ServiceFactory::GetImageService(
    const ICLDevBackendOptions* pBackendOptions, 
    ICLDevBackendImageService** ppBackendImageService)
{
    try
    {
        OPERATION_MODE mode = CPU_MODE;
        if(NULL != pBackendOptions)
        {
            std::string cpuArch = pBackendOptions->GetStringValue((int)CL_DEV_BACKEND_OPTION_CPU_ARCH, "auto");
            mode = Utils::SelectOperationMode(cpuArch.c_str());
        }

        CompilerConfiguration config(BackendConfiguration::GetInstance()->GetCPUCompilerConfig());
        config.ApplyRuntimeOptions(pBackendOptions);
        /// WORKAROUND!! Wee need to skip built-in module load for
        /// Image compiler instance
        config.SkipBuiltins();

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