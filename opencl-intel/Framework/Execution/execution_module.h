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

// forward declrations

namespace Intel { namespace OpenCL { namespace Framework {
    // forward declrations
    class PlatformModule;
    class ContextModule;
    class OCLObjectsMap;
    class EventsManager;
    class OclCommandQueue;
    class Context;

    /**
     * ExecutionModule class the platform module responsible of all execution related
     * operations. this might include queues events etc.
     */

    ///////////////////////////////////////////////////////////////////////////////////
    // Class name:	ExecutionModule
    //
    // Description:	ExecutionModule class  responsible of all execution related
    //              operations. this include queues, events, enqueue calls etc.
    //              TODO: verify synchronization on access to all functions!!!
    //
    // Author:		Arnon Peleg
    // Date:		January 2009
    ///////////////////////////////////////////////////////////////////////////////////

    class ExecutionModule : IExecution
    {

    public:

        ExecutionModule( PlatformModule *pPlatformModule, ContextModule* pContextModule );
        virtual ~ExecutionModule();
        // Initialization is done right after the construction in order to capture errors on initilazations.
        cl_err_code Initialize();

        
        // Command Queues functions
        cl_command_queue    CreateCommandQueue( cl_context clContext, cl_device_id clDevice, cl_command_queue_properties clQueueProperties, cl_int* pErrRet );
        cl_err_code         RetainCommandQueue      ( cl_command_queue clCommandQueue);
        cl_err_code         ReleaseCommandQueue     ( cl_command_queue clCommandQueue);
        cl_err_code         GetCommandQueueInfo     ( cl_command_queue clCommandQueue, cl_command_queue_info clParamName, size_t szParamValueSize, void* pParamValue, size_t* pszParamValueSizeRet );
        cl_err_code         SetCommandQueueProperty ( cl_command_queue clCommandQueue, cl_command_queue_properties clProperties, cl_bool bEnable, cl_command_queue_properties* pclOldProperties);

        // Out Of Order Execution synch commands
        // ---------------------
        cl_err_code EnqueueMarker           (cl_command_queue clCommandQueue, cl_event *pEvent);
        cl_err_code EnqueueWaitForEvents    (cl_command_queue clCommandQueue, cl_uint uiNumEvents, const cl_event* cpEventList);
        cl_err_code EnqueueBarrier          (cl_command_queue clCommandQueue);


        // Not implemented yet queue commands:
        // -----------------
        // cl_int Flush (cl_command_queue command_queue);
        // cl_int Finish (cl_command_queue command_queue);

        // Event objects functions
        cl_err_code WaitForEvents( cl_uint uiNumEvents, const cl_event* cpEventList );
        cl_err_code GetEventInfo ( cl_event clEvent, cl_event_info clParamName, size_t szParamValueSize, void* pParamValue, size_t* pszParamValueSizeRet );
        cl_err_code RetainEvent  (cl_event clEevent);
        cl_err_code ReleaseEvent (cl_event clEvent);

