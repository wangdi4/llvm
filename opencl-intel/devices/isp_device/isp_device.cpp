// Copyright (c) 2013 Intel Corporation
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

///////////////////////////////////////////////////////////
//  isp_device.cpp
///////////////////////////////////////////////////////////

#include "stdafx.h"

#include "isp_device.h"
#include "isp_logger.h"

#include <buildversion.h>
#include <cl_sys_info.h>

#include <string>
#include <sstream>

//TODO
//#include <iostream>

// TODO: verify this, change to 3
#define ISP_MAX_WORK_ITEM_DIMENSIONS 1
#define ISP_MAX_WORK_GROUP_SIZE 1
static const size_t ISP_MAX_WORK_ITEM_SIZES[ISP_MAX_WORK_ITEM_DIMENSIONS] =
{
    ISP_MAX_WORK_GROUP_SIZE
};

#define ISP_GLOBAL_MEM_SIZE Intel::OpenCL::Utils::TotalPhysicalSize()

#define VENDOR_STRING "Intel(R) Corporation"

using namespace Intel::OpenCL::ISPDevice;

ISPDevice::ISPDevice(cl_uint uiDevId, IOCLFrameworkCallbacks* frameworkCallbacks, IOCLDevLogDescriptor *logDesc) :
    m_uiIspId(uiDevId),
    m_pLogDescriptor(logDesc),
    m_iLogHandle(0),
    m_pFrameworkCallbacks(frameworkCallbacks),
    m_pCameraShim(nullptr),
    m_pProgramService(nullptr),
    m_pMemoryAllocator(nullptr),
    m_pTaskDispatcher(nullptr)
{
}

ISPDevice::~ISPDevice()
{
}

cl_dev_err_code ISPDevice::Init()
{
    cl_dev_err_code ret = CL_DEV_SUCCESS;

    // create the logger client before anything else
    if (nullptr != m_pLogDescriptor)
    {
        ret = (cl_dev_err_code) m_pLogDescriptor->clLogCreateClient(m_uiIspId, "ISP Device", &m_iLogHandle);
        if (CL_DEV_FAILED(ret))
        {
            return CL_DEV_ERROR_FAIL;
        }
    }
    IspInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("ISPDevice initialize enter"));

    // TODO: Hard-coded app name...
    // TODO: change CameraShim to ISPCameraService
    m_pCameraShim = CameraShim::instance("com.example.opencldemo", 0);
    if (nullptr == m_pCameraShim)
    {
        IspErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("Connecting to Camera Service has failed"));
        return CL_DEV_ERROR_FAIL;
    }

    m_pProgramService = new ISPProgramService(m_uiIspId, m_pLogDescriptor, m_pCameraShim);
    if (nullptr == m_pProgramService)
    {
        IspErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("Cannot allocate memory for ISP program service"));
        return CL_DEV_OUT_OF_MEMORY;
    }
    ret = m_pProgramService->Init();
    if (CL_DEV_FAILED(ret))
    {
        IspErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("Initializing ISP program service has failed"));
        return CL_DEV_ERROR_FAIL;
    }

    m_pMemoryAllocator = new ISPMemoryAllocator(m_uiIspId, m_pLogDescriptor, ISP_GLOBAL_MEM_SIZE);
    if (nullptr == m_pMemoryAllocator)
    {
        IspErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("Cannot allocate memory for ISP memory allocator"));
        return CL_DEV_OUT_OF_MEMORY;
    }
    ret = m_pMemoryAllocator->Init();
    if (CL_DEV_FAILED(ret))
    {
        IspErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("Initializing ISP memory allocator has failed"));
        return CL_DEV_ERROR_FAIL;
    }

    m_pTaskDispatcher = new ISPTaskDispatcher(m_uiIspId, m_pLogDescriptor, m_pFrameworkCallbacks, m_pCameraShim);
    if (nullptr == m_pTaskDispatcher)
    {
        IspErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("Cannot allocate memory for ISP task dispatcher"));
        return CL_DEV_OUT_OF_MEMORY;
    }
    ret = m_pTaskDispatcher->Init();
    if (CL_DEV_FAILED(ret))
    {
        IspErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("Initializing ISP task dispatcher has failed"));
        return CL_DEV_ERROR_FAIL;
    }
}

