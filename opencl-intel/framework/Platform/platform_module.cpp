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

#include "platform_module.h"
#include "Device.h"
#include "fe_compiler.h"

#include <cl_object_info.h>
#include <cl_objects_map.h>
#include <cl_device_api.h>
#include <cl_sys_defines.h>

#include <algorithm>
#include <assert.h>
#include <malloc.h>
#include <string>
#include "cl_local_array.h"
#include "cl_shared_ptr.hpp"
#include "cl_sys_info.h"

using namespace Intel::OpenCL::Utils;
using namespace Intel::OpenCL::Framework;

std::string PlatformModule::m_vPlatformInfoStr = "";
std::string PlatformModule::m_vPlatformNameStr = "";
unsigned int PlatformModule::m_uiPlatformInfoStrSize = 0;
unsigned int PlatformModule::m_uiPlatformNameStrSize = 0;
std::string PlatformModule::m_vPlatformVersionStr;
const char PlatformModule::m_vPlatformVendorStr[] = "Intel(R) Corporation";
const unsigned int PlatformModule::m_uiPlatformVendorStrSize = sizeof(m_vPlatformVendorStr) / sizeof(char);


///////////////////////////////////////////////////////////////////////////////////////////////////
// PlatformModule::PlatformModule
///////////////////////////////////////////////////////////////////////////////////////////////////
PlatformModule::PlatformModule() : OCLObjectBase("PlatformModule")
{
    m_ppRootDevices        = nullptr;
    m_uiRootDevicesCount   = 0;
    m_pOclEntryPoints      = nullptr;
    m_oclVersion           = OPENCL_VERSION_UNKNOWN;

    memset(&m_clPlatformId, 0, sizeof(m_clPlatformId));
    // initialize logger
    INIT_LOGGER_CLIENT("PlatformModule", LL_DEBUG);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// PlatformModule::~PlatformModule
///////////////////////////////////////////////////////////////////////////////////////////////////
PlatformModule::~PlatformModule()
{
    RELEASE_LOGGER_CLIENT;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// PlatformModule::InitDevices
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code PlatformModule::InitDevices(const vector<string>& devices, const string& defaultDevice)
{
    unsigned int supported_devices_type_count = (unsigned int)devices.size();

    if (0 == supported_devices_type_count)
    {
        return CL_INVALID_DEVICE;
    }

    m_uiRootDevicesCount = 0;
    m_pDefaultDevice = nullptr;
    m_ppRootDevices = nullptr;

    cl_err_code clErrRet = CL_SUCCESS;
    vector< SharedPtr<Device> > devicesList;
    for(unsigned int ui = 0; ui < supported_devices_type_count; ++ui)
    {
        string strDevice = OS_DLL_POST(devices[ui]);

        clErrRet = Device::CreateAndInitAllDevicesOfDeviceType(strDevice.c_str(), &m_clPlatformId, &devicesList);
        if (CL_FAILED(clErrRet))
        {
            if (CL_OUT_OF_HOST_MEMORY == clErrRet)
            {
                devicesList.clear();
                break;
            }
            // it's possible - if the device isn't presented on the system
            LOG_INFO(TEXT("InitDevice() failed with %d for %s"), clErrRet, devices[ui].c_str());
            continue;
        }
    }

    if (devicesList.size() == 0)
    {
        return clErrRet;
    }

    m_uiRootDevicesCount = devicesList.size();
    m_ppRootDevices = new SharedPtr<Device>[m_uiRootDevicesCount];
    if (nullptr == m_ppRootDevices)
    {
        m_uiRootDevicesCount = 0;
        return CL_OUT_OF_HOST_MEMORY;
    }

    for(size_t ui = 0; ui < m_uiRootDevicesCount; ++ui)
    {
        m_ppRootDevices[ui] = devicesList[ui];
        // assign device in the objects map
        m_mapDevices.AddObject(devicesList[ui]);

        if ((0 == m_pDefaultDevice) && (defaultDevice != "") && (defaultDevice == devices[ui]))
        {
            m_pDefaultDevice = devicesList[ui];
        }
    }

    return CL_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// PlatformModule::Initialize
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code    PlatformModule::Initialize(ocl_entry_points * pOclEntryPoints, OCLConfig * pConfig, ocl_gpa_data * pGPAData)
{
    LOG_INFO(TEXT("%s"), TEXT("Platform module logger initialized"));

    m_pOclEntryPoints = pOclEntryPoints;

    m_clPlatformId.object = &m_clPlatformId;
    *((ocl_entry_points*)(&m_clPlatformId)) = *m_pOclEntryPoints;

    // initialize devices
    m_pDefaultDevice = nullptr;

    // initialize GPA data
    m_pGPAData = pGPAData;

    // initialize device mode
    m_deviceMode = pConfig->GetDeviceMode();

    // get device agents dll names from configuration file
    string strDefaultDevice = pConfig->GetDefaultDevice();
    vector<string> strDevices = pConfig->GetDevices();
    if (strDevices.size() == 0)
    {
        return CL_ERR_DEVICE_INIT_FAIL;
    }

    // Initialize devices, included initialization of required FE compiler
    cl_err_code clErr = InitDevices(strDevices, strDefaultDevice);
    if (CL_FAILED(clErr))
    {
        LOG_CRITICAL(TEXT("%s"), TEXT("Failed to initialize devices compilers"));
    }

    m_oclVersion = pConfig->GetOpenCLVersion();
    switch(m_oclVersion)
    {
        case OPENCL_VERSION_1_2:
            m_vPlatformVersionStr = "OpenCL 1.2";
        break;
        case OPENCL_VERSION_2_2:
            m_vPlatformVersionStr = "OpenCL 2.2";
        break;
        case OPENCL_VERSION_2_1:
            m_vPlatformVersionStr = "OpenCL 2.1";
        break;
        case OPENCL_VERSION_2_0:
            m_vPlatformVersionStr = "OpenCL 2.0";
        break;
        default:
            m_vPlatformVersionStr = "OpenCL 1.0";
        break;
    }

    if (FPGA_EMU_DEVICE == m_deviceMode)
    {
        m_vPlatformVersionStr +=
          " Intel(R) FPGA SDK for OpenCL(TM), Version 19.2";
    }
    else
    {
#if defined (_WIN32)
        m_vPlatformVersionStr += " WINDOWS";
#else // LINUX
        m_vPlatformVersionStr += " LINUX";
#endif
    }

    return clErr;

}

///////////////////////////////////////////////////////////////////////////////////////////////////
// PlatformModule::Release
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code    PlatformModule::Release(bool bTerminate)
{
    // release devices
    m_mapDevices.ReleaseAllObjects(bTerminate);
    m_pDefaultDevice = nullptr;

    if (nullptr != m_ppRootDevices)
    {
        delete[] m_ppRootDevices;
        m_ppRootDevices = nullptr;
    }
    RELEASE_LOGGER_CLIENT;

    return CL_SUCCESS;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// PlatformModule::GetPlatformIDs
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code PlatformModule::GetPlatformIDs(cl_uint uiNumEntries,
                                           cl_platform_id * pclPlatforms,
                                           cl_uint * puiNumPlatforms)
{
    LOG_INFO(TEXT("Enter GetPlatformIDs. (uiNumEntries=%d, pclPlatforms=%d, puiNumPlatforms=%d)"),
        uiNumEntries, pclPlatforms, puiNumPlatforms);

    if ( ((0 == uiNumEntries) && (nullptr != pclPlatforms)) ||
         ((nullptr == puiNumPlatforms) && (nullptr == pclPlatforms)) )
    {
        LOG_ERROR(TEXT("%s"), TEXT("((0 == uiNumEntries) && (NULL != pclPlatforms)) || ((NULL == puiNumPlatforms) && (NULL != pclPlatforms))"));
        return CL_INVALID_VALUE;
    }

    if ( (uiNumEntries > 0) && (nullptr != pclPlatforms) )
    {
        *pclPlatforms = &m_clPlatformId;
    }


    if (nullptr != puiNumPlatforms)
    {
        *puiNumPlatforms = 1;
    }
    return CL_SUCCESS;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// PlatformModule::UnloadPlatformCompiler
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code PlatformModule::UnloadPlatformCompiler(cl_platform_id platform)
{
    LOG_INFO(TEXT("Enter UnloadPlatformCompiler. platform=%d"), platform);

    if ( false == CheckPlatformId(platform) )
    {
        LOG_ERROR(TEXT("%s"), TEXT("false == CheckPlatformId(platform)"));
        return CL_INVALID_PLATFORM;
    }

    return CL_SUCCESS;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// PlatformModule::GetPlatformInfo
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_int    PlatformModule::GetPlatformInfo(cl_platform_id clPlatform,
                                        cl_platform_info clParamName,
                                        size_t szParamValueSize,
                                        void* pParamValue,
                                        size_t* pszParamValueSizeRet)
{
    LOG_INFO(TEXT("Enter GetPlatformInfo (clParamName=%d, szParamValueSize=%d, pParamValue=%d, pszParamValueSizeRet=%d)"),
        clParamName, szParamValueSize, pParamValue, pszParamValueSizeRet);

    if (false == CheckPlatformId(clPlatform))
    {
        LOG_ERROR(TEXT("Current platform id (%d) is not supported"), clPlatform);
        return CL_INVALID_PLATFORM;
    }

    cl_err_code clErr = CL_SUCCESS;
    size_t szParamSize = 0;
    cl_ulong value = 0;
    void* pValue   = &value;
    char * pch = nullptr,  *pNextToken;
    SharedPtr<Device> pDevice = nullptr;
    bool bRes = true;
    cl_char pcPlatformExtension[8192] = {0};
    cl_char pcDeviceExtension[8192] = {0};
    cl_char pcOtherDeviceExtension[8192] = {0};
    const char * pcPlatformICDSuffixKhr;

    if (FPGA_EMU_DEVICE == m_deviceMode)
    {
        m_vPlatformInfoStr = "EMBEDDED_PROFILE";
        pcPlatformICDSuffixKhr = "IntelFPGA";
        m_vPlatformNameStr =
          "Intel(R) FPGA Emulation Platform for OpenCL(TM) (preview)";
    }
    else
    {
        m_vPlatformInfoStr = "FULL_PROFILE";
        pcPlatformICDSuffixKhr = "INTEL";
#ifdef BUILD_OPENCL_21
        m_vPlatformNameStr = "Intel(R) OpenCL 2.1 CPU Only Platform";
#else
        m_vPlatformNameStr = "Intel(R) OpenCL";
#endif // BUILD_OPENCL_21
    }

    switch (clParamName)
    {
    case CL_PLATFORM_PROFILE:
        m_uiPlatformInfoStrSize = m_vPlatformInfoStr.size() + 1;
        szParamSize = m_uiPlatformInfoStrSize;
        pValue = (void*)m_vPlatformInfoStr.c_str();
        break;
    case CL_PLATFORM_VERSION:
        // it must include the terminating null character
        szParamSize = m_vPlatformVersionStr.size() + 1;
        pValue = (void*)m_vPlatformVersionStr.c_str();
        break;
    case CL_PLATFORM_NAME:
        // it must include the terminating null character
        m_uiPlatformNameStrSize = m_vPlatformNameStr.size() + 1;
        szParamSize = m_uiPlatformNameStrSize;
        pValue = (void*)m_vPlatformNameStr.c_str();
        break;
    case CL_PLATFORM_VENDOR:
        szParamSize = m_uiPlatformVendorStrSize;
        pValue = (void*)m_vPlatformVendorStr;
        break;
    case CL_PLATFORM_EXTENSIONS:
        assert ((m_uiRootDevicesCount > 0) &&
            "No devices associated to the platform");
        clErr = m_ppRootDevices[0]->
          GetInfo(CL_DEVICE_EXTENSIONS, 8192, pcDeviceExtension, nullptr);
        if (CL_FAILED(clErr))
        {
            return CL_INVALID_VALUE;
        }
        pch = STRTOK_S((char*)pcDeviceExtension," ", &pNextToken);
        while (pch != nullptr)
        {
            bRes = true;
            for (size_t ui=1; ui<m_uiRootDevicesCount; ++ui)
            {
                clErr = m_ppRootDevices[ui]->
                  GetInfo(CL_DEVICE_EXTENSIONS, 8192, pcOtherDeviceExtension,
                           nullptr);
                if (CL_FAILED(clErr))
                {
                    return CL_INVALID_VALUE;
                }
                if (nullptr == strstr((char*)pcOtherDeviceExtension, pch))
                {
                    bRes = false;
                    break;
                }
            }
            if (bRes)
            {
                STRCAT_S((char*)pcPlatformExtension, 8192, pch);
                STRCAT_S((char*)pcPlatformExtension, 8192, " ");
            }
            pch = STRTOK_S(nullptr, " ", &pNextToken);
        }

        pValue = pcPlatformExtension;
        szParamSize = strlen((char*)pcPlatformExtension) + 1;
        break;
    case CL_PLATFORM_ICD_SUFFIX_KHR:
        pValue = (void*)pcPlatformICDSuffixKhr;
        szParamSize = strlen((char*)pcPlatformICDSuffixKhr) + 1;
        break;
    case CL_PLATFORM_HOST_TIMER_RESOLUTION:
        if (m_oclVersion >= OPENCL_VERSION_2_1)
        {
            *(cl_ulong*)pValue = (cl_ulong)ProfilingTimerResolution();
            szParamSize = sizeof(cl_ulong);
            break;
        }
    default:
        return CL_INVALID_VALUE;
    }

    if (nullptr != pParamValue)
    {
        if (szParamValueSize < szParamSize)
        {
            LOG_ERROR(TEXT("szParamValueSize (%d) < pszParamValueSizeRet (%d)"), szParamValueSize, szParamSize);
            return CL_INVALID_VALUE;
        }
        memset(pParamValue, 0, szParamValueSize);
        MEMCPY_S(pParamValue, szParamValueSize, pValue, szParamSize);
    }

    // The size should be return only if successful copy was completed (CSSD100011955)
    if (nullptr != pszParamValueSizeRet)
    {
        *pszParamValueSizeRet = szParamSize;
    }

    return CL_SUCCESS;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// PlatformModule::GetDeviceIDs
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_int    PlatformModule::GetDeviceIDs(cl_platform_id clPlatform,
                                     cl_device_type clDeviceType,
                                     cl_uint uiNumEntries,
                                     cl_device_id* pclDevices,
                                     cl_uint* puiNumDevices)
{
    LOG_INFO(TEXT("Enter GetDeviceIDs (device_type=%d, num_entried=%d, pclDevices=%d, puiNumDevices=%d)"),
        clDeviceType, uiNumEntries, pclDevices, puiNumDevices);

    if (!(clDeviceType & CL_DEVICE_TYPE_DEFAULT)        &&
        !(clDeviceType & CL_DEVICE_TYPE_CPU)            &&
        !(clDeviceType & CL_DEVICE_TYPE_GPU)            &&
        !(clDeviceType & CL_DEVICE_TYPE_ACCELERATOR)    &&
        !(clDeviceType & CL_DEVICE_TYPE_CUSTOM))
    {
        return CL_INVALID_DEVICE_TYPE;
    }

    if ((nullptr != pclDevices && 0 == uiNumEntries) ||
        (nullptr == pclDevices && nullptr == puiNumDevices))
    {
        LOG_ERROR(TEXT("%s"), TEXT("NULL == pclDevices && NULL == puiNumDevices"));
        return CL_INVALID_VALUE;
    }

    size_t uiNumDevices = m_uiRootDevicesCount;
    cl_uint uiRetNumDevices = 0; // this will be used for the num_devices return value;
    SharedPtr<Device>* ppDevices = nullptr;
    cl_device_id * pDeviceIds = nullptr;

    if (uiNumDevices == 0)
    {
        return CL_DEVICE_NOT_FOUND;
    }
    if (clDeviceType == CL_DEVICE_TYPE_DEFAULT && m_pDefaultDevice == 0)
    {
        return CL_DEVICE_NOT_FOUND;
    }

    // prepare list for all devices
    ppDevices = new SharedPtr<Device> [uiNumDevices];
    if (nullptr == ppDevices)
    {
        LOG_ERROR(TEXT("%s"), TEXT("can't allocate memory for devices (NULL == ppDevices)"));
        return CL_OUT_OF_HOST_MEMORY;
    }
    for (size_t i = 0; i < m_uiRootDevicesCount; i++)
    {
        ppDevices[i] = m_ppRootDevices[i];
    }
    pDeviceIds = new cl_device_id[uiNumDevices];
    if (nullptr == pDeviceIds)
    {
        LOG_ERROR(TEXT("%s"), TEXT("can't allocate memory for device ids (NULL == pDeviceIds)"));
        delete[] ppDevices;
        return CL_OUT_OF_HOST_MEMORY;
    }

    for (cl_uint ui=0; ui<uiNumDevices; ++ui)
    {
        if (0 != ppDevices[ui])
        {
            if ((clDeviceType & CL_DEVICE_TYPE_DEFAULT) &&
                ppDevices[ui]->GetId() == m_pDefaultDevice->GetId())
            {
                //found the default device
                pDeviceIds[uiRetNumDevices++] = ppDevices[ui]->GetHandle();
                continue;
            }
            if (clDeviceType == CL_DEVICE_TYPE_ALL)
            {
                pDeviceIds[uiRetNumDevices++] = ppDevices[ui]->GetHandle();
            }
            else
            {
                // get device type
                cl_device_type clType;
                cl_int iErrRet = ppDevices[ui]->GetInfo(CL_DEVICE_TYPE, sizeof(cl_device_type), &clType, nullptr);
                // check that the current device type satisfactory
                if (iErrRet == 0 && ((clType & clDeviceType) == clType))
                {
                    pDeviceIds[uiRetNumDevices++] = ppDevices[ui]->GetHandle();
                }
            }
        }
    }
    delete[] ppDevices;

    if (uiRetNumDevices == 0)
    {
        delete[] pDeviceIds;
        return CL_DEVICE_NOT_FOUND;
    }

    if (nullptr != pclDevices)
    {
        cl_uint uiNumDevicesToAdd = min(uiRetNumDevices,uiNumEntries);

        for (cl_uint ui=0; ui < uiNumDevicesToAdd; ++ui)
        {
            pclDevices[ui] = pDeviceIds[ui];
        }
    }

    if (nullptr != puiNumDevices)
    {
        *puiNumDevices = uiRetNumDevices;
    }

    delete[] pDeviceIds;
    return CL_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// PlatformModule::GetDeviceInfo
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_int    PlatformModule::GetDeviceInfo(cl_device_id clDevice,
                                      cl_device_info clParamName,
                                      size_t szParamValueSize,
                                      void* pParamValue,
                                      size_t* pszParamValueSizeRet)
{
    SharedPtr<FissionableDevice> pDevice = nullptr;
    size_t szParamSize = 0;
    cl_bool bBoolValue = CL_TRUE;
    const void * pValue = nullptr;
	  const cl_platform_id id = &m_clPlatformId;

    switch(clParamName)
    {
    case CL_DEVICE_PLATFORM:
        {
            szParamSize = sizeof(cl_platform_id);
            pValue = &id;
            break;
        }
    case CL_DEVICE_LINKER_AVAILABLE:
    case CL_DEVICE_COMPILER_AVAILABLE:
        {
            szParamSize = sizeof(cl_bool);
            bBoolValue = CL_TRUE;
            pValue = &bBoolValue;
            break;
        }
    case CL_DEVICE_SPIR_VERSIONS:
        {
            // Regardless to the device, Our SDK supports SPIR 1.2 (which is the
            // only existing version ATM).
            pValue = "1.2";
            szParamSize = strlen((const char*)pValue) + 1;
            break;
        }

    default:
        pDevice = m_mapDevices.GetOCLObject((_cl_device_id_int*)clDevice).DynamicCast<FissionableDevice>();
        if (0 == pDevice)
        {
            return CL_INVALID_DEVICE;
        }
        return pDevice->GetInfo(clParamName, szParamValueSize, pParamValue, pszParamValueSizeRet);
    }

    if (nullptr != pParamValue)
    {
        if (szParamValueSize < szParamSize)
        {
            LOG_ERROR(TEXT("szParamValueSize (%d) < pszParamValueSizeRet (%d)"), szParamValueSize, szParamSize);
            return CL_INVALID_VALUE;
        }

        MEMCPY_S(pParamValue, szParamValueSize, pValue, szParamSize);
    }

    if (nullptr != pszParamValueSizeRet)
    {
        *pszParamValueSizeRet = szParamSize;
    }

    return CL_SUCCESS;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// PlatformModule::GetDevice
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code    PlatformModule::GetRootDevice(cl_device_id clDeviceId, SharedPtr<Device>* ppDevice)
{
    LOG_INFO(TEXT("PlatformModule::GetDevice enter. clDeviceId=%d, ppDevices=%d"),clDeviceId, ppDevice);
    assert( (nullptr != ppDevice) );
    // get the device from the devices list
    SharedPtr<FissionableDevice> temp = m_mapDevices.GetOCLObject((_cl_device_id_int*)clDeviceId).DynamicCast<FissionableDevice>();
    if (!temp)
    {
        return CL_INVALID_DEVICE;
    }
    *ppDevice = temp->GetRootDevice();
    return CL_SUCCESS;
}

SharedPtr<FissionableDevice> PlatformModule::GetDevice(cl_device_id clDeviceId)
{
    LOG_INFO(TEXT("PlatformModule::GetDevice enter. clDeviceId=%d"),clDeviceId);
    // get the device from the devices list
    return m_mapDevices.GetOCLObject((_cl_device_id_int*)clDeviceId).DynamicCast<FissionableDevice>();
}

//////////////////////////////////////////////////////////////////////////
// PlatformModule::UnloadCompiler
//////////////////////////////////////////////////////////////////////////
cl_int PlatformModule::UnloadCompiler(void)
{
    for (cl_uint ui=0; ui<m_mapDevices.Count(); ++ui)
    {
        SharedPtr<FissionableDevice> pDevice = m_mapDevices.GetObjectByIndex(ui).DynamicCast<FissionableDevice>();
        if (0 != pDevice && 0 != pDevice->GetDeviceAgent())
        {
            pDevice->GetDeviceAgent()->clDevUnloadCompiler();
        }
    }
    return CL_SUCCESS;
}

// Device Fission
class ParentDeviceWrapper
{
public:
    explicit ParentDeviceWrapper(const SharedPtr<FissionableDevice>& ptr) : m_bNeedToCreateDevice(false), m_pParentDevice(ptr) {}
    ~ParentDeviceWrapper()
    {
        if (m_bNeedToCreateDevice)
        {
            m_pParentDevice->GetRootDevice()->CloseDeviceInstance();
        }
    }

    cl_err_code CreateRootDevice()
    {
        m_bNeedToCreateDevice = ((0 != m_pParentDevice) && (nullptr == m_pParentDevice->GetDeviceAgent()));
        return (m_bNeedToCreateDevice ? m_pParentDevice->GetRootDevice()->CreateInstance() : CL_SUCCESS);
    }

    operator SharedPtr<FissionableDevice>&            () { return m_pParentDevice; }
    SharedPtr<FissionableDevice>&           operator->() { return m_pParentDevice; }
    const SharedPtr<FissionableDevice>&     operator* () const { return m_pParentDevice; }

private:
    bool                         m_bNeedToCreateDevice;
    SharedPtr<FissionableDevice> m_pParentDevice;
};

bool operator==( void* p, const ParentDeviceWrapper& me )
{
    return (p == (*me).GetPtr());
}

cl_err_code PlatformModule::GetHostTimer(cl_device_id device, cl_ulong* host_timestamp)
{
    SharedPtr<FissionableDevice> pDevice = m_mapDevices.GetOCLObject((_cl_device_id_int *)device).DynamicCast<FissionableDevice>();
    if (0 == pDevice)
    {
        return CL_INVALID_DEVICE;
    }
    if (nullptr == host_timestamp)
    {
        return CL_INVALID_VALUE;
    }

    *host_timestamp = Intel::OpenCL::Utils::HostTime();

    return CL_SUCCESS;
}

cl_err_code PlatformModule::GetDeviceAndHostTimer(cl_device_id device, cl_ulong* device_timestamp, cl_ulong* host_timestamp)
{
    SharedPtr<FissionableDevice> pDevice = m_mapDevices.GetOCLObject((_cl_device_id_int *)device).DynamicCast<FissionableDevice>();
    if (0 == pDevice)
    {
        return CL_INVALID_DEVICE;
    }
    if ((nullptr == host_timestamp) || (nullptr == device_timestamp))
    {
        return CL_INVALID_VALUE;
    }
    *host_timestamp = Intel::OpenCL::Utils::HostTime();
    *device_timestamp = pDevice->GetDeviceTimer();

    return CL_SUCCESS;
}

cl_err_code PlatformModule::clCreateSubDevices(cl_device_id device, const cl_device_partition_property *properties, cl_uint num_entries, cl_device_id *out_devices, cl_uint *num_devices)
{
    OclAutoMutex CS(&m_deviceFissionMutex);
    cl_uint numOutputDevices, numSubdevicesToCreate, tNumDevices;
    tNumDevices = 0;
    SharedPtr<OCLObject<_cl_device_id_int, _cl_platform_id_int> > pParent_obj = m_mapDevices.GetOCLObject((_cl_device_id_int*)device);

    if (0 == pParent_obj)
    {
        return CL_INVALID_DEVICE;
    }

    if (nullptr == properties)
    {
        return CL_INVALID_VALUE;
    }

    if (0 == num_entries)
    {
        if (nullptr != out_devices)
        {
            return CL_INVALID_VALUE;
        }
    }

    ParentDeviceWrapper pParentDevice( pParent_obj.StaticCast<FissionableDevice>() );

    cl_err_code ret = pParentDevice.CreateRootDevice();
    if (CL_SUCCESS != ret)
    {
        return ret;
    }

    //Get the number of devices to be generated
    ret = pParentDevice->FissionDevice(properties, 0, nullptr, &numOutputDevices, nullptr);
    if (ret != CL_SUCCESS)
    {
        return ret;
    }

    //if the user is only interested in count
    if (nullptr == out_devices)
    {
        if (nullptr == num_devices)
        {
            return CL_INVALID_VALUE;
        }
        *num_devices = numOutputDevices;
        return CL_SUCCESS;
    }

    //Else, if the user only needs some of the sub-devices we'll provide, adjust the number of output device to equal the number of entries in the device id list
    if (numOutputDevices > num_entries)
    {
        numOutputDevices = num_entries;
    }

    tNumDevices = numOutputDevices;

    if (0 == num_entries)
    {
        return CL_INVALID_VALUE;
    }

    numSubdevicesToCreate = numOutputDevices;

    cl_dev_subdevice_id* subdevice_ids = new cl_dev_subdevice_id[numSubdevicesToCreate];
    if (nullptr == subdevice_ids)
    {
        return CL_OUT_OF_HOST_MEMORY;
    }
    size_t* sizes = new size_t[numSubdevicesToCreate];
    if (nullptr == sizes)
    {
        delete[] subdevice_ids;
        return CL_OUT_OF_HOST_MEMORY;
    }

    ret = pParentDevice->FissionDevice(properties, num_entries, subdevice_ids, &tNumDevices, sizes);
    if (ret != CL_SUCCESS)
    {
        return ret;
    }
    //If we're here, the device was successfully fissioned. Create the new FissionableDevice objects and add them as appropriate
    SharedPtr<FissionableDevice>* pNewDevices = new SharedPtr<FissionableDevice>[numSubdevicesToCreate];
    if (nullptr == pNewDevices)
    {
        delete[] subdevice_ids;
        delete[] sizes;
        return CL_OUT_OF_HOST_MEMORY;
    }
    //Get the partitioning mode
    cl_int partitionMode = (cl_int)properties[0];
    if (CL_DEVICE_PARTITION_BY_AFFINITY_DOMAIN == partitionMode)
    {
        partitionMode = (cl_int)properties[1];
    }
    for (cl_uint i = 0; i < numSubdevicesToCreate; ++i)
    {
        pNewDevices[i] = SubDevice::Allocate(pParentDevice, sizes[i], subdevice_ids[i], properties);
        if (0 == pNewDevices[i])
        {
            for (cl_uint j = 0; j < i; ++j)
            {
                pNewDevices[j]->Release();
            }
            delete[] pNewDevices;
            delete[] sizes;
            delete[] subdevice_ids;
            return CL_OUT_OF_HOST_MEMORY;
        }
        out_devices[i] = pNewDevices[i]->GetHandle();
    }
    //Can close the device instance as the sub-devices now hold references on the device agent
    delete[] sizes;
    delete[] subdevice_ids;

    //Successful fission. Update device maps
    ret = AddDevices(pNewDevices, numSubdevicesToCreate);
    if (ret != CL_SUCCESS)
    {
        for (cl_uint i = 0; i < numSubdevicesToCreate; ++i)
        {
            pNewDevices[i]->Release();
        }
        delete[] pNewDevices;
        return ret;
    }

    delete[] pNewDevices;

    if (nullptr != num_devices)
    {
        *num_devices = tNumDevices;
    }

    return CL_SUCCESS;
}

cl_err_code PlatformModule::clReleaseDevice(cl_device_id device)
{
    SharedPtr<FissionableDevice> pDevice = m_mapDevices.GetOCLObject((_cl_device_id_int *)device).DynamicCast<FissionableDevice>();
    if (0 == pDevice)
    {
        return CL_INVALID_DEVICE;
    }
    if (pDevice->IsRootLevelDevice())
    {
        return CL_SUCCESS;
    }
    return m_mapDevices.ReleaseObject((_cl_device_id_int *)device);
}

void PlatformModule::RemoveAllDevices(bool preserve_user_handles)
{
    m_mapDevices.DisableAdding();
    if (preserve_user_handles)
    {
        m_mapDevices.SetPreserveUserHandles();
    }
    m_mapDevices.ReleaseAllObjects(false);
      m_pDefaultDevice = nullptr;

    if (nullptr != m_ppRootDevices)
    {
        delete[] m_ppRootDevices;
        m_ppRootDevices      = nullptr;
        m_uiRootDevicesCount = 0;
    }
}

cl_err_code PlatformModule::clRetainDevice(cl_device_id device)
{
    SharedPtr<FissionableDevice> pDevice = m_mapDevices.GetOCLObject((_cl_device_id_int *)device).DynamicCast<FissionableDevice>();
    if (0 == pDevice)
    {
        return CL_INVALID_DEVICE;
    }
    if (pDevice->IsRootLevelDevice())
    {
        return CL_SUCCESS;
    }
    return pDevice->Retain();
}


cl_err_code PlatformModule::AddDevices(SharedPtr<FissionableDevice>* ppDevices, unsigned int count)
{
    for (unsigned int i = 0; i < count; ++i)
    {
        m_mapDevices.AddObject(ppDevices[i]);
    }
    return CL_SUCCESS;
}


#if defined (DX_MEDIA_SHARING)
cl_int PlatformModule::GetDeviceIDsFromD3D(cl_platform_id clPlatform,
                                            cl_uint uiNumMediaAdapters,
                                            int *pMediaAdaptersType,
                                            void** ppMediaAdapters,
                                            int clD3dDeviceSet,
                                            cl_uint uiNumEntries,
                                            cl_device_id *pclDevices,
                                            cl_uint *puiNumDevices,
                                            const ID3DSharingDefinitions& d3dDefinitions)
{
    if (nullptr == clPlatform)
    {
        LOG_ERROR(TEXT("clPlatform is NULL"));
        return CL_INVALID_PLATFORM;
    }
    LOG_INFO(TEXT("Enter GetDeviceIDsFromD3D(clPlatform=%d, uiNumMediaAdapters=%d, pMediaAdaptersType=%p, ppMediaAdapters=%p, clD3dDeviceSet=%d, uiNumEntries=%d, pclDevices=%p, puiNumDevices=%p"),
        clPlatform, uiNumMediaAdapters, pMediaAdaptersType, ppMediaAdapters, clD3dDeviceSet, uiNumEntries, pclDevices, puiNumDevices);
    if (nullptr != pclDevices && 0 == uiNumEntries)
    {
        LOG_ERROR(TEXT("uiNumEntries is equal to zero and pclDevices is not NULL."));
        return CL_INVALID_VALUE;
    }
    if (nullptr == pclDevices && nullptr == puiNumDevices)
    {
        LOG_ERROR(TEXT("both puiNumDevices and pclDevices are NULL."));
        return CL_INVALID_VALUE;
    }
    if (!CheckPlatformId(clPlatform))
    {
        LOG_ERROR(TEXT("clPlatform is not a valid platform."));
        return CL_INVALID_PLATFORM;
    }
    if (d3dDefinitions.GetPreferredDevicesForD3D() != clD3dDeviceSet && d3dDefinitions.GetAllDevicesForD3D() != clD3dDeviceSet)
    {
        LOG_ERROR(TEXT("clD3dDeviceSet is not a valid value."));
        return CL_INVALID_VALUE;
    }
    if (0 == uiNumMediaAdapters || nullptr == pMediaAdaptersType || nullptr == ppMediaAdapters && d3dDefinitions.GetVersion() == ID3DSharingDefinitions::D3D9_KHR)
    {
        return CL_INVALID_VALUE;
    }
    if (nullptr == ppMediaAdapters && d3dDefinitions.GetVersion() == ID3DSharingDefinitions::D3D9_INTEL)
    {
        return CL_DEVICE_NOT_FOUND;
    }
    for (cl_uint i = 0; i < uiNumMediaAdapters; i++)
    {
        const int iType = pMediaAdaptersType[i];
        const vector<int>& validDeviceTypes = d3dDefinitions.GetValidDeviceTypes();
        if (find(validDeviceTypes.begin(), validDeviceTypes.end(), iType) == validDeviceTypes.end())
        {
            return CL_INVALID_VALUE;
        }
        /* ppMediaAdapters is defined to be of type void* by the spec. However, this is clearly a bug and it should be of type void**. The problem is that the conformance
            test indeed treats it like void* while passing 1 as uiNumMediaAdapters. This of course can't work for numbers greater than 1. Therefore I'm writing the condition
            like this. When the spec is fixed, I'll change it. */
        if (nullptr == (void*)ppMediaAdapters && 1 == uiNumMediaAdapters || uiNumMediaAdapters > 1 && nullptr != (void*)ppMediaAdapters && nullptr == ((void**)ppMediaAdapters)[i])
        {
            if (d3dDefinitions.GetVersion() == ID3DSharingDefinitions::D3D9_INTEL)
            {
                return CL_DEVICE_NOT_FOUND; // we return this to be aligned with GEN
            }
            else
            {
                return CL_INVALID_VALUE;
            }
        }
    }
    size_t szDevIndex = 0;
    if (nullptr != puiNumDevices)
    {
        *puiNumDevices = 0;
    }
    size_t szFoundDevices = 0;

    // in case of CL_PREFERRED_DEVICES, we won't return the CPU device, since it is not the preferred device.
    if (d3dDefinitions.GetAllDevicesForD3D() == clD3dDeviceSet)
    {
        // find all devices that report supporting Direct3D Sharing
        for (unsigned int i = 0; i < m_uiRootDevicesCount; i++)
        {
            size_t szParamValSize;

            cl_err_code err = m_ppRootDevices[i]->GetInfo(CL_DEVICE_EXTENSIONS, 0, nullptr, &szParamValSize);
            if (CL_FAILED(err))
            {
                return err;
            }
            clLocalArray<char> sDevEx(szParamValSize);
            err = m_ppRootDevices[i]->GetInfo(CL_DEVICE_EXTENSIONS, szParamValSize, (char*)sDevEx, nullptr);
            if (CL_FAILED(err))
            {
                return err;
            }
            if (std::string((char*)sDevEx).find(d3dDefinitions.GetExtensionName()) != std::string::npos)
            {
                szFoundDevices++;
                if (nullptr != pclDevices && szDevIndex < uiNumEntries)
                {
                    pclDevices[szDevIndex++] = m_ppRootDevices[i]->GetHandle();
                }
                if (nullptr != puiNumDevices)
                {
                    (*puiNumDevices)++;
                }
            }
        }
    }

    if (0 == szFoundDevices)
    {
        return CL_DEVICE_NOT_FOUND;
    }
    return CL_SUCCESS;
}
#endif
