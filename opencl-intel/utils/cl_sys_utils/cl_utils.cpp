/////////////////////////////////////////////////////////////////////////
// cl_utils.cpp:
/////////////////////////////////////////////////////////////////////////
// INTEL CONFIDENTIAL
// Copyright 2007-2008 Intel Corporation All Rights Reserved.
//
// The source code contained or described herein and all documents related
// to the source code ("Material") are owned by Intel Corporation or its
// suppliers or licensors. Title to the Material remains with Intel Corporation
// or its suppliers and licensors. The Material may contain trade secrets and
// proprietary and confidential information of Intel Corporation and its
// suppliers and licensors, and is protected by worldwide copyright and trade
// secret laws and treaty provisions. No part of the Material may be used, copied,
// reproduced, modified, published, uploaded, posted, transmitted, distributed,
// or disclosed in any way without Intel�s prior express written permission.
//
// No license under any patent, copyright, trade secret or other intellectual
// property right is granted to or conferred upon you by disclosure or delivery
// of the Materials, either expressly, by implication, inducement, estoppel or
// otherwise. Any license under such intellectual property rights must be express
// and approved by Intel in writing.
//
// Unless otherwise agreed by Intel in writing, you may not remove or alter this notice
// or any other notice embedded in Materials by Intel or Intel�s suppliers or licensors
// in any way.
/////////////////////////////////////////////////////////////////////////
#ifdef WIN32
#define CL_DX9_MEDIA_SHARING_INTEL_EXT
#endif /*WIN32*/

#include "cl_utils.h"
#include "cl_sys_info.h"
#include <CL/cl.h>
#include <CL/cl_ext.h>
#include <CL/cl_gl_ext.h>
#include <cassert>
#include <stdint.h>
#include <sys/stat.h>

using namespace std;
using namespace Intel::OpenCL;

#ifdef WIN32
#include <windows.h>
#include <intrin.h>
#include <CL/cl_d3d11.h>
#include <CL/cl_d3d10.h>
#include <CL/cl_dx9_media_sharing.h>

bool clIsNumaAvailable()
{
    return false;
}

void clNUMASetLocalNodeAlloc()
{
}

void clSleep(int milliseconds)
{
    SleepEx(milliseconds, TRUE);
}

void clSetThreadAffinityMask(affinityMask_t* mask, threadid_t tid)
{
    assert(*mask <= (DWORD_PTR)-1);
    if (0 == tid)
    {        
        SetThreadAffinityMask(GetCurrentThread(), (DWORD_PTR)*mask);
    }
    else
    {
        HANDLE tid_handle = OpenThread(THREAD_ALL_ACCESS, FALSE, tid);
        SetThreadAffinityMask(tid_handle, (DWORD_PTR)*mask);
        CloseHandle(tid_handle);
    }
}

void clGetThreadAffinityMask(affinityMask_t* mask, threadid_t tid)
{
    // Currently not supported on Windows
    *mask = -1;
}

void clSetThreadAffinityToCore(unsigned int core, threadid_t tid)
{
    DWORD_PTR mask = 1 << core;
    if (0 == tid)
    {
        SetThreadAffinityMask(GetCurrentThread(), mask);
    }
    else
    {
        HANDLE tid_handle = OpenThread(THREAD_ALL_ACCESS, FALSE, tid);
        SetThreadAffinityMask(tid_handle, mask);
        CloseHandle(tid_handle);
    }
    //printf("Thread %d is running on processor %d (expected %d)\n", GetCurrentThreadId(), GetCurrentProcessorNumber(), core);
}

void clResetThreadAffinityMask(threadid_t tid)
{
    static const DWORD_PTR allMask = (DWORD_PTR)-1;
    if (0 == tid)
    {
        SetThreadAffinityMask(GetCurrentThread(), allMask);
    }
    else
    {
        HANDLE tid_handle = OpenThread(THREAD_ALL_ACCESS, FALSE, tid);
        SetThreadAffinityMask(tid_handle, allMask);
        CloseHandle(tid_handle);
    }
}

bool clTranslateAffinityMask(affinityMask_t* mask, unsigned int* IDs, size_t len)
{
    assert(mask);
    assert(IDs);
    assert(*mask <= (DWORD_PTR)-1);
    DWORD_PTR localMask = (DWORD_PTR)*mask;
    size_t i = 0;
    size_t set_bits = 0;
    while ((0 != localMask) && (set_bits < len))
    {
        if (localMask & 0x1)
        {
            IDs[set_bits++] = i;
        }
        ++i;
        localMask >>= 1;
    }
    return (len == set_bits) && (0 == localMask);
}

threadid_t clMyThreadId()
{
    return GetCurrentThreadId();
}

threadid_t clMyParentThreadId()
{
    //No such notion on Windows
    return clMyThreadId();
}

#else
#include <unistd.h>
#include <sys/syscall.h>

#ifndef DISABLE_NUMA_SUPPORT
#define DISABLE_NUMA_SUPPORT
#endif

#ifndef DISABLE_NUMA_SUPPORT

    #include <numa.h>
    bool clIsNumaAvailable()
    {    
        static int iNuma = -1;
    
        if ( -1 == iNuma )
        {
            iNuma = numa_available();
        }
        return (-1 != iNuma);
    }

    void clNUMASetLocalNodeAlloc()
    {
        numa_set_localalloc();
    }
#else
    bool clIsNumaAvailable()
    {    
        return false;
    }

    void clNUMASetLocalNodeAlloc()
    {
    }
    
#endif //DISABLE_NUMA_SUPPORT

void clSleep(int milliseconds)
{
    usleep(1000 * milliseconds);
}

void clSetThreadAffinityMask(affinityMask_t* mask, threadid_t tid)
{
    sched_setaffinity(tid, sizeof(affinityMask_t), mask);
}

void clGetThreadAffinityMask(affinityMask_t* mask, threadid_t tid)
{
    sched_getaffinity(tid, sizeof(affinityMask_t), mask);
}

void clSetThreadAffinityToCore(unsigned int core, threadid_t tid)
{
    affinityMask_t mask;
    CPU_ZERO(&mask);
    CPU_SET(core ,&mask);
    sched_setaffinity(tid, sizeof(affinityMask_t), &mask);
}
void clResetThreadAffinityMask(threadid_t tid)
{
    affinityMask_t mask;
    CPU_ZERO(&mask);
    for (size_t i = 0; i < 4 * sizeof(unsigned long long) * 8; i++)    // we assume no more than 256 HW threads (writing generic code is too much effort)
    {
        CPU_SET(i, &mask);
    }
    sched_setaffinity(tid, sizeof(affinityMask_t), &mask);
}
bool clTranslateAffinityMask(affinityMask_t* mask, unsigned int* IDs, size_t len)
{
    assert(mask);
    assert(IDs);
    size_t set_bits = 0;
    //Todo: assumes no more than 1024 HW threads
    for (size_t i = 0; i < 1024; ++i)
    {
        if (CPU_ISSET(i, mask))
        {
            IDs[set_bits++] = i;
        }
        if (set_bits >= len)
        {
            break;
        }
    }
    return (len == set_bits);
}

threadid_t clMyThreadId()
{
    static __thread threadid_t myThreadId = (threadid_t)-1;
    if (-1 == myThreadId)
    {
          myThreadId = GET_CURRENT_THREAD_ID();
    }
    return myThreadId;
}

threadid_t clMyParentThreadId()
{
    //Not an expensive call, no need to cache the return value
    return getpid();
}

#endif

