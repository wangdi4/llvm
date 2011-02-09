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
#include "cpu_logger.h"
#include "cl_sys_info.h"
#include "cpu_dev_limits.h"
#include "cl_sys_defines.h"
//#include <intrin.h>
#if defined (__GNUC__) && !(__INTEL_COMPILER)  && !(_WIN32)
#include "hw_utils.h"
#endif

#if defined( _WIN32 )
	#define STRDUP(X) (_strdup(X))
	#define CPUID(cpu_info, type) __cpuid(cpu_info, type)
#else
	#define STRDUP(X) (strdup(X))
#if defined (__INTEL_COMPILER)
	#define CPUID(cpu_info, type) __cpuid(cpu_info, type)
#else
	#define CPUID(cpu_info, type) cpuid(cpu_info, type)
#endif
#endif
#define __DOUBLE_ENABLED__
using namespace Intel::OpenCL::CPUDevice;

char clCPUDEVICE_CFG_PATH[MAX_PATH];

#define __MINUMUM_SUPPORT__
//#define __TEST__
const cl_image_format Intel::OpenCL::CPUDevice::suportedImageFormats[] = {
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
	{CL_BGRA,	CL_UNORM_INT8},

#ifndef __MINUMUM_SUPPORT__
	// CL_R
	{CL_R,		CL_UNORM_INT8},
	{CL_R,		CL_UNORM_INT16},
	{CL_R,		CL_SNORM_INT8},
	{CL_R,		CL_SNORM_INT16},
	{CL_R,		CL_SIGNED_INT8},
	{CL_R,		CL_SIGNED_INT16},
	{CL_R,		CL_SIGNED_INT32},
	{CL_R,		CL_UNSIGNED_INT8},
	{CL_R,		CL_UNSIGNED_INT16},
	{CL_R,		CL_UNSIGNED_INT32},
	//	{CL_R,		CL_HALF_FLOAT},
	{CL_R,		CL_FLOAT},

	// CL_A
	{CL_A,		CL_UNORM_INT8},
	{CL_A,		CL_UNSIGNED_INT8},
	{CL_A,		CL_SNORM_INT8},
	{CL_A,		CL_SIGNED_INT8},
	{CL_A,		CL_UNORM_INT16},
	{CL_A,		CL_UNSIGNED_INT16},
	{CL_A,		CL_SNORM_INT16},
	{CL_A,		CL_SIGNED_INT16},
	{CL_A,		CL_UNSIGNED_INT32},
	{CL_A,		CL_SIGNED_INT32},
	//	{CL_A,		CL_HALF_FLOAT},
	{CL_A,		CL_FLOAT},

	// CL_INTENSITY
	{CL_INTENSITY,	CL_UNORM_INT8},
	{CL_INTENSITY,	CL_UNORM_INT16},
	{CL_INTENSITY,	CL_SNORM_INT8},
	{CL_INTENSITY,	CL_SNORM_INT16},
	//	{CL_INTENSITY,	CL_HALF_FLOAT},
	{CL_INTENSITY,	CL_FLOAT},

	// CL_LUMINANCE
	{CL_LUMINANCE,	CL_UNORM_INT8},
	{CL_LUMINANCE,	CL_UNORM_INT16},
	{CL_LUMINANCE,	CL_SNORM_INT8},
	{CL_LUMINANCE,	CL_SNORM_INT16},
	//	{CL_LUMINANCE,	CL_HALF_FLOAT},
	{CL_LUMINANCE,	CL_FLOAT},

	// CL_RG
	{CL_RG,		CL_UNORM_INT8},
	{CL_RG,		CL_UNSIGNED_INT8},
	{CL_RG,		CL_SNORM_INT8},
	{CL_RG,		CL_SIGNED_INT8},
	{CL_RG,		CL_UNORM_INT16},
	{CL_RG,		CL_UNSIGNED_INT16},
	{CL_RG,		CL_SNORM_INT16},
	{CL_RG,		CL_SIGNED_INT16},
	{CL_RG,		CL_UNSIGNED_INT32},
	{CL_RG,		CL_SIGNED_INT32},
	//	{CL_RG,		CL_HALF_FLOAT},
	{CL_RG,		CL_FLOAT},

	// CL_RA
	{CL_RA,		CL_UNORM_INT8},
	{CL_RA,		CL_UNSIGNED_INT8},
	{CL_RA,		CL_SNORM_INT8},
	{CL_RA,		CL_SIGNED_INT8},
	{CL_RA,		CL_UNORM_INT16},
	{CL_RA,		CL_UNSIGNED_INT16},
	{CL_RA,		CL_SNORM_INT16},
	{CL_RA,		CL_SIGNED_INT16},
	{CL_RA,		CL_UNSIGNED_INT32},
	{CL_RA,		CL_SIGNED_INT32},
	//	{CL_RA,		CL_HALF_FLOAT},
	{CL_RA,		CL_FLOAT},

	// CL_RGB
	{CL_RGB,	CL_UNORM_SHORT_555},
	{CL_RGB,	CL_UNORM_SHORT_565},
	{CL_RGB,	CL_UNORM_INT_101010},

	// CL_RGBA
	{CL_RGBA, CL_SNORM_INT8},
	{CL_RGBA, CL_SNORM_INT16},

	// CL_BGRA
	{CL_BGRA,	CL_SNORM_INT8},
	{CL_BGRA,	CL_SIGNED_INT8},
	{CL_BGRA,	CL_UNSIGNED_INT8},

	// CL_ARGB
	{CL_ARGB,	CL_UNORM_INT8},
	{CL_ARGB,	CL_SNORM_INT8},
	{CL_ARGB,	CL_SIGNED_INT8},
	{CL_ARGB,	CL_UNSIGNED_INT8},
#endif

#else
	//	{CL_RG,		CL_UNORM_INT8},
	{CL_RGB,  CL_UNORM_SHORT_555},
	/*	// CL_INTENSITY
	{CL_INTENSITY,	CL_UNORM_INT8},
	{CL_INTENSITY,	CL_UNORM_INT16},
	{CL_INTENSITY,	CL_SNORM_INT8},
	{CL_INTENSITY,	CL_SNORM_INT16},
	{CL_INTENSITY,	CL_HALF_FLOAT},
	{CL_INTENSITY,	CL_FLOAT},
	*/
#endif
};

