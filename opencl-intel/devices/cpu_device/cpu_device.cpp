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

///////////////////////////////////////////////////////////
//  cpu_device.cpp
///////////////////////////////////////////////////////////

#include "stdafx.h"

#include "cpu_device.h"
#include "program_service.h"
#include "memory_allocator.h"
#include "task_dispatcher.h"
#include "cpu_logger.h"
#include "builtin_kernels.h"
#include "cl_shared_ptr.hpp"
#include <buildversion.h>
#include <CL/cl_ext.h>
#include "CL/cl_2_0.h"
#include <clang_device_info.h>
#include <cl_sys_info.h>
#include <cpu_dev_limits.h>
#include <cl_sys_defines.h>
#include <cl_cpu_detect.h>
#include <cl_shutdown.h>
#ifdef __INCLUDE_MKL__
#include <mkl_builtins.h>
#endif
#if defined (__GNUC__) && !(__INTEL_COMPILER)  && !(_WIN32)
#include "hw_utils.h"
#endif
#if !defined(__ANDROID__)
  #define __DOUBLE_ENABLED__
#endif

using namespace Intel::OpenCL::CPUDevice;

USE_SHUTDOWN_HANDLER(CPUDevice::WaitUntilShutdown);

char clCPUDEVICE_CFG_PATH[MAX_PATH];

#if defined(_M_X64) || defined(__x86_64__)
    #define MEMORY_LIMIT (TotalPhysicalSize())
#else
    // When running on 32bit application on 64bit OS, the total physical size might exceed virtual evailable memory 
    #define MEMORY_LIMIT    ( MIN(TotalPhysicalSize(), TotalVirtualSize()) )
#endif
#define MAX_MEM_ALLOC_SIZE ( MAX(128*1024*1024, MEMORY_LIMIT/4) )
#define GLOBAL_MEM_SIZE (MEMORY_LIMIT)

using namespace Intel::OpenCL::CPUDevice;
using namespace Intel::OpenCL::BuiltInKernels;

const char* Intel::OpenCL::CPUDevice::VENDOR_STRING = "Intel(R) Corporation";

volatile bool CPUDevice::m_bDeviceIsRunning = false;

// We put it here, because just here all the required macros are defined.
#include "ocl_supported_extensions.h"

struct Intel::OpenCL::ClangFE::CLANG_DEV_INFO *GetCPUDevInfo(CPUDeviceConfig& config)
{
    static struct Intel::OpenCL::ClangFE::CLANG_DEV_INFO CPUDevInfo = {NULL, 1, 1, 0};
    if (NULL == CPUDevInfo.sExtensionStrings)
    {
        if (CPUDetect::GetInstance()->IsProcessorType(PT_ATOM))
        {
            if (config.GetOpenCLVersion() == OPENCL_VERSION_1_2)
            {
                CPUDevInfo.sExtensionStrings = OCL_SUPPORTED_EXTENSIONS_ATOM_1_2;
            }
            else
            {
                CPUDevInfo.sExtensionStrings = OCL_SUPPORTED_EXTENSIONS_ATOM_2_0;
            }
        }
        else
        {
            if (config.GetOpenCLVersion() == OPENCL_VERSION_1_2)
            {
                CPUDevInfo.sExtensionStrings = OCL_SUPPORTED_EXTENSIONS_1_2;
            }
            else
            {
                CPUDevInfo.sExtensionStrings = OCL_SUPPORTED_EXTENSIONS_2_0;
            }
        }
    }
    return &CPUDevInfo;
}
// explicit instantiation of SharedPtrBase classes (needed for gcc ver. 4.3 on SLES):
template class Intel::OpenCL::Utils::SharedPtrBase<Intel::OpenCL::DeviceCommands::KernelCommand>;
template class Intel::OpenCL::Utils::SharedPtrBase<Intel::OpenCL::DeviceCommands::DeviceCommand>;
template class Intel::OpenCL::Utils::SharedPtrBase<ITaskList>;
template class Intel::OpenCL::Utils::SharedPtrBase<ITaskGroup>;

static const size_t CPU_MAX_WORK_ITEM_SIZES[CPU_MAX_WORK_ITEM_DIMENSIONS] =
    {
    CPU_MAX_WORK_GROUP_SIZE,
    CPU_MAX_WORK_GROUP_SIZE,
    CPU_MAX_WORK_GROUP_SIZE
    };

static const cl_device_partition_property CPU_SUPPORTED_FISSION_MODES[] = 
    {
        CL_DEVICE_PARTITION_BY_COUNTS,
        CL_DEVICE_PARTITION_EQUALLY,
        CL_DEVICE_PARTITION_BY_NAMES_INTEL
    };

typedef enum 
{
    CPU_DEVICE_DATA_TYPE_CHAR  = 0,
    CPU_DEVICE_DATA_TYPE_SHORT,
    CPU_DEVICE_DATA_TYPE_INT,
    CPU_DEVICE_DATA_TYPE_LONG, 
    CPU_DEVICE_DATA_TYPE_FLOAT,
    CPU_DEVICE_DATA_TYPE_DOUBLE,
    CPU_DEVICE_DATA_TYPE_HALF
} CPUDeviceDataTypes;

static const cl_uint CPU_DEVICE_NATIVE_VECTOR_WIDTH_SSE42[] = {16, 8, 4, 2, 4, 2, 0};  //SSE4.2 has 16 byte (XMM) registers
static const cl_uint CPU_DEVICE_NATIVE_VECTOR_WIDTH_AVX[]   = {16, 8, 4, 2, 8, 4, 0};  //AVX supports 32 byte (YMM) registers only for floats and doubles
static const cl_uint CPU_DEVICE_NATIVE_VECTOR_WIDTH_AVX2[]  = {32, 16, 8, 4, 8, 4, 0}; //AVX2 has a full set of 32 byte (YMM) registers

extern "C" const char* clDevErr2Txt(cl_dev_err_code errorCode)
{
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

typedef struct _cl_dev_internal_cmd_list
{
    ITaskList*                    cmd_list;
    cl_dev_internal_subdevice_id* subdevice_id;
} cl_dev_internal_cmd_list;

CPUDevice::CPUDevice(cl_uint uiDevId, IOCLFrameworkCallbacks *devCallbacks, IOCLDevLogDescriptor *logDesc): 
    m_pProgramService(NULL),
    m_pMemoryAllocator(NULL),
    m_pTaskDispatcher(NULL),
    m_pCPUDeviceConfig(NULL), 
    m_pFrameworkCallBacks(devCallbacks), 
    m_uiCpuId(uiDevId),
    m_pLogDescriptor(logDesc), 
    m_iLogHandle (0),
    m_defaultCommandList(NULL),
    m_numCores(0),
    m_pComputeUnitMap(NULL)
{
    m_bDeviceIsRunning = true;
}

cl_dev_err_code CPUDevice::Init()
{
    cl_dev_err_code ret = CL_DEV_SUCCESS;
    if ( NULL != m_pLogDescriptor )
    {
        ret = (cl_dev_err_code)m_pLogDescriptor->clLogCreateClient(m_uiCpuId, "CPU Device", &m_iLogHandle);
        if(CL_DEV_SUCCESS != ret)
        {
            return CL_DEV_ERROR_FAIL;
        }
    }

    ret = m_backendWrapper.Init(NULL);
    if (CL_DEV_FAILED(ret))
    {
        return CL_DEV_ERROR_FAIL;
    }

    m_pCPUDeviceConfig = new CPUDeviceConfig();
    m_pCPUDeviceConfig->Initialize(clCPUDEVICE_CFG_PATH);

    // Enable VTune source level profiling
    GetCPUDevInfo(*m_pCPUDeviceConfig)->bEnableSourceLevelProfiling = m_pCPUDeviceConfig->UseVTune();

    CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("CreateDevice function enter"));

    m_pProgramService = new ProgramService(m_uiCpuId, 
                                           m_pFrameworkCallBacks, 
                                           m_pLogDescriptor, 
                                           m_pCPUDeviceConfig,
                                           m_backendWrapper.GetBackendFactory());
    ret = m_pProgramService->Init(this);
    if (CL_DEV_SUCCESS != ret)
    {
        return CL_DEV_ERROR_FAIL;
    }

    ret = QueryHWInfo();
    if (CL_DEV_SUCCESS != ret)
    {
        return CL_DEV_ERROR_FAIL;
    }

    m_pMemoryAllocator = new MemoryAllocator(m_uiCpuId, m_pLogDescriptor, GLOBAL_MEM_SIZE, m_pProgramService->GetImageService());
    m_pTaskDispatcher = new TaskDispatcher(m_uiCpuId, m_pFrameworkCallBacks, m_pProgramService, m_pMemoryAllocator, m_pLogDescriptor, m_pCPUDeviceConfig, this);
    if ( (NULL == m_pMemoryAllocator) || (NULL == m_pTaskDispatcher) )
    {
        return CL_DEV_OUT_OF_MEMORY;
    }
    
    m_pTaskDispatcher->setWgContextPool(&m_wgContextPool);
    return m_pTaskDispatcher->init();
}

cl_dev_err_code CPUDevice::QueryHWInfo()
{
    m_numCores        = GetNumberOfProcessors();
    m_pComputeUnitMap = new unsigned int[m_numCores];
    if (NULL == m_pComputeUnitMap)
    {
      return CL_DEV_OUT_OF_MEMORY;
    }
    //Todo: m_pComputeUnitScoreboard.reserve(m_numCores);

    m_pCoreToThread.resize(m_numCores);
    m_pCoreInUse.resize(m_numCores);

    //Todo: calculate the real map here
    for (unsigned int i = 0; i < m_numCores; i++)
    {
        m_pComputeUnitMap[i] = i;
        m_pCoreToThread[i]   = INVALID_THREAD_HANDLE;
        m_pCoreInUse[i]      = false;
    }
#ifndef WIN32
    //For Linux, respect the process affinity mask in determining which cores to run on
    affinityMask_t myParentMask;
    threadid_t     myParentId = clMyParentThreadId();
    clGetThreadAffinityMask(&myParentMask, myParentId);
    clTranslateAffinityMask(&myParentMask, m_pComputeUnitMap, m_numCores);
#endif
    return CL_DEV_SUCCESS;
}

bool CPUDevice::AcquireComputeUnits(unsigned int* which, unsigned int how_many)
{
    if ((NULL == which) || (0 == how_many))
    {
        return true;
    }
    Intel::OpenCL::Utils::OclAutoMutex CS(&m_ComputeUnitScoreboardMutex);
    for (unsigned int i = 0; i < how_many; ++i)
    {
        if (m_pCoreInUse[which[i]])
        {
            //Failed, roll back
            for (unsigned int j = 0; j < i; ++j)
            {
                m_pCoreInUse[which[j]] = false;
            }
            return false;
        }
        else
        {
            m_pCoreInUse[which[i]] = true;
        }
    }
    return true;
}

void CPUDevice::ReleaseComputeUnits(unsigned int* which, unsigned int how_many)
{
    if (NULL == which)
    {
        return;
    }
    Intel::OpenCL::Utils::OclAutoMutex CS(&m_ComputeUnitScoreboardMutex);
    for (unsigned int i = 0; i < how_many; ++i)
    {
        m_pCoreInUse[which[i]] = false;
    }
}

