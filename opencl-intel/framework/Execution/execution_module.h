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

#include "cl_framework.h"
#include <Logger.h>
#include "iexecution.h"
#include "ocl_itt.h"
#include "ocl_config.h"
#include "command_queue.h"

// forward declarations

namespace Intel { namespace OpenCL { namespace Framework {
    // forward declarations
    class PlatformModule;
    class ContextModule;
    template <class HandleType, class ObjectType> class OCLObjectsMap;
    class EventsManager;
    class IOclCommandQueueBase;
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

    class ExecutionModule : public IExecution
    {

    public:

        ExecutionModule( PlatformModule *pPlatformModule, ContextModule* pContextModule );
        virtual ~ExecutionModule();

        // Initialization is done right after the construction in order to capture errors on initialization.
        cl_err_code Initialize(ocl_entry_points * pOclEntryPoints, OCLConfig * pOclConfig, ocl_gpa_data * pGPAData);


        // Command Queues functions
        cl_command_queue    CreateCommandQueue      ( cl_context clContext, cl_device_id clDevice, const cl_command_queue_properties* clQueueProperties, cl_int* pErrRet );
        cl_err_code         RetainCommandQueue      ( cl_command_queue clCommandQueue);
        cl_err_code         ReleaseCommandQueue     ( cl_command_queue clCommandQueue);
        cl_err_code         SetDefaultDeviceCommandQueue( cl_context context, cl_device_id device, cl_command_queue command_queue);
        cl_err_code         GetCommandQueueInfo     ( cl_command_queue clCommandQueue, cl_command_queue_info clParamName, size_t szParamValueSize, void* pParamValue, size_t* pszParamValueSizeRet );
        cl_err_code         SetCommandQueueProperty ( cl_command_queue clCommandQueue, cl_command_queue_properties clProperties, cl_bool bEnable, cl_command_queue_properties* pclOldProperties);
        cl_err_code         Flush                   ( cl_command_queue clCommandQueue);
        cl_err_code         Finish                  ( cl_command_queue clCommandQueue);
        SharedPtr<OclCommandQueue> GetCommandQueue(cl_command_queue clCommandQueue);

        // Out Of Order Execution synch commands
        // ---------------------
        cl_err_code EnqueueMarker           (cl_command_queue clCommandQueue, cl_event *pEvent, ApiLogger* pApiLogger);
        cl_err_code EnqueueMarkerWithWaitList(cl_command_queue clCommandQueue, cl_uint uiNumEvents, const cl_event* pEventList, cl_event* pEvent, ApiLogger* pApiLogger);
        cl_err_code EnqueueWaitForEvents    (cl_command_queue clCommandQueue, cl_uint uiNumEvents, const cl_event* cpEventList, ApiLogger* apiLogger);
        cl_err_code EnqueueBarrier          (cl_command_queue clCommandQueue, ApiLogger* pApiLogger);
        cl_err_code EnqueueBarrierWithWaitList(cl_command_queue clCommandQueue, cl_uint uiNumEvents, const cl_event* pEventList, cl_event* pEvent, ApiLogger* pApiLogger);


        // Event objects functions
        cl_err_code WaitForEvents           ( cl_uint uiNumEvents, const cl_event* cpEventList );
        cl_err_code GetEventInfo            ( cl_event clEvent, cl_event_info clParamName, size_t szParamValueSize, void* pParamValue, size_t* pszParamValueSizeRet );
        cl_err_code RetainEvent             ( cl_event clEevent);
        cl_err_code ReleaseEvent            ( cl_event clEvent);
		cl_event    CreateUserEvent         ( cl_context context, cl_int* errcode_ret);
		cl_int      SetUserEventStatus      ( cl_event evt, cl_int status);
		cl_err_code SetEventCallback        ( cl_event evt, cl_int status, void (CL_CALLBACK *fn)(cl_event, cl_int, void*), void* userData);

        // Enqueue commands
        cl_err_code EnqueueReadBuffer       (cl_command_queue clCommandQueue, cl_mem clBuffer, cl_bool bBlocking, size_t szOffset, size_t szCb, void* pOutData, cl_uint uNumEventsInWaitList, const cl_event* cpEeventWaitList, cl_event* pEvent, ApiLogger* apiLogger);		
        cl_err_code EnqueueWriteBuffer      (cl_command_queue clCommandQueue, cl_mem clBuffer, cl_bool bBlocking, size_t szOffset, size_t szCb, const void* cpSrcData, cl_uint uNumEventsInWaitList, const cl_event* cpEeventWaitList, cl_event* pEvent, ApiLogger* apiLogger);
		cl_err_code EnqueueCopyBuffer       (cl_command_queue clCommandQueue, cl_mem clSrcBuffer, cl_mem clDstBuffer, size_t szSrcOffset, size_t szDstOffset, size_t szCb, cl_uint uNumEventsInWaitList, const cl_event* cpEeventWaitList, cl_event* pEvent, ApiLogger* apiLogger);
		cl_err_code EnqueueFillBuffer       (cl_command_queue clCommandQueue, cl_mem clBuffer, const void *pattern, size_t pattern_size, size_t offset, size_t size, cl_uint num_events_in_wait_list, const cl_event *event_wait_list, cl_event *pEvent, ApiLogger* apiLogger);