const unsigned int Intel::OpenCL::CPUDevice::NUM_OF_SUPPORTED_IMAGE_FORMATS =
sizeof(Intel::OpenCL::CPUDevice::suportedImageFormats)/sizeof(cl_image_format);

using namespace Intel::OpenCL::CPUDevice;

const char* Intel::OpenCL::CPUDevice::CPU_STRING = "GenuineIntel";
const char* Intel::OpenCL::CPUDevice::VENDOR_STRING = "Intel Corporation";

// Update also in clang_driver.cpp (Guy)
#ifdef __DOUBLE_ENABLED__
static const char OCL_SUPPORTED_EXTENSIONS[] = "cl_khr_fp64 cl_khr_global_int32_base_atomics "\
												"cl_khr_global_int32_extended_atomics cl_khr_local_int32_base_atomics "\
												"cl_khr_local_int32_extended_atomics cl_khr_gl_sharing cl_khr_byte_addressable_store "\
												"cl_intel_printf cl_intel_overloading";

#else
static const char OCL_SUPPORTED_EXTENSIONS[] = "cl_khr_global_int32_base_atomics "\
											   "cl_khr_global_int32_extended_atomics cl_khr_local_int32_base_atomics "\
												"cl_khr_local_int32_extended_atomics cl_khr_gl_sharing cl_khr_byte_addressable_store "\
												"cl_intel_printf cl_intel_overloading";
#endif

static const size_t CPU_MAX_WORK_ITEM_SIZES[CPU_MAX_WORK_ITEM_DIMENSIONS] =
	{
	CPU_MAX_WORK_GROUP_SIZE,
	CPU_MAX_WORK_GROUP_SIZE,
	CPU_MAX_WORK_GROUP_SIZE
	};

