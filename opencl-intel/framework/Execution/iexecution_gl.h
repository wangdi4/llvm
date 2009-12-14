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
//  iexecution_gl.h
//  Implementation of the IExecutionGL interface
//  Created on:      10-Dec-2008 2:08:23 PM
//  Original author: ulevy
///////////////////////////////////////////////////////////////////////////////////////////////////

#if !defined(OCL_IEXECUTION_GL_H_)
#define OCL_IEXECUTION_GL_H_

#include <cl_types.h>
#ifdef WIN32
#include <Windows.h>
#endif
#include <gl\GL.h>
#include <gl\GLU.h>
#include <CL\cl_gl.h>

namespace Intel { namespace OpenCL { namespace Framework {

	/**********************************************************************************************
	* Class name:	IExecutionGL
	*
	* Description:	IExecutionGL iterface - outlines the execution OpneCL Sharing Memory Objects with 
	*				OpenGL / OpenGL ES Buffer, Texture and Renderbuffer Object srelated functions
	* Author:		Uri Levy
	* Date:			June 2009
	**********************************************************************************************/
	class IExecutionGL
	{
	public:
        virtual ~IExecutionGL(){};

		/******************************************************************************************
		* Function: 	EnqueueAcquireGLObjects    
		* Description:	used to acquire OpenCL memory objects that have been created from OpenGL 
		*				objects. These objects need to be acquired before they can be used by any 
		*				OpenCL commands queued to a command-queue. The OpenGL objects are acquired
		*				by the OpenCL context associated with clCommandQueue and can therefore be 
		*				used by all command-queues associated with the OpenCLcontext
		* Arguments:	clCommandQueue [in] -		is a valid command-queue. All devices used to 
		*											create the OpenCL context associated with 
		*											command_queue must support acquiring shared 
		*											CL/GL objects. This constraint is enforced at 
		*											context creation time
		*				uiNumObjects [in] -			is the number of memory objects to be acquired
		*											in pclMemObjects
		*				pclMemObjects [in] -		is a pointer to a list of CL memory objects 
		*											that correspond to GL objects
		*				uiNumEventsInWaitList [in]-	specify events that need to complete before 
		*											this particular command can be executed. 
		*											If pclEventWaitList is NULL, then this 
		*											particular command does not wait on any event 
		*											to complete. If pclEventWaitList is NULL, 
		*											uiNumEventsInWaitList must be 0. If 
		*											pclEventWaitList is not NULL, the list of 
		*											events pointed to by event_wait_list must be 
		*											valid and uiNumEventsInWaitList must be greater
		*											than 0. The events specified in
		*											pclEventWaitList act as synchronization points
		*				pclEvent [in] -				returns an event object that identifies this 
		*											command and can be used to query or queue a 
		*											wait for the command to complete. event can be
		*											NULL in which case it will not be possible for
		*											the application to query the status of this 
		*											command or queue a wait for this command to 
		*											complete
		* Return value:	CL_SUCCESS				if the function is executed successfully. If
		*										uiNumObjects is 0 and pclMemObjects is NULL the 
		*										function does nothing
		*				CL_INVALID_VALUE		if uiNumObjects is zero and pclMemObjects is not a
		*										NULL value or if uiNumObjects > 0 and pclMemObjects
		*										is NULL
		*				CL_INVALID_MEM_OBJECT	if memory objects in pclMemObjects are not valid 
		*										OpenCL memory objects
		*				CL_INVALID_COMMAND_QUEUE if clCommandQueue is not a valid command-queue
		*				CL_INVALID_CONTEXT		if context associated with clCommandQueue was not
		*										created from an OpenGL context
		*				CL_INVALID_GL_OBJECT	if memory objects in pclMemObjects have not been
		*										created from a GL object(s).
		*				CL_INVALID_EVENT_WAIT_LIST if pclEventWaitList is NULL and 
		*										uiNumEventsInWaitList > 0, or pclEventWaitList is 
		*										not NULL and uiNumEventsInWaitList is 0, or if 
		*										event objects in pclEventWaitList are not valid 
		*										events
		*				CL_OUT_OF_HOST_MEMORY	if there is a failure to allocate resources 
		*										required by the OpenCL implementation on the host
		* Author:		Uri Levy
		* Date:			June 2009
		******************************************************************************************/
		virtual cl_err_code EnqueueAcquireGLObjects(cl_command_queue  IN clCommandQueue,
													cl_uint           IN uiNumObjects,
													const cl_mem *    IN pclMemObjects,
													cl_uint           IN uiNumEventsInWaitList,
													const cl_event *  IN pclEventWaitList,
													cl_event *       OUT pclEvent) = 0;

