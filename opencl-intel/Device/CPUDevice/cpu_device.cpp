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


#include <stdafx.h>
#pragma warning(push)
  #pragma warning(disable:4391)
  #include <intrin.h>
#pragma warning(pop)



#include "cpu_device.h"
#include "program_service.h"
#include "cl_logger.h"



using namespace Intel::OpenCL::CPUDevice;
class ProgramService * CPUDevice::m_pProgramService;
class MemoryAllocator * CPUDevice::m_pMemoryAllocator;
class Scheduler * CPUDevice::m_pScheduler;
cl_dev_log_descriptor CPUDevice::m_logDescriptor;
cl_int	CPUDevice::m_iLogHandle;

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
		case (CL_DEV_STILL_BUILDING): return "CL_DEV_STILL_BUILDING";
		case (CL_DEV_INVALID_KERNEL_NAME): return "CL_DEV_INVALID_KERNEL_NAME";
	
	default: return "Unknown Error Code";
	}
}

// Static members initialization
CPUDevice* CPUDevice::m_pDevInstance = NULL;

CPUDevice::CPUDevice(cl_uint devId, cl_dev_call_backs *devCallbacks, cl_dev_log_descriptor *logDesc)
{
	m_uiCpuId = devId;
	memcpy(&m_frameWorkCallBacks, devCallbacks, sizeof(m_frameWorkCallBacks));
}

CPUDevice::~CPUDevice()
{
	m_logDescriptor.pfnclLogReleaseClient(m_iLogHandle);
}

// ---------------------------------------
// Public functions / Device entry points

CPUDevice* CPUDevice::CreateDevice(cl_uint uiDevId, cl_dev_call_backs *devCallbacks, cl_dev_log_descriptor *logDesc)
{
	
	if ( NULL == logDesc )
	{
		memset(&m_logDescriptor, 0, sizeof(cl_dev_log_descriptor));
	}
	else
	{
		memcpy_s(&m_logDescriptor, sizeof(cl_dev_log_descriptor), logDesc, sizeof(cl_dev_log_descriptor));
	}
	

	cl_int ret = m_logDescriptor.pfnclLogCreateClient(uiDevId, L"CPU: CPU Device", &m_iLogHandle);
	if(CL_DEV_SUCCESS != ret)
	{
		return NULL;
	}

	InfoLog(m_logDescriptor, m_iLogHandle, L"CreateDevice function enter");
		
	
	if ( NULL == m_pDevInstance )
	{
		m_pDevInstance = new CPUDevice(uiDevId, devCallbacks, logDesc);
	}

	m_pProgramService = new ProgramService(uiDevId, devCallbacks, logDesc);
	if ( NULL == m_pProgramService )
	{
		return NULL;
	}

	m_pMemoryAllocator = new MemoryAllocator(uiDevId, logDesc);
	if ( NULL == m_pMemoryAllocator )
	{
		return NULL;
	}

	m_pScheduler = new Scheduler(uiDevId, devCallbacks, m_pProgramService, m_pMemoryAllocator, logDesc);
	if ( NULL == m_pScheduler )
	{
		return NULL;
	}


	return m_pDevInstance;
}