const char* clDevErr2Txt(cl_dev_err_code errorCode)
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

CPUDevice::CPUDevice(cl_uint uiDevId, IOCLFrameworkCallbacks *devCallbacks, IOCLDevLogDescriptor *logDesc)
	: m_pCPUDeviceConfig(NULL), m_pFrameworkCallBacks(devCallbacks), m_uiCpuId(uiDevId),
	m_pLogDescriptor(logDesc), m_iLogHandle (0)
{
}

cl_int CPUDevice::Init()
{
	if ( NULL != m_pLogDescriptor )
	{
		cl_int ret = m_pLogDescriptor->clLogCreateClient(m_uiCpuId, L"CPU Device", &m_iLogHandle);
		if(CL_DEV_SUCCESS != ret)
		{
			return CL_DEV_ERROR_FAIL;
		}
	}

	m_pCPUDeviceConfig = new CPUDeviceConfig();
	m_pCPUDeviceConfig->Initialize(clCPUDEVICE_CFG_PATH);

	CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("CreateDevice function enter"));

	m_pProgramService = new ProgramService(m_uiCpuId, m_pFrameworkCallBacks, m_pLogDescriptor, m_pCPUDeviceConfig);
	m_pMemoryAllocator = new MemoryAllocator(m_uiCpuId, m_pLogDescriptor);
	m_pTaskDispatcher = new TaskDispatcher(m_uiCpuId, m_pFrameworkCallBacks, m_pProgramService,
		m_pMemoryAllocator, m_pLogDescriptor, m_pCPUDeviceConfig);


	if ( (NULL == m_pProgramService) ||	(NULL == m_pMemoryAllocator) ||	(NULL == m_pTaskDispatcher) )
	{
		return CL_DEV_OUT_OF_MEMORY;
	}
	cl_int ret = clDevCreateCommandList(CL_DEV_LIST_ENABLE_OOO, &m_defaultCommandList);
	if (CL_DEV_SUCCESS != ret)
	{
		return CL_DEV_ERROR_FAIL;
	}

	return CL_DEV_SUCCESS;
}

CPUDevice::~CPUDevice()
{
}