		cl_err_code EnqueueReadBufferRect   (cl_command_queue clCommandQueue, cl_mem clBuffer, cl_bool bBlocking, const size_t szBufferOrigin[3], const size_t szHostOrigin[3], const size_t region[3], size_t buffer_row_pitch, size_t buffer_slice_pitch, size_t host_row_pitch, size_t host_slice_pitch, void* pOutData, cl_uint uNumEventsInWaitList, const cl_event* cpEeventWaitList, cl_event* pEvent, ApiLogger* apiLogger);
		cl_err_code EnqueueWriteBufferRect  (cl_command_queue clCommandQueue, cl_mem clBuffer, cl_bool bBlocking, const size_t szBufferOrigin[3], const size_t szHostOrigin[3], const size_t region[3], size_t buffer_row_pitch, size_t buffer_slice_pitch, size_t host_row_pitch, size_t host_slice_pitch, const void* pOutData, cl_uint uNumEventsInWaitList, const cl_event* cpEeventWaitList, cl_event* pEvent, ApiLogger* apiLogger);
		cl_err_code EnqueueCopyBufferRect   (cl_command_queue clCommandQueue, cl_mem clSrcBuffer, cl_mem clDstBuffer, const size_t szSrcBufferOrigin[3], const size_t szDstBufferOrigin[3], const size_t region[3], size_t src_buffer_row_pitch, size_t src_buffer_slice_pitch, size_t dst_buffer_row_pitch, size_t dst_buffer_slice_pitch, cl_uint uNumEventsInWaitList, const cl_event* cpEeventWaitList, cl_event* pEvent, ApiLogger* apiLogger);

        cl_err_code EnqueueReadImage        (cl_command_queue clCommandQueue, cl_mem clImage, cl_bool bBlocking, const size_t szOrigin[3], const size_t szRegion[3], size_t szRowPitch, size_t szSlicePitch, void* pOutData, cl_uint uNumEventsInWaitList, const cl_event* cpEeventWaitList, cl_event* pEvent, ApiLogger* apiLogger);
        cl_err_code EnqueueWriteImage       (cl_command_queue clCommandQueue, cl_mem clImage, cl_bool bBlocking, const size_t szOrigin[3], const size_t szRegion[3], size_t szRowPitch, size_t szSlicePitch, const void* cpSrcData, cl_uint uNumEventsInWaitList, const cl_event* cpEeventWaitList, cl_event* pEvent, ApiLogger* apiLogger);
        cl_err_code EnqueueCopyImage        (cl_command_queue clCommandQueue, cl_mem clSrcImage, cl_mem clDstImage, const size_t szSrcOrigin[3], const size_t szDstOrigin[3], const size_t szRegion[3], cl_uint uNumEventsInWaitList, const cl_event* cpEeventWaitList, cl_event* pEvent, ApiLogger* apiLogger);
        cl_err_code EnqueueFillImage        (cl_command_queue clCommandQueue, cl_mem clImage, const void *fillColor, const size_t *origin, const size_t *region, cl_uint num_events_in_wait_list, const cl_event *event_wait_list, cl_event *event, ApiLogger* apiLogger);

