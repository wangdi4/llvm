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
//  ocl_command_queue.cpp
//  Implementation of the Class OclCommandQueue
//  Created on:      23-Dec-2008 3:23:05 PM
//  Original author: Peleg, Arnon
///////////////////////////////////////////////////////////
#include "ocl_command_queue.h"
#include "Context.h"
#include "events_manager.h"
#include "enqueue_commands.h"
#include "ocl_event.h"
#include "in_order_command_queue.h"
#include "out_of_order_command_queue.h"
#include "Device.h"

#include <sstream>
// Debug
#include <assert.h>

using namespace Intel::OpenCL::Framework;
using namespace Intel::OpenCL::Utils;

/******************************************************************
 * Command queue constructor
 ******************************************************************/
OclCommandQueue::OclCommandQueue(
    Context*                    pContext, 
    cl_device_id                clDefaultDeviceID,
    cl_command_queue_properties clProperties,
    EventsManager*              pEventsManager,
	ocl_entry_points *			pOclEntryPoints
    ): OCLObject<_cl_command_queue_int>("OclCommandQueue"),
    m_pContext(pContext),
    m_pEventsManager(pEventsManager),
    m_clDefaultDeviceHandle(clDefaultDeviceID),
	m_clDevCmdListId(0)
{
    m_clContextHandle = m_pContext->GetHandle();
    m_pContext->GetDevice(clDefaultDeviceID, &m_pDefaultDevice);    
    // Set queue options
    m_bOutOfOrderEnabled = ((clProperties & CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE) ? true : false);
    m_bProfilingEnabled  = ((clProperties & CL_QUEUE_PROFILING_ENABLE) ? true : false );
	// Add dependency to context
	m_pContext->AddPendency(this);
    // Set logger

	INIT_LOGGER_CLIENT(L"OclCommandQueue Logger Client",LL_DEBUG);

	LOG_INFO(TEXT("OclCommandQueue created: 0x%X"), this);

	// Set GPA data 
	m_pGPAData = m_pContext->GetGPAData();

	m_handle.object   = this;
	m_handle.dispatch = (KHRicdVendorDispatch*)pOclEntryPoints;
}

/******************************************************************
 *
 ******************************************************************/
OclCommandQueue::~OclCommandQueue()
{   
    LOG_INFO(TEXT("OclCommandQueue delete: 0x%X"), this);

	if (0 != m_clDevCmdListId)
	{
		m_pDefaultDevice->GetDeviceAgent()->clDevReleaseCommandList(m_clDevCmdListId);
	}
	m_pContext->RemovePendency(this);
    m_pContext = NULL;
    m_pDefaultDevice = NULL;

    RELEASE_LOGGER_CLIENT;
}


/******************************************************************
 *
 ******************************************************************/
cl_err_code OclCommandQueue::GetInfo( cl_int iParamName, size_t szParamValueSize, void* pParamValue, size_t* pszParamValueSizeRet )
{
    cl_err_code res = CL_SUCCESS;
    void* localParamValue = NULL;
    size_t szOutputValueSize = 0;
    cl_command_queue_properties propreties;
    
    switch (iParamName)
    {
        case CL_QUEUE_CONTEXT:
            localParamValue = &m_clContextHandle;
            szOutputValueSize = sizeof(cl_command_queue);
            break;
        case CL_QUEUE_DEVICE:
            localParamValue = &m_clDefaultDeviceHandle;
            szOutputValueSize = sizeof(cl_device_id);
            break;
        case CL_QUEUE_REFERENCE_COUNT:
            localParamValue = &m_uiRefCount;
            szOutputValueSize = sizeof(cl_uint);
            break;
        case CL_QUEUE_PROPERTIES:
            {
            int iOutOfOrder  = (m_bOutOfOrderEnabled) ? 1 : 0;
            int iProfilingEn = (m_bProfilingEnabled)  ? 1 : 0;
            propreties = ((iOutOfOrder) | ( iProfilingEn<<1 ));
            localParamValue = &propreties;
            szOutputValueSize = sizeof(cl_command_queue_properties); 
            break;
            }
        default:
            res = CL_INVALID_VALUE;
            break;
    }

    // check param_value_size
    if ( (NULL != pParamValue) && (szParamValueSize < szOutputValueSize))
    {
        res = CL_INVALID_VALUE;
    }
    else
    {
        if ( NULL != pParamValue )
        {
            memcpy(pParamValue, localParamValue, szOutputValueSize);
        }
        if ( NULL != pszParamValueSizeRet )
        {
            *pszParamValueSizeRet = szOutputValueSize;
        }
    }
    return res;
}

