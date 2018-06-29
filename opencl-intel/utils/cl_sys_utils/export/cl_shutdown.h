// INTEL CONFIDENTIAL
//
// Copyright 2007-2018 Intel Corporation.
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

#pragma once
#include <cl_types.h>

namespace Intel { namespace OpenCL { namespace Utils {

    typedef enum _AT_EXIT_GLB_PROCESSING
    {
        AT_EXIT_GLB_PROCESSING_STARTED, // global shutdown started
        AT_EXIT_GLB_PROCESSING_DONE     // global shutdown done
    } AT_EXIT_GLB_PROCESSING;

    typedef enum _AT_EXIT_UNLOADING_MODE
    {
        AT_EXIT_DLL_UNLOADING_MODE,
        AT_EXIT_PROCESS_UNLOADING_MODE
    } AT_EXIT_UNLOADING_MODE;

    typedef void (CL_CALLBACK *at_exit_dll_callback_fn)( AT_EXIT_GLB_PROCESSING processing_state, AT_EXIT_UNLOADING_MODE mode );

    class IAtExitCentralPoint
    {
    public:
        virtual void RegisterDllCallback( at_exit_dll_callback_fn fn ) = 0;
        virtual void UnregisterDllCallback( at_exit_dll_callback_fn fn ) = 0;
        virtual void SetDllUnloadingState( bool value ) = 0;
        virtual bool isDllUnloadingState() const = 0;
        virtual void AtExitTrigger( at_exit_dll_callback_fn fn ) = 0;
    };
    
    class UseShutdownHandler
    {
    public:
        typedef void (*at_exit_local_callback_fn)();
        enum PROCESS_STATE
        {
            WORKING = 0,        // still no atexit() happened
            EXIT_STARTED,       // global atexit() processing started
            EXIT_DONE           // global atexit() processing finished
        };
        
        static volatile PROCESS_STATE shutdown_mode;

        static void RegisterGlobalAtExitNotification( IAtExitCentralPoint* fn );

        // notify global callback that some DLL is going to unload/done unloading
        static void UnloadingDll( bool value );       

        // special support (HACK) to ensure we get atexit callback before not our dynamically loaded DLLs
        static void ReRegisterAtExit();

    private:
        static IAtExitCentralPoint*      global_at_exit_callback;
        static at_exit_local_callback_fn local_at_exit_callback;

        static void OS_atexit();
        static void CL_CALLBACK AtExitProcessingState( AT_EXIT_GLB_PROCESSING processing_state,  AT_EXIT_UNLOADING_MODE mode );

    public:
        // do not use it except locally;
        UseShutdownHandler( at_exit_local_callback_fn local_callback );
    };

    // Return TRUE if global shutdown finished 
    inline bool IsShutdownMode()    { return (UseShutdownHandler::EXIT_DONE == UseShutdownHandler::shutdown_mode); }

    // Return TRUE if global shutdown started or finished
    inline bool IsShuttingDown()    { return (UseShutdownHandler::shutdown_mode > UseShutdownHandler::WORKING); }    

    // call this function directly only if renaming is required (ex. when DLL is not loaded manually)
    extern "C" void RegisterGlobalAtExitNotification( IAtExitCentralPoint* fn );

}}}

#define USE_SHUTDOWN_HANDLER( local_dll_callback ) Intel::OpenCL::Utils::UseShutdownHandler use_shutdown_handler( local_dll_callback )