void CPUDevice::NotifyAffinity(threadid_t tid, unsigned int core_index)
{
    Intel::OpenCL::Utils::OclAutoMutex CS(&m_ComputeUnitScoreboardMutex);

    assert(core_index < m_numCores && "Access outside core map size");

    threadid_t   other_tid        = m_pCoreToThread[core_index];
    int          my_prev_core_idx = m_threadToCore[tid];
    
    // The other tid is valid if there was another thread pinned to the core I want to move to
    bool         other_valid      = (other_tid != INVALID_THREAD_HANDLE) && (other_tid != tid);
    // Either I'm not relocating another thread, or I am. If I am, make sure that I'm relocating the thread which resides on the core I want to move to
    // This assertion might fail if there's a bug that makes m_threadToCore desynch from m_pCoreToThread
    bool bMapsAreConsistent = !other_valid || (int)core_index == m_threadToCore[other_tid];
    assert(bMapsAreConsistent && "m_threadToCore and m_pCoreToThread mismatch");
    if (!bMapsAreConsistent)
    {
        return;
    }

    //Update map with regard to myself
    m_threadToCore[tid]         = core_index;
    m_pCoreToThread[core_index] = tid;

    //Set the caller's affinity as requested
    clSetThreadAffinityToCore(m_pComputeUnitMap[core_index], tid);

    if (other_valid)
    {
        //Need to relocate the other thread to my previous core
        m_threadToCore[other_tid] = my_prev_core_idx;
        if ( -1 != my_prev_core_idx )
        {
            m_pCoreToThread[my_prev_core_idx] = other_tid;
            clSetThreadAffinityToCore(m_pComputeUnitMap[my_prev_core_idx], other_tid);
        }
        else
        {
            // If current thread was not affinitize reset others thread affinity mask
            // TODO: Need to save the original mask of the thread and restore it in this case
            clResetThreadAffinityMask(other_tid);
        }
    }
    else
    {
        m_pCoreToThread[my_prev_core_idx] = INVALID_THREAD_HANDLE;      
    }

}

cl_uint GetNativeVectorWidth(CPUDeviceDataTypes dataType)
{
    const bool     avx1Support   = CPUDetect::GetInstance()->IsFeatureSupported(CFS_AVX10);
    const bool     avx2Support   = CPUDetect::GetInstance()->IsFeatureSupported(CFS_AVX20);
    const cl_uint* pVectorWidths = CPU_DEVICE_NATIVE_VECTOR_WIDTH_SSE42;

    if (avx1Support)
    {
        pVectorWidths = CPU_DEVICE_NATIVE_VECTOR_WIDTH_AVX;
    }
    if (avx2Support)
    {
        pVectorWidths = CPU_DEVICE_NATIVE_VECTOR_WIDTH_AVX2;
    }

    return pVectorWidths[(int)dataType];
}

CPUDeviceDataTypes NativeVectorToCPUDeviceDataType(cl_device_info native_type)
{
    // For OpenCL 1.2, the CL_DEVICE_NATIVE_VECTOR_WIDTH_XXX defines are consecutive and start at CL_DEVICE_NATIVE_VECTOR_WIDTH_CHAR
    // Re-implement this function if that ceases being the case
    cl_uint dataTypeOffset = native_type - CL_DEVICE_NATIVE_VECTOR_WIDTH_CHAR;

    // The CPUDeviceDataTypes enum starts at 0 and follows the same order of types
    return (CPUDeviceDataTypes)dataTypeOffset;
}

CPUDevice::~CPUDevice()
{
    m_bDeviceIsRunning = false;
}

void CPUDevice::WaitUntilShutdown()
{
    //
    // Actually we need to check here that TBB has shut down but because TBB may handle other
    // activities also that are still alive (ex. compilation services through framework proxy)
    // we cannot do here real wait for threads
    //
    while (m_bDeviceIsRunning)
    {
        hw_pause();
    }
}

// ---------------------------------------
// Public functions / Device entry points
extern "C" cl_dev_err_code clDevCreateDeviceInstance(  cl_uint      dev_id,
                                   IOCLFrameworkCallbacks   *pDevCallBacks,
                                   IOCLDevLogDescriptor     *pLogDesc,
                                   IOCLDeviceAgent*         *pDevice
                                   )
{
    if(NULL == pDevCallBacks || NULL == pDevice)
    {
        return CL_DEV_INVALID_OPERATION;
    }

    CPUDevice *pNewDevice = new CPUDevice(dev_id, pDevCallBacks, pLogDesc);
    if ( NULL == pNewDevice )
    {
        return CL_DEV_OUT_OF_MEMORY;
    }

    cl_dev_err_code rc = pNewDevice->Init();
    if ( CL_DEV_FAILED(rc) )
    {
        pNewDevice->clDevCloseDevice();
        return rc;
    }
    *pDevice = pNewDevice;
    return CL_DEV_SUCCESS;
}

// Device entry points
//Device Information function prototypes
//
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
        paramValSize_ret        Returns the actual size in bytes of data being queried by paramVal. If paramValSize_ret is NULL, it is ignored
    Returns
        CL_DEV_SUCCESS          If functions is executed successfully.
        CL_DEV_INVALID_VALUE    If param_name is not one of the supported values or if size in bytes specified by paramValSize is < size of return type as specified in OCL spec. table 4.3 and paramVal is not a NULL value
