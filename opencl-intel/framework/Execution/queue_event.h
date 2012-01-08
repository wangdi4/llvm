// Copyright (c) 2008-2009 Intel Corporation
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

struct ocl_gpa_data;

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
		friend class Command;
		QueueEvent( IOclCommandQueueBase* cmdQueue, ocl_entry_points * pOclEntryPoints );

		IOclCommandQueueBase*   GetEventQueue() const                               { return m_pEventQueue;}
		void                    SetEventQueue(IOclCommandQueueBase* pQueue)         { m_pEventQueue = pQueue;}

		cl_context       GetContextHandle() const;
		cl_command_queue GetQueueHandle() const;
		cl_int           GetReturnCode() const;
		// OCLObject implementation
		cl_err_code GetInfo(cl_int iParamName, size_t szParamValueSize, void * paramValue, size_t * szParamValueSizeRet);

		//Override to notify my command about failed events it depended on
		virtual cl_err_code NotifyEventDone(OclEvent* pEvent, cl_int returnCode); 

		// profiling support
		cl_err_code GetProfilingInfo(cl_profiling_info clParamName, size_t szParamValueSize, void * pParamValue, size_t * pszParamValueSizeRet);
		void		SetProfilingInfo(cl_profiling_info clParamName, cl_ulong ulData);

		//Must override RemovePendency to prevent this aggregated object from being deleted
		virtual long RemovePendency(OCLObjectBase* obj);

		void                    SetCommand(Command* cmd)                            { m_pCommand = cmd;  }
		Command*                GetCommand() const                                  { return m_pCommand; }

		OclEventStateColor      SetColor(OclEventStateColor newColor); //returns the previous color


	protected:
		virtual ~QueueEvent();        

        //overrides from OclEvent
        virtual void   NotifyReady(OclEvent* pEvent); 
        virtual void   NotifyComplete(cl_int returnCode = CL_SUCCESS);

		SProfilingInfo			m_sProfilingInfo;
		bool					m_bProfilingEnabled;
		Command*				m_pCommand;             // Pointer to the command represented by this event
		IOclCommandQueueBase*   m_pEventQueue;          // Pointer to the queue that this event was enqueued on  
	
	private:
		ocl_gpa_data*           m_pGPAData;
	};

}}}    // Intel::OpenCL::Framework
