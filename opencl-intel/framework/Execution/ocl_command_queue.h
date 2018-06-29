// INTEL CONFIDENTIAL
//
// Copyright 2008-2018 Intel Corporation.
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
#include <Logger.h>
#include "cl_object.h"
#include "queue_event.h"
#include "ocl_itt.h"

namespace Intel { namespace OpenCL { namespace Framework {

    // Forward declarations
    class Context;
    class Device;
    class FissionableDevice;
    class QueueWorkerThread;
    class EventsManager;
    class ICommandQueue;
    class Command;

    /**********************************************************************************************
     * Class name:    OclCommandQueue
     *
     * Description:    
     *
            The command-queue can be used to queue a set of operations (referred to as commands) in order. Having multiple
            command-queues allows applications to queue multiple independent commands without
            requiring synchronization. Note that this should work as long as these objects are not being
            shared. Sharing of objects across multiple command-queues will require the application to
            perform appropriate synchronization

     *
     * Author:        Arnon Peleg
     * Date:        December 2008
    **********************************************************************************************/
    class OclCommandQueue : public OCLObject<_cl_command_queue_int>
    {

    public:

        PREPARE_SHARED_PTR(OclCommandQueue)
        
        OclCommandQueue(
            const SharedPtr<Context>&   pContext,
            cl_device_id                clDefaultDeviceID, 
            cl_command_queue_properties clProperties,
            EventsManager*              pEventManager
            );
        virtual  cl_err_code     Initialize();
        cl_err_code        GetInfo(cl_int iParamName, size_t szParamValueSize, void * pParamValue, size_t * pszParamValueSizeRet) const;


        //These make little sense. Here for legacy support - deprecated in 1.1
        cl_bool         EnableProfiling( cl_bool bEnabled );
        virtual cl_bool EnableOutOfOrderExecMode( cl_bool bEnabled );

        cl_bool         IsPropertiesSupported( cl_command_queue_properties clProperties );
        cl_bool         IsProfilingEnabled() const              { return m_bProfilingEnabled;       }
        cl_bool         IsOutOfOrderExecModeEnabled() const     { return m_bOutOfOrderEnabled;      }
        cl_int          GetContextId() const;
        cl_device_id    GetQueueDeviceHandle() const            { return m_clDefaultDeviceHandle;   }
        const SharedPtr<FissionableDevice>&    GetDefaultDevice() const { return m_pDefaultDevice;            }
        EventsManager*  GetEventsManager() const                { return m_pEventsManager; }
        const SharedPtr<Context>& GetContext() const            { return m_pContext; } 
        SharedPtr<Context>&       GetContext()                  { return m_pContext; } 
        virtual cl_err_code CancelAll();
        virtual void    ReleaseQueue() { }    // called when the user calls clReleaesCommandQueue for this OclCommandQueue

        /**
         * @return the address of device agent's command list
         */
        void* GetDeviceCommandListPtr();

        // Create a custom tracker in GAP that correspond to the OCL queue
        cl_err_code GPA_InitializeQueue();
        ocl_gpa_queue * GPA_GetQueue() { return m_pOclGpaQueue; }
        cl_err_code GPA_ReleaseQueue();
        virtual ocl_gpa_data* GetGPAData() const;

    protected:

        OclCommandQueue(
            const SharedPtr<Context>&   pContext,
            cl_device_id                clDefaultDeviceID, 
            cl_command_queue_properties clProperties,
            EventsManager*              pEventManager,
            ocl_entry_points *            pOclEntryPoints
            );

        virtual         ~OclCommandQueue();

        virtual  void BecomeVisible() = 0;

        /**
         * @param iParamName    the parameter's name
         * @param pBuf            a buffer into which the parameter's information is to be written
         * @param szBuf            pBuf's size
         * @return the size of the data or 0 in case of error 
         */
        virtual size_t GetInfoInternal(cl_int iParamName, void* pBuf, size_t szBuf) const;

        SharedPtr<Context> m_pContext;
        SharedPtr<FissionableDevice> m_pDefaultDevice;
        EventsManager*      m_pEventsManager;
        cl_device_id        m_clDefaultDeviceHandle;
        cl_bool             m_bProfilingEnabled;
        cl_bool             m_bOutOfOrderEnabled;
        cl_dev_cmd_list     m_clDevCmdListId;

        ocl_gpa_queue*      m_pOclGpaQueue;
        ocl_gpa_data*        m_pGPAData;
        volatile bool        m_bCancelAll;
    };
}}}    // Intel::OpenCL::Framework