        cl_err_code EnqueueCopyImageToBuffer(cl_command_queue clCommandQueue, cl_mem clSrcImage, cl_mem clDstBuffer, const size_t szSrcOrigin[3], const size_t szRegion[3], size_t szDstOffset, cl_uint uNumEventsInWaitList, const cl_event* cpEeventWaitList, cl_event* pEvent, ApiLogger* apiLogger);
        cl_err_code EnqueueCopyBufferToImage(cl_command_queue clCommandQueue, cl_mem clSrcBuffer, cl_mem clDstImage, size_t szSrcOffset, const size_t szDstOrigin[3], const size_t szRegion[3], cl_uint uNumEventsInWaitList, const cl_event* cpEeventWaitList, cl_event* pEvent, ApiLogger* apiLogger);
        void*       EnqueueMapBuffer        (cl_command_queue clCommandQueue, cl_mem clBuffer, cl_bool bBlockingMap, cl_map_flags clMapFlags, size_t szOffset, size_t szCb, cl_uint uNumEventsInWaitList, const cl_event* cpEeventWaitList, cl_event* pEvent, cl_int* pErrcodeRet, ApiLogger* apiLogger);
        void*       EnqueueMapImage         (cl_command_queue clCommandQueue, cl_mem clImage, cl_bool bBlockingMap, cl_map_flags clMapFlags, const size_t szOrigin[3], const size_t szRegion[3], size_t* pszImageRowPitch, size_t* pszImageSlicePitch, cl_uint uNumEventsInWaitList, const cl_event* cpEeventWaitList, cl_event* pEvent, cl_int* pErrcodeRet, ApiLogger* apiLogger);
        cl_err_code EnqueueUnmapMemObject   (cl_command_queue clCommandQueue, cl_mem clMemObj, void* mappedPtr, cl_uint uNumEventsInWaitList, const cl_event* cpEeventWaitList, cl_event* pEvent, ApiLogger* apiLogger);
        cl_err_code EnqueueNDRangeKernel    (cl_command_queue clCommandQueue, cl_kernel clKernel, cl_uint uiWorkDim, const size_t* cpszGlobalWorkOffset, const size_t* cpszGlobalWorkSize, const size_t* cpszLocalWorkSize, cl_uint uNumEventsInWaitList, const cl_event* cpEeventWaitList, cl_event* pEvent, ApiLogger* apiLogger);
        cl_err_code EnqueueTask             (cl_command_queue clCommandQueue, cl_kernel clKernel, cl_uint uNumEventsInWaitList, const cl_event* cpEeventWaitList, cl_event* pEvent, ApiLogger* apiLogger);
        cl_err_code EnqueueNativeKernel     (cl_command_queue clCommandQueue, void (CL_CALLBACK*pUserFnc)(void *), void* pArgs, size_t szCbArgs, cl_uint uNumMemObjects, const cl_mem* clMemList, const void** ppArgsMemLoc, cl_uint uNumEventsInWaitList, const cl_event* cpEeventWaitList, cl_event* pEvent, ApiLogger* apiLogger);
        cl_err_code EnqueueMigrateMemObjects(cl_command_queue clCommandQueue, cl_uint uiNumMemObjects, const cl_mem* pMemObjects, cl_mem_migration_flags clFlags, cl_uint uiNumEventsInWaitList, const cl_event* pEventWaitList, cl_event* pEvent, ApiLogger* apiLogger);
        cl_err_code EnqueueSVMMigrateMem(cl_command_queue clCommandQueue, cl_uint num_svm_pointers, const void** svm_pointers, const size_t* sizes, cl_mem_migration_flags flags, cl_uint uiNumEventsInWaitList, const cl_event* pEventWaitList, cl_event* pEvent, ApiLogger* apiLogger);
        cl_err_code EnqueueUSMMemset(cl_command_queue command_queue,
                                     void* dst_ptr, cl_int value, size_t size,
                                     cl_uint num_events_in_wait_list,
                                     const cl_event* event_wait_list,
                                     cl_event* event, ApiLogger* api_logger);
        cl_err_code EnqueueUSMMemcpy(cl_command_queue command_queue,
                                     cl_bool blocking, void* dst_ptr,
                                     const void* src_ptr, size_t size,
                                     cl_uint num_events_in_wait_list,
                                     const cl_event* event_wait_list,
                                     cl_event* event, ApiLogger* api_logger);
        cl_err_code EnqueueUSMMigrateMem(cl_command_queue command_queue,
                                         const void* ptr, size_t size,
                                         cl_mem_migration_flags flags,
                                         cl_uint num_events_in_wait_list,
                                         const cl_event* event_wait_list,
                                         cl_event* event,
                                         ApiLogger* api_logger);
        cl_err_code EnqueueUSMMemAdvise(cl_command_queue command_queue,
                                        const void* ptr,
                                        size_t size,
                                        cl_mem_advice_intel advice,
                                        cl_uint num_events_in_wait_list,
                                        const cl_event* event_wait_list,
                                        cl_event* event,
                                        ApiLogger* api_logger);

        // Profiling
		cl_err_code GetEventProfilingInfo (cl_event clEvent, cl_profiling_info clParamName, size_t szParamValueSize, void * pParamValue, size_t * pszParamValueSizeRet);