**************************************************************************************************************************/
cl_dev_err_code CPUDevice::clDevGetDeviceInfo(unsigned int IN dev_id, cl_device_info IN param, size_t IN valSize, void* OUT paramVal,
                size_t* OUT paramValSizeRet)
{
    size_t  internalRetunedValueSize = valSize;
    size_t  *pinternalRetunedValueSize;
    unsigned int viCPUInfo[4] = {(unsigned int)-1};

    //if OUT paramValSize_ret is NULL it should be ignopred
    if(paramValSizeRet)
    {
        pinternalRetunedValueSize = paramValSizeRet;
    }
    else
    {
        pinternalRetunedValueSize = &internalRetunedValueSize;
    }

    static const char sOpenCL12Str[] = "OpenCL 1.2 ", sOpenCL20Str[] = "OpenCL 2.0 ";
    static const char sOpenCLC12Str[] = "OpenCL C 1.2 ", sOpenCLC20Str[] = "OpenCL C 2.0 ";
    switch (param)
    {
        case( CL_DEVICE_TYPE):
        {
            *pinternalRetunedValueSize = sizeof(cl_device_type);
            if(NULL != paramVal && valSize < *pinternalRetunedValueSize)
            {
                return CL_DEV_INVALID_VALUE;
            }
            //if OUT paramVal is NULL it should be ignored
            if(NULL != paramVal)
            {
                *(cl_device_type*)paramVal = (cl_device_type)CL_DEVICE_TYPE_CPU;
            }
            return CL_DEV_SUCCESS;
        }

        case( CL_DEVICE_VENDOR_ID):
        {
            *pinternalRetunedValueSize = sizeof(cl_uint);
            if(NULL != paramVal && valSize < *pinternalRetunedValueSize)
            {
                return CL_DEV_INVALID_VALUE;
            }
            //if OUT paramVal is NULL it should be ignored
            if(NULL != paramVal)
            {
                    *(cl_uint*)paramVal = 0x8086;
            }
            return CL_DEV_SUCCESS;
        }

        case( CL_DEVICE_PARTITION_MAX_SUB_DEVICES):
        {
            *pinternalRetunedValueSize = sizeof(cl_uint);
            if(NULL != paramVal && valSize < *pinternalRetunedValueSize)
            {
                return CL_DEV_INVALID_VALUE;
            }
            //if OUT paramVal is NULL it should be ignored
            if(NULL != paramVal)
            {
#ifndef __ANDROID__
                *(cl_uint*)paramVal = GetNumberOfProcessors();
#else
                // Disable device partition on android
                *(cl_uint*)paramVal = 0;
#endif
            }
            return CL_DEV_SUCCESS;
        }

        case( CL_DEVICE_MAX_COMPUTE_UNITS):
        {
            *pinternalRetunedValueSize = sizeof(cl_uint);
            if(NULL != paramVal && valSize < *pinternalRetunedValueSize)
            {
                return CL_DEV_INVALID_VALUE;
            }
            //if OUT paramVal is NULL it should be ignored
            if(NULL != paramVal)
            {
                *(cl_uint*)paramVal = GetNumberOfProcessors();
            }
            return CL_DEV_SUCCESS;
        }

        case( CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR):   //FALL THROUGH
        case( CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT):  //FALL THROUGH
        case( CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT):  //FALL THROUGH
        case( CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT):    //FALL THROUGH
        case( CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG):   //FALL THROUGH
        {
            //For all supported types, we currently prefer scalars so the vectorizer doesn't have to scalarize
            *pinternalRetunedValueSize = sizeof(cl_uint);
            if(NULL != paramVal && valSize < *pinternalRetunedValueSize)
            {
                return CL_DEV_INVALID_VALUE;
            }
            //if OUT paramVal is NULL it should be ignored
            if(NULL != paramVal)
            {
                *(cl_uint*)paramVal = 1;
            }
            return CL_DEV_SUCCESS;
        }
        case( CL_DEVICE_PREFERRED_VECTOR_WIDTH_HALF ):
        {
            //We prefer the users won't use half
            *pinternalRetunedValueSize = sizeof(cl_uint);
            if(NULL != paramVal && valSize < *pinternalRetunedValueSize)
            {
                return CL_DEV_INVALID_VALUE;
            }
            //if OUT paramVal is NULL it should be ignored
            if(NULL != paramVal)
            {
                *(cl_uint*)paramVal = 0;
            }
            return CL_DEV_SUCCESS;
        }
        case( CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE): //Keeping double separate to allow control via the __DOUBLE_ENABLED__ macro
        {
            //For all supported types, we currently prefer scalars so the vectorizer doesn't have to scalarize
            *pinternalRetunedValueSize = sizeof(cl_uint);
            if(NULL != paramVal && valSize < *pinternalRetunedValueSize)
            {
                return CL_DEV_INVALID_VALUE;
            }
            //if OUT paramVal is NULL it should be ignored
            if(NULL != paramVal)
            {
#ifdef __DOUBLE_ENABLED__
                if (CPUDetect::GetInstance()->IsProcessorType(PT_ATOM))
                {
                    *(cl_uint*)paramVal = 0;
                }
                else
                {
                    *(cl_uint*)paramVal = 1;
                }
#else
                *(cl_uint*)paramVal = 0;
#endif
            }
            return CL_DEV_SUCCESS;
        }

        case( CL_DEVICE_NATIVE_VECTOR_WIDTH_CHAR ): //FALL THROUGH
        case( CL_DEVICE_NATIVE_VECTOR_WIDTH_SHORT): //FALL THROUGH
        case( CL_DEVICE_NATIVE_VECTOR_WIDTH_INT):   //FALL THROUGH
        case( CL_DEVICE_NATIVE_VECTOR_WIDTH_FLOAT): //FALL THROUGH
        case( CL_DEVICE_NATIVE_VECTOR_WIDTH_LONG):  //FALL THROUGH
        case( CL_DEVICE_NATIVE_VECTOR_WIDTH_HALF ): //FALL THROUGH
        {
            *pinternalRetunedValueSize = sizeof(cl_uint);
            if(NULL != paramVal && valSize < *pinternalRetunedValueSize)
            {
                return CL_DEV_INVALID_VALUE;
            }
            //if OUT paramVal is NULL it should be ignored
            if(NULL != paramVal)
            {
               CPUDeviceDataTypes dataType = NativeVectorToCPUDeviceDataType(param);
                *(cl_uint*)paramVal        = GetNativeVectorWidth(dataType);
            }
            return CL_DEV_SUCCESS;
        }
        case( CL_DEVICE_NATIVE_VECTOR_WIDTH_DOUBLE): //Keeping double separate to allow control via the __DOUBLE_ENABLED__ macro
        {
            *pinternalRetunedValueSize = sizeof(cl_uint);
            if(NULL != paramVal && valSize < *pinternalRetunedValueSize)
            {
                return CL_DEV_INVALID_VALUE;
            }
            //if OUT paramVal is NULL it should be ignored
            if(NULL != paramVal)
            {
#ifdef __DOUBLE_ENABLED__
                if (CPUDetect::GetInstance()->IsProcessorType(PT_ATOM))
                {
                    *(cl_uint*)paramVal = 0;
                }
                else
                {
                    *(cl_uint*)paramVal = GetNativeVectorWidth(CPU_DEVICE_DATA_TYPE_DOUBLE);
                }
#else
                *(cl_uint*)paramVal = 0;
#endif
            }
            return CL_DEV_SUCCESS;
        }

        case( CL_DEVICE_IMAGE_SUPPORT):
            {
                *pinternalRetunedValueSize = sizeof(cl_bool);
                if(NULL != paramVal && valSize < *pinternalRetunedValueSize)
                {
                    return CL_DEV_INVALID_VALUE;
                }
                //if OUT paramVal is NULL it should be ignored
                if(NULL != paramVal)
                {
                    *(cl_bool*)paramVal = CL_TRUE;
                }
                return CL_DEV_SUCCESS;
            }

        case( CL_DEVICE_MIN_DATA_TYPE_ALIGN_SIZE):
        {
            *pinternalRetunedValueSize = sizeof(cl_uint);
            if(NULL != paramVal && valSize < *pinternalRetunedValueSize)
            {
                return CL_DEV_INVALID_VALUE;
            }
            //if OUT paramVal is NULL it should be ignored
            if(NULL != paramVal)
            {
                *(cl_uint*)paramVal = sizeof(cl_long16);
            }
            return CL_DEV_SUCCESS;
        }
        case( CL_DEVICE_SINGLE_FP_CONFIG):  
        {
            cl_device_fp_config fpConfig = CL_FP_ROUND_TO_NEAREST | CL_FP_INF_NAN | CL_FP_DENORM;
            *pinternalRetunedValueSize = sizeof(cl_device_fp_config);
            if(NULL != paramVal && valSize < *pinternalRetunedValueSize)
            {
                return CL_DEV_INVALID_VALUE;
            }
            //if OUT paramVal is NULL it should be ignored
            if(NULL != paramVal)
            {
                *(cl_device_fp_config*)paramVal = fpConfig;
            }
            return CL_DEV_SUCCESS;
        }
        case(CL_DEVICE_DOUBLE_FP_CONFIG):
        {
#ifdef __DOUBLE_ENABLED__
            cl_device_fp_config fpConfig = 0;
            if (!CPUDetect::GetInstance()->IsProcessorType(PT_ATOM))
            {
                fpConfig = CL_FP_ROUND_TO_NEAREST | CL_FP_INF_NAN | CL_FP_DENORM | CL_FP_FMA |  CL_FP_ROUND_TO_ZERO |  CL_FP_ROUND_TO_INF;
            }
#else
            cl_device_fp_config fpConfig = 0;
#endif
            *pinternalRetunedValueSize = sizeof(cl_device_fp_config);
            if(NULL != paramVal && valSize < *pinternalRetunedValueSize)
            {
                return CL_DEV_INVALID_VALUE;
            }
            //if OUT paramVal is NULL it should be ignored
            if(NULL != paramVal)
            {
                *(cl_device_fp_config*)paramVal = fpConfig;
            }
            return CL_DEV_SUCCESS;
        }
        case( CL_DEVICE_IMAGE2D_MAX_WIDTH): // FALL THROUGH
        case( CL_DEVICE_IMAGE2D_MAX_HEIGHT):// FALL THROUGH
            {
                *pinternalRetunedValueSize = sizeof(size_t);
                if(NULL != paramVal && valSize < *pinternalRetunedValueSize)
                {
                    return CL_DEV_INVALID_VALUE;
                }
                //if OUT paramVal is NULL it should be ignored
                if(NULL != paramVal)
                {
                    *(size_t*)paramVal = CPU_IMAGE2D_MAX_DIM_SIZE;
                }
                return CL_DEV_SUCCESS;
            }
        case( CL_DEVICE_IMAGE3D_MAX_WIDTH): // FALL THROUGH
        case( CL_DEVICE_IMAGE3D_MAX_HEIGHT):// FALL THROUGH
        case( CL_DEVICE_IMAGE3D_MAX_DEPTH):
        {               
            *pinternalRetunedValueSize = sizeof(size_t);
            if(NULL != paramVal && valSize < *pinternalRetunedValueSize)
            {
                return CL_DEV_INVALID_VALUE;
            }
            //if OUT paramVal is NULL it should be ignored
            if(NULL != paramVal)
            {
                *(size_t*)paramVal = CPU_IMAGE3D_MAX_DIM_SIZE;
            }
            return CL_DEV_SUCCESS;
        }

        case( CL_DEVICE_MAX_PARAMETER_SIZE):
        {
            *pinternalRetunedValueSize = sizeof(size_t);
            if(NULL != paramVal && valSize < *pinternalRetunedValueSize)
            {
                return CL_DEV_INVALID_VALUE;
            }
            //if OUT paramVal is NULL it should be ignored
            if(NULL != paramVal)
            {
                *(size_t*)paramVal = CPU_MAX_PARAMETER_SIZE;
            }
            return CL_DEV_SUCCESS;
        }
        case( CL_DEVICE_MAX_SAMPLERS):
        {               
            *pinternalRetunedValueSize = sizeof(cl_uint);
            if(NULL != paramVal && valSize < *pinternalRetunedValueSize)
            {
                return CL_DEV_INVALID_VALUE;
            }
            //if OUT paramVal is NULL it should be ignored
            if(NULL != paramVal)
            {
                *(cl_uint*)paramVal = CPU_MAX_SAMPLERS;
            }
            return CL_DEV_SUCCESS;
        }

        case( CL_DEVICE_MAX_READ_IMAGE_ARGS):
        {               
            *pinternalRetunedValueSize = sizeof(cl_uint);
            if(NULL != paramVal && valSize < *pinternalRetunedValueSize)
            {
                return CL_DEV_INVALID_VALUE;
            }
            //if OUT paramVal is NULL it should be ignored
            if(NULL != paramVal)
            {
                *(cl_uint*)paramVal = CPU_MAX_READ_IMAGE_ARGS;
            }
            return CL_DEV_SUCCESS;
        }
        case( CL_DEVICE_MAX_WRITE_IMAGE_ARGS):
        {               
            *pinternalRetunedValueSize = sizeof(cl_uint);
            if(NULL != paramVal && valSize < *pinternalRetunedValueSize)
            {
                return CL_DEV_INVALID_VALUE;
            }
            //if OUT paramVal is NULL it should be ignored
            if(NULL != paramVal)
            {
                *(cl_uint*)paramVal = CPU_MAX_WRITE_IMAGE_ARGS;
            }
            return CL_DEV_SUCCESS;
        }
        case( CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE):
        {               
            *pinternalRetunedValueSize = sizeof(cl_ulong);
            if(NULL != paramVal && valSize < *pinternalRetunedValueSize)
            {
                return CL_DEV_INVALID_VALUE;
            }
            //if OUT paramVal is NULL it should be ignored
            if(NULL != paramVal)
            {
                *(cl_ulong*)paramVal = CPU_MAX_CONSTANT_BUFFER_SIZE;
            }
            return CL_DEV_SUCCESS;
        }

        case( CL_DEVICE_MAX_CONSTANT_ARGS ):
        {               
            *pinternalRetunedValueSize = sizeof(cl_uint);
            if(NULL != paramVal && valSize < *pinternalRetunedValueSize)
            {
                return CL_DEV_INVALID_VALUE;
            }
            //if OUT paramVal is NULL it should be ignored
            if(NULL != paramVal)
            {
                *(cl_uint*)paramVal = CPU_MAX_CONSTANT_ARGS;
            }
            return CL_DEV_SUCCESS;
        }
        case( CL_DEVICE_MEM_BASE_ADDR_ALIGN):
        {               
            *pinternalRetunedValueSize = sizeof(cl_uint);
            if(NULL != paramVal && valSize < *pinternalRetunedValueSize)
            {
                return CL_DEV_INVALID_VALUE;
            }
            //if OUT paramVal is NULL it should be ignored
            if(NULL != paramVal)
            {
                *(cl_uint*)paramVal = CPU_MEM_BASE_ADDR_ALIGN;
            }
            return CL_DEV_SUCCESS;
        }
        case( CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS):
        {               
            *pinternalRetunedValueSize = sizeof(cl_uint);
            if(NULL != paramVal && valSize < *pinternalRetunedValueSize)
            {
                return CL_DEV_INVALID_VALUE;
            }
            //if OUT paramVal is NULL it should be ignored
            if(NULL != paramVal)
            {
                *(cl_uint*)paramVal = CPU_MAX_WORK_ITEM_DIMENSIONS;
            }
            return CL_DEV_SUCCESS;
        }
        case( CL_DEVICE_MAX_WORK_GROUP_SIZE):
        {               
            *pinternalRetunedValueSize = sizeof(size_t);
            if(NULL != paramVal && valSize < *pinternalRetunedValueSize)
            {
                return CL_DEV_INVALID_VALUE;
            }
            //if OUT paramVal is NULL it should be ignored
            if(NULL != paramVal)
            {
                *(size_t*)paramVal = CPU_MAX_WORK_GROUP_SIZE;
            }
            return CL_DEV_SUCCESS;
        }
        case( CL_DEVICE_MAX_WORK_ITEM_SIZES):
        {               
            *pinternalRetunedValueSize = CPU_MAX_WORK_ITEM_DIMENSIONS * sizeof(size_t);
            if(NULL != paramVal && valSize < *pinternalRetunedValueSize)
            {
                return CL_DEV_INVALID_VALUE;
            }
            //if OUT paramVal is NULL it should be ignored
            if(NULL != paramVal)
            {
                memcpy(paramVal, CPU_MAX_WORK_ITEM_SIZES, *pinternalRetunedValueSize);
            }
            return CL_DEV_SUCCESS;
        }

        case( CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE):
        case CL_DEVICE_IMAGE_PITCH_ALIGNMENT:    // BE recommends that these two queries will also return cache line size
        case CL_DEVICE_IMAGE_BASE_ADDRESS_ALIGNMENT:
        {
            *pinternalRetunedValueSize = sizeof(cl_uint);
            if(NULL != paramVal && valSize < *pinternalRetunedValueSize)
            {
                return CL_DEV_INVALID_VALUE;
            }
            //if OUT paramVal is NULL it should be ignored
            if(NULL != paramVal)
            {

                //Get the Cache info
                CPUID(viCPUInfo, 0x80000006);
                *(cl_uint*)paramVal = (cl_uint)viCPUInfo[2] & 0xff;
            }
            return CL_DEV_SUCCESS;
        }
        case( CL_DEVICE_GLOBAL_MEM_CACHE_SIZE):
        {
            *pinternalRetunedValueSize = sizeof(cl_ulong);
            if(NULL != paramVal && valSize < *pinternalRetunedValueSize)
            {
                return CL_DEV_INVALID_VALUE;
            }
            //if OUT paramVal is NULL it should be ignored
            if(NULL != paramVal)
            {

                //Get the Cache info
                CPUID(viCPUInfo, 0x80000006);
                *(cl_ulong*)paramVal = (cl_ulong)(((viCPUInfo[2] >> 16) & 0xffff)*1024);
            }
            return CL_DEV_SUCCESS;
        }
        case( CL_DEVICE_LOCAL_MEM_SIZE):                // Consider local memory size is 24Kbyte LCL_MEM_SIZE constant
        {
            *pinternalRetunedValueSize = sizeof(cl_ulong);
            if(NULL != paramVal && valSize < *pinternalRetunedValueSize)
            {
                return CL_DEV_INVALID_VALUE;
            }
            //if OUT paramVal is NULL it should be ignored
            if(NULL != paramVal)
            {
                *(cl_ulong*)paramVal = CPU_DEV_LCL_MEM_SIZE;
            }
            return CL_DEV_SUCCESS;
        }
        case( CL_DEVICE_MAX_CLOCK_FREQUENCY):
        {
            *pinternalRetunedValueSize = sizeof(cl_uint);
            if(NULL != paramVal && valSize < *pinternalRetunedValueSize)
            {
                return CL_DEV_INVALID_VALUE;
            }
            //if OUT paramVal is NULL it should be ignored
            if(NULL != paramVal)
            {
                *(cl_uint*)paramVal = (cl_uint)MaxClockFrequency();
            }
            return CL_DEV_SUCCESS;
        }

        case( CL_DEVICE_ADDRESS_BITS):
        {
            *pinternalRetunedValueSize = sizeof(cl_uint);
            if(NULL != paramVal && valSize < *pinternalRetunedValueSize)
            {
                return CL_DEV_INVALID_VALUE;
            }
            //if OUT paramVal is NULL it should be ignored
            if(NULL != paramVal)
            {
                *(cl_uint*)paramVal = sizeof( void* ) * 8;
            }
            return CL_DEV_SUCCESS;
        }

        case( CL_DEVICE_PROFILING_TIMER_RESOLUTION):
        {
            *pinternalRetunedValueSize = sizeof(size_t);
            if(NULL != paramVal && valSize < *pinternalRetunedValueSize)
            {
                return CL_DEV_INVALID_VALUE;
            }
            //if OUT paramVal is NULL it should be ignored
            if(NULL != paramVal)
            {
                *(size_t*)paramVal = (size_t)ProfilingTimerResolution();
            }
            return CL_DEV_SUCCESS;
        }
        case( CL_DEVICE_PRINTF_BUFFER_SIZE):
        {
            *pinternalRetunedValueSize = sizeof(size_t);
            if(NULL != paramVal && valSize < *pinternalRetunedValueSize)
            {
                return CL_DEV_INVALID_VALUE;
            }
            //if OUT paramVal is NULL it should be ignored
            if(NULL != paramVal)
            {
                *(size_t*)paramVal = CPU_MAX_PRINTF_BUFFER_SIZE;
            }
            return CL_DEV_SUCCESS;
        }
        case( CL_DEVICE_GLOBAL_MEM_CACHE_TYPE):
        {
            *pinternalRetunedValueSize = sizeof(cl_device_mem_cache_type);
            if(NULL != paramVal && valSize < *pinternalRetunedValueSize)
            {
                return CL_DEV_INVALID_VALUE;
            }
            //if OUT paramVal is NULL it should be ignored
            if(NULL != paramVal)
            {
                *(cl_device_mem_cache_type*)paramVal = CL_READ_WRITE_CACHE;
            }
            return CL_DEV_SUCCESS;
        }
        case( CL_PROGRAM_BUILD_GLOBAL_VARIABLE_TOTAL_SIZE):
        {
            *pinternalRetunedValueSize = sizeof(size_t);
            if (NULL != paramVal && valSize < *pinternalRetunedValueSize)
            {
                return CL_DEV_INVALID_VALUE;
            }
            if (NULL != paramVal)
            {
                *(size_t*)paramVal = MAX_MEM_ALLOC_SIZE;
            }
            return CL_DEV_SUCCESS;
        }
        case( CL_DEVICE_MAX_MEM_ALLOC_SIZE):
        {
            *pinternalRetunedValueSize = sizeof(cl_ulong);
            if(NULL != paramVal && valSize < *pinternalRetunedValueSize)
            {
                return CL_DEV_INVALID_VALUE;
            }
            //if OUT paramVal is NULL it should be ignored
            if(NULL != paramVal)
            {
                *(cl_ulong*)paramVal = MAX_MEM_ALLOC_SIZE;
            }
            return CL_DEV_SUCCESS;
        }
        case( CL_DEVICE_GLOBAL_MEM_SIZE):
        {
            *pinternalRetunedValueSize = sizeof(cl_ulong);
            if(NULL != paramVal && valSize < *pinternalRetunedValueSize)
            {
                return CL_DEV_INVALID_VALUE;
            }
            //if OUT paramVal is NULL it should be ignored
            if(NULL != paramVal)
            {
                *(cl_ulong*)paramVal = GLOBAL_MEM_SIZE;
            }
            return CL_DEV_SUCCESS;
        }
        case( CL_DEVICE_ENDIAN_LITTLE):
        {
            *pinternalRetunedValueSize = sizeof(cl_bool);
            if(NULL != paramVal && valSize < *pinternalRetunedValueSize)
            {
                return CL_DEV_INVALID_VALUE;
            }
            //if OUT paramVal is NULL it should be ignored
            if(NULL != paramVal)
            {
                *(cl_bool*)paramVal = CL_TRUE;
            }
            return CL_DEV_SUCCESS;
        }
        case( CL_DEVICE_ERROR_CORRECTION_SUPPORT):
        {
            *pinternalRetunedValueSize = sizeof(cl_bool);
            if(NULL != paramVal && valSize < *pinternalRetunedValueSize)
            {
                return CL_DEV_INVALID_VALUE;
            }
            //if OUT paramVal is NULL it should be ignored
            if(NULL != paramVal)
            {
                *(cl_bool*)paramVal = CL_FALSE;
            }
            return CL_DEV_SUCCESS;
        }
        case( CL_DEVICE_LOCAL_MEM_TYPE):
        {
            *pinternalRetunedValueSize = sizeof(cl_device_local_mem_type);
            if(NULL != paramVal && valSize < *pinternalRetunedValueSize)
            {
                return CL_DEV_INVALID_VALUE;
            }
            //if OUT paramVal is NULL it should be ignored
            if(NULL != paramVal)
            {
                *(cl_device_local_mem_type*)paramVal = CL_GLOBAL;
            }
            return CL_DEV_SUCCESS;
        }
        case( CL_DEVICE_AVAILABLE):
        {
            *pinternalRetunedValueSize = sizeof(cl_bool);
            if(NULL != paramVal && valSize < *pinternalRetunedValueSize)
            {
                return CL_DEV_INVALID_VALUE;
            }
            //if OUT paramVal is NULL it should be ignored
            if(NULL != paramVal)
            {
                *(cl_bool*)paramVal = CL_TRUE;
            }
            return CL_DEV_SUCCESS;
        }
        case( CL_DEVICE_EXECUTION_CAPABILITIES):
        {
            *pinternalRetunedValueSize = sizeof(cl_device_exec_capabilities);
            if(NULL != paramVal && valSize < *pinternalRetunedValueSize)
            {
                return CL_DEV_INVALID_VALUE;
            }
            //if OUT paramVal is NULL it should be ignored
            if(NULL != paramVal)
            {
                cl_device_exec_capabilities execCapabilities = CL_EXEC_NATIVE_KERNEL | CL_EXEC_KERNEL;  //changed from CL_EXEC_NATIVE_FN_AS_KERNEL
                *(cl_device_exec_capabilities*)paramVal = execCapabilities;
            }
            return CL_DEV_SUCCESS;

        }
        case( CL_DEVICE_QUEUE_ON_HOST_PROPERTIES ):
        {
            *pinternalRetunedValueSize = sizeof(cl_command_queue_properties);
            if(NULL != paramVal && valSize < *pinternalRetunedValueSize)
            {
                return CL_DEV_INVALID_VALUE;
            }
            //if OUT paramVal is NULL it should be ignored
            if(NULL != paramVal)
            {
                cl_command_queue_properties queueProperties = CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE | CL_QUEUE_PROFILING_ENABLE;
                *(cl_device_exec_capabilities*)paramVal = queueProperties;
            }
            return CL_DEV_SUCCESS;
        }
        case( CL_DEVICE_HOST_UNIFIED_MEMORY):
        {
            *pinternalRetunedValueSize = sizeof(cl_bool);
            if(NULL != paramVal && valSize < *pinternalRetunedValueSize)
            {
                return CL_DEV_INVALID_VALUE;
            }
            //if OUT paramVal is NULL it should be ignored
            if(NULL != paramVal)
            {
                *(cl_bool*)paramVal = CL_TRUE;
            }
            return CL_DEV_SUCCESS;
        }
        case( CL_DEVICE_NAME):
        {
            const char* name = CPUDetect::GetInstance()->GetCPUBrandString();
            *pinternalRetunedValueSize = (NULL == name) ? 0 : (strlen(name) + 1);
            if(NULL != paramVal && valSize < *pinternalRetunedValueSize)
            {
                return CL_DEV_INVALID_VALUE;
            }
            //if OUT paramVal is NULL it should be ignored
            if((NULL != paramVal) && (NULL != name))
            {
                STRCPY_S((char*)paramVal, valSize, name);
            }

            return CL_DEV_SUCCESS;
        }
        case( CL_DEVICE_VENDOR):
        {
            *pinternalRetunedValueSize = strlen(VENDOR_STRING) + 1;
            if(NULL != paramVal && valSize < *pinternalRetunedValueSize)
            {
                return CL_DEV_INVALID_VALUE;
            }
            //if OUT paramVal is NULL it should be ignored
            if(NULL != paramVal)
            {
                STRCPY_S((char*)paramVal, valSize, VENDOR_STRING);
            }
            return CL_DEV_SUCCESS;
        }
        case( CL_DEVICE_PROFILE):
        {
            *pinternalRetunedValueSize = strlen("FULL_PROFILE") + 1;
            if(NULL != paramVal && valSize < *pinternalRetunedValueSize)
            {
                return CL_DEV_INVALID_VALUE;
            }
            //if OUT paramVal is NULL it should be ignored
            if(NULL != paramVal)
            {
                STRCPY_S((char*)paramVal, valSize, "FULL_PROFILE");
            }
            return CL_DEV_SUCCESS;
        }
        case( CL_DEVICE_OPENCL_C_VERSION):
            {                
                CPUDeviceConfig config;
                config.Initialize(GetConfigFilePath());
                OPENCL_VERSION ver = config.GetOpenCLVersion();
                *pinternalRetunedValueSize = strlen(OPENCL_VERSION_1_2 == ver ? sOpenCLC12Str : sOpenCLC20Str) + 1;
                if(NULL != paramVal && valSize < *pinternalRetunedValueSize)
                {
                    return CL_DEV_INVALID_VALUE;
                }
                //if OUT paramVal is NULL it should be ignored
                if(NULL != paramVal)
                {
                    STRCPY_S((char*)paramVal, valSize, OPENCL_VERSION_1_2 == ver ? sOpenCLC12Str : sOpenCLC20Str);
                }
                return CL_DEV_SUCCESS;
            }
        case( CL_DEVICE_VERSION):
        {
            CPUDeviceConfig config;
            config.Initialize(GetConfigFilePath());
            OPENCL_VERSION ver = config.GetOpenCLVersion();
            *pinternalRetunedValueSize = strlen(OPENCL_VERSION_1_2 == ver ? sOpenCL12Str : sOpenCL20Str) + strlen(BUILDVERSIONSTR) + 1;
            if(NULL != paramVal && valSize < *pinternalRetunedValueSize)
            {
                return CL_DEV_INVALID_VALUE;
            }
            //if OUT paramVal is NULL it should be ignored
            if(NULL != paramVal)
            {
                SPRINTF_S((char*)paramVal, valSize, "%s%s", OPENCL_VERSION_1_2 == ver ? sOpenCL12Str : sOpenCL20Str, BUILDVERSIONSTR);
            }
            return CL_DEV_SUCCESS;
        }
        case( CL_DRIVER_VERSION ):
        {
            int major = 0;
            int minor = 0;
            int revision = 0;
            int build = 0;
            std::stringstream driverVerStream;
            if (GetModuleProductVersion(__FUNCTION__, &major, &minor, &revision, &build))
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

            *pinternalRetunedValueSize = driverVer.length() + 1;
            if(NULL != paramVal && valSize < *pinternalRetunedValueSize)
            {
                return CL_DEV_INVALID_VALUE;
            }
            //if OUT paramVal is NULL it should be ignored
            if(NULL != paramVal)
            {
                STRCPY_S((char*)paramVal, valSize, driverVer.c_str());
            }
            return CL_DEV_SUCCESS;
        }
        case( CL_DEVICE_EXTENSIONS):
        {
            CPUDeviceConfig config;
            config.Initialize(GetConfigFilePath());
            OPENCL_VERSION ver = config.GetOpenCLVersion();
            const char* oclSupportedExtensions = OPENCL_VERSION_1_2 == ver ? OCL_SUPPORTED_EXTENSIONS_1_2 : OCL_SUPPORTED_EXTENSIONS_2_0;
            if (CPUDetect::GetInstance()->IsProcessorType(PT_ATOM))
            {
                oclSupportedExtensions = OPENCL_VERSION_1_2 == ver ? OCL_SUPPORTED_EXTENSIONS_ATOM_1_2 : OCL_SUPPORTED_EXTENSIONS_ATOM_2_0;
            }
            *pinternalRetunedValueSize = strlen(oclSupportedExtensions) + 1;
            if(NULL != paramVal && valSize < *pinternalRetunedValueSize)
            {
                return CL_DEV_INVALID_VALUE;
            }
            //if OUT paramVal is NULL it should be ignored
            if(NULL != paramVal)
            {
                STRCPY_S((char*)paramVal, valSize, oclSupportedExtensions);
            }
            return CL_DEV_SUCCESS;

        }
        case( CL_DEVICE_BUILT_IN_KERNELS):
        {
            *pinternalRetunedValueSize = BuiltInKernelRegistry::GetInstance()->GetBuiltInKernelListSize();
            if(NULL != paramVal && valSize < *pinternalRetunedValueSize)
            {
                return CL_DEV_INVALID_VALUE;
            }
            //if OUT paramVal is NULL it should be ignored
            if(NULL != paramVal)
            {
                BuiltInKernelRegistry::GetInstance()->GetBuiltInKernelList((char*)paramVal, valSize);
            }
            return CL_DEV_SUCCESS;
        }
        

        case CL_DEVICE_PARTITION_PROPERTIES:
#ifndef __ANDROID__
            {
                const cl_device_partition_property* pSupportedProperties;
                pSupportedProperties       = CPU_SUPPORTED_FISSION_MODES;
                *pinternalRetunedValueSize = sizeof(CPU_SUPPORTED_FISSION_MODES);

                if(NULL != paramVal && valSize < *pinternalRetunedValueSize)
                {
                    return CL_DEV_INVALID_VALUE;
                }
                //if OUT paramVal is NULL it should be ignored
                if(NULL != paramVal)
                {
                    MEMCPY_S(paramVal, valSize, pSupportedProperties, *pinternalRetunedValueSize);
                }
                return CL_DEV_SUCCESS;
            }
#else
            // Disable device partition on android
            {
                *pinternalRetunedValueSize = sizeof(cl_device_partition_property);
                if(NULL != paramVal && valSize < *pinternalRetunedValueSize)
                {
                    return CL_DEV_INVALID_VALUE;
                }
                //if OUT paramVal is NULL it should be ignored
                if(NULL != paramVal)
                {
                    *(cl_device_partition_property*)paramVal = (cl_device_partition_property)0;
                }
                return CL_DEV_SUCCESS;
            }
#endif

        case CL_DEVICE_PARTITION_AFFINITY_DOMAIN:
            {
                *pinternalRetunedValueSize = sizeof(cl_device_affinity_domain);
                if(NULL != paramVal && valSize < *pinternalRetunedValueSize)
                {
                    return CL_DEV_INVALID_VALUE;
                }
                if (NULL != paramValSizeRet)
                {
                    *paramValSizeRet = *pinternalRetunedValueSize;
                }

                //if OUT paramVal is NULL it should be ignored
                if(NULL != paramVal)
                {
                    *((cl_device_affinity_domain*)paramVal) = (cl_device_affinity_domain)0;
                }
                return CL_DEV_SUCCESS;
            }

        case CL_DEVICE_IMAGE_MAX_ARRAY_SIZE:
            *pinternalRetunedValueSize = sizeof(size_t);
            if(NULL != paramVal && valSize < *pinternalRetunedValueSize)
            {
                return CL_DEV_INVALID_VALUE;
            }
            //if OUT paramVal is NULL it should be ignored
            if(NULL != paramVal)
            {
                // Use GEN constant
                *(size_t*)paramVal = CPU_MAX_ARRAY_SIZE;
            }
            return CL_DEV_SUCCESS;
        case CL_DEVICE_IMAGE_MAX_BUFFER_SIZE:
            *pinternalRetunedValueSize = sizeof(size_t);
            if (NULL != paramVal && valSize < *pinternalRetunedValueSize)
            {
                return CL_DEV_INVALID_VALUE;
            }
            if (NULL != paramVal)
            {
                // we want to support the maximum buffer when the pixel size is maximal
                const unsigned long long iImgMaxBufSize = MAX_MEM_ALLOC_SIZE / sizeof(cl_int4); // sizeof(CL_RGBA(INT32))
                if (iImgMaxBufSize < (size_t)-1)
                {
                    *(size_t*)paramVal = (size_t)iImgMaxBufSize;
                }
                else
                {
                    *(size_t*)paramVal = (size_t)-1;
                }                
            }
            return CL_DEV_SUCCESS;
        case CL_DEVICE_SVM_CAPABILITIES:
            *pinternalRetunedValueSize = sizeof(cl_device_svm_capabilities);
            if (NULL != paramVal && valSize < *pinternalRetunedValueSize)
            {
                return CL_DEV_INVALID_VALUE;
            }
            if (NULL != paramVal)
            {
                *(cl_device_svm_capabilities*)paramVal = CL_DEVICE_SVM_COARSE_GRAIN_BUFFER | CL_DEVICE_SVM_FINE_GRAIN_BUFFER | CL_DEVICE_SVM_FINE_GRAIN_SYSTEM | CL_DEVICE_SVM_ATOMICS;
            }
            return CL_DEV_SUCCESS;
        case CL_DEVICE_PIPE_MAX_ACTIVE_RESERVATIONS:
        case CL_DEVICE_PIPE_MAX_PACKET_SIZE:
            *pinternalRetunedValueSize = sizeof(cl_uint);
            if (NULL != paramVal && valSize < *pinternalRetunedValueSize)
            {
                return CL_DEV_INVALID_VALUE;
            }
            if (NULL != paramVal)
            {
                *(cl_uint*)paramVal = (cl_uint)-1;    // I don't see a reason to limit this
            }
            return CL_DEV_SUCCESS;
        case CL_DEVICE_QUEUE_ON_DEVICE_PREFERRED_SIZE:
        case CL_DEVICE_QUEUE_ON_DEVICE_MAX_SIZE:
        case CL_DEVICE_MAX_ON_DEVICE_QUEUES:
        case CL_DEVICE_MAX_ON_DEVICE_EVENTS:
            *pinternalRetunedValueSize = sizeof(cl_uint);
            if (NULL != paramVal && valSize < *pinternalRetunedValueSize)
            {
                return CL_DEV_INVALID_VALUE;
            }
            if (NULL != paramVal)
            {
                *(cl_uint*)paramVal = (cl_uint)-1;
            }
            return CL_DEV_SUCCESS;
        case CL_DEVICE_QUEUE_ON_DEVICE_PROPERTIES:
            *pinternalRetunedValueSize = sizeof(cl_command_queue_properties);
            if (NULL != paramVal && valSize < *pinternalRetunedValueSize)
            {
                return CL_DEV_INVALID_VALUE;
            }
            if (NULL != paramVal)
            {
                *(cl_command_queue_properties*)paramVal = CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE | CL_QUEUE_PROFILING_ENABLE;
            }
            return CL_DEV_SUCCESS;
        case CL_DEVICE_MAX_GLOBAL_VARIABLE_SIZE:
            *pinternalRetunedValueSize = sizeof(size_t);
            if (NULL != paramVal && valSize < *pinternalRetunedValueSize)
            {
                return CL_DEV_INVALID_VALUE;
            }
            if (NULL != paramVal)
            {
                *(size_t*)paramVal = 64 * 1024; // the same as VPG
            }
            return CL_DEV_SUCCESS;
        case CL_DEVICE_GLOBAL_VARIABLE_SHARING:
            *pinternalRetunedValueSize = sizeof(int);   // the spec has forgotten to define this query
            if (NULL != paramVal && valSize < *pinternalRetunedValueSize)
            {
                return CL_DEV_INVALID_VALUE;
            }
            if (NULL != paramVal)
            {
                *(int*)paramVal = CL_PROGRAM_VARIABLE_SHARING_NONE;
            }
            return CL_DEV_SUCCESS;
        default:
            return CL_DEV_INVALID_VALUE;
    };
    return CL_DEV_SUCCESS;

}

