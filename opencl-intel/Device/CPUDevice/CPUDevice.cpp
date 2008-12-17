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
#include <intrin.h>
#include <string.h>

#include "CPUDevice.h"
#include "ProgramService.h"


using namespace Intel::OpenCL;


wchar_t* ClDevErrTxt(cl_dev_err_code error_code)
{
	switch(error_code)
	{
		case (CL_DEV_ERROR_FAIL): return L"CL_DEV_ERROR_FAIL";
		case (CL_DEV_INVALID_VALUE): return L"CL_DEV_INVALID_VALUE";
		case (CL_DEV_INVALID_PROPERTIES): return L"CL_DEV_INVALID_PROPERTIES";
		case (CL_DEV_OUT_OF_MEMORY): return L"CL_DEV_OUT_OF_MEMORY";
		case (CL_DEV_INVALID_COMMAND_LIST): return L"CL_DEV_INVALID_COMMAND_LIST";
		case (CL_DEV_INVALID_COMMAND_TYPE): return L"CL_DEV_INVALID_COMMAND_TYPE";
		case (CL_DEV_INVALID_MEM_OBJECT): return L"CL_DEV_INVALID_MEM_OBJECT";
		case (CL_DEV_INVALID_KERNEL): return L"CL_DEV_INVALID_KERNEL";
		case (CL_DEV_INVALID_OPERATION): return L"CL_DEV_INVALID_OPERATION";
		case (CL_DEV_INVALID_WRK_DIM): return L"CL_DEV_INVALID_WRK_DIM";
		case (CL_DEV_INVALID_WG_SIZE): return L"CL_DEV_INVALID_WG_SIZE";
		case (CL_DEV_INVALID_GLB_OFFSET): return L"CL_DEV_INVALID_GLB_OFFSET";
		case (CL_DEV_INVALID_WRK_ITEM_SIZE): return L"CL_DEV_INVALID_WRK_ITEM_SIZE";
		case (CL_DEV_INVALID_IMG_FORMAT): return L"CL_DEV_INVALID_IMG_FORMAT";
		case (CL_DEV_INVALID_IMG_SIZE): return L"CL_DEV_INVALID_IMG_SIZE";
		case (CL_DEV_OBJECT_ALLOC_FAIL): return L"CL_DEV_INVALID_COMMAND_LIST";
		case (CL_DEV_INVALID_BINARY): return L"CL_DEV_INVALID_BINARY";
		case (CL_DEV_INVALID_BUILD_OPTIONS): return L"CL_DEV_INVALID_BUILD_OPTIONS";
		case (CL_DEV_INVALID_PROGRAM): return L"CL_DEV_INVALID_PROGRAM";
		case (CL_DEV_STILL_BUILDING): return L"CL_DEV_STILL_BUILDING";
		case (CL_DEV_INVALID_KERNEL_NAME): return L"CL_DEV_INVALID_KERNEL_NAME";
	
	default: return L"Unknown Error Code";
	}
}

// Static members initialization
CPUDevice* CPUDevice::m_pDevInstance = NULL;

CPUDevice::CPUDevice(cl_uint devId, cl_dev_call_backs *devCallbacks, cl_dev_log_descriptor *logDesc)
{
	uiCpuId = devId;
	memcpy(&frameWorkCallBacks, devCallbacks, sizeof(frameWorkCallBacks));
}


// ---------------------------------------
// Public functions / Device entry points

CPUDevice* CPUDevice::CreateDevice(cl_uint devId, cl_dev_call_backs *devCallbacks, cl_dev_log_descriptor *logDesc)
{
	// TODO : ADD log
	if ( NULL == m_pDevInstance )
	{
		m_pDevInstance = new CPUDevice(devId, devCallbacks, logDesc);
	}

	return m_pDevInstance;
}

CPUDevice* CPUDevice::GetInstance()
{
	return m_pDevInstance;
}

