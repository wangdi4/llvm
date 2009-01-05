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
//  iplatform.h
//  Implementation of the IContext interface
//  Created on:      10-Dec-2008 2:08:23 PM
//  Original author: ulevy
///////////////////////////////////////////////////////////////////////////////////////////////////

#if !defined(OCL_ICONTEXT_H_)
#define OCL_ICONTEXT_H_

#include <cl_types.h>

namespace Intel { namespace OpenCL { namespace Framework {

	/**********************************************************************************************
	* Class name:	IContext
	*
	* Description:	IContext class
	* Author:		Uri Levy
	* Date:			December 2008
	**********************************************************************************************/
	class IContext
	{
	public:

		/******************************************************************************************
		* Function: 	clCreateContext    
		* Description:	creates an OpenCL context. An OpenCL context is created with one or more 
		*				devices. Contexts are used by the OpenCL runtime for managing objects such 
		*				as command-queues, memory, program and kernel objects and for executing 
		*				kernels on one or more devices specified in the context
		* Arguments:	properties [in] -	is reserved and must be zero	
		*				num_devices [in] -	is the number of devices specified in the devices 
		*									argument
		*				devices [in] -		is a pointer to a list of unique devices returned by 
		*									clGetDeviceIDs. If more than one device is specified in 
		*									devices, an implementation-defined1 selection criteria 
		*									may be applied to determine if the list of devices 
		*									specified can be used together to create a context
		*				pfn_notify [in] -	is a callback function that can be registered by the 
		*									application. This callback function will be used by the 
		*									OpenCL implementation to report information on errors 
		*									that occur in this context. This callback function may 
		*									be called asynchronously by the OpenCL implementation.
		*									It is the application’s responsibility to ensure that 
		*									the callback function is thread-safe. The parameters to 
		*									this callback function are:
		*									errinfo					is a pointer to an error string.
		*									private_info and cb		represent a pointer 
		*															to binary data that is returned 
		*															by the OpenCL implementation  
		*															that can be used to log  
		*															additional information helpful 
		*															in debugging the error.
		*									user_data				is a pointer to user supplied 
		*															data.
		*									If pfn_notify is NULL, no callback function is registered
		*				user_data [in]		will be passed as the user_data argument when pfn_notify 
		*									is called. user_data can be NULL
		*				errcode_ret [out]	will return an appropriate error code. If errcode_ret is 
		*									NULL, no error code is returned
		* Return value:	CL_INVALID_VALUE 		if properties is not zero or if devices is NULL or if 
		*										num_devices is equal to zero
		*				CL_INVALID_DEVICE 		if devices contains an invalid device
		*				CL_INVALID_DEVICE_LIST	if more than one device is specified in devices and 
		*										the list of devices specified cannot be used together 
		*										to create a context
		*				CL_DEVICE_NOT_AVAILABLE if a device in devices is currently not available even 
		*										though the device was returned by clGetDeviceIDs
		*				CL_OUT_OF_HOST_MEMORY	if there is a failure to allocate resources required 
		*										by the OpenCL implementation on the host
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/
		virtual cl_context	CreateContext(	cl_context_properties properties,
											cl_uint num_devices,
											const cl_device_id *devices,
											logging_fn pfn_notify,
											void *user_data,
											cl_err_code *errcode_ret ) = 0;

		/******************************************************************************************
		* Function: 	RetainContext    
		* Description:	increments the context reference count
		* Arguments:	context [in] -	specifies the OpenCL context being queried	
		* Return value:	CL_SUCCESS				if the function is executed successfully
		*				CL_INVALID_CONTEXT		if context is not a valid OpenCL context
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/		
		virtual cl_err_code	RetainContext( cl_context context ) = 0;

		/******************************************************************************************
		* Function: 	ReleaseContext    
		* Description:	decrements the context reference count. After the context reference count 
		*				becomes zero and all the objects attached to context (such as memory 
		*				objects, command-queues) are released, the context is deleted.
		* Arguments:	context [in] -	specifies the OpenCL context being queried	
		* Return value:	CL_SUCCESS				if the function is executed successfully
		*				CL_INVALID_CONTEXT		if context is not a valid OpenCL context
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/
		virtual cl_err_code ReleaseContext( cl_context context ) = 0;

		/******************************************************************************************
		* Function: 	GetContextInfo    
		* Description:	can be used to query information about a context
		* Arguments:	context [in] -				specifies the OpenCL context being queried	
		*				param_name [in] -			is an enum that specifies the information to 
		*											query
		*				param_value [out] -			is a pointer to memory where the appropriate  
		*											result being queried is returned. If param_value
		*											is NULL, it is ignored
		*				param_value_size [in] -		specifies the size in bytes of memory pointed 
		*											to by param_value. This size must be greater 
		*											than or equal to the size of return type
		*				param_value_size_ret [out]-	returns the actual size in bytes of data being 
		*											queried by param_value. If param_value_size_ret 
		*											is NULL, it is ignored
		* Return value:	CL_SUCCESS				if the function is executed successfully
		*				CL_INVALID_CONTEXT		if context is not a valid OpenCL context
		*				CL_INVALID_VALUE		if param_name is not one of the supported values 
		*										or if size in bytes specified by param_value_size
		*										is < size of return type and param_value is not a 
		*										NULL value
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/
		virtual cl_err_code	GetContextInfo(	cl_context      context,
											cl_context_info param_name,
											size_t          param_value_size,
											void *          param_value,
											size_t *        param_value_size_ret ) = 0;

		/******************************************************************************************
		* Function: 	CreateProgramWithSource    
		* Description:	creates a program object for a context, and loads the source code specified 
		*				by the text strings in the strings array into the program object. The 
		*				devices associated with the program object are the devices associated with 
		*				context
		* Arguments:	clContext [in] -	must be a valid OpenCL context	
		*				ppcStrings [in] -	is an array of uiCount pointers to optionally null-
		*									terminated character strings that make up the source
		*									code
		*				szLengths [in] -	is an array with the number of chars in each string 
		*									(the string length). If an element in lengths is zero, 
		*									its accompanying string is null-terminated. If lengths 
		*									is NULL, all strings in the strings argument are 
		*									considered null-terminated. Any length value passed in 
		*									that is greater than zero excludes the null terminator 
		*									in its count
		*				pErrcodeRet [out] -	will return an appropriate error code. If errcode_ret 
		*									is NULL, no error code is returned
		* Return value:	CL_SUCCESS					the program object is created successfully
		*				CL_INVALID_CONTEXT			if context is not a valid OpenCL context
		*				CL_INVALID_VALUE			if count is zero or if strings or any entry in
		*											strings is NULL
		*				CL_COMPILER_NOT_AVAILABLE	if a compiler is not available i.e. 
		*											CL_DEVICE_COMPILER_AVAILABLE is set to 
		*											CL_FALSE
		*				CL_OUT_OF_HOST_MEMORY		if there is a failure to allocate resources 
		*											required by the OpenCL implementation on the 
		*											host
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/
		virtual cl_program CreateProgramWithSource(	cl_context     clContext,
													cl_uint        uiCount,
													const char **  ppcStrings,
													const size_t * szLengths,
													cl_int *       pErrcodeRet ) = 0;

	};

}}};
#endif // !defined(OCL_ICONTEXT_H_)
