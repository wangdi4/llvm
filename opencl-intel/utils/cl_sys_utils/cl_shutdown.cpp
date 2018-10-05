// INTEL CONFIDENTIAL
//
// Copyright 2006-2018 Intel Corporation.
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

/****************************************************
 *  ocl_shutdown.cpp
 *  Implementation of the atexit() notification protocol

 Protocol:
    Stage 1: Registering
       - Each DLL exposes an exported function named "RegisterGlobalAtExitNotification". As DLL is loaded this function is called and passed
         a global at_exit callback function that must be called first when at_exit() occured. 
       - Each DLL should instantiate UseShutdownHandler class as a global object and pass it a local at_exit callback to be called after global one
       - As RegisterGlobalAtExitNotification() function is called it should call back global callback in the "register" mode providing internal function
         to be called during at_exit() processing. 

    Stage 2: Processing
        - if global callback is not NULL - call it in a "processing" mode.
            WINDOWS
            - global callback will disable all OpenCL entry points
            LINUX
            - global callback will call back  internal functions in all DLLs to notify start of processing. 
            - global callback should perform system shutdown
            - global callback will call back  internal functions in all DLLs to notify end of processing. If user recorder local shutdown function it
              should be called at that point to ensure no active threads in DLL.

 ****************************************************/

#include "cl_config.h"
#include "cl_shutdown.h"
#include "cl_types.h"
#include "cl_dynamic_lib.h"
#include "cl_sys_info.h"

#ifdef _WIN32
#include <windows.h>
#endif

using namespace Intel::OpenCL::Utils;

volatile UseShutdownHandler::PROCESS_STATE UseShutdownHandler::shutdown_mode = UseShutdownHandler::WORKING;

IAtExitCentralPoint* UseShutdownHandler::global_at_exit_callback = nullptr;
UseShutdownHandler::at_exit_local_callback_fn 
                                 UseShutdownHandler::local_at_exit_callback  = nullptr;

// this DLL name
//static char myDllName[MAX_PATH+1];

UseShutdownHandler::UseShutdownHandler( at_exit_local_callback_fn local_callback ) 
{
    local_at_exit_callback = local_callback;  
//    GetModulePathName( (void*)(ptrdiff_t)OS_atexit, myDllName, sizeof(myDllName) );
}

void UseShutdownHandler::ReRegisterAtExit()
{
    // do it in some later stage to get callback earlier
    atexit(OS_atexit);
}

void UseShutdownHandler::RegisterGlobalAtExitNotification( IAtExitCentralPoint* fn )
{
    if ( nullptr == fn )
    {
        return;
    }
    
    if (nullptr == global_at_exit_callback)
    {
        global_at_exit_callback   = fn;
        atexit(OS_atexit);
        OclDynamicLib::SetGlobalAtExitNotification( fn );
        global_at_exit_callback->RegisterDllCallback( AtExitProcessingState );
    }
    else
    {
        assert( fn == global_at_exit_callback );
    }
}
 
void UseShutdownHandler::UnloadingDll( bool value )
{
    if (nullptr != global_at_exit_callback)
    {
        global_at_exit_callback->SetDllUnloadingState( value );
    }
}

//
// This function is called from inside lock - do not call global callback from inside
//
void CL_CALLBACK UseShutdownHandler::AtExitProcessingState(
    AT_EXIT_GLB_PROCESSING processing_state, AT_EXIT_UNLOADING_MODE mode,
    bool needToDisableAPIAtShutdown )
{
    switch (processing_state)
    {
        case AT_EXIT_GLB_PROCESSING_STARTED:
            if (WORKING == shutdown_mode)
            {
                shutdown_mode           = EXIT_STARTED;
                global_at_exit_callback = nullptr;
                OclDynamicLib::SetGlobalAtExitNotification( nullptr );
            }
            return;

        case AT_EXIT_GLB_PROCESSING_DONE:
        default:
            if (EXIT_DONE != shutdown_mode)
            {
                if (nullptr != local_at_exit_callback)
                {
                    if (needToDisableAPIAtShutdown)
                    {
                        if (AT_EXIT_DLL_UNLOADING_MODE == mode)
                        {
                            local_at_exit_callback();
                        }
                    }
                    else
                    {
                        local_at_exit_callback();
                    }
                    local_at_exit_callback = nullptr;
                }
                shutdown_mode           = EXIT_DONE;
            }
            return;
    }
}

void UseShutdownHandler::OS_atexit()
{
    if ((nullptr != global_at_exit_callback) && (WORKING == shutdown_mode))
    {
        // now do the job
        global_at_exit_callback->AtExitTrigger( AtExitProcessingState );
    }
}

extern "C" 
#ifdef _WIN32
    __declspec( dllexport )
#endif    
void RegisterGlobalAtExitNotification( IAtExitCentralPoint* fn )
{
    UseShutdownHandler::RegisterGlobalAtExitNotification( fn );
}

