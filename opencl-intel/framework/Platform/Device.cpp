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
//  Device.cpp
//  Implementation of the Class Device
//  Created on:      10-Dec-2008 2:08:23 PM
//  Original author: ulevy
///////////////////////////////////////////////////////////////////////////////////////////////////

#include <cl_sys_defines.h>
#include <CL/cl_gl.h>
#include <assert.h>
#include <stdarg.h>

#include "Device.h"
#include "observer.h"
#include "enqueue_commands.h"
#include "command_queue.h"
#include "cl_shared_ptr.hpp"
#include "platform_module.h"
#include <framework_proxy.h>
#include "cl_sys_info.h"

using namespace std;
using namespace Intel::OpenCL::Utils;
using namespace Intel::OpenCL::Framework;

PlatformModule* volatile Device::m_pPlatformModule = NULL;

Device::Device(_cl_platform_id_int* platform) :
    FissionableDevice(platform),m_bFrontEndCompilerDone(false),m_iNextClientId(1), m_pDeviceRefCount(0), m_devId(0), m_pDevice(NULL)
{
    // initialize logger client
    INIT_LOGGER_CLIENT(TEXT("Device"), LL_DEBUG);
    m_mapDeviceLoggerClinets[0] = GET_LOGGER_CLIENT;
    m_pFrontEndCompiler = NULL;

    LOG_DEBUG(TEXT("%s"), TEXT("Device constructor enter"));

    m_hGLContext = 0;
    m_hHDC = 0;
}

Device::~Device()
{
    LOG_DEBUG(TEXT("%s"), TEXT("Device destructor enter"));
}

void Device::Cleanup( bool bIsTerminate )
{
    // release logger clients
    map<cl_int,LoggerClient*>::iterator it = m_mapDeviceLoggerClinets.begin();
    while (it != m_mapDeviceLoggerClinets.end())
    {
        LoggerClient * pLoggerClient = it->second;
        if (NULL != pLoggerClient)
        {
            delete pLoggerClient;
        }
        it++;
    }
    m_mapDeviceLoggerClinets.clear();

    m_dlModule.Close();
    delete this;
}

cl_ulong Device::GetDeviceTimer() const
{
    return m_pFnClDevGetDeviceTimer();
}

cl_err_code    Device::GetInfo(cl_int param_name, size_t param_value_size, void * param_value, size_t * param_value_size_ret) const
{
    LOG_DEBUG(TEXT("Enter Device::GetInfo (param_name=%d, param_value_size=%d, param_value=%d, param_value_size_ret=%d"),
        param_name, param_value_size, param_value, param_value_size_ret);

    int clDevErr = CL_DEV_SUCCESS;
    size_t       szParamValueSize = 0;
    cl_device_id zeroHandle       = (cl_device_id)0;
    cl_uint      one              = 1;
    const cl_bool clFalse         = CL_FALSE;    
    
    const void * pValue = NULL;

    switch (param_name)
    {
    case CL_GL_CONTEXT_KHR:
        szParamValueSize = sizeof(cl_context_properties);
        pValue = &m_hGLContext;
        break;

    case CL_WGL_HDC_KHR:
        szParamValueSize = sizeof(cl_context_properties);
        pValue = &m_hHDC;
        break;

    case CL_DEVICE_PARENT_DEVICE:
        szParamValueSize = sizeof(cl_device_id);
        pValue = &zeroHandle;
        break;

    case CL_DEVICE_REFERENCE_COUNT:
        szParamValueSize = sizeof(cl_uint);
        pValue           = &one;
        break;

    case CL_DEVICE_PARTITION_TYPE:
        szParamValueSize = 0;
        break;

    case CL_DEVICE_PREFERRED_INTEROP_USER_SYNC:
        szParamValueSize = sizeof(cl_bool);
        pValue = &clFalse;
        break;

    default:
        size_t s;
        clDevErr = m_pFnClDevGetDeviceInfo(m_devId, param_name, param_value_size, param_value, &s);
        if ((clDevErr != (int)CL_DEV_SUCCESS) || (param_value && (param_value_size < s)))
        {
            return CL_INVALID_VALUE;
        }
        if (param_value_size_ret)
        {
            *param_value_size_ret = s;
        }
        return CL_SUCCESS;
    }

    // if param_value_size < actual value size return CL_INVALID_VALUE
    if (param_value && (param_value_size < szParamValueSize))
    {
        LOG_ERROR(TEXT("param_value_size (=%d) < szParamValueSize (=%d)"), param_value_size, szParamValueSize);
        return CL_INVALID_VALUE;
    }

    // return param value size
    if (NULL != param_value_size_ret)
    {
        *param_value_size_ret = szParamValueSize;
    }

    if (NULL != param_value && szParamValueSize > 0)
    {
        MEMCPY_S(param_value, param_value_size, pValue, szParamValueSize);
    }
    return CL_SUCCESS;
}

