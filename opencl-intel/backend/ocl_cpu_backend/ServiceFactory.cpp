// INTEL CONFIDENTIAL
//
// Copyright 2010-2018 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

#include "BackendConfiguration.h"
#include "CPUCompileService.h"
#include "CPUDetect.h"
#include "CPUExecutionService.h"
#include "ImageCallbackServices.h"
#include "ServiceFactory.h"
#include "debuggingservicewrapper.h"
#include "exceptions.h"

namespace Intel { namespace OpenCL { namespace DeviceBackend {

using Utils::CPUDetect;

const char* CPU_DEVICE = "cpu";

namespace Utils
{
    DEVICE_TYPE SelectDevice(const char* cpuArch)
    {
        if(0 == strcmp(cpuArch, CPU_DEVICE)) return CPU_MODE;

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
