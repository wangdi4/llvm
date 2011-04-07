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

///////////////////////////////////////////////////////////////////////////////////////////////////
//  Device.h
//  Implementation of the Class Device
//  Created on:      10-Dec-2008 2:08:23 PM
//  Original author: ulevy
///////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "cl_framework.h"
#include <cl_object.h>
#include <Logger.h>
#include <cl_synch_objects.h>
#include <cl_device_api.h>
#include <cl_dynamic_lib.h>
#include <map>
#include <list>

namespace Intel { namespace OpenCL { namespace Framework {

	// Froward declarations
    class IDeviceFissionObserver;
	class FECompiler;
    class Device;

    /**********************************************************************************************
    * Class name:	FissionableDevice
    *
    * Inherit:		OCLObject
    * Description:	An artificial base class to support repeated fissioning of devices
    * Author:		Doron Singer
    * Date:			March 2011
    **********************************************************************************************/
    class FissionableDevice : public OCLObject<_cl_device_id_int>
    {
    public:
    FissionableDevice() {}

        cl_err_code RegisterDeviceFissionObserver(IDeviceFissionObserver* ob); 
        void        UnregisterDeviceFissionObserver(IDeviceFissionObserver* ob);

        //virtual cl_err_code FissionDevice(const cl_device_partition_property_ext* props, cl_uint num_entries, cl_device_id* out_devices, cl_uint* num_devices) = 0;
        // The API to split the device into sub-devices. Used to query for how many devices will be generated, as well as return the list of their subdevice-IDs
        virtual cl_err_code FissionDevice(const cl_device_partition_property_ext* props, cl_uint num_entries, cl_dev_subdevice_id* out_devices, cl_uint* num_devices, size_t* sizes) = 0;

        virtual void NotifyDeviceFissioned(cl_uint numChildren, FissionableDevice** children);

        //Currently not implemented
        virtual void NotifyDeviceReleased(cl_device_id device) {}

        // An API to get the root-level device of deriving subclasses
        virtual Device*     GetRootDevice() = 0;

        // A convenience API to query whether a device is root-level or not
        virtual bool        IsRootLevelDevice() = 0;

        // An API to query the appropriate sub-device ID (0 for root-level devices)
        virtual cl_dev_subdevice_id GetSubdeviceId() { return 0; }

        virtual IOCLDevice*	GetDeviceAgent() = 0;

    protected:
        ~FissionableDevice() {}

        Utils::OclSpinMutex                     m_fissionObserverListMutex;

        std::list<IDeviceFissionObserver*>      m_fissionObserverList;

    };

	/**********************************************************************************************
	* Class name:	Device
	*
	* Inherit:		OCLObject
	* Description:	This object is a gate from the framework into the openCL device driver that is
	*				implemented by a separate library.
	* Author:		Uri Levy
	* Date:			December 2008
	**********************************************************************************************/
	class Device : public FissionableDevice, IOCLDevLogDescriptor, IOCLFrameworkCallbacks
	{
	public:

		/******************************************************************************************
		* Function: 	Device
		* Description:	The Device class constructor
		* Arguments:
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/
		Device();

		/******************************************************************************************
		* Function: 	GetInfo
		* Description:	get object specific information (inherited from OCLObject) the function
		*				query the desirable parameter value from the device
		* Arguments:	param_name [in]				parameter's name
		*				param_value_size [inout]	parameter's value size (in bytes)
		*				param_value [out]			parameter's value
		*				param_value_size_ret [out]	parameter's value return size
		* Return value:	CL_SUCCESS - operation succeeded
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/
		cl_err_code	GetInfo(cl_int		param_name,
							size_t		param_value_size,
							void *		param_value,
							size_t *	param_value_size_ret);

		/******************************************************************************************
		* Function: 	InitDevice
		* Description:	Load OpenCL device library
		* Arguments:	pwcDllPath [in]		full path of device driver's dll
		* Return value:	CL_SUCCESS - operation succeeded
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/
		cl_err_code InitDevice(const char * psDeviceAgentDllPath, ocl_entry_points * pOclEntryPoints);

		/******************************************************************************************
		* Function: 	CreateInstance
		* Description:	Create a device agent instance
		* Arguments:
		* Return value:	CL_SUCCESS - operation succeeded
		* Author:		Arnon Peleg
		* Date:			June 2009
		******************************************************************************************/
        cl_err_code CreateInstance();

		/******************************************************************************************
		* Function: 	CloseDeviceInstance
		* Description:	Close the instance that was created by CreateInstance.
        *               Local members of the device class may not be valid until next call to CreateInstance
		* Arguments:
		* Return value:	CL_SUCCESS - operation succeeded
		* Author:		Arnon Peleg
		* Date:			June 2009
		******************************************************************************************/
        cl_err_code CloseDeviceInstance();

