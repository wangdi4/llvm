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

// dllmain.cpp : Defines the entry point for the DLL application.

#define DEVICE_BACKEND_EXPORTS

#include "cl_dev_backend_api.h"
#include "BackendConfiguration.h"
#include "ServiceFactory.h"
#include "CPUDeviceBackendFactory.h"
#include "BuiltinModuleManager.h"
#include "ImageCallbackManager.h"
#include "Compiler.h"
#include "llvm/Support/Mutex.h"
#include "debuggingservicewrapper.h"
#include "CPUDetect.h"
#include "cl_shutdown.h"
#include "ocl_mutex.h"

#if defined(_WIN32)
#include <windows.h>
#endif

using namespace Intel::OpenCL::DeviceBackend;

// lock used to prevent the simultaneous initialization
static OclMutex s_init_lock;
// initialization count - used to prevent the multiple initialization
static int s_init_count = 0;
static bool s_compiler_initialized = false;
// initialization result
static cl_dev_err_code s_init_result = CL_DEV_SUCCESS;

// flag used to disable the termination sequence
bool s_ignore_termination = false;

// include shutdown protocol support for runtime
USE_SHUTDOWN_HANDLER(nullptr);

#if defined(_WIN32)

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        // FIXME: Calling this from dll_init on Linux can cause problems
        //        because the constructors of static objects in other files
        //        haven't necessarily been called before we get there.  By
        //        extension, it seems like a bad idea to call it from here
        //        on Windows.  While it may work, the behavior would be
        //        inconsistent.
        // Compiler::Init();
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

        if( !s_ignore_termination)
        {
            Compiler::Terminate();
        }
        break;
    }
    return TRUE;
}
#else
void __attribute__ ((constructor)) dll_init(void)
{
  // FIXME: Calling this from here can cause problems because the constructors
  //        of static objects in other files haven't necessarily been called
  //        before we get here.
  //Compiler::Init();
}

void __attribute__ ((destructor)) dll_fini(void)
{
    if( s_init_count > 0 )
    {
        s_ignore_termination = true;
    }

    if( !s_ignore_termination)
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
        OclAutoMutex lock(&s_init_lock);

        // The compiler can only be initialized once, even if the backend is
        //   terminated.  The s_init_count check is not sufficient.
        if (!s_compiler_initialized)
        {
            Compiler::Init();
            s_compiler_initialized = true;
        }

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
            BackendConfiguration::Init();
            Compiler::InitGlobalState( BackendConfiguration::GetInstance().GetGlobalCompilerConfig(pBackendOptions));
            ServiceFactory::Init();
            CPUDeviceBackendFactory::Init();
            BuiltinModuleManager::Init();
            ImageCallbackManager::Init();
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

        OclAutoMutex lock(&s_init_lock);
        //
        // Only perform the termination when initialization count drops to zero
        //
        --s_init_count;
        assert( s_init_count >= 0 );
        if( s_init_count > 0 )
        {
            return;
        }

        BuiltinModuleManager::Terminate();
        ImageCallbackManager::Terminate();
        CPUDeviceBackendFactory::Terminate();
        DebuggingServiceWrapper::GetInstance().Terminate();
        ServiceFactory::Terminate();
        BackendConfiguration::Terminate();

        Intel::OpenCL::DeviceBackend::Utils::CPUDetect::Release();
    }

    LLVM_BACKEND_API ICLDevBackendServiceFactory* GetDeviceBackendFactory()
    {
        return ServiceFactory::GetInstance();
    }
#ifdef __cplusplus
}
#endif
