// INTEL CONFIDENTIAL
//
// Copyright 2006-2018 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

#pragma once

#include "cl_framework.h"
#include "cl_synch_objects.h"
#include "iplatform.h"
#include "ocl_itt.h"
#include "cl_objects_map.h"
#include "cl_shared_ptr.h"
#include <Logger.h>
#include <vector>
#include <ocl_config.h>
#if defined (DX_MEDIA_SHARING)
#include "d3d9_definitions.h"
#endif

namespace Intel { namespace OpenCL { namespace Framework {

#define CL_PLATFORM_ID_DEFAULT    0x0
#define CL_PLATFORM_ID_INTEL    0x1

    class OCLObjectInfo;
    template <class HandleType, class ObjectType> class OCLObjectsMap;
    class Device;
    class FissionableDevice;
    class OCLConfig;

    /**********************************************************************************************
    * Class name:    PlatformModule
    *
    * Description:    platform module class
    * Author:        Uri Levy
    * Date:            December 2008
    **********************************************************************************************/
    class PlatformModule : public OCLObjectBase, public IPlatform
    {
    
    public:

        /******************************************************************************************
        * Function:     PlatformModule
        * Description:    The Platform Module class constructor
        * Arguments:        
        * Author:        Uri Levy
        * Date:            December 2008
        ******************************************************************************************/
        PlatformModule();

        /******************************************************************************************
        * Function:     ~PlatformModule
        * Description:    The Platform Module class destructor
        * Arguments:        
        * Author:        Uri Levy
        * Date:            December 2008
        ******************************************************************************************/
        virtual ~PlatformModule();

        /******************************************************************************************
        * Function:     Initialize    
        * Description:    Initialize the platform module object: set platform's information
        *                and load devices
        * Arguments:        
        * Return value:    CL_SUCCESS - The initialization operation succeeded
        * Author:        Uri Levy
        * Date:            December 2008
        ******************************************************************************************/        
        cl_err_code        Initialize(ocl_entry_points * pOclEntryPoints, OCLConfig * pConfig, ocl_gpa_data * pGPAData);

        /******************************************************************************************
        * Function:     Release    
        * Description:    Release the platform module's resources
        * Arguments:        
        * Return value:    CL_SUCCESS - The release operation succeeded
        * Author:        Uri Levy
        * Date:            December 2008
        ******************************************************************************************/
        cl_err_code        Release(bool bTerminate);

        /******************************************************************************************
        * Function:     GetRootDevice    
        * Description:    Get the root device object that assigned to the device id
        * Arguments:    clDeviceId [in] -    device id
        *                ppDevice [out] -    pointer to the device
        * Return value:    CL_SUCCESS - The operation succeeded
        * Author:        Uri Levy
        * Date:            December 2008
        ******************************************************************************************/
        cl_err_code        GetRootDevice(cl_device_id IN  clDeviceId, SharedPtr<Device>* OUT ppDevice);

        /******************************************************************************************
        * Function:     GetDevice    
        * Description:    Get the device object that assigned to the device id
        * Arguments:    clDeviceId [in] -    device id
        * Return value:    a SharedPtr<FissionableDevice> pointing to the device object or NULL if it
        *               cannot be found
        * Author:        Doron Singer
        * Date:            March 2011
        ******************************************************************************************/
        SharedPtr<FissionableDevice> GetDevice(cl_device_id IN  clDeviceId);

        /******************************************************************************************
        * Function:     GetDeviceMode
        * Description:    Returns a DeviceMode enum value
        * Arguments:    None
        * Return value:    DeviceMode
        * Author:
        * Date:
        ******************************************************************************************/
        DeviceMode GetDeviceMode() const { return m_deviceMode; }

