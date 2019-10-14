// INTEL CONFIDENTIAL
//
// Copyright 2007-2018 Intel Corporation.
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
#ifdef WINDOWS_ONECORE
    assert(0 && "Affinity is not supported on universal windows platform");
#else
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
#endif
}

void clGetThreadAffinityMask(affinityMask_t* mask, threadid_t tid)
{
#ifdef WINDOWS_ONECORE
    assert(0 && "Affinity is not supported on universal windows platform");
#else
    // Currently not supported on Windows
    *mask = -1;
#endif
}

void clSetThreadAffinityToCore(unsigned int core, threadid_t tid)
{
#ifdef WINDOWS_ONECORE
    assert(0 && "Affinity is not supported on universal windows platform");
#else
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
#endif
}

void clResetThreadAffinityMask(threadid_t tid)
{
#ifdef WINDOWS_ONECORE
    assert(0 && "Affinity is not supported on universal windows platform");
#else
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
#endif
}

bool clTranslateAffinityMask(affinityMask_t* mask, unsigned int* IDs, size_t len)
{
#ifdef WINDOWS_ONECORE
    assert(0 && "Affinity is not supported on universal windows platform");
    return false;
#else
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
#endif
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

#ifdef USE_NUMA
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
        static __thread bool isLocalAllocSet = false;
        if (!isLocalAllocSet)
        {
            numa_set_localalloc();
            isLocalallocSet = true;
        }
    }
#else
    bool clIsNumaAvailable()
    {    
        return false;
    }

    void clNUMASetLocalNodeAlloc()
    {
    }
    
#endif // USE_NUMA

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
    if (nullptr == pclImageFormat)
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
    if (nullptr == pclImageFormat)
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
        
        int shift = internal_clz( mantissa ) - 8;
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
int internal_clz(unsigned int pattern)
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
int internal_clz(unsigned int pattern)
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
            nullptr,                   // lpType
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