cl_err_code Device::CreateAndInitAllDevicesOfDeviceType(const char * psDeviceAgentDllPath, _cl_platform_id_int* pClPlatformId, vector< SharedPtr<Device> >* pOutDevices)
{
    Intel::OpenCL::Utils::OclDynamicLib dlModule(false);    // this is a work-around, because Vtune crashes when one of the libraries is loaded and then unloaded.
    // Load the DA library (First time); dlModule call to unload at destruction (when exiting from this function) BUT Device::InitDevice() is going to load it again before the unload...
    if (!dlModule.Load(Intel::OpenCL::Utils::GetFullModuleNameForLoad(psDeviceAgentDllPath)))
    {
        return CL_ERR_DEVICE_INIT_FAIL;
    }

    fn_clDevInitDeviceAgent* pFnClDevInitDeviceAgent = (fn_clDevInitDeviceAgent*)dlModule.GetFunctionPtrByName("clDevInitDeviceAgent");
    if ( NULL == pFnClDevInitDeviceAgent )
    {
        return CL_ERR_DEVICE_INIT_FAIL;
    }

    // Get pointer to the GetInfo function
    fn_clDevGetDeviceInfo*    pFnClDevGetDeviceInfo = (fn_clDevGetDeviceInfo*)dlModule.GetFunctionPtrByName("clDevGetDeviceInfo");
    if (NULL == pFnClDevGetDeviceInfo)
    {
        return CL_ERR_DEVICE_INIT_FAIL;
    }

    // Get pointer to the GetTimer function
    fn_clDevGetDeviceTimer*    pFnClDevGetDeviceTimer = (fn_clDevGetDeviceTimer*)dlModule.GetFunctionPtrByName("clDevGetDeviceTimer");
    if (NULL == pFnClDevGetDeviceTimer)
    {
        return CL_ERR_DEVICE_INIT_FAIL;
    }

    fn_clDevGetAvailableDeviceList* pFnClDevGetAvailableDeviceList = (fn_clDevGetAvailableDeviceList*)dlModule.GetFunctionPtrByName("clDevGetAvailableDeviceList");
    if (NULL == pFnClDevGetAvailableDeviceList)
    {
        return CL_ERR_DEVICE_INIT_FAIL;
    }


    if ( CL_DEV_FAILED(pFnClDevInitDeviceAgent()) )
    {
        return CL_ERR_DEVICE_INIT_FAIL;
    }

    size_t numDevicesInDeviceType = 0;
    cl_dev_err_code dev_err = pFnClDevGetAvailableDeviceList(0, NULL, &numDevicesInDeviceType);

    if ((CL_DEV_FAILED(dev_err)) || (0 == numDevicesInDeviceType))
    {
        return CL_ERR_DEVICE_INIT_FAIL;
    }

    unsigned int* deviceIdsList = (unsigned int*)STACK_ALLOC(sizeof(unsigned int) * numDevicesInDeviceType);
    if (NULL == deviceIdsList)
    {
        return CL_ERR_DEVICE_INIT_FAIL;
    }

    size_t numDevicesInDeviceTypeRet = 0;
    dev_err = pFnClDevGetAvailableDeviceList(numDevicesInDeviceType, deviceIdsList, &numDevicesInDeviceTypeRet);
    if ((CL_DEV_FAILED(dev_err)) || (numDevicesInDeviceTypeRet != numDevicesInDeviceType))
    {
        STACK_FREE(deviceIdsList);
        return CL_ERR_DEVICE_INIT_FAIL;
    }
    assert(numDevicesInDeviceType == numDevicesInDeviceTypeRet);

    cl_err_code clErrRet = CL_SUCCESS;
    cl_err_code clErr = CL_SUCCESS;
    for (unsigned int i = 0; i < numDevicesInDeviceTypeRet; i++)
    {
        // create new device object
        SharedPtr<Device> pDevice = Device::Allocate(pClPlatformId);
        if (0 == pDevice)
        {
            pOutDevices->clear();
            clErrRet = CL_OUT_OF_HOST_MEMORY;
            break;
        }

        clErr = pDevice->InitDevice(psDeviceAgentDllPath, pFnClDevGetDeviceInfo, pFnClDevGetDeviceTimer, deviceIdsList[i]);
        if (CL_FAILED(clErr))
        {
            clErrRet = clErr;
            pDevice = NULL;
            continue;
        }

        pOutDevices->push_back(pDevice);
    }

    STACK_FREE(deviceIdsList);
    return clErrRet;
}