//Device Information function prototypes
/************************************************************************************************************************
   clDevGetDeviceInfo
    Description
        This function return device specific information defined by cl_device_info enumeration as specified in OCL spec. table 4.3.
    Input
        dev_id                  The device ID in specific device type.
        param                   An enumeration that identifies the device information being queried. It can be one of
                                    the following values as specified in OCL spec. table 4.3
        valSize                 Specifies the size in bytes of memory pointed to by paramValue. This size in
                                    bytes must be >= size of return type
    Output
        paramVal                A pointer to memory location where appropriate values for a given param as specified in OCL spec. table 4.3 will be returned. If paramVal is NULL, it is ignored
        paramValSizeRet         Returns the actual size in bytes of data being queried by paramVal. If paramValSizeRet is NULL, it is ignored
    Returns
        CL_DEV_SUCCESS          If functions is executed successfully.
        CL_DEV_INVALID_VALUE    If param_name is not one of the supported values or if size in bytes specified by paramValSize is < size of return type as specified in OCL spec. table 4.3 and paramVal is not a NULL value
**************************************************************************************************************************/
cl_dev_err_code ISPDevice::clDevGetDeviceInfo(unsigned int IN dev_id, cl_device_info IN param, size_t IN valSize,
                                              void* OUT paramVal, size_t* OUT paramValSizeRet)
{
    size_t  internalRetunedValueSize = valSize;
    size_t  *pinternalRetunedValueSize;

    //if OUT paramValSize_ret is NULL it should be ignored
    if(paramValSizeRet)
    {
        pinternalRetunedValueSize = paramValSizeRet;
    }
    else
    {
        pinternalRetunedValueSize = &internalRetunedValueSize;
    }

    switch (param)
    {
        case (CL_DEVICE_TYPE):
        {
            *pinternalRetunedValueSize = sizeof(cl_device_type);
            if(nullptr != paramVal && valSize < *pinternalRetunedValueSize)
            {
                return CL_DEV_INVALID_VALUE;
            }
            //if OUT paramVal is NULL it should be ignored
            if(nullptr != paramVal)
            {
                *(cl_device_type*)paramVal = (cl_device_type)CL_DEVICE_TYPE_CUSTOM;
            }
            return CL_DEV_SUCCESS;
        }

        case (CL_DEVICE_VENDOR_ID):
        {
            *pinternalRetunedValueSize = sizeof(cl_uint);
            if(nullptr != paramVal && valSize < *pinternalRetunedValueSize)
            {
                return CL_DEV_INVALID_VALUE;
            }
            //if OUT paramVal is NULL it should be ignored
            if(nullptr != paramVal)
            {
                    *(cl_uint*)paramVal = 0x8086;
            }
            return CL_DEV_SUCCESS;
        }

        case (CL_DEVICE_PARTITION_MAX_SUB_DEVICES):
        {
            *pinternalRetunedValueSize = sizeof(cl_uint);
            if(nullptr != paramVal && valSize < *pinternalRetunedValueSize)
            {
                return CL_DEV_INVALID_VALUE;
            }
            //if OUT paramVal is NULL it should be ignored
            if(nullptr != paramVal)
            {
                // No device partitions for now
                *(cl_uint*)paramVal = 0;
            }
            return CL_DEV_SUCCESS;
        }

        case (CL_DEVICE_MAX_COMPUTE_UNITS):
        {
            *pinternalRetunedValueSize = sizeof(cl_uint);
            if(nullptr != paramVal && valSize < *pinternalRetunedValueSize)
            {
                return CL_DEV_INVALID_VALUE;
            }
            //if OUT paramVal is NULL it should be ignored
            if(nullptr != paramVal)
            {
                *(cl_uint*)paramVal = 1;
            }
            return CL_DEV_SUCCESS;
        }

        case (CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS):
        {
            *pinternalRetunedValueSize = sizeof(cl_uint);
            if(nullptr != paramVal && valSize < *pinternalRetunedValueSize)
            {
                return CL_DEV_INVALID_VALUE;
            }
            //if OUT paramVal is NULL it should be ignored
            if(nullptr != paramVal)
            {
                *(cl_uint*)paramVal = ISP_MAX_WORK_ITEM_DIMENSIONS;
            }
            return CL_DEV_SUCCESS;
        }

        case (CL_DEVICE_MAX_WORK_GROUP_SIZE):
        {
            *pinternalRetunedValueSize = sizeof(size_t);
            if(nullptr != paramVal && valSize < *pinternalRetunedValueSize)
            {
                return CL_DEV_INVALID_VALUE;
            }
            //if OUT paramVal is NULL it should be ignored
            if(nullptr != paramVal)
            {
                *(size_t*)paramVal = ISP_MAX_WORK_GROUP_SIZE;
            }
            return CL_DEV_SUCCESS;
        }

        case (CL_DEVICE_MAX_WORK_ITEM_SIZES):
        {
            *pinternalRetunedValueSize = ISP_MAX_WORK_ITEM_DIMENSIONS * sizeof(size_t);
            if(nullptr != paramVal && valSize < *pinternalRetunedValueSize)
            {
                return CL_DEV_INVALID_VALUE;
            }
            //if OUT paramVal is NULL it should be ignored
            if(nullptr != paramVal)
            {
                // TODO: unsafe copy
                memcpy(paramVal, ISP_MAX_WORK_ITEM_SIZES, *pinternalRetunedValueSize);
            }
            return CL_DEV_SUCCESS;
        }

        case (CL_DEVICE_MAX_MEM_ALLOC_SIZE):
        case (CL_DEVICE_GLOBAL_MEM_SIZE):
        {
            *pinternalRetunedValueSize = sizeof(cl_ulong);
            if(nullptr != paramVal && valSize < *pinternalRetunedValueSize)
            {
                return CL_DEV_INVALID_VALUE;
            }
            //if OUT paramVal is NULL it should be ignored
            if(nullptr != paramVal)
            {
                *(cl_ulong*)paramVal = ISP_GLOBAL_MEM_SIZE;
            }
            return CL_DEV_SUCCESS;
        }

        case (CL_DEVICE_LOCAL_MEM_TYPE):
        {
            *pinternalRetunedValueSize = sizeof(cl_device_local_mem_type);
            if(nullptr != paramVal && valSize < *pinternalRetunedValueSize)
            {
                return CL_DEV_INVALID_VALUE;
            }
            //if OUT paramVal is NULL it should be ignored
            if(nullptr != paramVal)
            {
                *(cl_device_local_mem_type*)paramVal = CL_NONE;
            }
            return CL_DEV_SUCCESS;
        }

        case (CL_DEVICE_LOCAL_MEM_SIZE):
        {
            *pinternalRetunedValueSize = sizeof(cl_ulong);
            if(nullptr != paramVal && valSize < *pinternalRetunedValueSize)
            {
                return CL_DEV_INVALID_VALUE;
            }
            //if OUT paramVal is NULL it should be ignored
            if(nullptr != paramVal)
            {
                *(cl_ulong*)paramVal = 0;
            }
            return CL_DEV_SUCCESS;
        }

        case (CL_DEVICE_AVAILABLE):
        {
            *pinternalRetunedValueSize = sizeof(cl_bool);
            if(nullptr != paramVal && valSize < *pinternalRetunedValueSize)
            {
                return CL_DEV_INVALID_VALUE;
            }
            //if OUT paramVal is NULL it should be ignored
            if(nullptr != paramVal)
            {
                *(cl_bool*)paramVal = CL_TRUE;
            }
            return CL_DEV_SUCCESS;
        }

        //TODO
        case (CL_DEVICE_EXECUTION_CAPABILITIES):
        {
            *pinternalRetunedValueSize = sizeof(cl_device_exec_capabilities);
            if(nullptr != paramVal && valSize < *pinternalRetunedValueSize)
            {
                return CL_DEV_INVALID_VALUE;
            }
            //if OUT paramVal is NULL it should be ignored
            if(nullptr != paramVal)
            {
                *(cl_device_exec_capabilities*)paramVal = (cl_device_exec_capabilities)0;
            }
            return CL_DEV_SUCCESS;
        }

        // TODO
        case (CL_DEVICE_HOST_UNIFIED_MEMORY):
        {
            *pinternalRetunedValueSize = sizeof(cl_bool);
            if(nullptr != paramVal && valSize < *pinternalRetunedValueSize)
            {
                return CL_DEV_INVALID_VALUE;
            }
            //if OUT paramVal is NULL it should be ignored
            if(nullptr != paramVal)
            {
                *(cl_bool*)paramVal = CL_TRUE;
            }
            return CL_DEV_SUCCESS;
        }

        case (CL_DEVICE_NAME):
        {
            // TODO: hard coded for now...
            const char* name = "Intel(R) Atom ISP";
            *pinternalRetunedValueSize = (nullptr == name) ? 0 : (strlen(name) + 1);
            if(nullptr != paramVal && valSize < *pinternalRetunedValueSize)
            {
                return CL_DEV_INVALID_VALUE;
            }
            //if OUT paramVal is NULL it should be ignored
            if((nullptr != paramVal) && (nullptr != name))
            {
                STRCPY_S((char*)paramVal, valSize, name);
            }
            return CL_DEV_SUCCESS;
        }

        case (CL_DEVICE_VENDOR):
        {
            *pinternalRetunedValueSize = strlen(VENDOR_STRING) + 1;
            if(nullptr != paramVal && valSize < *pinternalRetunedValueSize)
            {
                return CL_DEV_INVALID_VALUE;
            }
            //if OUT paramVal is NULL it should be ignored
            if(nullptr != paramVal)
            {
                STRCPY_S((char*)paramVal, valSize, VENDOR_STRING);
            }
            return CL_DEV_SUCCESS;
        }

        //TODO
        case (CL_DEVICE_PROFILE):
        {
            *pinternalRetunedValueSize = strlen("EMBEDDED_PROFILE") + 1;
            if(nullptr != paramVal && valSize < *pinternalRetunedValueSize)
            {
                return CL_DEV_INVALID_VALUE;
            }
            //if OUT paramVal is NULL it should be ignored
            if(nullptr != paramVal)
            {
                STRCPY_S((char*)paramVal, valSize, "EMBEDDED_PROFILE");
            }
            return CL_DEV_SUCCESS;
        }

        case (CL_DEVICE_VERSION):
        {
            std::string sDeviceVersion = "OpenCL 1.2 ";
            sDeviceVersion += BUILDVERSIONSTR;
            *pinternalRetunedValueSize = sDeviceVersion.size() + 1;
            if(nullptr != paramVal && valSize < *pinternalRetunedValueSize)
            {
                return CL_DEV_INVALID_VALUE;
            }
            //if OUT paramVal is NULL it should be ignored
            if(nullptr != paramVal)
            {
                STRCPY_S((char*)paramVal, valSize, sDeviceVersion.c_str());
            }
            return CL_DEV_SUCCESS;
        }

        case (CL_DRIVER_VERSION):
        {
            int major = 0;
            int minor = 0;
            int revision = 0;
            int build = 0;
            std::stringstream driverVerStream;
            if (Intel::OpenCL::Utils::GetModuleProductVersion(__FUNCTION__, &major, &minor, &revision, &build))
            {
                // format is (Major version).(Minor version).(Revision number).(Build number)
                driverVerStream << major << "." << minor << "." << revision << "." << build;
            }
            else
            {
                // TODO: remove this once GetModuleProductVersion is implemented on Linux
                driverVerStream << "1.2.0." << (int)BUILDVERSION;
            }
            std::string driverVer = driverVerStream.str();

            *pinternalRetunedValueSize = driverVer.size() + 1;
            if(nullptr != paramVal && valSize < *pinternalRetunedValueSize)
            {
                return CL_DEV_INVALID_VALUE;
            }
            //if OUT paramVal is NULL it should be ignored
            if(nullptr != paramVal)
            {
                STRCPY_S((char*)paramVal, valSize, driverVer.c_str());
            }
            return CL_DEV_SUCCESS;
        }

        case (CL_DEVICE_EXTENSIONS):
        {
            *pinternalRetunedValueSize = 1;
            if(nullptr != paramVal && valSize < *pinternalRetunedValueSize)
            {
                return CL_DEV_INVALID_VALUE;
            }
            //if OUT paramVal is NULL it should be ignored
            if(nullptr != paramVal)
            {
                *(char*)paramVal = '\0';
            }
            return CL_DEV_SUCCESS;
        }

        // TODO
        case (CL_DEVICE_BUILT_IN_KERNELS):
        {
            *pinternalRetunedValueSize = 1;
            if(nullptr != paramVal && valSize < *pinternalRetunedValueSize)
            {
                return CL_DEV_INVALID_VALUE;
            }
            //if OUT paramVal is NULL it should be ignored
            if(nullptr != paramVal)
            {
                *(char*)paramVal = '\0';
            }
            return CL_DEV_SUCCESS;
        }

        case (CL_DEVICE_PARTITION_PROPERTIES):
        {
            *pinternalRetunedValueSize = sizeof(cl_device_partition_property);
            if(nullptr != paramVal && valSize < *pinternalRetunedValueSize)
            {
                return CL_DEV_INVALID_VALUE;
            }
            //if OUT paramVal is NULL it should be ignored
            if(nullptr != paramVal)
            {
                *(cl_device_partition_property*)paramVal = (cl_device_partition_property)0;
            }
            return CL_DEV_SUCCESS;
        }
        case (CL_DEVICE_PARTITION_AFFINITY_DOMAIN):
        {
            *pinternalRetunedValueSize = sizeof(cl_device_affinity_domain);
            if(nullptr != paramVal && valSize < *pinternalRetunedValueSize)
            {
                return CL_DEV_INVALID_VALUE;
            }
            //if OUT paramVal is NULL it should be ignored
            if(nullptr != paramVal)
            {
                *((cl_device_affinity_domain*)paramVal) = (cl_device_affinity_domain)0;
            }
            return CL_DEV_SUCCESS;
        }

        // the below queries are considered not supported - i.e ISP cannot answer the query
        // TODO: need to verify that with the team what to do with these queries
        case (CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR):
        case (CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT):
        case (CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT):
        case (CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT):
        case (CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG):
        case (CL_DEVICE_PREFERRED_VECTOR_WIDTH_HALF ):
        case (CL_DEVICE_NATIVE_VECTOR_WIDTH_CHAR ):
        case (CL_DEVICE_NATIVE_VECTOR_WIDTH_SHORT):
        case (CL_DEVICE_NATIVE_VECTOR_WIDTH_INT):
        case (CL_DEVICE_NATIVE_VECTOR_WIDTH_FLOAT):
        case (CL_DEVICE_NATIVE_VECTOR_WIDTH_LONG):
        case (CL_DEVICE_NATIVE_VECTOR_WIDTH_HALF ):
        case (CL_DEVICE_NATIVE_VECTOR_WIDTH_DOUBLE):

        case (CL_DEVICE_MIN_DATA_TYPE_ALIGN_SIZE):

        case (CL_DEVICE_SINGLE_FP_CONFIG):
        case (CL_DEVICE_DOUBLE_FP_CONFIG):

        // TODO: image support
        case (CL_DEVICE_IMAGE_SUPPORT):
        case (CL_DEVICE_IMAGE2D_MAX_WIDTH):
        case (CL_DEVICE_IMAGE2D_MAX_HEIGHT):
        case (CL_DEVICE_IMAGE3D_MAX_WIDTH):
        case (CL_DEVICE_IMAGE3D_MAX_HEIGHT):
        case (CL_DEVICE_IMAGE3D_MAX_DEPTH):
        case (CL_DEVICE_MAX_READ_IMAGE_ARGS):
        case (CL_DEVICE_MAX_WRITE_IMAGE_ARGS):
        case (CL_DEVICE_IMAGE_MAX_ARRAY_SIZE):
        case (CL_DEVICE_IMAGE_MAX_BUFFER_SIZE):

        case (CL_DEVICE_MAX_PARAMETER_SIZE):
        case (CL_DEVICE_MAX_SAMPLERS):

        case (CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE):
        case (CL_DEVICE_MAX_CONSTANT_ARGS):

        case (CL_DEVICE_MEM_BASE_ADDR_ALIGN):

        case (CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE):

        case (CL_DEVICE_IMAGE_PITCH_ALIGNMENT):
        case (CL_DEVICE_IMAGE_BASE_ADDRESS_ALIGNMENT):
        case (CL_DEVICE_PREFERRED_PLATFORM_ATOMIC_ALIGNMENT):
        case (CL_DEVICE_PREFERRED_GLOBAL_ATOMIC_ALIGNMENT):
        case (CL_DEVICE_PREFERRED_LOCAL_ATOMIC_ALIGNMENT):

        case (CL_DEVICE_MAX_CLOCK_FREQUENCY):
        case (CL_DEVICE_ADDRESS_BITS):

        case (CL_DEVICE_PROFILING_TIMER_RESOLUTION):

        case (CL_DEVICE_PRINTF_BUFFER_SIZE):

        case (CL_DEVICE_GLOBAL_MEM_CACHE_TYPE):

        case (CL_PROGRAM_BUILD_GLOBAL_VARIABLE_TOTAL_SIZE):

        case (CL_DEVICE_ENDIAN_LITTLE):

        case (CL_DEVICE_ERROR_CORRECTION_SUPPORT):

        case (CL_DEVICE_QUEUE_ON_HOST_PROPERTIES):

        case (CL_DEVICE_OPENCL_C_VERSION):

        case (CL_DEVICE_SVM_CAPABILITIES):

        case (CL_DEVICE_MAX_PIPE_ARGS):
        case (CL_DEVICE_PIPE_MAX_ACTIVE_RESERVATIONS):
        case (CL_DEVICE_PIPE_MAX_PACKET_SIZE):

        case (CL_DEVICE_QUEUE_ON_DEVICE_PREFERRED_SIZE):
        case (CL_DEVICE_QUEUE_ON_DEVICE_MAX_SIZE):
        case (CL_DEVICE_QUEUE_ON_DEVICE_PROPERTIES):
        case (CL_DEVICE_MAX_ON_DEVICE_QUEUES):
        case (CL_DEVICE_MAX_ON_DEVICE_EVENTS):

        case (CL_DEVICE_MAX_GLOBAL_VARIABLE_SIZE):
        case (CL_DEVICE_GLOBAL_VARIABLE_PREFERRED_TOTAL_SIZE):

            return CL_DEV_NOT_SUPPORTED;

        default:
            return CL_DEV_INVALID_VALUE;
    }
    return CL_DEV_SUCCESS;

}

