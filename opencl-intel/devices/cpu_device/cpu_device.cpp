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

#include <intrin.h>

#pragma comment (lib, "cl_builtin_functions.lib")

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

#define OCL_SUPPORTED_EXTENSIONS "cl_khr_global_int32_extended_atomics cl_khr_global_int32_base_atomics cl_khr_byte_addressable_store"

const unsigned int Intel::OpenCL::CPUDevice::NUM_OF_SUPPORTED_IMAGE_FORMATS =
sizeof(Intel::OpenCL::CPUDevice::suportedImageFormats)/sizeof(cl_image_format);

using namespace Intel::OpenCL::CPUDevice;

const char* Intel::OpenCL::CPUDevice::CPU_STRING = "GenuineIntel";
const size_t CPU_MAX_WORK_ITEM_SIZES[CPU_MAX_WORK_ITEM_DIMENSIONS] =
	{
	CPU_MAX_WORK_GROUP_SIZE,
	CPU_MAX_WORK_GROUP_SIZE,
	CPU_MAX_WORK_GROUP_SIZE
	};

char* clDevErr2Txt(cl_dev_err_code errorCode)
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

// Static members initialization
CPUDevice* CPUDevice::m_pDevInstance = NULL;

CPUDevice::CPUDevice(cl_uint uiDevId, cl_dev_call_backs *devCallbacks, cl_dev_log_descriptor *logDesc)
	: m_iLogHandle (0), m_uiCpuId(uiDevId), m_pCPUDeviceConfig(NULL)
{	
	memcpy(&m_frameWorkCallBacks, devCallbacks, sizeof(m_frameWorkCallBacks));

	if ( NULL == logDesc )
	{
		memset(&m_logDescriptor, 0, sizeof(cl_dev_log_descriptor));
	}
	else
	{
		memcpy_s(&m_logDescriptor, sizeof(cl_dev_log_descriptor), logDesc, sizeof(cl_dev_log_descriptor));
	}
	
	if ( NULL != m_logDescriptor.pfnclLogCreateClient )
	{
		cl_int ret = m_logDescriptor.pfnclLogCreateClient(uiDevId, L"CPU Device", &m_iLogHandle);
		if(CL_DEV_SUCCESS != ret)
		{
			return;
		}
	}

	m_pCPUDeviceConfig = new CPUDeviceConfig();
	m_pCPUDeviceConfig->Initialize(clCPUDEVICE_CFG_PATH);

	clRTLibInitLibrary();

	m_pProgramService = new ProgramService(uiDevId, devCallbacks, logDesc);
	m_pMemoryAllocator = new MemoryAllocator(uiDevId, logDesc);
	m_pTaskDispatcher = new TaskDispatcher(uiDevId, devCallbacks, m_pProgramService, 
		m_pMemoryAllocator, logDesc, m_pCPUDeviceConfig);
}

CPUDevice::~CPUDevice()
{
	if( NULL != m_pCPUDeviceConfig)
	{
		delete m_pCPUDeviceConfig;
	}
	if (0 != m_iLogHandle)
	{
		m_logDescriptor.pfnclLogReleaseClient(m_iLogHandle);
	}
	if(NULL != m_pProgramService)
	{
		delete m_pProgramService;
		m_pProgramService = NULL;
	}
	if(NULL != m_pMemoryAllocator)
	{
		delete m_pMemoryAllocator;
		m_pMemoryAllocator = NULL;
	}
	if(NULL != m_pTaskDispatcher)
	{
		delete m_pTaskDispatcher;
		m_pTaskDispatcher = NULL;
	}	
}

void CPUDevice::Destroy()
{
    if( NULL != m_pDevInstance )
    {
        delete m_pDevInstance;
        m_pDevInstance = NULL;
    }
}

// ---------------------------------------
// Public functions / Device entry points

CPUDevice* CPUDevice::CreateDevice(cl_uint uiDevId, cl_dev_call_backs *devCallbacks, cl_dev_log_descriptor *logDesc)
{

	if ( NULL == m_pDevInstance )
	{
		m_pDevInstance = new CPUDevice(uiDevId, devCallbacks, logDesc);
	}

	InfoLog(m_pDevInstance->m_logDescriptor, m_pDevInstance->m_iLogHandle, L"CreateDevice function enter");

	if ( (NULL == m_pDevInstance->m_pProgramService) ||
			(NULL == m_pDevInstance->m_pMemoryAllocator) ||
			(NULL == m_pDevInstance->m_pTaskDispatcher)
		)
	{
		delete m_pDevInstance;
		m_pDevInstance = NULL;
	}

	return m_pDevInstance;
}