//! This function return IDs list for all devices in the same device type.
/*!
    \param[in]  deviceListSize          Specifies the size of memory pointed to by deviceIdsList.(in term of amount of IDs it can store)
                                        If deviceIdsList != NULL that deviceListSize must be greater than 0.
    \param[out] deviceIdsList           A pointer to memory location where appropriate values for each device ID will be store. If paramVal is NULL, it is ignored
    \param[out] deviceIdsListSizeRet    If deviceIdsList!= NULL it store the actual amount of IDs being store in deviceIdsList. 
                                        If deviceIdsList == NULL and deviceIdsListSizeRet than it store the amount of available devices.
                                        If deviceIdsListSizeRet is NULL, it is ignored.
    \retval     CL_DEV_SUCCESS          If function is executed successfully.
    \retval     CL_DEV_ERROR_FAIL        If function failed to figure the IDs of the devices.
*/
cl_dev_err_code CPUDevice::clDevGetAvailableDeviceList(size_t IN  deviceListSize, unsigned int*   OUT deviceIdsList, size_t*   OUT deviceIdsListSizeRet)
{
    if (((NULL != deviceIdsList) && (0 == deviceListSize)) || ((NULL == deviceIdsList) && (NULL == deviceIdsListSizeRet)))
    {
        return CL_DEV_ERROR_FAIL;
    }
    assert(((deviceListSize > 0) || (NULL == deviceIdsList)) && "If deviceIdsList != NULL, deviceListSize must be 1 in case of CPU device");
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

static void rollBackSubdeviceAllocation(cl_dev_subdevice_id* IN subdevice_ids, cl_uint num_successfully_allocated)
{
    for (cl_uint j = 0; j < num_successfully_allocated; ++j)
    {
          cl_dev_internal_subdevice_id* pSubdeviceId = static_cast<cl_dev_internal_subdevice_id*>(subdevice_ids[j]);
          if (NULL != pSubdeviceId->legal_core_ids)
        {
            delete[] pSubdeviceId->legal_core_ids;
        }
        delete pSubdeviceId;
    }
}

bool CPUDevice::CoreToCoreIndex(unsigned int* core)
{
    // Only support affinity masks on Linux
#ifdef WIN32
    return true;
#endif

    // Todo: maybe use binary search here
    unsigned int result = 0;
    for (; result < m_numCores; ++result)
    {
        if (*core == m_pComputeUnitMap[result])
        {
            *core = result;
            return true;
        }
    }
    return false;
}
/****************************************************************************************************************
 clDevPartition
    Calculate appropriate affinity mask to support the partitioning mode and instantiate as many SubdeviceTaskDispatcher objects as needed
********************************************************************************************************************/
cl_dev_err_code CPUDevice::clDevPartition(  cl_dev_partition_prop IN props, cl_uint IN num_requested_subdevices, cl_dev_subdevice_id IN parent_id, cl_uint* INOUT num_subdevices, void* param, cl_dev_subdevice_id* OUT subdevice_ids)
{
    if (NULL == num_subdevices)
    {
        return CL_DEV_INVALID_VALUE;
    }

    cl_dev_internal_subdevice_id* pParent = static_cast<cl_dev_internal_subdevice_id*>(parent_id);
    size_t availableComputeUnits;
    unsigned int* pParentComputeUnits;

    if (NULL == pParent)
    {
        availableComputeUnits = m_numCores;
        pParentComputeUnits   = m_pComputeUnitMap;
    }
    else
    {
        availableComputeUnits = pParent->num_compute_units;
        pParentComputeUnits   = pParent->legal_core_ids;
    }

    switch (props)
    {
    case CL_DEV_PARTITION_EQUALLY:
        {
            if (NULL == param)
            {
                return CL_DEV_INVALID_VALUE;
            }

            size_t partitionSize = *((size_t *)param);
            //Todo: at the moment disallowing a partition that's equal to the parent device
            if ((partitionSize >= availableComputeUnits) || (0 == partitionSize))
            {
                return CL_DEV_INVALID_VALUE;
            }
            size_t numPartitions = availableComputeUnits / partitionSize;
            if (NULL == subdevice_ids)
            {
                *num_subdevices = (cl_uint)numPartitions;
                return CL_DEV_SUCCESS;
            }
            if (*num_subdevices < numPartitions)
            {
                return CL_DEV_INVALID_VALUE;
            }
            *num_subdevices = (cl_uint) numPartitions;
            if (numPartitions > num_requested_subdevices)
            {
                numPartitions = num_requested_subdevices;
            }
            if (NULL != pParent)
            {
                //Disallow re-partitioning devices BY_NAMES
                if (pParent->is_by_names)
                {
                    return CL_DEV_NOT_SUPPORTED;
                }
            }

            for (cl_uint i = 0; i < (cl_uint)numPartitions; ++i)
            {
                cl_dev_internal_subdevice_id* pNewsubdeviceId = new cl_dev_internal_subdevice_id;
                if (NULL == pNewsubdeviceId)
                {
                  rollBackSubdeviceAllocation(subdevice_ids, i);
                  return CL_DEV_OUT_OF_MEMORY;
                }

                pNewsubdeviceId->legal_core_ids     = NULL;
                pNewsubdeviceId->is_by_names        = false;
                pNewsubdeviceId->num_compute_units  = (cl_uint)partitionSize;
                pNewsubdeviceId->is_acquired        = false;
                pNewsubdeviceId->ref_count          = 0;

                subdevice_ids[i] = pNewsubdeviceId;
            }
            break;
        }

    case CL_DEV_PARTITION_BY_COUNTS:
        {
            if (NULL == param)
            {
                return CL_DEV_INVALID_VALUE;
            }
            std::vector<size_t> partitionSizes = *((std::vector<size_t>*)(param));
            size_t  totalNumUnits  = 0;
            for (size_t i = 0; i < *num_subdevices; ++i)
            {
                if (0 == partitionSizes[i])
                {
                    return CL_DEV_INVALID_VALUE;
                }
                totalNumUnits += partitionSizes[i];
            }
            if (totalNumUnits > availableComputeUnits)
            {
                return CL_DEV_INVALID_VALUE;
            }
            if (NULL == subdevice_ids)
            {
                return CL_DEV_SUCCESS;
            }
            cl_uint num_subdevices_to_create = *num_subdevices;
            if (num_subdevices_to_create > num_requested_subdevices)
            {
                num_subdevices_to_create = num_requested_subdevices;
            }

            if (NULL != pParent)
            {
                //Disallow re-partitioning devices BY_NAMES
                if (pParent->is_by_names)
                {
                    return CL_DEV_NOT_SUPPORTED;
                }
            }

            for (cl_uint i = 0; i < num_subdevices_to_create; ++i)
            {
                cl_dev_internal_subdevice_id* pNewsubdeviceId = new cl_dev_internal_subdevice_id;
                if (NULL == pNewsubdeviceId)
                {
                    rollBackSubdeviceAllocation(subdevice_ids, i);
                    return CL_DEV_OUT_OF_MEMORY;
                }

                pNewsubdeviceId->legal_core_ids     = NULL;
                pNewsubdeviceId->is_by_names        = false;
                pNewsubdeviceId->num_compute_units  = (cl_uint)partitionSizes[i];
                pNewsubdeviceId->is_acquired        = false;
                pNewsubdeviceId->ref_count          = 0;

                subdevice_ids[i] = pNewsubdeviceId;
            }
            break;
        }

    case CL_DEV_PARTITION_BY_NAMES:
        {
            if (NULL == param)
            {
                return CL_DEV_INVALID_VALUE;
            }
            std::vector<size_t> requestedUnits = *((std::vector<size_t>*)(param));
            size_t  totalNumUnits  = requestedUnits.size();

            //Disallow partitions that equal the sub-device
            if (totalNumUnits >= availableComputeUnits)
            {
                return CL_DEV_INVALID_VALUE;
            }
            if (NULL == subdevice_ids)
            {
                return CL_DEV_SUCCESS;
            }
            if (NULL != pParent)
            {
                //Disallow mixing BY_NAMES with other modes
                return CL_DEV_NOT_SUPPORTED;
            }

            cl_dev_internal_subdevice_id* pNewsubdeviceId = new cl_dev_internal_subdevice_id;
            if (NULL == pNewsubdeviceId)
            {
                return CL_DEV_OUT_OF_MEMORY;
            }
            pNewsubdeviceId->legal_core_ids = new unsigned int[totalNumUnits];
            if (NULL == pNewsubdeviceId->legal_core_ids)
            {
                delete pNewsubdeviceId;
                return CL_DEV_OUT_OF_MEMORY;
            }

            pNewsubdeviceId->is_by_names       = true;
            pNewsubdeviceId->num_compute_units = (cl_uint)totalNumUnits;
            for (cl_uint core = 0; core < totalNumUnits; ++core)
            {
                pNewsubdeviceId->legal_core_ids[core] = (cl_uint)requestedUnits[core];
            }

            // Translate all the cores to "core indices"
            for (unsigned int core = 0; core < pNewsubdeviceId->num_compute_units; ++core)
            {
                if (!CoreToCoreIndex(pNewsubdeviceId->legal_core_ids + core))
                {
                    // Invalid core looked up, outside the affinity mask
                    delete[] pNewsubdeviceId->legal_core_ids;
                    delete pNewsubdeviceId;
                    return CL_DEV_INVALID_VALUE;
                }
            }

            *subdevice_ids = pNewsubdeviceId;
            if (NULL != num_subdevices)
            {
              *num_subdevices = 1;
            }

            break;
        }
    //Non-supported modes
    case CL_DEV_PARTITION_AFFINITY_L1:
    case CL_DEV_PARTITION_AFFINITY_L2:
    case CL_DEV_PARTITION_AFFINITY_L3:
    case CL_DEV_PARTITION_AFFINITY_L4:
    case CL_DEV_PARTITION_AFFINITY_NUMA:
    case CL_DEV_PARTITION_AFFINITY_NEXT:
        return CL_DEV_INVALID_PROPERTIES;
    default:
        return CL_DEV_INVALID_VALUE;

    }

    // create sub-devices in TaskExecutor
    for (size_t i = 0; i < *num_subdevices; i++)
    {
        cl_dev_internal_subdevice_id& subdevId = *(cl_dev_internal_subdevice_id*)subdevice_ids[i];
        subdevId.pSubDevice = m_pTaskDispatcher->createSubdevice(subdevId.num_compute_units, &subdevId);
        if (NULL == subdevId.pSubDevice)
        {
            for (size_t j = 0; j < i; j++)
            {
                m_pTaskDispatcher->releaseSubdevice(((cl_dev_internal_subdevice_id*)subdevice_ids[j])->pSubDevice);
            }
            rollBackSubdeviceAllocation( subdevice_ids, *num_subdevices );
            return CL_DEV_OUT_OF_MEMORY;
        }
    }

    return CL_DEV_SUCCESS;
}
/****************************************************************************************************************
 clDevReleaseSubdevice
    Release a sub-device created by a clDevPartition call. Releases the appropriate SubdeviceTaskDispatcher object
********************************************************************************************************************/
cl_dev_err_code CPUDevice::clDevReleaseSubdevice(  cl_dev_subdevice_id IN subdevice_id)
{
    if (NULL == subdevice_id)
    {
        return CL_DEV_INVALID_VALUE;
    }
    cl_dev_internal_subdevice_id* pSubdeviceData = reinterpret_cast<cl_dev_internal_subdevice_id*>(subdevice_id);
    if (NULL != pSubdeviceData)
    {
         m_pTaskDispatcher->releaseSubdevice(pSubdeviceData->pSubDevice);
         if (NULL != pSubdeviceData->legal_core_ids)
         {
             delete[] pSubdeviceData->legal_core_ids;
         }
         delete pSubdeviceData;
    }
    return CL_DEV_SUCCESS;
}

// Execution commands
/****************************************************************************************************************
 clDevCreateCommandList
    Call TaskDispatcher to create command list
********************************************************************************************************************/
cl_dev_err_code CPUDevice::clDevCreateCommandList( cl_dev_cmd_list_props IN props, cl_dev_subdevice_id IN subdevice_id, cl_dev_cmd_list* OUT list)
{
    CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("clDevCreateCommandList Function enter"));
    cl_dev_internal_cmd_list* pList = new cl_dev_internal_cmd_list();
    if (NULL == pList)
    {
        return CL_DEV_OUT_OF_MEMORY;
    }

    cl_dev_internal_subdevice_id* pSubdeviceData = reinterpret_cast<cl_dev_internal_subdevice_id*>(subdevice_id);
    pList->subdevice_id    = pSubdeviceData;

    // If subdevice_id is not NULL, we are creating list for a sub-device
    if (NULL != pSubdeviceData)
    {
        long prev = pSubdeviceData->ref_count++;
        if (0 == prev)
        {
            //When fissioned by name, need first to acquire the required cores
            // Not used in other fission modes
            if ( (NULL != pSubdeviceData->legal_core_ids) )
            {
                if ( !AcquireComputeUnits(pSubdeviceData->legal_core_ids, pSubdeviceData->num_compute_units) )
                {
                    delete pList;
                    pSubdeviceData->ref_count--;
                    return CL_DEV_ERROR_FAIL;
                }
#ifdef __HARD_TRAPPING__
                // Now "trap" threads in device
                if ( !pSubdeviceData->pSubDevice->AcquireWorkerThreads() )
                {
                    assert(0 && "Acquring of the worker threads failed");
                    delete pList;
                    pSubdeviceData->ref_count--;
                    ReleaseComputeUnits(pSubdeviceData->legal_core_ids, pSubdeviceData->num_compute_units);
                    return CL_DEV_ERROR_FAIL;
                }
#endif // __HARD_TRAPPING__                
            }
            pSubdeviceData->is_acquired = true;
        }
        else
        {
            while (!pSubdeviceData->is_acquired)
            {
                //Todo: not a good spin loop methodology
                clSleep(0);
            }
        }
    }

    ITEDevice* pDevice = (NULL != pSubdeviceData) ? pSubdeviceData->pSubDevice : NULL;

    cl_dev_err_code ret = m_pTaskDispatcher->createCommandList(props, pDevice, &pList->cmd_list);
    if ( CL_DEV_FAILED(ret) )
    {
        delete pList;
        if ( (NULL!=pSubdeviceData) )
        {
            long prev = pSubdeviceData->ref_count--;
            if ( 1 == prev )
            {
              if ( NULL!=pSubdeviceData->legal_core_ids )
              {
#ifdef __HARD_TRAPPING__
                  pSubdeviceData->pSubDevice->RelinquishWorkerThreads();
#endif // __HARD_TRAPPING__                
                  ReleaseComputeUnits(pSubdeviceData->legal_core_ids, pSubdeviceData->num_compute_units);
              }
              pSubdeviceData->is_acquired = false;
            }
        }
        return ret;
    }
    *list = pList;
    return CL_DEV_SUCCESS;
}