/************************************************************************************************************************
   clDevGetAvailableDeviceList
    Description
        This function return IDs list for all devices in the same device type.
    Input
        deviceListSize          Specifies the size of memory pointed to by deviceIdsList.(in term of amount of IDs it can store)
                                    If deviceIdsList != NULL then deviceListSize must be greater than 0.
    Output
        deviceIdsList           A pointer to memory location where appropriate values for each device ID will be store. If paramVal is NULL, it is ignored
        deviceIdsListSizeRet    If deviceIdsList != NULL it store the actual amount of IDs being store in deviceIdsList.
                                If deviceIdsList == NULL and deviceIdsListSizeRet than it store the amount of available devices.
                                If deviceIdsListSizeRet is NULL, it is ignored.
    Returns
        CL_DEV_SUCCESS          If functions is executed successfully.
        CL_DEV_ERROR_FAIL       If function failed to figure the IDs of the devices.
**************************************************************************************************************************/
cl_dev_err_code ISPDevice::clDevGetAvailableDeviceList(size_t IN  deviceListSize, unsigned int*   OUT deviceIdsList, size_t*   OUT deviceIdsListSizeRet)
{
    if (((nullptr != deviceIdsList) && (0 == deviceListSize)) || ((nullptr == deviceIdsList) && (nullptr == deviceIdsListSizeRet)))
    {
        return CL_DEV_ERROR_FAIL;
    }
    assert(((deviceListSize > 0) || (nullptr == deviceIdsList)) && "If deviceIdsList != NULL, deviceListSize must be 1 in case of ISP device");

    // TODO: Hard-coded app name...
    // TODO: change CameraShim to ISPCameraService
/*    CameraShim* pCameraShim = CameraShim::instance("com.example.opencldemo", 0);
    //CameraShim* pCameraShim = CameraShim::instance("test_api", 0);
    if (NULL == pCameraShim)
    {
        return CL_DEV_ERROR_FAIL;
    }*/

    if (deviceIdsList)
    {
        deviceIdsList[0] = 0;
    }
    if (deviceIdsListSizeRet)
    {
        *deviceIdsListSizeRet = 1;
    }

    return CL_DEV_SUCCESS;
}


