/////////////////////////////////////////////////////////////////////////
// ocl_shutdown.h:
/////////////////////////////////////////////////////////////////////////
// INTEL CONFIDENTIAL
// Copyright 2007-2008 Intel Corporation All Rights Reserved.
//
// The source code contained or described herein and all documents related
// to the source code ("Material") are owned by Intel Corporation or its
// suppliers or licensors. Title to the Material remains with Intel Corporation
// or its suppliers and licensors. The Material may contain trade secrets and
// proprietary and confidential information of Intel Corporation and its
// suppliers and licensors, and is protected by worldwide copyright and trade
// secret laws and treaty provisions. No part of the Material may be used, copied,
// reproduced, modified, published, uploaded, posted, transmitted, distributed,
// or disclosed in any way without Intel’s prior express written permission.
//
// No license under any patent, copyright, trade secret or other intellectual
// property right is granted to or conferred upon you by disclosure or delivery
// of the Materials, either expressly, by implication, inducement, estoppel or
// otherwise. Any license under such intellectual property rights must be express
// and approved by Intel in writing.
//
// Unless otherwise agreed by Intel in writing, you may not remove or alter this notice
// or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors
// in any way.
/////////////////////////////////////////////////////////////////////////

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