const char* ClErrTxt(cl_err_code error_code)
{
    switch(error_code)
    {
    // OpenCL error codes
    case (CL_SUCCESS): return "CL_SUCCESS";
    case (CL_DEVICE_NOT_FOUND): return "CL_DEVICE_NOT_FOUND";
    case (CL_DEVICE_NOT_AVAILABLE): return "CL_DEVICE_NOT_AVAILABLE";
    case (CL_COMPILER_NOT_AVAILABLE): return "CL_COMPILER_NOT_AVAILABLE";
    case (CL_MEM_OBJECT_ALLOCATION_FAILURE): return "CL_MEM_OBJECT_ALLOCATION_FAILURE";
    case (CL_OUT_OF_RESOURCES): return "CL_OUT_OF_RESOURCES";
    case (CL_OUT_OF_HOST_MEMORY): return "CL_OUT_OF_HOST_MEMORY";
    case (CL_PROFILING_INFO_NOT_AVAILABLE): return "CL_PROFILING_INFO_NOT_AVAILABLE";
    case (CL_MEM_COPY_OVERLAP): return "CL_MEM_COPY_OVERLAP";
    case (CL_IMAGE_FORMAT_MISMATCH): return "CL_IMAGE_FORMAT_MISMATCH";
    case (CL_IMAGE_FORMAT_NOT_SUPPORTED): return "CL_IMAGE_FORMAT_NOT_SUPPORTED";
    case (CL_BUILD_PROGRAM_FAILURE): return "CL_BUILD_PROGRAM_FAILURE";
    case (CL_MAP_FAILURE): return "CL_MAP_FAILURE";
    case (CL_MISALIGNED_SUB_BUFFER_OFFSET): return "CL_MISALIGNED_SUB_BUFFER_OFFSET";
    case (CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST): return "CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST";
    case (CL_COMPILE_PROGRAM_FAILURE): return "CL_COMPILE_PROGRAM_FAILURE";
    case (CL_LINKER_NOT_AVAILABLE): return "CL_LINKER_NOT_AVAILABLE";
    case (CL_LINK_PROGRAM_FAILURE): return "CL_LINK_PROGRAM_FAILURE";
    case (CL_DEVICE_PARTITION_FAILED): return "CL_DEVICE_PARTITION_FAILED";
    case (CL_KERNEL_ARG_INFO_NOT_AVAILABLE): return "CL_KERNEL_ARG_INFO_NOT_AVAILABLE";
    case (CL_INVALID_VALUE): return "CL_INVALID_VALUE";
    case (CL_INVALID_DEVICE_TYPE): return "CL_INVALID_DEVICE_TYPE";
    case (CL_INVALID_PLATFORM): return "CL_INVALID_PLATFORM";
    case (CL_INVALID_DEVICE): return "CL_INVALID_DEVICE";
    case (CL_INVALID_CONTEXT): return "CL_INVALID_CONTEXT";
    case (CL_INVALID_QUEUE_PROPERTIES): return "CL_INVALID_QUEUE_PROPERTIES";
    case (CL_INVALID_COMMAND_QUEUE): return "CL_INVALID_COMMAND_QUEUE";
    case (CL_INVALID_HOST_PTR): return "CL_INVALID_HOST_PTR";
    case (CL_INVALID_MEM_OBJECT): return "CL_INVALID_MEM_OBJECT";
    case (CL_INVALID_IMAGE_FORMAT_DESCRIPTOR): return "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR";
    case (CL_INVALID_IMAGE_SIZE): return "CL_INVALID_IMAGE_SIZE";
    case (CL_INVALID_SAMPLER): return "CL_INVALID_SAMPLER";
    case (CL_INVALID_BINARY): return "CL_INVALID_BINARY";
    case (CL_INVALID_BUILD_OPTIONS): return "CL_INVALID_BUILD_OPTIONS";
    case (CL_INVALID_PROGRAM): return "CL_INVALID_PROGRAM";
    case (CL_INVALID_PROGRAM_EXECUTABLE): return "CL_INVALID_PROGRAM_EXECUTABLE";
    case (CL_INVALID_KERNEL_NAME): return "CL_INVALID_KERNEL_NAME";
    case (CL_INVALID_KERNEL_DEFINITION): return "CL_INVALID_KERNEL_DEFINITION";
    case (CL_INVALID_KERNEL): return "CL_INVALID_KERNE";
    case (CL_INVALID_ARG_INDEX): return "CL_INVALID_ARG_INDEX";
    case (CL_INVALID_ARG_VALUE): return "CL_INVALID_ARG_VALUE";
    case (CL_INVALID_ARG_SIZE): return "CL_INVALID_ARG_SIZE";
    case (CL_INVALID_KERNEL_ARGS): return "CL_INVALID_KERNEL_ARGS";
    case (CL_INVALID_WORK_DIMENSION): return "CL_INVALID_WORK_DIMENSION";
    case (CL_INVALID_WORK_GROUP_SIZE): return "CL_INVALID_WORK_GROUP_SIZE";
    case (CL_INVALID_WORK_ITEM_SIZE): return "CL_INVALID_WORK_ITEM_SIZE";
    case (CL_INVALID_GLOBAL_OFFSET): return "CL_INVALID_GLOBAL_OFFSET";
    case (CL_INVALID_EVENT_WAIT_LIST): return "CL_INVALID_EVENT_WAIT_LIST";
    case (CL_INVALID_EVENT): return "CL_INVALID_EVENT";
    case (CL_INVALID_OPERATION): return "CL_INVALID_OPERATION";
    case (CL_INVALID_GL_OBJECT): return "CL_INVALID_GL_OBJECT";
    case (CL_INVALID_BUFFER_SIZE): return "CL_INVALID_BUFFER_SIZE";
    case (CL_INVALID_MIP_LEVEL): return "CL_INVALID_MIP_LEVE";
    case (CL_INVALID_PROPERTY) : return "CL_INVALID_PROPERTY";
    case (CL_INVALID_IMAGE_DESCRIPTOR) : return "CL_INVALID_IMAGE_DESCRIPTOR";
    case (CL_INVALID_COMPILER_OPTIONS) : return "CL_INVALID_COMPILER_OPTIONS";
    case (CL_INVALID_LINKER_OPTIONS) : return "CL_INVALID_LINKER_OPTIONS";
    case (CL_INVALID_DEVICE_PARTITION_COUNT) : return "CL_INVALID_DEVICE_PARTITION_COUNT";

        // OpenCL framework error codes

    case (CL_ERR_LOGGER_FAILED): return "CL_ERR_LOGGER_FAILED";
    case (CL_ERR_NOT_IMPLEMENTED): return "CL_ERR_NOT_IMPLEMENTED";
    case (CL_ERR_NOT_SUPPORTED): return "CL_ERR_NOT_SUPPORTED";
    case (CL_ERR_INITILIZATION_FAILED): return "CL_ERR_INITILIZATION_FAILED";
    case (CL_ERR_PLATFORM_FAILED): return "CL_ERR_PLATFORM_FAILED";
    case (CL_ERR_CONTEXT_FAILED): return "CL_ERR_CONTEXT_FAILED";
    case (CL_ERR_EXECUTION_FAILED): return "CL_ERR_EXECUTION_FAILED";
    case (CL_ERR_FILE_NOT_EXISTS): return "CL_ERR_FILE_NOT_EXISTS";
    case (CL_ERR_KEY_NOT_FOUND): return "CL_ERR_KEY_NOT_FOUND";
    case (CL_ERR_KEY_ALLREADY_EXISTS): return "CL_ERR_KEY_ALLREADY_EXISTS";
    case (CL_ERR_LIST_EMPTY): return "CL_ERR_LIST_EMPTY";
    case (CL_ERR_DEVICE_INIT_FAIL): return "CL_ERR_DEVICE_INIT_FAI";
    case (CL_ERR_FE_COMPILER_INIT_FAIL): return "CL_ERR_FE_COMPILER_INIT_FAI";
    default:
        return "Unknown Error Code";
    }
}

// used to make a macro definition into a string simply call DEFINE_TO_STRING with you defined macro "s"
#define DEFINE_TO_STRING(s) #s

//just return the DEFINE name as is in a string format
#define  CASE_DEFINE_RETURN_STRING(def) case (def): return #def
//a little more complex because this defines are bit fields, se usage below 
#define DEFAULT_PREFIX " "
#define DEFAULT_SUFFIX ""