        /******************************************************************************************
        * Function:     GetGPAData    
        * Description:    Returns a pointer to the GPA data object.
        * Arguments:    None
        * Return value:    ocl_gpa_data
        * Author:        Oded Perez
        * Date:            July 2011
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
        virtual cl_err_code UnloadPlatformCompiler(cl_platform_id pclPlatforms);
        virtual cl_int GetGLContextInfo(const cl_context_properties * properties, cl_gl_context_info param_name, size_t param_value_size, void *param_value, size_t *param_value_size_ret);
#if defined (DX_MEDIA_SHARING)
        virtual cl_int GetDeviceIDsFromD3D(cl_platform_id platform, cl_uint num_media_adapters, int *media_adapters_type, void** media_adapters,
            int media_adapter_set, cl_uint num_entries, cl_device_id *devices, cl_uint *num_devices, const ID3DSharingDefinitions& d3d9Definitions);
#endif

        // Device Fission support
        cl_err_code clCreateSubDevices(cl_device_id device,
                                       const cl_device_partition_property* properties,
                                       cl_uint num_entries,
                                       cl_device_id* out_devices,
                                       cl_uint* num_devices);

        cl_err_code clReleaseDevice(cl_device_id device);

        cl_err_code clRetainDevice(cl_device_id device);

        bool CheckPlatformId(cl_platform_id clPlatform) const { return (clPlatform == &m_clPlatformId ) ||
            (clPlatform==nullptr); }

        //
        // Utilities
        //
        void RemoveAllDevices(bool preserve_user_handles);

        void DeviceCreated() { ++m_activeDeviceCount; }
        void DeviceClosed()  { --m_activeDeviceCount; }
        long GetActiveDeviceCount() const { return m_activeDeviceCount; }
        void WaitForAllDevices() { while (m_activeDeviceCount > 0) { Intel::OpenCL::Utils::hw_pause(); }; }
        cl_err_code GetHostTimer(cl_device_id device, cl_ulong* host_timestamp);
        cl_err_code GetDeviceAndHostTimer(cl_device_id device, cl_ulong* device_timestamp, cl_ulong* host_timestamp);

    private:

        /******************************************************************************************
        * Function:     InitDevices
        * Description:    Load OpenCL devices and initialize them. all device dll's must be located
        *                in the same folder as the framework dll
        * Arguments:    vector<string> devices -    list of device dll names
        *                string defaultDevice -        name of default device agent's dll name
        *                vector<string> compilers -    list of fron-end compilers dll names
        *                string defaultCompiler -    name of default front-end compiler's dll name
        * Return value:    CL_SUCCESS - The initializtion operation succeded
        *                CL_ERR_DEVICE_INIT_FAIL - one or more devices falied to initialize
        * Author:        Uri Levy
        * Date:            December 2008
        ******************************************************************************************/
        cl_err_code InitDevices(const std::vector<std::string>& devices, const std::string& defaultDevice);

        cl_err_code AddDevices(SharedPtr<FissionableDevice>* ppDevices, unsigned int count);

        _cl_platform_id_int    m_clPlatformId;
        
        // map list of all devices
        OCLObjectsMap<_cl_device_id_int, _cl_platform_id_int> m_mapDevices;

        // A list of root-level devices only. This list is static throughout the module's existence
        SharedPtr<Device>*        m_ppRootDevices;
        size_t                    m_uiRootDevicesCount; // size of m_ppRootDevices array
        Intel::OpenCL::Utils::AtomicCounter m_activeDeviceCount; // how many devices are still not closed

        // default device
        SharedPtr<Device> m_pDefaultDevice;

        // A mutex to prevent concurrent calls to clCreateSubDevices
        Intel::OpenCL::Utils::OclMutex m_deviceFissionMutex;

        // static chars array - holds the platform's information string
        static std::string m_vPlatformInfoStr;
        // platform's information string length
        static unsigned int m_uiPlatformInfoStrSize;

        // static chars array - holds the platform's version string
        static std::string m_vPlatformVersionStr;

        static std::string m_vPlatformNameStr;
        static unsigned int m_uiPlatformNameStrSize;

        static const char m_vPlatformVendorStr[];
        static const unsigned int m_uiPlatformVendorStrSize;

        DeviceMode m_deviceMode;

        ocl_entry_points * m_pOclEntryPoints;

        ocl_gpa_data * m_pGPAData;

        Intel::OpenCL::Utils::OPENCL_VERSION m_oclVersion;

        DECLARE_LOGGER_CLIENT;

        PlatformModule(const PlatformModule&);
        PlatformModule& operator=(const PlatformModule&);
    };

}}}
