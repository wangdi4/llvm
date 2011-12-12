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

///////////////////////////////////////////////////////////
//  CPUDevice.cpp
///////////////////////////////////////////////////////////


#include "stdafx.h"
#include "cpu_device.h"
#include "program_service.h"
#include "memory_allocator.h"
#include "task_dispatcher.h"
#include "cpu_logger.h"
#include "buildversion.h"
#include "CL/cl_ext.h"

#include <cl_sys_info.h>
#include <cpu_dev_limits.h>
#include <cl_sys_defines.h>
#include <cl_cpu_detect.h>

#if defined (__GNUC__) && !(__INTEL_COMPILER)  && !(_WIN32)
#include "hw_utils.h"
#endif

#define __DOUBLE_ENABLED__
using namespace Intel::OpenCL::CPUDevice;

char clCPUDEVICE_CFG_PATH[MAX_PATH];

#if defined(_M_X64) || defined(__x86_64__)
	#define MAX_MEM_ALLOC_SIZE (MAX(128*1024*1024, TotalPhysicalSize()/4))
	#define GLOBAL_MEM_SIZE (TotalPhysicalSize())
#else
	// When running on 32bit application on 64bit OS, the total physical size might exceed virtual evailable memory 
	#define MEMORY_LIMIT	(MIN(TotalPhysicalSize(), TotalVirtualSize()))
	#define MAX_MEM_ALLOC_SIZE (MAX(128*1024*1024, MEMORY_LIMIT/4))
	#define GLOBAL_MEM_SIZE (MEMORY_LIMIT)
#endif

#define __MINUMUM_SUPPORT__
//#define __TEST__
const cl_image_format Intel::OpenCL::CPUDevice::supportedImageFormats[] = {
#ifndef __TEST__

    // Minimum supported image formats
    // CL_RGBA
    {CL_RGBA, CL_UNORM_INT8},
    {CL_RGBA, CL_UNORM_INT16},
    {CL_RGBA, CL_SIGNED_INT8},
    {CL_RGBA, CL_SIGNED_INT16},
    {CL_RGBA, CL_SIGNED_INT32},
    {CL_RGBA, CL_UNSIGNED_INT8},
    {CL_RGBA, CL_UNSIGNED_INT16},
    {CL_RGBA, CL_UNSIGNED_INT32},
    {CL_RGBA, CL_HALF_FLOAT},
    {CL_RGBA, CL_FLOAT},

    // CL_BGRA
    {CL_BGRA,   CL_UNORM_INT8},

    // Additional formats required by users
    // CL_INTENCITY
    {CL_INTENSITY,  CL_FLOAT},

    // CL_LUMINANCE
    {CL_LUMINANCE,  CL_FLOAT},
#ifndef __MINUMUM_SUPPORT__
    // CL_R
    {CL_R,      CL_UNORM_INT8},
    {CL_R,      CL_UNORM_INT16},
    {CL_R,      CL_SNORM_INT8},
    {CL_R,      CL_SNORM_INT16},
    {CL_R,      CL_SIGNED_INT8},
    {CL_R,      CL_SIGNED_INT16},
    {CL_R,      CL_SIGNED_INT32},
    {CL_R,      CL_UNSIGNED_INT8},
    {CL_R,      CL_UNSIGNED_INT16},
    {CL_R,      CL_UNSIGNED_INT32},
    //  {CL_R,      CL_HALF_FLOAT},
    {CL_R,      CL_FLOAT},

    // CL_A
    {CL_A,      CL_UNORM_INT8},
    {CL_A,      CL_UNSIGNED_INT8},
    {CL_A,      CL_SNORM_INT8},
    {CL_A,      CL_SIGNED_INT8},
    {CL_A,      CL_UNORM_INT16},
    {CL_A,      CL_UNSIGNED_INT16},
    {CL_A,      CL_SNORM_INT16},
    {CL_A,      CL_SIGNED_INT16},
    {CL_A,      CL_UNSIGNED_INT32},
    {CL_A,      CL_SIGNED_INT32},
    //  {CL_A,      CL_HALF_FLOAT},
    {CL_A,      CL_FLOAT},

    // CL_INTENSITY
    {CL_INTENSITY,  CL_UNORM_INT8},
    {CL_INTENSITY,  CL_UNORM_INT16},
    {CL_INTENSITY,  CL_SNORM_INT8},
    {CL_INTENSITY,  CL_SNORM_INT16},
    //  {CL_INTENSITY,  CL_HALF_FLOAT},
    {CL_INTENSITY,  CL_FLOAT},

    // CL_LUMINANCE
    {CL_LUMINANCE,  CL_UNORM_INT8},
    {CL_LUMINANCE,  CL_UNORM_INT16},
    {CL_LUMINANCE,  CL_SNORM_INT8},
    {CL_LUMINANCE,  CL_SNORM_INT16},
    //  {CL_LUMINANCE,  CL_HALF_FLOAT},
    {CL_LUMINANCE,  CL_FLOAT},

    // CL_RG
    {CL_RG,     CL_UNORM_INT8},
    {CL_RG,     CL_UNSIGNED_INT8},
    {CL_RG,     CL_SNORM_INT8},
    {CL_RG,     CL_SIGNED_INT8},
    {CL_RG,     CL_UNORM_INT16},
    {CL_RG,     CL_UNSIGNED_INT16},
    {CL_RG,     CL_SNORM_INT16},
    {CL_RG,     CL_SIGNED_INT16},
    {CL_RG,     CL_UNSIGNED_INT32},
    {CL_RG,     CL_SIGNED_INT32},
    //  {CL_RG,     CL_HALF_FLOAT},
    {CL_RG,     CL_FLOAT},

    // CL_RA
    {CL_RA,     CL_UNORM_INT8},
    {CL_RA,     CL_UNSIGNED_INT8},
    {CL_RA,     CL_SNORM_INT8},
    {CL_RA,     CL_SIGNED_INT8},
    {CL_RA,     CL_UNORM_INT16},
    {CL_RA,     CL_UNSIGNED_INT16},
    {CL_RA,     CL_SNORM_INT16},
    {CL_RA,     CL_SIGNED_INT16},
    {CL_RA,     CL_UNSIGNED_INT32},
    {CL_RA,     CL_SIGNED_INT32},
    //  {CL_RA,     CL_HALF_FLOAT},
    {CL_RA,     CL_FLOAT},

    // CL_RGB
    {CL_RGB,    CL_UNORM_SHORT_555},
    {CL_RGB,    CL_UNORM_SHORT_565},
    {CL_RGB,    CL_UNORM_INT_101010},

    // CL_RGBA
    {CL_RGBA, CL_SNORM_INT8},
    {CL_RGBA, CL_SNORM_INT16},

    // CL_BGRA
    {CL_BGRA,   CL_SNORM_INT8},
    {CL_BGRA,   CL_SIGNED_INT8},
    {CL_BGRA,   CL_UNSIGNED_INT8},

    // CL_ARGB
    {CL_ARGB,   CL_UNORM_INT8},
    {CL_ARGB,   CL_SNORM_INT8},
    {CL_ARGB,   CL_SIGNED_INT8},
    {CL_ARGB,   CL_UNSIGNED_INT8},
#endif

#else
    //  {CL_RG,     CL_UNORM_INT8},
    {CL_RGB,  CL_UNORM_SHORT_555},
    /*  // CL_INTENSITY
    {CL_INTENSITY,  CL_UNORM_INT8},
    {CL_INTENSITY,  CL_UNORM_INT16},
    {CL_INTENSITY,  CL_SNORM_INT8},
    {CL_INTENSITY,  CL_SNORM_INT16},
    {CL_INTENSITY,  CL_HALF_FLOAT},
    {CL_INTENSITY,  CL_FLOAT},
    */
#endif
};

