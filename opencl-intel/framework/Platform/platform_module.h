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

#include "cl_framework.h"
#include "cl_synch_objects.h"
#include "iplatform.h"
#include "ocl_itt.h"
#include "cl_objects_map.h"
#include <Logger.h>
#include <vector>

namespace Intel { namespace OpenCL { namespace Framework {

#define CL_PLATFORM_ID_DEFAULT	0x0
#define CL_PLATFORM_ID_INTEL	0x1

	class OCLObjectInfo;
	template <class HandleType> class OCLObjectsMap;
	class Device;
    class FissionableDevice;
	class OCLConfig;
	class FrontEndCompiler;

	/**********************************************************************************************
	* Class name:	PlatformModule
	*
	* Description:	platform module class
	* Author:		Uri Levy
	* Date:			December 2008
	**********************************************************************************************/
	class PlatformModule : public OCLObjectBase, public IPlatform
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
		* Return value:	CL_SUCCESS - The initialization operation succeeded
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/		
		cl_err_code		Initialize(ocl_entry_points * pOclEntryPoints, OCLConfig * pConfig, ocl_gpa_data * pGPAData);

		/******************************************************************************************
		* Function: 	Release    
		* Description:	Release the platform module's resources
		* Arguments:		
		* Return value:	CL_SUCCESS - The release operation succeeded
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/
		cl_err_code		Release();

		/******************************************************************************************
		* Function: 	GetRootDevice    
		* Description:	Get the root device object that assigned to the device id
		* Arguments:	clDeviceId [in] -	device id
		*				ppDevice [out] -	pointer to the device
		* Return value:	CL_SUCCESS - The operation succeeded
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/
		cl_err_code		GetRootDevice(cl_device_id IN  clDeviceId, Device ** OUT ppDevice);

        /******************************************************************************************
        * Function: 	GetDevice    
        * Description:	Get the device object that assigned to the device id
        * Arguments:	clDeviceId [in] -	device id
        *				ppDevice [out] -	pointer to the device
        * Return value:	CL_SUCCESS - The operation succeeded
        * Author:		Doron Singer
        * Date:			March 2011
        ******************************************************************************************/
        cl_err_code		GetDevice(cl_device_id IN  clDeviceId, FissionableDevice ** OUT ppDevice);

		/******************************************************************************************
        * Function: 	GetGPAData    
        * Description:	Returns a pointer to the GPA data object.
        * Arguments:	None
        * Return value:	ocl_gpa_data
        * Author:		Oded Perez
        * Date:			July 2011
        ******************************************************************************************/
		ocl_gpa_data * GetGPAData() const { return m_pGPAData; }

		///////////////////////////////////////////////////////////////////////////////////////////
		// IPlatform methods
		///////////////////////////////////////////////////////////////////////////////////////////
		virtual cl_int GetPlatformIDs(cl_uint uiNumEntries, cl_platform_id * pclPlatforms, cl_uint * puiNumPlatforms);
		virtual cl_int GetPlatformInfo(cl_platform_id clPlatform, cl_platform_info clParamName, size_t szParamValueSize, void* pParamValue, size_t* pszParamValueSizeRet);
		virtual cl_int GetDeviceIDs(cl_platform_id clPlatform, cl_device_type clDeviceType, cl_uint uiNumEntries, cl_device_id* pclDevices, cl_uint* puiNumDevices);
		virtual cl_int GetDeviceInfo(cl_device_id  clDevice, cl_device_info clParamName, size_t szParamValueSize, void* pParamValue, size_t* pszParamValueSizeRet);
		virtual cl_int UnloadCompiler(void);
		virtual cl_int GetGLContextInfo(const cl_context_properties * properties, cl_gl_context_info param_name, size_t param_value_size, void *param_value, size_t *param_value_size_ret);
#if defined (DX9_MEDIA_SHARING)
        virtual cl_int GetDeviceIDsFromD3D9(cl_platform_id clPlatform, cl_dx9_device_source_intel clD3dDeviceSource, void *pD3dObject, cl_dx9_device_set_intel clD3dDeviceSet, cl_uint uiNumEntries, cl_device_id *pclDevices, cl_uint *puiNumDevices);
#endif

        // Device Fission support
        cl_err_code clCreateSubDevices(cl_device_id device,
                                       const cl_device_partition_property_ext* properties,
                                       cl_uint num_entries,
                                       cl_device_id* out_devices,
                                       cl_uint* num_devices);

        cl_err_code clReleaseDevice(cl_device_id device);

        cl_err_code clRetainDevice(cl_device_id device);

		/******************************************************************************************
		* Function: 	InitFECompiler
		* Description:	Load OpenCL front-end compiler for the specific device.
		* Arguments:	pRootDevice - a pointer to device for which FE should be initialized
		* Return value:	CL_SUCCESS - The initializtion operation succeded
		*				CL_ERR_FE_COMPILER_INIT_FAIL - one or more devices falied to initialize
		* Author:		Uri Levy
		* Date:			March 2008
		******************************************************************************************/
		cl_err_code InitFECompiler(Device* pRootDevice);

        bool CheckPlatformId(cl_platform_id clPlatform) const { return (clPlatform == &m_clPlatformId ) ||
            (clPlatform==NULL); }

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

		cl_err_code ReleaseFECompilers();

        cl_err_code AddDevices(FissionableDevice** ppDevices, unsigned int count);

		_cl_platform_id_int	m_clPlatformId;
		
		// map list of all devices
		OCLObjectsMap<_cl_device_id_int> m_mapDevices;

        // A list of root-level devices only. This list is static throughout the module's existence
		Device **		m_ppRootDevices;
        unsigned int	m_uiRootDevicesCount;

		// default device
		Device * m_pDefaultDevice;

		// map list of all front-end compilers
		OCLObjectsMap<_cl_object>	m_mapFECompilers;

        // A mutex to prevent concurrent calls to clCreateSubDevices
        Intel::OpenCL::Utils::OclMutex m_deviceFissionMutex;

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

		static const char m_vPlatformExtensionsStr[];
		static const unsigned int m_uiPlatformExtensionsStrSize;

		ocl_entry_points * m_pOclEntryPoints;

		ocl_gpa_data * m_pGPAData;

		DECLARE_LOGGER_CLIENT;

	};

}}}