// Device Fission support
/****************************************************************************************************************
 clDevPartition
    Calculate appropriate affinity mask to support the partitioning mode and instantiate as many SubdeviceTaskDispatcher objects as needed
********************************************************************************************************************/
cl_dev_err_code ISPDevice::clDevPartition( cl_dev_partition_prop IN props, cl_uint IN num_requested_subdevices, cl_dev_subdevice_id IN parent_id, cl_uint* INOUT num_subdevices, void* param, cl_dev_subdevice_id* OUT subdevice_ids )
{
    IspInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("clDevPartition Function enter"));

    IspErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("clDevPartition should not have been called on ISP, it's not supported now"));
    return CL_DEV_NOT_SUPPORTED;
}
/****************************************************************************************************************
 clDevReleaseSubdevice
    Release a subdevice created by a clDevPartition call. Releases the appropriate SubdeviceTaskDispatcher object
********************************************************************************************************************/
cl_dev_err_code ISPDevice::clDevReleaseSubdevice( cl_dev_subdevice_id IN subdevice_id )
{
    IspInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("clDevReleaseSubdevice Function enter"));

    IspErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("clDevReleaseSubdevice should not have been called on ISP, it's not supported now"));
    return CL_DEV_NOT_SUPPORTED;
}

// Execution commands
/****************************************************************************************************************
 clDevCreateCommandList
    Call TaskDispatcher to create command queue
********************************************************************************************************************/
// TODO: better change to clDevCreateCommandQueue
cl_dev_err_code ISPDevice::clDevCreateCommandList( cl_dev_cmd_list_props IN props, cl_dev_subdevice_id IN subdevice_id, cl_dev_cmd_list* OUT list )
{
    IspInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("clDevCreateCommandList Function enter"));

    return m_pTaskDispatcher->CreateCommandList(props, subdevice_id, list);
}