#define CPU_DEVICE_NAME_SIZE    48

const unsigned int Intel::OpenCL::CPUDevice::NUM_OF_SUPPORTED_IMAGE_FORMATS =
sizeof(Intel::OpenCL::CPUDevice::supportedImageFormats)/sizeof(cl_image_format);

using namespace Intel::OpenCL::CPUDevice;

const char* Intel::OpenCL::CPUDevice::VENDOR_STRING = "Intel(R) Corporation";

// We put it here, because just here all the required macros are defined.
#include "ocl_supported_extensions.h"

static const size_t CPU_MAX_WORK_ITEM_SIZES[CPU_MAX_WORK_ITEM_DIMENSIONS] =
    {
    CPU_MAX_WORK_GROUP_SIZE,
    CPU_MAX_WORK_GROUP_SIZE,
    CPU_MAX_WORK_GROUP_SIZE
    };

static const cl_device_partition_property_ext CPU_SUPPORTED_FISSION_MODES[] = 
    {
        CL_DEVICE_PARTITION_BY_AFFINITY_DOMAIN_EXT,
        CL_DEVICE_PARTITION_BY_COUNTS_EXT,
        CL_DEVICE_PARTITION_EQUALLY_EXT
    };

static const cl_device_partition_property_ext CPU_SUPPORTED_AFFINITY_DOMAINS[] =
    {
        CL_AFFINITY_DOMAIN_NUMA_EXT
    };

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

typedef struct _cl_dev_internal_subdevice_id
{
	//Arch. data
	cl_uint  num_compute_units;
	cl_uint  numa_id;
	bool     is_numa;
	bool     is_by_names;
	cl_uint* legal_core_ids;

	//Task dispatcher for this sub-device
	TaskDispatcher*                     task_dispatcher;
	Intel::OpenCL::Utils::AtomicCounter task_dispatcher_ref_count;
	volatile bool                       task_dispatcher_init_complete;
} cl_dev_internal_subdevice_id;

typedef struct _cl_dev_internal_cmd_list
{
    void*                         cmd_list;     
    cl_dev_internal_subdevice_id* subdevice_id;
	TaskDispatcher*               task_dispatcher;
} cl_dev_internal_cmd_list;

//A data structure keeping track of which thread ID is which OS-dependent ID and which core it's mapped to
struct Intel::OpenCL::CPUDevice::ThreadMapping
{
	// Int so -1 means no affinity for this thread yet
	int core;

	// The OS-dependent per-thread ID
	threadid_t os_tid;

	// A flag indicating whether this core is a part of a sub-device (and thus has a thread pinned to it)
	bool core_in_use;
};


CPUDevice::CPUDevice(cl_uint uiDevId, IOCLFrameworkCallbacks *devCallbacks, IOCLDevLogDescriptor *logDesc): 
    m_pProgramService(NULL),
    m_pMemoryAllocator(NULL),
    m_pTaskDispatcher(NULL),
    m_pCPUDeviceConfig(NULL), 
    m_pFrameworkCallBacks(devCallbacks), 
    m_uiCpuId(uiDevId),
    m_pLogDescriptor(logDesc), 
    m_iLogHandle (0),
	m_pComputeUnitMap(NULL),
	m_pComputeUnitScoreboard(NULL),
	m_pCoreToThread(NULL)
{
}

cl_dev_err_code CPUDevice::Init()
{
    cl_dev_err_code ret = CL_DEV_SUCCESS;
    if ( NULL != m_pLogDescriptor )
    {
        ret = (cl_dev_err_code)m_pLogDescriptor->clLogCreateClient(m_uiCpuId, L"CPU Device", &m_iLogHandle);
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

    CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("CreateDevice function enter"));

    m_pProgramService = new ProgramService(m_uiCpuId, 
                                           m_pFrameworkCallBacks, 
                                           m_pLogDescriptor, 
                                           m_pCPUDeviceConfig,
                                           m_backendWrapper.GetBackendFactory());
    ret = m_pProgramService->Init();
    if (CL_DEV_SUCCESS != ret)
    {
        return CL_DEV_ERROR_FAIL;
    }

	ret = QueryHWInfo();
    if (CL_DEV_SUCCESS != ret)
    {
        return CL_DEV_ERROR_FAIL;
    }

    m_pMemoryAllocator = new MemoryAllocator(m_uiCpuId, m_pLogDescriptor, MAX_MEM_ALLOC_SIZE, m_pProgramService->GetImageService());
    m_pTaskDispatcher = new TaskDispatcher(m_uiCpuId, m_pFrameworkCallBacks, m_pProgramService,
    m_pMemoryAllocator, m_pLogDescriptor, m_pCPUDeviceConfig, this);
    if ( (NULL == m_pMemoryAllocator) || (NULL == m_pTaskDispatcher) )
    {
        return CL_DEV_OUT_OF_MEMORY;
    }
    
    ret = m_pTaskDispatcher->init();
    if ( CL_DEV_FAILED(ret) )
    {
    	return ret;
    }
    ret = clDevCreateCommandList(CL_DEV_LIST_ENABLE_OOO, 0, &m_defaultCommandList);

    return ret;
}

