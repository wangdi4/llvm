// Copyright (c) 2006-2012 Intel Corporation
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
#include "fe_compiler.h"
#include <cl_object.h>
#include <Logger.h>
#include <cl_synch_objects.h>
#include <cl_device_api.h>
#include <cl_dynamic_lib.h>
#include <map>
#include <list>
#if defined (DX_MEDIA_SHARING)
#include <d3d9.h>
#endif

using namespace Intel::OpenCL::Utils;

namespace Intel { namespace OpenCL { namespace Framework {

	// Froward declarations
    class Device;

    /**********************************************************************************************
    * Class name:	FissionableDevice
    *
    * Inherit:		OCLObject
    * Description:	An artificial base class to support repeated fissioning of devices
    * Author:		Doron Singer
    * Date:			March 2011
    **********************************************************************************************/
    class FissionableDevice : public OCLObject<_cl_device_id_int,_cl_platform_id_int>
    {
    public:

        PREPARE_SHARED_PTR(FissionableDevice)      

        // The API to split the device into sub-devices. Used to query for how many devices will be generated, as well as return the list of their subdevice-IDs
        virtual cl_err_code FissionDevice(const cl_device_partition_property* props, cl_uint num_entries, cl_dev_subdevice_id* out_devices, cl_uint* num_devices, size_t* sizes);

        // An API to get the root-level device of deriving subclasses
        virtual SharedPtr<Device>     GetRootDevice() = 0;
        virtual ConstSharedPtr<Device> GetRootDevice() const = 0;

        // A convenience API to query whether a device is root-level or not
        virtual bool        IsRootLevelDevice() = 0;

        // An API to query the appropriate sub-device ID (0 for root-level devices)
        virtual cl_dev_subdevice_id GetSubdeviceId() { return 0; }

		virtual IOCLDeviceAgent*    GetDeviceAgent() = 0;

        virtual const IOCLDeviceAgent* GetDeviceAgent() const = 0;
		void AddedToContext();
		void RemovedFromContext();
		bool IsInContext() const  { return m_numContexts > 0; }

#if defined (DX_MEDIA_SHARING)
        /**
         * @fn  void FissionableDevice::setD3D9Device(IUnknown* const pD3DDevice)
         *
         * @brief   Sets a Direct3D device
         *
         * @author  Aharon
         * @date    7/5/2011
         *
         * @param   the Direct3D device to set (may be NULL)
         * @param   the Direct3D device type
         */

        void SetD3DDevice(IUnknown* const pD3DDevice, cl_context_properties iDevType)
        {
            m_pD3DDevice = pD3DDevice;
            m_iD3DDevType = iDevType;
        }

        /**
         * @fn  IUnknown* FissionableDevice::GetD3DDevice() const
         *
         * @brief   Gets the Direct3D device
         *
         * @author  Aharon
         * @date    7/18/2011
         *
         * @return  the Direct3D devicce
         */

        IUnknown* GetD3DDevice() const { return m_pD3DDevice; }

        /**
         * @fn  cl_context_properties GetD3DDevType() const { return m_iD3DDevType; }
         * 		
         * @return the Direct3D device type
         */

        cl_context_properties GetD3DDevType() const { return m_iD3DDevType; }
#endif

        /**
         * @param clImgFormat   a cl_image_format
         * @param clMemFlags    cl_mem_flags for image usage information
         * @param clMemObjType  cl_mem_object_type of the image
         * @return whether clImgFormat is supported by the device						
         */
        bool IsImageFormatSupported(const cl_image_format& clImgFormat, cl_mem_flags clMemFlags, cl_mem_object_type clMemObjType) const;

    protected:

        FissionableDevice(_cl_platform_id_int* platform) :
		  OCLObject<_cl_device_id_int,_cl_platform_id_int>(platform, "FissionableDevice"), m_pD3DDevice(NULL) {}

        ~FissionableDevice() {}