// ---------------------------------------
// Public functions / Device entry points
cl_int clDevCreateDeviceInstance(  cl_uint		dev_id,
								   IOCLFrameworkCallbacks	*pDevCallBacks,
								   IOCLDevLogDescriptor		*pLogDesc,
								   IOCLDevice*				*pDevice
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

	cl_int rc = pNewDevice->Init();
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
		param					An enumeration that identifies the device information being queried. It can be one of
								the following values as specified in OCL spec. table 4.3
		valSize				Specifies the size in bytes of memory pointed to by paramValue. This size in
								bytes must be >= size of return type
	Output
		paramVal				A pointer to memory location where appropriate values for a given param as specified in OCL spec. table 4.3 will be returned. If paramVal is NULL, it is ignored
		paramValSize_ret		Returns the actual size in bytes of data being queried by paramVal. If paramValSize_ret is NULL, it is ignored
	Returns
		CL_DEV_SUCCESS			If functions is executed successfully.
		CL_DEV_INVALID_VALUE	If param_name is not one of the supported values or if size in bytes specified by paramValSize is < size of return type as specified in OCL spec. table 4.3 and paramVal is not a NULL value
**************************************************************************************************************************/
cl_int CPUDevice::clDevGetDeviceInfo(cl_device_info IN param, size_t IN valSize, void* OUT paramVal,
				size_t* OUT paramValSizeRet)
{
	size_t  internalRetunedValueSize = valSize;
	size_t  *pinternalRetunedValueSize;
	int		viCPUInfo[4] = {-1};

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
				*(cl_uint*)paramVal = 0;	// TODO: change when halfs are supported
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
		case( CL_DEVICE_LOCAL_MEM_SIZE):				// Consider local memory size is 24Kbyte LCL_MEM_SIZE constant
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
				*(size_t*)paramVal = (size_t)(1e9/ProfilingTimerFrequency());
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
				*(cl_ulong*)paramVal = MAX(128*1024*1024, TotalVirtualSize()/4);
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
				*(cl_ulong*)paramVal = TotalVirtualSize();
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
			*pinternalRetunedValueSize = strlen(CPU_STRING) + 1;
			if(NULL != paramVal && valSize < *pinternalRetunedValueSize)
			{
				return CL_DEV_INVALID_VALUE;
			}
			//if OUT paramVal is NULL it should be ignored
			if(NULL != paramVal)
			{
		  		STRCPY_S((char*)paramVal, valSize, CPU_STRING);
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
			*pinternalRetunedValueSize = strlen("OpenCL 1.1 ") + 1;
			if(NULL != paramVal && valSize < *pinternalRetunedValueSize)
			{
				return CL_DEV_INVALID_VALUE;
			}
			//if OUT paramVal is NULL it should be ignored
			if(NULL != paramVal)
			{
				STRCPY_S((char*)paramVal, valSize, "OpenCL 1.1 ");
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

		default:
			return CL_DEV_INVALID_VALUE;
	};
	return CL_DEV_SUCCESS;

}
// Execution commands
/****************************************************************************************************************
 clDevCreateCommandList
	Call TaskDispatcher to create command list
********************************************************************************************************************/
cl_int CPUDevice::clDevCreateCommandList( cl_dev_cmd_list_props IN props, cl_dev_cmd_list* OUT list)
{
	CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("clDevCreateCommandList Function enter"));
	return m_pTaskDispatcher->createCommandList(props,list);
}
/****************************************************************************************************************
 clDevFlushCommandList
	Call TaskDispatcher to flush command list
********************************************************************************************************************/
cl_int CPUDevice::clDevFlushCommandList( cl_dev_cmd_list IN list)
{
	CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("clDevFlushCommandList Function enter"));
	return m_pTaskDispatcher->flushCommandList(list);
}
/****************************************************************************************************************
 clDevRetainCommandList
	Call TaskDispatcher to retain command list
********************************************************************************************************************/
cl_int CPUDevice::clDevRetainCommandList( cl_dev_cmd_list IN list)
{
	CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("clDevRetainCommandList Function enter"));
	return m_pTaskDispatcher->retainCommandList(list);
}
/****************************************************************************************************************
 clDevReleaseCommandList
	Call TaskDispatcher to release command list
********************************************************************************************************************/
cl_int CPUDevice::clDevReleaseCommandList( cl_dev_cmd_list IN list )
{
	CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("clDevReleaseCommandList Function enter"));
	return m_pTaskDispatcher->releaseCommandList(list);
}
/****************************************************************************************************************
 clDevCommandListExecute
	Call TaskDispatcher to execute command list
********************************************************************************************************************/
cl_int CPUDevice::clDevCommandListExecute( cl_dev_cmd_list IN list, cl_dev_cmd_desc* IN *cmds, cl_uint IN count)
{
	CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("clDevCommandListExecute Function enter"));
	if (list)
		return m_pTaskDispatcher->commandListExecute(list,cmds,count);
	else
	{
		cl_int ret = m_pTaskDispatcher->commandListExecute(m_defaultCommandList,cmds,count);
		if (CL_DEV_FAILED(ret))
		{
			return ret;
		}
		return m_pTaskDispatcher->flushCommandList(m_defaultCommandList);
	}
}

/****************************************************************************************************************
 clDevCommandListExecute
	Call clDevCommandListWaitCompletion to add calling thread to execution pool
********************************************************************************************************************/
cl_int CPUDevice::clDevCommandListWaitCompletion(cl_dev_cmd_list IN list)
{
	CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("clDevCommandListWaitCompletion Function enter"));
	return m_pTaskDispatcher->commandListWaitCompletion(list);
}