		/******************************************************************************************
		* Function: 	EnqueueReleaseGLObjects    
		* Description:	used to release OpenCL memory objects that have been created from OpenGL 
		*				objects. These objects need to be released before they can be used by 
		*				OpenGL. The OpenGL objects are released by the OpenCL context associated 
		*				with clCommandQueue
		* Arguments:	clCommandQueue [in] -		is a valid command-queue. All devices used to 
		*											create the OpenCL context associated with 
		*											command_queue must support acquiring shared 
		*											CL/GL objects. This constraint is enforced at 
		*											context creation time
		*				uiNumObjects [in] -			is the number of memory objects to be acquired
		*											in pclMemObjects
		*				pclMemObjects [in] -		is a pointer to a list of CL memory objects 
		*											that correspond to GL objects
		*				uiNumEventsInWaitList [in]-	specify events that need to complete before 
		*											this particular command can be executed. 
		*											If pclEventWaitList is NULL, then this 
		*											particular command does not wait on any event 
		*											to complete. If pclEventWaitList is NULL, 
		*											uiNumEventsInWaitList must be 0. If 
		*											pclEventWaitList is not NULL, the list of 
		*											events pointed to by event_wait_list must be 
		*											valid and uiNumEventsInWaitList must be greater
		*											than 0. The events specified in
		*											pclEventWaitList act as synchronization points
		*				pclEvent [in] -				returns an event object that identifies this 
		*											command and can be used to query or queue a 
		*											wait for the command to complete. event can be
		*											NULL in which case it will not be possible for
		*											the application to query the status of this 
		*											command or queue a wait for this command to 
		*											complete
		* Return value:	CL_SUCCESS				if the function is executed successfully. If
		*										uiNumObjects is 0 and pclMemObjects is NULL the 
		*										function does nothing
		*				CL_INVALID_VALUE		if uiNumObjects is zero and pclMemObjects is not a
		*										NULL value or if uiNumObjects > 0 and pclMemObjects
		*										is NULL
		*				CL_INVALID_MEM_OBJECT	if memory objects in pclMemObjects are not valid 
		*										OpenCL memory objects
		*				CL_INVALID_COMMAND_QUEUE if clCommandQueue is not a valid command-queue
		*				CL_INVALID_CONTEXT		if context associated with clCommandQueue was not
		*										created from an OpenGL context
		*				CL_INVALID_GL_OBJECT	if memory objects in pclMemObjects have not been
		*										created from a GL object(s).
		*				CL_INVALID_EVENT_WAIT_LIST if pclEventWaitList is NULL and 
		*										uiNumEventsInWaitList > 0, or pclEventWaitList is 
		*										not NULL and uiNumEventsInWaitList is 0, or if 
		*										event objects in pclEventWaitList are not valid 
		*										events
		*				CL_OUT_OF_HOST_MEMORY	if there is a failure to allocate resources 
		*										required by the OpenCL implementation on the host
		* Author:		Uri Levy
		* Date:			June 2009
		******************************************************************************************/
		virtual cl_err_code EnqueueReleaseGLObjects(cl_command_queue  IN clCommandQueue,
													cl_uint           IN uiNumObjects,
													const cl_mem *    IN pclMemObjects,
													cl_uint           IN uiNumEventsInWaitList,
													const cl_event *  IN pclEventWaitList,
													cl_event *       OUT pclEvent) = 0;


	};

}}};
#endif // !defined(OCL_IEXECUTION_GL_H_)