#define IF_DEFINE_APPEND_STRING_DEFAULT(bitfield, def, str) if ( ((bitfield) & (def)) != 0 ) str += string(DEFAULT_PREFIX) + #def + string(DEFAULT_SUFFIX)
const string channelOrderToString(const cl_channel_order& co)
{
    switch (co) {
        CASE_DEFINE_RETURN_STRING(CL_R);
        CASE_DEFINE_RETURN_STRING(CL_A);
        CASE_DEFINE_RETURN_STRING(CL_INTENSITY);
        CASE_DEFINE_RETURN_STRING(CL_LUMINANCE);
        CASE_DEFINE_RETURN_STRING(CL_RG);
        CASE_DEFINE_RETURN_STRING(CL_RA);
        CASE_DEFINE_RETURN_STRING(CL_RGB);
        CASE_DEFINE_RETURN_STRING(CL_RGBA);
        CASE_DEFINE_RETURN_STRING(CL_ARGB);
        CASE_DEFINE_RETURN_STRING(CL_BGRA);
        CASE_DEFINE_RETURN_STRING(CL_Rx);
        CASE_DEFINE_RETURN_STRING(CL_RGx);
        CASE_DEFINE_RETURN_STRING(CL_RGBx);
        CASE_DEFINE_RETURN_STRING(CL_DEPTH);
        CASE_DEFINE_RETURN_STRING(CL_DEPTH_STENCIL);
        CASE_DEFINE_RETURN_STRING(CL_sRGB);
        CASE_DEFINE_RETURN_STRING(CL_sRGBx);
        CASE_DEFINE_RETURN_STRING(CL_sRGBA);
        CASE_DEFINE_RETURN_STRING(CL_sBGRA);
        CASE_DEFINE_RETURN_STRING(CL_ABGR);
    default:
        return "Not Recognized";
    }
}
const string channelTypeToString(const cl_channel_type& ct)
{
    switch (ct)
    {
        CASE_DEFINE_RETURN_STRING(CL_SNORM_INT8);
        CASE_DEFINE_RETURN_STRING(CL_SNORM_INT16);
        CASE_DEFINE_RETURN_STRING(CL_UNORM_INT8);
        CASE_DEFINE_RETURN_STRING(CL_UNORM_INT16);
        CASE_DEFINE_RETURN_STRING(CL_UNORM_SHORT_565);
        CASE_DEFINE_RETURN_STRING(CL_UNORM_SHORT_555);
        CASE_DEFINE_RETURN_STRING(CL_UNORM_INT_101010);
        CASE_DEFINE_RETURN_STRING(CL_SIGNED_INT8);
        CASE_DEFINE_RETURN_STRING(CL_SIGNED_INT16);
        CASE_DEFINE_RETURN_STRING(CL_SIGNED_INT32);
        CASE_DEFINE_RETURN_STRING(CL_UNSIGNED_INT8);
        CASE_DEFINE_RETURN_STRING(CL_UNSIGNED_INT16);
        CASE_DEFINE_RETURN_STRING(CL_UNSIGNED_INT32);
        CASE_DEFINE_RETURN_STRING(CL_HALF_FLOAT);
        CASE_DEFINE_RETURN_STRING(CL_FLOAT);
        CASE_DEFINE_RETURN_STRING(CL_UNORM_INT24);
    default:
        return "Not Recognized";
    }
}

const string imageFormatToString (const cl_image_format& format){
    return "{" + channelOrderToString(format.image_channel_order) + ", " + channelTypeToString(format.image_channel_data_type) + "}";
}

const string memTypeToString(const cl_mem_object_type& mo)
{
    switch (mo)
    {
        CASE_DEFINE_RETURN_STRING(CL_MEM_OBJECT_BUFFER);
        CASE_DEFINE_RETURN_STRING(CL_MEM_OBJECT_IMAGE2D);
        CASE_DEFINE_RETURN_STRING(CL_MEM_OBJECT_IMAGE3D);
        CASE_DEFINE_RETURN_STRING(CL_MEM_OBJECT_IMAGE2D_ARRAY);
        CASE_DEFINE_RETURN_STRING(CL_MEM_OBJECT_IMAGE1D);
        CASE_DEFINE_RETURN_STRING(CL_MEM_OBJECT_IMAGE1D_ARRAY);
        CASE_DEFINE_RETURN_STRING(CL_MEM_OBJECT_IMAGE1D_BUFFER);
        CASE_DEFINE_RETURN_STRING(CL_MEM_OBJECT_PIPE);
    default:
        return "Not Recognized";
    }
}

const string memFlagsToString(const cl_mem_flags& flags)
{
    string str = "";
    IF_DEFINE_APPEND_STRING_DEFAULT(flags,CL_MEM_READ_WRITE,str);
    IF_DEFINE_APPEND_STRING_DEFAULT(flags,CL_MEM_WRITE_ONLY,str);
    IF_DEFINE_APPEND_STRING_DEFAULT(flags,CL_MEM_READ_ONLY,str);
    IF_DEFINE_APPEND_STRING_DEFAULT(flags,CL_MEM_USE_HOST_PTR,str);
    IF_DEFINE_APPEND_STRING_DEFAULT(flags,CL_MEM_ALLOC_HOST_PTR,str);
    IF_DEFINE_APPEND_STRING_DEFAULT(flags,CL_MEM_COPY_HOST_PTR,str);
    IF_DEFINE_APPEND_STRING_DEFAULT(flags,CL_MEM_HOST_WRITE_ONLY,str);
    IF_DEFINE_APPEND_STRING_DEFAULT(flags,CL_MEM_HOST_READ_ONLY,str);
    IF_DEFINE_APPEND_STRING_DEFAULT(flags,CL_MEM_HOST_NO_ACCESS,str);
    if ( flags == 0)
    {
        return "None";
    }
    if ( str == "")
    {
        return "Not Recognized: " + stringify(flags);
    }

    return str.substr(strlen(DEFAULT_PREFIX)); //we don't want the first prefix
}

const string addressingModeToString(const cl_addressing_mode& am)
{
    switch (am)
    {
        CASE_DEFINE_RETURN_STRING(CL_ADDRESS_NONE);
        CASE_DEFINE_RETURN_STRING(CL_ADDRESS_CLAMP_TO_EDGE);
        CASE_DEFINE_RETURN_STRING(CL_ADDRESS_CLAMP);
        CASE_DEFINE_RETURN_STRING(CL_ADDRESS_REPEAT);
        CASE_DEFINE_RETURN_STRING(CL_ADDRESS_MIRRORED_REPEAT);
    default:
        return "Not Recognized";
    }
}

const string filteringModeToString(const cl_filter_mode& fm)
{
    switch (fm) {
        CASE_DEFINE_RETURN_STRING(CL_FILTER_LINEAR);
        CASE_DEFINE_RETURN_STRING(CL_FILTER_NEAREST);
    default:
        return "Not Recognized";
    }
}


const string commandQueuePropertiesToString(const cl_command_queue_properties& prop){
    string str = "";
    IF_DEFINE_APPEND_STRING_DEFAULT(prop,CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE,str);
    IF_DEFINE_APPEND_STRING_DEFAULT(prop,CL_QUEUE_PROFILING_ENABLE,str);
    IF_DEFINE_APPEND_STRING_DEFAULT(prop,CL_QUEUE_ON_DEVICE,str);
    IF_DEFINE_APPEND_STRING_DEFAULT(prop,CL_QUEUE_ON_DEVICE_DEFAULT,str);
    IF_DEFINE_APPEND_STRING_DEFAULT(prop,CL_QUEUE_THREAD_LOCAL_EXEC_ENABLE_INTEL,str);
    //IF_DEFINE_APPEND_STRING_DEFAULT(prop,CL_QUEUE_IMMEDIATE_EXECUTION_ENABLE_EXT,str); //TODO: extension, add?
    if ( prop == 0)
        return "None";
    if ( str == "")
        return "Not Recognized: " + stringify(prop);

    return  str.substr(strlen(DEFAULT_PREFIX));
}

const string deviceTypeToString(const cl_device_type& type)
{
    switch(type)
    {
        CASE_DEFINE_RETURN_STRING(CL_DEVICE_TYPE_DEFAULT);
        CASE_DEFINE_RETURN_STRING(CL_DEVICE_TYPE_CPU);
        CASE_DEFINE_RETURN_STRING(CL_DEVICE_TYPE_GPU);
        CASE_DEFINE_RETURN_STRING(CL_DEVICE_TYPE_ACCELERATOR);
        CASE_DEFINE_RETURN_STRING(CL_DEVICE_TYPE_CUSTOM);
        CASE_DEFINE_RETURN_STRING(CL_DEVICE_TYPE_ALL);
        default:
            return "Not Recognized: " + stringify(type);
    }
}
const string fpConfigToString(const cl_device_fp_config& fp_config)
{
    string str = "";
    IF_DEFINE_APPEND_STRING_DEFAULT(fp_config,CL_FP_DENORM,str);
    IF_DEFINE_APPEND_STRING_DEFAULT(fp_config,CL_FP_INF_NAN,str);
    IF_DEFINE_APPEND_STRING_DEFAULT(fp_config,CL_FP_ROUND_TO_NEAREST,str);
    IF_DEFINE_APPEND_STRING_DEFAULT(fp_config,CL_FP_ROUND_TO_ZERO,str);
    IF_DEFINE_APPEND_STRING_DEFAULT(fp_config,CL_FP_ROUND_TO_INF,str);
    IF_DEFINE_APPEND_STRING_DEFAULT(fp_config,CL_FP_FMA,str);
    IF_DEFINE_APPEND_STRING_DEFAULT(fp_config,CL_FP_SOFT_FLOAT,str);
    IF_DEFINE_APPEND_STRING_DEFAULT(fp_config,CL_FP_CORRECTLY_ROUNDED_DIVIDE_SQRT,str);
    if ( fp_config == 0)
        return "None";
    if ( str == "")
        return "Not Recognized: " + stringify(fp_config);

    return str.substr(strlen(DEFAULT_PREFIX));
}