cl_dev_err_code CPUDevice::QueryHWInfo()
{
	m_numCores        = GetNumberOfProcessors();
	m_numNumaNodes    = GetMaxNumaNode() + 1;
	m_numCoresPerL1   = 2; //Todo: calculate
	m_pComputeUnitMap = new unsigned int[m_numCores];
	if (NULL == m_pComputeUnitMap)
	{
		return CL_DEV_OUT_OF_MEMORY;
	}
	m_pComputeUnitScoreboard = new ThreadMapping[m_numCores];
	if (NULL == m_pComputeUnitScoreboard)
	{
		delete[] m_pComputeUnitMap;
		return CL_DEV_OUT_OF_MEMORY;
	}
	m_pCoreToThread = new unsigned int[m_numCores];
	if (NULL == m_pCoreToThread)
	{
		delete[] m_pComputeUnitScoreboard;
		delete[] m_pComputeUnitMap;
	}

	//Todo: calculate the real map here
	for (unsigned long i = 0; i < m_numCores; ++i)
	{
		m_pComputeUnitMap[i]                    = i;
		m_pCoreToThread[i]                      = i;
		m_pComputeUnitScoreboard[i].os_tid      = 0;
		m_pComputeUnitScoreboard[i].core_in_use = false;
		m_pComputeUnitScoreboard[i].core        = -1;
	}
	return CL_DEV_SUCCESS;
}

