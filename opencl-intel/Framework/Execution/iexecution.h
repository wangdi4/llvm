// Copyright (c) 2006-2007 Intel Corporation
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
//  iexecution.h
//  Defintion of the execution model interface.
//  Corresponds with OpenCL API 1.0 release.
//  Created on:      19-Jan-2009 
//  Original author: Peleg, Arnon
///////////////////////////////////////////////////////////////////////////////////////////////////

#if !defined(__OCL_IEXECUTION_H__)
#define __OCL_IEXECUTION_H__

#include <cl_types.h>

namespace Intel { namespace OpenCL { namespace Framework {

	///////////////////////////////////////////////////////////////////////////////////////////////////
    // Class name:	IExecution	
	// Description:	IExecution interface,
    //              This abstract class provide the execution interface as is in the OpenCL Spec
    //              Spec version: 1.0, Revision: 29, Official OpenCL 1.0 release from 12/8/08 
	// Author:		Peleg, Arnon
	// Date:		January 2009
	///////////////////////////////////////////////////////////////////////////////////////////////////

	class IExecution
	{
        /******************************************************************************************
		 * Function: 	CreateCommandQueue    
		 * Description:	creates a command-queue on a specific default device.
         *              
		 * Arguments:	clContext   [in] -  Must be a valid OpenCL context handle
         *              clDevice    [in] -  Must be a valid handle to device associated with context. 
         *              clQueueProperties [in] - Sets the properties of the queue with the following options:
         *                      CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE - 
         *                                  Determines whether the commands queued in the command-queue are executed in-order 
         *                                  or out-of order. If set, the commands in the command-queue are executed out-of-order. 
         *                                  Otherwise, commands are executed in-order.
         *                      CL_QUEUE_PROFILING_ENABLE - 
         *                                  Enable or disable profiling of commands in the command-queue. If set, the profiling
         *                                  of commands is enabled. Otherwise profiling of commands is disabled.
		 *				pErrRet     [out] - Return an appropriate error code. If errcode_ret is NULL, no error code is returned
         *
         * Return value:    On success returns a valid non-zero command-queue handle and errcode_ret is set to CL_SUCCESS.
         *                  Else, it returns a NULL value with one of the following error values returned in pErrRet:
         *                  CL_INVALID_CONTEXT      -   If context is not a valid context.
         *                  CL_INVALID_DEVICE       -   If device is not a valid device or is not associated with context.
         *                  CL_INVALID_VALUE        -   If values specified in properties are not valid.
         *                  CL_INVALID_QUEUE_PROPERTIES - If values specified in properties are valid but are not supported by the device.
         *                  CL_OUT_OF_HOST_MEMORY   -   If there is a failure to allocate resources required by 
         *                                              the OpenCL implementation on the host.
         *
		 * Author:		Arnon Peleg
		 * Date:		January 2009
		 ******************************************************************************************/        
        virtual cl_command_queue CreateCommandQueue(    cl_context                  IN  clContext,
                                                        cl_device_id                IN  clDevice,
                                                        cl_command_queue_properties IN  clQueueProperties,
                                                        cl_int*                     OUT pErrRet             ) = 0;
		/******************************************************************************************
		 * Function: 	RetainCommandQueue    
		 * Description:	increments the clCommandQueue reference count
         *              This function performs an implicit retain. 
		 * Arguments:	clCommandQueue [in] -	specifies the OpenCL commands queue being queried	
         *
		 * Return value:	CL_SUCCESS				 - If the function is executed successfully
		 *				    CL_INVALID_COMMAND_QUEUE - If clCommandQueue is not a valid command-queue.
         *
		 * Author:		Arnon Peleg
		 * Date:		January 2009
		 ******************************************************************************************/	
        virtual cl_err_code RetainCommandQueue (cl_command_queue IN clCommandQueue) = 0;

