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

#include <cl_framework.h>
#include "..\cl_object_info.h"
#include "..\cl_objects_map.h"
#include "logger.h"
#include "device.h"
#include "cl_device_api.h"
#include "cl_config.h"

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
		cl_err_code		Initialize(ConfigFile * pConfigFile);

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

		/******************************************************************************************
		* Function: 	GetPlatformInfo    
		* Description:	OpenCL API function - obtain the list of available devices
		* Arguments:	device_type [in]	is a bitfield that identifies the type of OpenCL device. 
		*									The device_type can be used to query specific OpenCL 
		*									devices or all OpenCL devices available.
		*				num_entries [in]	is the number of cl_device entries that can be added to
		*									devices. If devices is not NULL, the num_entries must 
		*									be greater than zero
		*				devices [out]		returns a list of OpenCL devices found. The cl_device_id 
		*									values returned in devices can be used to identify a 
		*									specific OpenCL device. If devices argument is NULL, 
		*									this argument is ignored. The number of OpenCL devices 
		*									returned is the mininum of value specified by num_entries 
		*									or the number of OpenCL devices whose type matches 
		*									device_type
		*				num_devices [out]	returns the number of OpenCL devices available that match 
		*									device_type. If num_devices is NULL, this argument is ignored.
		* Return value:	CL_INVALID_DEVICE_TYPE	device_type is not a valid value
		*				CL_INVALID_VALUE		num_entries is equal to zero and devices is not NULL
		*										or if both num_devices and devices are NULL
		*				CL_DEVICE_ NOT_FOUND	no OpenCL devices that matched device_type were found
		*				CL_SUCCESS				the function is executed successfully
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/
		cl_err_code		GetDeviceIDs(	cl_device_type device_type, 
										cl_uint num_entries, 
										cl_device_id* devices, 
										cl_uint* num_devices );

		/******************************************************************************************
		* Function: 	GetDeviceInfo    
		* Description:	OpenCL API function - gets specific information about an OpenCL device
		*				The application can query specific capabilities of the OpenCL device(s) 
		*				returned by clGetDeviceIDs. This can be used by the application to 
		*				determine which device(s) to use
		* Arguments:	device [in]					is a device returned by clGetDeviceIDs
		*				param_name [in]				is an enum that identifies the device 
		*											information being queried
		*				param_value [in]			is a pointer to memory location where 
		*											appropriate values for a given param_name will 
		*											be returned. If param_value is NULL, it is 
		*											ignored
		*				param_value_size [in]		specifies the size in bytes of memory pointed 
		*											to by param_value. This size in bytes must 
		*											be >= size of return type
		*				param_value_size_ret [in]	returns the actual size in bytes of data being 
		*											queried by param_value. If param_value_size_ret 
		*											is NULL, it is ignored
		* Return value:	CL_SUCCESS				the function is executed successfully
		*				CL_INVALID_DEVICE		device is not valid
		*				CL_INVALID_VALUE		param_name is not one of the supported values or if 
		*										size in bytes specified by param_value_size is < 
		*										size of return type and param_value is not a NULL 
		*										value
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/
		cl_err_code		GetDeviceInfo(cl_device_id device,
									  cl_device_info param_name, 
									  size_t param_value_size, 
									  void* param_value,
									  size_t* param_value_size_ret );


	private:

		/******************************************************************************************
		* Function: 	InitDevices
		* Description:	Load OpenCL devices and initialize them. all device dll's must be located
		*				in the same folder as the framework dll
		* Arguments:	vector<string> devices -	list of device dll names
		* Return value:	CL_SUCCESS - The initializtion operation succeded
		*				CL_ERR_DEVICE_INIT_FAIL - one or more devices falied to initialize
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/
		cl_err_code InitDevices(vector<string> devices);

		// map list of devices
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
