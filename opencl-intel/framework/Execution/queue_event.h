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
//  queue_event.h
//  Implementation of the Class QueueEvent
//  Created on:      23-Dec-2008 3:23:06 PM
//  Original author: Peleg, Arnon
///////////////////////////////////////////////////////////

#pragma once
#include <cl_types.h>
#include <cl_object.h>
#include <cl_synch_objects.h>
#include "queue_event.h"
#include "ocl_event.h"
#include "ocl_itt.h"

namespace Intel { namespace OpenCL { namespace Framework {

    class Command;


    /**********************************************************************************************
    * Class name:    OclEvent
    *
    * Description:    
    *      TODO
    *
    * Author:      Doron Singer
    * Date:        July 2010
    **********************************************************************************************/    
    class QueueEvent : public OclEvent
    {

    public:
        PREPARE_SHARED_PTR(QueueEvent)
        friend class Command;
        /**
         * @param ptr       pointer to a memory area to initialize the object at
         * @param cmdQueue  the command queue of this event
         * @return a new QueueEvent placed in address 'ptr'
         */
        static SharedPtr<QueueEvent> Allocate(void* ptr, SharedPtr<IOclCommandQueueBase> cmdQueue)
        {
            return ::new(ptr) QueueEvent(cmdQueue);
        }

        ~QueueEvent();

        SharedPtr<IOclCommandQueueBase>   GetEventQueue() const { return m_pEventQueue;}
        void SetEventQueue(SharedPtr<IOclCommandQueueBase> pQueue);

        cl_command_queue GetEventQueueHandle() const { return m_pEventQueueHandle; }

        cl_command_queue GetQueueHandle() const;
        cl_int           GetReturnCode() const;
        // OCLObject implementation
        cl_err_code GetInfo(cl_int iParamName, size_t szParamValueSize, void * paramValue, size_t * szParamValueSizeRet) const;

        //Override to notify my command about failed events it depended on
        virtual cl_err_code ObservedEventStateChanged(const SharedPtr<OclEvent>& pEvent, cl_int returnCode); 

        // profiling support
        cl_err_code GetProfilingInfo(cl_profiling_info clParamName, size_t szParamValueSize, void * pParamValue, size_t * pszParamValueSizeRet);
        void        SetProfilingInfo(cl_profiling_info clParamName, cl_ulong ulData);

        // include times from other command into me
        void        IncludeProfilingInfo( const ConstSharedPtr<QueueEvent>& other );

        void        SetCommand(Command* cmd) { m_pCommand = cmd;  }
        Command*    GetCommand() const                                  { return m_pCommand; }

        OclEventState           SetEventState(OclEventState newColor); //returns the previous color

        // return true is command was ever executed
        bool        EverIssuedToDevice() const { return m_bEverIssuedToDevice; }

        // Add notification to ITT library when event state change occurs
        void        AddProfilerMarker(const char* szMarkerName, int iMarkerMask);

#if defined(USE_ITT) && defined(USE_ITT_INTERNAL)
        // Override wait to track it in VTune. Need to track it here and not in parent class as
        // parent class does not contain required data and what we need now is only a Queue Wait.
        // Calls to parent Wait to do the real work.
        virtual void    Wait();
#endif

        virtual void Cleanup(bool bIsTerminate = false);

    protected:
        QueueEvent( SharedPtr<IOclCommandQueueBase> cmdQueue);

        //overrides from OclEvent
        virtual void   DoneWithDependencies(const SharedPtr<OclEvent>& pEvent);
        virtual void   NotifyComplete(cl_int returnCode = CL_SUCCESS);

        SProfilingInfo          m_sProfilingInfo;
        bool                    m_bProfilingEnabled;
        bool                    m_bCommandQueuedValid;
        bool                    m_bCommandSubmitValid;
        bool                    m_bCommandStartValid;
        bool                    m_bCommandEndValid;
        Command*                m_pCommand;                 // Pointer to the command represented by this event
        SharedPtr<IOclCommandQueueBase>   m_pEventQueue;    // Pointer to the queue that this event was enqueued on
        cl_command_queue        m_pEventQueueHandle;        // A cached copy of m_pEventQueue's handle to use in QueueEvent::GetInfo
    
    private:

        bool                    m_bEverIssuedToDevice;

        ocl_gpa_data*           m_pGPAData;
    #if defined(USE_ITT)
        __itt_id                m_ittID;
    #endif

    };

}}}    // Intel::OpenCL::Framework