        /******************************************************************************************
		* Function: 	SetFECompiler
		* Description:	Set the front-end compiler to the device
		* Arguments:	pFECompiler [in]	pointer to the front-end compiler
		* Return value:	void
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/
		void SetFECompiler(FECompiler * pFECompiler) { m_pFECompiler = pFECompiler; }

		/******************************************************************************************
		* Function: 	GetFECompiler
		* Description:	Get the front-end compiler of the device
		* Arguments:	N/A
		* Return value:	pointer to the front-end compiler (null if compiler not initialized)
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/
		const FECompiler * GetFECompiler(){ return m_pFECompiler; }

		IOCLDevice*	GetDeviceAgent() {return m_pDevice;}

		void SetGLProperties(cl_context_properties hGLCtx, cl_context_properties hHDC)
							 { m_hGLContext = hGLCtx; m_hHDC = hHDC;}

		cl_ulong GetMaxLocalMemorySize() const {return m_stMaxLocalMemorySize;}

        // Inherited from FissionableDevice
        cl_err_code FissionDevice(const cl_device_partition_property_ext* props, cl_uint num_entries, cl_dev_subdevice_id* out_devices, cl_uint* num_devices, size_t* sizes);
        Device* GetRootDevice() { return this; }
        bool    IsRootLevelDevice() { return true; }

        //Override the OCLObject defaults
        // Cannot release root-level devices, always pretend to have another reference
        long Release() { return 1; }
        // Cannot retains root-level devices
        cl_err_code Retain() { return CL_SUCCESS; }

	protected:
		/******************************************************************************************
		* Function: 	~Device
		* Description:	The OCLObject class destructor
		* Arguments:
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/
		virtual ~Device();

	private:

		///////////////////////////////////////////////////////////////////////////////////////////
		// callback functions
		///////////////////////////////////////////////////////////////////////////////////////////

		// IOCLFrameworkCallbacks
		void clDevBuildStatusUpdate(cl_dev_program clDevProg, void * pData, cl_build_status clBuildStatus);
		void clDevCmdStatusChanged(cl_dev_cmd_id cmd_id, void * pData, cl_int cmd_status, cl_int status_result, cl_ulong timer);

		// IOCLLoggerDescriptor
		cl_int clLogCreateClient(cl_int device_id, const wchar_t* client_name, cl_int * client_id);
		cl_int clLogReleaseClient(cl_int client_id);
		cl_int clLogAddLine(cl_int client_id, cl_int log_level, const wchar_t* IN source_file, const wchar_t* IN function_name, cl_int IN line_num, const wchar_t* IN message, ...);
		cl_int clLogAddLine(cl_int client_id, cl_int log_level, const char* IN source_file, const char* IN function_name, cl_int IN line_num, const wchar_t* IN message, ...);

		///////////////////////////////////////////////////////////////////////////////////////////
		// class private members
		///////////////////////////////////////////////////////////////////////////////////////////

		Utils::OclDynamicLib					m_dlModule;
		// front-end compiler
		FECompiler *						    m_pFECompiler;

        // Pointer to the device GetInfo function.
        fn_clDevGetDeviceInfo*                  m_pFnClDevGetDeviceInfo;

        cl_int							        m_iNextClientId;		// hold the next client logger id

		Utils::AtomicCounter                    m_pDeviceRefCount;     // holds the reference count for the associated IOCLDevice

		Utils::OclSpinMutex                     m_deviceInitializationMutex;

		std::map<cl_int, Intel::OpenCL::Utils::LoggerClient*>	m_mapDeviceLoggerClinets; // OpenCL device's logger clients

		IOCLDevice*								m_pDevice;

		DECLARE_LOGGER_CLIENT;											// device's class logger client

		cl_ulong								m_stMaxLocalMemorySize;
		// GL Sharing info
		cl_context_properties					m_hGLContext;
		cl_context_properties					m_hHDC;

	};

    class SubDevice : public FissionableDevice
    {
    public:
        SubDevice(FissionableDevice* pParent, size_t numComputeUnits, cl_dev_subdevice_id id, const cl_device_partition_property_ext* props, ocl_entry_points * pOclEntryPoints);

        /******************************************************************************************
        * Function: 	GetInfo
        * Description:	get object specific information (inherited from OCLObject) the function
        *				queries the desirable parameter value from the root-level device
        * Arguments:	param_name [in]				parameter's name
        *				param_value_size [inout]	parameter's value size (in bytes)
        *				param_value [out]			parameter's value
        *				param_value_size_ret [out]	parameter's value return size
        * Return value:	CL_SUCCESS - operation succeeded
        * Author:		Doron Singer
        * Date:			March 2011
        ******************************************************************************************/
        cl_err_code	GetInfo(cl_int		param_name,
                            size_t		param_value_size,
                            void *		param_value,
                            size_t *	param_value_size_ret);

        // Inherited from FissionableDevice
        cl_err_code FissionDevice(const cl_device_partition_property_ext* props, cl_uint num_entries, cl_dev_subdevice_id* out_devices, cl_uint* num_devices, size_t* sizes);
        Device* GetRootDevice() { return m_pRootDevice; }
        bool    IsRootLevelDevice() { return false; }

        size_t              GetNumComputeUnits() { return m_numComputeUnits; }
        cl_dev_subdevice_id GetSubdeviceId()     { return m_deviceId; }
        FissionableDevice*  GetParentDevice()    { return m_pParentDevice; }
        IOCLDevice*         GetDeviceAgent()     { return m_pParentDevice->GetDeviceAgent(); }

    protected:
        ~SubDevice(); 

        void CacheFissionProperties(const cl_device_partition_property_ext* props); 

        Device*             m_pRootDevice;
        FissionableDevice*  m_pParentDevice;   // Can be a sub-device or a device 
        cl_dev_subdevice_id m_deviceId;        // The ID assigned to me by the device
        size_t              m_numComputeUnits; // The amount of compute units represented by this sub-device
        cl_int              m_fissionMode;     // The fission mode that created this sub-device

        cl_device_partition_property_ext* m_cachedFissionMode;   // A copy of the property list used to create this device
        cl_uint                           m_cachedFissionLength; // How many entries the list contains
    };


}}}
