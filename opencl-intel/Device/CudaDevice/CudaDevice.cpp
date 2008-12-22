// CudaDevice.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "CudaDevice.h"
#include <fstream>

using namespace Intel::OpenCL;
using namespace std;


// Static members initialization
CudaDevice* CudaDevice::m_pDevInstance = NULL;

CudaDevice::CudaDevice(cl_uint devId, cl_dev_call_backs *devCallbacks, cl_dev_log_descriptor *logDesc)
{
	m_id = devId;
	m_NumberOfPrograms = 0;
}


CudaDevice::~CudaDevice()
{
	for (int i = 0; i < m_NumberOfPrograms; i++)
	{
		if (m_Programs[i].functions != NULL)
		{
			delete m_Programs[i].functions;
		}
		if (m_Programs[i].module != NULL)
		{
			cuModuleUnload(m_Programs[i].module);
		}
	}
	delete m_Programs;
}


// ---------------------------------------
// Public functions / Device entry points

CudaDevice* CudaDevice::CreateDevice(cl_uint devId, cl_dev_call_backs *devCallbacks, cl_dev_log_descriptor *logDesc)
{
	// TODO : ADD log
	CUresult CuRes = CUDA_SUCCESS;

	if ( NULL == m_pDevInstance )
	{
		m_pDevInstance = new CudaDevice(devId, devCallbacks, logDesc);

		CuRes = cuInit(0);
		if ( CuRes != CUDA_SUCCESS )
		{
			delete m_pDevInstance;
			return NULL;
		}

		CuRes = cuDeviceGet( &( m_pDevInstance->m_device ) , 0);
		if ( CuRes != CUDA_SUCCESS )
		{
			delete m_pDevInstance;
			return NULL;
		}

		CuRes = cuCtxCreate( &( m_pDevInstance->m_context ), 0, m_pDevInstance->m_device);
		if ( CuRes != CUDA_SUCCESS )
		{
			delete m_pDevInstance;
			return NULL;
		}
	}

	return m_pDevInstance;
}

CudaDevice* CudaDevice::GetInstance()
{
	return m_pDevInstance;
}

// Device entry points
cl_int CudaDevice::clDevGetDeviceInfo(cl_device_info IN param, size_t IN val_size, void* OUT param_val,
				size_t* OUT param_val_size_ret)
{
	// TODO : ADD log
	return CL_DEV_INVALID_OPERATION;
}

cl_int CudaDevice::clDevCreateCommandList( cl_dev_cmd_list_props IN props, cl_dev_cmd_list* OUT list)
{
	// TODO : ADD log
	return CL_DEV_INVALID_OPERATION;
}

cl_int CudaDevice::clDevRetainCommandList( cl_dev_cmd_list IN list)
{
	// TODO : ADD log
	return CL_DEV_INVALID_OPERATION;
}

cl_int CudaDevice::clDevReleaseCommandList( cl_dev_cmd_list IN list )
{
	// TODO : ADD log
	return CL_DEV_INVALID_OPERATION;
}

cl_int CudaDevice::clDevCommandListExecute( cl_dev_cmd_list IN list, cl_dev_cmd_desc* IN cmds, cl_int IN count)
{
	// TODO : ADD log
	return CL_DEV_INVALID_OPERATION;
}

cl_int CudaDevice::clDevGetSupportedImageFormats( cl_dev_mem_flags IN flags, cl_dev_mem_object_type IN image_type,
				cl_uint IN num_entries, cl_image_format* OUT formats, cl_uint* OUT num_entries_ret)
{
	// TODO : ADD log
	return CL_DEV_INVALID_OPERATION;
}

cl_int CudaDevice::clDevCreateMemoryObject( cl_dev_mem_flags IN flags, const cl_image_format* IN format,
						size_t IN width, size_t IN height, size_t IN depth, cl_dev_mem* OUT memObj)
{
	// TODO : ADD log
	return CL_DEV_INVALID_OPERATION;
}

cl_int CudaDevice::clDevDeleteMemoryObject( cl_dev_mem* IN memObj )
{
	// TODO : ADD log
	return CL_DEV_INVALID_OPERATION;
}

cl_int CudaDevice::clDevCreateMappedRegion( cl_dev_mem IN memObj, const size_t IN origin[3], const size_t IN region[3],
						 void** OUT ptr, size_t* OUT row_pitch, size_t* OUT slice_pitch)
{
	// TODO : ADD log
	return CL_DEV_INVALID_OPERATION;
}

cl_int CudaDevice::clDevReleaseMappedRegion( cl_dev_mem IN memObj, void* IN ptr)
{
	// TODO : ADD log
	return CL_DEV_INVALID_OPERATION;
}

cl_int CudaDevice::clDevCheckProgramBinary( size_t IN bin_size, const void* IN bin )
{
	// TODO : ADD log
	return CL_DEV_INVALID_OPERATION;
}

cl_int CudaDevice::clDevBuildProgram( size_t IN bin_size, const void* IN bin, const cl_char* IN options, void* IN user_data,
				   cl_dev_binary_prop IN prop, cl_dev_program* OUT prog )
{
	// TODO : ADD log
	CUresult CuRes = CUDA_SUCCESS;
	cl_int ClRes = CL_DEV_SUCCESS;

	if (prop != CL_DEV_BINARY_USER)
	{
		return CL_DEV_INVALID_BINARY;
	}

	CUDA_MODULE tProg;
	tProg.NumberOfFunctions = 0;
	CuRes = cuModuleLoad( &( tProg.module ), (const char*)bin );
	if (CuRes != CUDA_SUCCESS)
	{
		return CL_DEV_INVALID_BINARY;
	}

	ClRes = m_pDevInstance->m_ParseCubin((char*)bin, tProg);
	if (ClRes != CL_DEV_SUCCESS)
	{
		cuModuleUnload(tProg.module);
		return CL_DEV_INVALID_BINARY;
	}

	for ( int i = 0; i < tProg.NumberOfFunctions; i++ )
	{
		CuRes = cuModuleGetFunction( &(tProg.functions[i].function), tProg.module, tProg.functions[i].OriginalName);
	}


	m_pDevInstance->m_Programs[ m_pDevInstance->m_NumberOfPrograms ] = tProg;
	*((int*)prog) = m_pDevInstance->m_NumberOfPrograms;
	m_pDevInstance->m_NumberOfPrograms++;
	return CL_DEV_SUCCESS;
}