cl_err_code Device::InitDevice(const char * psDeviceAgentDllPath, fn_clDevGetDeviceInfo* pFnClDevGetDeviceInfo, fn_clDevGetDeviceTimer* pFnClDevGetDeviceTimer, unsigned int devId)
{
    LogDebugA("Device::InitDevice enter. pwcDllPath=%s", psDeviceAgentDllPath);

    // Loading again the library in order to increase the reference counter of the library.
    LogDebugA("LoadLibrary(%s)", psDeviceAgentDllPath);

    if (!m_dlModule.Load(Intel::OpenCL::Utils::GetFullModuleNameForLoad(psDeviceAgentDllPath)))
    {
        LogErrorA("LoadLibrary(%s) failed", psDeviceAgentDllPath);
        return CL_ERR_DEVICE_INIT_FAIL;
    }

    m_pFnClDevGetDeviceInfo = pFnClDevGetDeviceInfo;
    m_pFnClDevGetDeviceTimer = pFnClDevGetDeviceTimer;
    m_devId = devId;

    m_stMaxLocalMemorySize = 0;
    cl_dev_err_code dev_err = m_pFnClDevGetDeviceInfo(m_devId, CL_DEVICE_LOCAL_MEM_SIZE, sizeof(cl_ulong), &m_stMaxLocalMemorySize, NULL);

    if (CL_DEV_SUCCEEDED( dev_err ))
    {
        dev_err = m_pFnClDevGetDeviceInfo(m_devId, CL_DEVICE_TYPE, sizeof(cl_device_type), &m_deviceType, NULL);
    }

    if (CL_DEV_SUCCEEDED( dev_err ))
    {
        dev_err = m_pFnClDevGetDeviceInfo(m_devId, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(m_CL_DEVICE_MAX_WORK_GROUP_SIZE), &m_CL_DEVICE_MAX_WORK_GROUP_SIZE, NULL);
    }

    if (CL_DEV_SUCCEEDED( dev_err ))
    {
        dev_err = m_pFnClDevGetDeviceInfo(m_devId, CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, sizeof(m_CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS), &m_CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, NULL);
    }

    if (CL_DEV_SUCCEEDED( dev_err ))
    {
        dev_err = m_pFnClDevGetDeviceInfo(m_devId, CL_DEVICE_MAX_WORK_ITEM_SIZES, sizeof(m_CL_DEVICE_MAX_WORK_ITEM_SIZES), &m_CL_DEVICE_MAX_WORK_ITEM_SIZES, NULL);
    }

    if (CL_DEV_SUCCEEDED( dev_err ))
    {
        cl_dev_err_code svm_dev_err = m_pFnClDevGetDeviceInfo(m_devId, CL_DEVICE_SVM_CAPABILITIES, sizeof(m_CL_DEVICE_SVM_CAPABILITIES), &m_CL_DEVICE_SVM_CAPABILITIES, NULL);
        m_bSvmSupported = CL_DEV_SUCCEEDED(svm_dev_err);
    }

    // Here we still don't have DeviceAgent instance intialized.
    // We should wait for CreateContext or Device Fission to create Device Agent instance, potentially saves memory footprint on Atom machines
    return CL_DEV_SUCCEEDED( dev_err ) ? CL_SUCCESS : CL_ERR_DEVICE_INIT_FAIL;
}