		/******************************************************************************************
		 * Function: 	ReleaseCommandQueue    
		 * Description:	decrements the command_queue reference count.
         *              After the clCommandQueue reference count becomes zero and all commands queued to
         *              clCommandQueue have finished (eg. kernel executions, memory object updates etc.), 
         *              the command-queue is deleted.
         *
		 * Arguments:	clCommandQueue [in] -	specifies the OpenCL commands queue being queried
         *
		 * Return value:	CL_SUCCESS				 - If the function is executed successfully
		 *				    CL_INVALID_COMMAND_QUEUE - If clCommandQueue is not a valid command-queue.
         *
		 * Author:		Arnon Peleg
		 * Date:		January 2009
		 ******************************************************************************************/	
        virtual cl_err_code ReleaseCommandQueue (cl_command_queue IN clCommandQueue) = 0;

        /******************************************************************************************
		 * Function: 	GetCommandQueueInfo    
		 * Description:	can be used to query information about a command-queue
         *
		 * Arguments:	clCommandQueue      [in]    - Specifies the command-queue being queried	
		 *				clParamName         [in]    - Specifies the information to query
		 *				szParamValueSize    [in]    - Is used to specify the size in bytes of memory pointed to 
         *                                            by szParamValueSize. This size must be >= size of return type as described 
         *                                            below. If param_value is NULL, it is ignored.
		 *				pParamValue         [out]   - A pointer to memory where the appropriate result being queried is returned. 
         *                                            If pParamValue is NULL, it is ignored.
		 *				pszParamValueSizeRet [out]
         *                                          - Returns the actual size in bytes of data being queried by pParamValue. 
         *                                            If param_value_size_ret is NULL, it is ignored.
         *  
		 * Return value: CL_SUCCESS				    - If the function is executed successfully
		 *				 CL_INVALID_COMMAND_QUEUE   - If clCommandQueue is not a valid command-queue
		 *				 CL_INVALID_VALUE		    - If clParamName is not one of the supported values or if size in
         *                                            bytes specified by szParamValueSize is < size of return type as 
         *                                            specified below and pParamValue is not a NULL value.
         * Supported clParamNames:
         *       CL_QUEUE_CONTEXT [cl_context]      - Return the context specified when the command-queue is created.
         *       CL_QUEUE_DEVICE  [cl_device_id]    - Return the device specified when the command-queue is created.
         *       CL_QUEUE_REFERENCE_COUNT [cl_uint] - Return the command-queue reference count.
         *       CL_QUEUE_PROPERTIES [cl_command_queue_properties] 
         *                                          - Return the currently specified properties properties are specified by the properties
		 * Author:		Arnon Peleg
		 * Date:		January 2009
		 ******************************************************************************************/	
        virtual cl_err_code GetCommandQueueInfo (   cl_command_queue        IN  clCommandQueue,
                                                    cl_command_queue_info   IN  clParamName,
                                                    size_t                  IN  szParamValueSize,
                                                    void*                   OUT pParamValue,
                                                    size_t*                 OUT pszParamValueSizeRet    ) = 0;

        /******************************************************************************************
		 * Function: 	SetCommandQueueProperty    
		 * Description:	Can be used to enable or disable the properties of a command-queue.
         *              Setting CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE command-queue property determines
         *              whether the commands in a clCommandQueue are executed in-order or out-of-order. 
         *              Changing this command-queue property will cause the OpenCL implementation to block 
         *              until all previously queued commands in command_queue have completed. This can be an expensive 
         *              operation and therefore this kind of change should be only done when absolutely necessary!
         *
		 * Arguments:	clCommandQueue [in]         - Specifies the command-queue being queried.
		 *				clProperties   [in]         - Specifies the new command-queue properties to be applied to clCommandQueue.
		 *				bEnable        [in]         - Determines whether the values specified by properties are enabled 
         *                                            (if enable is CL_TRUE) or disabled (if enable is CL_FALSE) for the commandQueue
		 *				pclOldProperties [out]      - Returns the command-queue properties before they were changed by this call. 
         *                                            If old_properties is NULL, it is ignored.			
         *  
		 * Return value: CL_SUCCESS				    - If the function is executed successfully
		 *				 CL_INVALID_COMMAND_QUEUE   - If clCommandQueue is not a valid clCommandQueue
		 *				 CL_INVALID_VALUE		    - If the values specified in clProperties are not valid
         *               CL_INVALID_QUEUE_PROPERTIES - If values specified in properties are not supported by the device.
         *
         * Author:		Arnon Peleg
		 * Date:		January 2009
		 ******************************************************************************************/	
        virtual cl_err_code SetCommandQueueProperty (   cl_command_queue                IN  clCommandQueue,
                                                        cl_command_queue_properties     IN  clProperties,
                                                        cl_bool                         IN  bEnable,
                                                        cl_command_queue_properties*    OUT pclOldProperties ) = 0;
        
        
        /******************************************************************************************
		 * Function: 	WaitForEvents    
		 * Description:	Waits on the host thread for commands identified by event objects in cpEventList to complete.
         *              A command is considered complete if its execution status is CL_COMPLETE or a negative value.
         *              The events specified in event_list act as synchronization points.
         *              All events in the list must belongs to the same context
         *
		 * Arguments:	uiNumEvents [in]        - Number of the events in the eveny list.
		 *				cpEventList [in]        - List of events handle that represents the commands to wait on for completion.
         *  
		 * Return value: CL_SUCCESS				- If the function is executed successfully
		 *				 CL_INVALID_VALUE		- If num_events is zero.
         *               CL_INVALID_CONTEXT     - If events specified in event_list do not belong to the same context
         *               CL_INVALID_EVENT       - If event objects specified in cpEventList are not valid event objects.
         *
         * Author:		Arnon Peleg
		 * Date:		January 2009
         ******************************************************************************************/	
        virtual cl_err_code WaitForEvents ( cl_uint         IN  uiNumEvents, 
                                            const cl_event* IN  cpEventList ) = 0;