/****************************************************************************************************************
 clDevFlushCommandList
    Call TaskDispatcher to flush command list
********************************************************************************************************************/
cl_dev_err_code CPUDevice::clDevFlushCommandList( cl_dev_cmd_list IN list)
{
    CpuDbgLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("clDevFlushCommandList Function enter"));
    cl_dev_internal_cmd_list* pList = static_cast<cl_dev_internal_cmd_list*>(list);
    if (NULL == pList)
    {
        return CL_DEV_INVALID_VALUE;
    }
    return m_pTaskDispatcher->flushCommandList(pList->cmd_list);
}

/****************************************************************************************************************
 clDevReleaseCommandList
    Call TaskDispatcher to release command list
********************************************************************************************************************/
cl_dev_err_code CPUDevice::clDevReleaseCommandList( cl_dev_cmd_list IN list )
{
    CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("clDevReleaseCommandList Function enter"));
    cl_dev_internal_cmd_list* pList = static_cast<cl_dev_internal_cmd_list*>(list);
    if (NULL == pList)
    {
        return CL_DEV_INVALID_VALUE;
    }
    cl_dev_err_code ret = m_pTaskDispatcher->releaseCommandList(pList->cmd_list);
    if (CL_DEV_FAILED(ret))
    {
        return ret;
    }
    if (NULL != pList->subdevice_id)
    {
        long prev = pList->subdevice_id->ref_count--;
        if (1 == prev)
        {
            if ( NULL != pList->subdevice_id->legal_core_ids )
            {
#ifdef __HARD_TRAPPING__
                pList->subdevice_id->pSubDevice->RelinquishWorkerThreads();
#endif // __HARD_TRAPPING__
                ReleaseComputeUnits(pList->subdevice_id->legal_core_ids, pList->subdevice_id->num_compute_units);
            }
            pList->subdevice_id->is_acquired = false;
        }
    }
    delete pList;
    return CL_DEV_SUCCESS;
}