CPUDevice* CPUDevice::GetInstance()
{
	assert(m_pDevInstance);
	return m_pDevInstance;
}

// Device entry points
//Device Inforamtion function prototypes
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
				if(NULL != paramVal && valSize != *pinternalRetunedValueSize)
				{
					return CL_INVALID_VALUE;
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
				if(NULL != paramVal && valSize != *pinternalRetunedValueSize)
				{
					return CL_DEV_INVALID_VALUE;
				}
				//if OUT paramVal is NULL it should be ignored
				if(NULL != paramVal)
				{
					
					//Get the CPU info
					__cpuid(viCPUInfo, 0);
					
					*(cl_uint*)paramVal = (cl_uint)viCPUInfo[0];
				}
				return CL_DEV_SUCCESS;
			}

		case( CL_DEVICE_MAX_COMPUTE_UNITS):
			{
				*pinternalRetunedValueSize = sizeof(cl_uint);
				if(NULL != paramVal && valSize != *pinternalRetunedValueSize)
				{
					return CL_DEV_INVALID_VALUE;
				}
				//if OUT paramVal is NULL it should be ignored
				if(NULL != paramVal)
				{
					// get the CPU info
					__cpuid(viCPUInfo, 1);
					cl_uint uiCoreCount = (viCPUInfo[1] >> 16) & 0xff;

					*(cl_uint*)paramVal = uiCoreCount;
				}
				return CL_DEV_SUCCESS;
			}
		case( CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR):
			{
				*pinternalRetunedValueSize = sizeof(cl_uint);
				if(NULL != paramVal && valSize != *pinternalRetunedValueSize)
				{
					return CL_DEV_INVALID_VALUE;
				}
				//if OUT paramVal is NULL it should be ignored
				if(NULL != paramVal)
				{
					// get the CPU info
					__cpuid(viCPUInfo, 0);
					//in word 2 bit 28 is the 256-bit Intel advanced vector extensions (Intel) 
					cl_uint value = sizeof(__m128)/ sizeof(char); 
					if(viCPUInfo[2] & 0x8000000)
					{
	 					*(cl_uint*)paramVal = value; //8;
					}
					else
					{
	 					*(cl_uint*)paramVal = 2 * value; //16;
					}
				}
				return CL_DEV_SUCCESS;
			}
		case( CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT):
			{
				*pinternalRetunedValueSize = sizeof(cl_uint);
				if(NULL != paramVal && valSize != *pinternalRetunedValueSize)
				{
					return CL_DEV_INVALID_VALUE;
				}
				//if OUT paramVal is NULL it should be ignored
				cl_uint value = sizeof(__m128)/ sizeof(short);

				if(NULL != paramVal)
				{
					// get the CPU info
					__cpuid(viCPUInfo, 0);

					if(viCPUInfo[2] & 0x8000000)
					{
	 					*(cl_uint*)paramVal = value; 
					}
					else
					{
	 					*(cl_uint*)paramVal = value *2;
					}

				}
				return CL_DEV_SUCCESS;
			}

		case( CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT):// FALL THROUGH
		case( CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT):
			{
				*pinternalRetunedValueSize = sizeof(cl_uint);
				if(NULL != paramVal && valSize != *pinternalRetunedValueSize)
				{
					return CL_DEV_INVALID_VALUE;
				}
				//if OUT paramVal is NULL it should be ignored
				if(NULL != paramVal)
				{
					// get the CPU info
					__cpuid(viCPUInfo, 0);
					cl_uint value = sizeof(__m128)/ sizeof(int);
					if(viCPUInfo[2] & 0x8000000)
					{
	 					*(cl_uint*)paramVal = value; 
					}
					else
					{
	 					*(cl_uint*)paramVal = value *2;
					}				}
				return CL_DEV_SUCCESS;
			}

		case( CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG): 
		{
				*pinternalRetunedValueSize = sizeof(cl_uint);
				if(NULL != paramVal && valSize != *pinternalRetunedValueSize)
				{
					return CL_DEV_INVALID_VALUE;
				}
				//if OUT paramVal is NULL it should be ignored
				if(NULL != paramVal)
				{
					// get the CPU info
					__cpuid(viCPUInfo, 0);
					cl_uint value = sizeof(__m128)/ sizeof(long);
					if(viCPUInfo[2] & 0x8000000)
					{
	 					*(cl_uint*)paramVal = value; 
					}
					else
					{
	 					*(cl_uint*)paramVal = value * 2; 
					}

				}
				return CL_DEV_SUCCESS;
			}
		case( CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE):
	{
				*pinternalRetunedValueSize = sizeof(cl_uint);
				if(NULL != paramVal && valSize != *pinternalRetunedValueSize)
				{
					return CL_DEV_INVALID_VALUE;
				}
				//if OUT paramVal is NULL it should be ignored
				if(NULL != paramVal)
				{
					cl_uint value = 0; //If the cl_khr_fp64 extension is not supported, CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE must return 0.
 					*(cl_uint*)paramVal = value; 
				}
				return CL_DEV_SUCCESS;
			}
	
		case( CL_DEVICE_IMAGE_SUPPORT):
			{
				*pinternalRetunedValueSize = sizeof(cl_bool);
				if(NULL != paramVal && valSize != *pinternalRetunedValueSize)
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
				if(NULL != paramVal && valSize != *pinternalRetunedValueSize)
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
		case( CL_DEVICE_SINGLE_FP_CONFIG):	
		{
				cl_device_fp_config fpConfig = CL_FP_ROUND_TO_NEAREST | CL_FP_INF_NAN | CL_FP_DENORM;
				*pinternalRetunedValueSize = sizeof(cl_device_fp_config);
				if(NULL != paramVal && valSize != *pinternalRetunedValueSize)
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
				if(NULL != paramVal && valSize != *pinternalRetunedValueSize)
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
				if(NULL != paramVal && valSize != *pinternalRetunedValueSize)
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
				if(NULL != paramVal && valSize != *pinternalRetunedValueSize)
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
				if(NULL != paramVal && valSize != *pinternalRetunedValueSize)
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
				if(NULL != paramVal && valSize != *pinternalRetunedValueSize)
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
				if(NULL != paramVal && valSize != *pinternalRetunedValueSize)
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
				if(NULL != paramVal && valSize != *pinternalRetunedValueSize)
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
				if(NULL != paramVal && valSize != *pinternalRetunedValueSize)
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
				if(NULL != paramVal && valSize != *pinternalRetunedValueSize)
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
				if(NULL != paramVal && valSize != *pinternalRetunedValueSize)
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
				if(NULL != paramVal && valSize != *pinternalRetunedValueSize)
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
				if(NULL != paramVal && valSize != *pinternalRetunedValueSize)
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
				if(NULL != paramVal && valSize != *pinternalRetunedValueSize)
				{
					return CL_DEV_INVALID_VALUE;
				}
				//if OUT paramVal is NULL it should be ignored
				if(NULL != paramVal)
				{
					
					//Get the Cache info
					__cpuid(viCPUInfo, 0x80000006);
					*(cl_uint*)paramVal = (cl_uint)viCPUInfo[2] & 0xff;
				}
				return CL_DEV_SUCCESS;
		}
		case( CL_DEVICE_GLOBAL_MEM_CACHE_SIZE):
			{
				*pinternalRetunedValueSize = sizeof(cl_ulong);
				if(NULL != paramVal && valSize != *pinternalRetunedValueSize)
				{
					return CL_DEV_INVALID_VALUE;
				}
				//if OUT paramVal is NULL it should be ignored
				if(NULL != paramVal)
				{
					
					//Get the Cache info
					__cpuid(viCPUInfo, 0x80000006);
					*(cl_ulong*)paramVal = (cl_ulong)(viCPUInfo[2] >> 16) & 0xffff;
				}
				return CL_DEV_SUCCESS;
		}
		case( CL_DEVICE_LOCAL_MEM_SIZE):				// Consider local memory size is 24Kbyte LCL_MEM_SIZE constant
		{
				*pinternalRetunedValueSize = sizeof(cl_ulong);
				if(NULL != paramVal && valSize != *pinternalRetunedValueSize)
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
				if(NULL != paramVal && valSize != *pinternalRetunedValueSize)
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
				if(NULL != paramVal && valSize != *pinternalRetunedValueSize)
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
				if(NULL != paramVal && valSize != *pinternalRetunedValueSize)
				{
					return CL_DEV_INVALID_VALUE;
				}
				//if OUT paramVal is NULL it should be ignored
				if(NULL != paramVal)
				{
					*(size_t*)paramVal = CPU_PROFILING_TIMER_RESOLUTION;
				}
				return CL_DEV_SUCCESS;
		}		
		case( CL_DEVICE_GLOBAL_MEM_CACHE_TYPE):
			{
				*pinternalRetunedValueSize = sizeof(cl_device_mem_cache_type);
				if(NULL != paramVal && valSize != *pinternalRetunedValueSize)
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
				if(NULL != paramVal && valSize != *pinternalRetunedValueSize)
				{
					return CL_DEV_INVALID_VALUE;
				}
				//if OUT paramVal is NULL it should be ignored
				if(NULL != paramVal)
				{
					*(cl_ulong*)paramVal = max(128*1024*1024, TotalVirtualSize()/4);
				}
				return CL_DEV_SUCCESS;
			}
		case( CL_DEVICE_GLOBAL_MEM_SIZE):
			{
				*pinternalRetunedValueSize = sizeof(cl_ulong);
				if(NULL != paramVal && valSize != *pinternalRetunedValueSize)
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
				if(NULL != paramVal && valSize != *pinternalRetunedValueSize)
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
			if(NULL != paramVal && valSize != *pinternalRetunedValueSize)
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
			if(NULL != paramVal && valSize != *pinternalRetunedValueSize)
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
				if(NULL != paramVal && valSize != *pinternalRetunedValueSize)
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
				if(NULL != paramVal && valSize != *pinternalRetunedValueSize)
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
				if(NULL != paramVal && valSize != *pinternalRetunedValueSize)
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
				if(NULL != paramVal && valSize != *pinternalRetunedValueSize)
				{
					return CL_DEV_INVALID_VALUE;
				}
					//if OUT paramVal is NULL it should be ignored
				if(NULL != paramVal)
				{
					*(cl_bool*)paramVal = CL_FALSE;//Currently we dont have compiler yet
				}
				return CL_DEV_SUCCESS; 
			}
		case( CL_DEVICE_NAME):
			{
				*pinternalRetunedValueSize = strlen("CPU");
				if(NULL != paramVal && valSize < *pinternalRetunedValueSize)
				{
					return CL_DEV_INVALID_VALUE;
				}
				//if OUT paramVal is NULL it should be ignored
				if(NULL != paramVal)
				{
			  		strcpy_s((char*)paramVal, valSize, "CPU");
				}

				return CL_DEV_SUCCESS; 
			}
		case( CL_DEVICE_VENDOR):
			{
				*pinternalRetunedValueSize = strlen(CPU_STRING);
				if(NULL != paramVal && valSize < *pinternalRetunedValueSize)
				{
					return CL_DEV_INVALID_VALUE;
				}
				//if OUT paramVal is NULL it should be ignored
				if(NULL != paramVal)
				{
					strcpy_s((char*)paramVal, valSize, CPU_STRING);
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
					strcpy_s((char*)paramVal, valSize, "FULL_PROFILE");
				}
				return CL_DEV_SUCCESS; 
			}
		case( CL_DEVICE_VERSION):
			{
				*pinternalRetunedValueSize = strlen("OpenCL 1.0 ") + 1;
				if(NULL != paramVal && valSize < *pinternalRetunedValueSize)
				{
					return CL_DEV_INVALID_VALUE;
				}
				//if OUT paramVal is NULL it should be ignored
				if(NULL != paramVal)
				{
					strcpy_s((char*)paramVal, valSize, "OpenCL 1.0 ");
				}
				return CL_DEV_SUCCESS; 
			}
        
		case( CL_DRIVER_VERSION ):
			{
				*pinternalRetunedValueSize = strlen("1.0") + 1;
				if(NULL != paramVal && valSize < *pinternalRetunedValueSize)
				{
					return CL_DEV_INVALID_VALUE;
				}
				//if OUT paramVal is NULL it should be ignored
				if(NULL != paramVal)
				{
					strcpy_s((char*)paramVal, valSize, "1.0");
				}
				return CL_DEV_SUCCESS; 
			}
        
		case( CL_DEVICE_EXTENSIONS): 

			*pinternalRetunedValueSize = strlen(OCL_SUPPORTED_EXTENSIONS) + 1;
			if(NULL != paramVal && valSize < *pinternalRetunedValueSize)
			{
				return CL_DEV_INVALID_VALUE;
			}
			//if OUT paramVal is NULL it should be ignored
			if(NULL != paramVal)
			{
				strcpy_s((char*)paramVal, valSize, OCL_SUPPORTED_EXTENSIONS);
			}
			return CL_DEV_SUCCESS; 

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
	InfoLog(m_pDevInstance->m_logDescriptor, m_pDevInstance->m_iLogHandle, L"clDevCreateCommandList Function enter");
	return m_pDevInstance->m_pTaskDispatcher->createCommandList(props,list);
}
/****************************************************************************************************************
 clDevFlushCommandList
	Call TaskDispatcher to flush command list
********************************************************************************************************************/
cl_int CPUDevice::clDevFlushCommandList( cl_dev_cmd_list IN list)
{
	InfoLog(m_pDevInstance->m_logDescriptor, m_pDevInstance->m_iLogHandle, L"clDevFlushCommandList Function enter");
	return m_pDevInstance->m_pTaskDispatcher->flushCommandList(list);
}
/****************************************************************************************************************
 clDevRetainCommandList
	Call TaskDispatcher to retain command list
********************************************************************************************************************/
cl_int CPUDevice::clDevRetainCommandList( cl_dev_cmd_list IN list)
{
	InfoLog(m_pDevInstance->m_logDescriptor, m_pDevInstance->m_iLogHandle, L"clDevRetainCommandList Function enter");
	return m_pDevInstance->m_pTaskDispatcher->retainCommandList(list);
}
/****************************************************************************************************************
 clDevReleaseCommandList
	Call TaskDispatcher to release command list
********************************************************************************************************************/
cl_int CPUDevice::clDevReleaseCommandList( cl_dev_cmd_list IN list )
{
	InfoLog(m_pDevInstance->m_logDescriptor, m_pDevInstance->m_iLogHandle, L"clDevReleaseCommandList Function enter");
	return m_pDevInstance->m_pTaskDispatcher->releaseCommandList(list);
}
/****************************************************************************************************************
 clDevCommandListExecute
	Call TaskDispatcher to execute command list
********************************************************************************************************************/
cl_int CPUDevice::clDevCommandListExecute( cl_dev_cmd_list IN list, cl_dev_cmd_desc* IN *cmds, cl_uint IN count)
{
	InfoLog(m_pDevInstance->m_logDescriptor, m_pDevInstance->m_iLogHandle, L"clDevCommandListExecute Function enter");
	return m_pDevInstance->m_pTaskDispatcher->commandListExecute(list,cmds,count);
}
//Memory API's
/****************************************************************************************************************
 clDevGetSupportedImageFormats
	Call Memory Allocator to get supported image formats
********************************************************************************************************************/
cl_int CPUDevice::clDevGetSupportedImageFormats( cl_dev_mem_flags IN flags, cl_dev_mem_object_type IN imageType,
				cl_uint IN numEntries, cl_image_format* OUT formats, cl_uint* OUT numEntriesRet)
{
	InfoLog(m_pDevInstance->m_logDescriptor, m_pDevInstance->m_iLogHandle, L"clDevGetSupportedImageFormats Function enter");
	return m_pDevInstance->m_pMemoryAllocator->GetSupportedImageFormats(flags, imageType,numEntries, formats, numEntriesRet);
	
}
/****************************************************************************************************************
 clDevCreateMemoryObject
	Call Memory Allocator to create memory object
********************************************************************************************************************/
cl_int CPUDevice::clDevCreateMemoryObject( cl_dev_mem_flags IN flags, const cl_image_format* IN format,
									cl_uint	IN dim_count, const size_t* IN dim_size, void*	IN buffer_ptr, const size_t* IN pitch,
									cl_dev_host_ptr_flags IN ptr_flags, cl_dev_mem* OUT memObj)
{
	InfoLog(m_pDevInstance->m_logDescriptor, m_pDevInstance->m_iLogHandle, L"clDevCreateMemoryObject Function enter");
	return m_pDevInstance->m_pMemoryAllocator->CreateObject(flags, format, dim_count, dim_size, buffer_ptr, pitch, ptr_flags, memObj);
}
/****************************************************************************************************************
 clDevDeleteMemoryObject
	Call Memory Allocator to delete memory object
********************************************************************************************************************/
cl_int CPUDevice::clDevDeleteMemoryObject( cl_dev_mem IN memObj )
{
	InfoLog(m_pDevInstance->m_logDescriptor, m_pDevInstance->m_iLogHandle, L"clDevDeleteMemoryObject Function enter");
	return m_pDevInstance->m_pMemoryAllocator->ReleaseObject(memObj);
}
/****************************************************************************************************************
 clDevCreateMappedRegion
	Call Memory Allocator to craete mapped region
********************************************************************************************************************/
cl_int CPUDevice::clDevCreateMappedRegion( cl_dev_cmd_param_map* INOUT pMapParams)
{
	InfoLog(m_pDevInstance->m_logDescriptor, m_pDevInstance->m_iLogHandle, L"clDevCreateMappedRegion Function enter");
	return m_pDevInstance->m_pMemoryAllocator->CreateMappedRegion(pMapParams);

}
/****************************************************************************************************************
 clDevReleaseMappedRegion
	Call Memory Allocator to release mapped region
********************************************************************************************************************/
cl_int CPUDevice::clDevReleaseMappedRegion( cl_dev_cmd_param_map* IN pMapParams )
{
	InfoLog(m_pDevInstance->m_logDescriptor, m_pDevInstance->m_iLogHandle, L"clDevReleaseMappedRegion Function enter");
	return m_pDevInstance->m_pMemoryAllocator->ReleaseMappedRegion( pMapParams );
}