/****************************************************************************************************************
 clDevFlushCommandList
    Call TaskDispatcher to flush command list
********************************************************************************************************************/
cl_dev_err_code ISPDevice::clDevFlushCommandList( cl_dev_cmd_list IN list )
{
    IspInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("clDevFlushCommandList Function enter"));

    return m_pTaskDispatcher->FlushCommandList(list);
}

/****************************************************************************************************************
 clDevReleaseCommandList
    Call TaskDispatcher to release command list
********************************************************************************************************************/
cl_dev_err_code ISPDevice::clDevReleaseCommandList( cl_dev_cmd_list IN list )
{
    IspInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("clDevReleaseCommandList Function enter"));

    return m_pTaskDispatcher->ReleaseCommandList(list);
}

/****************************************************************************************************************
 clDevCommandListExecute
    Call TaskDispatcher to execute command list
********************************************************************************************************************/
cl_dev_err_code ISPDevice::clDevCommandListExecute( cl_dev_cmd_list IN list, cl_dev_cmd_desc* IN *cmds, cl_uint IN count )
{
    IspInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("clDevCommandListExecute Function enter"));

    return m_pTaskDispatcher->ExecuteCommandList(list, cmds, count);
}

/****************************************************************************************************************
 clDevCommandListWaitCompletion
    Call TaskDispatcher to wait for command list to complete
********************************************************************************************************************/
cl_dev_err_code ISPDevice::clDevCommandListWaitCompletion( cl_dev_cmd_list IN list, cl_dev_cmd_desc* IN cmdToWait )
{
    IspInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("clDevCommandListExecute Function enter"));

    return m_pTaskDispatcher->WaitForCompletion(list, cmdToWait);
}