/******************************************************************
 *
 ******************************************************************/
 cl_bool OclCommandQueue::EnableProfiling( cl_bool bEnabled )
 {
     // Profiling is not yet supported!!!
     // always return false
     return CL_FALSE;
 }

/******************************************************************
 * This has been deprecated, so support it vacuously for backwards compatibility
 ******************************************************************/
cl_bool OclCommandQueue::EnableOutOfOrderExecMode( cl_bool bEnabled )
{
    cl_err_code res = CL_SUCCESS;
	return res;
}

/******************************************************************
 *
 ******************************************************************/
 cl_bool OclCommandQueue::IsPropertiesSupported( cl_command_queue_properties clProperties )
 {
    // Get device info
    cl_command_queue_properties clDeviceProperties;
    cl_err_code res = m_pDefaultDevice->GetInfo(CL_DEVICE_QUEUE_PROPERTIES, sizeof(cl_command_queue_properties), &clDeviceProperties, NULL);
    if( CL_SUCCEEDED(res) )
    {
        if( clProperties == (clDeviceProperties & clProperties) )
        {
            // properties are supported
            return true;
        }
    }
    return false;
 }

 cl_err_code OclCommandQueue::Initialize()
 {
     cl_dev_subdevice_id subdevice_id = m_pContext->GetSubdeviceId(m_clDefaultDeviceHandle);
	 cl_dev_err_code retDev = m_pDefaultDevice->GetDeviceAgent()->clDevCreateCommandList(CL_DEV_LIST_NONE, subdevice_id, &m_clDevCmdListId);
	 if (CL_DEV_FAILED(retDev))
	 {
		 m_clDevCmdListId = 0;
		 return CL_OUT_OF_RESOURCES;
	 }

	 return CL_SUCCESS;
 }

 cl_int OclCommandQueue::GetContextId() const
 { 
	 return m_pContext->GetId();      
 }   

 cl_err_code OclCommandQueue::GPA_InitializeQueue()
 {
 #if defined(USE_GPA)
	 if ((NULL != m_pGPAData) && (m_pGPAData->bUseGPA) && (m_pGPAData->bEnableContextTracing))
	 {
		 m_pOclGpaQueue = new ocl_gpa_queue();
		 if (NULL == m_pOclGpaQueue)
		 {
			 return CL_OUT_OF_HOST_MEMORY;
		 }

		 std::stringstream ssQueueTrackName;
		 ssQueueTrackName << (m_bOutOfOrderEnabled ? "Out Of Order Queue (CPU)" : "In Order Queue (CPU)") << std::endl;
		 ssQueueTrackName << "Queue id: " << m_iId << std::endl;
		 ssQueueTrackName << "Queue handle: " << (int)&m_handle;
	  
  		 m_pOclGpaQueue->m_pStrHndl = __itt_string_handle_createA(ssQueueTrackName.str().c_str());

		 m_pOclGpaQueue->m_pTrack = __itt_track_create(m_pGPAData->pContextTrackGroup, m_pOclGpaQueue->m_pStrHndl, __itt_track_type_queue);
	 }
#endif
     return CL_SUCCESS;
 }
 cl_err_code OclCommandQueue::GPA_ReleaseQueue()
 {
#if defined(USE_GPA)
	 if ((NULL != m_pGPAData) && (m_pGPAData->bUseGPA) && (m_pGPAData->bEnableContextTracing))
	 {
		delete m_pOclGpaQueue;
	 }
#endif
     return CL_SUCCESS;
 }