/****************************************************************************************************************
 clDevCheckProgramBinary
	Call Program Serice to check binaries
********************************************************************************************************************/
cl_int CPUDevice::clDevCheckProgramBinary( size_t IN binSize, const void* IN bin )
{
	InfoLog(m_pDevInstance->m_logDescriptor, m_pDevInstance->m_iLogHandle, L"clDevCheckProgramBinary Function enter");
	return m_pDevInstance->m_pProgramService->CheckProgramBinary(binSize, bin );
}

/*******************************************************************************************************************
clDevCreateProgram
	Call programService to create program
**********************************************************************************************************************/

cl_int CPUDevice::clDevCreateProgram( size_t IN binSize, const void* IN bin, cl_dev_binary_prop IN prop, cl_dev_program* OUT prog )
{
	InfoLog(m_pDevInstance->m_logDescriptor, m_pDevInstance->m_iLogHandle, L"clDevCreateProgram Function enter");
	return m_pDevInstance->m_pProgramService->CreateProgram(binSize, bin, prop, prog );
}

/*******************************************************************************************************************
clDevBuildProgram
	Call programService to build program
**********************************************************************************************************************/

cl_int CPUDevice::clDevBuildProgram( cl_dev_program IN prog, const cl_char* IN options, void* IN userData )
{
	InfoLog(m_pDevInstance->m_logDescriptor, m_pDevInstance->m_iLogHandle, L"clDevBuildProgram Function enter");
	return m_pDevInstance->m_pProgramService->BuildProgram(prog, options, userData);
}