        /******************************************************************************************
		 * Function: 	GetEventInfo    
		 * Description:	Returns information about the event object
         *
		 * Arguments:	clEvent             [in]    - Specifies the event object being queried	
		 *				clParamName         [in]    - Specifies the information to query. See supported names below.
		 *				szParamValueSize    [in]    - Is used to specify the size in bytes of memory pointed to 
         *                                            by szParamValueSize. This size must be >= size of return type as described 
         *                                            below. If pParamValue is NULL, it is ignored.
		 *				pParamValue         [out]   - A pointer to memory where the appropriate result being queried is returned. 
         *                                            If pParamValue is NULL, it is ignored.
		 *				pszParamValueSizeRet [out]  - Returns the actual size in bytes of data being queried by pParamValue. 
         *                                            If param_value_size_ret is NULL, it is ignored.
         *  
		 * Return value: CL_SUCCESS				    - If the function is executed successfully
		 *				 CL_INVALID_EVENT           - If clEvent is not a valid event object
		 *				 CL_INVALID_VALUE		    - If clParamName is not one of the supported values or if size in
         *                                            bytes specified by szParamValueSize is < size of return type as 
         *                                            specified below and pParamValue is not a NULL value.
         * Supported clParamNames:
         *  CL_EVENT_COMMAND_QUEUU [cl_command_queue]   - Return the command-queue associated with event.
         *  CL_EVENT_COMMAND_TYPE  [cl_command_type]    - Return the command associated with event.
         *  CL_EVENT_COMMAND_EXECUTION_STATUS [cl_int]  - Return the execution status of the command identified by event.
         *                                                Valid values are: CL_QUEUED, CL_SUBMITTED, CL_RUNNING, CL_COMPLETE,
         *                                                                  or an Error code given by a negative integer value. 
         *  CL_EVENT_REFERENCE_COUNT [cl_uint]          - Return the event reference count
         *
         * Author:		Arnon Peleg
		 * Date:		January 2009
         ******************************************************************************************/	
        virtual cl_err_code GetEventInfo (  cl_event      IN  clEvent,
                                            cl_event_info IN  clParamName,
                                            size_t        IN  szParamValueSize,
                                            void*         OUT pParamValue,
                                            size_t*       OUT pszParamValueSizeRet ) = 0;

		/******************************************************************************************
		 * Function: 	RetainEvent    
		 * Description:	Increments the event reference count
		 * Arguments:	clEvent [in] -	Specifies the event being queried	
         *
		 * Return value:	CL_SUCCESS		 - The function is executed successfully.
		 *				    CL_INVALID_EVENT - If clEvent is not a valid event object.
         *
		 * Author:		Arnon Peleg
		 * Date:		January 2009
		 ******************************************************************************************/	
        virtual cl_err_code RetainEvent (cl_event IN clEevent) = 0;