        // Enqueue commands
        cl_err_code EnqueueReadBuffer   (cl_command_queue clCommandQueue, cl_mem clBuffer, cl_bool bBlocking, size_t szOffset, size_t szCb, void* pOutData, cl_uint uNumEventsInWaitList, const cl_event* cpEeventWaitList, cl_event* pEvent);
        cl_err_code EnqueueWriteBuffer  (cl_command_queue clCommandQueue, cl_mem clBuffer, cl_bool bBlocking, size_t szOffset, size_t szCb, const void* cpSrcData, cl_uint uNumEventsInWaitList, const cl_event* cpEeventWaitList, cl_event* pEvent);
        cl_err_code EnqueueCopyBuffer   (cl_command_queue clCommandQueue, cl_mem clSrcBuffer, cl_mem clDstBuffer, size_t szSrcOffset, size_t szDstOffset, size_t szCb, cl_uint uNumEventsInWaitList, const cl_event* cpEeventWaitList, cl_event* pEvent);
        void*       EnqueueMapBuffer    (cl_command_queue clCommandQueue, cl_mem clBuffer, cl_bool bBlockingMap, cl_map_flags clMapFlags, size_t szOffset, size_t szCb, cl_uint uNumEventsInWaitList, const cl_event* cpEeventWaitList, cl_event* pEvent, cl_int* pErrcodeRet);
        cl_err_code EnqueueUnmapMemObject(cl_command_queue clCommandQueue,cl_mem clMemObj, void* mappedPtr, cl_uint uNumEventsInWaitList, const cl_event* cpEeventWaitList, cl_event* pEvent);
        cl_err_code EnqueueNDRangeKernel(cl_command_queue clCommandQueue, cl_kernel clKernel, cl_uint uiWorkDim, const size_t* cpszGlobalWorkOffset, const size_t* cpszGlobalWorkSize, const size_t* cpszLocalWorkSize, cl_uint uNumEventsInWaitList, const cl_event* cpEeventWaitList, cl_event* pEvent);
        cl_err_code EnqueueTask         (cl_command_queue clCommandQueue, cl_kernel clKernel, cl_uint uNumEventsInWaitList, const cl_event* cpEeventWaitList, cl_event* pEvent);
        cl_err_code EnqueueNativeKernel (cl_command_queue clCommandQueue, void (*pUserFnc)(void *), void* pArgs, size_t szCbArgs, cl_uint uNumMemObjects, const cl_mem* clMemList, const void** ppArgsMemLoc, cl_uint uNumEventsInWaitList, const cl_event* cpEeventWaitList, cl_event* pEvent);

        // Enqueue commands that are not implemented yet
        //   ---------------------
        cl_err_code EnqueueReadImage(cl_command_queue command_queue, cl_mem image, cl_bool blocking_read, const size_t* origin[3], const size_t* region[3], size_t row_pitch, size_t slice_pitch, void* ptr, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* pEvent);
        cl_err_code EnqueueWriteImage(cl_command_queue command_queue, cl_mem image, cl_bool blocking_write, const size_t* origin[3], const size_t* region[3], size_t row_pitch, size_t slice_pitch, const void* ptr, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* pEvent);
        cl_err_code EnqueueCopyImage(cl_command_queue command_queue, cl_mem src_image, cl_mem dst_image, const size_t* src_origin[3], const size_t* dst_origin[3], const size_t* region[3], cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* pEvent);
        cl_err_code EnqueueCopyImageToBuffer(cl_command_queue command_queue, cl_mem src_image, cl_mem dst_buffer, const size_t* src_origin[3], const size_t* region[3], size_t dst_offset, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* pEvent);
        cl_err_code EnqueueCopyBufferToImage(cl_command_queue command_queue, cl_mem src_buffer, cl_mem dst_image, size_t src_offset, const size_t* dst_origin[3], const size_t* region[3], cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* pEvent);
        void*       EnqueueMapImage     (cl_command_queue clCommandQueue, cl_mem image, cl_bool blocking_map, cl_map_flags map_flags, const size_t* origin[3], const size_t* region[3], size_t* image_row_pitch, size_t* image_slice_pitch, cl_uint num_events_in_wait_list, const cl_event* event_wait_list, cl_event* pEvent, cl_int* errcode_ret);
        //

        cl_err_code         Release() { return CL_SUCCESS; }  // Release resources ???

    private:

        // Private functions
        OclCommandQueue*    GetCommandQueue(cl_command_queue clCommandQueue);

        // Input parameters validation commands
        cl_err_code         CheckCreateCommandQueueParams( cl_context clContext, cl_device_id clDevice, cl_command_queue_properties clQueueProperties, Context** ppContext );

        // Members
        PlatformModule*     m_pPlatfromModule;          // Pointer to the platfrom operation. This is the internal interface of the module.
        ContextModule*      m_pContextModule;           // Pointer to the context operation. This is the internal interface of the module.
        OCLObjectsMap*      m_pOclCommandQueueMap;      // Holds the set of active queues.
        EventsManager*      m_pEventsManager;           // Placeholder for all active events.
        Intel::OpenCL::Utils::LoggerClient* m_pLoggerClient; // Logger client for logging operations.
    };

}}};    // Intel::OpenCL::Framework
#endif  // !defined(__OCL_EXECUTION_MODULE_H__)