CPUDevice* CPUDevice::GetInstance()
{
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
	
	InfoLog(m_logDescriptor, m_iLogHandle, L"Get Device Info Function enter");
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
				return CL_SUCCESS;

			}
		case( CL_DEVICE_VENDOR_ID):
			{
				*pinternalRetunedValueSize = sizeof(cl_uint);
				if(NULL != paramVal && valSize != *pinternalRetunedValueSize)
				{
					return CL_INVALID_VALUE;
				}
				//if OUT paramVal is NULL it should be ignored
				if(NULL != paramVal)
				{
					//Get CPUID
					int viCPUInfo[4] = {-1};
					// get the CPU info
					__cpuid(viCPUInfo, 0);
					
					*(cl_uint*)paramVal = (cl_uint)viCPUInfo[0];
				}
				return CL_SUCCESS;
			}
		case( CL_DEVICE_MAX_COMPUTE_UNITS):
			{
				*pinternalRetunedValueSize = sizeof(cl_uint);
				if(NULL != paramVal && valSize != *pinternalRetunedValueSize)
				{
					return CL_INVALID_VALUE;
				}
				//if OUT paramVal is NULL it should be ignored
				if(NULL != paramVal)
				{
					//Get CPUID
					int viCPUInfo[4] = {-1};
					// get the CPU info
					__cpuid(viCPUInfo, 1);
					cl_uint uiCoreCount = (viCPUInfo[1] >> 16) & 0xff;

					*(cl_uint*)paramVal = uiCoreCount;
				}
				return CL_SUCCESS;
				
			}
		case( CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS):
		case( CL_DEVICE_MAX_WORK_GROUP_SIZE):
		case( CL_DEVICE_MAX_WORK_ITEM_SIZES):
		case( CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR):
		case( CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT):
		case( CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT):
		case( CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG):
		case( CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT):
		case( CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE):
		case( CL_DEVICE_MAX_CLOCK_FREQUENCY):
		case( CL_DEVICE_ADDRESS_BITS):
		case( CL_DEVICE_MAX_READ_IMAGE_ARGS):
		case( CL_DEVICE_MAX_WRITE_IMAGE_ARGS):
		case( CL_DEVICE_MAX_MEM_ALLOC_SIZE):
		case( CL_DEVICE_IMAGE2D_MAX_WIDTH):
		case( CL_DEVICE_IMAGE2D_MAX_HEIGHT):
		case( CL_DEVICE_IMAGE3D_MAX_WIDTH):
		case( CL_DEVICE_IMAGE3D_MAX_HEIGHT):
		case( CL_DEVICE_IMAGE3D_MAX_DEPTH):
		case( CL_DEVICE_IMAGE_SUPPORT):
		case( CL_DEVICE_MAX_PARAMETER_SIZE):
		case( CL_DEVICE_MAX_SAMPLERS):
		case( CL_DEVICE_MEM_BASE_ADDR_ALIGN):
		case( CL_DEVICE_MAX_DATA_TYPE_ALIGN_SIZE):
		case( CL_DEVICE_SINGLE_FP_CONFIG):
		case( CL_DEVICE_GLOBAL_MEM_CACHE_TYPE):
		case( CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE):
		case( CL_DEVICE_GLOBAL_MEM_CACHE_SIZE):
		case( CL_DEVICE_GLOBAL_MEM_SIZE):
		case( CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE):
		case( CL_DEVICE_MAX_CONSTANT_ARGS ):
		case( CL_DEVICE_LOCAL_MEM_TYPE):
		case( CL_DEVICE_LOCAL_MEM_SIZE):
		case( CL_DEVICE_ERROR_CORRECTION_SUPPORT):
		case( CL_DEVICE_PROFILING_TIMER_RESOLUTION):
		case( CL_DEVICE_ENDIAN_LITTLE):
			// FALL THROUGH
			return CL_INVALID_VALUE;
		case( CL_DEVICE_AVAILABLE):
			{
				*pinternalRetunedValueSize = sizeof(cl_bool);
				if(NULL != paramVal && valSize != *pinternalRetunedValueSize)
				{
					return CL_INVALID_VALUE;
				}
				//if OUT paramVal is NULL it should be ignored
				if(NULL != paramVal)
				{
					*(cl_bool*)paramVal = CL_TRUE;
				}
				return CL_SUCCESS;
			}
			

		case( CL_DEVICE_EXECUTION_CAPABILITIES):
			{
				*pinternalRetunedValueSize = sizeof(cl_device_exec_capabilities);
				if(NULL != paramVal && valSize != *pinternalRetunedValueSize)
				{
					return CL_INVALID_VALUE;
				}
				//if OUT paramVal is NULL it should be ignored
				if(NULL != paramVal)
				{
					cl_device_exec_capabilities execCapabilities = CL_EXEC_NATIVE_FN_AS_KERNEL;
					*(cl_device_exec_capabilities*)paramVal = execCapabilities;
				}
				return CL_SUCCESS;
								
			}
		case( CL_DEVICE_QUEUE_PROPERTIES ):
			{
				*pinternalRetunedValueSize = sizeof(cl_command_queue_properties);
				if(NULL != paramVal && valSize != *pinternalRetunedValueSize)
				{
					return CL_INVALID_VALUE;
				}
				//if OUT paramVal is NULL it should be ignored
				if(NULL != paramVal)
				{
					cl_command_queue_properties queueProperties = CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE | CL_QUEUE_PROFILING_ENABLE;
					*(cl_device_exec_capabilities*)paramVal = queueProperties;
				}
				return CL_SUCCESS;
			}
		case( CL_DEVICE_COMPILER_AVAILABLE):
			{
				*pinternalRetunedValueSize = sizeof(cl_bool);
				if(NULL != paramVal && valSize != *pinternalRetunedValueSize)
				{
					return CL_INVALID_VALUE;
				}
					//if OUT paramVal is NULL it should be ignored
				if(NULL != paramVal)
				{
					*(cl_bool*)paramVal = CL_FALSE;//Currently we dont have compiler yet
				}
				return CL_SUCCESS; 
			}
		case( CL_DEVICE_NAME):
			{
				*pinternalRetunedValueSize = strlen("CPU");
				if(NULL != paramVal && valSize < *pinternalRetunedValueSize)
				{
					return CL_INVALID_VALUE;
				}
				//if OUT paramVal is NULL it should be ignored
				if(NULL != paramVal)
				{
			  		strcpy_s((char*)paramVal, valSize, "CPU");
				}

				return CL_SUCCESS; 
			}
		case( CL_DEVICE_VENDOR):
			{
				*pinternalRetunedValueSize = strlen(CPU_STRING);
				if(NULL != paramVal && valSize < *pinternalRetunedValueSize)
				{
					return CL_INVALID_VALUE;
				}
				//if OUT paramVal is NULL it should be ignored
				if(NULL != paramVal)
				{
					strcpy_s((char*)paramVal, valSize, CPU_STRING);
				}
				return CL_SUCCESS; 
			}

		case( CL_DEVICE_PROFILE):
			{
				*pinternalRetunedValueSize = strlen("FULL_PROFILE");
				if(NULL != paramVal && valSize < *pinternalRetunedValueSize)
				{
					return CL_INVALID_VALUE;
				}
				//if OUT paramVal is NULL it should be ignored
				if(NULL != paramVal)
				{
					strcpy_s((char*)paramVal, valSize, "FULL_PROFILE");
				}
				return CL_SUCCESS; 
			}
		case( CL_DRIVER_VERSION ):
		case( CL_DEVICE_VERSION):
			{
				*pinternalRetunedValueSize = strlen("1.0");
				if(NULL != paramVal && valSize < *pinternalRetunedValueSize)
				{
					return CL_INVALID_VALUE;
				}
				//if OUT paramVal is NULL it should be ignored
				if(NULL != paramVal)
				{
					strcpy_s((char*)paramVal, valSize, "1.0");
				}
				return CL_SUCCESS; 
			}
        
		case( CL_DEVICE_EXTENSIONS):
		default:
			return CL_INVALID_VALUE;
	};
	return CL_INVALID_VALUE;

}
// Execution commands
/****************************************************************************************************************
 clDevCreateCommandList
	Call Scheduler to create command list
********************************************************************************************************************/
cl_int CPUDevice::clDevCreateCommandList( cl_dev_cmd_list_props IN props, cl_dev_cmd_list* OUT list)
{
	InfoLog(m_logDescriptor, m_iLogHandle, L"clDevCreateCommandList Function enter");
	return m_pScheduler->createCommandList(props,list);
}
/****************************************************************************************************************
 clDevRetainCommandList
	Call Scheduler to retain command list
********************************************************************************************************************/
cl_int CPUDevice::clDevRetainCommandList( cl_dev_cmd_list IN list)
{
	InfoLog(m_logDescriptor, m_iLogHandle, L"clDevRetainCommandList Function enter");
	return m_pScheduler->retainCommandList(list);
}
/****************************************************************************************************************
 clDevReleaseCommandList
	Call Scheduler to release command list
********************************************************************************************************************/
cl_int CPUDevice::clDevReleaseCommandList( cl_dev_cmd_list IN list )
{
	InfoLog(m_logDescriptor, m_iLogHandle, L"clDevReleaseCommandList Function enter");
	return m_pScheduler->releaseCommandList(list);
}
/****************************************************************************************************************
 clDevCommandListExecute
	Call Scheduler to execute command list
********************************************************************************************************************/
cl_int CPUDevice::clDevCommandListExecute( cl_dev_cmd_list IN list, cl_dev_cmd_desc* IN cmds, cl_uint IN count)
{
	InfoLog(m_logDescriptor, m_iLogHandle, L"clDevCommandListExecute Function enter");
	return m_pScheduler->commandListExecute(list,cmds,count);
}
//Memory API's
/****************************************************************************************************************
 clDevGetSupportedImageFormats
	Call Memory Allocator to get supported image formats
********************************************************************************************************************/
cl_int CPUDevice::clDevGetSupportedImageFormats( cl_dev_mem_flags IN flags, cl_dev_mem_object_type IN imageType,
				cl_uint IN numEntries, cl_image_format* OUT formats, cl_uint* OUT numEntriesRet)
{
	InfoLog(m_logDescriptor, m_iLogHandle, L"clDevGetSupportedImageFormats Function enter");
	if(numEntriesRet)
	{
		*numEntriesRet = 0;
	}
	return CL_SUCCESS;
}
/****************************************************************************************************************
 clDevCreateMemoryObject
	Call Memory Allocator to create memory object
********************************************************************************************************************/
cl_int CPUDevice::clDevCreateMemoryObject( cl_dev_mem_flags IN flags, const cl_image_format* IN format,
						size_t IN width, size_t IN height, size_t IN depth, cl_dev_mem* OUT memObj)
{
	InfoLog(m_logDescriptor, m_iLogHandle, L"clDevCreateMemoryObject Function enter");
	return m_pMemoryAllocator->CreateObject(flags, format, width, height, depth, memObj);
}
/****************************************************************************************************************
 clDevDeleteMemoryObject
	Call Memory Allocator to delete memory object
********************************************************************************************************************/
cl_int CPUDevice::clDevDeleteMemoryObject( cl_dev_mem* IN memObj )
{
	InfoLog(m_logDescriptor, m_iLogHandle, L"clDevDeleteMemoryObject Function enter");
	return m_pMemoryAllocator->ReleaseObject(memObj);
}
/****************************************************************************************************************
 clDevCreateMappedRegion
	Call Memory Allocator to craete mapped region
********************************************************************************************************************/
cl_int CPUDevice::clDevCreateMappedRegion( cl_dev_mem IN memObj, const size_t IN origin[3], const size_t IN region[3],
						 void** OUT ptr, size_t* OUT row_pitch, size_t* OUT slice_pitch)
{
	InfoLog(m_logDescriptor, m_iLogHandle, L"clDevCreateMappedRegion Function enter");
	return m_pMemoryAllocator->CreateMappedRegion(memObj,origin, region, ptr, row_pitch, slice_pitch);

}
/****************************************************************************************************************
 clDevReleaseMappedRegion
	Call Memory Allocator to release mapped region
********************************************************************************************************************/
cl_int CPUDevice::clDevReleaseMappedRegion( cl_dev_mem IN memObj, void* IN ptr)
{
	InfoLog(m_logDescriptor, m_iLogHandle, L"clDevReleaseMappedRegion Function enter");
	return m_pMemoryAllocator->ReleaseMappedRegion(memObj,ptr);
}