const string memCacheTypeToString(const cl_device_mem_cache_type& memType)
{
    switch (memType) {
        CASE_DEFINE_RETURN_STRING(CL_NONE);
        CASE_DEFINE_RETURN_STRING(CL_READ_ONLY_CACHE);
        CASE_DEFINE_RETURN_STRING(CL_READ_WRITE_CACHE);
    default:
        return "Not Recognized";
    }
}


const string localMemTypeToString(const cl_device_local_mem_type& memType)
{
    switch (memType) {
        CASE_DEFINE_RETURN_STRING(CL_LOCAL);
        CASE_DEFINE_RETURN_STRING(CL_GLOBAL);
    default:
        return "Not Recognized";
    }
}

const string execCapabilitiesToString(const cl_device_exec_capabilities& execCap)
{
    string str = "";
    IF_DEFINE_APPEND_STRING_DEFAULT(execCap,CL_EXEC_KERNEL,str);
    IF_DEFINE_APPEND_STRING_DEFAULT(execCap,CL_EXEC_NATIVE_KERNEL,str);
    if ( execCap == 0)
        return "None";
    if ( str == "")
        return "Not Recognized: " + stringify(execCap);

    return str.substr(strlen(DEFAULT_PREFIX));
}

const string commandTypeToString(const cl_command_type& type)
{
    switch (type) {
        CASE_DEFINE_RETURN_STRING(CL_COMMAND_NDRANGE_KERNEL);
        CASE_DEFINE_RETURN_STRING(CL_COMMAND_TASK);
        CASE_DEFINE_RETURN_STRING(CL_COMMAND_NATIVE_KERNEL);
        CASE_DEFINE_RETURN_STRING(CL_COMMAND_READ_BUFFER);
        CASE_DEFINE_RETURN_STRING(CL_COMMAND_WRITE_BUFFER);
        CASE_DEFINE_RETURN_STRING(CL_COMMAND_COPY_BUFFER);
        CASE_DEFINE_RETURN_STRING(CL_COMMAND_READ_IMAGE);
        CASE_DEFINE_RETURN_STRING(CL_COMMAND_WRITE_IMAGE);
        CASE_DEFINE_RETURN_STRING(CL_COMMAND_COPY_IMAGE);
        CASE_DEFINE_RETURN_STRING(CL_COMMAND_COPY_IMAGE_TO_BUFFER);
        CASE_DEFINE_RETURN_STRING(CL_COMMAND_COPY_BUFFER_TO_IMAGE);
        CASE_DEFINE_RETURN_STRING(CL_COMMAND_MAP_BUFFER);
        CASE_DEFINE_RETURN_STRING(CL_COMMAND_MAP_IMAGE);
        CASE_DEFINE_RETURN_STRING(CL_COMMAND_UNMAP_MEM_OBJECT);
        CASE_DEFINE_RETURN_STRING(CL_COMMAND_MARKER);
        CASE_DEFINE_RETURN_STRING(CL_COMMAND_ACQUIRE_GL_OBJECTS);
        CASE_DEFINE_RETURN_STRING(CL_COMMAND_RELEASE_GL_OBJECTS);
        CASE_DEFINE_RETURN_STRING(CL_COMMAND_READ_BUFFER_RECT);
        CASE_DEFINE_RETURN_STRING(CL_COMMAND_WRITE_BUFFER_RECT);
        CASE_DEFINE_RETURN_STRING(CL_COMMAND_COPY_BUFFER_RECT);
        CASE_DEFINE_RETURN_STRING(CL_COMMAND_USER);
        CASE_DEFINE_RETURN_STRING(CL_COMMAND_BARRIER);
        CASE_DEFINE_RETURN_STRING(CL_COMMAND_MIGRATE_MEM_OBJECTS);
        CASE_DEFINE_RETURN_STRING(CL_COMMAND_FILL_BUFFER);
        CASE_DEFINE_RETURN_STRING(CL_COMMAND_FILL_IMAGE);
        CASE_DEFINE_RETURN_STRING(CL_COMMAND_SVM_FREE);
        CASE_DEFINE_RETURN_STRING(CL_COMMAND_SVM_MEMCPY);
        CASE_DEFINE_RETURN_STRING(CL_COMMAND_SVM_MEMFILL);
        CASE_DEFINE_RETURN_STRING(CL_COMMAND_SVM_MAP);
        CASE_DEFINE_RETURN_STRING(CL_COMMAND_SVM_UNMAP);
        CASE_DEFINE_RETURN_STRING(CL_COMMAND_GL_FENCE_SYNC_OBJECT_KHR);
        #ifdef WIN32
        CASE_DEFINE_RETURN_STRING(CL_COMMAND_ACQUIRE_D3D10_OBJECTS_KHR);
        CASE_DEFINE_RETURN_STRING(CL_COMMAND_RELEASE_D3D10_OBJECTS_KHR);
        CASE_DEFINE_RETURN_STRING(CL_COMMAND_ACQUIRE_D3D11_OBJECTS_KHR);
        CASE_DEFINE_RETURN_STRING(CL_COMMAND_RELEASE_D3D11_OBJECTS_KHR);
        CASE_DEFINE_RETURN_STRING(CL_COMMAND_ACQUIRE_DX9_MEDIA_SURFACES_KHR);
        CASE_DEFINE_RETURN_STRING(CL_COMMAND_RELEASE_DX9_MEDIA_SURFACES_KHR);
        CASE_DEFINE_RETURN_STRING(CL_COMMAND_ACQUIRE_DX9_OBJECTS_INTEL);
        CASE_DEFINE_RETURN_STRING(CL_COMMAND_RELEASE_DX9_OBJECTS_INTEL);
        #endif
    default:
        return "Not Recognized";
    }
}

const string executionStatusToString(const cl_int& status)
{
    switch (status) {
        CASE_DEFINE_RETURN_STRING(CL_COMPLETE);
        CASE_DEFINE_RETURN_STRING(CL_RUNNING);
        CASE_DEFINE_RETURN_STRING(CL_SUBMITTED);
        CASE_DEFINE_RETURN_STRING(CL_QUEUED);
    default:
        return "Not Recognized";
    }
}

const string buildStatusToString(const cl_build_status& status)
{
    switch (status) {
        CASE_DEFINE_RETURN_STRING(CL_BUILD_NONE);
        CASE_DEFINE_RETURN_STRING(CL_BUILD_SUCCESS);
        CASE_DEFINE_RETURN_STRING(CL_BUILD_ERROR);
        CASE_DEFINE_RETURN_STRING(CL_BUILD_IN_PROGRESS);
    default:
        return "Not Recognized";
    }
}

const string binaryTypeToString(const cl_program_binary_type& type)
{
    switch (type) {
        CASE_DEFINE_RETURN_STRING(CL_PROGRAM_BINARY_TYPE_NONE);
        CASE_DEFINE_RETURN_STRING(CL_PROGRAM_BINARY_TYPE_COMPILED_OBJECT);
        CASE_DEFINE_RETURN_STRING(CL_PROGRAM_BINARY_TYPE_LIBRARY);
        CASE_DEFINE_RETURN_STRING(CL_PROGRAM_BINARY_TYPE_EXECUTABLE);
        CASE_DEFINE_RETURN_STRING(CL_PROGRAM_BINARY_TYPE_INTERMEDIATE);
    default:
        return "Not Recognized";
    }
}