//Memory API's
/****************************************************************************************************************
 clDevGetSupportedImageFormats
	Call Memory Allocator to get supported image formats
********************************************************************************************************************/
cl_int CPUDevice::clDevGetSupportedImageFormats( cl_dev_mem_flags IN flags, cl_dev_mem_object_type IN imageType,
				cl_uint IN numEntries, cl_image_format* OUT formats, cl_uint* OUT numEntriesRet)
{
	CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("clDevGetSupportedImageFormats Function enter"));
	return m_pMemoryAllocator->GetSupportedImageFormats(flags, imageType,numEntries, formats, numEntriesRet);

}
/****************************************************************************************************************
 clDevCreateMemoryObject
	Call Memory Allocator to create memory object
********************************************************************************************************************/
cl_int CPUDevice::clDevCreateMemoryObject( cl_dev_mem_flags IN flags, const cl_image_format* IN format,
									cl_uint	IN dim_count, const size_t* IN dim_size, void*	IN buffer_ptr, const size_t* IN pitch,
									cl_dev_host_ptr_flags IN ptr_flags, cl_dev_mem* OUT memObj)
{
	CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("clDevCreateMemoryObject Function enter"));
	return m_pMemoryAllocator->CreateObject(flags, format, dim_count, dim_size, buffer_ptr, pitch, ptr_flags, memObj);
}
/****************************************************************************************************************
 clDevDeleteMemoryObject
	Call Memory Allocator to delete memory object
********************************************************************************************************************/
cl_int CPUDevice::clDevDeleteMemoryObject( cl_dev_mem IN memObj )
{
	CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("clDevDeleteMemoryObject Function enter"));
	return m_pMemoryAllocator->ReleaseObject(memObj);
}
/****************************************************************************************************************
 clDevCreateMappedRegion
	Call Memory Allocator to craete mapped region
********************************************************************************************************************/
cl_int CPUDevice::clDevCreateMappedRegion( cl_dev_cmd_param_map* INOUT pMapParams)
{
	CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("clDevCreateMappedRegion Function enter"));
	return m_pMemoryAllocator->CreateMappedRegion(pMapParams);

}
/****************************************************************************************************************
 clDevReleaseMappedRegion
	Call Memory Allocator to release mapped region
********************************************************************************************************************/
cl_int CPUDevice::clDevReleaseMappedRegion( cl_dev_cmd_param_map* IN pMapParams )
{
	CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("clDevReleaseMappedRegion Function enter"));
	return m_pMemoryAllocator->ReleaseMappedRegion( pMapParams );
}

/****************************************************************************************************************
 clDevCheckProgramBinary
	Call Program Serice to check binaries
********************************************************************************************************************/
cl_int CPUDevice::clDevCheckProgramBinary( size_t IN binSize, const void* IN bin )
{
	CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("clDevCheckProgramBinary Function enter"));
	return m_pProgramService->CheckProgramBinary(binSize, bin );
}

/*******************************************************************************************************************
clDevCreateProgram
	Call programService to create program
**********************************************************************************************************************/

cl_int CPUDevice::clDevCreateProgram( size_t IN binSize, const void* IN bin, cl_dev_binary_prop IN prop, cl_dev_program* OUT prog )
{
	CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("clDevCreateProgram Function enter"));
	return m_pProgramService->CreateProgram(binSize, bin, prop, prog );
}

/*******************************************************************************************************************
clDevBuildProgram
	Call programService to build program
**********************************************************************************************************************/

cl_int CPUDevice::clDevBuildProgram( cl_dev_program IN prog, const char* IN options, void* IN userData )
{
	CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("clDevBuildProgram Function enter"));
	return m_pProgramService->BuildProgram(prog, options, userData);
}

/*******************************************************************************************************************
clDevReleaseProgram
	Call programService to release program
**********************************************************************************************************************/

cl_int CPUDevice::clDevReleaseProgram( cl_dev_program IN prog )
{
	CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("clDevReleaseProgram Function enter"));
	return m_pProgramService->ReleaseProgram( prog );
}