bool CPUDevice::AcquireComputeUnits(unsigned int* which, unsigned int how_many)
{
	if ((NULL == which) || (0 == how_many))
	{
		return false;
	}
	Intel::OpenCL::Utils::OclAutoMutex CS(&m_ComputeUnitScoreboardMutex);
	for (unsigned int i = 0; i < how_many; ++i)
	{
		if (m_pComputeUnitScoreboard[which[i]].core_in_use)
		{
			//Failed, roll back
			for (unsigned int j = 0; j < i; ++j)
			{
				m_pComputeUnitScoreboard[which[j]].core_in_use = false;
			}
			return false;
		}
		else
		{
			m_pComputeUnitScoreboard[which[i]].core_in_use = true;
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
		m_pComputeUnitScoreboard[which[i]].core_in_use = false;
	}
}

void CPUDevice::NotifyAffinity(unsigned int tid, unsigned int core)
{
	Intel::OpenCL::Utils::OclAutoMutex CS(&m_ComputeUnitScoreboardMutex);
	unsigned int other_tid    = m_pCoreToThread[core];
	int          my_prev_core = m_pComputeUnitScoreboard[tid].core;
	bool         other_valid  = (other_tid != tid) && (-1 != m_pComputeUnitScoreboard[other_tid].core);

	if (0 == m_pComputeUnitScoreboard[tid].os_tid)
	{
	    m_pComputeUnitScoreboard[tid].os_tid = clMyThreadId();
	}

	//Update map with regard to myself
	m_pComputeUnitScoreboard[tid].core   = core;
	m_pCoreToThread[core]                = tid;

        //Set the caller's affinity as requested
        clSetThreadAffinityToCore(core, m_pComputeUnitScoreboard[tid].os_tid);

	if (other_valid)
	{
		//Need to relocate the other thread to my previous core
		m_pComputeUnitScoreboard[other_tid].core = my_prev_core;
		m_pCoreToThread[my_prev_core]            = other_tid;
		clSetThreadAffinityToCore(my_prev_core, m_pComputeUnitScoreboard[other_tid].os_tid);
	}
}

CPUDevice::~CPUDevice()
{
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
        param                   An enumeration that identifies the device information being queried. It can be one of
                                the following values as specified in OCL spec. table 4.3
        valSize             Specifies the size in bytes of memory pointed to by paramValue. This size in
                                bytes must be >= size of return type
    Output
        paramVal                A pointer to memory location where appropriate values for a given param as specified in OCL spec. table 4.3 will be returned. If paramVal is NULL, it is ignored
        paramValSize_ret        Returns the actual size in bytes of data being queried by paramVal. If paramValSize_ret is NULL, it is ignored
    Returns
        CL_DEV_SUCCESS          If functions is executed successfully.
        CL_DEV_INVALID_VALUE    If param_name is not one of the supported values or if size in bytes specified by paramValSize is < size of return type as specified in OCL spec. table 4.3 and paramVal is not a NULL value
**************************************************************************************************************************/
cl_dev_err_code CPUDevice::clDevGetDeviceInfo(cl_device_info IN param, size_t IN valSize, void* OUT paramVal,
                size_t* OUT paramValSizeRet)
{
    size_t  internalRetunedValueSize = valSize;
    size_t  *pinternalRetunedValueSize;
    int     viCPUInfo[4] = {-1};

    //if both paramVal and paramValSize is NULL return error
    if(NULL == paramVal && NULL == paramValSizeRet)
    {
        return CL_DEV_INVALID_VALUE;
    }
    //if OUT paramValSize_ret is NULL it should be ignopred
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

        case( CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR):
        case( CL_DEVICE_NATIVE_VECTOR_WIDTH_CHAR ):
        {
            *pinternalRetunedValueSize = sizeof(cl_uint);
            if(NULL != paramVal && valSize < *pinternalRetunedValueSize)
            {
                return CL_DEV_INVALID_VALUE;
            }
            //if OUT paramVal is NULL it should be ignored
            if(NULL != paramVal)
            {
                *(cl_uint*)paramVal = 16;
            }
            return CL_DEV_SUCCESS;
        }
        case( CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT):
        case( CL_DEVICE_NATIVE_VECTOR_WIDTH_SHORT):
        {
            *pinternalRetunedValueSize = sizeof(cl_uint);
            if(NULL != paramVal && valSize < *pinternalRetunedValueSize)
            {
                return CL_DEV_INVALID_VALUE;
            }
            //if OUT paramVal is NULL it should be ignored
            if(NULL != paramVal)
            {
                *(cl_uint*)paramVal = 8;
            }
            return CL_DEV_SUCCESS;
        }
        case( CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT):// FALL THROUGH
        case( CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT):
        case( CL_DEVICE_NATIVE_VECTOR_WIDTH_FLOAT):// FALL THROUGH
        case( CL_DEVICE_NATIVE_VECTOR_WIDTH_INT):
        {
            *pinternalRetunedValueSize = sizeof(cl_uint);
            if(NULL != paramVal && valSize < *pinternalRetunedValueSize)
            {
                return CL_DEV_INVALID_VALUE;
            }
            //if OUT paramVal is NULL it should be ignored
            if(NULL != paramVal)
            {
                *(cl_uint*)paramVal = 4;
            }
            return CL_DEV_SUCCESS;
        }

        case( CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG): 
        case( CL_DEVICE_NATIVE_VECTOR_WIDTH_LONG): 
            {
                *pinternalRetunedValueSize = sizeof(cl_uint);
                if(NULL != paramVal && valSize < *pinternalRetunedValueSize)
                {
                    return CL_DEV_INVALID_VALUE;
                }
                //if OUT paramVal is NULL it should be ignored
                if(NULL != paramVal)
                {
                    *(cl_uint*)paramVal = 2;
                }
                return CL_DEV_SUCCESS;
            }
        case( CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE):
        case( CL_DEVICE_NATIVE_VECTOR_WIDTH_DOUBLE):
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
                *(cl_uint*)paramVal = 2;
#else
                *(cl_uint*)paramVal = 0;
#endif
            }
            return CL_DEV_SUCCESS;
        }
        case( CL_DEVICE_PREFERRED_VECTOR_WIDTH_HALF ):
        case( CL_DEVICE_NATIVE_VECTOR_WIDTH_HALF ):
        {
            *pinternalRetunedValueSize = sizeof(cl_uint);
            if(NULL != paramVal && valSize < *pinternalRetunedValueSize)
            {
                return CL_DEV_INVALID_VALUE;
            }
            //if OUT paramVal is NULL it should be ignored
            if(NULL != paramVal)
            {
                *(cl_uint*)paramVal = 0;    // TODO: change when halfs are supported
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
            cl_device_fp_config fpConfig = CL_FP_ROUND_TO_NEAREST | CL_FP_INF_NAN | CL_FP_DENORM | CL_FP_FMA |  CL_FP_ROUND_TO_ZERO |  CL_FP_ROUND_TO_INF;
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
        case( CL_DEVICE_QUEUE_PROPERTIES ):
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
        case( CL_DEVICE_COMPILER_AVAILABLE):
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
            *pinternalRetunedValueSize = CPU_DEVICE_NAME_SIZE; // Already counts for \0
            if(NULL != paramVal && valSize < *pinternalRetunedValueSize)
            {
                return CL_DEV_INVALID_VALUE;
            }
            //if OUT paramVal is NULL it should be ignored
            if(NULL != paramVal)
            {
                STRCPY_S((char*)paramVal, valSize, CPUDetect::GetInstance()->GetCPUBrandString());
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
                *pinternalRetunedValueSize = strlen("OpenCL C 1.1 ") + 1;
                if(NULL != paramVal && valSize < *pinternalRetunedValueSize)
                {
                    return CL_DEV_INVALID_VALUE;
                }
                //if OUT paramVal is NULL it should be ignored
                if(NULL != paramVal)
                {
                    STRCPY_S((char*)paramVal, valSize, "OpenCL C 1.1 ");
                }
                return CL_DEV_SUCCESS;
            }
        case( CL_DEVICE_VERSION):
        {
            *pinternalRetunedValueSize = strlen("OpenCL 1.1 ") + strlen(BUILDVERSIONSTR) + 1;
            if(NULL != paramVal && valSize < *pinternalRetunedValueSize)
            {
                return CL_DEV_INVALID_VALUE;
            }
            //if OUT paramVal is NULL it should be ignored
            if(NULL != paramVal)
            {
                STRCPY_S((char*)paramVal, valSize, "OpenCL 1.1 ");
                STRCAT_S((char*)paramVal, valSize, BUILDVERSIONSTR);
            }
            return CL_DEV_SUCCESS;
        }
        case( CL_DRIVER_VERSION ):
        {
            *pinternalRetunedValueSize = strlen("1.1") + 1;
            if(NULL != paramVal && valSize < *pinternalRetunedValueSize)
            {
                return CL_DEV_INVALID_VALUE;
            }
            //if OUT paramVal is NULL it should be ignored
            if(NULL != paramVal)
            {
                STRCPY_S((char*)paramVal, valSize, "1.1");
            }
            return CL_DEV_SUCCESS;
        }
        case( CL_DEVICE_EXTENSIONS):
        {
            *pinternalRetunedValueSize = strlen(OCL_SUPPORTED_EXTENSIONS) + 1;
            if(NULL != paramVal && valSize < *pinternalRetunedValueSize)
            {
                return CL_DEV_INVALID_VALUE;
            }
            //if OUT paramVal is NULL it should be ignored
            if(NULL != paramVal)
            {
                STRCPY_S((char*)paramVal, valSize, OCL_SUPPORTED_EXTENSIONS);
            }
            return CL_DEV_SUCCESS;

        }

        case CL_DEVICE_PARTITION_TYPES_EXT:
            *pinternalRetunedValueSize = sizeof(CPU_SUPPORTED_FISSION_MODES);
            if(NULL != paramVal && valSize < *pinternalRetunedValueSize)
            {
                return CL_DEV_INVALID_VALUE;
            }
            //if OUT paramVal is NULL it should be ignored
            if(NULL != paramVal)
            {
                MEMCPY_S(paramVal, sizeof(CPU_SUPPORTED_FISSION_MODES), CPU_SUPPORTED_FISSION_MODES, sizeof(CPU_SUPPORTED_FISSION_MODES));
            }
            return CL_DEV_SUCCESS;

        case CL_DEVICE_AFFINITY_DOMAINS_EXT:
            {
#ifndef _WIN32 //Numa support disabled for windows machines
                //No NUMA -> return a 0 sized array
                if (0 == GetMaxNumaNode())
#endif
                {
                    *pinternalRetunedValueSize = 0;
                    return CL_DEV_SUCCESS;
                }
                //else, we support NUMA
                *pinternalRetunedValueSize = 1;
                if(NULL != paramVal && valSize < *pinternalRetunedValueSize)
                {
                    return CL_DEV_INVALID_VALUE;
                }
                //if OUT paramVal is NULL it should be ignored
                if(NULL != paramVal)
                {
                    *((cl_device_partition_property_ext*)paramVal) = CPU_SUPPORTED_AFFINITY_DOMAINS[0];
                }
                return CL_DEV_SUCCESS;
            }
            break;
#if 0   // disabled until changes in the spec regarding 2D image arrays are made
		case CL_DEVICE_IMAGE_ARRAY_MAX_SIZE:
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
#endif
        default:
            return CL_DEV_INVALID_VALUE;
    };
    return CL_DEV_SUCCESS;

}
// Device Fission support

static void rollBackSubdeviceAllocation(cl_dev_subdevice_id* IN subdevice_ids, cl_uint num_successfully_allocated)
{
        for (cl_uint j = 0; j < num_successfully_allocated; ++j)
        {
			cl_dev_internal_subdevice_id* pSubdeviceId = static_cast<cl_dev_internal_subdevice_id*>(subdevice_ids[j]);
			delete[] pSubdeviceId->legal_core_ids;
            delete pSubdeviceId;
        }
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
	unsigned long numNumaNodes = m_numNumaNodes;

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
            if (partitionSize >= availableComputeUnits)
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
            for (cl_uint i = 0; i < (cl_uint)numPartitions; ++i)
            {
				cl_dev_internal_subdevice_id* pNewsubdeviceId = new cl_dev_internal_subdevice_id;
				if (NULL != pNewsubdeviceId)
				{
					pNewsubdeviceId->legal_core_ids = new unsigned int[partitionSize];
				}
			
                if ((NULL == pNewsubdeviceId) || (NULL == pNewsubdeviceId->legal_core_ids))
                {
					if (NULL != pNewsubdeviceId) delete pNewsubdeviceId;
					rollBackSubdeviceAllocation(subdevice_ids, i);
                    return CL_DEV_OUT_OF_MEMORY;
                }
				if (NULL == pParent)
				{
    				pNewsubdeviceId->is_numa     = false;
	    			pNewsubdeviceId->numa_id     = 0;
				}
				else
				{
					//Disallow re-partitioning devices BY_NAMES
					if (pParent->is_by_names)
					{
						return CL_DEV_NOT_SUPPORTED;
					}
					pNewsubdeviceId->is_numa     = pParent->is_numa;
					pNewsubdeviceId->numa_id     = pParent->numa_id;
				}
				pNewsubdeviceId->is_by_names       = false;
				pNewsubdeviceId->num_compute_units = (cl_uint)partitionSize;
				pNewsubdeviceId->task_dispatcher_init_complete = false;
				pNewsubdeviceId->task_dispatcher_ref_count      = 0;
				pNewsubdeviceId->task_dispatcher                = NULL;
				//Todo: add affinity mapper query here
				for (cl_uint core = 0; core < partitionSize; ++core)
				{
				    pNewsubdeviceId->legal_core_ids[core] = (cl_uint)(i * partitionSize + core);
				}
				subdevice_ids[i] = pNewsubdeviceId;
            }
            return CL_DEV_SUCCESS;
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
			//Disallow partitions that equal the sub-device
            if (totalNumUnits >= availableComputeUnits)
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

			//Todo: remove
			int tempCoreId = 0;
            for (cl_uint i = 0; i < num_subdevices_to_create; ++i)
            {
				cl_dev_internal_subdevice_id* pNewsubdeviceId = new cl_dev_internal_subdevice_id;
				if (NULL != pNewsubdeviceId)
				{
					pNewsubdeviceId->legal_core_ids = new unsigned int[partitionSizes[i]];
				}
			
                if ((NULL == pNewsubdeviceId) || (NULL == pNewsubdeviceId->legal_core_ids))
                {
					if (NULL != pNewsubdeviceId) delete pNewsubdeviceId;
					rollBackSubdeviceAllocation(subdevice_ids, i);
                    return CL_DEV_OUT_OF_MEMORY;
                }
				if (NULL == pParent)
				{
    				pNewsubdeviceId->is_numa     = false;
	    			pNewsubdeviceId->numa_id     = 0;
				}
				else
				{
					//Disallow re-partitioning devices BY_NAMES
					if (pParent->is_by_names)
					{
						return CL_DEV_NOT_SUPPORTED;
					}
					pNewsubdeviceId->is_numa     = pParent->is_numa;
					pNewsubdeviceId->numa_id     = pParent->numa_id;
				}
				pNewsubdeviceId->is_by_names       = false;
				pNewsubdeviceId->num_compute_units = (cl_uint)partitionSizes[i];
				pNewsubdeviceId->task_dispatcher_init_complete = false;
				pNewsubdeviceId->task_dispatcher_ref_count      = 0;
				pNewsubdeviceId->task_dispatcher                = NULL;
				//Todo: add affinity mapper query here
				for (cl_uint core = 0; core < partitionSizes[i]; ++core)
				{
				    pNewsubdeviceId->legal_core_ids[core] = tempCoreId++;
				}
				subdevice_ids[i] = pNewsubdeviceId;
            }
            return CL_DEV_SUCCESS;
        }

    case CL_DEV_PARTITION_AFFINITY_NUMA:
        {
            if (numNumaNodes <= 1)
            {
                return CL_DEV_INVALID_VALUE;
            }
			if (NULL != pParent)
			{
				//Disallow a partition that equals the entire device
				if (pParent->is_numa || pParent->is_by_names)
				{
					return CL_DEV_INVALID_VALUE;
				}
			}
            if (NULL == subdevice_ids)
            {
                *num_subdevices = numNumaNodes;
                return CL_DEV_SUCCESS;
            }
            if (*num_subdevices < numNumaNodes)
            {
                return CL_DEV_INVALID_VALUE;
            }
            *num_subdevices = numNumaNodes;
            cl_uint num_subdevices_to_create = *num_subdevices;
            if (num_subdevices_to_create > num_requested_subdevices)
            {
                num_subdevices_to_create = num_requested_subdevices;
            }

            //Todo: currently I assume NUMA systems are symmetric rather than counting bits
            //size_t processorSize = GetNumberOfProcessors() / (maxNumaNode + 1);
			size_t processorsPerNode  = availableComputeUnits / numNumaNodes;
			size_t leftoverProcessors = availableComputeUnits % numNumaNodes;
            cl_uint i;
			cl_uint lastUsedIdx = 0;
            for (i = 0; i < num_subdevices_to_create; ++i)
            {
				size_t extraProcessors = i < leftoverProcessors ? 1 : 0;
				cl_dev_internal_subdevice_id* pNewsubdeviceId = new cl_dev_internal_subdevice_id;
				if (NULL == pNewsubdeviceId)
				{
					rollBackSubdeviceAllocation(subdevice_ids, i);
					return CL_DEV_OUT_OF_MEMORY;
				}

				pNewsubdeviceId->num_compute_units = (cl_uint)(processorsPerNode + extraProcessors);
			    pNewsubdeviceId->is_numa           = true;
			    pNewsubdeviceId->numa_id           = i;
				pNewsubdeviceId->is_by_names       = false;
				pNewsubdeviceId->legal_core_ids    = new unsigned int[pNewsubdeviceId->num_compute_units];
				pNewsubdeviceId->task_dispatcher_init_complete = false;
				pNewsubdeviceId->task_dispatcher_ref_count      = 0;
				pNewsubdeviceId->task_dispatcher                = NULL;
				if (NULL == pNewsubdeviceId->legal_core_ids)
				{
					delete pNewsubdeviceId;
					rollBackSubdeviceAllocation(subdevice_ids, i);
				}
				for (unsigned int k = 0; k < pNewsubdeviceId->num_compute_units; ++k)
				{
					pNewsubdeviceId->legal_core_ids[k] = pParentComputeUnits[k + lastUsedIdx];
				}
				lastUsedIdx += pNewsubdeviceId->num_compute_units;

				subdevice_ids[i] = pNewsubdeviceId;

			}
            return CL_DEV_SUCCESS;
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

			pNewsubdeviceId->is_numa           = false;
   			pNewsubdeviceId->numa_id           = 0;
			pNewsubdeviceId->is_by_names       = true;
			pNewsubdeviceId->num_compute_units = (cl_uint)totalNumUnits;
			for (cl_uint core = 0; core < totalNumUnits; ++core)
			{
			    pNewsubdeviceId->legal_core_ids[core] = (cl_uint)requestedUnits[core];
			}
			*subdevice_ids = pNewsubdeviceId;

            return CL_DEV_SUCCESS;
        }
    //Non-supported modes
    case CL_DEV_PARTITION_AFFINITY_L1:
    case CL_DEV_PARTITION_AFFINITY_L2:
    case CL_DEV_PARTITION_AFFINITY_L3:
    case CL_DEV_PARTITION_AFFINITY_L4:
    case CL_DEV_PARTITION_AFFINITY_NEXT:
        return CL_DEV_INVALID_PROPERTIES;
    default:
        return CL_DEV_INVALID_VALUE;

    }
    return CL_DEV_INVALID_VALUE;
}
/****************************************************************************************************************
 clDevReleaseSubdevice
    Release a subdevice created by a clDevPartition call. Releases the appropriate SubdeviceTaskDispatcher object
********************************************************************************************************************/
cl_dev_err_code CPUDevice::clDevReleaseSubdevice(  cl_dev_subdevice_id IN subdevice_id)
{
    if (NULL == subdevice_id)
    {
        return CL_DEV_INVALID_VALUE;
    }
	cl_dev_internal_subdevice_id* pSubdeviceData = static_cast<cl_dev_internal_subdevice_id*>(subdevice_id);
	if (NULL != pSubdeviceData)
	{
		delete pSubdeviceData->legal_core_ids;
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
    CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("clDevCreateCommandList Function enter"));
    cl_dev_internal_cmd_list* pList = new cl_dev_internal_cmd_list();
    if (NULL == pList)
    {
        return CL_DEV_OUT_OF_MEMORY;
    }
	cl_dev_internal_subdevice_id* pSubdeviceData = static_cast<cl_dev_internal_subdevice_id*>(subdevice_id);
    pList->subdevice_id    = pSubdeviceData;
	pList->task_dispatcher = m_pTaskDispatcher;

	if (NULL != pSubdeviceData)
	{
		long prev = pSubdeviceData->task_dispatcher_ref_count++;
		if (0 == prev)
		{
			//Need to create a new sub-device task dispatcher
			if (!AcquireComputeUnits(pSubdeviceData->legal_core_ids, pSubdeviceData->num_compute_units))
			{
				delete pList;
				pSubdeviceData->task_dispatcher_init_complete = true;
				return CL_DEV_ERROR_FAIL;
			}

			TaskDispatcher* pNewTaskDispatcher = new SubdeviceTaskDispatcher(pSubdeviceData->num_compute_units, pSubdeviceData->legal_core_ids, m_uiCpuId, m_pFrameworkCallBacks, m_pProgramService, m_pMemoryAllocator, m_pLogDescriptor, m_pCPUDeviceConfig, this);
			if (NULL == pNewTaskDispatcher)
			{
				delete pList;
				pSubdeviceData->task_dispatcher_init_complete = true;
				return CL_DEV_OUT_OF_MEMORY;
			}
			cl_dev_err_code ret = pNewTaskDispatcher->init();
			if (CL_DEV_FAILED(ret))
			{
				delete pNewTaskDispatcher;
				delete pList;
				pSubdeviceData->task_dispatcher_init_complete = true;
				return CL_DEV_OUT_OF_MEMORY;
			}
			pSubdeviceData->task_dispatcher = pNewTaskDispatcher;
			pList->task_dispatcher          = pNewTaskDispatcher;
			pSubdeviceData->task_dispatcher_init_complete = true;
		}
		else 
		{
			while (!pSubdeviceData->task_dispatcher_init_complete)
			{
				//Todo: not a good spin loop methodology
				clSleep(0);
			}
			if (NULL == pSubdeviceData->task_dispatcher)
			{
				//Can happen if init failed on the other thread
				return CL_DEV_OUT_OF_MEMORY;
			}
			pList->task_dispatcher = pList->subdevice_id->task_dispatcher;
		}
	}

    cl_dev_err_code ret  = pList->task_dispatcher->createCommandList(props,&pList->cmd_list);
    *list = pList;
    return ret;
}
/****************************************************************************************************************
 clDevFlushCommandList
    Call TaskDispatcher to flush command list
********************************************************************************************************************/
cl_dev_err_code CPUDevice::clDevFlushCommandList( cl_dev_cmd_list IN list)
{
    CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("clDevFlushCommandList Function enter"));
    cl_dev_internal_cmd_list* pList = static_cast<cl_dev_internal_cmd_list*>(list);
    if (NULL == pList)
    {
        return CL_DEV_INVALID_VALUE;
    }
	return pList->task_dispatcher->flushCommandList(pList->cmd_list);
}
/****************************************************************************************************************
 clDevRetainCommandList
    Call TaskDispatcher to retain command list
********************************************************************************************************************/
cl_dev_err_code CPUDevice::clDevRetainCommandList( cl_dev_cmd_list IN list)
{
    CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("clDevRetainCommandList Function enter"));
    cl_dev_internal_cmd_list* pList = static_cast<cl_dev_internal_cmd_list*>(list);
    if (NULL == pList)
    {
        return CL_DEV_INVALID_VALUE;
    }
    return pList->task_dispatcher->retainCommandList(pList->cmd_list);
}
/****************************************************************************************************************
 clDevReleaseCommandList
    Call TaskDispatcher to release command list
********************************************************************************************************************/
cl_dev_err_code CPUDevice::clDevReleaseCommandList( cl_dev_cmd_list IN list )
{
    CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("clDevReleaseCommandList Function enter"));
    cl_dev_internal_cmd_list* pList = static_cast<cl_dev_internal_cmd_list*>(list);
    if (NULL == pList)
    {
        return CL_DEV_INVALID_VALUE;
    }
	cl_dev_err_code ret = pList->task_dispatcher->releaseCommandList(pList->cmd_list);
	if (CL_DEV_FAILED(ret))
	{
		return ret;
	}
	if (NULL != pList->subdevice_id)
	{
		long prev = pList->subdevice_id->task_dispatcher_ref_count--;
		if (1 == prev)
		{
			//Need to also delete the corresponding subdevice task dispatcher
			delete pList->task_dispatcher;
			ReleaseComputeUnits(pList->subdevice_id->legal_core_ids, pList->subdevice_id->num_compute_units);
			pList->subdevice_id->task_dispatcher = NULL;
		}
	}
	delete pList;
	return CL_DEV_SUCCESS;
}
/****************************************************************************************************************
 clDevCommandListExecute
    Call TaskDispatcher to execute command list
********************************************************************************************************************/
cl_dev_err_code CPUDevice::clDevCommandListExecute( cl_dev_cmd_list IN list, cl_dev_cmd_desc* IN *cmds, cl_uint IN count)
{
    CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("clDevCommandListExecute Function enter"));
    if (NULL != list)
    {
        cl_dev_internal_cmd_list* pList = static_cast<cl_dev_internal_cmd_list*>(list);
        return pList->task_dispatcher->commandListExecute(pList->cmd_list,cmds,count);
    }
    else
    {
        cl_dev_internal_cmd_list* pList = static_cast<cl_dev_internal_cmd_list*>(m_defaultCommandList);
        cl_dev_err_code ret = m_pTaskDispatcher->commandListExecute(pList->cmd_list,cmds,count);
        if (CL_DEV_FAILED(ret))
        {
            return ret;
        }
        return m_pTaskDispatcher->flushCommandList(pList->cmd_list);
    }
}

