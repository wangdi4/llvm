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
#include <logger.h>
#include "iplatform.h"
#include <vector>
using namespace Intel::OpenCL::Utils;

namespace Intel { namespace OpenCL { namespace Framework {

#define CL_PLATFORM_ID_DEFAULT	0x0
#define CL_PLATFORM_ID_INTEL	0x1

	class OCLObjectInfo;
	class OCLObjectsMap;
	class Device;
	class OCLConfig;
	class FECompiler;

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
		cl_err_code		Initialize(ocl_entry_points * pOclEntryPoints, OCLConfig * pConfig);

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
		cl_err_code		GetDevice(cl_device_id IN  clDeviceId, Device ** OUT ppDevice);

		/******************************************************************************************
		* Function: 	GetDevices    
		* Description:	Get device objects 
		* Arguments:	puiNumDevices [in] -	pointer to number of devices
		* Return value:	Device ** - Devices list
		* Author:		Uri Levy
		* Date:			July 2007
		******************************************************************************************/
		Device ** GetDevices(unsigned int * puiNumDevices)
		{ 
			if (NULL != puiNumDevices)
			{
				*puiNumDevices = m_uiDevicesCount;
			}
			return m_ppDevices; 
		}

		
		// get default front-end compiler
		virtual FECompiler * GetDefaultFECompiler();

		///////////////////////////////////////////////////////////////////////////////////////////
		// IPlatform methods
		///////////////////////////////////////////////////////////////////////////////////////////
		virtual cl_int GetPlatformIDs(cl_uint uiNumEntries, cl_platform_id * pclPlatforms, cl_uint * puiNumPlatforms);
		virtual cl_int GetPlatformInfo(cl_platform_id clPlatform, cl_platform_info clParamName, size_t szParamValueSize, void* pParamValue, size_t* pszParamValueSizeRet);
		virtual cl_int	GetDeviceIDs(cl_platform_id clPlatform, cl_device_type clDeviceType, cl_uint uiNumEntries, cl_device_id* pclDevices, cl_uint* puiNumDevices);
		virtual cl_int	GetDeviceInfo(cl_device_id  clDevice, cl_device_info clParamName, size_t szParamValueSize, void* pParamValue, size_t* pszParamValueSizeRet);
		virtual cl_int UnloadCompiler(void);

	private:

		/******************************************************************************************
		* Function: 	InitDevices
		* Description:	Load OpenCL devices and initialize them. all device dll's must be located
		*				in the same folder as the framework dll
		* Arguments:	vector<string> devices -	list of device dll names
		*				string defaultDevice -		name of default device agent's dll name
		*				vector<string> compilers -	list of fron-end compilers dll names
		*				string defaultCompiler -	name of default front-end compiler's dll name
		* Return value:	CL_SUCCESS - The initializtion operation succeded
		*				CL_ERR_DEVICE_INIT_FAIL - one or more devices falied to initialize
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/
		cl_err_code InitDevices(const std::vector<std::string>& devices, const std::string& defaultDevice);

		/******************************************************************************************
		* Function: 	InitFECompilers
		* Description:	Load OpenCL front-end compilers and initialize them. all fe compilers dll's 
		*				must be located in the same folder as the framework dll
		* Arguments:	vector<string> compilers -	list of fron-end compilers dll names
		*				string defaultCompiler -	name of default front-end compiler's dll name
		* Return value:	CL_SUCCESS - The initializtion operation succeded
		*				CL_ERR_FE_COMPILER_INIT_FAIL - one or more devices falied to initialize
		* Author:		Uri Levy
		* Date:			March 2008
		******************************************************************************************/
		cl_err_code InitFECompilers(const std::vector<std::string>& compilers, const std::string& defaultCompiler);

		cl_err_code ReleaseFECompilers();

		bool CheckPlatformId(cl_platform_id clPlatform) const { return (clPlatform==m_clPlatformIds[0]) ||
																	   (clPlatform==NULL); }

		cl_platform_id	m_clPlatformIds[2];
		
		// map list of devices
		OCLObjectsMap * m_pDevices;

		Device **		m_ppDevices;

		cl_device_id *	m_pDeviceIds;

		unsigned int	m_uiDevicesCount;

		// default device
		Device * m_pDefaultDevice;

		// map list of all front-end compilers
		OCLObjectsMap * m_pFECompilers;

		// default front-end compiler
		FECompiler * m_pDefaultFECompiler;

		// static chars array - holds the platform's information string
		static const char m_vPlatformInfoStr[];
		// platform's information string length
		static const unsigned int m_uiPlatformInfoStrSize;

		// static chars array - holds the platform's version string
		static const char m_vPlatformVersionStr[];
		// platform's version string length
		static const unsigned int m_uiPlatformVersionStrSize;

		static const char m_vPlatformNameStr[];
		static const unsigned int m_uiPlatformNameStrSize;

		static const char m_vPlatformVendorStr[];
		static const unsigned int m_uiPlatformVendorStrSize;

		ocl_entry_points * m_pOclEntryPoints;

		DECLARE_LOGGER_CLIENT;

	};

}}};
#endif // !defined(OCL_PLATFORM_MODULE_H_)