cl_int CudaDevice::clDevUnloadCompiler()
{
	// TODO : ADD log
	return CL_DEV_INVALID_OPERATION;
}

cl_int CudaDevice::clDevGetProgramBinary( cl_dev_program IN prog, const void** OUT binary, size_t* OUT size_ret )
{
	// TODO : ADD log
	return CL_DEV_INVALID_OPERATION;
}

cl_int CudaDevice::clDevGetBuildLog( cl_dev_program IN prog, size_t IN size, char* OUT log, size_t* OUT size_ret)
{
	// TODO : ADD log
	return CL_DEV_INVALID_OPERATION;
}

cl_int CudaDevice::clDevGetSupportedBinaries( cl_uint IN count, cl_prog_binary_desc* OUT types, size_t* OUT size_ret )
{
	// TODO : ADD log
	return CL_DEV_INVALID_OPERATION;
}

cl_int CudaDevice::clDevGetKernelId( cl_dev_program IN prog, const char* IN name, cl_dev_kernel* OUT kernel_id )
{
	// TODO : ADD log

	if ( name == NULL )
	{
		return CL_DEV_INVALID_VALUE;
	}
	if ( (*(int*)prog) >= m_pDevInstance->m_NumberOfPrograms )
	{
		return CL_DEV_INVALID_PROGRAM;
	}
	CUDA_MODULE tprog = m_pDevInstance->m_Programs[*(int*)prog];
	for (int i = 0; i < tprog.NumberOfFunctions; i++)
	{
		if ( strcmp(tprog.functions[i].Name, name) == 0 )
		{
			*(int*)kernel_id = i;
			return CL_DEV_SUCCESS;
		}
	}
	return CL_INVALID_KERNEL_NAME;
}

cl_int CudaDevice::clDevGetProgramKernels( cl_dev_program IN prog, cl_uint IN num_kernels, cl_dev_kernel* OUT kernels,
						 size_t* OUT num_kernels_ret )
{
	// TODO : ADD log
	return CL_DEV_INVALID_OPERATION;
}

cl_int CudaDevice::clDevGetKernelInfo( cl_dev_kernel IN kernel, cl_dev_kernel_info IN param, size_t IN value_size,
					void* OUT value, size_t* OUT value_size_ret )
{
	// TODO : ADD log
	return CL_DEV_INVALID_OPERATION;
}



cl_int CudaDevice::m_ParseCubin( const char* stCubinName , CUDA_MODULE OUT &Prog)
{
	ifstream fCubin;
	char st[N_MAX_STRING_SIZE];
	fCubin.open( stCubinName, ios_base::in );
	if ( fCubin.bad() )
	{
		fCubin.close();
		return CL_DEV_ERROR_FAIL;
	}
	fCubin >> st;
	while (st[0] != NULL)
	{
		if ( strncmp(st, "__globfunc__", 12) == 0 )
		{
			CUDA_FUNCTION temp_kernel;
			temp_kernel.NumberOfArguments = 0;
			strcpy_s(temp_kernel.OriginalName, N_MAX_STRING_SIZE, st);
			int index = 13;
			int iTemp = atoi(&st[index]);
			while( isdigit(st[index]) != 0)
			{
				index++;
			}
			strncpy_s(temp_kernel.Name, N_MAX_STRING_SIZE, &st[index], iTemp);
			index += iTemp;
			while (st[index] != NULL)
			{
				cl_kernel_arg_type temp_parameter;
				switch (st[index])
				{
				case 'i' : temp_parameter = (cl_kernel_arg_type)sizeof(int);
					break;
				case 'f' : temp_parameter = (cl_kernel_arg_type)sizeof(float);
					break;
				case 'd' : temp_parameter = (cl_kernel_arg_type)sizeof(double);
					break;
				case 'c' : temp_parameter = (cl_kernel_arg_type)sizeof(char);
					break;
				case 'j' : temp_parameter = (cl_kernel_arg_type)sizeof(unsigned int);
					break;
				case 'h' : temp_parameter = (cl_kernel_arg_type)sizeof(unsigned char);
					break;
				case 'P' : index++;
					switch (st[index])
					{
					case 'i' : 
					case 'f' :
					case 'd' :
					case 'c' :
					case 'j' :
					case 'h' : temp_parameter = CL_KRNL_ARG_PTR;
						break;
					default: fCubin.close();
						return CL_DEV_ERROR_FAIL;
					}
					break;
				default : fCubin.close();
					return CL_DEV_ERROR_FAIL;
				}
				temp_kernel.Arguments[ temp_kernel.NumberOfArguments ] = temp_parameter;
				temp_kernel.NumberOfArguments++;
				index++;
			}
			Prog.functions[Prog.NumberOfFunctions]=temp_kernel;
			Prog.NumberOfFunctions++;
		}
		fCubin >> st;
	}
	fCubin.close();
	return CL_DEV_SUCCESS;
}