/****************************************************************************************************************
 clDevReleaseCommand
********************************************************************************************************************/
void CPUDevice::clDevReleaseCommand(cl_dev_cmd_desc* IN cmdToRelease)
{
    SharedPtr<ITaskBase> ptr = (ITaskBase*)cmdToRelease->device_agent_data;
    ptr.DecRefCnt();    // see IncRefCnt in TaskDispatcher::SubmitTaskArray
}

/****************************************************************************************************************
 clDevCommandListExecute
    Call TaskDispatcher to execute command list
********************************************************************************************************************/
cl_dev_err_code CPUDevice::clDevCommandListExecute( cl_dev_cmd_list IN list, cl_dev_cmd_desc* IN *cmds, cl_uint IN count)
{
    CpuDbgLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("clDevCommandListExecute Function enter"));
    if (NULL != list)
    {
        cl_dev_internal_cmd_list* pList = static_cast<cl_dev_internal_cmd_list*>(list);
        return m_pTaskDispatcher->commandListExecute(pList->cmd_list, cmds, count);
    }
    else
    {
        if ( NULL == m_defaultCommandList )
        {
            cl_dev_err_code ret = clDevCreateCommandList(CL_DEV_LIST_ENABLE_OOO, 0, &m_defaultCommandList);
            if ( CL_DEV_FAILED(ret) )
            {
                CpuErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("clDevCommandListExecute failed to create internal command list: %d"), ret);
                return ret;
            }
        }
        cl_dev_internal_cmd_list* pList = static_cast<cl_dev_internal_cmd_list*>(m_defaultCommandList);
        cl_dev_err_code ret = m_pTaskDispatcher->commandListExecute(pList->cmd_list,cmds,count);
        if (CL_DEV_FAILED(ret))
        {
            CpuErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("clDevCommandListExecute failed to submit command to execution: %d"), ret);
            return ret;
        }
        return m_pTaskDispatcher->flushCommandList(pList->cmd_list);
    }
}