/****************************************************************************************************************
 clDevCheckProgramBinary
	Call Program Serice to check binaries
********************************************************************************************************************/
cl_int CPUDevice::clDevCheckProgramBinary( size_t IN binSize, const void* IN bin )
{
	InfoLog(m_logDescriptor, m_iLogHandle, L"clDevCheckProgramBinary Function enter");
	return m_pProgramService->CheckProgramBinary(binSize, bin );
}

/*******************************************************************************************************************
clDevBuildProgram
	Call programService to build program
**********************************************************************************************************************/

cl_int CPUDevice::clDevBuildProgram( size_t IN binSize, const void* IN bin, const cl_char* IN options, void* IN userData,
				   cl_dev_binary_prop IN prop, cl_dev_program* OUT prog )
{
	InfoLog(m_logDescriptor, m_iLogHandle, L"clDevBuildProgram Function enter");
	return m_pProgramService->BuildProgram(binSize, bin, options, userData, prop, prog );
}

/*******************************************************************************************************************
clDevReleaseProgram
	Call programService to release program
**********************************************************************************************************************/

cl_int CPUDevice::clDevReleaseProgram( cl_dev_program IN prog )
{
	InfoLog(m_logDescriptor, m_iLogHandle, L"clDevReleaseProgram Function enter");
	return m_pProgramService->ReleaseProgram( prog );
}