		/******************************************************************************************
		 * Function: 	ReleaseEvent    
		 * Description:	Decrements the event reference count.
         *              The event object is deleted once the reference count becomes zero, 
         *              the specific command identified by this event has completed (or terminated) and 
         *              there are no commands in the command-queues of a context that require a wait for this
         *              event to complete.
         *
		 * Arguments:	clEvent [in] -	specifies the event being queried
         *
		 * Return value:	CL_SUCCESS	     - If the function is executed successfully
		 *				    CL_INVALID_EVENT - If clEevent is not a valid event object.
         *
		 * Author:		Arnon Peleg
		 * Date:		January 2009
		 ******************************************************************************************/	
        virtual cl_err_code ReleaseEvent (cl_event IN clEvent) = 0;

        /******************************************************************************************
		 * Function: 	EnqueueReadBuffer    
		 * Description:	Enqueue commands to read from a buffer object to host memory.
         *              
		 * Arguments:	clCommandQueue  [in] -  Refers to the command-queue in which the read / write command will be
         *                                      queued. clCommandQueue and clBuffer must be created with the same OpenCL context.
         *              clBuffer        [in] -  Refers to a valid buffer object, the buffer to read from.
         *              bBlocking       [in] -  If bBlocking is CL_TRUE the command is blocking, the function does
         *                                      not return until the buffer data has been read and copied into memory pointed
         *                                      to by pOutData.
         *                                      If bBlocking is CL_FALSE the read command is non-blocking, The function queues
         *                                      a non-blocking read command and returns. 
         *                                      The contents of the buffer that pOutData points to cannot be used until the
         *                                      read command has completed. The event argument returns an event object which 
         *                                      can be used to query the execution status of the read command. When the read
         *                                      command has completed, the contents of the buffer that pOutData points to can
         *                                      be used by the application.
         *              szOffset        [in] -  The offset in bytes in the buffer object to read from.
         *              szCb            [in] -  The size in bytes of data that is being requested to be read.
         *              pOutData        [out]-  The pointer to buffer in host memory where data is to be read into.
		 *              uNumEventsInWaitList [in]   - Number of the events in the event wait list.
		 *				cpEeventWaitList[in] -  Specify events that need to complete before this particular command can
         *                                      be executed. The events specified in the list act as synchronization points
		 *				pEvent          [out]-  Returns an event object that identifies this particular command and can
         *                                      be used to query or queue a wait for this particular command to complete.
         *                                      Event can be NULL.
         *
         * Return value:    CL_SUCCESS              -   The function is executed successfully
         *                  CL_INVALID_COMMAND_QUEUE-   If clCommandQueue is not a valid command-queue.
         *                  CL_INVALID_CONTEXT      -   If the context associated with clCommandQueue and clBuffer are not the same.
         *                  CL_INVALID_MEM_OBJECT   -   If buffer is not a valid buffer object
         *                  CL_INVALID_EVENT_WAIT_LIST - if cpEeventWaitList is NULL and uNumEventsInWaitList > 0, 
         *                                              or cpEeventWaitList is not NULL and uNumEventsInWaitList is 0, 
         *                                              or if event objects in cpEeventWaitList are not valid events.
         *                  CL_OUT_OF_HOST_MEMORY   -   If there is a failure to allocate resources on the host
         *                  CL_INVALID_VALUE        -   If the region being read or written specified by (offset, cb) is out of
         *                                              bounds or if ptr is a NULL value.
         *
		 * Author:		Arnon Peleg
		 * Date:		January 2009
		 ******************************************************************************************/        
        virtual cl_err_code EnqueueReadBuffer ( cl_command_queue    IN  clCommandQueue, 
                                                cl_mem              IN  clBuffer,
                                                cl_bool             IN  bBlocking, 
                                                size_t              IN  szOffset,
                                                size_t              IN  szCb, 
                                                void*               OUT pOutData, 
                                                cl_uint             IN  uNumEventsInWaitList, 
                                                const cl_event*     IN  cpEeventWaitList, 
                                                cl_event*           OUT pEvent      )  = 0;

