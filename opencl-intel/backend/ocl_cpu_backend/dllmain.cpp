/*****************************************************************************\

Copyright (c) Intel Corporation (2010,2011).

    INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
    LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
    ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
    PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
    DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
    PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
    including liability for infringement of any proprietary rights, relating to
    use of the code. No license, express or implied, by estoppels or otherwise,
    to any intellectual property rights is granted herein.

File Name:  dllmain.cpp

\*****************************************************************************/

// dllmain.cpp : Defines the entry point for the DLL application.

#define DEVICE_BACKEND_EXPORTS

#include "cl_dev_backend_api.h"
#include "BackendConfiguration.h"
#include "ServiceFactory.h"
#include "CPUDeviceBackendFactory.h"
#include "MICDeviceBackendFactory.h"
#include "BuiltinModuleManager.h"
#include "Compiler.h"
#include "MICSerializationService.h"
#include "plugin_manager.h"
#include "llvm/System/Mutex.h"
#include "debuggingservicewrapper.h"


#if defined(_WIN32)
#include <windows.h>
#endif

using namespace Intel::OpenCL::DeviceBackend;

// lock used to prevent the simultaneous initialization
static llvm::sys::SmartMutex<true> s_init_lock;
// initialization count - used to prevent the multiple initialization
static int s_init_count = 0;
// initialization result
static cl_dev_err_code s_init_result = CL_DEV_SUCCESS;
// flag used to disable the termination sequence
static bool s_ignore_termination = false;


#if defined(_WIN32)

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        Compiler::Init();
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
        if( s_init_count > 0 )
        {
            // Dll is unloaded prior to Terminate call - in this case the TerminateDeviceBackend 
            // method should not attempts to free the resources, since the system
            // could be in non-stable state
            s_ignore_termination = true;
        }
        else
            Compiler::Terminate();

        break;
    }
    return TRUE;
}
#else
void __attribute__ ((constructor)) dll_init(void)
{
        Compiler::Init();
}
void __attribute__ ((destructor)) dll_fini(void)
{
        if( s_init_count > 0 )
        {
            s_ignore_termination = true;
        }
        else
        {
            Compiler::Terminate();
        }
}
#endif



// Defines the exported functions for the DLL application.
#ifdef __cplusplus
extern "C" 
{
#endif
    ///@brief
    /// 
    LLVM_BACKEND_API cl_dev_err_code  InitDeviceBackend(const ICLDevBackendOptions* pBackendOptions)
    {
        llvm::sys::SmartScopedLock<true> lock(s_init_lock);
        
        ++s_init_count;
        if( s_init_count > 1 )
        {
            //
            // Initialization was already completed - just return the result
            //
            return s_init_result;
        }

        try
        {
            BackendConfiguration::Init(pBackendOptions);
            ServiceFactory::Init();
            CPUDeviceBackendFactory::Init();
            MICDeviceBackendFactory::Init();
            BuiltinModuleManager::Init();
#ifdef OCL_DEV_BACKEND_PLUGINS
            PluginManager::Init();
#endif
            DefaultJITMemoryManager::Init();
            // Attempt to initialize the debug service. If debugging is 
            // disabled this is a no-op returning success.
            //
            if (CL_DEV_FAILED(DebuggingServiceWrapper::GetInstance().Init()))
                s_init_result = CL_DEV_ERROR_FAIL;
            else
                s_init_result = CL_DEV_SUCCESS;
        }
        catch( std::runtime_error& )
        {
            s_init_result = CL_DEV_ERROR_FAIL;
        }
        return s_init_result;

    }

    LLVM_BACKEND_API void TerminateDeviceBackend()
    {
        if( s_ignore_termination )
        {
            return;
        }

        llvm::sys::SmartScopedLock<true> lock(s_init_lock);
        //
        // Only perform the termination when initialization count drops to zero
        // 
        --s_init_count;
        assert( s_init_count >= 0 );
        if( s_init_count > 0 )
        {
            return;
        }

        DefaultJITMemoryManager::Terminate();
#ifdef OCL_DEV_BACKEND_PLUGINS
        PluginManager::Terminate();
#endif
        BuiltinModuleManager::Terminate();
        MICDeviceBackendFactory::Terminate();
        CPUDeviceBackendFactory::Terminate();
        DebuggingServiceWrapper::GetInstance().Terminate();
        ServiceFactory::Terminate();
        BackendConfiguration::Terminate();
    }

    LLVM_BACKEND_API ICLDevBackendServiceFactory* GetDeviceBackendFactory()
    {
        return ServiceFactory::GetInstance();
    }
#ifdef __cplusplus
}
#endif