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

#include "BackendConfiguration.h"
#include "CPUCompileService.h"
#include "CPUDetect.h"
#include "CPUExecutionService.h"
#include "ImageCallbackServices.h"
#include "ServiceFactory.h"
#include "debuggingservicewrapper.h"
#include "exceptions.h"
#if defined(INCLUDE_MIC_DEVICE)
#include "MICCompileService.h"
#include "MICSerializationService.h"
#endif

/**
 * Description of ENABLE_SDE mode for MIC:
 * In general we have dynamic library which compiles and execute the code for
 * the CPU, but in MIC we have the compilation functionality done in this
 * dynamic lib and the execution functionality in another static lib (executor)
 * which will be loaded in the device.
 *
 * The main reason for this separation is because of that Compilation done in
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
#if defined(ENABLE_SDE) && defined(INCLUDE_MIC_DEVICE)
#include "MICExecutionService.h"
#endif


namespace Intel { namespace OpenCL { namespace DeviceBackend {

using Utils::CPUDetect;

const char* CPU_DEVICE = "cpu";
const char* MIC_DEVICE = "mic";

namespace Utils
{
    DEVICE_TYPE SelectDevice(const char* cpuArch)
    {
        if(0 == strcmp(cpuArch, CPU_DEVICE)) return CPU_MODE;
        if(0 == strcmp(cpuArch, MIC_DEVICE)) return MIC_MODE;

        throw Exceptions::DeviceBackendExceptionBase("Unsupported device", CL_DEV_INVALID_OPERATION_MODE);
    }
}

ServiceFactory* ServiceFactory::s_pInstance = nullptr;

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
    if( nullptr != s_pInstance)
    {
        delete s_pInstance;
        s_pInstance = nullptr;
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
        if(nullptr == ppBackendCompilationService)
        {
            return CL_DEV_INVALID_VALUE;
        }

        // TODO: (later) need to remove these lines select operation mode should get the operation from the options
        DEVICE_TYPE mode = CPU_MODE;
        if(nullptr != pBackendOptions)
        {
            std::string device = pBackendOptions->GetStringValue((int)CL_DEV_BACKEND_OPTION_DEVICE, CPU_DEVICE);
            mode = Utils::SelectDevice(device.c_str());
        }
#if defined(INCLUDE_MIC_DEVICE)
        if(MIC_MODE == mode)
        {
            MICCompilerConfig config( BackendConfiguration::GetInstance().GetMICCompilerConfig(pBackendOptions));
            *ppBackendCompilationService = new MICCompileService(config);
            return CL_DEV_SUCCESS;
        }
#endif
        //if(CPU_MODE == mode)
        CompilerConfig config(BackendConfiguration::GetInstance().GetCPUCompilerConfig(pBackendOptions));
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
        if(nullptr == ppBackendExecutionService)
        {
            return CL_DEV_INVALID_VALUE;
        }

        // TODO: maybe need to remove these lines select operation mode should get the operation from the options
        DEVICE_TYPE mode = CPU_MODE;
        if(nullptr != pBackendOptions)
        {
            std::string device = pBackendOptions->GetStringValue((int)CL_DEV_BACKEND_OPTION_DEVICE, CPU_DEVICE);
            mode = Utils::SelectDevice(device.c_str());
        }

        if(MIC_MODE == mode)
        {
        #if defined(ENABLE_SDE) && defined(INCLUDE_MIC_DEVICE)
            std::string cpuArch = pBackendOptions->GetStringValue((int)CL_DEV_BACKEND_OPTION_SUBDEVICE, "auto");
            Intel::ECPU cpu = Intel::CPUId::GetCPUByName(cpuArch.c_str());
            Intel::CPUId cpuId(cpu, Intel::CFS_NONE, true);
            assert(cpuId.HasGatherScatter() && "MIC mode chosen but CPU ID is not right");
            *ppBackendExecutionService = new MICExecutionService(pBackendOptions, cpuId);
            return CL_DEV_SUCCESS;
        #else
            *ppBackendExecutionService = nullptr;
            //assert(false && "Execution Service Not Implemented for MIC Device on Host Lib");
            return CL_DEV_NOT_SUPPORTED;
        #endif
        }
        //if(CPU_MODE == mode)
        *ppBackendExecutionService = new CPUExecutionService(pBackendOptions);
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
        if(nullptr == pBackendSerializationService)
        {
            return CL_DEV_INVALID_VALUE;
        }

        // TODO: maybe need to remove these lines select operation mode should get the operation from the options
        DEVICE_TYPE mode = CPU_MODE;
        if(nullptr != pBackendOptions)
        {
            std::string device = pBackendOptions->GetStringValue((int)CL_DEV_BACKEND_OPTION_DEVICE, CPU_DEVICE);
            mode = Utils::SelectDevice(device.c_str());
        }
#if defined(INCLUDE_MIC_DEVICE)
        if(MIC_MODE == mode)
        {
            *pBackendSerializationService = new MICSerializationService(pBackendOptions);
            return CL_DEV_SUCCESS;
        }
#endif
        //if(CPU_MODE == mode)
        throw Exceptions::DeviceBackendExceptionBase("Serialization Service Not Implemented for CPU Device", CL_DEV_INVALID_OPERATION_MODE);
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
    return instance == nullptr ? CL_DEV_ERROR_FAIL : CL_DEV_SUCCESS;
}

cl_dev_err_code ServiceFactory::GetImageService(
    const ICLDevBackendOptions* pBackendOptions,
    ICLDevBackendImageService** ppBackendImageService)
{
    try
    {
        //TODO:vlad
        DEVICE_TYPE mode = CPU_MODE;

        if(nullptr != pBackendOptions)
        {
            std::string device = pBackendOptions->GetStringValue((int)CL_DEV_BACKEND_OPTION_DEVICE, CPU_DEVICE);
            mode = Utils::SelectDevice(device.c_str());
        }

        /// WORKAROUND!! Wee need to skip built-in module load for
        /// Image compiler instance
        CompilerConfig config(BackendConfiguration::GetInstance().GetCPUCompilerConfig(pBackendOptions));
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