cl_err_code Device::CreateInstance()
{
    fn_clDevCreateDeviceInstance *devCreateInstance;
    if (0 == m_pDeviceRefCount)
    {
        LOG_DEBUG(TEXT("%s"), TEXT("Need to create a new device instance (Device::CreateInstance)"));
        OclAutoMutex CS(&m_deviceInitializationMutex);
        if (0 == m_pDeviceRefCount)
        {
            LOG_DEBUG(TEXT("%s"), TEXT("Creating new device instance (Device::CreateInstance)"));
            devCreateInstance = (fn_clDevCreateDeviceInstance*)m_dlModule.GetFunctionPtrByName("clDevCreateDeviceInstance");
            if (NULL == devCreateInstance)
            {
                LOG_ERROR(TEXT("%s"), TEXT("GetProcAddress(clDevCreateDeviceInstance) failed (devCreateInstance==NULL)"));
                return CL_ERR_DEVICE_INIT_FAIL;
            }

            LOG_DEBUG(TEXT("%s"), TEXT("Call Device::fn_clDevCreateDeviceInstance"));
            int clDevErr = devCreateInstance(m_devId, this, this, &m_pDevice, g_pUserLogger);
            if (clDevErr != (int)CL_DEV_SUCCESS)
            {
                LOG_ERROR(TEXT("Device::devCreateInstance returned %d"), clDevErr);
                return CL_DEVICE_NOT_AVAILABLE;
            }
            m_pDeviceRefCount++;
            if (NULL == m_pPlatformModule)
            {
                m_pPlatformModule = FrameworkProxy::Instance()->GetPlatformModule();
                assert( NULL != m_pPlatformModule );
            }
            m_pPlatformModule->DeviceCreated();
            LOG_DEBUG(TEXT("%s"), TEXT("Device::fn_clDevCreateDeviceInstance exit. (CL_SUCCESS)"));
            return CL_SUCCESS;
        }
    }
    m_pDeviceRefCount++;
    LOG_DEBUG(TEXT("%s"), TEXT("Device::CreateInstance exit without doing anything"));
    return CL_SUCCESS;
}

cl_err_code Device::CloseDeviceInstance()
{
    OclAutoMutex CS(&m_deviceInitializationMutex);
    LOG_DEBUG(TEXT("%s"), TEXT("CloseDeviceInstance enter"));
    if (0 == --m_pDeviceRefCount)
    {
        if ( !m_bTerminate )
        {
            m_pDevice->clDevCloseDevice();
            m_pPlatformModule->DeviceClosed();
        }
        m_pDevice = NULL;
    }
    assert(m_pDeviceRefCount>=0);
    return CL_SUCCESS;
}

cl_int Device::clLogCreateClient(cl_int device_id, const char* client_name, cl_int * client_id)
{
    if (NULL == client_id)
    {
        return CL_INVALID_VALUE;
    }

    if (!Logger::GetInstance().IsActive())
    {
        *client_id = 0;
        return CL_SUCCESS;
    }

    LoggerClient *pLoggerClient = new LoggerClient(client_name,LL_DEBUG);
    if (NULL == pLoggerClient)
    {
        return CL_ERR_LOGGER_FAILED;
    }
    *client_id = m_iNextClientId++;
    m_mapDeviceLoggerClinets[*client_id] = pLoggerClient;
    return CL_SUCCESS;
}

cl_int Device::clLogReleaseClient(cl_int client_id)
{
    map<cl_int,LoggerClient*>::iterator it =  m_mapDeviceLoggerClinets.find(client_id);
    if (it == m_mapDeviceLoggerClinets.end())
    {
        return CL_ERR_KEY_NOT_FOUND;
    }
    LoggerClient *pLoggerClient = it->second;
    delete pLoggerClient;
    m_mapDeviceLoggerClinets.erase(it);

    return CL_SUCCESS;
}

