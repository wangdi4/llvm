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

#include <cl_types.h>
#include <logger.h>
#include "iplatform.h"
#include <vector>

namespace Intel { namespace OpenCL { namespace Framework {

	class OCLObjectInfo;
	class OCLObjectsMap;
	class Device;
	class ConfigFile;

	/**********************************************************************************************
	* Class name:	PlatformModule
	*
	* Description:	platform module class
	* Author:		Uri Levy
	* Date:			December 2008
	**********************************************************************************************/
	class PlatformModule : public IPlatform
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
		* Function: 	GetDevice    
		* Description:	Get device object that asigned to the device id
		* Arguments:	clDeviceId [in] -	device id
		*				ppDevice [out] -	pointer to the device
		* Return value:	CL_SUCCESS - The operation succedeed
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/
		cl_err_code		GetDevice(	cl_device_id IN  clDeviceId, 
									Device **    OUT ppDevice);

		///////////////////////////////////////////////////////////////////////////////////////////
		// IPlatform methods
		///////////////////////////////////////////////////////////////////////////////////////////
		virtual cl_int GetPlatformInfo(cl_platform_info clParamName, size_t szParamValueSize, void* pParamValue, size_t* pszParamValueSizeRet);
		virtual cl_int	GetDeviceIDs(cl_device_type clDeviceType, cl_uint uiNumEntries, cl_device_id* pclDevices, cl_uint* puiNumDevices);
		virtual cl_int	GetDeviceInfo(cl_device_id  clDevice, cl_device_info clParamName, size_t szParamValueSize, void* pParamValue, size_t* pszParamValueSizeRet);

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
		cl_err_code InitDevices(std::vector<std::string> devices);

		// map list of devices
		OCLObjectsMap * m_pDevices;
		
		// pointer to the platoform module's logger client
		Intel::OpenCL::Utils::LoggerClient * m_pLoggerClient;

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