        /******************************************************************************************
		 * Function: 	EnqueueWriteBuffer    
		 * Description:	Enqueue commands to write to a buffer object from host memory.
         *              
		 * Arguments:	clCommandQueue  [in] -  Refers to the command-queue in which the read / write command will be
         *                                      queued. clCommandQueue and clBuffer must be created with the same OpenCL context.
         *              clBuffer        [in] -  Refers to a valid buffer object, the buffer to read from.
         *              bBlocking       [in] -  If bBlocking is CL_TRUE the command is blocking, the function copies the 
         *                                      data referred to by cpSrcData and enqueues the write operation in the clCommandQueue.
         *                                      The memory pointed to by cpSrcData can be reused after the call returns.
         *                                      If bBlocking is CL_FALSE the function will use cpSrcData to perform a nonblocking
         *                                      write and the function returns immediately. cpSrcData cannot be reused after 
         *                                      the call returns. The event argument returns an event object which can be used
         *                                      to query the execution status of the write command. When the write command has 
         *                                      completed, cpSrcData can then be reused.
         *              szOffset        [in] -  The offset in bytes in the buffer object to read from.
         *              szCb            [in] -  The size in bytes of data that is being requested to be read.
         *              cpSrcData       [out]-  The pointer to buffer in host memory where data is to be to be written from.
		 *              uNumEventsInWaitList [in]   - Number of the events in the event wait list.
		 *				cpEeventWaitList[in] -  Specify events that need to complete before this particular command can
         *                                      be executed. The events specified in the list act as synchronization points
		 *				pEvent          [out]-  Returns an event object that identifies this particular command and can
         *                                      be used to query or queue a wait for this particular command to complete.
         *                                      Event can be NULL.
         *
         * Return value:    CL_SUCCESS              -   The function is executed successfully
         *                  CL_INVALID_COMMAND_QUEUE-   If clCommandQueue is not a valid command-queue.
         *                  CL_INVALID_CONTEXT      -   If the context associated with clCommandQueue and clBuffer are not the same.
         *                  CL_INVALID_MEM_OBJECT   -   If buffer is not a valid buffer object
         *                  CL_INVALID_EVENT_WAIT_LIST - if cpEeventWaitList is NULL and uNumEventsInWaitList > 0, 
         *                                              or cpEeventWaitList is not NULL and uNumEventsInWaitList is 0, 
         *                                              or if event objects in cpEeventWaitList are not valid events.
         *                  CL_OUT_OF_HOST_MEMORY   -   If there is a failure to allocate resources on the host
         *
		 * Author:		Arnon Peleg
		 * Date:		January 2009
		 ******************************************************************************************/        
        virtual cl_err_code EnqueueWriteBuffer( cl_command_queue    IN  clCommandQueue, 
                                                cl_mem              IN  clBuffer,
                                                cl_bool             IN  bBlocking, 
                                                size_t              IN  szOffset,
                                                size_t              IN  szCb, 
                                                const void*         OUT cpSrcData, 
                                                cl_uint             IN  uNumEventsInWaitList, 
                                                const cl_event*     IN  cpEeventWaitList, 
                                                cl_event*           OUT pEvent      )  = 0;