cl_int Device::clLogAddLine(cl_int client_id, cl_int log_level,
                                const char* IN source_file,
                                const char* IN function_name,
                                cl_int line_num,
                                const char* IN message, ...)
{
    map<cl_int,LoggerClient*>::iterator it =  m_mapDeviceLoggerClinets.find(client_id);
    if (it == m_mapDeviceLoggerClinets.end())
    {
        return CL_ERR_KEY_NOT_FOUND;
    }
    LoggerClient *pLoggerClient = it->second;
    if (NULL != pLoggerClient)
    {
        va_list va;
        va_start(va, message);

        pLoggerClient->LogArgList((ELogLevel)log_level, source_file, function_name, line_num, message, va);

        va_end(va);
    }
    return CL_SUCCESS;
}

void Device::clDevBuildStatusUpdate(cl_dev_program clDevProg, void * pData, cl_build_status clBuildStatus)
{
    IBuildDoneObserver * pBuildDoneObserver = (IBuildDoneObserver*)pData;

    assert(pBuildDoneObserver);
    pBuildDoneObserver->NotifyBuildDone((cl_device_id)m_iId, clBuildStatus);
    return;
}

void Device::clDevCmdStatusChanged(cl_dev_cmd_id cmd_id, void * pData, cl_int cmd_status, cl_int status_result, cl_ulong timer)
{
    if (NULL == pData)  // it's a device-side command
    {
        return;
    }
    ICmdStatusChangedObserver *pObserver = (ICmdStatusChangedObserver *)pData;

    pObserver->NotifyCmdStatusChanged(cmd_id, cmd_status, 
                                      (cl_int(CL_DEV_COMMAND_CANCELLED)==status_result) ? CL_DEVICE_NOT_AVAILABLE : status_result, 
                                      timer);
    return;
}