/****************************************************************************************************************
 clDevCommandListExecute
    Call clDevCommandListWaitCompletion to add calling thread to execution pool
********************************************************************************************************************/
cl_dev_err_code CPUDevice::clDevCommandListWaitCompletion(cl_dev_cmd_list IN list, cl_dev_cmd_desc* IN cmdToWait)
{
    CpuDbgLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("clDevCommandListWaitCompletion Function enter"));
    cl_dev_internal_cmd_list* pList = static_cast<cl_dev_internal_cmd_list*>(list);
    if (NULL == pList)
    {
        return CL_DEV_INVALID_VALUE;
    }

    return m_pTaskDispatcher->commandListWaitCompletion(pList->cmd_list, cmdToWait);
}

/****************************************************************************************************************
 clDevCommandListCancel
********************************************************************************************************************/
cl_dev_err_code CPUDevice::clDevCommandListCancel(cl_dev_cmd_list IN list)
{
    CpuDbgLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("clDevCommandListCancel Function enter"));
    cl_dev_internal_cmd_list* pList = static_cast<cl_dev_internal_cmd_list*>(list);
    if (NULL == pList)
    {
        return CL_DEV_INVALID_VALUE;
    }

    return m_pTaskDispatcher->cancelCommandList(pList->cmd_list);
}

//Memory API's
/****************************************************************************************************************
 clDevGetSupportedImageFormats
    Call Memory Allocator to get supported image formats
********************************************************************************************************************/
cl_dev_err_code CPUDevice::clDevGetSupportedImageFormats( cl_mem_flags IN flags, cl_mem_object_type IN imageType,
                cl_uint IN numEntries, cl_image_format* OUT formats, cl_uint* OUT numEntriesRet) const
{
    CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("clDevGetSupportedImageFormats Function enter"));
    return (cl_dev_err_code)m_pProgramService->GetSupportedImageFormats(flags, imageType, numEntries, formats, numEntriesRet);
}

cl_dev_err_code CPUDevice::clDevGetMemoryAllocProperties( cl_mem_object_type IN memObjType, cl_dev_alloc_prop* OUT pAllocProp )
{
    CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("clDevGetMemoryAllocProperties Function enter"));
    return m_pMemoryAllocator->GetAllocProperties(memObjType, pAllocProp);
}

/****************************************************************************************************************
 clDevCreateMemoryObject
    Call Memory Allocator to create memory object
********************************************************************************************************************/
cl_dev_err_code CPUDevice::clDevCreateMemoryObject( cl_dev_subdevice_id node_id, cl_mem_flags IN flags, const cl_image_format* IN format,
                                    size_t  IN dim_count, const size_t* IN dim_size, IOCLDevRTMemObjectService* pRTService, IOCLDevMemoryObject* OUT *memObj)
{
    CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("clDevCreateMemoryObject Function enter"));
    return m_pMemoryAllocator->CreateObject(node_id, flags, format, dim_count, dim_size, pRTService, memObj);
}

/****************************************************************************************************************
 clDevCheckProgramBinary
    Call Program Serice to check binaries
********************************************************************************************************************/
cl_dev_err_code CPUDevice::clDevCheckProgramBinary( size_t IN binSize, const void* IN bin )
{
    CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("clDevCheckProgramBinary Function enter"));
    return (cl_dev_err_code)m_pProgramService->CheckProgramBinary(binSize, bin );
}

/*******************************************************************************************************************
clDevCreateProgram
    Call programService to create program
**********************************************************************************************************************/

cl_dev_err_code CPUDevice::clDevCreateProgram( size_t IN binSize, const void* IN bin, cl_dev_binary_prop IN prop, cl_dev_program* OUT prog )
{
    CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("clDevCreateProgram Function enter"));
    return (cl_dev_err_code)m_pProgramService->CreateProgram(binSize, bin, prop, prog );
}

cl_dev_err_code CPUDevice::clDevCreateBuiltInKernelProgram( const char* IN szBuiltInNames, cl_dev_program* OUT prog )
{
    CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("clDevCreateBuiltInKernelProgram Function enter"));
    return (cl_dev_err_code)m_pProgramService->CreateBuiltInKernelProgram(szBuiltInNames, prog);
}

/*******************************************************************************************************************
clDevBuildProgram
    Call programService to build program
**********************************************************************************************************************/

cl_dev_err_code CPUDevice::clDevBuildProgram( cl_dev_program IN prog, const char* IN options, cl_build_status* OUT buildStatus )
{
    CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("clDevBuildProgram Function enter"));
    return (cl_dev_err_code)m_pProgramService->BuildProgram(prog, options, buildStatus);
}

/*******************************************************************************************************************
clDevReleaseProgram
    Call programService to release program
**********************************************************************************************************************/

cl_dev_err_code CPUDevice::clDevReleaseProgram( cl_dev_program IN prog )
{
    CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("clDevReleaseProgram Function enter"));
    return (cl_dev_err_code)m_pProgramService->ReleaseProgram( prog );
}

/*******************************************************************************************************************
clDevUnloadCompiler
    Call programService to unload the backend compiler
**********************************************************************************************************************/
cl_dev_err_code CPUDevice::clDevUnloadCompiler()
{
    CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("clDevUnloadCompiler Function enter"));
    return (cl_dev_err_code)m_pProgramService->UnloadCompiler();
}
/*******************************************************************************************************************
clDevGetProgramBinary
    Call programService to get the program binary
**********************************************************************************************************************/
cl_dev_err_code CPUDevice::clDevGetProgramBinary( cl_dev_program IN prog, size_t IN size, void* OUT binary, size_t* OUT sizeRet )
{
    CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("clDevGetProgramBinary Function enter"));
    return (cl_dev_err_code)m_pProgramService->GetProgramBinary(prog, size, binary, sizeRet );
}
/*******************************************************************************************************************
clDevGetBuildLog
    Call programService to get the build log
**********************************************************************************************************************/
cl_dev_err_code CPUDevice::clDevGetBuildLog( cl_dev_program IN prog, size_t IN size, char* OUT log, size_t* OUT sizeRet)
{
    CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("clDevGetBuildLog Function enter"));
    return (cl_dev_err_code)m_pProgramService->GetBuildLog(prog, size, log, sizeRet);
}
/*******************************************************************************************************************
clDevUnloadCompiler
    Call programService to get supported binary description
**********************************************************************************************************************/
cl_dev_err_code CPUDevice::clDevGetSupportedBinaries( size_t IN count, cl_prog_binary_desc* OUT types, size_t* OUT sizeRet )
{
    CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("clDevGetSupportedBinaries Function enter"));
    return (cl_dev_err_code)m_pProgramService->GetSupportedBinaries(count,types,sizeRet );
}
/*******************************************************************************************************************
clDevUnloadCompiler
    Call programService to get kernel id from its name
**********************************************************************************************************************/
cl_dev_err_code CPUDevice::clDevGetKernelId( cl_dev_program IN prog, const char* IN name, cl_dev_kernel* OUT kernelId )
{
    CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("clDevGetKernelId Function enter"));
    return (cl_dev_err_code)m_pProgramService->GetKernelId(prog, name, kernelId );
}
/*******************************************************************************************************************
clDevUnloadCompiler
    Call programService to get kernels from the program
**********************************************************************************************************************/
cl_dev_err_code CPUDevice::clDevGetProgramKernels( cl_dev_program IN prog, cl_uint IN numKernels, cl_dev_kernel* OUT kernels,
                         cl_uint* OUT numKernelsRet )
{
    CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("clDevGetProgramKernels Function enter"));
    return (cl_dev_err_code)m_pProgramService->GetProgramKernels(prog, numKernels, kernels,numKernelsRet );
}
/*******************************************************************************************************************
clDevGetKernelInfo
    Call programService to get kernel info
**********************************************************************************************************************/
cl_dev_err_code CPUDevice::clDevGetKernelInfo( cl_dev_kernel IN kernel, cl_dev_kernel_info IN param, size_t IN valueSize,
                    void* OUT value, size_t* OUT valueSizeRet )
{
    CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("clDevGetKernelInfo Function enter"));
    return (cl_dev_err_code)m_pProgramService->GetKernelInfo(kernel, param, valueSize,value,valueSizeRet );
}