/****************************************************************************************************************
 clDevCommandListCancel
    Call TaskDispatcher to cancel execution of a command list
********************************************************************************************************************/
cl_dev_err_code ISPDevice::clDevCommandListCancel( cl_dev_cmd_list IN list )
{
    IspInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("clDevCommandListCancel Function enter"));

    return m_pTaskDispatcher->CancelCommandList(list);
}

/****************************************************************************************************************
 clDevReleaseCommand
    Call TaskDispatcher to release a command
********************************************************************************************************************/
cl_dev_err_code ISPDevice::clDevReleaseCommand(cl_dev_cmd_desc* IN cmdToRelease)
{
    IspInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("clDevReleaseCommand Function enter"));

    return m_pTaskDispatcher->ReleaseCommand(cmdToRelease);
}

/****************************************************************************************************************
 clDevGetSupportedImageFormats
    Call Program Service to get supported image formats
********************************************************************************************************************/
cl_dev_err_code ISPDevice::clDevGetSupportedImageFormats( cl_mem_flags IN flags, cl_mem_object_type IN imageType,
                cl_uint IN numEntries, cl_image_format* OUT formats, cl_uint* OUT numEntriesRet ) const
{
    IspInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("clDevGetSupportedImageFormats Function enter"));

    return m_pProgramService->GetSupportedImageFormats(flags, imageType, numEntries, formats, numEntriesRet);
}

//Memory API's
/****************************************************************************************************************
 clDevGetMemoryAllocProperties
    Call Memory Allocator to get allocation properties
********************************************************************************************************************/
cl_dev_err_code ISPDevice::clDevGetMemoryAllocProperties( cl_mem_object_type IN memObjType, cl_dev_alloc_prop* OUT pAllocProp )
{
    IspInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("clDevGetMemoryAllocProperties Function enter"));

    return m_pMemoryAllocator->GetAllocProperties(memObjType, pAllocProp);
}

/****************************************************************************************************************
 clDevCreateMemoryObject
    Call Memory Allocator to create memory object
********************************************************************************************************************/
cl_dev_err_code ISPDevice::clDevCreateMemoryObject( cl_dev_subdevice_id node_id, cl_mem_flags IN flags, const cl_image_format* IN format,
                                    size_t  IN dim_count, const size_t* IN dim_size, IOCLDevRTMemObjectService* pRTService, IOCLDevMemoryObject* OUT *memObj)
{
    IspInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("clDevGetMemoryAllocProperties Function enter"));

    return m_pMemoryAllocator->CreateMemoryObject(node_id, flags, format, dim_count, dim_size, pRTService, memObj);
}

/****************************************************************************************************************
 clDevCheckProgramBinary
    Call Program Serice to check binaries
********************************************************************************************************************/
cl_dev_err_code ISPDevice::clDevCheckProgramBinary( size_t IN binSize, const void* IN bin )
{
    IspInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("clDevCheckProgramBinary Function enter"));

    return m_pProgramService->CheckProgramBinary(binSize, bin);
}

