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
//  PlatformModule.h
//  Implementation of the Class PlatformModule
//  Created on:      10-Dec-2008 2:08:23 PM
//  Original author: ulevy
///////////////////////////////////////////////////////////////////////////////////////////////////

#if !defined(OCL_PLATFORM_MODULE_H_)
#define OCL_PLATFORM_MODULE_H_

#include "cl_framework.h"
#include "..\OCLObjectInfo.h"
#include "..\OCLObjectsMap.h"
#include "Logger.h"
#include "Device.h"

namespace Intel { namespace OpenCL { namespace Framework {

	/**********************************************************************************************
	* Class name:	PlatformModule
	*
	* Description:	platform module class
	* Author:		Uri Levy
	* Date:			December 2008
	**********************************************************************************************/
	class PlatformModule
	{
	
	public:

		/******************************************************************************************
		* Function: 	PlatformModule
		* Description:	The Platform Module class constructor
		* Arguments:		
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/
		PlatformModule();

		/******************************************************************************************
		* Function: 	~PlatformModule
		* Description:	The Platform Module class destructor
		* Arguments:		
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/
		virtual ~PlatformModule();

		/******************************************************************************************
		* Function: 	Initialize    
		* Description:	Initialize the platform module object: set platform's information
		*				and load devices
		* Arguments:		
		* Return value:	CL_SUCCESS - The initializtion operation succeded
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/		
		cl_err_code		Initialize();

		/******************************************************************************************
		* Function: 	Release    
		* Description:	Release the platform module's resources
		* Arguments:		
		* Return value:	CL_SUCCESS - The release operation succeded
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/
		cl_err_code		Release();

		/******************************************************************************************
		* Function: 	GetPlatformInfo    
		* Description:	OpenCL API function - gets specific information about the OpenCL 
		*				platform
		* Arguments:	param_name [in]				is an enum that identifies the platform  
		*											information being queried
		*				param_value [inout]			is a pointer to memory location where appropriate
		*											values for a given param_name will be returned. 
		*											If param_value is NULL, it is ignored.
		*				param_value_size [in]		param_value_size specifies the size in bytes 
		*											of memory pointed to by param_value. This size
		*											in bytes must be >= size of return type
		*				param_value_size_ret [out]	returns the actual size in bytes of 
		*											data being queried by param_value. If 
		*											param_value_size_ret is NULL, it is ignored.
		* Return value:	CL_SUCCESS			the function is executed successfully
		*				CL_INVALID_VALUE	param_name is not one of the supported values or if size 
		*									in bytes specified by param_value_size is < size of 
		*									return type and param_value is not a NULL value.
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/
		cl_err_code		GetPlatformInfo(cl_platform_info param_name, 
										size_t param_value_size, 
										void* param_value, 
										size_t* param_value_size_ret );
		
		cl_err_code		GetDeviceIDs(	cl_device_type device_type, 
										cl_uint num_entries, 
										cl_device_id* devices, 
										cl_uint* num_devices );
		
		cl_err_code		clGetDeviceInfo(cl_device_id device,
										cl_device_info param_name, 
										size_t param_value_size, 
										void* param_value,
										size_t* param_value_size_ret );

	private:

		OCLObjectsMap * m_pDevices;
		
		// pointer to the platoform module's logger client
		LoggerClient * m_pPlatformLoggerClient;

		// pointer to the platform module's information object
		OCLObjectInfo *	m_pObjectInfo;
		
		// static chars array - holds the platform's information string
		static const char m_vPlatformInfoStr[];
		// platform's information string length
		static const unsigned int m_uiPlatformInfoStrSize;

		// static chars array - holds the platform's version string
		static const char m_vPlatformVersionStr[];
		// platform's version string length
		static const unsigned int m_uiPlatformVersionStrSize;

	};

}}};
#endif // !defined(OCL_PLATFORM_MODULE_H_)