/****************************************************************************************************************
 clDevCommandListExecute
    Call clDevCommandListWaitCompletion to add calling thread to execution pool
********************************************************************************************************************/
cl_dev_err_code CPUDevice::clDevCommandListWaitCompletion(cl_dev_cmd_list IN list)
{
    CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("clDevCommandListWaitCompletion Function enter"));
    cl_dev_internal_cmd_list* pList = static_cast<cl_dev_internal_cmd_list*>(list);
    if (NULL == pList)
    {
        return CL_DEV_INVALID_VALUE;
    }
    return pList->task_dispatcher->commandListWaitCompletion(pList->cmd_list);
}

//Memory API's
/****************************************************************************************************************
 clDevGetSupportedImageFormats
    Call Memory Allocator to get supported image formats
********************************************************************************************************************/
cl_dev_err_code CPUDevice::clDevGetSupportedImageFormats( cl_mem_flags IN flags, cl_mem_object_type IN imageType,
                cl_uint IN numEntries, cl_image_format* OUT formats, cl_uint* OUT numEntriesRet)
{
    CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("clDevGetSupportedImageFormats Function enter"));
    return (cl_dev_err_code)m_pProgramService->GetSupportedImageFormats(flags, imageType, numEntries, formats, numEntriesRet);
}

cl_dev_err_code CPUDevice::clDevGetMemoryAllocProperties( cl_mem_object_type IN memObjType, cl_dev_alloc_prop* OUT pAllocProp )
{
    CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("clDevGetMemoryAllocProperties Function enter"));
    return m_pMemoryAllocator->GetAllocProperties(memObjType, pAllocProp);
}