/*******************************************************************************************************************
clDevCreateProgram
    Call programService to create program
**********************************************************************************************************************/
cl_dev_err_code ISPDevice::clDevCreateProgram( size_t IN binSize, const void* IN bin, cl_dev_binary_prop IN prop, cl_dev_program* OUT prog )
{
    IspInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("clDevCreateProgram Function enter"));

    return m_pProgramService->CreateProgram(binSize, bin, prop, prog);
}

/*******************************************************************************************************************
clDevCreateBuiltInKernelProgram
    Call programService to create program with built-in kernels
**********************************************************************************************************************/
cl_dev_err_code ISPDevice::clDevCreateBuiltInKernelProgram( const char* IN szBuiltInNames, cl_dev_program* OUT prog )
{
    IspInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("clDevCreateBuiltInKernelProgram Function enter"));

    return m_pProgramService->CreateBuiltInKernelProgram(szBuiltInNames, prog);
}

/*******************************************************************************************************************
clDevBuildProgram
    Call programService to build program
**********************************************************************************************************************/
cl_dev_err_code ISPDevice::clDevBuildProgram( cl_dev_program IN prog, const char* IN options, cl_build_status* OUT buildStatus )
{
    IspInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("clDevBuildProgram Function enter"));

    return m_pProgramService->BuildProgram(prog, options, buildStatus);
}

/*******************************************************************************************************************
clDevReleaseProgram
    Call programService to release program
**********************************************************************************************************************/
cl_dev_err_code ISPDevice::clDevReleaseProgram( cl_dev_program IN prog )
{
    IspInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("clDevReleaseProgram Function enter"));

    return m_pProgramService->ReleaseProgram(prog);
}

/*******************************************************************************************************************
clDevUnloadCompiler
    Call programService to unload the backend compiler
**********************************************************************************************************************/
cl_dev_err_code ISPDevice::clDevUnloadCompiler()
{
    IspInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("clDevReleaseProgram Function enter"));

    IspErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("clDevUnloadCompiler should not been called on ISP, it's not supported now"));
    return CL_DEV_NOT_SUPPORTED;
}

/*******************************************************************************************************************
clDevGetProgramBinary
    Call programService to get the program binary
**********************************************************************************************************************/
cl_dev_err_code ISPDevice::clDevGetProgramBinary( cl_dev_program IN prog, size_t IN size, void* OUT binary, size_t* OUT sizeRet )
{
    IspInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("clDevGetProgramBinary Function enter"));

    return m_pProgramService->GetProgramBinary(prog, size, binary, sizeRet);
}

/*******************************************************************************************************************
clDevGetBuildLog
    Call programService to get the build log
**********************************************************************************************************************/
cl_dev_err_code ISPDevice::clDevGetBuildLog( cl_dev_program IN prog, size_t IN size, char* OUT log, size_t* OUT sizeRet)
{
    IspInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("clDevGetBuildLog Function enter"));

    return m_pProgramService->GetBuildLog(prog, size, log, sizeRet);
}

/*******************************************************************************************************************
clDevGetSupportedBinaries
    Call programService to get supported binary description
**********************************************************************************************************************/
cl_dev_err_code ISPDevice::clDevGetSupportedBinaries( size_t IN count, cl_prog_binary_desc* OUT types, size_t* OUT sizeRet )
{
    IspInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("clDevGetSupportedBinaries Function enter"));

    return m_pProgramService->GetSupportedBinaries(count, types, sizeRet);
}

/*******************************************************************************************************************
clDevGetKernelId
    Call programService to get kernel id from its name
**********************************************************************************************************************/
cl_dev_err_code ISPDevice::clDevGetKernelId( cl_dev_program IN prog, const char* IN name, cl_dev_kernel* OUT kernelId )
{
    IspInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("clDevGetKernelId Function enter"));

    return m_pProgramService->GetKernelId(prog, name, kernelId);
}

/*******************************************************************************************************************
clDevGetProgramKernels
    Call programService to get kernels from the program
**********************************************************************************************************************/
cl_dev_err_code ISPDevice::clDevGetProgramKernels( cl_dev_program IN prog, cl_uint IN numKernels, cl_dev_kernel* OUT kernels,
                         cl_uint* OUT numKernelsRet )
{
    IspInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("clDevGetProgramKernels Function enter"));

    return m_pProgramService->GetProgramKernels(prog, numKernels, kernels, numKernelsRet);
}

/*******************************************************************************************************************
clDevGetGlobalVariableTotalSize
    Call programService to get the total size of global variables
**********************************************************************************************************************/
cl_dev_err_code ISPDevice::clDevGetGlobalVariableTotalSize( cl_dev_program IN prog, size_t* OUT size )
{
    IspInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("clDevGetGlobalVariableTotalSize Function enter"));

    // TODO: can we support this interface ?
    return CL_DEV_NOT_SUPPORTED;
}

/*******************************************************************************************************************
clDevGetKernelInfo
    Call programService to get kernel info
**********************************************************************************************************************/
cl_dev_err_code ISPDevice::clDevGetKernelInfo( cl_dev_kernel IN kernel, cl_dev_kernel_info IN param, size_t IN valueSize,
                    void* OUT value, size_t* OUT valueSizeRet )
{
    IspInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("clDevGetKernelInfo Function enter"));

    return m_pProgramService->GetKernelInfo(kernel, param, valueSize, value, valueSizeRet);
}

