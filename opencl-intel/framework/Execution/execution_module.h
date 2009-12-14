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

#pragma once
///////////////////////////////////////////////////////////
//  execution_module.h
//  Implementation of the Class ExecutionModule
//  Created on:      23-Dec-2008 3:23:00 PM
//  Original author: Peleg, Arnon
///////////////////////////////////////////////////////////
#if !defined(__OCL_EXECUTION_MODULE_H__)
#define __OCL_EXECUTION_MODULE_H__

#include <cl_types.h>
#include <logger.h>
#include "iexecution.h"
#include "iexecution_gl.h"

// forward declrations

namespace Intel { namespace OpenCL { namespace Framework {
    // forward declrations
    class PlatformModule;
    class ContextModule;
    class OCLObjectsMap;
    class EventsManager;
    class OclCommandQueue;
    class Context;
    class MemoryObject;

    /**
     * ExecutionModule class the platform module responsible of all execution related
     * operations. this might include queues events etc.
     */

    ///////////////////////////////////////////////////////////////////////////////////
    // Class name:  ExecutionModule
    //
    // Description:    ExecutionModule class responsible of all execution related
    //              operations. this include queues, events, enqueue calls etc.
    //              TODO: verify synchronization on access to all functions!!!
    //
    // Author:      Arnon Peleg
    // Date:        January 2009
    ///////////////////////////////////////////////////////////////////////////////////

    class ExecutionModule : public IExecution, IExecutionGL
    {

    public:

        ExecutionModule( PlatformModule *pPlatformModule, ContextModule* pContextModule );
        virtual ~ExecutionModule();

        // Initialization is done right after the construction in order to capture errors on initilazations.
        cl_err_code Initialize(ocl_entry_points * pOclEntryPoints);

        
        // Command Queues functions
        cl_command_queue    CreateCommandQueue      ( cl_context clContext, cl_device_id clDevice, cl_command_queue_properties clQueueProperties, cl_int* pErrRet );
        cl_err_code         RetainCommandQueue      ( cl_command_queue clCommandQueue);
        cl_err_code         ReleaseCommandQueue     ( cl_command_queue clCommandQueue);
        cl_err_code         GetCommandQueueInfo     ( cl_command_queue clCommandQueue, cl_command_queue_info clParamName, size_t szParamValueSize, void* pParamValue, size_t* pszParamValueSizeRet );
        cl_err_code         SetCommandQueueProperty ( cl_command_queue clCommandQueue, cl_command_queue_properties clProperties, cl_bool bEnable, cl_command_queue_properties* pclOldProperties);
        cl_err_code         Flush                   ( cl_command_queue clCommandQueue);
        cl_err_code         Finish                  ( cl_command_queue clCommandQueue);

        // Out Of Order Execution synch commands
        // ---------------------
        cl_err_code EnqueueMarker           (cl_command_queue clCommandQueue, cl_event *pEvent);
        cl_err_code EnqueueWaitForEvents    (cl_command_queue clCommandQueue, cl_uint uiNumEvents, const cl_event* cpEventList);
        cl_err_code EnqueueBarrier          (cl_command_queue clCommandQueue);


        // Event objects functions
        cl_err_code WaitForEvents           ( cl_uint uiNumEvents, const cl_event* cpEventList );
        cl_err_code GetEventInfo            ( cl_event clEvent, cl_event_info clParamName, size_t szParamValueSize, void* pParamValue, size_t* pszParamValueSizeRet );
        cl_err_code RetainEvent             (cl_event clEevent);
        cl_err_code ReleaseEvent            (cl_event clEvent);

        // Enqueue commands
        cl_err_code EnqueueReadBuffer       (cl_command_queue clCommandQueue, cl_mem clBuffer, cl_bool bBlocking, size_t szOffset, size_t szCb, void* pOutData, cl_uint uNumEventsInWaitList, const cl_event* cpEeventWaitList, cl_event* pEvent);
        cl_err_code EnqueueWriteBuffer      (cl_command_queue clCommandQueue, cl_mem clBuffer, cl_bool bBlocking, size_t szOffset, size_t szCb, const void* cpSrcData, cl_uint uNumEventsInWaitList, const cl_event* cpEeventWaitList, cl_event* pEvent);
        cl_err_code EnqueueCopyBuffer       (cl_command_queue clCommandQueue, cl_mem clSrcBuffer, cl_mem clDstBuffer, size_t szSrcOffset, size_t szDstOffset, size_t szCb, cl_uint uNumEventsInWaitList, const cl_event* cpEeventWaitList, cl_event* pEvent);