/*******************************************************************************************************************
clDevUnloadCompiler
	Call programService to unload the backend compiler
**********************************************************************************************************************/
cl_int CPUDevice::clDevUnloadCompiler()
{
	InfoLog(m_logDescriptor, m_iLogHandle, L"clDevUnloadCompiler Function enter");
	return m_pProgramService->UnloadCompiler();
}
/*******************************************************************************************************************
clDevGetProgramBinary
	Call programService to get the program binary
**********************************************************************************************************************/
cl_int CPUDevice::clDevGetProgramBinary( cl_dev_program IN prog, size_t IN size, void* OUT binary, size_t* OUT sizeRet )
{
	InfoLog(m_logDescriptor, m_iLogHandle, L"clDevGetProgramBinary Function enter");
	return m_pProgramService->GetProgramBinary(prog, size, binary, sizeRet );
}
/*******************************************************************************************************************
clDevGetBuildLog
	Call programService to get the build log
**********************************************************************************************************************/
cl_int CPUDevice::clDevGetBuildLog( cl_dev_program IN prog, size_t IN size, char* OUT log, size_t* OUT sizeRet)
{
	InfoLog(m_logDescriptor, m_iLogHandle, L"clDevGetBuildLog Function enter");
	return m_pProgramService->GetBuildLog(prog, size, log, sizeRet);
}
/*******************************************************************************************************************
clDevUnloadCompiler
	Call programService to get supported binary description
**********************************************************************************************************************/
cl_int CPUDevice::clDevGetSupportedBinaries( cl_uint IN count, cl_prog_binary_desc* OUT types, size_t* OUT sizeRet )
{
	InfoLog(m_logDescriptor, m_iLogHandle, L"clDevGetSupportedBinaries Function enter");
	return m_pProgramService->GetSupportedBinaries(count,types,sizeRet );
}
/*******************************************************************************************************************
clDevUnloadCompiler
	Call programService to get kernel id from its name
**********************************************************************************************************************/
cl_int CPUDevice::clDevGetKernelId( cl_dev_program IN prog, const char* IN name, cl_dev_kernel* OUT kernelId )
{
	InfoLog(m_logDescriptor, m_iLogHandle, L"clDevGetKernelId Function enter");
	return m_pProgramService->GetKernelId(prog, name, kernelId );
}
/*******************************************************************************************************************
clDevUnloadCompiler
	Call programService to get kernels from the program
**********************************************************************************************************************/
cl_int CPUDevice::clDevGetProgramKernels( cl_dev_program IN prog, cl_uint IN numKernels, cl_dev_kernel* OUT kernels,
						 size_t* OUT numKernelsRet )
{
	InfoLog(m_logDescriptor, m_iLogHandle, L"clDevGetProgramKernels Function enter");
	return m_pProgramService->GetProgramKernels(prog, numKernels, kernels,numKernelsRet );
}
/*******************************************************************************************************************
clDevGetKernelInfo
	Call programService to get kernel info
**********************************************************************************************************************/
cl_int CPUDevice::clDevGetKernelInfo( cl_dev_kernel IN kernel, cl_dev_kernel_info IN param, size_t IN valueSize,
					void* OUT value, size_t* OUT valueSizeRet )
{
	InfoLog(m_logDescriptor, m_iLogHandle, L"clDevGetKernelInfo Function enter");
	return m_pProgramService->GetKernelInfo(kernel, param, valueSize,value,valueSizeRet );
}

/*******************************************************************************************************************
clDevCloseDevice
	Close device
**********************************************************************************************************************/
void CPUDevice::clDevCloseDevice(void)
{
	InfoLog(m_logDescriptor, m_iLogHandle, L"clCloseDevice Function enter");
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

	if ( NULL != m_pScheduler )
	{
		delete m_pScheduler;
		m_pScheduler = NULL;
	}
}