/*******************************************************************************************************************
clDevGetPerofrmanceCounter
    Get performance counter value
**********************************************************************************************************************/
cl_ulong ISPDevice::clDevGetPerformanceCounter()
{
    IspInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("clDevGetPerformanceCounter Function enter"));

    return Intel::OpenCL::Utils::HostTime();
}

/*******************************************************************************************************************
clDevSetLogger
**********************************************************************************************************************/
cl_dev_err_code ISPDevice::clDevSetLogger(IOCLDevLogDescriptor *pLogDescriptor)
{
     IspInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("clDevSetLogger Function enter"));

    if (nullptr != m_pLogDescriptor)
    {
        m_pLogDescriptor->clLogReleaseClient(m_iLogHandle);
    }
    m_pLogDescriptor = pLogDescriptor;
    if (nullptr != m_pLogDescriptor)
    {
        cl_dev_err_code ret = (cl_dev_err_code)m_pLogDescriptor->clLogCreateClient(m_uiIspId, "ISP Device", &m_iLogHandle);
        if (CL_DEV_FAILED(ret))
        {
            return CL_DEV_ERROR_FAIL;
        }
    }
    return CL_DEV_SUCCESS;
}

/*******************************************************************************************************************
clDevCloseDevice
    Close device
**********************************************************************************************************************/
void ISPDevice::clDevCloseDevice(void)
{
    IspInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("clDevCloseDevice Function enter"));

    if (nullptr != m_pTaskDispatcher)
    {
        delete m_pTaskDispatcher;
        m_pTaskDispatcher = nullptr;
    }
    if (nullptr != m_pMemoryAllocator)
    {
        delete m_pMemoryAllocator;
        m_pMemoryAllocator = nullptr;
    }
    if (nullptr != m_pProgramService)
    {
        delete m_pProgramService;
        m_pProgramService = nullptr;
    }
    if (nullptr != m_pCameraShim)
    {
        delete m_pCameraShim;
        m_pCameraShim = nullptr;
    }

    if (0 != m_iLogHandle)
    {
        m_pLogDescriptor->clLogReleaseClient(m_iLogHandle);
    }

    delete this;
}

/*******************************************************************************************************************
clDevAllocateMemory
    Allocate memory that is accessible from the host
**********************************************************************************************************************/
void* ISPDevice::clDevAllocateRawMemory( size_t IN allocSize, size_t IN alignment )
{
    // CameraShim provides buffer that are page-aligned, so ignore alignment parameter
    return m_pCameraShim->host_alloc(allocSize);
}

/*******************************************************************************************************************
clDevFreeMemory
    Releases a memory that was allocated by clDevAllocateMemory()
**********************************************************************************************************************/
void ISPDevice::clDevFreeRawMemory( void* IN allocatedMemory )
{
    m_pCameraShim->host_free(allocatedMemory);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Exported functions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/************************************************************************************************************************
clDevCreateDeviceInstance
**************************************************************************************************************************/
extern "C" cl_dev_err_code clDevCreateDeviceInstance(cl_uint IN dev_id,
                                   IOCLFrameworkCallbacks*   IN pDevCallBacks,
                                   IOCLDevLogDescriptor*     IN pLogDesc,
                                   IOCLDeviceAgent**        OUT ppDevice,
                                   Intel::OpenCL::Utils::FrameworkUserLogger*      IN pUserLogger)
{
    if (nullptr == ppDevice)
    {
        return CL_DEV_INVALID_OPERATION;
    }

    // TODO: currently pUserLogger is ignored

    ISPDevice *pNewDevice = new ISPDevice(dev_id, pDevCallBacks, pLogDesc);
    if ( nullptr == pNewDevice )
    {
        return CL_DEV_OUT_OF_MEMORY;
    }

    cl_dev_err_code rc = pNewDevice->Init();
    if(CL_DEV_FAILED(rc))
    {
        pNewDevice->clDevCloseDevice();
        return rc;
    }

    *ppDevice = pNewDevice;
    return CL_DEV_SUCCESS;
}

/************************************************************************************************************************
clDevGetDeviceInfo
**************************************************************************************************************************/
extern "C" cl_dev_err_code clDevGetDeviceInfo(unsigned int IN dev_id,
                            cl_device_info  IN  param,
                            size_t          IN  valSize,
                            void*           OUT paramVal,
                            size_t*         OUT paramValSizeRet)
{
    return ISPDevice::clDevGetDeviceInfo(dev_id, param, valSize, paramVal, paramValSizeRet);
}

/************************************************************************************************************************
clDevGetAvailableDeviceList
*************************************************************************************************************************/
extern "C" cl_dev_err_code clDevGetAvailableDeviceList(size_t   IN  deviceListSize,
                        unsigned int*   OUT deviceIdsList,
                        size_t*         OUT deviceIdsListSizeRet)
{
    return ISPDevice::clDevGetAvailableDeviceList(deviceListSize, deviceIdsList, deviceIdsListSizeRet);
}

/************************************************************************************************************************
clDevInitDeviceAgent
    This function initializes device agent internal data. This function should be called prior to any device agent calls.
        retval     CL_DEV_SUCCESS           If function is executed successfully.
        retval     CL_DEV_ERROR_FAIL        If function failed to figure the IDs of the devices.
*************************************************************************************************************************/
extern "C" cl_dev_err_code clDevInitDeviceAgent(void)
{
    return CL_DEV_SUCCESS;
}
