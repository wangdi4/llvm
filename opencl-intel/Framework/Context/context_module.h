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
//  ContextModule.h
//  Implementation of the Class ContextModule
//  Created on:      10-Dec-2008 2:08:23 PM
//  Original author: ulevy
///////////////////////////////////////////////////////////////////////////////////////////////////

#if !defined(OCL_CONTEXT_MODULE_H_)
#define OCL_CONTEXT_MODULE_H_

#include <cl_types.h>
#include <logger.h>
#include "icontext.h"

namespace Intel { namespace OpenCL { namespace Framework {

	class PlatformModule;
	class Device;
	class Context;
	class OCLObjectsMap;

	/**********************************************************************************************
	* Class name:	ContextModule
	*
	* Description:	context module class
	* Author:		Uri Levy
	* Date:			December 2008
	**********************************************************************************************/
	class ContextModule : IContext
	{
	
	public:

		/******************************************************************************************
		* Function: 	ContextModule
		* Description:	The Context Module class constructor
		* Arguments:	pPlatformModule [in] -	pointer to the platform module	
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/
		ContextModule(PlatformModule *pPlatformModule);

		/******************************************************************************************
		* Function: 	~ContextModule
		* Description:	The Context Module class destructor
		* Arguments:		
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/
		virtual ~ContextModule();

		/******************************************************************************************
		* Function: 	Initialize    
		* Description:	Initialize the context module object
		*				and load devices
		* Arguments:		
		* Return value:	CL_SUCCESS - The initializtion operation succeded
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/		
		cl_err_code		Initialize();

		/******************************************************************************************
		* Function: 	Release    
		* Description:	Release the context module's resources
		* Arguments:		
		* Return value:	CL_SUCCESS - The release operation succeded
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/
		cl_err_code		Release();

		///////////////////////////////////////////////////////////////////////////////////////////
		// IContext methods
		///////////////////////////////////////////////////////////////////////////////////////////
		virtual cl_context CreateContext(cl_context_properties properties, cl_uint num_devices, const cl_device_id *devices, logging_fn pfn_notify, void *user_data, cl_err_code *errcode_ret);
		virtual cl_err_code RetainContext(cl_context context);
		virtual cl_err_code ReleaseContext(cl_context context);
		virtual cl_err_code GetContextInfo(cl_context context, cl_context_info param_name, size_t param_value_size, void * param_value, size_t * param_value_size_ret);
		virtual cl_program CreateProgramWithSource(cl_context clContext, cl_uint uiCount, const char ** ppcStrings, const size_t * szLengths, cl_int * pErrcodeRet);

	private:

		cl_err_code			CheckDevices(cl_uint uiNumDevices, const cl_device_id *pclDeviceIds, Device ** ppDevices);

		PlatformModule *	m_pPlatformModule; // handle to the platform module

		Intel::OpenCL::Utils::LoggerClient *		m_pLoggerClient; // handle to the logger client

		OCLObjectsMap *		m_pContexts; // map list of contexts
		
		Intel::OpenCL::Utils::LoggerClient *		m_pContextLoggerClient; // pointer to the context module's logger client

	};

}}};
#endif // !defined(OCL_CONTEXT_MODULE_H_)