/****************************************************************************************************************
 clDevCreateMemoryObject
    Call Memory Allocator to create memory object
********************************************************************************************************************/
cl_dev_err_code CPUDevice::clDevCreateMemoryObject( cl_dev_subdevice_id node_id, cl_mem_flags IN flags, const cl_image_format* IN format,
                                    size_t  IN dim_count, const size_t* IN dim_size, IOCLDevRTMemObjectService* pRTService, IOCLDevMemoryObject* OUT *memObj)
{
    CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("clDevCreateMemoryObject Function enter"));
    return m_pMemoryAllocator->CreateObject(node_id, flags, format, dim_count, dim_size, pRTService, memObj);
}

/****************************************************************************************************************
 clDevCheckProgramBinary
    Call Program Serice to check binaries
********************************************************************************************************************/
cl_dev_err_code CPUDevice::clDevCheckProgramBinary( size_t IN binSize, const void* IN bin )
{
    CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("clDevCheckProgramBinary Function enter"));
    return (cl_dev_err_code)m_pProgramService->CheckProgramBinary(binSize, bin );
}

/*******************************************************************************************************************
clDevCreateProgram
    Call programService to create program
**********************************************************************************************************************/

cl_dev_err_code CPUDevice::clDevCreateProgram( size_t IN binSize, const void* IN bin, cl_dev_binary_prop IN prop, cl_dev_program* OUT prog )
{
    CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("clDevCreateProgram Function enter"));
    return (cl_dev_err_code)m_pProgramService->CreateProgram(binSize, bin, prop, prog );
}