const string addressQualifierToString_def(const cl_kernel_arg_address_qualifier& add)
{
    switch (add) {
        CASE_DEFINE_RETURN_STRING(CL_KERNEL_ARG_ADDRESS_GLOBAL);
        CASE_DEFINE_RETURN_STRING(CL_KERNEL_ARG_ADDRESS_LOCAL);
        CASE_DEFINE_RETURN_STRING(CL_KERNEL_ARG_ADDRESS_CONSTANT);
        CASE_DEFINE_RETURN_STRING(CL_KERNEL_ARG_ADDRESS_PRIVATE);
    default:
        return "Not Recognized";
    }
}

const string typeQualifierToString(const cl_kernel_arg_type_qualifier& type)
{
    string str = "";
    IF_DEFINE_APPEND_STRING_DEFAULT(type,CL_KERNEL_ARG_TYPE_CONST,str);
    IF_DEFINE_APPEND_STRING_DEFAULT(type,CL_KERNEL_ARG_TYPE_RESTRICT,str);
    IF_DEFINE_APPEND_STRING_DEFAULT(type,CL_KERNEL_ARG_TYPE_VOLATILE,str);
    IF_DEFINE_APPEND_STRING_DEFAULT(type,CL_KERNEL_ARG_TYPE_PIPE,str);
    if ( type == 0) //    CL_KERNEL_ARG_TYPE_NONE = 0
    {
        return "CL_KERNEL_ARG_TYPE_NONE";
    }

    if ( str == "")
    {
        return "Not Recognized: " + stringify(type);
    }

    return str.substr(strlen(DEFAULT_PREFIX));
}