/*******************************************************************************************************************
clDevUnloadCompiler
	Call programService to unload the backend compiler
**********************************************************************************************************************/
cl_int CPUDevice::clDevUnloadCompiler()
{
	CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("clDevUnloadCompiler Function enter"));
	return m_pProgramService->UnloadCompiler();
}
/*******************************************************************************************************************
clDevGetProgramBinary
	Call programService to get the program binary
**********************************************************************************************************************/
cl_int CPUDevice::clDevGetProgramBinary( cl_dev_program IN prog, size_t IN size, void* OUT binary, size_t* OUT sizeRet )
{
	CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("clDevGetProgramBinary Function enter"));
	return m_pProgramService->GetProgramBinary(prog, size, binary, sizeRet );
}
/*******************************************************************************************************************
clDevGetBuildLog
	Call programService to get the build log
**********************************************************************************************************************/
cl_int CPUDevice::clDevGetBuildLog( cl_dev_program IN prog, size_t IN size, char* OUT log, size_t* OUT sizeRet)
{
	CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("clDevGetBuildLog Function enter"));
	return m_pProgramService->GetBuildLog(prog, size, log, sizeRet);
}
/*******************************************************************************************************************
clDevUnloadCompiler
	Call programService to get supported binary description
**********************************************************************************************************************/
cl_int CPUDevice::clDevGetSupportedBinaries( size_t IN count, cl_prog_binary_desc* OUT types, size_t* OUT sizeRet )
{
	CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("clDevGetSupportedBinaries Function enter"));
	return m_pProgramService->GetSupportedBinaries(count,types,sizeRet );
}
/*******************************************************************************************************************
clDevUnloadCompiler
	Call programService to get kernel id from its name
**********************************************************************************************************************/
cl_dev_err_code CPUDevice::clDevGetKernelId( cl_dev_program IN prog, const char* IN name, cl_dev_kernel* OUT kernelId )
{
	CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("clDevGetKernelId Function enter"));
	return m_pProgramService->GetKernelId(prog, name, kernelId );
}
/*******************************************************************************************************************
clDevUnloadCompiler
	Call programService to get kernels from the program
**********************************************************************************************************************/
cl_int CPUDevice::clDevGetProgramKernels( cl_dev_program IN prog, cl_uint IN numKernels, cl_dev_kernel* OUT kernels,
						 size_t* OUT numKernelsRet )
{
	CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("clDevGetProgramKernels Function enter"));
	return m_pProgramService->GetProgramKernels(prog, numKernels, kernels,numKernelsRet );
}
/*******************************************************************************************************************
clDevGetKernelInfo
	Call programService to get kernel info
**********************************************************************************************************************/
cl_dev_err_code CPUDevice::clDevGetKernelInfo( cl_dev_kernel IN kernel, cl_dev_kernel_info IN param, size_t IN valueSize,
					void* OUT value, size_t* OUT valueSizeRet )
{
	CpuInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("clDevGetKernelInfo Function enter"));
	return m_pProgramService->GetKernelInfo(kernel, param, valueSize,value,valueSizeRet );
}

/*******************************************************************************************************************
clDevGetPerofrmanceCounter
	Get performance counter value
**********************************************************************************************************************/
cl_ulong CPUDevice::clDevGetPerformanceCounter()
{
	return Intel::OpenCL::Utils::HostTime();
}

cl_int CPUDevice::clDevSetLogger(IOCLDevLogDescriptor *pLogDescriptor)
{

	if ( NULL != m_pLogDescriptor )
	{
		m_pLogDescriptor->clLogReleaseClient(m_iLogHandle);
	}
	m_pLogDescriptor = pLogDescriptor;
	if ( NULL != m_pLogDescriptor )
	{
		cl_int ret = m_pLogDescriptor->clLogCreateClient(m_uiCpuId, L"CPU Device", &m_iLogHandle);
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

	clDevReleaseCommandList(m_defaultCommandList);
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

	delete this;
}
