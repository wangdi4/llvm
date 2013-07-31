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

#include <cl_sys_defines.h>


// TODO: remove this include
#include <iostream>

using namespace Intel::OpenCL::ISPDevice;

char clISPDEVICE_CFG_PATH[MAX_PATH];

ISPDevice::ISPDevice()
{
}

ISPDevice::~ISPDevice()
{
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
cl_dev_err_code ISPDevice::clDevGetDeviceInfo(unsigned int IN dev_id, cl_device_info IN param, size_t IN valSize, void* OUT paramVal,
                size_t* OUT paramValSizeRet)
{
    size_t  internalRetunedValueSize = valSize;
    size_t  *pinternalRetunedValueSize;

    //if OUT paramValSize_ret is NULL it should be ignopred
    if(paramValSizeRet)
    {
        pinternalRetunedValueSize = paramValSizeRet;
    }
    else
    {
        pinternalRetunedValueSize = &internalRetunedValueSize;
    }

    // TODO: Not implemented yet
    switch (param)
    {
        case (CL_DEVICE_TYPE):
        {
            *pinternalRetunedValueSize = sizeof(cl_device_type);
            if(NULL != paramVal && valSize < *pinternalRetunedValueSize)
            {
                return CL_DEV_INVALID_VALUE;
            }
            //if OUT paramVal is NULL it should be ignored
            if(NULL != paramVal)
            {
                *(cl_device_type*)paramVal = (cl_device_type)CL_DEVICE_TYPE_CUSTOM;
            }
            return CL_DEV_SUCCESS;
        }

        case (CL_DEVICE_VENDOR_ID):
        case (CL_DEVICE_PARTITION_MAX_SUB_DEVICES):
        case (CL_DEVICE_MAX_COMPUTE_UNITS):
        case (CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR):
        case (CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT):
        case (CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT):
        case (CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT):
        case (CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG):
        case (CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE):
        case (CL_DEVICE_PREFERRED_VECTOR_WIDTH_HALF ):
        case (CL_DEVICE_NATIVE_VECTOR_WIDTH_CHAR ):
        case (CL_DEVICE_NATIVE_VECTOR_WIDTH_SHORT):
        case (CL_DEVICE_NATIVE_VECTOR_WIDTH_INT):
        case (CL_DEVICE_NATIVE_VECTOR_WIDTH_FLOAT):
        case (CL_DEVICE_NATIVE_VECTOR_WIDTH_LONG):
        case (CL_DEVICE_NATIVE_VECTOR_WIDTH_HALF ):
        case (CL_DEVICE_NATIVE_VECTOR_WIDTH_DOUBLE):
        case (CL_DEVICE_IMAGE_SUPPORT):
        case (CL_DEVICE_MIN_DATA_TYPE_ALIGN_SIZE):
        case (CL_DEVICE_SINGLE_FP_CONFIG):
        case (CL_DEVICE_DOUBLE_FP_CONFIG):
        case (CL_DEVICE_IMAGE2D_MAX_WIDTH):
        case (CL_DEVICE_IMAGE2D_MAX_HEIGHT):
        case (CL_DEVICE_IMAGE3D_MAX_WIDTH):
        case (CL_DEVICE_IMAGE3D_MAX_HEIGHT):
        case (CL_DEVICE_IMAGE3D_MAX_DEPTH):
        case (CL_DEVICE_MAX_PARAMETER_SIZE):
        case (CL_DEVICE_MAX_SAMPLERS):
        case (CL_DEVICE_MAX_READ_IMAGE_ARGS):
        case (CL_DEVICE_MAX_WRITE_IMAGE_ARGS):
        case (CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE):
        case (CL_DEVICE_MAX_CONSTANT_ARGS ):
        case (CL_DEVICE_MEM_BASE_ADDR_ALIGN):
        case (CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS):
        case (CL_DEVICE_MAX_WORK_GROUP_SIZE):
        case (CL_DEVICE_MAX_WORK_ITEM_SIZES):
        case (CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE):
        case (CL_DEVICE_IMAGE_PITCH_ALIGNMENT):
        case (CL_DEVICE_IMAGE_BASE_ADDRESS_ALIGNMENT):
        case (CL_DEVICE_GLOBAL_MEM_CACHE_SIZE):
            return CL_DEV_NOT_SUPPORTED;

        case (CL_DEVICE_LOCAL_MEM_SIZE):
        {
            *pinternalRetunedValueSize = sizeof(cl_ulong);
            if(NULL != paramVal && valSize < *pinternalRetunedValueSize)
            {
                return CL_DEV_INVALID_VALUE;
            }
            //if OUT paramVal is NULL it should be ignored
            if(NULL != paramVal)
            {
                // TODO: change this. it was for debugging
                *(cl_ulong*)paramVal = (32*1024);
            }
            return CL_DEV_SUCCESS;
        }

        case (CL_DEVICE_MAX_CLOCK_FREQUENCY):
        case (CL_DEVICE_ADDRESS_BITS):
        case (CL_DEVICE_PROFILING_TIMER_RESOLUTION):
        case (CL_DEVICE_PRINTF_BUFFER_SIZE):
        case (CL_DEVICE_GLOBAL_MEM_CACHE_TYPE):
        case (CL_DEVICE_MAX_MEM_ALLOC_SIZE):
        case (CL_DEVICE_GLOBAL_MEM_SIZE):
        case (CL_DEVICE_ENDIAN_LITTLE):
        case (CL_DEVICE_ERROR_CORRECTION_SUPPORT):
        case (CL_DEVICE_LOCAL_MEM_TYPE):
        case (CL_DEVICE_AVAILABLE):
            return CL_DEV_NOT_SUPPORTED;

        case (CL_DEVICE_EXECUTION_CAPABILITIES):
        {
            *pinternalRetunedValueSize = sizeof(cl_device_exec_capabilities);
            if(NULL != paramVal && valSize < *pinternalRetunedValueSize)
            {
                return CL_DEV_INVALID_VALUE;
            }
            //if OUT paramVal is NULL it should be ignored
            if(NULL != paramVal)
            {
                // TODO: change this. it was for debugging
                cl_device_exec_capabilities execCapabilities = CL_EXEC_KERNEL;
                *(cl_device_exec_capabilities*)paramVal = execCapabilities;
            }
            return CL_DEV_SUCCESS;
        }

        case (CL_DEVICE_QUEUE_PROPERTIES ):
        case (CL_DEVICE_HOST_UNIFIED_MEMORY):
        case (CL_DEVICE_NAME):
        case (CL_DEVICE_VENDOR):
        case (CL_DEVICE_PROFILE):
        case (CL_DEVICE_OPENCL_C_VERSION):
        case (CL_DEVICE_VERSION):
        case (CL_DRIVER_VERSION ):
            return CL_DEV_NOT_SUPPORTED;

        case (CL_DEVICE_EXTENSIONS):
        {
            *pinternalRetunedValueSize = 1;
            if(NULL != paramVal && valSize < *pinternalRetunedValueSize)
            {
                return CL_DEV_INVALID_VALUE;
            }
            //if OUT paramVal is NULL it should be ignored
            if(NULL != paramVal)
            {
                // TODO: change this. it was for debugging
                *(char*)paramVal = '\0';
            }
            return CL_DEV_SUCCESS;
        }

        case (CL_DEVICE_BUILT_IN_KERNELS):
        case (CL_DEVICE_PARTITION_PROPERTIES):
        case (CL_DEVICE_PARTITION_AFFINITY_DOMAIN):
        case (CL_DEVICE_IMAGE_MAX_ARRAY_SIZE):
        case (CL_DEVICE_IMAGE_MAX_BUFFER_SIZE):
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
    // TODO: Not implemented yet
    if (((NULL != deviceIdsList) && (0 == deviceListSize)) || ((NULL == deviceIdsList) && (NULL == deviceIdsListSizeRet)))
    {
        return CL_DEV_ERROR_FAIL;
    }
    assert(((deviceListSize > 0) || (NULL == deviceIdsList)) && "If deviceIdsList != NULL, deviceListSize must be 1 in case of ISP device");
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
    // TODO: Not implemented yet
    std::cout << "ISP clDevPartition is called." << std::endl;
    return CL_DEV_NOT_SUPPORTED;
}

/****************************************************************************************************************
 clDevReleaseSubdevice
    Release a subdevice created by a clDevPartition call. Releases the appropriate SubdeviceTaskDispatcher object
********************************************************************************************************************/
cl_dev_err_code ISPDevice::clDevReleaseSubdevice( cl_dev_subdevice_id IN subdevice_id )
{
    // TODO: Not implemented yet
    std::cout << "ISP clDevReleaseSubdevice is called." << std::endl;
    return CL_DEV_NOT_SUPPORTED;
}

// Execution commands
/****************************************************************************************************************
 clDevCreateCommandList
    Call TaskDispatcher to create command list
********************************************************************************************************************/
cl_dev_err_code ISPDevice::clDevCreateCommandList( cl_dev_cmd_list_props IN props, cl_dev_subdevice_id IN subdevice_id, cl_dev_cmd_list* OUT list )
{
    // TODO: Not implemented yet
    std::cout << "ISP clDevCreateCommandList is called." << std::endl;
    return CL_DEV_NOT_SUPPORTED;
}

/****************************************************************************************************************
 clDevFlushCommandList
    Call TaskDispatcher to flush command list
********************************************************************************************************************/
cl_dev_err_code ISPDevice::clDevFlushCommandList( cl_dev_cmd_list IN list )
{
    // TODO: Not implemented yet
    std::cout << "ISP clDevFlushCommandList is called." << std::endl;
    return CL_DEV_NOT_SUPPORTED;
}

/****************************************************************************************************************
 clDevReleaseCommandList
    Call TaskDispatcher to release command list
********************************************************************************************************************/
cl_dev_err_code ISPDevice::clDevReleaseCommandList( cl_dev_cmd_list IN list )
{
    // TODO: Not implemented yet
    std::cout << "ISP clDevReleaseCommandList is called." << std::endl;
    return CL_DEV_NOT_SUPPORTED;
}

/****************************************************************************************************************
 clDevCommandListExecute
    Call TaskDispatcher to execute command list
********************************************************************************************************************/
cl_dev_err_code ISPDevice::clDevCommandListExecute( cl_dev_cmd_list IN list, cl_dev_cmd_desc* IN *cmds, cl_uint IN count)
{
    // TODO: Not implemented yet
    std::cout << "ISP clDevCommandListExecute is called." << std::endl;
    return CL_DEV_NOT_SUPPORTED;
}

/****************************************************************************************************************
 clDevCommandListWaitCompletion
    Call clDevCommandListWaitCompletion to add calling thread to execution pool
********************************************************************************************************************/
cl_dev_err_code ISPDevice::clDevCommandListWaitCompletion(cl_dev_cmd_list IN list, cl_dev_cmd_desc* IN cmdToWait)
{
    // TODO: Not implemented yet
    std::cout << "ISP clDevCommandListWaitCompletion is called." << std::endl;
    return CL_DEV_NOT_SUPPORTED;
}

/****************************************************************************************************************
 clDevReleaseCommand
********************************************************************************************************************/
void ISPDevice::clDevReleaseCommand(cl_dev_cmd_desc* IN cmdToRelease)
{
    // TODO: Not implemented yet
    std::cout << "ISP clDevReleaseCommand is called." << std::endl;
    // TODO: should clDevReleaseCommand return error code ?
    //return CL_DEV_NOT_SUPPORTED;
}

//Memory API's
/****************************************************************************************************************
 clDevGetSupportedImageFormats
    Call Memory Allocator to get supported image formats
********************************************************************************************************************/
cl_dev_err_code ISPDevice::clDevGetSupportedImageFormats( cl_mem_flags IN flags, cl_mem_object_type IN imageType,
                cl_uint IN numEntries, cl_image_format* OUT formats, cl_uint* OUT numEntriesRet) const
{
    // TODO: Not implemented yet
    std::cout << "ISP clDevGetSupportedImageFormats is called." << std::endl;
    return CL_DEV_NOT_SUPPORTED;
}

/****************************************************************************************************************
 clDevGetMemoryAllocProperties
    Call Memory Allocator to get allocation properties
********************************************************************************************************************/
cl_dev_err_code ISPDevice::clDevGetMemoryAllocProperties( cl_mem_object_type IN memObjType, cl_dev_alloc_prop* OUT pAllocProp )
{
    // TODO: Not implemented yet
    std::cout << "ISP clDevGetMemoryAllocProperties is called." << std::endl;
    return CL_DEV_NOT_SUPPORTED;
}

/****************************************************************************************************************
 clDevCreateMemoryObject
    Call Memory Allocator to create memory object
********************************************************************************************************************/
cl_dev_err_code ISPDevice::clDevCreateMemoryObject( cl_dev_subdevice_id node_id, cl_mem_flags IN flags, const cl_image_format* IN format,
                                    size_t  IN dim_count, const size_t* IN dim_size, IOCLDevRTMemObjectService* pRTService, IOCLDevMemoryObject* OUT *memObj)
{
    // TODO: Not implemented yet
    std::cout << "ISP clDevCreateMemoryObject is called." << std::endl;
    return CL_DEV_NOT_SUPPORTED;
}

/****************************************************************************************************************
 clDevCheckProgramBinary
    Call Program Serice to check binaries
********************************************************************************************************************/
cl_dev_err_code ISPDevice::clDevCheckProgramBinary( size_t IN binSize, const void* IN bin )
{
    // TODO: Not implemented yet
    std::cout << "ISP clDevCheckProgramBinary is called." << std::endl;
    return CL_DEV_NOT_SUPPORTED;
}

/*******************************************************************************************************************
clDevCreateProgram
    Call programService to create program
**********************************************************************************************************************/
cl_dev_err_code ISPDevice::clDevCreateProgram( size_t IN binSize, const void* IN bin, cl_dev_binary_prop IN prop, cl_dev_program* OUT prog )
{
    // TODO: Not implemented yet
    std::cout << "ISP clDevCreateProgram is called." << std::endl;
    return CL_DEV_NOT_SUPPORTED;
}

/*******************************************************************************************************************
clDevCreateBuiltInKernelProgram
    Call programService to create program with build-in kernels
**********************************************************************************************************************/
cl_dev_err_code ISPDevice::clDevCreateBuiltInKernelProgram( const char* IN szBuiltInNames, cl_dev_program* OUT prog )
{
    // TODO: Not implemented yet
    std::cout << "ISP clDevCreateBuiltInKernelProgram is called." << std::endl;
    return CL_DEV_NOT_SUPPORTED;
}

/*******************************************************************************************************************
clDevBuildProgram
    Call programService to build program
**********************************************************************************************************************/
cl_dev_err_code ISPDevice::clDevBuildProgram( cl_dev_program IN prog, const char* IN options, cl_build_status* OUT buildStatus )
{
    // TODO: Not implemented yet
    std::cout << "ISP clDevBuildProgram is called." << std::endl;
    return CL_DEV_NOT_SUPPORTED;
}

/*******************************************************************************************************************
clDevReleaseProgram
    Call programService to release program
**********************************************************************************************************************/
cl_dev_err_code ISPDevice::clDevReleaseProgram( cl_dev_program IN prog )
{
    // TODO: Not implemented yet
    std::cout << "ISP clDevReleaseProgram is called." << std::endl;
    return CL_DEV_NOT_SUPPORTED;
}

/*******************************************************************************************************************
clDevUnloadCompiler
    Call programService to unload the backend compiler
**********************************************************************************************************************/
cl_dev_err_code ISPDevice::clDevUnloadCompiler()
{
    // TODO: Not implemented yet
    std::cout << "ISP clDevUnloadCompiler is called." << std::endl;
    return CL_DEV_NOT_SUPPORTED;
}

/*******************************************************************************************************************
clDevGetProgramBinary
    Call programService to get the program binary
**********************************************************************************************************************/
cl_dev_err_code ISPDevice::clDevGetProgramBinary( cl_dev_program IN prog, size_t IN size, void* OUT binary, size_t* OUT sizeRet )
{
    // TODO: Not implemented yet
    std::cout << "ISP clDevGetProgramBinary is called." << std::endl;
    return CL_DEV_NOT_SUPPORTED;
}

/*******************************************************************************************************************
clDevGetBuildLog
    Call programService to get the build log
**********************************************************************************************************************/
cl_dev_err_code ISPDevice::clDevGetBuildLog( cl_dev_program IN prog, size_t IN size, char* OUT log, size_t* OUT sizeRet)
{
    // TODO: Not implemented yet
    std::cout << "ISP clDevGetBuildLog is called." << std::endl;
    return CL_DEV_NOT_SUPPORTED;
}

/*******************************************************************************************************************
clDevGetSupportedBinaries
    Call programService to get supported binary description
**********************************************************************************************************************/
cl_dev_err_code ISPDevice::clDevGetSupportedBinaries( size_t IN count, cl_prog_binary_desc* OUT types, size_t* OUT sizeRet )
{
    // TODO: Not implemented yet
    std::cout << "ISP clDevGetSupportedBinaries is called." << std::endl;
    return CL_DEV_NOT_SUPPORTED;
}

/*******************************************************************************************************************
clDevGetKernelId
    Call programService to get kernel id from its name
**********************************************************************************************************************/
cl_dev_err_code ISPDevice::clDevGetKernelId( cl_dev_program IN prog, const char* IN name, cl_dev_kernel* OUT kernelId )
{
    // TODO: Not implemented yet
    std::cout << "ISP clDevGetKernelId is called." << std::endl;
    return CL_DEV_NOT_SUPPORTED;
}

/*******************************************************************************************************************
clDevGetProgramKernels
    Call programService to get kernels from the program
**********************************************************************************************************************/
cl_dev_err_code ISPDevice::clDevGetProgramKernels( cl_dev_program IN prog, cl_uint IN numKernels, cl_dev_kernel* OUT kernels,
                         cl_uint* OUT numKernelsRet )
{
    // TODO: Not implemented yet
    std::cout << "ISP clDevGetProgramKernels is called." << std::endl;
    return CL_DEV_NOT_SUPPORTED;
}

/*******************************************************************************************************************
clDevGetKernelInfo
    Call programService to get kernel info
**********************************************************************************************************************/
cl_dev_err_code ISPDevice::clDevGetKernelInfo( cl_dev_kernel IN kernel, cl_dev_kernel_info IN param, size_t IN valueSize,
                    void* OUT value, size_t* OUT valueSizeRet )
{
    // TODO: Not implemented yet
    std::cout << "ISP clDevGetKernelInfo is called." << std::endl;
    return CL_DEV_NOT_SUPPORTED;
}

/*******************************************************************************************************************
clDevGetPerofrmanceCounter
    Get performance counter value
**********************************************************************************************************************/
cl_ulong ISPDevice::clDevGetPerformanceCounter()
{
    // TODO: Not implemented yet
    std::cout << "ISP clDevGetPerformanceCounter is called." << std::endl;
    return CL_DEV_NOT_SUPPORTED;
}

/*******************************************************************************************************************
clDevSetLogger
**********************************************************************************************************************/
cl_dev_err_code ISPDevice::clDevSetLogger(IOCLDevLogDescriptor *pLogDescriptor)
{
    // TODO: Not implemented yet
    std::cout << "ISP clDevSetLogger is called." << std::endl;
    return CL_DEV_NOT_SUPPORTED;
}

/*******************************************************************************************************************
clDevCloseDevice
    Close device
**********************************************************************************************************************/
void ISPDevice::clDevCloseDevice(void)
{
    // TODO: Not implemented yet
    std::cout << "ISP clDevSetLogger is called." << std::endl;
    //return CL_DEV_NOT_SUPPORTED;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Exported functions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/************************************************************************************************************************
clDevCreateDeviceInstance
**************************************************************************************************************************/
extern "C" cl_dev_err_code clDevCreateDeviceInstance(  cl_uint      dev_id,
                                   IOCLFrameworkCallbacks   *pDevCallBacks,
                                   IOCLDevLogDescriptor     *pLogDesc,
                                   IOCLDeviceAgent*         *pDevice
                                   )
{
    if (NULL == pDevice)
    {
        return CL_DEV_INVALID_OPERATION;
    }
    // TODO: Not implemented yet
    ISPDevice *pNewDevice = new ISPDevice();
    if ( NULL == pNewDevice )
    {
        return CL_DEV_OUT_OF_MEMORY;
    }
    *pDevice = pNewDevice;
    return CL_DEV_SUCCESS;
}

/************************************************************************************************************************
clDevGetDeviceInfo
**************************************************************************************************************************/
extern "C" cl_dev_err_code clDevGetDeviceInfo(unsigned int IN dev_id,
                            cl_device_info  param, 
                            size_t          valSize, 
                            void*           paramVal,
                            size_t*         paramValSizeRet)
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
clDevErr2Txt
*************************************************************************************************************************/
extern "C" const char* clDevErr2Txt(cl_dev_err_code errorCode)
{
    // TODO: Not implemented yet
    switch(errorCode)
    {
        case (CL_DEV_ERROR_FAIL): return "CL_DEV_ERROR_FAIL";
        case (CL_DEV_INVALID_VALUE): return "CL_DEV_INVALID_VALUE";
        case (CL_DEV_INVALID_PROPERTIES): return "CL_DEV_INVALID_PROPERTIES";
        case (CL_DEV_OUT_OF_MEMORY): return "CL_DEV_OUT_OF_MEMORY";
        case (CL_DEV_INVALID_COMMAND_LIST): return "CL_DEV_INVALID_COMMAND_LIST";
        case (CL_DEV_INVALID_COMMAND_TYPE): return "CL_DEV_INVALID_COMMAND_TYPE";
        case (CL_DEV_INVALID_MEM_OBJECT): return "CL_DEV_INVALID_MEM_OBJECT";
        case (CL_DEV_INVALID_KERNEL): return "CL_DEV_INVALID_KERNEL";
        case (CL_DEV_INVALID_OPERATION): return "CL_DEV_INVALID_OPERATION";
        case (CL_DEV_INVALID_WRK_DIM): return "CL_DEV_INVALID_WRK_DIM";
        case (CL_DEV_INVALID_WG_SIZE): return "CL_DEV_INVALID_WG_SIZE";
        case (CL_DEV_INVALID_GLB_OFFSET): return "CL_DEV_INVALID_GLB_OFFSET";
        case (CL_DEV_INVALID_WRK_ITEM_SIZE): return "CL_DEV_INVALID_WRK_ITEM_SIZE";
        case (CL_DEV_INVALID_IMG_FORMAT): return "CL_DEV_INVALID_IMG_FORMAT";
        case (CL_DEV_INVALID_IMG_SIZE): return "CL_DEV_INVALID_IMG_SIZE";
        case (CL_DEV_OBJECT_ALLOC_FAIL): return "CL_DEV_INVALID_COMMAND_LIST";
        case (CL_DEV_INVALID_BINARY): return "CL_DEV_INVALID_BINARY";
        case (CL_DEV_INVALID_BUILD_OPTIONS): return "CL_DEV_INVALID_BUILD_OPTIONS";
        case (CL_DEV_INVALID_PROGRAM): return "CL_DEV_INVALID_PROGRAM";
        case (CL_DEV_BUILD_IN_PROGRESS): return "CL_DEV_BUILD_IN_PROGRESS";
        case (CL_DEV_INVALID_KERNEL_NAME): return "CL_DEV_INVALID_KERNEL_NAME";

        default: return "Unknown Error Code";
    }
}

/************************************************************************************************************************
clDevInitDeviceAgent
    This function initializes device agent internal data. This function should be called prior to any device agent calls.
        retval     CL_DEV_SUCCESS          If function is executed successfully.
        retval     CL_DEV_ERROR_FAIL	    If function failed to figure the IDs of the devices.
*************************************************************************************************************************/
extern "C" cl_dev_err_code clDevInitDeviceAgent(void)
{
    // TODO: Not implemented yet
    return CL_DEV_SUCCESS;
}
