// Copyright (c) 2008-2012 Intel Corporation
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

///////////////////////////////////////////////////////////
//  ocl_command_queue.h
//  Implementation of the Class OclCommandQueue
//  Created on:      23-Dec-2008 3:23:05 PM
//  Original author: Peleg, Arnon
///////////////////////////////////////////////////////////

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