        cl_err_code EnqueueReadImage        (cl_command_queue clCommandQueue, cl_mem clImage, cl_bool bBlocking, const size_t szOrigin[3], const size_t szRegion[3], size_t szRowPitch, size_t szSlicePitch, void* pOutData, cl_uint uNumEventsInWaitList, const cl_event* cpEeventWaitList, cl_event* pEvent);
        cl_err_code EnqueueWriteImage       (cl_command_queue clCommandQueue, cl_mem clImage, cl_bool bBlocking, const size_t szOrigin[3], const size_t szRegion[3], size_t szRowPitch, size_t szSlicePitch, const void* cpSrcData, cl_uint uNumEventsInWaitList, const cl_event* cpEeventWaitList, cl_event* pEvent);
        cl_err_code EnqueueCopyImage        (cl_command_queue clCommandQueue, cl_mem clSrcImage, cl_mem clDstImage, const size_t szSrcOrigin[3], const size_t szDstOrigin[3], const size_t szRegion[3], cl_uint uNumEventsInWaitList, const cl_event* cpEeventWaitList, cl_event* pEvent);
        cl_err_code EnqueueCopyImageToBuffer(cl_command_queue clCommandQueue, cl_mem clSrcImage, cl_mem clDstBuffer, const size_t szSrcOrigin[3], const size_t szRegion[3], size_t szDstOffset, cl_uint uNumEventsInWaitList, const cl_event* cpEeventWaitList, cl_event* pEvent);
        cl_err_code EnqueueCopyBufferToImage(cl_command_queue clCommandQueue, cl_mem clSrcBuffer, cl_mem clDstImage, size_t szSrcOffset, const size_t szDstOrigin[3], const size_t szRegion[3], cl_uint uNumEventsInWaitList, const cl_event* cpEeventWaitList, cl_event* pEvent);
        void*       EnqueueMapBuffer        (cl_command_queue clCommandQueue, cl_mem clBuffer, cl_bool bBlockingMap, cl_map_flags clMapFlags, size_t szOffset, size_t szCb, cl_uint uNumEventsInWaitList, const cl_event* cpEeventWaitList, cl_event* pEvent, cl_int* pErrcodeRet);
        void*       EnqueueMapImage         (cl_command_queue clCommandQueue, cl_mem clImage, cl_bool bBlockingMap, cl_map_flags clMapFlags, const size_t szOrigin[3], const size_t szRegion[3], size_t* pszImageRowPitch, size_t* pszImageSlicePitch, cl_uint uNumEventsInWaitList, const cl_event* cpEeventWaitList, cl_event* pEvent, cl_int* pErrcodeRet);
        cl_err_code EnqueueUnmapMemObject   (cl_command_queue clCommandQueue, cl_mem clMemObj, void* mappedPtr, cl_uint uNumEventsInWaitList, const cl_event* cpEeventWaitList, cl_event* pEvent);
        cl_err_code EnqueueNDRangeKernel    (cl_command_queue clCommandQueue, cl_kernel clKernel, cl_uint uiWorkDim, const size_t* cpszGlobalWorkOffset, const size_t* cpszGlobalWorkSize, const size_t* cpszLocalWorkSize, cl_uint uNumEventsInWaitList, const cl_event* cpEeventWaitList, cl_event* pEvent);
        cl_err_code EnqueueTask             (cl_command_queue clCommandQueue, cl_kernel clKernel, cl_uint uNumEventsInWaitList, const cl_event* cpEeventWaitList, cl_event* pEvent);
        cl_err_code EnqueueNativeKernel     (cl_command_queue clCommandQueue, void (*pUserFnc)(void *), void* pArgs, size_t szCbArgs, cl_uint uNumMemObjects, const cl_mem* clMemList, const void** ppArgsMemLoc, cl_uint uNumEventsInWaitList, const cl_event* cpEeventWaitList, cl_event* pEvent);

        // Profiling
		cl_err_code GetEventProfilingInfo (cl_event clEvent, cl_profiling_info clParamName, size_t szParamValueSize, void * pParamValue, size_t * pszParamValueSizeRet);

		// GL
		cl_err_code EnqueueAcquireGLObjects(cl_command_queue clCommandQueue, cl_uint uiNumObjects, const cl_mem * pclMemObjects, cl_uint uiNumEventsInWaitList, const cl_event * pclEventWaitList, cl_event * pclEvent);
		cl_err_code EnqueueReleaseGLObjects(cl_command_queue clCommandQueue, cl_uint uiNumObjects, const cl_mem * pclMemObjects, cl_uint uiNumEventsInWaitList, const cl_event * pclEventWaitList, cl_event * pclEvent);


        cl_err_code         Release() { return CL_SUCCESS; }  // Release resources ???

    private:

        // Private functions
        OclCommandQueue*    GetCommandQueue(cl_command_queue clCommandQueue);

        // Input parameters validation commands
        cl_err_code         CheckCreateCommandQueueParams( cl_context clContext, cl_device_id clDevice, cl_command_queue_properties clQueueProperties, Context** ppContext );
        cl_err_code         Check2DImageParameters( MemoryObject* pImage, const size_t szOrigin[3], const size_t szRegion[3]);
        cl_err_code         CheckImageFormat( MemoryObject* pSrcImage, MemoryObject* pDstImage);
        bool                CheckMemoryObjectOverlapping(MemoryObject* pMemObj, const size_t* szSrcOrigin, const size_t* szDstOrigin, const size_t* szRegion);
        size_t              CalcRegionSizeInBytes(MemoryObject* pImage, const size_t* szRegion);

        // Members
        PlatformModule*     m_pPlatfromModule;          // Pointer to the platfrom operation. This is the internal interface of the module.
        ContextModule*      m_pContextModule;           // Pointer to the context operation. This is the internal interface of the module.
        OCLObjectsMap*      m_pOclCommandQueueMap;      // Holds the set of active queues.
        EventsManager*      m_pEventsManager;           // Placeholder for all active events.
        
		ocl_entry_points *	m_pOclEntryPoints;

		DECLARE_LOGGER_CLIENT; // Logger client for logging operations.
    };

}}};    // Intel::OpenCL::Framework
#endif  // !defined(__OCL_EXECUTION_MODULE_H__)
