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
    // Class name:    IExecution    
    // Description:    IExecution interface,
    //              This abstract class provide the execution interface as is in the OpenCL Spec
    //              Spec version: 1.0, Revision: 29, Official OpenCL 1.0 release from 12/8/08 
    // Author:        Peleg, Arnon
    // Date:        January 2009
    ///////////////////////////////////////////////////////////////////////////////////////////////////

    class IExecution
    {
    public:
        virtual ~IExecution(){};

        /******************************************************************************************
         * Function:     CreateCommandQueue    
         * Description:    creates a command-queue on a specific default device.
         *              
         * Arguments:    clContext   [in] -  Must be a valid OpenCL context handle
         *              clDevice    [in] -  Must be a valid handle to device associated with context. 
         *              clQueueProperties [in] - Sets the properties of the queue with the following options:
         *                      CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE - 
         *                                  Determines whether the commands queued in the command-queue are executed in-order 
         *                                  or out-of order. If set, the commands in the command-queue are executed out-of-order. 
         *                                  Otherwise, commands are executed in-order.
         *                      CL_QUEUE_PROFILING_ENABLE - 
         *                                  Enable or disable profiling of commands in the command-queue. If set, the profiling
         *                                  of commands is enabled. Otherwise profiling of commands is disabled.
         *                pErrRet     [out] - Return an appropriate error code. If errcode_ret is NULL, no error code is returned
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
         * Author:        Arnon Peleg
         * Date:        January 2009
         ******************************************************************************************/        
        virtual cl_command_queue CreateCommandQueue(    cl_context                  IN  clContext,
                                                        cl_device_id                IN  clDevice,
                                                        cl_command_queue_properties IN  clQueueProperties,
                                                        cl_int*                     OUT pErrRet             ) = 0;
        /******************************************************************************************
         * Function:     RetainCommandQueue    
         * Description:    increments the clCommandQueue reference count
         *              This function performs an implicit retain. 
         * Arguments:    clCommandQueue [in] -    specifies the OpenCL commands queue being queried    
         *
         * Return value:    CL_SUCCESS                 - If the function is executed successfully
         *                    CL_INVALID_COMMAND_QUEUE - If clCommandQueue is not a valid command-queue.
         *
         * Author:        Arnon Peleg
         * Date:        January 2009
         ******************************************************************************************/    
        virtual cl_err_code RetainCommandQueue (cl_command_queue IN clCommandQueue) = 0;

        /******************************************************************************************
         * Function:     ReleaseCommandQueue    
         * Description:    decrements the command_queue reference count.
         *              After the clCommandQueue reference count becomes zero and all commands queued to
         *              clCommandQueue have finished (eg. kernel executions, memory object updates etc.), 
         *              the command-queue is deleted.
         *
         * Arguments:    clCommandQueue [in] -    specifies the OpenCL commands queue being queried
         *
         * Return value:    CL_SUCCESS                 - If the function is executed successfully
         *                    CL_INVALID_COMMAND_QUEUE - If clCommandQueue is not a valid command-queue.
         *
         * Author:        Arnon Peleg
         * Date:        January 2009
         ******************************************************************************************/    
        virtual cl_err_code ReleaseCommandQueue (cl_command_queue IN clCommandQueue) = 0;

        /******************************************************************************************
         * Function:     GetCommandQueueInfo    
         * Description:    can be used to query information about a command-queue
         *
         * Arguments:    clCommandQueue      [in]    - Specifies the command-queue being queried    
         *                clParamName         [in]    - Specifies the information to query
         *                szParamValueSize    [in]    - Is used to specify the size in bytes of memory pointed to 
         *                                            by szParamValueSize. This size must be >= size of return type as described 
         *                                            below. If param_value is NULL, it is ignored.
         *                pParamValue         [out]   - A pointer to memory where the appropriate result being queried is returned. 
         *                                            If pParamValue is NULL, it is ignored.
         *                pszParamValueSizeRet [out]
         *                                          - Returns the actual size in bytes of data being queried by pParamValue. 
         *                                            If param_value_size_ret is NULL, it is ignored.
         *  
         * Return value: CL_SUCCESS                    - If the function is executed successfully
         *                 CL_INVALID_COMMAND_QUEUE   - If clCommandQueue is not a valid command-queue
         *                 CL_INVALID_VALUE            - If clParamName is not one of the supported values or if size in
         *                                            bytes specified by szParamValueSize is < size of return type as 
         *                                            specified below and pParamValue is not a NULL value.
         * Supported clParamNames:
         *       CL_QUEUE_CONTEXT [cl_context]      - Return the context specified when the command-queue is created.
         *       CL_QUEUE_DEVICE  [cl_device_id]    - Return the device specified when the command-queue is created.
         *       CL_QUEUE_REFERENCE_COUNT [cl_uint] - Return the command-queue reference count.
         *       CL_QUEUE_PROPERTIES [cl_command_queue_properties] 
         *                                          - Return the currently specified properties properties are specified by the properties
         * Author:        Arnon Peleg
         * Date:        January 2009
         ******************************************************************************************/    
        virtual cl_err_code GetCommandQueueInfo (   cl_command_queue        IN  clCommandQueue,
                                                    cl_command_queue_info   IN  clParamName,
                                                    size_t                  IN  szParamValueSize,
                                                    void*                   OUT pParamValue,
                                                    size_t*                 OUT pszParamValueSizeRet    ) = 0;

        /******************************************************************************************
         * Function:    SetCommandQueueProperty    
         * Description: Can be used to enable or disable the properties of a command-queue.
         *              Setting CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE command-queue property determines
         *              whether the commands in a clCommandQueue are executed in-order or out-of-order. 
         *              Changing this command-queue property will cause the OpenCL implementation to block 
         *              until all previously queued commands in command_queue have completed. This can be an expensive 
         *              operation and therefore this kind of change should be only done when absolutely necessary!
         *
         * Arguments:   clCommandQueue [in]         - Specifies the command-queue being queried.
         *              clProperties   [in]         - Specifies the new command-queue properties to be applied to clCommandQueue.
         *              bEnable        [in]         - Determines whether the values specified by properties are enabled 
         *                                            (if enable is CL_TRUE) or disabled (if enable is CL_FALSE) for the commandQueue
         *              pclOldProperties [out]      - Returns the command-queue properties before they were changed by this call. 
         *                                            If old_properties is NULL, it is ignored.            
         *  
         * Return value: CL_SUCCESS                  - If the function is executed successfully
         *               CL_INVALID_COMMAND_QUEUE    - If clCommandQueue is not a valid clCommandQueue
         *               CL_INVALID_VALUE            - If the values specified in clProperties are not valid
         *               CL_INVALID_QUEUE_PROPERTIES - If values specified in properties are not supported by the device.
         *
         * Author:        Arnon Peleg
         * Date:          January 2009
         ******************************************************************************************/    
        virtual cl_err_code SetCommandQueueProperty (   cl_command_queue                IN  clCommandQueue,
                                                        cl_command_queue_properties     IN  clProperties,
                                                        cl_bool                         IN  bEnable,
                                                        cl_command_queue_properties*    OUT pclOldProperties ) = 0;
        
        
        /******************************************************************************************
         * Function:    WaitForEvents    
         * Description: Waits on the host thread for commands identified by event objects in cpEventList to complete.
         *              A command is considered complete if its execution status is CL_COMPLETE or a negative value.
         *              The events specified in event_list act as synchronization points.
         *              All events in the list must belongs to the same context
         *
         *              The function performs an implicit flush of the command-queue.
         *              
         * Arguments:    uiNumEvents [in]        - Number of the events in the eveny list.
         *               cpEventList [in]        - List of events handle that represents the commands to wait on for completion.
         *  
         * Return value: CL_SUCCESS                - If the function is executed successfully
         *               CL_INVALID_VALUE        - If num_events is zero.
         *               CL_INVALID_CONTEXT     - If events specified in event_list do not belong to the same context
         *               CL_INVALID_EVENT       - If event objects specified in cpEventList are not valid event objects.
         *
         * Author:       Arnon Peleg
         * Date:         January 2009
         ******************************************************************************************/    
        virtual cl_err_code WaitForEvents ( cl_uint         IN  uiNumEvents, 
                                            const cl_event* IN  cpEventList ) = 0;

        /******************************************************************************************
         * Function:     GetEventInfo    
         * Description:    Returns information about the event object
         *
         * Arguments:    clEvent             [in]    - Specifies the event object being queried    
         *                clParamName         [in]    - Specifies the information to query. See supported names below.
         *                szParamValueSize    [in]    - Is used to specify the size in bytes of memory pointed to 
         *                                            by szParamValueSize. This size must be >= size of return type as described 
         *                                            below. If pParamValue is NULL, it is ignored.
         *                pParamValue         [out]   - A pointer to memory where the appropriate result being queried is returned. 
         *                                            If pParamValue is NULL, it is ignored.
         *                pszParamValueSizeRet [out]  - Returns the actual size in bytes of data being queried by pParamValue. 
         *                                            If param_value_size_ret is NULL, it is ignored.
         *  
         * Return value: CL_SUCCESS                    - If the function is executed successfully
         *                 CL_INVALID_EVENT           - If clEvent is not a valid event object
         *                 CL_INVALID_VALUE            - If clParamName is not one of the supported values or if size in
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
         * Author:        Arnon Peleg
         * Date:        January 2009
         ******************************************************************************************/    
        virtual cl_err_code GetEventInfo (  cl_event      IN  clEvent,
                                            cl_event_info IN  clParamName,
                                            size_t        IN  szParamValueSize,
                                            void*         OUT pParamValue,
                                            size_t*       OUT pszParamValueSizeRet ) = 0;

        /******************************************************************************************
         * Function:     RetainEvent    
         * Description:    Increments the event reference count
         * Arguments:    clEvent [in] -    Specifies the event being queried    
         *
         * Return value:    CL_SUCCESS         - The function is executed successfully.
         *                    CL_INVALID_EVENT - If clEvent is not a valid event object.
         *
         * Author:        Arnon Peleg
         * Date:        January 2009
         ******************************************************************************************/    
        virtual cl_err_code RetainEvent (cl_event IN clEevent) = 0;

        /******************************************************************************************
         * Function:     ReleaseEvent    
         * Description:    Decrements the event reference count.
         *              The event object is deleted once the reference count becomes zero, 
         *              the specific command identified by this event has completed (or terminated) and 
         *              there are no commands in the command-queues of a context that require a wait for this
         *              event to complete.
         *
         * Arguments:    clEvent [in] -    specifies the event being queried
         *
         * Return value:    CL_SUCCESS         - If the function is executed successfully
         *                    CL_INVALID_EVENT - If clEevent is not a valid event object.
         *
         * Author:        Arnon Peleg
         * Date:        January 2009
         ******************************************************************************************/    
        virtual cl_err_code ReleaseEvent (cl_event IN clEvent) = 0;

        /******************************************************************************************
         * Function:     EnqueueReadBuffer    
         * Description:    Enqueue commands to read from a buffer object to host memory.
         *
         *              If bBlocking is set to CL_TRUE the function performs an implicit flush of the command-queue.
         *              
         * Arguments:    clCommandQueue  [in] -  Refers to the command-queue in which the read / write command will be
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
         *                cpEeventWaitList[in] -  Specify events that need to complete before this particular command can
         *                                      be executed. The events specified in the list act as synchronization points
         *                pEvent          [out]-  Returns an event object that identifies this particular command and can
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
         * Author:        Arnon Peleg
         * Date:        January 2009
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
         * Function:     EnqueueWriteBuffer    
         * Description:    Enqueue commands to write to a buffer object from host memory.
         *
         *              If bBlocking is set to CL_TRUE the function performs an implicit flush of the command-queue.
         *              
         * Arguments:   clCommandQueue  [in] -  Refers to the command-queue in which the read / write command will be
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
         *                cpEeventWaitList[in] -  Specify events that need to complete before this particular command can
         *                                      be executed. The events specified in the list act as synchronization points
         *                pEvent          [out]-  Returns an event object that identifies this particular command and can
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
         * Author:        Arnon Peleg
         * Date:        January 2009
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
         * Function:     EnqueueCopyBuffer    
         * Description:    Enqueues a command to copy a buffer object identified by clSrcBuffer to another 
         *              buffer object identified by clDstBuffer.
         *              
         * Arguments:    clCommandQueue  [in] -  Refers to the command-queue in which the copy command will be queued. 
         *                                      The OpenCL context associated with clCommandQueue, clSrcBuffer, and clDstBuffer
         *                                      must be the same.
         *              clSrcBuffer     [in] -  Refers to a valid buffer object, the buffer to read from.
         *              clDstBuffer     [in] -  Refers to a valid buffer object, the buffer to write into.
         *              szSrcOffset     [in] -  Refers to the offset where to begin copying data from clSrcBuffer.
         *              szDstOffset     [in] -  Refers to the offset where to begin copying data into clDstBuffer.
         *              szCb            [in] -  Refers to the size in bytes to copy.
         *              uNumEventsInWaitList [in]   - Number of the events in the event wait list.
         *                cpEeventWaitList[in] -  Specify events that need to complete before this particular command can
         *                                      be executed. The events specified in the list act as synchronization points
         *                pEvent          [out]-  Returns an event object that identifies this particular command and can
         *                                      be used to query or queue a wait for this particular command to complete.
         *                                      Event can be NULL.
         *
         * Return value:    CL_SUCCESS              -   The function is executed successfully
         *                  CL_INVALID_COMMAND_QUEUE-   If clCommandQueue is not a valid command-queue.
         *                  CL_INVALID_CONTEXT      -   If the context associated with clCommandQueue, clSrcBuffer, and clDstBuffer
         *                                              is not the same.
         *                  CL_INVALID_MEM_OBJECT   -   If clSrcBuffer or clDstBuffer are not a valid buffer objects
         *                  CL_INVALID_VALUE        -   If szSrcOffset, szDstOffset, szCb, szSrcOffset + szCb or szDstOffset + szCb
         *                                              require accessing elements outside the buffer memory objects.
         *                  CL_INVALID_EVENT_WAIT_LIST - if cpEeventWaitList is NULL and uNumEventsInWaitList > 0, 
         *                                              or cpEeventWaitList is not NULL and uNumEventsInWaitList is 0, 
         *                                              or if event objects in cpEeventWaitList are not valid events.
         *                  CL_MEM_COPY_OVERLAP     -   If clSrcBuffer and clDstBuffer are the same buffer object and the source and
         *                                              destination regions overlap.
         *                  CL_OUT_OF_HOST_MEMORY   -   If there is a failure to allocate resources on the host
         *
         * Author:        Arnon Peleg
         * Date:        February 2009
         ******************************************************************************************/        
        virtual cl_err_code EnqueueCopyBuffer(  cl_command_queue    IN  clCommandQueue,
                                                cl_mem              IN  clSrcBuffer,
                                                cl_mem              IN  clDstBuffer,
                                                size_t              IN  szSrcOffset,
                                                size_t              IN  szDstOffset,
                                                size_t              IN  szCb,
                                                cl_uint             IN  uNumEventsInWaitList, 
                                                const cl_event*     IN  cpEeventWaitList, 
                                                cl_event*           OUT pEvent      )  = 0;

        /******************************************************************************************
         * Function:    EnqueueReadImage    
         * Description: Enqueue commands to read from a 2D or 3D image object to host memory.
         *
         *              If bBlocking is set to CL_TRUE the function performs an implicit flush of the command-queue.
         *              
         * Arguments:   clCommandQueue  [in] -  Refers to the command-queue in which the read / write command will be
         *                                      queued. clCommandQueue and clImage must be created with the same OpenCL context.
         *              clImage         [in] -  Refers to a valid 2D or 3D image object to read from.
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
         *              szOrigin        [in] -  defines the (x, y, z) offset in pixels in the image from where to read.
         *                                      If image is a 2D image object, the z value given by szOrigin[2] must be 0.
         *              szRegion        [in] -  Defines the (width, height, depth) in pixels of the 2D or 3D rectangle being read.
         *                                      If image is a 2D image object, the depth value given by szRegion[2] must be 1.
         *              szRowPitch      [in] -  The length of each row in bytes. 
         *                                      This value must be greater than or equal to the (element size in bytes) * (width). 
         *                                      If szRowPitch is set to 0, the appropriate row pitch is calculated based on the
         *                                      size of each element in bytes multiplied by width.
         *              szSlicePitch    [in] -  The size in bytes of the 2D slice of the 3D region of a 3D image being read. 
         *                                      This must be 0 if image is a 2D image. This value must be greater than or equal 
         *                                      to (szRowPitch) * (height). If szSlicePitch is set to 0, the appropriate slice pitch
         *                                      is calculated based on the row_pitch * height.
         *              pOutData        [out]-  Pointer to a buffer in host memory where image data is to be read from.
         *              uNumEventsInWaitList [in]   - Number of the events in the event wait list.
         *              cpEeventWaitList[in] -  Specify events that need to complete before this particular command can
         *                                      be executed. The events specified in the list act as synchronization points
         *              pEvent          [out]-  Returns an event object that identifies this particular command and can
         *                                      be used to query or queue a wait for this particular command to complete.
         *                                      Event can be NULL.
         *
         * Return value:    CL_SUCCESS              -   The function is executed successfully
         *                  CL_INVALID_COMMAND_QUEUE-   If clCommandQueue is not a valid command-queue.
         *                  CL_INVALID_CONTEXT      -   If the context associated with clCommandQueue and clImage are not the same.
         *                  CL_INVALID_MEM_OBJECT   -   If clImage is not a valid image object
         *                  CL_INVALID_VALUE        -   If the region being read by origin and region is out of bounds or if pOutData
         *                                              is a NULL value
         *                  CL_INVALID_VALUE        -   If image is a 2D image object and origin[2] is not equal to 0 or
         *                                              region[2] is not equal to 1 or slice_pitch is not equal to 0.
         *                  CL_INVALID_EVENT_WAIT_LIST - if cpEeventWaitList is NULL and uNumEventsInWaitList > 0, 
         *                                              or cpEeventWaitList is not NULL and uNumEventsInWaitList is 0, 
         *                                              or if event objects in cpEeventWaitList are not valid events.
         *                  CL_OUT_OF_HOST_MEMORY   -   If there is a failure to allocate resources on the host
         *
         * Author:      Arnon Peleg
         * Date:        April 2009
         ******************************************************************************************/        
        virtual cl_err_code EnqueueReadImage (  cl_command_queue    IN  clCommandQueue, 
                                                cl_mem              IN  clImage,
                                                cl_bool             IN  bBlocking, 
                                                const size_t        IN  szOrigin[3],
                                                const size_t        IN  szRegion[3],
                                                size_t              IN  szRowPitch,
                                                size_t              IN  szSlicePitch,
                                                void*               OUT pOutData, 
                                                cl_uint             IN  uNumEventsInWaitList, 
                                                const cl_event*     IN  cpEeventWaitList, 
                                                cl_event*           OUT pEvent      )  = 0;


        /******************************************************************************************
         * Function:    EnqueueWriteImage    
         * Description: Enqueue commands to write to a 2D or 3D image object from host memory.
         *
         *              If bBlocking is set to CL_TRUE the function performs an implicit flush of the command-queue.
         *              
         * Arguments:   clCommandQueue  [in] -  Refers to the command-queue in which the write command will be queued. 
         *                                      clCommandQueue and clImage must be created with the same OpenCL context.
         *              clImage         [in] -  Refers to a valid 2D or 3D image object to write to.
         *              bBlocking       [in] -  If bBlocking is CL_TRUE the command is blocking, the function copies the 
         *                                      data referred to by cpSrcData and enqueues the write operation in the clCommandQueue.
         *                                      The memory pointed to by cpSrcData can be reused after the call returns.
         *                                      If bBlocking is CL_FALSE the function will use cpSrcData to perform a nonblocking
         *                                      write and the function returns immediately. cpSrcData cannot be reused after 
         *                                      the call returns. The event argument returns an event object which can be used
         *                                      to query the execution status of the write command. When the write command has 
         *                                      completed, cpSrcData can then be reused.
         *              szOrigin        [in] -  defines the (x, y, z) offset in pixels in the image from where to write.
         *                                      If image is a 2D image object, the z value given by szOrigin[2] must be 0.
         *              szRegion        [in] -  Defines the (width, height, depth) in pixels of the 2D or 3D rectangle being written.
         *                                      If image is a 2D image object, the depth value given by szRegion[2] must be 1.
         *              szRowPitch      [in] -  The length of each row in bytes. 
         *                                      This value must be greater than or equal to the (element size in bytes) * (width). 
         *                                      If szRowPitch is set to 0, the appropriate row pitch is calculated based on the
         *                                      size of each element in bytes multiplied by width.
         *              szSlicePitch    [in] -  The size in bytes of the 2D slice of the 3D region of a 3D image being read. 
         *                                      This must be 0 if image is a 2D image. This value must be greater than or equal 
         *                                      to (szRowPitch) * (height). If szSlicePitch is set to 0, the appropriate slice pitch
         *                                      is calculated based on the row_pitch * height.
         *              cpSrcData       [out]-  The pointer to buffer in host memory where image is to be to be written from.
         *              uNumEventsInWaitList [in]   - Number of the events in the event wait list.
         *              cpEeventWaitList[in] -  Specify events that need to complete before this particular command can
         *                                      be executed. The events specified in the list act as synchronization points
         *              pEvent          [out]-  Returns an event object that identifies this particular command and can
         *                                      be used to query or queue a wait for this particular command to complete.
         *                                      Event can be NULL.
         *
         * Return value:    CL_SUCCESS              -   The function is executed successfully
         *                  CL_INVALID_COMMAND_QUEUE-   If clCommandQueue is not a valid command-queue.
         *                  CL_INVALID_CONTEXT      -   If the context associated with clCommandQueue and clImage are not the same.
         *                  CL_INVALID_MEM_OBJECT   -   If clImage is not a valid image object
         *                  CL_INVALID_VALUE        -   If the region being read by origin and region is out of bounds or if pOutData
         *                                              is a NULL value
         *                  CL_INVALID_VALUE        -   If image is a 2D image object and origin[2] is not equal to 0 or
         *                                              region[2] is not equal to 1 or slice_pitch is not equal to 0.
         *                  CL_INVALID_EVENT_WAIT_LIST - if cpEeventWaitList is NULL and uNumEventsInWaitList > 0, 
         *                                              or cpEeventWaitList is not NULL and uNumEventsInWaitList is 0, 
         *                                              or if event objects in cpEeventWaitList are not valid events.
         *                  CL_OUT_OF_HOST_MEMORY   -   If there is a failure to allocate resources on the host
         *
         * Author:      Arnon Peleg
         * Date:        April 2009
         ******************************************************************************************/        
        virtual cl_err_code EnqueueWriteImage(  cl_command_queue    IN  clCommandQueue, 
                                                cl_mem              IN  clImage,
                                                cl_bool             IN  bBlocking, 
                                                const size_t        IN  szOrigin[3],
                                                const size_t        IN  szRegion[3],
                                                size_t              IN  szRowPitch,
                                                size_t              IN  szSlicePitch,
                                                const void *        IN  cpSrcData,
                                                cl_uint             IN  uNumEventsInWaitList, 
                                                const cl_event*     IN  cpEeventWaitList, 
                                                cl_event*           OUT pEvent      )  = 0;


        /******************************************************************************************
         * Function:    EnqueueCopyImage    
         * Description: Enqueues a command to copy image objects. clSrcImage and clDstImage can be 2D or 3D image
         *              objects allowing us to perform the following actions:
         *                  - Copy a 2D image object to a 2D image object.
         *                  - Copy a 2D image object to a 2D slice of a 3D image object.
         *                  - Copy a 2D slice of a 3D image object to a 2D image object.
         *                  - Copy a 3D image object to a 3D image object.
         *
         *              It is currently a requirement that the clSrcImage and clDstImage image memory objects must have
         *              the exact same image format (i.e. the cl_image_format descriptor specified when clSrcImage and 
         *              clDstImage are created must match).
         *
         * Arguments:    clCommandQueue [in] -  Refers to the command-queue in which the copy command will be queued. 
         *                                      The OpenCL context associated with clCommandQueue, clSrcImage, and clDstImage
         *                                      must be the same.
         *              clSrcImage      [in] -  Refers to a valid image object, the image to read from.
         *              clDstImage      [in] -  Refers to a valid image object, the image to write into.
         *              szSrcOrigin     [in] -  Defines the starting (x, y, z) location in pixels in clSrcImage from where to 
         *                                      start the data copy. If clSrcImage is a 2D image object, the z value given by
         *                                      szSrcOrigin[2] must be 0.
         *              szDstOrigin    [in] -  Defines the starting (x, y, z) location in pixels in clDstImage from where to
         *                                      start the data copy. If clDstImage is a 2D image object, the z value given by
         *                                      szSrcOrigin[2] must be 0.
         *              szRegion        [in] -  Defines the (width, height, depth) in pixels of the 2D or 3D rectangle to copy.
         *                                      If clSrcImage or clDstImage is a 2D image object, the depth value given by 
         *                                      szRegion[2] must be 1.
         *              uNumEventsInWaitList [in]   - Number of the events in the event wait list.
         *              cpEeventWaitList[in] -  Specify events that need to complete before this particular command can
         *                                      be executed. The events specified in the list act as synchronization points
         *              pEvent          [out]-  Returns an event object that identifies this particular command and can
         *                                      be used to query or queue a wait for this particular command to complete.
         *                                      Event can be NULL.
         *
         * Return value:    CL_SUCCESS              -   The function is executed successfully
         *                  CL_INVALID_COMMAND_QUEUE-   If clCommandQueue is not a valid command-queue.
         *                  CL_INVALID_CONTEXT      -   If the context associated with clCommandQueue, clSrcImage, and clDstImage
         *                                              is not the same.
         *                  CL_INVALID_MEM_OBJECT   -   If clSrcImage or clDstImage are not a valid buffer objects
         *                  CL_IMAGE_FORMAT_MISMATCH-   if clSrcImage and clDstImage do not use the same image format.
         *                  CL_INVALID_VALUE        -   (1) If the 2D or 3D rectangular region specified by szSrcOffsets and 
         *                                              szDstOffset + szRegion refers to a region outside clSrcImage, or if the 
         *                                              2D or 3D rectangular region specified by szDstOffset and szDstOffset + szRegion
         *                                              refers to a region outside clDstImage.
         *                                              (2) If clSrcImage is a 2D image object and szSrcOffsets[2] is not equal to 0
         *                                              or szRegion[2] is not equal to 1.
         *                                              (3) If clDstImage is a 2D image object and szDstOffset[2] is not equal to 0 
         *                                              or szRegion[2] is not equal to 1
         *                  CL_MEM_COPY_OVERLAP     -   If clSrcImage and clDstImage are the same image object and the source and
         *                                              destination regions overlap.
         *                  CL_INVALID_EVENT_WAIT_LIST - if cpEeventWaitList is NULL and uNumEventsInWaitList > 0, 
         *                                              or cpEeventWaitList is not NULL and uNumEventsInWaitList is 0, 
         *                                              or if event objects in cpEeventWaitList are not valid events.
         *                  CL_OUT_OF_HOST_MEMORY   -   If there is a failure to allocate resources on the host
         * 
         * Author:      Arnon Peleg
         * Date:        April 2009
         ******************************************************************************************/        
        virtual cl_err_code EnqueueCopyImage( cl_command_queue    IN  clCommandQueue,
                                                cl_mem              IN  clSrcImage,
                                                cl_mem              IN  clDstImage,
                                                const size_t        IN  szSrcOrigin[3],
                                                const size_t        IN  szDstOrigin[3],
                                                const size_t        IN  szRegion[3],
                                                cl_uint             IN  uNumEventsInWaitList, 
                                                const cl_event*     IN  cpEeventWaitList, 
                                                cl_event*           OUT pEvent      )  = 0;

        /******************************************************************************************
         * Function:    EnqueueCopyImageToBuffer    
         * Description: enqueues a command to copy an image object to a buffer object.
         *
         * Arguments:    clCommandQueue [in] -  Refers to the command-queue in which the copy command will be queued. 
         *                                      The OpenCL context associated with clCommandQueue, clSrcImage, and clDstBuffer
         *                                      must be the same.
         *              clSrcImage      [in] -  Refers to a valid image object, the image to read from.
         *              clDstBuffer     [in] -  Refers to a valid buffer object, the buffer to write into.
         *              szSrcOrigin     [in] -  Defines the starting (x, y, z) location in pixels in clSrcImage from where to 
         *                                      start the data copy. If clSrcImage is a 2D image object, the z value given by
         *                                      szSrcOrigin[2] must be 0.
         *              szRegion        [in] -  Defines the (width, height, depth) in pixels of the 2D or 3D rectangle to copy.
         *                                      If clSrcImage is a 2D image object, the depth value given by szRegion[2] must be 1.
         *              szDstOffset     [in] -  Refers to the offset where to begin copying data into clDstBuffer. 
         *                                      The size in bytes of the region to be copied referred to as dst_cb is computed as
         *                                      width * height * depth * (bytes in image element) if clSrcImage is a 3D image object,
         *                                      and as width * height * bytes/image element if clSrcImage is a 2D image object.
         *              uNumEventsInWaitList [in]   - Number of the events in the event wait list.
         *              cpEeventWaitList[in] -  Specify events that need to complete before this particular command can
         *                                      be executed. The events specified in the list act as synchronization points
         *              pEvent          [out]-  Returns an event object that identifies this particular command and can
         *                                      be used to query or queue a wait for this particular command to complete.
         *                                      Event can be NULL.
         *
         * Return value:    CL_SUCCESS              -   The function is executed successfully
         *                  CL_INVALID_COMMAND_QUEUE-   If clCommandQueue is not a valid command-queue.
         *                  CL_INVALID_CONTEXT      -   If the context associated with clCommandQueue, clSrcImage, and clDstBuffer
         *                                              is not the same.
         *                  CL_INVALID_MEM_OBJECT   -   If clSrcImage or clDstBuffer are not a valid buffer objects
         *                  CL_INVALID_VALUE        -   (1) If the 2D or 3D rectangular region specified by szSrcOrigin and
         *                                              szSrcOrigin + szRegion refers to a region outside clSrcImage, or if the
         *                                              region specified by szDstOffset and szDstOffset + dst_cb to a region 
         *                                              outside clDstBuffer.
         *                                              (2) If clSrcImage is a 2D image object and szSrcOrigin[2] is not equal to 0
         *                                              or szRegion[2] is not equal to 1.
         *                  CL_INVALID_EVENT_WAIT_LIST - if cpEeventWaitList is NULL and uNumEventsInWaitList > 0, 
         *                                              or cpEeventWaitList is not NULL and uNumEventsInWaitList is 0, 
         *                                              or if event objects in cpEeventWaitList are not valid events.
         *                  CL_OUT_OF_HOST_MEMORY   -   If there is a failure to allocate resources on the host
         * 
         * Author:      Arnon Peleg
         * Date:        April 2009
         ******************************************************************************************/        
        virtual cl_err_code EnqueueCopyImageToBuffer( 
                                                cl_command_queue    IN  clCommandQueue,
                                                cl_mem              IN  clSrcImage,
                                                cl_mem              IN  clDstBuffer,
                                                const size_t        IN  szSrcOrigin[3],
                                                const size_t        IN  szRegion[3],
                                                size_t              IN  szDstOffset,
                                                cl_uint             IN  uNumEventsInWaitList, 
                                                const cl_event*     IN  cpEeventWaitList, 
                                                cl_event*           OUT pEvent      )  = 0;

        /******************************************************************************************
         * Function:    EnqueueCopyBufferToImage    
         * Description: Enqueues a command to copy a buffer object to an image object.
         *
         * Arguments:   clCommandQueue [in] -   Refers to the command-queue in which the copy command will be queued. 
         *                                      The OpenCL context associated with clCommandQueue, clSrcBuffer, and clDstm,age
         *                                      must be the same.
         *              clSrcBuffer     [in] -  Refers to a valid buffer object, the buffer to read from.
         *              clDstImage      [in] -  Refers to a valid image object, the image to write into.
         *              szSrcOffset     [in] -  refers to the offset where to begin copying data from clSrcBuffer.
         *              szDstOrigin     [in] -  Refers to the (x, y, z) offset in pixels where to begin copying data to clDstImage.
         *                                      If clDstImage is a 2D image object, the z value given by szDstOrigin[2] must be 0.
         *              szRegion        [in] -  Defines the (width, height, depth) in pixels of the 2D or 3D rectangle to copy. 
         *                                      If clDstImage is a 2D image object, the depth value given by region[2] must be 1.
         *                                      The size in bytes of the region to be copied from clSrcBuffer referred to as src_cb
         *                                      is computed as width * height * depth * bytes/image element if clDstImage is a 
         *                                      3D image object and is computed as width * height * bytes/image element 
         *                                      if clDstImage is a 2D image object.
         *              uNumEventsInWaitList [in]   - Number of the events in the event wait list.
         *              cpEeventWaitList[in] -  Specify events that need to complete before this particular command can
         *                                      be executed. The events specified in the list act as synchronization points
         *              pEvent          [out]-  Returns an event object that identifies this particular command and can
         *                                      be used to query or queue a wait for this particular command to complete.
         *                                      Event can be NULL.
         *
         * Return value:    CL_SUCCESS              -   The function is executed successfully
         *                  CL_INVALID_COMMAND_QUEUE-   If clCommandQueue is not a valid command-queue.
         *                  CL_INVALID_CONTEXT      -   If the context associated with clCommandQueue, clSrcBuffer, and clDstImage
         *                                              is not the same.
         *                  CL_INVALID_MEM_OBJECT   -   If clSrcBuffer or clDstImage are not a valid buffer objects
         *                  CL_INVALID_VALUE        -   (1) If the 2D or 3D rectangular region specified by szDstOrigin and
         *                                              szDstOrigin + szRegion refers to a region outside clDstImage, or if the
         *                                              region specified by szSrcOffset and szSrcOffset + dst_cb to a region 
         *                                              outside clSrcBuffer.
         *                                              (2) If clDstImage is a 2D image object and szDstOrigin[2] is not equal to 0
         *                                              or szRegion[2] is not equal to 1.
         *                  CL_INVALID_EVENT_WAIT_LIST - if cpEeventWaitList is NULL and uNumEventsInWaitList > 0, 
         *                                              or cpEeventWaitList is not NULL and uNumEventsInWaitList is 0, 
         *                                              or if event objects in cpEeventWaitList are not valid events.
         *                  CL_OUT_OF_HOST_MEMORY   -   If there is a failure to allocate resources on the host
         * 
         * Author:      Arnon Peleg
         * Date:        April 2009
         ******************************************************************************************/        
        virtual cl_err_code EnqueueCopyBufferToImage( 
                                                cl_command_queue    IN  clCommandQueue,
                                                cl_mem              IN  clSrcBuffer,
                                                cl_mem              IN  clDstImage,
                                                size_t              IN  szSrcOffset,
                                                const size_t        IN  szDstOrigin[3],
                                                const size_t        IN  szRegion[3],
                                                cl_uint             IN  uNumEventsInWaitList, 
                                                const cl_event*     IN  cpEeventWaitList, 
                                                cl_event*           OUT pEvent      )  = 0;

        /******************************************************************************************
         * Function:    EnqueueMapBuffer    
         * Description: Enqueues a command to map a region of the buffer object given by buffer into
         *              the host address space and returns a pointer to this mapped region.
         *
         *              If bBlockingMap is set to CL_TRUE the function performs an implicit flush
         *              of the command-queue
         *              
         * Arguments:   clCommandQueue  [in] -  Refers to the command-queue in which the mao command will be queued. 
         *              clBuffer        [in] -  A valid buffer object. The OpenCL context associated with clCommandQueue
         *                                      and clBuffer must be the same.
         *              bBlockingMap    [in] -  Indicates if the map operation is blocking or non-blocking.
         *                                      If blocking_map is CL_TRUE, tunction does not return until the specified region
         *                                      in buffer can be mapped.
         *                                      If blocking_map is CL_FALSE i.e. map operation is non-blocking, the pointer to the mapped
         *                                      region returned and cannot be used until the map command has completed. 
         *              clMapFlags      [in] -  Is a bit-field and can be set to CL_MAP_READ to indicate that the region specified by
         *                                      (offset, cb) in the buffer object is being mapped for reading, and/or CL_MAP_WRITE
         *                                      to indicate that the region is being mapped for writing.
         *              szSrcOffset     [in] -  Offset in bytes of the region in the buffer object that is being mapped.         
         *              szCb            [in] -  Size in bytes of the region in the buffer object that is being mapped.
         *              uNumEventsInWaitList [in]   - Number of the events in the event wait list.
         *              cpEeventWaitList[in] -  Specify events that need to complete before this particular command can
         *                                      be executed. The events specified in the list act as synchronization points
         *              pEvent          [out]-  Returns an event object that identifies this particular command and can
         *                                      be used to query or queue a wait for this particular command to complete.
         *                                      Event can be NULL.
         *              pErrcodeRet     [out]-  Returns an appropriate error code. If errcode_ret is NULL, no error code is returned.
         *
         * Error codes:     CL_SUCCESS              -   The function is executed successfully.
         *                  CL_INVALID_COMMAND_QUEUE-   If clCommandQueue is not a valid command-queue.
         *                  CL_INVALID_CONTEXT      -   If the context associated with clCommandQueue and clBuffer
         *                                              is not the same.
         *                  CL_INVALID_MEM_OBJECT   -   If clBuffer is not a valid buffer object.
         *                  CL_INVALID_VALUE        -   If region being mapped given by (offset, cb) is out of bounds or if
         *                                              values specified in map_flags are not valid.
         *                  CL_INVALID_EVENT_WAIT_LIST - if cpEeventWaitList is NULL and uNumEventsInWaitList > 0, 
         *                                              or cpEeventWaitList is not NULL and uNumEventsInWaitList is 0, 
         *                                              or if event objects in cpEeventWaitList are not valid events.
         *                  CL_OUT_OF_HOST_MEMORY   -   If there is a failure to allocate resources on the host
         *
         * Return Value:    The pointer returned maps a region starting at offset and is at least cb bytes in size. The result of
         *                  a memory access outside this region is undefined.
         *                  A valid pointer to a mapped region is returned if buffer is created with a host_ptr and CL_MEM_USE_HOST_PTR
         *                  is set in flags arguments to clCreateBuffer or if buffer is created with CL_MEM_ALLOC_HOST_PTR set in flags
         *                  argument to clCreateBuffer and the region specified by (offset, cb) is a valid region in the buffer object. 
         *                  In case of error, a NULL pointer is retured.
         * 
         * Author:      Arnon Peleg
         * Date:        February 2009
         ******************************************************************************************/        
        virtual void* EnqueueMapBuffer(     cl_command_queue    IN  clCommandQueue,
                                            cl_mem              IN  clBuffer,
                                            cl_bool             IN  bBlockingMap,
                                            cl_map_flags        IN  clMapFlags,
                                            size_t              IN  szOffset,
                                            size_t              IN  szCb,
                                            cl_uint             IN  uNumEventsInWaitList, 
                                            const cl_event*     IN  cpEeventWaitList, 
                                            cl_event*           OUT pEvent,      
                                            cl_int*             OUT pErrcodeRet     )  = 0;

        /******************************************************************************************
         * Function:    EnqueueMapImage    
         * Description: Enqueues a command to map a region in the image object given by image into 
         *              the host address space and returns a pointer to this mapped region.
         *
         *              If bBlockingMap is set to CL_TRUE the function performs an implicit flush
         *              of the command-queue
         *              
         * Arguments:   clCommandQueue  [in] -  Refers to the command-queue in which the map command will be queued. 
         *              clImage         [in] -  A valid image object. The OpenCL context associated with clCommandQueue
         *                                      and clImage must be the same.
         *              bBlockingMap    [in] -  Indicates if the map operation is blocking or non-blocking.
         *                                      If blocking_map is CL_TRUE, function does not return until the specified region
         *                                      in image can be mapped.
         *                                      If blocking_map is CL_FALSE i.e. map operation is non-blocking, the pointer to the mapped
         *                                      region returned and cannot be used until the map command has completed. 
         *              clMapFlags      [in] -  Is a bit-field and can be set to CL_MAP_READ to indicate that the region specified by
         *                                      (origin, region) in the image object is being mapped for reading, and/or CL_MAP_WRITE
         *                                      to indicate that the region is being mapped for writing.
         *              szOrigin        [in] -   
         *              szRegion        [in] -  origin and region define the (x, y, z) offset in pixels and (width, height, depth) 
         *                                      in pixels of the 2D or 3D rectangle region that is to be mapped. 
         *                                      If image is a 2D image object, the z value given by origin[2] must be 0 and the
         *                                      depth value given by region[2] must be 1.
         *              pszImageRowPitch  [in]- Returns the scan-line pitch in bytes for the mapped region. 
         *                                      This must be a non-NULL value.  
         *              pszImageSlicePitch[in]- Returns the size in bytes of each 2D slice for the mapped region. For a 2D
         *                                      image, zero is returned if this argument is not NULL. 
         *                                      For a 3D image, pszImageSlicePitch must be a non-NULL value.
         *              uNumEventsInWaitList [in] - Number of the events in the event wait list.
         *              cpEeventWaitList[in] -  Specify events that need to complete before this particular command can
         *                                      be executed. The events specified in the list act as synchronization points
         *              pEvent          [out]-  Returns an event object that identifies this particular command and can
         *                                      be used to query or queue a wait for this particular command to complete.
         *                                      Event can be NULL.
         *              pErrcodeRet     [out]-  Returns an appropriate error code. If errcode_ret is NULL, no error code is returned.
         *
         * Error codes:     CL_SUCCESS              -   The function is executed successfully.
         *                  CL_INVALID_COMMAND_QUEUE-   If clCommandQueue is not a valid command-queue.
         *                  CL_INVALID_CONTEXT      -   If the context associated with clCommandQueue and clBuffer
         *                                              is not the same.
         *                  CL_INVALID_MEM_OBJECT   -   If clImage is not a valid buffer object.
         *                  CL_INVALID_VALUE        -   If region being mapped given by (offset, cb) is out of bounds or if
         *                                              values specified in map_flags are not valid.
         *                  CL_INVALID_EVENT_WAIT_LIST - if cpEeventWaitList is NULL and uNumEventsInWaitList > 0, 
         *                                              or cpEeventWaitList is not NULL and uNumEventsInWaitList is 0, 
         *                                              or if event objects in cpEeventWaitList are not valid events.
         *                  CL_OUT_OF_HOST_MEMORY   -   If there is a failure to allocate resources on the host
         *
         * Return Value:    EnqueueMapImage will return a pointer to the mapped region if image is created with a
         *                  host_ptr and CL_MEM_USE_HOST_PTR is set in flags arguments to clCreateImage{2D|3D} or
         *                  if image is created with CL_MEM_ALLOC_HOST_PTR set in flags argument to
         *                  clCreateImage{2D|3D} and the 2D or 3D rectangle specified by origin and region is a valid
         *                  region in the image object. The pErrcodeRet is set to CL_SUCCESS.
         *                  A NULL pointer is returned otherwise with one of the aboce error codes returned in pErrcodeRet:
         * 
         * Author:      Arnon Peleg
         * Date:        April 2009
         ******************************************************************************************/        
        virtual void* EnqueueMapImage(  cl_command_queue    IN  clCommandQueue,
                                        cl_mem              IN  clImage,
                                        cl_bool             IN  bBlockingMap,
                                        cl_map_flags        IN  clMapFlags,
                                        const size_t        IN  szOrigin[3],
                                        const size_t        IN  szRegion[3],
                                        size_t*             OUT pszImageRowPitch,
                                        size_t*             OUT pszImageSlicePitch,                                            
                                        cl_uint             IN  uNumEventsInWaitList, 
                                        const cl_event*     IN  cpEeventWaitList, 
                                        cl_event*           OUT pEvent,      
                                        cl_int*             OUT pErrcodeRet     )  = 0;

        /******************************************************************************************
         * Function:     EnqueueUnmapMemObject    
         * Description:    Enqueues a command to unmap a previously mapped region of a memory object. Reads or writes from 
         *              the host using the pointer returned by EnqueueMapBuffer or EnqueueMapImage are considered to be complete.
         *              
         * Arguments:    clCommandQueue  [in] -  Refers to the command-queue in which the unmap command will be queued.
         *              clMemObj        [in] -  Is a valid memory object. The OpenCL context associated with clCommandQueue and
         *                                      clMemObj must be the same.
         *              mappedPtr       [in] -  Is the host address returned by a previous call to EnqueueMapBuffer or
         *                                      EnqueueMapImage for clMemObj.
         *              uNumEventsInWaitList [in]   - Number of the events in the event wait list.
         *                cpEeventWaitList[in] -  Specify events that need to complete before this particular command can
         *                                      be executed. The events specified in the list act as synchronization points
         *                pEvent          [out]-  Returns an event object that identifies this particular command and can
         *                                      be used to query or queue a wait for this particular command to complete.
         *                                      Event can be NULL.
         *
         * Return value:    CL_SUCCESS              -   The function is executed successfully
         *                  CL_INVALID_COMMAND_QUEUE-   If clCommandQueue is not a valid command-queue.
         *                  CL_INVALID_CONTEXT      -   If the context associated with clCommandQueue and clMemObj is not the same.
         *                  CL_INVALID_MEM_OBJECT   -   If clMemObj is not a valid memory object
         *                  CL_INVALID_VALUE        -   If mappedPtr is not a valid pointer returned by EnqueueMapBuffer or 
         *                                              EnqueueMapImage for memobj.
         *                  CL_INVALID_EVENT_WAIT_LIST - if cpEeventWaitList is NULL and uNumEventsInWaitList > 0, 
         *                                              or cpEeventWaitList is not NULL and uNumEventsInWaitList is 0, 
         *                                              or if event objects in cpEeventWaitList are not valid events.
         *                  CL_OUT_OF_HOST_MEMORY   -   If there is a failure to allocate resources on the host
         *
         * Author:        Arnon Peleg
         * Date:        February 2009
         ******************************************************************************************/        
        virtual cl_err_code EnqueueUnmapMemObject(  cl_command_queue    IN  clCommandQueue,
                                                    cl_mem              IN  clMemObj,
                                                    void*               IN  mappedPtr,
                                                    cl_uint             IN  uNumEventsInWaitList, 
                                                    const cl_event*     IN  cpEeventWaitList, 
                                                    cl_event*           OUT pEvent  )  = 0;     

        /******************************************************************************************
         * Function:     EnqueueNDRangeKernel    
         * Description:    Enqueues a command to execute a kernel on a device.
         *              
         * Arguments:    clCommandQueue      [in] - A valid command-queue. The kernel will be queued for execution on the
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
         *                cpEeventWaitList    [in] - Specify events that need to complete before this particular command can
         *                                         be executed. The events specified in the list act as synchronization points.
         *                pEvent              [out]- Returns an event object that identifies this particular command and can
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
         * Author:        Arnon Peleg
         * Date:        January 2009
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


        /******************************************************************************************
         * Function:     EnqueueTask    
         * Description:    Enqueues a command to execute a kernel on a device. The kernel is executed using a single
         *              work-item.
         *              EnqueueTask is equivalent to calling EnqueueNDRangeKernel with work_dim = 1,
         *              global_work_offset = NULL, global_work_size[0] set to 1 and local_work_size[0] set to 1.
         *              
         * Arguments:    clCommandQueue      [in] - A valid command-queue. The kernel will be queued for execution on the
         *                                         device associated with clCommandQueue.
         *              clKernel            [in] - A valid kernel object. The  context associated with kernel and clCommandQueue
         *                                         must be the same.
         *              uNumEventsInWaitList[in] - Number of the events in the event wait list.
         *                cpEeventWaitList    [in] - Specify events that need to complete before this particular command can
         *                                         be executed. The events specified in the list act as synchronization points.
         *                pEvent              [out]- Returns an event object that identifies this particular command and can
         *                                         be used to query or queue a wait for this particular command to complete.
         *                                         Event can be NULL.
         *
         * Return value:    CL_SUCCESS              -   The function is executed successfully
         *                  CL_INVALID_PROGRAM_EXECUTABLE - If there is no successfully built program executable available 
         *                                              for device associated with clCommandQueue
         *                  CL_INVALID_COMMAND_QUEUE-   If clCommandQueue is not a valid command-queue.
         *                  CL_INVALID_KERNEL       -   If kernel is not a valid kernel object.
         *                  CL_INVALID_KERNEL_ARGS  -   If the kernel argument values have not been specified.
         *                  CL_INVALID_WORK_GROUP_SIZE- If a work-group size is specified for kernel using the
         *                                              __attribute__((reqd_work_group_size(X, Y, Z))) qualifier in program
         *                                              source and is not (1, 1, 1).
         *                  CL_OUT_OF_RESOURCES     -   If there is a failure to queue the execution instance of kernel on the 
         *                                              clCommandQueue because of insufficient resources needed to execute the kernel.                  
         *                  CL_MEM_OBJECT_ALLOCATION_FAILURE - If there is a failure to allocate memory for image or buffer 
         *                                              objects specified as arguments to kernel
         *                  CL_INVALID_EVENT_WAIT_LIST- If cpEeventWaitList is NULL and uNumEventsInWaitList > 0, 
         *                                              or cpEeventWaitList is not NULL and uNumEventsInWaitList is 0, 
         *                                              or if event objects in cpEeventWaitList are not valid events.
         *                  CL_OUT_OF_HOST_MEMORY   -   If there is a failure to allocate resources on the host
         *
         * Author:        Arnon Peleg
         * Date:        February 2009
         ******************************************************************************************/            
        virtual cl_err_code EnqueueTask (   cl_command_queue    IN  clCommandQueue,
                                            cl_kernel           IN  clKernel,
                                            cl_uint             IN  uNumEventsInWaitList, 
                                            const cl_event*     IN  cpEeventWaitList,
                                            cl_event*           OUT pEvent      ) = 0;

        /******************************************************************************************
         * Function:     EnqueueNativeKernel
         * Description:    Enqueues a command to execute a native C/C++ function not compiled using the OpenCL compiler.
         *              
         * Arguments:    clCommandQueue      [in] -  A valid command-queue. A native user function can only be executed on a
         *                                          command-queue created on a device that has CL_EXEC_NATIVE_KERNEL capability 
         *                                          set in CL_DEVICE_EXECUTION_CAPABILITIES.
         *              pUserFnc            [in] -  A pointer to a host-callable user function.
         *              pArgs               [in] -  A pointer to the args list that user_func should be called with.
         *              szCbArgs            [in] -  The size in bytes of the args list that args points to.
         *                                          The data pointed to by args will be copied and a pointer to this copied region will be
         *                                          passed to pUserFnc. When EnqueueNativeKernel returns, the memory region pointed
         *                                          to by args can be reused by the application.
         *              uNumMemObjects      [in] -  The number of buffer objects that are passed in args.
         *              clMemList           [in] -  A list of valid buffer objects, if num_mem_objects > 0.
         *              ppArgsMemLoc        [in] -  A pointer to appropriate locations that args points to where memory object
         *                                          handles (cl_mem values) are stored. Before the user function is executed, the 
         *                                          memory object handles are replaced by pointers to global memory.
         *              uNumEventsInWaitList[in] - Number of the events in the event wait list.
         *                cpEeventWaitList    [in] - Specify events that need to complete before this particular command can
         *                                         be executed. The events specified in the list act as synchronization points.
         *                pEvent              [out]- Returns an event object that identifies this particular command and can
         *                                         be used to query or queue a wait for this particular command to complete.
         *                                         Event can be NULL.
         *
         * Return value:    CL_SUCCESS              -   The function is executed successfully
         *                  CL_INVALID_COMMAND_QUEUE-   If clCommandQueue is not a valid command-queue.
         *                  CL_INVALID_VALUE        -   
         *                              1. If pUserFnc is NULL.
         *                              2. If pArgs is a NULL value and szCbArgs > 0
         *                              3. If pArgs is a NULL value and uNumMemObjects > 0.
         *                              4. if pArgs is not NULL and szCbArgs is 0.
         *                              5. If uNumMemObjects > 0 and clMemList or ppArgsMemLoc are NULL.
         *                              6. if uNumMemObjects = 0 and clMemList or ppArgsMemLoc are not NULL.
         *                  CL_INVALID_OPERATION    -   If device cannot execute the native kernel.
         *                  CL_INVALID_MEM_OBJECT   -   if one or more memory objects specified in clMemList are not valid 
         *                                              or are not buffer objects.
         *                  CL_OUT_OF_RESOURCES     -   If there is a failure to queue the execution instance of kernel on the 
         *                                              clCommandQueue because of insufficient resources needed to execute the kernel.                  
         *                  CL_MEM_OBJECT_ALLOCATION_FAILURE - If there is a failure to allocate memory for image or buffer 
         *                                              objects specified as arguments to kernel
         *                  CL_INVALID_EVENT_WAIT_LIST- If cpEeventWaitList is NULL and uNumEventsInWaitList > 0, 
         *                                              or cpEeventWaitList is not NULL and uNumEventsInWaitList is 0, 
         *                                              or if event objects in cpEeventWaitList are not valid events.
         *                  CL_OUT_OF_HOST_MEMORY   -   If there is a failure to allocate resources on the host
         *
         * Author:        Arnon Peleg
         * Date:        February 2009
         ******************************************************************************************/            
        virtual cl_err_code EnqueueNativeKernel (   cl_command_queue    IN  clCommandQueue,
                                                    void                IN  (*pUserFnc)(void *),
                                                    void*               IN  pArgs,
                                                    size_t              IN  szCbArgs,
                                                    cl_uint             IN  uNumMemObjects,
                                                    const cl_mem*       IN  clMemList,
                                                    const void **       IN  ppArgsMemLoc,
                                                    cl_uint             IN  uNumEventsInWaitList, 
                                                    const cl_event*     IN  cpEeventWaitList,
                                                    cl_event*           OUT pEvent      ) = 0;


        /******************************************************************************************
         * Function:     EnqueueMarker    
         * Description:    Enqueues a marker command to clCommandQueue. The marker command returns an event which
         *              can be used by to queue a wait on this marker event i.e. wait for all commands queued
         *              before the marker command to complete. Has no effect in In-Order-Execution.
         *              
         * Arguments:    clCommandQueue  [in] -  Refers to the command-queue in which marker command will be queued.
         *                                      Marker can only queued in Out-Of-Order queue.
         *                pEvent          [out]-  Returns an event object that identifies this particular command. There is
         *                                      no meaning for this command without the use of this event
         *
         * Return value:    CL_SUCCESS              -   The function is executed successfully
         *                  CL_INVALID_COMMAND_QUEUE-   If clCommandQueue is not a valid command-queue.
         *                  CL_INVALID_VALUE        -   If event is a NULL value
         *                  CL_OUT_OF_HOST_MEMORY   -   If there is a failure to allocate resources on the host
         *
         * Author:        Arnon Peleg
         * Date:        February 2009
         ******************************************************************************************/        
        virtual cl_err_code EnqueueMarker ( cl_command_queue    IN  clCommandQueue, 
                                            cl_event*           OUT pEvent      ) = 0;

        /******************************************************************************************
         * Function:     EnqueueWaitForEvents    
         * Description:    Enqueues a wait for a specific event or a list of events to complete before any future commands
         *              queued in the command-queue are executed. 
         
         * Arguments:    clCommandQueue  [in] -  Refers to the command-queue in which EnqueueWaitForEvents command will be queued.
         *                                      WaitForEvents can only queued in Out-Of-Order queue.
         *              uiNumEvents     [in] -  Specifies the number of events given by event_list.
         *                cpEeventList    [in] -  Specifies events that need to complete before this command is marked completed.
         *                                      The events specified in event_list act as synchronization points.
         *
         * Return value:    CL_SUCCESS              -   The function is executed successfully
         *                  CL_INVALID_COMMAND_QUEUE-   If clCommandQueue is not a valid command-queue.
         *                  CL_INVALID_VALUE        -   If event objects specified in event_list are not valid events
         *                  CL_OUT_OF_HOST_MEMORY   -   If there is a failure to allocate resources on the host
         *
         * Author:        Arnon Peleg
         * Date:        February 2009
         ******************************************************************************************/        
        virtual cl_err_code EnqueueWaitForEvents (  cl_command_queue    IN  clCommandQueue, 
                                                    cl_uint             IN  uiNumEvents, 
                                                    const cl_event*     OUT cpEventList     ) =0;

        /******************************************************************************************
         * Function:     EnqueueBarrier    
         * Description:    Enqueues a barrier operation. The EnqueueBarrier command ensures that all queued
         *              commands in command_queue have finished execution before the next batch of commands can
         *              begin execution. EnqueueBarrier is a synchronization point.
         
         * Arguments:    clCommandQueue  [in] -  Refers to the command-queue in which EnqueueBarrier command will be queued.
         *                                      EnqueueBarrier can only queued in Out-Of-Order queue.
         *
         * Return value:    CL_SUCCESS              -   The function is executed successfully
         *                  CL_INVALID_COMMAND_QUEUE-   If clCommandQueue is not a valid command-queue.
         *                  CL_OUT_OF_HOST_MEMORY   -   If there is a failure to allocate resources on the host
         *
         * Author:        Arnon Peleg
         * Date:        February 2009
         ******************************************************************************************/        
        virtual cl_err_code EnqueueBarrier (cl_command_queue IN clCommandQueue) =0;

        /******************************************************************************************
         * Function:     Flush    
         * Description:    Issues all previously queued OpenCL commands in command_queue to the device associated
         *              with command_queue. Flush only guarantees that all queued commands to command_queue
         *              get issued to the appropriate device. There is no guarantee that they will be completed after
         *              Flush returns.
         *              In case of dependency between commands in differet queues, the function returns immidatly and 
         *              the implementation is responsible to issue all commands when it able to.
         *
         * Arguments:    clCommandQueue  [in] -  Refers to the command-queue to be flushed.
         *
         * Return value:    CL_SUCCESS              -   The function is executed successfully
         *                  CL_INVALID_COMMAND_QUEUE-   If clCommandQueue is not a valid command-queue.
         *                  CL_OUT_OF_HOST_MEMORY   -   If there is a failure to allocate resources on the host
         *
         * Author:        Arnon Peleg
         * Date:        March 2009
         ******************************************************************************************/        
        virtual cl_err_code Flush  (cl_command_queue IN clCommandQueue) =0;

        /******************************************************************************************
         * Function:     Finish    
         * Description:    Blocks until all previously queued OpenCL commands in command_queue are issued
         *              to the associated device and have completed. Finish does not return until all
         *              queued commands in command_queue have been processed and completed. 
         *              Finish is also a synchronization point.
         *
         * Arguments:    clCommandQueue  [in] -  Refers to the command-queue to be flushed.
         *
         * Return value:    CL_SUCCESS              -   The function is executed successfully
         *                  CL_INVALID_COMMAND_QUEUE-   If clCommandQueue is not a valid command-queue.
         *                  CL_OUT_OF_HOST_MEMORY   -   If there is a failure to allocate resources on the host
         *
         * Author:        Arnon Peleg
         * Date:        March 2009
         ******************************************************************************************/        
        virtual cl_err_code Finish (cl_command_queue IN clCommandQueue) =0;

        /******************************************************************************************
         * Function:	GetEventProfilingInfo    
         * Description:	returns profiling information for the command associated with event.
         *
         * Arguments:	clEvent  [in]		- specifies the event object
		 *				clParamName [in]	- specifies the profiling data to query.
		 *				szParamValueSize [in] - is a pointer to memory where the appropriate result
		 *										being queried is returned. If pParamValue is NULL, 
		 *										it is ignored
		 *				pParamValue [out]	- is used to specify the size in bytes of memory pointed
		 *									  to by param_value. This size must be >= size of return 
		 *									  type
		 *				pszParamValueSizeRet [out] - returns the actual size in bytes of data copied
		 *											 to param_value. If pszParamValueSizeRet is NULL,
		 *											it is ignored
         *
         * Return value:    CL_SUCCESS              -   The function is executed successfully and the
		 *												profiling information has been recorded
         *                  CL_PROFILING_INFO_NOT_AVAILABLE	-	if the CL_QUEUE_PROFILING_ENABLE flag
		 *														is not set for the command-queue and 
		 *														if the profiling information is 
		 *														currently not available (because the 
		 *														command identified by event has not 
		 *														completed)
		 *					CL_INVALID_VALUE	-	if param_name is not valid, or if size in bytes
		 *											specified by param_value_size is < size of return
		 *											type
         *                  CL_INVALID_EVENT	-	if event is a not a valid event object
         *
         * Author:		Uri Levy
         * Date:        May 2009
         ******************************************************************************************/    
		virtual cl_err_code GetEventProfilingInfo (cl_event clEvent, cl_profiling_info clParamName, size_t szParamValueSize, void * pParamValue, size_t * pszParamValueSizeRet) = 0;

        };

}}};    // Intel::OpenCL::Framework
#endif  // !defined(__OCL_IEXECUTION_H__)
