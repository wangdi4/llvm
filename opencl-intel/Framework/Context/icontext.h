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
	* Description:	IContext iterface
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
		virtual cl_context	CreateContext(	cl_context_properties IN  properties,
											cl_uint               IN  num_devices,
											const cl_device_id *  IN  devices,
											logging_fn            IN  pfn_notify,
											void *                IN  user_data,
											cl_err_code *         OUT errcode_ret ) = 0;

		/******************************************************************************************
		* Function: 	RetainContext    
		* Description:	increments the context reference count
		* Arguments:	context [in] -	specifies the OpenCL context being queried	
		* Return value:	CL_SUCCESS				if the function is executed successfully
		*				CL_INVALID_CONTEXT		if context is not a valid OpenCL context
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/		
		
		virtual cl_err_code	RetainContext( cl_context IN context ) = 0;

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
		virtual cl_err_code ReleaseContext( cl_context IN context ) = 0;

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
		virtual cl_err_code	GetContextInfo(	cl_context      IN  context,
											cl_context_info IN  param_name,
											size_t          IN  param_value_size,
											void *          OUT param_value,
											size_t *        OUT param_value_size_ret ) = 0;

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
		virtual cl_program CreateProgramWithSource(	cl_context     IN  clContext,
													cl_uint        IN  uiCount,
													const char **  IN  ppcStrings,
													const size_t * IN  szLengths,
													cl_int *       OUT pErrcodeRet ) = 0;

		/******************************************************************************************
		* Function: 	CreateProgramWithBinary    
		* Description:	creates a program object for a context, and loads the binary bits specified 
		*				by binary into the program object
		* Arguments:	clContext [in] 		must be a valid OpenCL context	
		*				pclDeviceList [in]	is a pointer to a list of devices that are in context. 
		*									device_list must be a non-NULL value. The binaries are
		*									loaded for devices specified in this list.
		*									The devices associated with the program object will be 
		*									the list of devices specified by device_list. The list 
		*									of devices specified by pclDeviceList must be devices 
		*									associated with context.
		*				uiNumDevices [in] 	is the number of devices listed in pclDeviceList 
		*				pszLengths [in] 	is an array of the size in bytes of the program binaries 
		*									to be loaded for devices specified by pclDeviceList
		*				ppBinaries [in] 	is an array of pointers to program binaries to be loaded
		*									for devices specified by pclDeviceList. For each device 
		*									given by pclDeviceList[i], the pointer to the program 
		*									binary for that device is given by ppBinaries[i] and the
		*									length of this corresponding binary is given by 
		*									pszLengths[i]. pszLengths[i] cannot be zero and 
		*									ppBinaries[i] cannot be a NULL pointer
		*									The program binaries specified by binaries contain the 
		*									bits that describe the program executable that will be 
		*									run on the device(s) associated with context. The 
		*									program binary can consist of either or both:
		*									-	Device-specific executable(s), and/or,
		*									-	Implementation-specific intermediate representation
		*										(IR) which will be converted to the device-specific 
		*										executable.
		*				piBinaryStatus [in]	returns whether the program binary for each device 
		*									specified in device_list wasloaded successfully or not
		*									It is an array of num_devices entries and returns 
		*									CL_SUCCESS in piBinaryStatus[i] if binary was 
		*									successfully loaded for device specified by 
		*									pclDeviceList[i]; otherwise returns CL_INVALID_VALUE if
		*									pszLengths[i] is zero or if ppBinaries[i] is a NULL 
		*									value or CL_INVALID_BINARY in piBinaryStatus[i] if 
		*									program binary is not a valid binary for the specified 
		*									device. If binary_status is NULL, it is ignored.
		*				pErrRet [in]		will return an appropriate error code. If pErrRet is 
		*									NULL, no error code isreturned
		* Return value:	CL_SUCCESS					the program object is created successfully
		*				CL_INVALID_CONTEXT			if context is not a valid OpenCL context
		*				CL_INVALID_VALUE			if pclDeviceList is NULL or uiNumDevicesis
		*											zero or if pszLengths or ppBinaries are NULL 
		*											or if any entry in pszLengths[i] is zero or 
		*											ppBinaries[i] is NULL
		*				CL_INVALID_DEVICE			if OpenCL devices listed in pclDeviceList are 
		*											not in the list of devices associated with 
		*											context
		*				CL_INVALID_BINARY			if an invalid program binary was encountered
		*											for any device. piBinaryStatus will return 
		*											specific status for each device
		*				CL_OUT_OF_HOST_MEMORY		if there is a failure to allocate resources 
		*											required by the OpenCL implementation on the 
		*											host
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/
		virtual cl_program CreateProgramWithBinary(	cl_context           IN  clContext,
													cl_uint              IN  uiNumDevices,
													const cl_device_id * IN  pclDeviceList,
													const size_t *       IN  pszLengths,
													const void **        IN  ppBinaries,
													cl_int *             OUT piBinaryStatus,
													cl_int *             OUT pErrRet ) = 0;

		/******************************************************************************************
		* Function: 	RetainProgram    
		* Description:	increments the context reference count
		* Arguments:	clProgram [in] -	specifies the OpenCL program being queried	
		* Return value:	CL_SUCCESS				if the function is executed successfully
		*				CL_INVALID_PROGRAM		if clProgram is not a valid OpenCL program
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/		
		virtual cl_err_code	RetainProgram( cl_program IN clProgram ) = 0;

		/******************************************************************************************
		* Function: 	ReleaseProgram    
		* Description:	decrements the program reference count. The program object is deleted after
		*				all kernel objects associated with program have been deleted and the 
		*				program reference count becomes zero.
		* Arguments:	clProgram [in] -	specifies the OpenCL program being queried	
		* Return value:	CL_SUCCESS				if the function is executed successfully
		*				CL_INVALID_CONTEXT		if clProgram is not a valid OpenCL program
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/
		virtual cl_err_code ReleaseProgram( cl_program IN clProgram ) = 0;

		/******************************************************************************************
		* Function: 	BuildProgram    
		* Description:	builds (compiles & links) a program executable from the program source or 
		*				binary for all the devices or a specific device(s) in the OpenCL context 
		*				associated with program. OpenCL allows program executables to be built 
		*				using the source or the binary
		* Arguments:	clProgram [in]		is the program object	
		*				uiNumDevices [in]	is the number of devices listed in pclDeviceList
		*				pclDeviceList [in]	is a pointer to a list of devices associated with 
		*									program. If device_list is a NULL value, the program 
		*									executable is built for all devices associated with 
		*									program for which a source or binary has been loaded.
		*									If pclDeviceList is a non-NULL value, the program
		*									executable is built for devices specified in this list
		*									for which a source or binary has been loaded
		*				pcOptions [in]		is a pointer to a string that describes the build 
		*									options to be used for building the program executable
		*				pfn_notify [in]		is a function pointer to a notification routine. The 
		*									notification routine allows an application to register
		*									a callback function which will be called when the 
		*									program executable has been built (successfully or 
		*									unsuccessfully). If pfn_notify is not NULL, 
		*									BuildProgram does not need to wait for the build to 
		*									complete and can return immediately. If pfn_notify is
		*									NULL, BuildProgram does not return until the build has
		*									completed. This callback function may be called 
		*									asynchronously by the OpenCL implementation. It is the
		*									application’s responsibility to ensure that the callback
		*									function is thread-safe
		*				pUserData [in]		will be passed as an argument when pfn_notify is called.
		*									pUserData can be NULL
		* Return value:	CL_SUCCESS					if the function is executed successfully
		*				CL_INVALID_PROGRAM			if clProgram is not a valid OpenCL program
		*				CL_INVALID_VALUE			if pclDeviceList is NULL and num_devices is 
		*											greater than zero, or if pclDeviceList is not 
		*											NULL and num_devices is zero
		*				CL_INVALID_DEVICE			if OpenCL devices listed in pclDeviceList are
		*											not in the list of devices associated with 
		*											program
		*				CL_INVALID_BINARY			if program is created with 
		*											CreateWithProgramBinary and devices listed in
		*											pclDeviceList do not have a valid program 
		*											binary loaded
		*				CL_INVALID_BUILD_OPTIONS	if the build pcOptions specified by options are
		*											invalid
		*				CL_INVALID_OPERATION		if the build of a program executable for any of
		*											the devices listed in pclDeviceList by a 
		*											previous call to clBuildProgram for program has
		*											not completed or if there are kernel objects 
		*											attached to program
		*				CL_OUT_OF_HOST_MEMORY		if there is a failure to allocate resources 
		*											required by the OpenCL implementation on the
		*											host
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/
		virtual cl_int BuildProgram(cl_program           IN clProgram,
									cl_uint              IN uiNumDevices,
									const cl_device_id * IN pclDeviceList,
									const char *         IN pcOptions, 
									void (*pfn_notify)(cl_program program, void * user_data),
									void *               IN pUserData ) = 0;


	};

}}};
#endif // !defined(OCL_ICONTEXT_H_)