        cl_err_code         Release(bool bTerminate);
        cl_err_code         Finish                  ( const SharedPtr<IOclCommandQueueBase>& pCommandQueue);
        void                DeleteAllActiveQueues( bool preserve_user_handles );

		cl_int EnqueueSVMFree(cl_command_queue clCommandQueue, cl_uint uiNumSvmPointers, void* pSvmPointers[],
							  void (CL_CALLBACK* pfnFreeFunc)(cl_command_queue queue, cl_uint uiNumSvmPointers, void* pSvmPointers[], void* pUserData),
                void* pUserData, cl_uint uiNumEventsInWaitList,	const cl_event* pEventWaitList,	cl_event* pEvent, ApiLogger* apiLogger);
		cl_int EnqueueSVMMemcpy(cl_command_queue clCommandQueue, cl_bool bBlockingCopy, void* pDstPtr, const void* pSrcPtr, size_t size, cl_uint uiNumEventsInWaitList,
								const cl_event* pEventWaitList, cl_event* pEvent, ApiLogger* apiLogger);
		cl_int EnqueueSVMMemFill(cl_command_queue clCommandQueue, void* pSvmPtr, const void* pPattern, size_t szPatternSize, size_t size, cl_uint uiNumEventsInWaitList,
								 const cl_event* pEventWaitList, cl_event* pEvent, ApiLogger* apiLogger);
		cl_int EnqueueSVMMap(cl_command_queue clCommandQueue, cl_bool bBlockingMap,	cl_map_flags mapflags, void* pSvmPtr, size_t size, cl_uint uiNumEventsInWaitList,
							 const cl_event* pEventWaitList, cl_event* pEvent, ApiLogger* apiLogger);
		cl_int EnqueueSVMUnmap(cl_command_queue clCommandQueue, void* pSvmPtr, cl_uint uiNumEventsInWaitList, const cl_event* pEventWaitList, cl_event* pEvent, ApiLogger* apiLogger);

        cl_err_code RunAutorunKernels(const SharedPtr<Program>& program,
            ApiLogger* apiLogger);

        EventsManager*      GetEventsManager() const { return m_pEventsManager; }
        void                ReleaseAllUserEvents( bool preserve_user_handles );

		ocl_entry_points *  GetDispatchTable() const {return m_pOclEntryPoints; }

		ocl_gpa_data *      GetGPAData() const { return m_pGPAData; }
    private:
	    // Private functions

		bool				IsValidQueueHandle(cl_command_queue clCommandQueue);

        // Input parameters validation commands
        cl_err_code         CheckCreateCommandQueueParams( cl_context clContext, cl_device_id clDevice, const cl_command_queue_properties* clQueueProperties, SharedPtr<Context>* ppContext,
														   cl_command_queue_properties& queueProps, cl_uint& uiQueueSize);
        cl_err_code         CheckImageFormats( SharedPtr<MemoryObject> pSrcImage, SharedPtr<MemoryObject> pDstImage);
        bool                CheckMemoryObjectOverlapping(SharedPtr<MemoryObject> pMemObj, const size_t* szSrcOrigin, const size_t* szDstOrigin, const size_t* szRegion);
        size_t              CalcRegionSizeInBytes(SharedPtr<MemoryObject> pImage, const size_t* szRegion);
        cl_err_code         FlushAllQueuesForContext(cl_context ctx);
        cl_err_code         EnqueueMarkerWithWaitList(const SharedPtr<IOclCommandQueueBase>& clCommandQueue, cl_uint uiNumEvents, const cl_event* pEventList, cl_event* pEvent, ApiLogger* pApiLogger);
        cl_err_code         EnqueueMarker(const SharedPtr<IOclCommandQueueBase>& clCommandQueue, cl_event *pEvent, ApiLogger* pApiLogger);

        PlatformModule*     m_pPlatfromModule;                                                  // Pointer to the platform operation. This is the internal interface of the module.
        ContextModule*      m_pContextModule;                                                   // Pointer to the context operation. This is the internal interface of the module.
        OCLObjectsMap<_cl_command_queue_int, _cl_context_int>*      m_pOclCommandQueueMap;      // Holds the set of active queues.
        EventsManager*      m_pEventsManager;                                                   // Placeholder for all active events.

        ocl_entry_points *	m_pOclEntryPoints;

        ocl_gpa_data *      m_pGPAData;

        OPENCL_VERSION      m_opencl_ver;

        DECLARE_LOGGER_CLIENT; // Logger client for logging operations.

        // Disable copy consructors
        ExecutionModule(const ExecutionModule&);
        ExecutionModule& operator=(const ExecutionModule&);
};

}}}    // Intel::OpenCL::Framework
