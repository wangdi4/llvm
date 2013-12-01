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

#pragma once
///////////////////////////////////////////////////////////////////////////////////////////////////
//  FrameworkProxy.h
//  Implementation of the Class FrameworkProxy
//  Created on:      10-Dec-2008 8:45:02 AM
//  Original author: ulevy
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "cl_types.h"
#include <platform_module.h>
#include <context_module.h>
#include <execution_module.h>
#include <task_executor.h>
#include "Logger.h"
#include "ocl_config.h"
#include "cl_synch_objects.h"
#include "ocl_itt.h"
#include "cl_thread.h"
#include "cl_shutdown.h"
#include <icd_dispatch.h>
#include <set>

namespace Intel { namespace OpenCL { namespace Framework {

    /**********************************************************************************************
    * Class name:    FrameworkProxy
    *
    * Description:    the framework proxy class design to pass the OpenCL api calls to the 
    *                framework's modules
    * Author:        Uri Levy
    * Date:            December 2008
    **********************************************************************************************/
    class FrameworkProxy : public Intel::OpenCL::Utils::IAtExitCentralPoint
    {
    private:
        enum GLOBAL_STATE {
            WORKING = 0,
            TERMINATING,
            TERMINATED
        };

    public:
        
        /******************************************************************************************
        * Function:     Instance
        * Description:    Get the instance of the framework proxy module.
        * Arguments:        
        * Return value:    instance to the framework factory
        * Author:        Uri Levy
        * Date:            December 2008
        ******************************************************************************************/
        static    FrameworkProxy * Instance();

        static void Destroy();

        /******************************************************************************************
        * Function:     GetContextModule
        * Description:    Get handle to the context module
        * Arguments:        
        * Return value:    pointer to the ContextModule class. NULL if context module wasn't
        *                initialized successfully
        * Author:        Uri Levy
        * Date:            December 2008
        ******************************************************************************************/
        ContextModule * GetContextModule() const { return m_pContextModule; }
        
        /******************************************************************************************
        * Function:     GetExecutionModule
        * Description:    Get handle to the execution module
        * Arguments:        
        * Return value:    pointer to the ExecutionModule class. NULL if module wasn't initialized
        *                successfully
        * Author:        Uri Levy
        * Date:            December 2008
        ******************************************************************************************/
        ExecutionModule * GetExecutionModule() const { return m_pExecutionModule; }
        
        /******************************************************************************************
        * Function:     GetPlatformModule
        * Description:    Get handle to the platform module
        * Arguments:        
        * Return value:    pointer to the PlatformModule class. NULL if module wasn't initialized
        *                successfully
        * Author:        Uri Levy
        * Date:            December 2008
        ******************************************************************************************/
        PlatformModule * GetPlatformModule() const { return m_pPlatformModule; }

           /******************************************************************************************
        * Function:     Activate
        * Description:    Simple TaskExecutor Interface for Framework
        * Arguments:        
        * Return value:    false on error
        * Author:        
        * Date:            
        ******************************************************************************************/
        bool            ActivateTaskExecutor() const;

           /******************************************************************************************
        * Function:     Deactivate
        * Description:    Simple TaskExecutor Interface for Framework
        * Arguments:        
        * Return value:    
        * Author:        
        * Date:            
        ******************************************************************************************/
        void            DeactivateTaskExecutor() const;

           /******************************************************************************************
        * Function:     CancelAllTasks()
        * Description:    Simple TaskExecutor Interface for Framework
        * Arguments:        
        * Return value:    
        * Author:        
        * Date:            
        ******************************************************************************************/
        void            CancelAllTasks(bool wait_for_finish) const;

        /******************************************************************************************
        * Function:     Execute task on TaskExecutor
        * Description:    Simple TaskExecutor Interface for Framework
        * Arguments:        
        * Return value:    false on error
        * Author:        
        * Date:            
        ******************************************************************************************/
        bool            Execute(const Intel::OpenCL::Utils::SharedPtr<Intel::OpenCL::TaskExecutor::ITaskBase>& pTask) const;

        /******************************************************************************************
        * Function:     ~FrameworkProxy
        * Description:    The FrameworkProxy class destructor
        * Arguments:        
        * Author:        Uri Levy
        * Date:            December 2008
        ******************************************************************************************/
        bool            API_Disabled() const { return (gGlobalState >= TERMINATING); }

        /******************************************************************************************
        * Manage process shutdown (IAtExitCentralPoint)
        ******************************************************************************************/
        void RegisterDllCallback( Intel::OpenCL::Utils::at_exit_dll_callback_fn fn );
        void UnregisterDllCallback( Intel::OpenCL::Utils::at_exit_dll_callback_fn fn );
        void SetDllUnloadingState( bool value ) { m_bIgnoreAtExit = value; }
        bool isDllUnloadingState() const        { return m_bIgnoreAtExit; }
        void AtExitTrigger( Intel::OpenCL::Utils::at_exit_dll_callback_fn cb );

    private:
        /******************************************************************************************
        * Function:     FrameworkProxy
        * Description:    The FrameworkProxy class constructor
        * Arguments:        
        * Author:        Uri Levy
        * Date:            December 2008
        ******************************************************************************************/
        FrameworkProxy();

        /******************************************************************************************
        * Function:     ~FrameworkProxy
        * Description:    The FrameworkProxy class destructor
        * Arguments:        
        * Author:        Uri Levy
        * Date:            December 2008
        ******************************************************************************************/
        virtual ~FrameworkProxy();
            
        void Initialize();
        void Release(bool bTerminate);
        
        // static instance of the framework factory class
        static  FrameworkProxy * m_pInstance;

        static KHRicdVendorDispatch        ICDDispatchTable;
        static SOCLCRTDispatchTable        CRTDispatchTable;
        static ocl_entry_points     OclEntryPoints;

        static void InitOCLEntryPoints();

        // handle to the platform module
        PlatformModule * m_pPlatformModule;
        
        // handle to the context module        
        ContextModule * m_pContextModule;

        // handle to the execution module
        ExecutionModule * m_pExecutionModule;

        // handle to the file log handler
        Intel::OpenCL::Utils::FileLogHandler * m_pFileLogHandler;
        
        // handle to the configuration object
        OCLConfig * m_pConfig;

        ocl_gpa_data m_GPAData;

        // handle to TaskExecutor
        // During shutdown task_executor dll may finish before current dll and destroy all internal objects
        // We can discover this case but we cannot access any task_executor object at that time point because
        // it may be already destroyed. As SharedPtr accesses the object itself to manage counters, we cannot use
        // SharedPointers at all.
        Intel::OpenCL::TaskExecutor::ITaskExecutor*         m_pTaskExecutor;
        mutable Intel::OpenCL::TaskExecutor::ITaskList*     m_pTaskList;
        mutable unsigned int    m_uiTEActivationCount;

        // a lock to prevent double initialization
        static Intel::OpenCL::Utils::OclSpinMutex m_initializationMutex;

        // Linux shutdown process
        static void CL_CALLBACK TerminateProcess();
        static volatile GLOBAL_STATE               gGlobalState;
        static std::set<Intel::OpenCL::Utils::at_exit_dll_callback_fn>   m_at_exit_cbs; // use m_initializationMutex
        static THREAD_LOCAL bool                   m_bIgnoreAtExit;

        // handle to the logger client
        DECLARE_LOGGER_CLIENT;

    };



}}}