/*******************************************************************************************************************
clDevReleaseProgram
	Call programService to release program
**********************************************************************************************************************/

cl_int CPUDevice::clDevReleaseProgram( cl_dev_program IN prog )
{
	InfoLog(m_pDevInstance->m_logDescriptor, m_pDevInstance->m_iLogHandle, L"clDevReleaseProgram Function enter");
	return m_pDevInstance->m_pProgramService->ReleaseProgram( prog );
}

/*******************************************************************************************************************
clDevUnloadCompiler
	Call programService to unload the backend compiler
**********************************************************************************************************************/
cl_int CPUDevice::clDevUnloadCompiler()
{
	InfoLog(m_pDevInstance->m_logDescriptor, m_pDevInstance->m_iLogHandle, L"clDevUnloadCompiler Function enter");
	return m_pDevInstance->m_pProgramService->UnloadCompiler();
}
/*******************************************************************************************************************
clDevGetProgramBinary
	Call programService to get the program binary
**********************************************************************************************************************/
cl_int CPUDevice::clDevGetProgramBinary( cl_dev_program IN prog, size_t IN size, void* OUT binary, size_t* OUT sizeRet )
{
	InfoLog(m_pDevInstance->m_logDescriptor, m_pDevInstance->m_iLogHandle, L"clDevGetProgramBinary Function enter");
	return m_pDevInstance->m_pProgramService->GetProgramBinary(prog, size, binary, sizeRet );
}
/*******************************************************************************************************************
clDevGetBuildLog
	Call programService to get the build log
**********************************************************************************************************************/
cl_int CPUDevice::clDevGetBuildLog( cl_dev_program IN prog, size_t IN size, char* OUT log, size_t* OUT sizeRet)
{
	InfoLog(m_pDevInstance->m_logDescriptor, m_pDevInstance->m_iLogHandle, L"clDevGetBuildLog Function enter");
	return m_pDevInstance->m_pProgramService->GetBuildLog(prog, size, log, sizeRet);
}
/*******************************************************************************************************************
clDevUnloadCompiler
	Call programService to get supported binary description
**********************************************************************************************************************/
cl_int CPUDevice::clDevGetSupportedBinaries( cl_uint IN count, cl_prog_binary_desc* OUT types, size_t* OUT sizeRet )
{
	InfoLog(m_pDevInstance->m_logDescriptor, m_pDevInstance->m_iLogHandle, L"clDevGetSupportedBinaries Function enter");
	return m_pDevInstance->m_pProgramService->GetSupportedBinaries(count,types,sizeRet );
}
/*******************************************************************************************************************
clDevUnloadCompiler
	Call programService to get kernel id from its name
**********************************************************************************************************************/
cl_int CPUDevice::clDevGetKernelId( cl_dev_program IN prog, const char* IN name, cl_dev_kernel* OUT kernelId )
{
	InfoLog(m_pDevInstance->m_logDescriptor, m_pDevInstance->m_iLogHandle, L"clDevGetKernelId Function enter");
	return m_pDevInstance->m_pProgramService->GetKernelId(prog, name, kernelId );
}
/*******************************************************************************************************************
clDevUnloadCompiler
	Call programService to get kernels from the program
**********************************************************************************************************************/
cl_int CPUDevice::clDevGetProgramKernels( cl_dev_program IN prog, cl_uint IN numKernels, cl_dev_kernel* OUT kernels,
						 cl_uint* OUT numKernelsRet )
{
	InfoLog(m_pDevInstance->m_logDescriptor, m_pDevInstance->m_iLogHandle, L"clDevGetProgramKernels Function enter");
	return m_pDevInstance->m_pProgramService->GetProgramKernels(prog, numKernels, kernels,numKernelsRet );
}
/*******************************************************************************************************************
clDevGetKernelInfo
	Call programService to get kernel info
**********************************************************************************************************************/
cl_int CPUDevice::clDevGetKernelInfo( cl_dev_kernel IN kernel, cl_dev_kernel_info IN param, size_t IN valueSize,
					void* OUT value, size_t* OUT valueSizeRet )
{
	InfoLog(m_pDevInstance->m_logDescriptor, m_pDevInstance->m_iLogHandle, L"clDevGetKernelInfo Function enter");
	return m_pDevInstance->m_pProgramService->GetKernelInfo(kernel, param, valueSize,value,valueSizeRet );
}

/*******************************************************************************************************************
clDevCloseDevice
	Close device
**********************************************************************************************************************/
void CPUDevice::clDevCloseDevice(void)
{
	InfoLog(m_pDevInstance->m_logDescriptor, m_pDevInstance->m_iLogHandle, L"clCloseDevice Function enter");
	if ( NULL != m_pDevInstance->m_pProgramService )
	{
		delete m_pDevInstance->m_pProgramService;
		m_pDevInstance->m_pProgramService = NULL;
	}
	if ( NULL != m_pDevInstance->m_pMemoryAllocator )
	{
		delete m_pDevInstance->m_pMemoryAllocator;
		m_pDevInstance->m_pMemoryAllocator = NULL;
	}
	if ( NULL != m_pDevInstance->m_pTaskDispatcher )
	{
		delete m_pDevInstance->m_pTaskDispatcher;
		m_pDevInstance->m_pTaskDispatcher = NULL;
	}
   
    delete m_pDevInstance;
    m_pDevInstance = NULL;
}