const string accessQualifierToString_def(const cl_kernel_arg_access_qualifier& acc)
{
    switch (acc) {
        CASE_DEFINE_RETURN_STRING(CL_KERNEL_ARG_ACCESS_READ_ONLY);
        CASE_DEFINE_RETURN_STRING(CL_KERNEL_ARG_ACCESS_WRITE_ONLY);
        CASE_DEFINE_RETURN_STRING(CL_KERNEL_ARG_ACCESS_READ_WRITE);
        CASE_DEFINE_RETURN_STRING(CL_KERNEL_ARG_ACCESS_NONE);
    default:
        return "Not Recognized";
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////
// addressQualifierToString
///////////////////////////////////////////////////////////////////////////////////////////////////////
const string addressQualifierToString(const cl_kernel_arg_address_qualifier& AddressQualifer)  {
    switch (AddressQualifer)
    {
    case CL_KERNEL_ARG_ADDRESS_GLOBAL:
        return "__global";
    case CL_KERNEL_ARG_ADDRESS_LOCAL:
        return "__local";
    case CL_KERNEL_ARG_ADDRESS_CONSTANT:
        return "__const";
    case CL_KERNEL_ARG_ADDRESS_PRIVATE:
    default:
        return "__private";
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
// accessQualifierToString
///////////////////////////////////////////////////////////////////////////////////////////////////////
const string accessQualifierToString (const cl_kernel_arg_access_qualifier& AccessQualifier)
{
    switch (AccessQualifier)
    {
    case CL_KERNEL_ARG_ACCESS_READ_ONLY:
        return "READ_ONLY";
    case CL_KERNEL_ARG_ACCESS_WRITE_ONLY:
        return "WRITE_ONLY";
    case CL_KERNEL_ARG_ACCESS_READ_WRITE:
        return "READ_WRITE";
    case CL_KERNEL_ARG_ACCESS_NONE:
        return "NONE";
    }
    string Error("Failed to get a string from access qualifier ");
    Error += stringify<unsigned int>((unsigned int)AccessQualifier);
    throw Error;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
// GetAddressingModeFromString
///////////////////////////////////////////////////////////////////////////////////////////////////////
cl_addressing_mode GetAddressingModeFromString(const std::string& Mode)
{
    if(Mode == "CL_ADDRESS_MIRRORED_REPEAT")
    {
        return CL_ADDRESS_MIRRORED_REPEAT;
    }
    else if (Mode == "CL_ADDRESS_REPEAT")
    {
        return CL_ADDRESS_REPEAT;
    }
    else if (Mode == "CL_ADDRESS_CLAMP_TO_EDGE")
    {
        return CL_ADDRESS_CLAMP_TO_EDGE;
    }
    else if (Mode == "CL_ADDRESS_CLAMP")
    {
        return CL_ADDRESS_CLAMP;
    }
    else if (Mode == "CL_ADDRESS_NONE")
    {
        return CL_ADDRESS_NONE;
    }
    string Error("Unrecognized addressing mode '");
    Error += Mode + "'";
    throw Error;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
// GetFilterModeFromString
///////////////////////////////////////////////////////////////////////////////////////////////////////
cl_filter_mode GetFilterModeFromString(const string& Mode)
{
    if (Mode == "CL_FILTER_NEAREST")
    {
        return CL_FILTER_NEAREST;
    }
    else if (Mode == "CL_FILTER_LINEAR")
    {
        return CL_FILTER_LINEAR;
    }
    string Error("Unrecognized filter mode '");
    Error += Mode + "'";
    throw Error;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
// GetChannelOrderFromString
///////////////////////////////////////////////////////////////////////////////////////////////////////
cl_channel_order GetChannelOrderFromString(const string& Order)
{
    if (Order == "CL_R")
    {
        return CL_R;
    }
    else if (Order == "CL_Rx")
    {
        return CL_Rx;
    }
    else if (Order == "CL_A")
    {
        return CL_A;
    }
    else if (Order == "CL_INTENSITY")
    {
        return CL_INTENSITY;
    }
    else if (Order == "CL_LUMINANCE") 
    {
        return CL_LUMINANCE;
    }
    else if (Order == "CL_RG") 
    {
        return CL_RG;
    }
    else if (Order == "CL_RGx")
    {
        return CL_RGx;
    }
    else if (Order == "CL_RA")
    {
        return CL_RA;
    }
    else if (Order == "CL_RGB")
    {
        return CL_RGB;
    }
    else if (Order == "CL_RGBx")
    {
        return CL_RGBx;
    }
    else if (Order == "CL_RGBA")
    {
        return CL_RGBA;
    } 
    else if (Order == "CL_ARGB")
    {
        return CL_ARGB;
    } 
    else if (Order == "CL_BGRA")
    {
        return CL_BGRA;
    }
    else if (Order == "CL_DEPTH")
    {
        return CL_DEPTH;
    }
    else if (Order == "CL_sRGB")
    {
        return CL_sRGB;
    }
    else if (Order == "CL_sRGBx")
    {
        return CL_sRGBx;
    }
    else if (Order == "CL_sRGBA")
    {
        return CL_sRGBA;
    }
    else if (Order == "CL_sBGRA")
    {
        return CL_sBGRA;
    }
    else if (Order == "CL_ABGR")
    {
        return CL_ABGR;
    }
    string Error("Unrecognized channel order '");
    Error += Order + "'";
    throw Error;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
// GetChannelTypeFromString
///////////////////////////////////////////////////////////////////////////////////////////////////////
cl_channel_type GetChannelTypeFromString(const string& Type)
{
    if (Type == "CL_SNORM_INT8")
    {
        return CL_SNORM_INT8;
    }
    else if (Type == "CL_SNORM_INT16")
    {
        return CL_SNORM_INT16;
    }
    else if (Type == "CL_UNORM_INT8")
    {
        return CL_UNORM_INT8;
    }
    else if (Type == "CL_UNORM_INT16")
    {
        return CL_UNORM_INT16;
    }
    else if (Type == "CL_UNORM_SHORT_565")
    {
        return CL_UNORM_SHORT_565;
    }
    else if (Type == "CL_UNORM_SHORT_555")
    {
        return CL_UNORM_SHORT_555;
    }
    else if (Type == "CL_UNORM_INT_101010")
    {
        return CL_UNORM_INT_101010;
    }
    else if (Type == "CL_SIGNED_INT8")
    {
        return CL_SIGNED_INT8;
    }
    else if (Type == "CL_SIGNED_INT16")
    {
        return CL_SIGNED_INT16;
    }
    else if (Type == "CL_SIGNED_INT32")
    {
        return CL_SIGNED_INT32;
    }
    else if (Type == "CL_UNSIGNED_INT8")
    {
        return CL_UNSIGNED_INT8;
    }
    else if (Type == "CL_UNSIGNED_INT16")
    {
        return CL_UNSIGNED_INT16;
    }
    else if (Type == "CL_UNSIGNED_INT32")
    {
        return CL_UNSIGNED_INT32;
    }
    else if (Type == "CL_HALF_FLOAT")
    {
        return CL_HALF_FLOAT;
    }
    else if (Type == "CL_FLOAT")
    {
        return CL_FLOAT;
    }
    string Error("Unrecognized channel type '");
    Error += Type + "'";
    throw Error;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
// GetImageTypeFromString
///////////////////////////////////////////////////////////////////////////////////////////////////////
cl_mem_object_type GetImageTypeFromString(const string& Type)
{
    if (Type == "CL_MEM_OBJECT_IMAGE1D" || Type == "image1d_t")
    {
        return CL_MEM_OBJECT_IMAGE1D;
    }
    else if (Type == "CL_MEM_OBJECT_IMAGE1D_BUFFER" || Type == "image1d_buffer_t")
    {
        return CL_MEM_OBJECT_IMAGE1D_BUFFER;
    }
    else if (Type == "CL_MEM_OBJECT_IMAGE1D_ARRAY"|| Type == "image1d_array_t")
    {
        return CL_MEM_OBJECT_IMAGE1D_ARRAY;
    }
    else if (Type == "CL_MEM_OBJECT_IMAGE2D"|| Type == "image2d_t")
    {
        return CL_MEM_OBJECT_IMAGE2D;
    }
    else if (Type == "CL_MEM_OBJECT_IMAGE2D_ARRAY"|| Type == "image2d_array_t")
    {
        return CL_MEM_OBJECT_IMAGE2D_ARRAY;
    }
    else if (Type == "CL_MEM_OBJECT_IMAGE3D"|| Type == "image3d_t")
    {
        return CL_MEM_OBJECT_IMAGE3D;
    }
    string Error("Unrecognized image type '");
    Error += Type + "'";
    throw Error;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
// SetKernelArgument
///////////////////////////////////////////////////////////////////////////////////////////////////////
cl_mem_flags GetSingleMemoryFlagFromString(const string& OneFlagStr)
{
    if (OneFlagStr  == "CL_MEM_ALLOC_HOST_PTR")
    {
        return  CL_MEM_ALLOC_HOST_PTR;
    }
    else if (OneFlagStr  == "CL_MEM_COPY_HOST_PTR")
    {
        return CL_MEM_COPY_HOST_PTR;
    }
    else if (OneFlagStr == "CL_MEM_HOST_NO_ACCESS")
    {
        return CL_MEM_HOST_NO_ACCESS;
    }
    else if (OneFlagStr == "CL_MEM_HOST_READ_ONLY")
    {
        return CL_MEM_HOST_READ_ONLY;
    }
    else if (OneFlagStr == "CL_MEM_HOST_WRITE_ONLY")
    {
        return CL_MEM_HOST_WRITE_ONLY;
    }
    else if (OneFlagStr == "CL_MEM_READ_ONLY")
    {
        return CL_MEM_READ_ONLY;
    }
    else if (OneFlagStr == "CL_MEM_READ_WRITE")
    {
        return CL_MEM_READ_WRITE;
    }
    else if (OneFlagStr == "CL_MEM_USE_HOST_PTR")
    {
        return CL_MEM_USE_HOST_PTR;
    }
    else if (OneFlagStr == "CL_MEM_WRITE_ONLY")
    {
        return CL_MEM_WRITE_ONLY;
    }
    else
    {
        return 0;
    }
}

cl_mem_flags GetMemFlagsFromString(const string& FlagsStr) 
{
    cl_mem_flags Flags(0);
    size_t Tokenizer = FlagsStr.find_first_of(" ,"), PrevToken = 0; 
    string iToken;
    while (Tokenizer != string::npos)
    {
        iToken = FlagsStr.substr(PrevToken, Tokenizer-PrevToken); 
        Flags |= GetSingleMemoryFlagFromString(iToken); 
        PrevToken = Tokenizer+1; 
        Tokenizer = FlagsStr.find_first_of(", ", PrevToken); 
    }
    iToken = FlagsStr.substr(PrevToken); 
    // Only last value
    Flags |= GetSingleMemoryFlagFromString(iToken);
    return Flags;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////
// GetAddressQualifierFromString
///////////////////////////////////////////////////////////////////////////////////////////////////////
cl_kernel_arg_address_qualifier GetAddressQualifierFromString(const std::string& AddressQualifier)
{
    if (AddressQualifier == "__global")
    {
        return CL_KERNEL_ARG_ADDRESS_GLOBAL;
    }
    else if (AddressQualifier == "__local")
    {
        return CL_KERNEL_ARG_ADDRESS_LOCAL;
    }
    else if (AddressQualifier == "__const")
    {
        return CL_KERNEL_ARG_ADDRESS_CONSTANT;
    }
    else if (AddressQualifier == "__private")
    {
        return CL_KERNEL_ARG_ADDRESS_PRIVATE;
    }
    string Error("Failed to get an address qualifier from ");
    Error += AddressQualifier;
    throw Error;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
// GetAccessQualifierFromString
///////////////////////////////////////////////////////////////////////////////////////////////////////
cl_kernel_arg_access_qualifier GetAccessQualifierFromString(const std::string& AccessQualifier)
{
    if (AccessQualifier == "READ_ONLY")
    {
        return CL_KERNEL_ARG_ACCESS_READ_ONLY;
    }
    else if (AccessQualifier == "WRITE_ONLY")
    {
        return CL_KERNEL_ARG_ACCESS_WRITE_ONLY;
    }
    else if (AccessQualifier == "READ_WRITE")
    {
        return CL_KERNEL_ARG_ACCESS_READ_WRITE;
    }
    else if (AccessQualifier == "NONE")
    {
        return CL_KERNEL_ARG_ACCESS_NONE;
    }
    string Error("Failed to get an address qualifier from ");
    Error += AccessQualifier;
    throw Error;
}

void clCopyMemoryRegion(SMemCpyParams* pCopyCmd)
{
    // Copy 1D array only
    if ( 1 == pCopyCmd->uiDimCount )
    {
        //useless memcpy_s, this function is called from many different places
        MEMCPY_S(pCopyCmd->pDst, pCopyCmd->vRegion[0], pCopyCmd->pSrc, pCopyCmd->vRegion[0]);
        return;
    }

    SMemCpyParams sRecParam;

    // Copy current parameters
    MEMCPY_S(&sRecParam, sizeof(SMemCpyParams), pCopyCmd, sizeof(SMemCpyParams));
    sRecParam.uiDimCount = pCopyCmd->uiDimCount-1;
    // Make recursion
    for(unsigned int i=0; i<pCopyCmd->vRegion[sRecParam.uiDimCount]; ++i)
    {
        clCopyMemoryRegion(&sRecParam);
        sRecParam.pSrc = sRecParam.pSrc + pCopyCmd->vSrcPitch[sRecParam.uiDimCount-1];
        sRecParam.pDst = sRecParam.pDst + pCopyCmd->vDstPitch[sRecParam.uiDimCount-1];
    }
}

size_t clGetPixelBytesCount(const cl_image_format* pclImageFormat)
{
    if (NULL == pclImageFormat)
    {
        return 0;
    }
    size_t szBytesCount = 0, szElementsCount = 0;

    // get size of element in bytes
    szBytesCount = clGetChannelTypeBytesCount(pclImageFormat->image_channel_data_type);

    // get number of elements in pixel
    szElementsCount = clGetPixelElementsCount(pclImageFormat);

    return szBytesCount * szElementsCount;
}

size_t clGetPixelElementsCount(const cl_image_format* pclImageFormat)
{
    if (NULL == pclImageFormat)
    {
        return 0;
    }

    switch(pclImageFormat->image_channel_order)
    {
    case CL_R:
    case CL_A:
        return 1;
    case CL_RG:
    case CL_RA:
        return 2;
    case CL_RGB:
        return 1;
    case CL_RGBA:
        return 4;
    case CL_BGRA:
    case CL_ARGB:
    case CL_ABGR:
        if (CL_UNORM_INT8 != pclImageFormat->image_channel_data_type && CL_SNORM_INT8 != pclImageFormat->image_channel_data_type && CL_SIGNED_INT8 != pclImageFormat->image_channel_data_type &&
            CL_UNSIGNED_INT8 != pclImageFormat->image_channel_data_type)
        {
            return 0;
        }
        return 4;
    case CL_LUMINANCE:
    case CL_INTENSITY:
        return 1;
    case CL_DEPTH:
        if (CL_UNORM_INT16 != pclImageFormat->image_channel_data_type && CL_FLOAT != pclImageFormat->image_channel_data_type)    // this isn't allowed
        {
            return 0;
        }
        return 1;
    case CL_sRGB:
    case CL_sRGBx:
        if (CL_UNORM_INT8 != pclImageFormat->image_channel_data_type)
        {
            return 0;
        }
        return 3;
    case CL_sRGBA:
    case CL_sBGRA:
        if (CL_UNORM_INT8 != pclImageFormat->image_channel_data_type)
        {
            return 0;
        }
        return 4;
    default:
        return 0;
    }
}

size_t clGetChannelTypeBytesCount(cl_channel_type pclImageChannelType)
{
    switch(pclImageChannelType)
    {
    case CL_SNORM_INT8:
    case CL_SIGNED_INT8:
        return sizeof(cl_char);
    case CL_UNORM_INT8:
    case CL_UNSIGNED_INT8:
        return sizeof(cl_uchar);
    case CL_SNORM_INT16:
    case CL_SIGNED_INT16:
        return sizeof(cl_short);
    case CL_UNORM_SHORT_565:
    case CL_UNORM_SHORT_555:
    case CL_UNORM_INT16:
    case CL_UNSIGNED_INT16:
        return sizeof(cl_ushort);
    case CL_SIGNED_INT32:
    case CL_UNORM_INT_101010:
        return sizeof(cl_int);
    case CL_UNSIGNED_INT32:
        return sizeof(cl_uint);
    case CL_HALF_FLOAT:
        return sizeof(cl_half);
    case CL_FLOAT:
        return sizeof(cl_float);
    default:
        return 0;
    }
}

std::vector<std::string> &SplitString(const std::string &s, char delim, std::vector<std::string> &elems) {
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) 
    {
        if (!item.empty() )
            elems.push_back(item);
    }
    return elems;
}

std::vector<std::string> SplitString(const std::string &s, char delim) {
    std::vector<std::string> elems;
    SplitString(s, delim, elems);
    return elems;
}


/////////////////////////////////////////////////////////////////////////
// TrimString
/////////////////////////////////////////////////////////////////////////
string TrimString(const string& sSource, const char *chars)
{
    size_t Start = sSource.find_first_not_of(chars);
    if (Start == string::npos) {
        // only "*chars"
        return "";
    }
    size_t End = sSource.find_last_not_of(chars);
    if ((Start == 0) && (End == sSource.length() - 1)) {
        // noting to trim
        return sSource;
    }
    return sSource.substr(Start, (End - Start) + 1);
}

/////////////////////////////////////////////////////////////////////////
// ToNarrow
/////////////////////////////////////////////////////////////////////////
string  ToNarrow(const wchar_t *s, char dfault, 
    const locale& loc)
{
    ostringstream stm;

    while( *s != L'\0' ) {
        stm << use_facet< ctype<wchar_t> >(loc).narrow(*s++, dfault);
    }
    return stm.str();
}

/////////////////////////////////////////////////////////////////////////
// FormatClError
/////////////////////////////////////////////////////////////////////////
string FormatClError(const string& Base, cl_int CLError) 
{
    string Error(Base);
    Error += ": ";
    Error += stringify<cl_int>(CLError);
    Error += " (";
    //Error += ToNarrow(ClErrTxt(CLError));
    Error += ClErrTxt(CLError);
    Error += ")";
    return Error;
}

/////////////////////////////////////////////////////////////////////////
// GetTempDir
/////////////////////////////////////////////////////////////////////////
string GetTempDir()
{
    string TmpDir;
#if defined (_WIN32) // Windows
    char *EnvTemp = getenv("TEMP");
    if (EnvTemp)
    {
        TmpDir = EnvTemp;
        TmpDir += "\\";
    }
#else // Linux
    char *EnvUser = getenv("USER");
    #if defined(__ANDROID__)
        TmpDir = "/data/local/tmp/";
    #else
        TmpDir = "/tmp/";
    #endif
        if (EnvUser)
          {
            TmpDir += EnvUser;
            TmpDir += "/";
          }
        // Just to make sure the directory exists
        string command("mkdir -p ");
        command += TmpDir;
        if(system(command.c_str()) != 0)
        {
            perror("Error creating temp directory");
        }
#endif
    return TmpDir;
}

/////////////////////////////////////////////////////////////////////////
// GetDeviceTypeString
/////////////////////////////////////////////////////////////////////////
string GetDeviceTypeString(const cl_device_type& Type)
{
    string DevType;
    if (Type & CL_DEVICE_TYPE_CPU)
    {
        DevType += "CL_DEVICE_TYPE_CPU | ";
    }
    if (Type & CL_DEVICE_TYPE_GPU)
    {
        DevType += "CL_DEVICE_TYPE_GPU | ";
    }
    if (Type & CL_DEVICE_TYPE_ACCELERATOR)
    {
        DevType += "CL_DEVICE_TYPE_ACCELERATOR | ";
    }
    if (Type & CL_DEVICE_TYPE_CUSTOM)
    {
        DevType += "CL_DEVICE_TYPE_CUSTOM | ";
    }
    if (Type & CL_DEVICE_TYPE_DEFAULT)
    {
        DevType += "CL_DEVICE_TYPE_DEFAULT | ";
    }
    if (DevType.empty())
    {
        throw string("Failed to get device type");
    }
    return DevType.substr(0, DevType.length()-3); // Remove the last " | "
}

cl_ushort float2half_rte( float f )
{
    union{ float f; cl_uint u; } u = {f};
    cl_uint sign = (u.u >> 16) & 0x8000;
    float x = fabsf(f);

    //Nan
    if( x != x )
    {
        u.u >>= (24-11);
        u.u &= 0x7fff;
        u.u |= 0x0200;      //silence the NaN
        return u.u | sign;
    }

    // overflow
    if( x >= MAKE_HEX_FLOAT(0, 0x1ffeL, 3) )
        return 0x7c00 | sign;

    // underflow
    if( x <= MAKE_HEX_FLOAT(0, 0x1L, -25) )
        return sign;    // The halfway case can return 0x0001 or 0. 0 is even.
    
    // very small
    if( x < MAKE_HEX_FLOAT(0, 0x18L, -28) )
        return sign | 1;

    // half denormal
    if( x < MAKE_HEX_FLOAT(0, 0x1L, -14) )
    {
        u.f = x * MAKE_HEX_FLOAT(0, 0x1L, -125);
        return sign | u.u;
    }
    
    u.f *= MAKE_HEX_FLOAT(0, 0x1L, 13);
    u.u &= 0x7f800000;
    x += u.f;
    u.f = x - u.f;
    u.f *= MAKE_HEX_FLOAT(0, 0x1L, -112);
    
    return (u.u >> (24-11)) | sign;
}

float half2float( cl_ushort us )
{
    uint32_t u = us;                   
    uint32_t sign = (u << 16) & 0x80000000;
    int32_t exponent = (u & 0x7c00) >> 10;     
    uint32_t mantissa = (u & 0x03ff) << 13;
    union{ unsigned int u; float f;}uu;
    
    if( exponent == 0 )
    {
        if( mantissa == 0 )
            return sign ? -0.0f : 0.0f;
        
        int shift = __builtin_clz( mantissa ) - 8;
        exponent -= shift-1;
        mantissa <<= shift;
        mantissa &= 0x007fffff;
    }
    else
        if( exponent == 31)
        {
            uu.u = mantissa | sign;
            if( mantissa )
                uu.u |= 0x7fc00000;
            else
                uu.u |= 0x7f800000;
            
            return uu.f;
        }
    
    exponent += 127 - 15;
    exponent <<= 23;
    
    exponent |= mantissa;
    uu.u = exponent | sign;
    
    return uu.f;
}

void CopyPattern(const void* pPattern, size_t szPatternSize, void* pBuffer, size_t szBufferSize)
{
    if (szPatternSize > sizeof(long long) || szBufferSize < sizeof(long long))
    {
        // for long patterns do memcpy
        for (size_t offset=0 ; offset < szBufferSize ; offset += szPatternSize)
        {        
            // using memcpy intentionally, because MEMCPY_S has too much overhead in this loop
            memcpy((char*)pBuffer + offset, pPattern, szPatternSize);
        }
    }
    else if (szPatternSize > 1)
    {
        // for patterns the size of long long or smaller (but not a single byte) do direct assignments and save the overhead of calling memcpy
        long long llPatten = 0;
        for (size_t i = 0; i < sizeof(llPatten) / szPatternSize; i++)
        {
            // using memcpy intentionally, because MEMCPY_S has too much overhead in this loop
            memcpy((char*)&llPatten + i * szPatternSize, pPattern, szPatternSize);
        }
        for (size_t offset = 0; offset < (szBufferSize % sizeof(llPatten) == 0 ? szBufferSize : szBufferSize - sizeof(llPatten)); offset += sizeof(llPatten))
        {
            *(long long*)((char*)pBuffer + offset) = llPatten;
        }
        // deal with the reminder
        if (szBufferSize % sizeof(llPatten) != 0)
        {
            for (size_t offset = szBufferSize - szBufferSize % sizeof(llPatten); offset < szBufferSize; offset += szPatternSize)
            {
                // using memcpy intentionally, because MEMCPY_S has too much overhead in this loop
                memcpy((char*)pBuffer + offset, pPattern, szPatternSize);
            }            
        }
    }
    else
    {
        // for single-byte patterns memset is the fastest
        memset(pBuffer, ((char*)pPattern)[0], szBufferSize);
    }
}

std::string GetConfigFilePath()
{
    std::string path("cl.cfg");
    return path;
}

#if defined(_MSC_VER) && !defined(_WIN64)
//  Returns the number of leading 0-bits in x, 
//  starting at the most significant bit position. 
//  If x is 0, the result is undefined.
// 
int __builtin_clz(unsigned int pattern)
{
    unsigned long index;
    unsigned char res = _BitScanReverse( &index, pattern);
    if (res) {
        return 8*sizeof(int) - 1 - index;
    } else {
        return 8*sizeof(int);
    }
}
#else
int __builtin_clz(unsigned int pattern)
{
   int count;
   if (pattern == 0u) {
       return 32;
   }
   count = 31;
   if (pattern >= 1u<<16) { pattern >>= 16; count -= 16; }
   if (pattern >=  1u<<8) { pattern >>=  8; count -=  8; }
   if (pattern >=  1u<<4) { pattern >>=  4; count -=  4; }
   if (pattern >=  1u<<2) { pattern >>=  2; count -=  2; }
   if (pattern >=  1u<<1) {                 count -=  1; }
   return count;
}
#endif //defined(_MSC_VER) && !defined(_WIN64)


std::string getLocalHostName()
{
    string computerName("");
    char buffer[1024];
#ifdef _WIN32
    DWORD nameLength = 1024;
    if(GetComputerNameA(buffer,&nameLength))
#else
    if(!gethostname(buffer,1024))
#endif
    {
        computerName = buffer;
    }
    return computerName;
}

#ifdef WIN32
bool GetStringValueFromRegistryOrETC( HKEY       top_hkey,
                                        const char *keyPath,
                                        const char *valueName,
                                        char       *retValue,
                                        DWORD      size )
{
    HKEY hkey;

    // Open the registry path. hkey will hold the entry
    LONG retCode = RegOpenKeyExA(
        top_hkey,                   // hkey
        keyPath,                    // lpSubKey
        0,                          // ulOptions
        KEY_READ,                   // samDesired
        &hkey                       // phkResult
        );

    if( ERROR_SUCCESS == retCode )
    {
        // Get the value by name from the key
        retCode = RegQueryValueExA(
            hkey,                   // hkey
            valueName,              // lpValueName
            0,                      // lpReserved
            NULL,                   // lpType
            ( LPBYTE )retValue,     // lpData
            &size                   // lpcbData
            );

        // Close the key - we don't need it any more
        RegCloseKey( hkey );

        if( ERROR_SUCCESS == retCode )
        {
            return true;
        }
    }

    return false;
}
#endif

bool GetCpuPath( char *pCpuPath, size_t bufferSize )
{
#if defined( _WIN32 )
    const char *regPath = "SOFTWARE\\Intel\\OpenCL";

    // pCpuPath is expected to be MAX_PATH in size
    if( NULL != pCpuPath )
    {
        string valueName;
        if (EmulatorEnabled())
        {
            valueName = "cpu_2_0_emulator_path";
        }
        else
        {
            valueName = "cpu_path";
        }
        return GetStringValueFromRegistryOrETC( HKEY_LOCAL_MACHINE, regPath, valueName.c_str(), pCpuPath, bufferSize );
    }
#endif
    return false;
}

bool GetCpuVersion( char *pCpuVersion, size_t bufferSize )
{
    #if defined( _WIN32 )
    const char *regPath = "SOFTWARE\\Intel\\OpenCL";

    // pCpuPath is expected to be MAX_PATH in size
    if( NULL != pCpuVersion )
    {
        string valueName;
        if (EmulatorEnabled())
        {
            valueName = "cpu_2_0_emulator_version";
        }
        else
        {
            valueName = "cpu_version";
        }
        return GetStringValueFromRegistryOrETC( HKEY_LOCAL_MACHINE, regPath, valueName.c_str(), pCpuVersion, bufferSize );
    }
#endif
    return false;
}

bool EmulatorEnabled()
{
#if defined( _WIN32 )
    const char *regPath = "SOFTWARE\\Intel\\OpenCL";
    char emulatorVal[16];
    bool retVal = GetStringValueFromRegistryOrETC( HKEY_LOCAL_MACHINE, regPath, "ocl_2_0_enabled", emulatorVal, 16 );

    if(retVal)
    {
        if(_stricmp(emulatorVal, "true") == 0)
        {
            return true;
        }
    }
#endif
    return false;
}

string ReadFileContents(const string& filePath)
{
    ifstream stream(filePath.c_str());
    if (!stream.good())
    {
        return "";
    }

    stringstream sstr;
    sstr << stream.rdbuf();
    stream.close();
    return sstr.str();
}

unsigned int ReadBinaryFileContents(const string& filePath, char** bufferPtr, unsigned int sizeToRead)
{
    ifstream stream(filePath.c_str(), std::ios::binary);
    if (stream)
    {
        // get length of filePath:
        stream.seekg (0, stream.end);
        unsigned int length = stream.tellg();
        stream.seekg (0, stream.beg);

        if (sizeToRead > length)
        {
            throw string("File " + filePath + " contains only " + stringify(length) + " bytes\nCan't read " + stringify(sizeToRead));
        }

        unsigned int size = sizeToRead == 0 ? length : sizeToRead;
        char * buffer = new char [size];

        stream.read (buffer, size);

        if (!stream)
        {
            throw string("Error: only " + stringify(stream.gcount()) + " bytes could be read from file " + filePath);
        }
        stream.close();

        *bufferPtr = buffer;
        return size;
    }

    return 0;
}

void WriteContentToFile(const string& content, const string& filePath)
{
    ofstream stream(filePath.c_str());
    if (!stream.good())
    {
        throw string("Failed to open file: " + filePath);
    }

    stream << content;
    stream.close();
}

void WriteBinaryContentToFile(const char* content, unsigned int size, const string& filePath)
{
    ofstream stream(filePath.c_str(), std::ios::binary);
    if (!stream.good())
    {
        throw string("Failed to open file: " + filePath);
    }

    stream.write(content, size);
    if (stream.fail() || stream.bad())
    {
        throw string("Failed to write to file: " + filePath);
    }
    stream.close();
}

OclMemObjectType getSimplifiedMemoryObjectType(const cl_mem_object_type MemObjectType)
{
    switch(MemObjectType)
    {
    case CL_MEM_OBJECT_BUFFER:
        return OCL_BUFFER;
    case CL_MEM_OBJECT_IMAGE2D:
    case CL_MEM_OBJECT_IMAGE3D:
    case CL_MEM_OBJECT_IMAGE2D_ARRAY:
    case CL_MEM_OBJECT_IMAGE1D:
    case CL_MEM_OBJECT_IMAGE1D_ARRAY:
    case CL_MEM_OBJECT_IMAGE1D_BUFFER:
        return OCL_IMAGE;
    case CL_MEM_OBJECT_PIPE:
        return OCL_PIPE;
    default:
        return OCL_UNKNOWN;
    }
}