        /**
         * @return the cl_dev_subdevice_id associated with this device
         */
        virtual cl_dev_subdevice_id GetSubdeviceId() const { return NULL; }

		Intel::OpenCL::Utils::AtomicCounter m_numContexts;

    private:

#if ! defined (DX_MEDIA_SHARING)
        struct IUnknown;
#endif
        /* I define this attribute even if DX_MEDIA_SHARING is not defined, since it is too dangerous to
        make the size of the FissionableDevice object dependent on a macro definition, which may
        differ from one project to another. This might cause bugs that wouldn't be caught by the
        compiler, but appear in runtime and would be very hard to detect. */
        IUnknown* m_pD3DDevice;
        cl_context_properties m_iD3DDevType;

        Intel::OpenCL::Utils::OclMutex m_devMutex;

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

        PREPARE_SHARED_PTR(Device)

        static SharedPtr<Device> Allocate(_cl_platform_id_int* platform) { return SharedPtr<Device>(new Device(platform)); }

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
							size_t *	param_value_size_ret) const;

		/******************************************************************************************
		* Function: 	InitDevice
		* Description:	Load OpenCL device library
		* Arguments:	pwcDllPath [in]		full path of device driver's dll
		* Return value:	CL_SUCCESS - operation succeeded
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/
		cl_err_code InitDevice(const char * psDeviceAgentDllPath);

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
		* Function: 	SetFrontEndCompiler
		* Description:	Set the front-end compiler to the device
		* Arguments:	pFrontEndCompiler [in]	pointer to the front-end compiler
		* Return value:	void
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/
		void SetFrontEndCompiler(SharedPtr<FrontEndCompiler> pFrontEndCompiler) { m_pFrontEndCompiler = pFrontEndCompiler; }

		/******************************************************************************************
		* Function: 	GetFrontEndCompiler
		* Description:	Get the front-end compiler of the device
		* Arguments:	N/A
		* Return value:	pointer to the front-end compiler (null if compiler not initialized)
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/
		ConstSharedPtr<FrontEndCompiler> GetFrontEndCompiler(){ return m_pFrontEndCompiler; }

		IOCLDeviceAgent*	GetDeviceAgent() {return m_pDevice;}

        const IOCLDeviceAgent* GetDeviceAgent() const { return m_pDevice; }

		cl_device_type		GetDeviceType() {return m_deviceType;}

		void SetGLProperties(cl_context_properties hGLCtx, cl_context_properties hHDC)
							 { m_hGLContext = hGLCtx; m_hHDC = hHDC;}

		cl_ulong GetMaxLocalMemorySize() const {return m_stMaxLocalMemorySize;}

        // Inherited from FissionableDevice
        
        SharedPtr<Device> GetRootDevice() { return this; }
        ConstSharedPtr<Device> GetRootDevice() const { return this; }

        bool    IsRootLevelDevice() { return true; }

        //Override the OCLObject defaults
        // Cannot release root-level devices, always pretend to have another reference
        long Release() { return 1; }
        // Cannot retains root-level devices
        cl_err_code Retain() { return CL_SUCCESS; }
		void Cleanup( bool bIsTerminate = false );

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

        /******************************************************************************************
		* Function: 	Device
		* Description:	The Device class constructor
		* Arguments:
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/
		Device(_cl_platform_id_int* platform);

		///////////////////////////////////////////////////////////////////////////////////////////
		// callback functions
		///////////////////////////////////////////////////////////////////////////////////////////

		// IOCLFrameworkCallbacks
		void clDevBuildStatusUpdate(cl_dev_program clDevProg, void * pData, cl_build_status clBuildStatus);
		void clDevCmdStatusChanged(cl_dev_cmd_id cmd_id, void * pData, cl_int cmd_status, cl_int status_result, cl_ulong timer);

		// IOCLLoggerDescriptor
		cl_int clLogCreateClient(cl_int device_id, const char* client_name, cl_int * client_id);
		cl_int clLogReleaseClient(cl_int client_id);
		cl_int clLogAddLine(cl_int client_id, cl_int log_level, const char* IN source_file, const char* IN function_name, cl_int IN line_num, const char* IN message, ...);

		///////////////////////////////////////////////////////////////////////////////////////////
		// class private members
		///////////////////////////////////////////////////////////////////////////////////////////

		Intel::OpenCL::Utils::OclDynamicLib			m_dlModule;
		// front-end compiler
		SharedPtr<FrontEndCompiler>						    m_pFrontEndCompiler;

        // Pointer to the device GetInfo function.
        fn_clDevGetDeviceInfo*	m_pFnClDevGetDeviceInfo;

        cl_int							        m_iNextClientId;		// hold the next client logger id

		Intel::OpenCL::Utils::AtomicCounter     m_pDeviceRefCount;     // holds the reference count for the associated IOCLDevice

		Utils::OclSpinMutex                     m_deviceInitializationMutex;

		std::map<cl_int, Intel::OpenCL::Utils::LoggerClient*>	m_mapDeviceLoggerClinets; // OpenCL device's logger clients

		IOCLDeviceAgent*	m_pDevice;

		DECLARE_LOGGER_CLIENT;											// device's class logger client

		// Prefetched data
		cl_ulong								m_stMaxLocalMemorySize;

		cl_device_type							m_deviceType;
		// GL Sharing info
		cl_context_properties					m_hGLContext;
		cl_context_properties					m_hHDC;

	};

    class SubDevice : public FissionableDevice
    {
    public:        

        PREPARE_SHARED_PTR(SubDevice)     

        static SharedPtr<SubDevice> Allocate(SharedPtr<FissionableDevice> pParent, size_t numComputeUnits, cl_dev_subdevice_id id,
            const cl_device_partition_property* props)
        {
            return SharedPtr<SubDevice>(new SubDevice(pParent, numComputeUnits, id, props));
        }

        ~SubDevice();

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
                            size_t *	param_value_size_ret) const;

        // Inherited from FissionableDevice
        SharedPtr<Device> GetRootDevice() { return m_pRootDevice; }
        ConstSharedPtr<Device> GetRootDevice() const { return m_pRootDevice; }
        bool    IsRootLevelDevice() { return false; }

        size_t              GetNumComputeUnits() { return m_numComputeUnits; }
        cl_dev_subdevice_id GetSubdeviceId()     { return m_deviceId; }
        SharedPtr<FissionableDevice>  GetParentDevice()    { return m_pParentDevice; }
        IOCLDeviceAgent*    GetDeviceAgent()     { return m_pRootDevice->GetDeviceAgent(); }
        const IOCLDeviceAgent* GetDeviceAgent() const { return m_pRootDevice->GetDeviceAgent(); }

    protected:
        
        SubDevice(SharedPtr<FissionableDevice> pParent, size_t numComputeUnits, cl_dev_subdevice_id id, const cl_device_partition_property* props);

        void CacheFissionProperties(const cl_device_partition_property* props); 

        virtual cl_dev_subdevice_id GetSubdeviceId() const { return m_deviceId; }

        SharedPtr<Device>             m_pRootDevice;
        SharedPtr<FissionableDevice>  m_pParentDevice;   // Can be a sub-device or a device 
        cl_dev_subdevice_id m_deviceId;        // The ID assigned to me by the device
        size_t              m_numComputeUnits; // The amount of compute units represented by this sub-device
        cl_int              m_fissionMode;     // The fission mode that created this sub-device

        cl_device_partition_property* m_cachedFissionMode;   // A copy of the property list used to create this device
        cl_uint                       m_cachedFissionLength; // How many entries the list contains
	private:
		SubDevice(const SubDevice&);
		SubDevice& operator=(const SubDevice&);
    };


}}}