        /******************************************************************************************
		 * Function: 	EnqueueNDRangeKernel    
		 * Description:	Enqueues a command to execute a kernel on a device.
         *              
		 * Arguments:	clCommandQueue      [in] - A valid command-queue. The kernel will be queued for execution on the
         *                                         device associated with clCommandQueue.
         *              clKernel            [in] - A valid kernel object. The  context associated with kernel and clCommandQueue
         *                                         must be the same.
         *              uiWorkDim           [in] - The number of dimensions used to specify the global work-items and work-items
         *                                         in the work-group. 
         *                                         uiWorkDim must be greater than zero and less than or equal to three
         *              cpszGlobalWorkOffset[in] - Must currently be a NULL value.
         *              cpszGlobalWorkSize  [in] - Points to an array of work_dim unsigned values that describe the number of
         *                                         global work-items in work_dim dimensions that will execute the kernel function
         *              cpszLocalWorkSize   [out]- Points to an array of work_dim unsigned values that describe the number of
         *                                         work-items that make up a work-group (also referred to as the size of the 
         *                                         work-group) that will execute the kernel specified by kernel.
         *                                         cpszLocalWorkSize can also be a NULL value in which case the function will
         *                                         determine how to be break the global work-items into appropriate work-group
         *                                         instances.
		 *              uNumEventsInWaitList[in] - Number of the events in the event wait list.
		 *				cpEeventWaitList    [in] - Specify events that need to complete before this particular command can
         *                                         be executed. The events specified in the list act as synchronization points.
		 *				pEvent              [out]- Returns an event object that identifies this particular command and can
         *                                         be used to query or queue a wait for this particular command to complete.
         *                                         Event can be NULL.
         *
         * Return value:    CL_SUCCESS              -   The function is executed successfully
         *                  CL_INVALID_PROGRAM_EXECUTABLE - If there is no successfully built program executable available 
         *                                              for device associated with clCommandQueue
         *                  CL_INVALID_COMMAND_QUEUE-   If clCommandQueue is not a valid command-queue.
         *                  CL_INVALID_KERNEL       -   If kernel is not a valid kernel object.
         *                  CL_INVALID_KERNEL_ARGS  -   If the kernel argument values have not been specified.
         *                  CL_INVALID_WORK_DIMENSION - If work_dim is not a valid value (not a value between 1 and 3).
         *                  CL_INVALID_WORK_GROUP_SIZE- if cpszLocalWorkSize is specified and number of workitems
         *                                              specified by cpszGlobalWorkSize is not evenly divisable by size of
         *                                              work-group given by local_work_size.
         *                  CL_INVALID_WORK_ITEM_SIZE-  If the number of work-items specified in any of
         *                                              local_work_size[0], … local_work_size[work_dim – 1] is greater 
         *                                              than the corresponding values specified by 
         *                                              CL_DEVICE_MAX_WORK_ITEM_SIZES[0], ….CL_DEVICE_MAX_WORK_ITEM_SIZES[work_dim – 1].
         *                  CL_INVALID_GLOBAL_OFFSET    - If cpszGlobalWorkOffset is not NULL
         *                  CL_OUT_OF_RESOURCES     -   If there is a failure to queue the execution instance of kernel on the 
         *                                              clCommandQueue because of insufficient resources needed to execute the kernel.                  
         *                  CL_MEM_OBJECT_ALLOCATION_FAILURE - If there is a failure to allocate memory for image or buffer 
         *                                              objects specified as arguments to kernel
         *                  CL_INVALID_EVENT_WAIT_LIST- If cpEeventWaitList is NULL and uNumEventsInWaitList > 0, 
         *                                              or cpEeventWaitList is not NULL and uNumEventsInWaitList is 0, 
         *                                              or if event objects in cpEeventWaitList are not valid events.
         *                  CL_OUT_OF_HOST_MEMORY   -   If there is a failure to allocate resources on the host
         *
		 * Author:		Arnon Peleg
		 * Date:		January 2009
		 ******************************************************************************************/            
        virtual cl_err_code EnqueueNDRangeKernel (  cl_command_queue    IN  clCommandQueue, 
                                                    cl_kernel           IN  clKernel,
                                                    cl_uint             IN  uiWorkDim,
                                                    const size_t*       IN  cpszGlobalWorkOffset,
                                                    const size_t*       IN  cpszGlobalWorkSize, 
                                                    const size_t*       IN  cpszLocalWorkSize, 
                                                    cl_uint             IN  uNumEventsInWaitList, 
                                                    const cl_event*     IN  cpEeventWaitList,
                                                    cl_event*           OUT pEvent      ) = 0;


        // Out Of Order Execution support - Not implemented yet...
        // ---------------------
        // cl_int EnqueueMarker (cl_command_queue command_queue, cl_event *event);
        // cl_int EnqueueWaitForEvents (cl_command_queue command_queue, cl_uint num_events, const cl_event *event_list);
        // cl_int EnqueueBarrier (cl_command_queue command_queue);

        // Not implemented yet queue commands:
        // -----------------
        // cl_int Flush (cl_command_queue command_queue);
        // cl_int Finish (cl_command_queue command_queue);

        };

}}};    // Intel::OpenCL::Framework
#endif  // !defined(__OCL_IEXECUTION_H__)