// Device entry points
cl_int CPUDevice::clDevGetDeviceInfo(cl_device_info IN param, size_t IN val_size, void* OUT param_val,
				size_t* OUT param_val_size_ret)
{
	size_t  internalRetunedValueSize = val_size;
	size_t  *pinternalRetunedValueSize;

	//if OUT param_val_size_ret is NULL it should be ignopred
	if(param_val_size_ret)
	{
		pinternalRetunedValueSize = param_val_size_ret;
	}
	else
	{
		pinternalRetunedValueSize = &internalRetunedValueSize;
	}
	// TODO : ADD log
	switch (param)
	{
		case( CL_DEVICE_TYPE):
			{
				*pinternalRetunedValueSize = sizeof(cl_device_type);
				if(val_size != *pinternalRetunedValueSize)
				{
					return CL_INVALID_VALUE;
				}
				*(cl_device_type*)param_val = (cl_device_type)CL_DEVICE_TYPE_CPU;
				return CL_SUCCESS;

			}
		case( CL_DEVICE_VENDOR_ID):
			{
				*pinternalRetunedValueSize = sizeof(cl_uint);
				if(val_size != *pinternalRetunedValueSize)
				{
					return CL_INVALID_VALUE;
				}
				//Get CPUID
				int viCPUInfo[4] = {-1};
				// get the CPU info
				__cpuid(viCPUInfo, 0);
				
				*(cl_uint*)param_val = (cl_uint)viCPUInfo[0];
				return CL_SUCCESS;
			}
		case( CL_DEVICE_MAX_COMPUTE_UNITS):
			{
				*pinternalRetunedValueSize = sizeof(cl_uint);
				if(val_size != *pinternalRetunedValueSize)
				{
					return CL_INVALID_VALUE;
				}
				//Get CPUID
				int viCPUInfo[4] = {-1};
				// get the CPU info
				__cpuid(viCPUInfo, 1);
				cl_uint uiCoreCount = (viCPUInfo[1] >> 16) & 0xff;

				*(cl_uint*)param_val = uiCoreCount;
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
			return CL_INVALID_VALUE;
		case( CL_DEVICE_AVAILABLE):
			{
				*pinternalRetunedValueSize = sizeof(cl_bool);
				if(val_size != *pinternalRetunedValueSize)
				{
					return CL_INVALID_VALUE;
				}
				*(cl_bool*)param_val = CL_TRUE;
				return CL_SUCCESS;
			}
			

		case( CL_DEVICE_EXECUTION_CAPABILITIES):
			{
				*pinternalRetunedValueSize = sizeof(cl_device_exec_capabilities);
				if(val_size != *pinternalRetunedValueSize)
				{
					return CL_INVALID_VALUE;
				}
				cl_device_exec_capabilities execCapabilities = CL_EXEC_NATIVE_FN_AS_KERNEL;

				*(cl_device_exec_capabilities*)param_val = execCapabilities;
				return CL_SUCCESS;
								
			}
		case( CL_DEVICE_QUEUE_PROPERTIES ):
			{
				*pinternalRetunedValueSize = sizeof(cl_command_queue_properties);
				if(val_size != *pinternalRetunedValueSize)
				{
					return CL_INVALID_VALUE;
				}
				cl_command_queue_properties queueProperties = CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE | CL_QUEUE_PROFILING_ENABLE;

				*(cl_device_exec_capabilities*)param_val = queueProperties;
				return CL_SUCCESS;
			}
		case( CL_DEVICE_COMPILER_AVAILABLE):
			{
				*pinternalRetunedValueSize = sizeof(cl_bool);
				if(val_size != *pinternalRetunedValueSize)
				{
					return CL_INVALID_VALUE;
				}
				*(cl_bool*)param_val = CL_FALSE;//Currently we dont have compiler yet
				return CL_SUCCESS; 
			}
		case( CL_DEVICE_NAME):
			{
				*pinternalRetunedValueSize = strlen("CPU");
				if(val_size < *pinternalRetunedValueSize)
				{
					return CL_INVALID_VALUE;
				}
				
			  	strcpy_s((char*)param_val, val_size, "CPU");

				return CL_SUCCESS; 
			}
		case( CL_DEVICE_VENDOR):
			{
				*pinternalRetunedValueSize = strlen(CPU_STRING);
				if(val_size < *pinternalRetunedValueSize)
				{
					return CL_INVALID_VALUE;
				}
				strcpy_s((char*)param_val, val_size, CPU_STRING);
				return CL_SUCCESS; 
			}

		case( CL_DEVICE_PROFILE):
			{
				*pinternalRetunedValueSize = strlen("FULL_PROFILE");
				if(val_size < *pinternalRetunedValueSize)
				{
					return CL_INVALID_VALUE;
				}
				strcpy_s((char*)param_val, val_size, "FULL_PROFILE");
				return CL_SUCCESS; 
			}
		case( CL_DRIVER_VERSION ):
		case( CL_DEVICE_VERSION):
			{
				*pinternalRetunedValueSize = strlen("1.0");
				if(val_size < *pinternalRetunedValueSize)
				{
					return CL_INVALID_VALUE;
				}
				strcpy_s((char*)param_val, val_size, "1.0");
				return CL_SUCCESS; 
			}
        
		case( CL_DEVICE_EXTENSIONS):
		default:
			return CL_INVALID_VALUE;
	};
	return CL_INVALID_VALUE;

}
// Execution commands
cl_int CPUDevice::clDevCreateCommandList( cl_dev_cmd_list_props IN props, cl_dev_cmd_list* OUT list)
{
	// TODO : ADD log
	return CL_DEV_INVALID_OPERATION;
}

cl_int CPUDevice::clDevRetainCommandList( cl_dev_cmd_list IN list)
{
	// TODO : ADD log
	return CL_DEV_INVALID_OPERATION;
}

cl_int CPUDevice::clDevReleaseCommandList( cl_dev_cmd_list IN list )
{
	// TODO : ADD log
	return CL_DEV_INVALID_OPERATION;
}

cl_int CPUDevice::clDevCommandListExecute( cl_dev_cmd_list IN list, cl_dev_cmd_desc* IN cmds, cl_int IN count)
{
	// TODO : ADD log
	return CL_DEV_INVALID_OPERATION;
}

cl_int CPUDevice::clDevGetSupportedImageFormats( cl_dev_mem_flags IN flags, cl_dev_mem_object_type IN image_type,
				cl_uint IN num_entries, cl_image_format* OUT formats, cl_uint* OUT num_entries_ret)
{
	// TODO : ADD log
	return CL_DEV_INVALID_OPERATION;
}

cl_int CPUDevice::clDevCreateMemoryObject( cl_dev_mem_flags IN flags, const cl_image_format* IN format,
						size_t IN width, size_t IN height, size_t IN depth, cl_dev_mem* OUT memObj)
{
	// TODO : ADD log
	return CL_DEV_INVALID_OPERATION;
}

cl_int CPUDevice::clDevDeleteMemoryObject( cl_dev_mem* IN memObj )
{
	// TODO : ADD log
	return CL_DEV_INVALID_OPERATION;
}

cl_int CPUDevice::clDevCreateMappedRegion( cl_dev_mem IN memObj, const size_t IN origin[3], const size_t IN region[3],
						 void** OUT ptr, size_t* OUT row_pitch, size_t* OUT slice_pitch)
{
	// TODO : ADD log
	return CL_DEV_INVALID_OPERATION;
}

cl_int CPUDevice::clDevReleaseMappedRegion( cl_dev_mem IN memObj, void* IN ptr)
{
	// TODO : ADD log
	return CL_DEV_INVALID_OPERATION;
}

cl_int CPUDevice::clDevCheckProgramBinary( size_t IN bin_size, const void* IN bin )
{
	// TODO : ADD log
	return CL_DEV_INVALID_OPERATION;
}

cl_int CPUDevice::clDevBuildProgram( size_t IN bin_size, const void* IN bin, const cl_char* IN options, void* IN user_data,
				   cl_dev_binary_prop IN prop, cl_dev_program* OUT prog )
{
	// TODO : ADD log
	return CL_DEV_INVALID_OPERATION;
}

cl_int CPUDevice::clDevUnloadCompiler()
{
	// TODO : ADD log
	return CL_DEV_INVALID_OPERATION;
}

cl_int CPUDevice::clDevGetProgramBinary( cl_dev_program IN prog, size_t IN size, void* OUT binary, size_t* OUT size_ret )
{
	// TODO : ADD log
	return CL_DEV_INVALID_OPERATION;
}

cl_int CPUDevice::clDevGetBuildLog( cl_dev_program IN prog, size_t IN size, char* OUT log, size_t* OUT size_ret)
{
	// TODO : ADD log
	return CL_DEV_INVALID_OPERATION;
}

cl_int CPUDevice::clDevGetSupportedBinaries( cl_uint IN count, cl_prog_binary_desc* OUT types, size_t* OUT size_ret )
{
	// TODO : ADD log
	return CL_DEV_INVALID_OPERATION;
}

cl_int CPUDevice::clDevGetKernelId( cl_dev_program IN prog, const char* IN name, cl_dev_kernel* OUT kernel_id )
{
	// TODO : ADD log
	return CL_DEV_INVALID_OPERATION;
}

cl_int CPUDevice::clDevGetProgramKernels( cl_dev_program IN prog, cl_uint IN num_kernels, cl_dev_kernel* OUT kernels,
						 size_t* OUT num_kernels_ret )
{
	// TODO : ADD log
	return CL_DEV_INVALID_OPERATION;
}

cl_int CPUDevice::clDevGetKernelInfo( cl_dev_kernel IN kernel, cl_dev_kernel_info IN param, size_t IN value_size,
					void* OUT value, size_t* OUT value_size_ret )
{
	// TODO : ADD log
	return CL_DEV_INVALID_OPERATION;
}

