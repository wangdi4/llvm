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
//  Implementation of the IPlatform interface
//  Created on:      10-Dec-2008 2:08:23 PM
//  Original author: ulevy
///////////////////////////////////////////////////////////////////////////////////////////////////

#if !defined(OCL_IPLATFORM_H_)
#define OCL_IPLATFORM_H_

#include <cl_types.h>

namespace Intel { namespace OpenCL { namespace Framework {

	/**********************************************************************************************
	* Class name:	IPlatform
	*
	* Description:	IPlatform interface - outlines the OpenCL platform related functions
	* Author:		Uri Levy
	* Date:			December 2008
	**********************************************************************************************/
	class IPlatform
	{
	public:
        virtual ~IPlatform(){};

		/******************************************************************************************
		* Function: 	GetPlatformIDs    
		* Description:	OpenCL API function - get the list of platforms available
		* Arguments:	uiNumEntries [in]		the number of cl_platform_id entries that can be  
		*										added to platforms. If platforms is not NULL, the 
		*										num_entries must be greater than zero.  
		*				pclPlatforms [out]		returns a list of OpenCL platforms found. The 
		*										cl_platform_id values returned in platforms can be 
		*										used to identify a specific OpenCL platform. If  
		*										platforms argument is NULL, this argument is 
		*										ignored. The number of OpenCL platforms returned  
		*										is the mininum of the value specified by  
		*										uiNumEntries or the number of OpenCL platforms
		*										available
		*				puiNumPlatforms [in]	returns the number of OpenCL platforms available. 
		*										If puiNumPlatforms is NULL, this argument is
		*										ignored
		* Return value:	CL_SUCCESS			the function is executed successfully
		*				CL_INVALID_VALUE	if uiNumEntries is equal to zero and platforms is not 
		*									NULL or if both puiNumPlatforms and pclPlatforms are 
		*									NULL
		* Author:		Uri Levy
		* Date:			April 2008
		******************************************************************************************/
		virtual cl_int GetPlatformIDs (	cl_uint          IN  uiNumEntries, 
										cl_platform_id * OUT pclPlatforms, 
										cl_uint *        OUT puiNumPlatforms) = 0;

		/******************************************************************************************
		* Function: 	GetPlatformInfo    
		* Description:	OpenCL API function - gets specific information about the OpenCL 
		*				platform
		* Arguments:	clParamName [in]			is an enum that identifies the platform  
		*											information being queried
		*				pParamValue [inout]			is a pointer to memory location where appropriate
		*											values for a given param_name will be returned. 
		*											If param_value is NULL, it is ignored.
		*				szParamValueSize [in]		param_value_size specifies the size in bytes 
		*											of memory pointed to by param_value. This size
		*											in bytes must be >= size of return type
		*				pszParamValueSizeRet [out]	returns the actual size in bytes of 
		*											data being queried by param_value. If 
		*											param_value_size_ret is NULL, it is ignored.
		* Return value:	CL_SUCCESS			the function is executed successfully
		*				CL_INVALID_VALUE	param_name is not one of the supported values or if size 
		*									in bytes specified by param_value_size is < size of 
		*									return type and param_value is not a NULL value.
		*				CL_INVALID_PLATFORM	if platform is not a valid platform
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/
		virtual cl_int	GetPlatformInfo(cl_platform_id   IN    clPlatform,
										cl_platform_info IN    clParamName, 
										size_t           IN    szParamValueSize, 
										void*            INOUT pParamValue, 
										size_t*          OUT   pszParamValueSizeRet) = 0;

		/******************************************************************************************
		* Function: 	GetPlatformInfo    
		* Description:	OpenCL API function - obtain the list of available devices on a platform
		*				clPlatform [in]		refers to the platform ID returned by clGetPlatformsIDs 
		*									or can be NULL. If platform is NULL, the behavior is 
		*									implementation-defined
		* Arguments:	clPlatform [in]		refers to the platform ID returned by clGetPlatformsIDs 
		*									or can be NULL. If platform is NULL, the behavior is 
		*									implementation-defined
		*				clDeviceType [in]	is a bitfield that identifies the type of OpenCL device. 
		*									The device_type can be used to query specific OpenCL 
		*									devices or all OpenCL devices available.
		*				uiNumEntries [in]	is the number of cl_device entries that can be added to
		*									devices. If devices is not NULL, the num_entries must 
		*									be greater than zero
		*				pclDevices [out]	returns a list of OpenCL devices found. The cl_device_id 
		*									values returned in devices can be used to identify a 
		*									specific OpenCL device. If devices argument is NULL, 
		*									this argument is ignored. The number of OpenCL devices 
		*									returned is the mininum of value specified by num_entries 
		*									or the number of OpenCL devices whose type matches 
		*									device_type
		*				puiNumDevices [out]	returns the number of OpenCL devices available that match 
		*									device_type. If num_devices is NULL, this argument is ignored.
		* Return value:	CL_INVALID_DEVICE_TYPE	device_type is not a valid value
		*				CL_INVALID_VALUE		num_entries is equal to zero and devices is not NULL
		*										or if both num_devices and devices are NULL
		*				CL_DEVICE_ NOT_FOUND	no OpenCL devices that matched device_type were found
		*				CL_SUCCESS				the function is executed successfully
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/
		virtual cl_int	GetDeviceIDs(	cl_platform_id IN  clPlatform,
										cl_device_type IN  clDeviceType, 
										cl_uint        IN  uiNumEntries, 
										cl_device_id*  OUT pclDevices, 
										cl_uint*       OUT puiNumDevices ) = 0;

		/******************************************************************************************
		* Function: 	GetDeviceInfo    
		* Description:	OpenCL API function - gets specific information about an OpenCL device
		*				The application can query specific capabilities of the OpenCL device(s) 
		*				returned by clGetDeviceIDs. This can be used by the application to 
		*				determine which device(s) to use
		* Arguments:	clDevice [in]				is a device returned by clGetDeviceIDs
		*				clParamName [in]			is an enum that identifies the device 
		*											information being queried
		*				pParamValue [out]			is a pointer to memory location where 
		*											appropriate values for a given param_name will 
		*											be returned. If param_value is NULL, it is 
		*											ignored
		*				szParamValueSize [in]		specifies the size in bytes of memory pointed 
		*											to by param_value. This size in bytes must 
		*											be >= size of return type
		*				pszParamValueSizeRet [out]	returns the actual size in bytes of data being 
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
		virtual cl_int GetDeviceInfo(	cl_device_id   IN  clDevice,
										cl_device_info IN  clParamName,
										size_t         IN  szParamValueSize,
										void*          OUT pParamValue,
										size_t*        OUT pszParamValueSizeRet ) = 0;

		/******************************************************************************************
		* Function: 	UnloadCompiler    
		* Description:	allows the implementation to release the resources allocated by the OpenCL 
		*				compiler. This is a hint from the application and does not guarantee that 
		*				the compiler will not be used in the future or that the compiler will 
		*				actually be unloaded by the implementation. Calls to clBuildProgram after
		*				clUnloadCompiler will reload the compiler, if necessary, to build the 
		*				appropriate program executable. 
		* Arguments:	
		* Return value:	This call currently always returns CL_SUCCESS
		* Author:		Uri Levy
		* Date:			January 2009
		******************************************************************************************/
		virtual cl_int UnloadCompiler( void ) = 0;

	};

}}};
#endif // !defined(OCL_IPLATFORM_H_)