Intel::OpenCL::TaskExecutor::ITaskExecutor* Device::clDevGetTaskExecutor()
{
    return FrameworkProxy::Instance()->GetTaskExecutor();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// PlatformModule::InitFECompilers
///////////////////////////////////////////////////////////////////////////////////////////////////
void Device::InitFECompiler() const
{
    const IOCLDeviceFECompilerDescription* pFEConfig = m_pDevice->clDevGetFECompilerDecription();
    if (NULL == pFEConfig)
    {
        // device doesn't have front-end compiler
        m_pFrontEndCompiler = NULL;
        return;
    }
    string strModule = pFEConfig->clDevFEModuleName();
    m_pFrontEndCompiler = FrontEndCompiler::Allocate();

    if (0 == m_pFrontEndCompiler)
    {
        assert( false && "Cannot allocate wrapper class for FrontEndCompiler" );
        return;
    }

    cl_err_code clErrRet = m_pFrontEndCompiler->Initialize(OS_DLL_POST(strModule).c_str(),
                                                           pFEConfig->clDevFEDeviceInfo(), 
                                                           pFEConfig->clDevFEDeviceInfoSize() );
    if (CL_FAILED(clErrRet))
    {
        assert( false && "FrontEndCompiler initialization failed" );
        m_pFrontEndCompiler = NULL;
    }
}

const SharedPtr<FrontEndCompiler>& Device::GetFrontEndCompiler() const 
{ 
    if (!m_bFrontEndCompilerDone)
    {
        OclAutoMutex CS(&m_deviceInitializationMutex);
        if (!m_bFrontEndCompilerDone)
        {
            InitFECompiler();
            m_bFrontEndCompilerDone = true;
        }
    }
    return m_pFrontEndCompiler; 
}

cl_err_code FissionableDevice::FissionDevice(const cl_device_partition_property* props, cl_uint num_entries, cl_dev_subdevice_id* out_devices, cl_uint* num_devices, size_t* sizes)
{
    cl_err_code ret = CL_SUCCESS;
    cl_dev_err_code dev_ret = CL_DEV_SUCCESS;
    m_default_command_queue = NULL;
    //identify the partition mode and translate to device enum
    cl_dev_partition_prop partitionMode;

    // Disable device fission on Android
#if defined __ANDROID__
    return CL_INVALID_VALUE;
#endif

    switch (props[0])
    {
    case CL_DEVICE_PARTITION_EQUALLY:
        partitionMode = CL_DEV_PARTITION_EQUALLY;
        break;

    case CL_DEVICE_PARTITION_BY_COUNTS:
        partitionMode = CL_DEV_PARTITION_BY_COUNTS;
        break;

    case CL_DEVICE_PARTITION_BY_NAMES_INTEL:
        partitionMode = CL_DEV_PARTITION_BY_NAMES;
        break;

    case CL_DEVICE_PARTITION_BY_AFFINITY_DOMAIN:
        switch (props[1])
        {
        case CL_DEVICE_AFFINITY_DOMAIN_L1_CACHE:
            partitionMode = CL_DEV_PARTITION_AFFINITY_L1;
            break;

        case CL_DEVICE_AFFINITY_DOMAIN_L2_CACHE:
            partitionMode = CL_DEV_PARTITION_AFFINITY_L2;
            break;

        case CL_DEVICE_AFFINITY_DOMAIN_L3_CACHE:
            partitionMode = CL_DEV_PARTITION_AFFINITY_L3;
            break;

        case CL_DEVICE_AFFINITY_DOMAIN_L4_CACHE:
            partitionMode = CL_DEV_PARTITION_AFFINITY_L4;
            break;

        case CL_DEVICE_AFFINITY_DOMAIN_NUMA:
            partitionMode = CL_DEV_PARTITION_AFFINITY_NUMA;
            break;

        case CL_DEVICE_AFFINITY_DOMAIN_NEXT_PARTITIONABLE:
            partitionMode = CL_DEV_PARTITION_AFFINITY_NEXT;
            break;

        default:
            return CL_INVALID_VALUE;
        }
        break;

    default:
        return CL_INVALID_VALUE;
    }

    // prepare additional info for the CPU device, for counts / equally / names
    if (CL_DEV_PARTITION_BY_COUNTS == partitionMode)
    {
        std::vector<size_t> partitionSizes;
        size_t partitionIndex = 1;
        cl_uint maxSubDevices;
        while (0 != props[partitionIndex])
        {
            partitionSizes.push_back((size_t)props[partitionIndex++]);
        }
        if (0 == partitionSizes.size() ||
            GetInfo(CL_DEVICE_PARTITION_MAX_SUB_DEVICES, sizeof(maxSubDevices), &maxSubDevices, NULL) != CL_SUCCESS ||
            partitionSizes.size() > maxSubDevices)
        {
            return CL_DEVICE_PARTITION_FAILED;
        }
        if (NULL != sizes)
        {
            for (size_t i = 0; i < partitionSizes.size(); ++i)
            {
                sizes[i] = (size_t)partitionSizes[i];
            }
        }
        //If the user doesn't actually want fission, no reason to send it to the device, just return the size
        if (NULL == out_devices)
        {
            *num_devices = (cl_uint)partitionSizes.size();
            return CL_SUCCESS;
        }
        dev_ret = GetDeviceAgent()->clDevPartition(partitionMode, num_entries, GetSubdeviceId(), num_devices, &partitionSizes, out_devices);
    }
    else if (CL_DEV_PARTITION_EQUALLY == partitionMode)
    {
        size_t partitionSize = (size_t)props[1];
        if (0 != props[2])
        {
            return CL_INVALID_VALUE;
        }
        if (0 == partitionSize)
        {
            return CL_DEVICE_PARTITION_FAILED;
        }

        dev_ret = GetDeviceAgent()->clDevPartition(partitionMode, num_entries, GetSubdeviceId(), num_devices, &partitionSize, out_devices);
        if (NULL != sizes)
        {
            if (CL_DEV_SUCCESS == dev_ret)
            {
                for (cl_uint i = 0; i < *num_devices; ++i )
                {
                    sizes[i] = partitionSize;
                }
            }
        }
    }
    else if (CL_DEV_PARTITION_BY_NAMES == partitionMode)
    {
        std::vector<size_t> requestedUnits;
        size_t partitionIndex = 1;
        while ((cl_device_partition_property)CL_PARTITION_BY_NAMES_LIST_END_INTEL != props[partitionIndex])
        {
            requestedUnits.push_back((size_t)props[partitionIndex++]);
        }
        if (0 != props[partitionIndex + 1])
        {
            return CL_INVALID_VALUE;
        }
        if (NULL != sizes)
        {
            *sizes = partitionIndex - 1;
        }
        //If the user doesn't actually want fission, no reason to send it to the device, just return the size
        if (NULL == out_devices)
        {
            *num_devices = 1;
            return CL_SUCCESS;
        }
        dev_ret = GetDeviceAgent()->clDevPartition(partitionMode, num_entries, GetSubdeviceId(), num_devices, &requestedUnits, out_devices);
    }
    else if (CL_DEV_PARTITION_AFFINITY_NUMA == partitionMode)
    {
        dev_ret = GetDeviceAgent()->clDevPartition(partitionMode, num_entries, GetSubdeviceId(), num_devices, sizes, out_devices);
    }
    else // no other mode today requires an additional param
    {
        dev_ret = GetDeviceAgent()->clDevPartition(partitionMode, num_entries, GetSubdeviceId(), num_devices, NULL, out_devices);
    }
    if (CL_SUCCESS != ret)
    {
        return ret;
    }
    if (CL_DEV_SUCCESS == dev_ret)
    {
        return ret;
    }
    if (CL_DEV_INVALID_PROPERTIES == dev_ret)
    {
        //Unsupported fission mode
        return CL_INVALID_VALUE;
    }
    return CL_DEVICE_PARTITION_FAILED;
}


cl_err_code
FissionableDevice::SetDefaultDeviceQueue(OclCommandQueue* command_queue,
                                         cl_dev_cmd_list  clDevCmdListId)
{
    assert(command_queue && "Invalid command queue passed.");
    assert(clDevCmdListId && "Invalid command list passed.");
    OclAutoMutex CS(&m_changeDefaultDeviceMutex);
    // The next two operations should be executed under a mutex otherwise
    // races may occur.
    m_default_command_queue = command_queue;
    return GetDeviceAgent()->clDevSetDefaultCommandList(clDevCmdListId);
}

cl_err_code
FissionableDevice::UnsetDefaultQueueIfEqual(OclCommandQueue* command_queue)
{
    OclAutoMutex CS(&m_changeDefaultDeviceMutex);
    // The next two operations should be executed under a mutex otherwise
    // races may occur.
    const OclCommandQueue* res =
        m_default_command_queue.test_and_set(command_queue, NULL);
    // If command queue was default command queue
    // than unset corresponding command list
    if (command_queue == res)
        return GetDeviceAgent()->clDevSetDefaultCommandList(NULL);
    return CL_SUCCESS;
}

OclCommandQueue* FissionableDevice::GetDefaultDeviceQueue()
{
    if (m_default_command_queue)
    {
        return m_default_command_queue;
    }
    return CL_INVALID_HANDLE;
}

bool FissionableDevice::IsImageFormatSupported(const cl_image_format& clImgFormat, cl_mem_flags clMemFlags, cl_mem_object_type clMemObjType) const
{
    cl_uint uiNumEntries;
    bool bSupported = false;
    cl_dev_err_code clErr = GetDeviceAgent()->clDevGetSupportedImageFormats(clMemFlags, clMemObjType, 0, NULL, &uiNumEntries);
    assert(CL_SUCCESS == clErr);    
    cl_image_format* const pFormats = new cl_image_format[uiNumEntries];

    if (NULL == pFormats)
    {
        LOG_ERROR(TEXT("out of memory"), "");
        return false;
    }
    clErr = GetDeviceAgent()->clDevGetSupportedImageFormats(clMemFlags, clMemObjType, uiNumEntries, pFormats, NULL);
    assert(CL_SUCCESS == clErr);
    for (cl_uint i = 0; i < uiNumEntries; i++)
    {
        if (pFormats[i].image_channel_data_type == clImgFormat.image_channel_data_type &&
            pFormats[i].image_channel_order == clImgFormat.image_channel_order)
        {
            bSupported = true;
            break;
        }
    }
    delete[] pFormats;
    return bSupported;
}

SubDevice::SubDevice(SharedPtr<FissionableDevice>pParent, size_t numComputeUnits, cl_dev_subdevice_id id, const cl_device_partition_property* props) :
    FissionableDevice((_cl_platform_id_int *)pParent->GetHandle()), m_pParentDevice(pParent), m_deviceId(id),
        m_numComputeUnits(numComputeUnits), m_cachedFissionMode(NULL), m_cachedFissionLength(0)
{
    m_pRootDevice = m_pParentDevice->GetRootDevice();
    CacheFissionProperties(props);
    //Todo: handle more intelligently
    m_pRootDevice->CreateInstance();
}

SubDevice::~SubDevice()
{
    if (NULL != m_cachedFissionMode)
    {
        delete []m_cachedFissionMode;
    }
    IOCLDeviceAgent* pRootAgent = GetDeviceAgent();
    if ( !m_bTerminate && (NULL != pRootAgent) )
    {
        pRootAgent->clDevReleaseSubdevice(m_deviceId);
    }
    //Todo: handle more intelligently
    m_pRootDevice->CloseDeviceInstance();
}

cl_err_code SubDevice::GetInfo(cl_int param_name, size_t param_value_size, void *param_value, size_t *param_value_size_ret) const
{
    size_t szParamValueSize = 0;
    cl_uint uValue = 0;
    cl_device_id clDevIdVal = 0;
    const void * pValue = NULL;

    switch (param_name)
    {
    case CL_DEVICE_PARTITION_MAX_SUB_DEVICES: 
        szParamValueSize = sizeof(cl_uint);
        uValue = m_numComputeUnits > 1 ? (cl_uint)m_numComputeUnits : 0;
        //If this was created by PARTITION_BY_NAMES, no more sub-devices allowed
        if (CL_DEVICE_PARTITION_BY_NAMES_INTEL == m_fissionMode)
        {
            uValue = 0;
        }
        pValue = &uValue;
        break;

    case CL_DEVICE_MAX_COMPUTE_UNITS:
        szParamValueSize = sizeof(cl_uint);
        uValue = (cl_uint)m_numComputeUnits;
        pValue = &uValue;
        break;

    //Todo: handle these
    case CL_DEVICE_PARENT_DEVICE:
        szParamValueSize = sizeof(cl_device_id);
        clDevIdVal = m_pParentDevice->GetHandle();
        pValue = &clDevIdVal;
        break;

    //CL_DEVICE_PARTITION_TYPES_EXT and CL_DEVICE_AFFINITY_DOMAINS_EXT handled on root-level device

    case CL_DEVICE_REFERENCE_COUNT:
        szParamValueSize = sizeof(cl_uint);
        pValue = &m_uiRefCount;
        break;

    case CL_DEVICE_PARTITION_TYPE:
        szParamValueSize = m_cachedFissionLength * sizeof(cl_device_partition_property);
        pValue = m_cachedFissionMode;
        break;

    default:
        return m_pRootDevice->GetInfo(param_name, param_value_size, param_value, param_value_size_ret);
    }

    // if param_value_size < actual value size return CL_INVALID_VALUE
    if (NULL != param_value && param_value_size < szParamValueSize)
    {
        LOG_ERROR(TEXT("param_value_size (=%d) < szParamValueSize (=%d)"), param_value_size, szParamValueSize);
        return CL_INVALID_VALUE;
    }

    // return param value size
    if (NULL != param_value_size_ret)
    {
        *param_value_size_ret = szParamValueSize;
    }

    if (NULL != param_value && szParamValueSize > 0)
    {
        MEMCPY_S(param_value, param_value_size, pValue, szParamValueSize);
    }
    return CL_SUCCESS;
}

void SubDevice::CacheFissionProperties(const cl_device_partition_property* props)
{
    m_cachedFissionLength = 0;
    //Todo: don't copy the partition properties for every sub-device, keep it in the parent
    if (props)
    {
        m_fissionMode = (cl_int)props[0];
        if (CL_DEVICE_PARTITION_AFFINITY_DOMAIN == m_fissionMode)
        {
            m_fissionMode = (cl_int)props[1];
        }

        //Ninja-style is still the most readable here, I think
        while (props[m_cachedFissionLength++] != 0)
        {
            //Nothing, I'm just counting the property list length 
        }
        m_cachedFissionMode = new cl_device_partition_property[m_cachedFissionLength];
        if (NULL == m_cachedFissionMode)
        {
            //Todo: what?
            return;
        }
        MEMCPY_S(m_cachedFissionMode, m_cachedFissionLength * sizeof(cl_device_partition_property), props, m_cachedFissionLength * sizeof(cl_device_partition_property));
    }
}
