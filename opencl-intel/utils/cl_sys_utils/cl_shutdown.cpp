// Copyright (c) 2006-2012 Intel Corporation
// All rights reserved.
//
// WARRANTY DISCLAIMER
//
// THESE MATERIALS ARE PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL INTEL OR ITS
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THESE
// MATERIALS, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Intel Corporation is the author of the Materials, and requests that all
// problem reports or change requests be submitted to it directly

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
void CL_CALLBACK UseShutdownHandler::AtExitProcessingState( AT_EXIT_GLB_PROCESSING processing_state,  AT_EXIT_UNLOADING_MODE mode )
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
#ifdef _WIN32
                    if (AT_EXIT_DLL_UNLOADING_MODE == mode)
#endif
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