/*******************************************************************************************************************
clDevBuildProgram
    Call programService to build program
**********************************************************************************************************************/

cl_dev_err_code CPUDevice::clDevBuildProgram( cl_dev_program IN prog, const char* IN options, void* IN userData )
{
    CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("clDevBuildProgram Function enter"));
    return (cl_dev_err_code)m_pProgramService->BuildProgram(prog, options, userData);
}

/*******************************************************************************************************************
clDevReleaseProgram
    Call programService to release program
**********************************************************************************************************************/

cl_dev_err_code CPUDevice::clDevReleaseProgram( cl_dev_program IN prog )
{
    CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("clDevReleaseProgram Function enter"));
    return (cl_dev_err_code)m_pProgramService->ReleaseProgram( prog );
}

/*******************************************************************************************************************
clDevUnloadCompiler
    Call programService to unload the backend compiler
**********************************************************************************************************************/
cl_dev_err_code CPUDevice::clDevUnloadCompiler()
{
    CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("clDevUnloadCompiler Function enter"));
    return (cl_dev_err_code)m_pProgramService->UnloadCompiler();
}
/*******************************************************************************************************************
clDevGetProgramBinary
    Call programService to get the program binary
**********************************************************************************************************************/
cl_dev_err_code CPUDevice::clDevGetProgramBinary( cl_dev_program IN prog, size_t IN size, void* OUT binary, size_t* OUT sizeRet )
{
    CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("clDevGetProgramBinary Function enter"));
    return (cl_dev_err_code)m_pProgramService->GetProgramBinary(prog, size, binary, sizeRet );
}
/*******************************************************************************************************************
clDevGetBuildLog
    Call programService to get the build log
**********************************************************************************************************************/
cl_dev_err_code CPUDevice::clDevGetBuildLog( cl_dev_program IN prog, size_t IN size, char* OUT log, size_t* OUT sizeRet)
{
    CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("clDevGetBuildLog Function enter"));
    return (cl_dev_err_code)m_pProgramService->GetBuildLog(prog, size, log, sizeRet);
}
/*******************************************************************************************************************
clDevUnloadCompiler
    Call programService to get supported binary description
**********************************************************************************************************************/
cl_dev_err_code CPUDevice::clDevGetSupportedBinaries( size_t IN count, cl_prog_binary_desc* OUT types, size_t* OUT sizeRet )
{
    CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("clDevGetSupportedBinaries Function enter"));
    return (cl_dev_err_code)m_pProgramService->GetSupportedBinaries(count,types,sizeRet );
}
/*******************************************************************************************************************
clDevUnloadCompiler
    Call programService to get kernel id from its name
**********************************************************************************************************************/
cl_dev_err_code CPUDevice::clDevGetKernelId( cl_dev_program IN prog, const char* IN name, cl_dev_kernel* OUT kernelId )
{
    CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("clDevGetKernelId Function enter"));
    return (cl_dev_err_code)m_pProgramService->GetKernelId(prog, name, kernelId );
}
/*******************************************************************************************************************
clDevUnloadCompiler
    Call programService to get kernels from the program
**********************************************************************************************************************/
cl_dev_err_code CPUDevice::clDevGetProgramKernels( cl_dev_program IN prog, cl_uint IN numKernels, cl_dev_kernel* OUT kernels,
                         cl_uint* OUT numKernelsRet )
{
    CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("clDevGetProgramKernels Function enter"));
    return (cl_dev_err_code)m_pProgramService->GetProgramKernels(prog, numKernels, kernels,numKernelsRet );
}
/*******************************************************************************************************************
clDevGetKernelInfo
    Call programService to get kernel info
**********************************************************************************************************************/
cl_dev_err_code CPUDevice::clDevGetKernelInfo( cl_dev_kernel IN kernel, cl_dev_kernel_info IN param, size_t IN valueSize,
                    void* OUT value, size_t* OUT valueSizeRet )
{
    CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("clDevGetKernelInfo Function enter"));
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
        cl_dev_err_code ret = (cl_dev_err_code)m_pLogDescriptor->clLogCreateClient(m_uiCpuId, L"CPU Device", &m_iLogHandle);
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
    CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("clCloseDevice Function enter"));

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
        delete m_pTaskDispatcher;
        m_pTaskDispatcher = NULL;
    }

	if ( NULL != m_pComputeUnitMap)
	{
		delete[] m_pComputeUnitMap;
		m_pComputeUnitMap = NULL;
	}
	if ( NULL != m_pComputeUnitScoreboard)
	{
		delete[] m_pComputeUnitScoreboard;
		m_pComputeUnitScoreboard = NULL;
	}
    
    m_backendWrapper.Terminate();

    delete this;
}

const char* CPUDevice::clDevFEModuleName() const
{
	static const char* sFEModuleName = "clang_compiler";
	return sFEModuleName;
}

const void* CPUDevice::clDevFEDeviceInfo() const
{
	return OCL_SUPPORTED_EXTENSIONS;
}

size_t CPUDevice::clDevFEDeviceInfoSize() const
{
	return strlen(OCL_SUPPORTED_EXTENSIONS)+1;
}