/*******************************************************************************************************************
clDevGetPerofrmanceCounter
    Get performance counter value
**********************************************************************************************************************/
cl_ulong CPUDevice::clDevGetPerformanceCounter()
{
    return Intel::OpenCL::Utils::HostTime();
}

cl_dev_err_code CPUDevice::clDevSetLogger(IOCLDevLogDescriptor *pLogDescriptor)
{

    if ( NULL != m_pLogDescriptor )
    {
        m_pLogDescriptor->clLogReleaseClient(m_iLogHandle);
    }
    m_pLogDescriptor = pLogDescriptor;
    if ( NULL != m_pLogDescriptor )
    {
        cl_dev_err_code ret = (cl_dev_err_code)m_pLogDescriptor->clLogCreateClient(m_uiCpuId, "CPU Device", &m_iLogHandle);
        if(CL_DEV_SUCCESS != ret)
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
void CPUDevice::clDevCloseDevice(void)
{
    CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("clCloseDevice Function enter"));

    if ( NULL != m_defaultCommandList )
    {
        clDevReleaseCommandList(m_defaultCommandList);
    }
    if( NULL != m_pCPUDeviceConfig)
    {
        delete m_pCPUDeviceConfig;
        m_pCPUDeviceConfig = NULL;
    }
    if ( 0 != m_iLogHandle)
    {
        m_pLogDescriptor->clLogReleaseClient(m_iLogHandle);
    }

    if ( NULL != m_pProgramService )
    {
        delete m_pProgramService;
        m_pProgramService = NULL;
    }
    if ( NULL != m_pMemoryAllocator )
    {
        delete m_pMemoryAllocator;
        m_pMemoryAllocator = NULL;
    }
    if ( NULL != m_pTaskDispatcher )
    {
        // The WGContextPool will set to NULL in the destructor
        delete m_pTaskDispatcher;
        m_pTaskDispatcher = NULL;
    }

    if ( NULL != m_pComputeUnitMap)
    {
        delete[] m_pComputeUnitMap;
        m_pComputeUnitMap = NULL;
    }

    m_pCoreToThread.clear();
    m_pCoreInUse.clear();
    m_threadToCore.clear();
    m_wgContextPool.Clear();    // we have to clear the pool before the BE wrapper is terminated, because ~WGContext still needs it
    m_backendWrapper.Terminate();

    delete this;
}

const char* CPUDevice::clDevFEModuleName() const
{
#if defined (_WIN32)
#if defined (_M_X64)
    static const char* sFEModuleName = "clang_compiler64";
#else
    static const char* sFEModuleName = "clang_compiler32";
#endif
#else
    static const char* sFEModuleName = "clang_compiler";
#endif
    return sFEModuleName;
}

const void* CPUDevice::clDevFEDeviceInfo() const
{
	return GetCPUDevInfo(*m_pCPUDeviceConfig);
}

size_t CPUDevice::clDevFEDeviceInfoSize() const
{
    return sizeof(Intel::OpenCL::ClangFE::CLANG_DEV_INFO);
}

using Intel::OpenCL::DeviceCommands::DeviceCommand;

int CPUDevice::EnqueueKernel(queue_t queue, kernel_enqueue_flags_t flags, cl_uint uiNumEventsInWaitList, const clk_event_t* pEventWaitList, clk_event_t* pEventRet,
        const Intel::OpenCL::DeviceBackend::ICLDevBackendKernel_* pKernel, const void* pContext, size_t szContextSize, const cl_work_description_type* pNdrange
    )
{    
    // verify parameters
    ASSERT_RET_VAL(pKernel != NULL, "Trying to enqueue with NULL kernel", CL_INVALID_KERNEL);
    if (NULL == queue || (NULL == pEventWaitList && uiNumEventsInWaitList > 0) || (NULL != pEventWaitList && 0 == uiNumEventsInWaitList) ||
        (flags != CLK_ENQUEUE_FLAGS_NO_WAIT && flags != CLK_ENQUEUE_FLAGS_WAIT_KERNEL && flags != CLK_ENQUEUE_FLAGS_WAIT_WORK_GROUP))
    {
        return CL_ENQUEUE_FAILURE;
    }
        
    ITaskList* pList = (ITaskList*)queue;
    if (pList->DoesSupportDeviceSideCommandEnqueue())
    {
        return CL_INVALID_OPERATION;
    }
    KernelCommand* pParent = NDRange::GetThreadLocalNDRange();
    SharedPtr<KernelCommand> pChild;
#if 0
        DeviceNDRange* const pChildAddress = m_deviceNDRangeAllocator.allocate(sizeof(DeviceNDRange));    // currently we ignore bad_alloc
        pChild = ::new(pChildAddress) DeviceNDRange(m_pTaskDispatcher, pList, pParent, pKernel, pContext, szContextSize, pNdrange, m_deviceNDRangeAllocator, m_deviceNDRangeContextAllocator);
#else
        pChild = DeviceNDRange::Allocate(m_pTaskDispatcher, pList, pParent, pList->GetNDRangeChildrenTaskGroup(), pKernel, pContext, szContextSize, pNdrange);
#endif
    pParent->AddChildKernel(pChild, flags);    
    const bool bAllEventsCompleted = pChild->AddWaitListDependencies(pEventWaitList, uiNumEventsInWaitList);

    // if no need to wait, enqueue and flush
    if (CLK_ENQUEUE_FLAGS_NO_WAIT == flags && bAllEventsCompleted)
    {
        pChild->Launch();
    }

    // update pEventRet
    if (pEventRet != NULL)
    {
        *pEventRet = static_cast<DeviceCommand*>(pChild.GetPtr());
        pChild.IncRefCnt();    // it will decremeneted in release_event
    }
    return CL_SUCCESS;
}

int CPUDevice::EnqueueMarker(queue_t queue, cl_uint uiNumEventsInWaitList, const clk_event_t* pEventWaitList, clk_event_t* pEventRet)
{
    if (NULL == queue || NULL == pEventWaitList || 0 == uiNumEventsInWaitList)
    {
        return CL_ENQUEUE_FAILURE;
    }

    const SharedPtr<ITaskList> pList = (ITaskList*)queue;
    if (!pList->DoesSupportDeviceSideCommandEnqueue())
    {
        return CL_INVALID_OPERATION;
    }
    SharedPtr<Marker> marker = Marker::Allocate(pList);    // should we use a pool here too?
    const bool bAllEventsCompleted = marker->AddWaitListDependencies(pEventWaitList, uiNumEventsInWaitList);
    if (bAllEventsCompleted)
    {
        marker->Launch();
    }
    if (pEventRet != NULL)
    {
        *pEventRet = static_cast<DeviceCommand*>(marker.GetPtr());
        marker.IncRefCnt();    // it will decremeneted in release_event
    }
    return CL_SUCCESS;
}

int CPUDevice::RetainEvent(clk_event_t event)
{
    if (NULL == event)
    {
        return CL_INVALID_EVENT;
    }
    SharedPtr<DeviceCommand> pCmd = (DeviceCommand*)event;
    pCmd.IncRefCnt();
    return CL_SUCCESS;
}

int CPUDevice::ReleaseEvent(clk_event_t event)
{
    if (NULL == event)
    {
        return CL_INVALID_EVENT;
    }
    SharedPtr<DeviceCommand> pCmd = (DeviceCommand*)event;
    pCmd.DecRefCnt();
    return CL_SUCCESS;
}

clk_event_t CPUDevice::CreateUserEvent(int* piErrcodeRet)
{
    SharedPtr<UserEvent> pUserEvent = UserEvent::Allocate();
    if (NULL != pUserEvent)
    {
        pUserEvent.IncRefCnt();
    }
    if (NULL != piErrcodeRet)
    {
        if (NULL != pUserEvent)
        {
            *piErrcodeRet = CL_SUCCESS;
        }
        else
        {
            *piErrcodeRet = CL_EVENT_ALLOCATION_FAILURE;
        }
    }
    return (clk_event_t)(pUserEvent.GetPtr());
}
    
int CPUDevice::SetEventStatus(clk_event_t event, int iStatus)
{    
    if (NULL == event || !((DeviceCommand*)event)->IsUserCommand())
    {
        return CL_INVALID_EVENT;
    }
    if (CL_COMPLETE != iStatus && iStatus >= 0)
    {
        return CL_INVALID_VALUE;
    }
    SharedPtr<UserEvent> pUserEvent = (UserEvent*)event;
    pUserEvent->SetStatus(iStatus);
    return CL_SUCCESS;
}

void CPUDevice::CaptureEventProfilingInfo(clk_event_t event, clk_profiling_info name, volatile void* pValue)
{
    if (NULL == event || name != CLK_PROFILING_COMMAND_EXEC_TIME || NULL == pValue)
    {
        return;
    }    

    SharedPtr<DeviceCommand> pCmd = (DeviceCommand*)event;
    if ((pCmd->GetList() != NULL && !pCmd->GetList()->IsProfilingEnabled()) || pCmd->IsUserCommand())
    {
        return;
    }
    if (!pCmd->SetExecTimeUserPtr(pValue))    // otherwise the information will be available when the command completes
    {
        *(cl_ulong*)pValue = pCmd->GetExecutionTime();
    }        
}

queue_t CPUDevice::GetDefaultQueueForDevice() const
{    
    NDRange* pParent = NDRange::GetThreadLocalNDRange();
    return pParent->GetTaskDispatcher()->GetDefaultQueue();
}

unsigned int CPUDevice::GetNumComputeUnits() const
{
    KernelCommand* pParent = NDRange::GetThreadLocalNDRange();
    return pParent->GetList()->GetDevice()->GetConcurrency();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Static extern functions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/************************************************************************************************************************
   clDevGetDeviceInfo
**************************************************************************************************************************/
extern "C" cl_dev_err_code clDevGetDeviceInfo(  unsigned int IN    dev_id,
                            cl_device_info  param, 
                            size_t          valSize, 
                            void*           paramVal,
                            size_t*         paramValSizeRet
                            )
{
    return CPUDevice::clDevGetDeviceInfo(dev_id, param, valSize, paramVal, paramValSizeRet);
}

/************************************************************************************************************************
    clDevGetAvailableDeviceList
*************************************************************************************************************************/
extern "C" cl_dev_err_code clDevGetAvailableDeviceList(size_t    IN  deviceListSize,
                        unsigned int*   OUT deviceIdsList,
                        size_t*   OUT deviceIdsListSizeRet)
{
    return CPUDevice::clDevGetAvailableDeviceList(deviceListSize, deviceIdsList, deviceIdsListSizeRet);
}

////////////////////////////////////////////////////////////////////////////////////
// clDevInitDeviceAgent
//! This function initializes device agent internal data. This function should be called prior to any device agent calls.
/*!
    \retval     CL_DEV_SUCCESS          If function is executed successfully.
    \retval     CL_DEV_ERROR_FAIL        If function failed to figure the IDs of the devices.
*/
extern "C" cl_dev_err_code clDevInitDeviceAgent(void)
{
#ifdef __INCLUDE_MKL__
    Intel::OpenCL::MKLKernels::InitLibrary();
#endif
    return CL_DEV_SUCCESS;
}
