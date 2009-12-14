// CudaDevice.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "CudaDevice.h"
#include "cl_logger.h"

#include <assert.h>
#include <fstream>


using namespace Intel::OpenCL::CudaDevice;
using namespace std;


wchar_t* ClDevErr2Txt(cl_dev_err_code error_code)
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
cCudaDevice* cCudaDevice::m_pDevInstance = NULL;

//global functions declaration
const wstring str2wstr(string str);

//global variables declaration
cl_dev_log_descriptor log;
cl_int LogClientID;

//LOGER defines
#define LOG_DEBUG(DBG_PRINT, ...)			\
	if (log.pfnclLogAddLine) log.pfnclLogAddLine(LogClientID, LL_DEBUG, WIDEN(__FILE__), WIDEN(__FUNCTION__), __LINE__, DBG_PRINT, __VA_ARGS__);
#define LOG_INFO(DBG_PRINT, ...)			\
	if (log.pfnclLogAddLine) log.pfnclLogAddLine(LogClientID, LL_INFO, WIDEN(__FILE__), WIDEN(__FUNCTION__), __LINE__, DBG_PRINT, __VA_ARGS__);
#define LOG_ERROR(DBG_PRINT, ...)			\
	if (log.pfnclLogAddLine) log.pfnclLogAddLine(LogClientID, LL_ERROR, WIDEN(__FILE__), WIDEN(__FUNCTION__), __LINE__, DBG_PRINT, __VA_ARGS__);
#define LOG_CRITICAL(DBG_PRINT, ...)		\
	if (log.pfnclLogAddLine) log.pfnclLogAddLine(LogClientID, LL_CRITICAL, WIDEN(__FILE__), WIDEN(__FUNCTION__), __LINE__, DBG_PRINT, __VA_ARGS__);

cCudaDevice::cCudaDevice(cl_uint devId, cl_dev_call_backs *devCallbacks, cl_dev_log_descriptor *logDesc)
{
	m_id = devId;
	CUresult cuRes = CUDA_SUCCESS;

	cuRes = cuDeviceGet( & m_CuDev , 0);
	cuRes = cuDeviceGetProperties(&m_Prop, m_CuDev); 

	if(  NULL != devCallbacks )
	{
		memcpy(&m_CallBacks, devCallbacks, sizeof(m_CallBacks));
	}

	if (NULL != logDesc)
	{
		memcpy(&log, logDesc, sizeof(log));
		log.pfnclLogCreateClient( m_id, L"CUDA Device", &LogClientID);
	}

	else
	{
		log.pfnclLogAddLine = NULL;
		log.pfnclLogCreateClient = NULL;
		log.pfnclLogReleaseClient = NULL;
	}
	

}


cCudaDevice::~cCudaDevice()
{
	for (unsigned int i = 0; i < m_Programs.size(); i++)
	{
		delete m_Programs[i];
	}
	if (log.pfnclLogReleaseClient != NULL)
	{
		log.pfnclLogReleaseClient(LogClientID);
	}
}


// ---------------------------------------
// Public functions / Device entry points

cCudaDevice* cCudaDevice::CreateDevice(cl_uint devId, cl_dev_call_backs *devCallbacks, cl_dev_log_descriptor *logDesc)
{
	LOG_INFO(L"Enter CreateDevice, device ID : ");
	CUresult CuRes = CUDA_SUCCESS;

	CuRes = cuInit(0);
	if( CUDA_SUCCESS != CuRes )
	{
		LOG_ERROR(L"CUDA not supported, exit CreateDevice");
		return NULL;
	}

	if ( NULL == m_pDevInstance )
	{
		m_pDevInstance = new cCudaDevice(devId, devCallbacks, logDesc);

		cCudaCommandList *CommandList = new cCudaCommandList(m_pDevInstance->m_CommandLists.size(), m_pDevInstance);
		if( NULL == CommandList )
		{
			delete m_pDevInstance;
			return NULL;
		}
		m_pDevInstance->m_CommandLists.push_back(CommandList);
		CommandList->StartThread();


		cCudaProgram *DummyProgramt = new cCudaProgram( m_pDevInstance );
		if( NULL == DummyProgramt )
		{
			delete m_pDevInstance;
			return NULL;
		}
		m_pDevInstance->m_Programs.push_back( DummyProgramt );
	}
	
	LOG_INFO(L"Exit CreateDevice");
	return m_pDevInstance;
}

cCudaDevice* cCudaDevice::GetInstance()
{
	return m_pDevInstance;
}

// Device entry points
cl_int cCudaDevice::clDevGetDeviceInfo(cl_device_info IN param, size_t IN val_size, void* OUT param_val,
				size_t* OUT param_val_size_ret)
{
	LOG_INFO(L"Enter clDevGetDeviceInfo, device ID : %d", m_pDevInstance->m_id);
	size_t size_ret;
	void* val;
	switch (param)
	{
	case CL_DEVICE_TYPE : size_ret = sizeof(cl_device_type);
		val = new char[size_ret];
		*(cl_device_type*)val = CL_DEVICE_TYPE_GPU;
		break;
	case CL_DEVICE_MAX_COMPUTE_UNITS : size_ret = sizeof(cl_uint);
		val = new char[size_ret];
		cuDeviceGetAttribute((int*)val, CU_DEVICE_ATTRIBUTE_MULTIPROCESSOR_COUNT, m_pDevInstance->m_CuDev);
		break;
	case CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS : size_ret = sizeof(cl_uint);
		val = new char[size_ret];
		*(cl_uint*)val = 3;
		break;
	case CL_DEVICE_MAX_WORK_ITEM_SIZES : size_ret = 3 * sizeof(size_t);
		val = new char[size_ret];
		((size_t*)val)[0] = min(m_pDevInstance->m_Prop.maxThreadsDim[0], m_pDevInstance->m_Prop.maxGridSize[0]);
		((size_t*)val)[1] = min(m_pDevInstance->m_Prop.maxThreadsDim[1], m_pDevInstance->m_Prop.maxGridSize[1]);
		((size_t*)val)[2] = min(m_pDevInstance->m_Prop.maxThreadsDim[2], m_pDevInstance->m_Prop.maxGridSize[2]);
		break;
	case CL_DEVICE_MAX_WORK_GROUP_SIZE : size_ret = sizeof(size_t);
		val = new char[size_ret];
		*(size_t*)val = m_pDevInstance->m_Prop.maxThreadsPerBlock;
		break;
	case CL_DEVICE_MAX_CLOCK_FREQUENCY : size_ret = sizeof(cl_uint);
		val = new char[size_ret];
		*(cl_uint*)val = m_pDevInstance->m_Prop.clockRate / 1000;
		break;
	case CL_DEVICE_MAX_MEM_ALLOC_SIZE : size_ret = sizeof(cl_ulong);
		val = new char[size_ret];
		*(cl_ulong*)val = m_pDevInstance->m_Prop.sharedMemPerBlock;
		break;
	case CL_DEVICE_IMAGE_SUPPORT : size_ret = sizeof(cl_bool);
		val = new char[size_ret];
		*(cl_bool*)val = CL_FALSE;
		break;
	case CL_DEVICE_VENDOR_ID : size_ret = sizeof(cl_uint);
		val = new char[size_ret];
		*(cl_uint*)val = 0xCADA;
		break;
	default : LOG_ERROR(L"Invalid param : %d", param);
		LOG_INFO(L"Exit clDevGetDeviceInfo");
		return CL_DEV_INVALID_VALUE;
		break;
	}

	if( ( NULL == param_val ) && ( NULL == param_val_size_ret ) )
	{
		LOG_ERROR(L"Both param_val and param_val_size_ret are NULL");
		LOG_INFO(L"Exit clDevGetDeviceInfo");
		return CL_DEV_INVALID_VALUE;
	}
	if( ( NULL != param_val ) && ( NULL != param_val_size_ret ) && ( *param_val_size_ret < size_ret ) )
	{
		LOG_ERROR(L"param_val is not NULL and param_val_size_ret is insufficient");
		LOG_INFO(L"Exit clDevGetDeviceInfo");
		return CL_DEV_INVALID_VALUE;
	}
	if( NULL != param_val )
	{
		memcpy(param_val, val, size_ret);
		delete[] val;
	}
	if( NULL != param_val_size_ret )
	{
		*param_val_size_ret = size_ret;
	}
	LOG_INFO(L"Exit clDevGetDeviceInfo");
	return CL_DEV_SUCCESS;
}

cl_int cCudaDevice::clDevCreateCommandList( cl_dev_cmd_list_props IN props, cl_dev_cmd_list* OUT list)
{
	LOG_INFO(L"Enter clDevCreateCommandList, device ID : %d", m_pDevInstance->m_id);
	if( CL_DEV_LIST_NONE != props)
	{
		LOG_ERROR(L"Not supporting OOO");
		LOG_INFO(L"Exit clDevCreateCommandList");
		return CL_DEV_INVALID_PROPERTIES;
	}
	
	(*list) = ( m_pDevInstance->m_CommandLists[ m_pDevInstance->m_CommandLists.size() - 1 ] ) -> GetID();
	LOG_INFO(L"Exit clDevCreateCommandList");
	return CL_DEV_SUCCESS;
}

cl_int cCudaDevice::clDevRetainCommandList( cl_dev_cmd_list IN list)
{
	LOG_INFO(L"clDevRetainCommandList not implemented");
	return CL_DEV_SUCCESS;
}

cl_int cCudaDevice::clDevReleaseCommandList( cl_dev_cmd_list IN list )
{
	LOG_INFO(L"clDevReleaseCommandList not implemented");
	return CL_DEV_SUCCESS;
}

cl_int cCudaDevice::clDevCommandListExecute( cl_dev_cmd_list IN list, cl_dev_cmd_desc* IN *cmds, cl_uint IN count)
{
	


	LOG_INFO(L"Enter clDevCommandListExecute, device ID : %d, list : %d, count : %d", 
			  m_pDevInstance->m_id, (unsigned int)list, count);
	if( 0 != (unsigned int)list )
	{
		LOG_ERROR(L"Invalid command list");
		LOG_INFO(L"Exit clDevCommandListExecute");
		return CL_DEV_INVALID_COMMAND_LIST;
	}

	//from this point forward we assume valid input 
	cCudaCommandList *tList = m_pDevInstance->m_CommandLists[(unsigned int)list];

	for( unsigned int i = 0; i < count; i++ )
	{
		m_pDevInstance->m_CallBacks.pclDevCmdStatusChanged( cmds[i]->id, cmds[i]->data, CL_SUBMITTED, 0, 0);

		if( CL_DEV_CMD_READ == cmds[i]->type )
		{
			tList->PushRead(*(cmds + i));
		}
		else if( CL_DEV_CMD_WRITE == cmds[i]->type )
		{
			tList->PushWrite(*(cmds + i));
		}
		else if( CL_DEV_CMD_COPY == cmds[i]->type )
		{
			LOG_ERROR(L"recived unsuported command type \"CL_DEV_CMD_COPY\"");
		}
		else if( CL_DEV_CMD_MAP == cmds[i]->type )
		{
			LOG_ERROR(L"recived unsuported command type \"CL_DEV_CMD_MAP\"");
		}
		else if( CL_DEV_CMD_UNMAP == cmds[i]->type )
		{
			LOG_ERROR(L"recived unsuported command type \"CL_DEV_CMD_UNMAP\"");
		}
		else if( CL_DEV_CMD_EXEC_KERNEL == cmds[i]->type )
		{
			tList->PushKernel(*(cmds + i));
		}
		else if( CL_DEV_CMD_EXEC_TASK == cmds[i]->type )
		{
			LOG_ERROR(L"recived unsuported command type \"CL_DEV_CMD_EXEC_TASK\"");
		}
		else if( CL_DEV_CMD_EXEC_NATIVE == cmds[i]->type )
		{
			LOG_ERROR(L"recived unsuported command type \"CL_DEV_CMD_EXEC_NATIVE\"");
		}
	}
	LOG_INFO(L"Exit clDevCommandListExecute");
	return CL_DEV_SUCCESS;
}

cl_int cCudaDevice::clDevGetSupportedImageFormats( cl_dev_mem_flags IN flags, cl_dev_mem_object_type IN image_type,
				cl_uint IN num_entries, cl_image_format* OUT formats, cl_uint* OUT num_entries_ret)
{
	LOG_INFO(L"clDevGetSupportedImageFormats not implemented");
	return CL_DEV_SUCCESS;
}

cl_int cCudaDevice::clDevCreateMemoryObject( cl_dev_mem_flags IN flags, const cl_image_format* IN format,
						cl_uint	IN dim_count, const size_t* dim, void* buffer_ptr, const size_t* pitch,
						cl_dev_host_ptr_flags host_flags, cl_dev_mem* OUT memObj)
{
	//only supports buffers (dim_count = 1 and format = NULL) 
	LOG_INFO(L"Enter clDevCreateMemoryObject, device ID : %d", m_pDevInstance->m_id);
	if( 1 != dim_count )
	{
		LOG_ERROR(L"Received dim_count other then 1");
		LOG_INFO(L"Exit clDevCreateMemoryObject");
		return CL_DEV_INVALID_IMG_SIZE;
	}
	if( NULL != format )
	{
		LOG_ERROR(L"Received format other then NULL");
		LOG_INFO(L"Exit clDevCreateMemoryObject");
		return CL_DEV_INVALID_IMG_FORMAT;
	}

	cCudaMemObject* pMem = new cCudaMemObject(flags, format, dim_count, dim, buffer_ptr, pitch);
	if( NULL == pMem )
	{
		LOG_ERROR(L"Object allocation failed");
		LOG_INFO(L"Exit clDevCreateMemoryObject");
		return CL_DEV_OBJECT_ALLOC_FAIL;
	}

	cl_dev_mem tpMemObj = new (_cl_dev_mem);


	tpMemObj->allocId = m_pDevInstance->m_id;
	tpMemObj->objHandle = pMem;

	*memObj = tpMemObj;

	LOG_INFO(L"Exit clDevCreateMemoryObject");
	return CL_DEV_SUCCESS;
}

cl_int cCudaDevice::clDevDeleteMemoryObject( cl_dev_mem IN memObj )
{
	LOG_INFO(L"Enter clDevDeleteMemoryObject, device ID : %d", m_pDevInstance->m_id);
	cCudaMemObject* pMem = (cCudaMemObject*)memObj->objHandle;
	delete pMem;
	LOG_INFO(L"Exit clDevDeleteMemoryObject");
	return CL_DEV_SUCCESS;
}

cl_int cCudaDevice::clDevCreateMappedRegion( cl_dev_cmd_param_map* INOUT pMapParams )
{
	LOG_INFO(L"clDevCreateMappedRegion not implemented");
	return CL_DEV_SUCCESS;
}

cl_int cCudaDevice::clDevReleaseMappedRegion( cl_dev_cmd_param_map* IN pMapParams )
{
	LOG_INFO(L"clDevReleaseMappedRegion not implemented");
	return CL_DEV_SUCCESS;
}

cl_int cCudaDevice::clDevCheckProgramBinary( size_t IN bin_size, const void* IN bin )
{
	LOG_INFO(L"clDevCheckProgramBinary not implemented");
	return CL_DEV_SUCCESS;
}


cl_int cCudaDevice::clDevCreateProgram( size_t IN bin_size, const void* IN bin, cl_dev_binary_prop IN prop, cl_dev_program* OUT prog )
{
	LOG_INFO(L"Enter clDevCreateProgram, device ID : %d", m_pDevInstance->m_id);
	cl_prog_container *ProgContainer = (cl_prog_container*)bin;

	if (prop != CL_DEV_BINARY_USER)
	{
		LOG_ERROR(L"Invalid binary properties");
		LOG_INFO(L"Exit clDevCreateProgram");
		return CL_DEV_INVALID_BINARY;
	}

	if (ProgContainer->container_type != CL_PROG_CNT_PRIVATE)
	{
		LOG_ERROR(L"Invalid container type");
		LOG_INFO(L"Exit clDevCreateProgram");
		return CL_DEV_INVALID_BINARY;
	}

	if (ProgContainer->description.bin_type != CL_PROG_BIN_CUBIN)
	{
		LOG_ERROR(L"Invalid binary type");
		LOG_INFO(L"Exit clDevCreateProgram");
		return CL_DEV_INVALID_BINARY;
	}

	if ( NULL == ProgContainer->container )
	{
		LOG_ERROR(L"NULL == ProgContainer->container");
		LOG_INFO(L"Exit clDevCreateProgram");
		return CL_DEV_INVALID_BINARY;
	}

	if (ProgContainer->container_size != ( strlen( (char*)ProgContainer->container ) + 1 ) )
	{
		LOG_ERROR(L"Inconsistent container size");
		LOG_INFO(L"Exit clDevCreateProgram");
		return CL_DEV_INVALID_BINARY;
	}

	cCudaProgram *tProg = new cCudaProgram( m_pDevInstance, ProgContainer, m_pDevInstance->m_Programs.size() );
	m_pDevInstance->m_Programs.push_back(tProg);
	*prog = (cl_dev_program)tProg->m_ProgramID;

	LOG_INFO(L"Exit clDevCreateProgram");
	return CL_DEV_SUCCESS;
}

cl_int cCudaDevice::clDevBuildProgram(cl_dev_program prog, const cl_char *options, void *user_data)
{
	LOG_INFO(L"Enter clDevBuildProgram, device ID : %d, program ID : %d", m_pDevInstance->m_id, (cl_uint)prog);
	m_pDevInstance->m_CommandLists[0]->PushBuildProgram((cl_uint)prog, options, user_data);

	LOG_INFO(L"Exit clDevBuildProgram");
	return CL_DEV_SUCCESS;
}

cl_int cCudaDevice::clDevReleaseProgram( cl_dev_program IN prog )
{
	LOG_INFO(L"clDevReleaseProgram not implemented");
	return CL_DEV_SUCCESS;
}

cl_int cCudaDevice::clDevUnloadCompiler()
{
	LOG_INFO(L"clDevUnloadCompiler not implemented");
	return CL_DEV_SUCCESS;
}

cl_int cCudaDevice::clDevGetProgramBinary(cl_dev_program prog, size_t size, void *binary, size_t *size_ret)
{
	LOG_INFO(L"Enter clDevGetProgramBinary, device ID : %d, program ID : %d", m_pDevInstance->m_id, (cl_uint)prog);
	unsigned int uiProg = (unsigned int)prog;
	if( (uiProg < 0) || ( uiProg >= m_pDevInstance->m_Programs.size()) )
	{
		return CL_DEV_INVALID_VALUE;
	}

	cCudaProgram *tProg = m_pDevInstance->m_Programs[uiProg];

	LOG_INFO(L"Exit clDevGetProgramBinary");
	return tProg->GetBinary(size, binary, size_ret);
}

cl_int cCudaDevice::clDevGetBuildLog( cl_dev_program IN prog, size_t IN size, char* OUT cLog, size_t* OUT size_ret)
{
	LOG_INFO(L"clDevGetBuildLog not implemented");
	return CL_DEV_SUCCESS;
}

cl_int cCudaDevice::clDevGetSupportedBinaries( cl_uint IN count, cl_prog_binary_desc* OUT types, size_t* OUT size_ret )
{
	LOG_INFO(L"clDevGetSupportedBinaries not implemented");
	return CL_DEV_SUCCESS;
}

cl_int cCudaDevice::clDevGetKernelId( cl_dev_program IN prog, const char* IN name, cl_dev_kernel* OUT kernel_id )
{
	LOG_INFO(L"Enter clDevGetKernelId, device ID : %d, program ID : %d", m_pDevInstance->m_id, (cl_uint)prog);

	unsigned int uiProg = (unsigned int)prog;
	if ( name == NULL )
	{
		LOG_ERROR(L"name is NULL");
		LOG_INFO(L"Exit clDevGetKernelId");
		return CL_DEV_INVALID_VALUE;
	}
	if ( uiProg >= m_pDevInstance->m_Programs.size() )
	{
		LOG_ERROR(L"Invalid program");
		LOG_INFO(L"Exit clDevGetKernelId");
		return CL_DEV_INVALID_PROGRAM;
	}
	if ( NULL == m_pDevInstance->m_Programs[uiProg] )
	{
		LOG_ERROR(L"Invalid program");
		LOG_INFO(L"Exit clDevGetKernelId");
		return CL_DEV_INVALID_PROGRAM;
	}
	cCudaProgram *tProg = m_pDevInstance->m_Programs[uiProg];

	LOG_INFO(L"Exit clDevGetKernelId");
	return tProg->GetKernelID(name, kernel_id);
}

cl_int cCudaDevice::clDevGetProgramKernels( cl_dev_program IN prog, cl_uint IN num_kernels, cl_dev_kernel* OUT kernels,
						 cl_uint* OUT num_kernels_ret )
{
	LOG_INFO(L"Enter clDevGetProgramKernels, device ID : %d, program ID : %d", m_pDevInstance->m_id, (cl_uint)prog);

	unsigned int uiProg = (unsigned int)prog;
	if ( uiProg >= m_pDevInstance->m_Programs.size() )
	{
		LOG_ERROR(L"Invalid program");
		LOG_INFO(L"Exit clDevGetProgramKernels");
		return CL_DEV_INVALID_PROGRAM;
	}
	if ( NULL == m_pDevInstance->m_Programs[uiProg] )
	{
		LOG_ERROR(L"Invalid program");
		LOG_INFO(L"Exit clDevGetProgramKernels");
		return CL_DEV_INVALID_PROGRAM;
	}
	cCudaProgram *tProg = m_pDevInstance->m_Programs[uiProg];
	
	LOG_INFO(L"Exit clDevGetProgramKernels");
	return tProg->GetKernels(num_kernels, (KERNEL_ID**)kernels, num_kernels_ret);
}

cl_int cCudaDevice::clDevGetKernelInfo( cl_dev_kernel IN kernel, cl_dev_kernel_info IN param, size_t IN value_size,
					void* OUT value, size_t* OUT value_size_ret )
{
	KERNEL_ID* tKernel = (KERNEL_ID*)kernel;

	wstring KernelName = str2wstr( tKernel->KernelName );
	int ProgramID = tKernel->ProgramID;
	LOG_INFO(L"Enter clDevGetKernelInfo, device ID : %d, program ID : %d kernel name : \"%ws\"", m_pDevInstance->m_id, ProgramID, KernelName);
	
	if( CL_DEV_KERNEL_NAME == param )
	{
		if( ( NULL != value ) && ( value_size < tKernel->KernelName.length() + 1 ) )
		{
			LOG_ERROR(L"value is not NULL and value_size is insufficient");
			LOG_INFO(L"Exit clDevGetKernelInfo");
			return CL_DEV_INVALID_VALUE;
		}
		if( NULL != value )
		{
			strcpy((char*)value, tKernel->KernelName.c_str());
		}
		if( NULL != value_size_ret )
		{
			*value_size_ret = tKernel->KernelName.length() + 1;
		}
		return CL_DEV_SUCCESS;
	}

	if( CL_DEV_KERNEL_PROTOTYPE == param )
	{
		unsigned int uiProg = tKernel->ProgramID;
		if ( uiProg >= m_pDevInstance->m_Programs.size() )
		{
			return CL_DEV_INVALID_PROGRAM;
		}
		if ( NULL == m_pDevInstance->m_Programs[uiProg] )
		{
			return CL_DEV_INVALID_PROGRAM;
		}
		cCudaProgram *tProg = m_pDevInstance->m_Programs[uiProg];
		return tProg->GetKernelArguments(tKernel->KernelName, value_size, (cl_kernel_argument*)value, value_size_ret);
	}

	return CL_DEV_INVALID_VALUE;
}

//
//cCudaMemObject
//
cCudaMemObject::cCudaMemObject(cl_dev_mem_flags flags, 
							   const cl_image_format *format, 
							   cl_uint dim_count, 
							   const size_t *dim,
							   void* buffer_ptr,
							   const size_t* pitch)
{
	//only supports buffers (dim_count = 1 and format = NULL) 
	CUresult CuRes = CUDA_SUCCESS;
	m_RW = flags & 0x03 ;
	m_OnHost = flags & 0x04 ;
	m_DimCount = dim_count;
	m_dim = new size_t( dim_count );
	memcpy( m_dim, dim, dim_count * sizeof(size_t) );
	m_DevPtr = NULL;
	m_HostPtr = buffer_ptr;
	if ( NULL != buffer_ptr )
	{
		if( NULL != pitch )
		{
			LOG_ERROR(L"rceived pitch != NULL for 1D buffer");
		}
	}
}
cCudaMemObject::~cCudaMemObject()
{
	delete[] m_dim;
	if( NULL != m_DevPtr )
	{
		cuMemFree( m_DevPtr );
	}
}
CUdeviceptr cCudaMemObject::GetPtr()
{
	CUresult CuRes = CUDA_SUCCESS;
	if( NULL == m_DevPtr )
	{
		CuRes = cuMemAlloc( &m_DevPtr, m_dim[0]);
		if( CUDA_SUCCESS != CuRes )
		{
			return CL_DEV_OBJECT_ALLOC_FAIL;
		}
	}
	return m_DevPtr;
}
int cCudaMemObject::GetRW()
{
	return m_RW;
}
int cCudaMemObject::Read(cl_uint dim_count, size_t *origin, size_t *region, void *ptr, size_t *pitch)
{
	CUresult CuRes = CUDA_SUCCESS;
	if( m_DimCount != dim_count )
	{
		return CL_DEV_ERROR_FAIL;
	}
	//if( 0 == ( m_RW & 0x01 ) )
	//{
	//	return CL_DEV_INVALID_MEM_OBJECT;
	//}
	if( NULL == m_DevPtr )
	{
		CuRes = cuMemAlloc( &m_DevPtr, m_dim[0]);
		if( CUDA_SUCCESS != CuRes )
		{
			return CL_DEV_OBJECT_ALLOC_FAIL;
		}
	}
	CuRes = cuMemcpyDtoH(ptr, m_DevPtr + origin[0], region[0]);
	if( CUDA_SUCCESS != CuRes )
	{
		return CL_DEV_ERROR_FAIL;
	}
	return CL_DEV_SUCCESS;
}
int cCudaMemObject::Write(cl_uint dim_count, size_t *origin, size_t *region, void *ptr, size_t *pitch)
{
	CUresult CuRes = CUDA_SUCCESS;
	if( m_DimCount != dim_count )
	{
		return CL_DEV_ERROR_FAIL;
	}
	//if( 0 == ( m_RW & 0x02 ) )
	//{
	//	return CL_DEV_INVALID_MEM_OBJECT;
	//}
	if( NULL == m_DevPtr )
	{
		CuRes = cuMemAlloc( &m_DevPtr, m_dim[0]);
		if( CUDA_SUCCESS != CuRes )
		{
			return CL_DEV_OBJECT_ALLOC_FAIL;
		}
	}
	CuRes = cuMemcpyHtoD(m_DevPtr + origin[0], ptr, region[0]);
	if( CUDA_SUCCESS != CuRes )
	{
		return CL_DEV_ERROR_FAIL;
	}
	return CL_DEV_SUCCESS;
}
//
//cCudaProgram
//
cCudaProgram::cCudaProgram( cCudaDevice* device )
{
	m_module = NULL;
	m_ProgramID = -1;
	m_ProgContainer.container = new char[N_MAX_STRING_SIZE];
	m_ProgContainer.container_size = 0;
	m_device = device;
}
cCudaProgram::cCudaProgram( cCudaDevice* device, cl_prog_container *ProgContainer, cl_uint ID )
{
	m_module = NULL;
	m_ProgramID = ID;
	if( NULL != ProgContainer )
	{
		int NameLength = strlen( (char*)ProgContainer->container );
		m_ProgContainer.container = new char[NameLength + 1];
		m_ProgContainer.container_size = ProgContainer->container_size;
		m_ProgContainer.container_type = ProgContainer->container_type;
		m_ProgContainer.description = ProgContainer->description;
		strcpy_s( (char*)m_ProgContainer.container, NameLength + 1, (char*)ProgContainer->container );
		m_ProgramName = (char*)ProgContainer->container;
	}
	m_device = device;
}
cCudaProgram::~cCudaProgram()
{
	delete[] (m_ProgContainer.container);
	if( NULL != m_module )
	{
		cuModuleUnload(m_module);
	}
	m_kernels.clear();
}


cl_int cCudaProgram::GetBinary(size_t size, void *binary, size_t *size_ret)
{
	char* stCubinName = (char*)m_ProgContainer.container;

	if( (NULL == binary) && (0 == size) )
	{
		*size_ret = sizeof(cl_prog_container) + strlen(stCubinName) +1;
		return CL_DEV_SUCCESS;
	}

	if( size < (sizeof(cl_prog_container) + strlen(stCubinName) +1) )
	{
		return CL_DEV_INVALID_VALUE;
	}

	if( (NULL == binary) && (0 != size) )
	{
		return CL_DEV_INVALID_VALUE;
	}

	cl_prog_container *tContainer = (cl_prog_container*)binary;
	tContainer->container_size = m_ProgContainer.container_size;
	tContainer->container_type = m_ProgContainer.container_type;
	tContainer->description = m_ProgContainer.description;
	tContainer->container = ((char*)tContainer) + sizeof(cl_prog_container);
	memcpy((void*)tContainer->container, m_ProgContainer.container, strlen((char*)m_ProgContainer.container) + 1);
	if( NULL != size_ret )
	{
		*(size_ret) = sizeof(cl_prog_container) + strlen(stCubinName) +1;
	}
	return CL_DEV_SUCCESS;
}
cl_int cCudaProgram::GetKernelID(const char *name, cl_dev_kernel *kernel_id)
{
	if ( m_kernels.find(name) == m_kernels.end() )
	{
		return CL_INVALID_KERNEL_NAME;
	}
	cCudaKernel *tKernel = m_kernels[name];
	*kernel_id = (cl_dev_kernel)(& (tKernel->m_KernelID) );
	return CL_SUCCESS;
}
cl_int cCudaProgram::GetKernels(cl_uint num_kernels, KERNEL_ID **kernels, cl_uint *num_kernels_ret)
{
	if( ( NULL != kernels ) && ( num_kernels < m_kernels.size() ) )
	{
		return CL_DEV_INVALID_VALUE;
	}
	if( ( NULL == kernels ) && ( 0 != num_kernels ) )
	{
		return CL_DEV_INVALID_VALUE;
	}
	if( NULL != num_kernels_ret )
	{
		*num_kernels_ret = m_kernels.size();
	}
	if( NULL != kernels )
	{
		KERNELS::iterator it;
		int i;
		for ( it = m_kernels.begin(), i = 0; it != m_kernels.end(); it++, i++)
		{
			kernels[i] = &(it->second->m_KernelID);
		}
	}
	return CL_DEV_SUCCESS;
}
cl_int cCudaProgram::GetKernelArguments(string KernelName, 
									   size_t value_size, 
									   cl_kernel_argument* value, 
									   size_t* value_size_ret)
{
	size_t num_args = m_kernels[KernelName]->m_NumberOfArguments;
	if( ( NULL != value ) && ( value_size < num_args ) )
	{
		return CL_DEV_INVALID_VALUE;
	}
	if( NULL != value_size_ret )
	{
		*value_size_ret = num_args * sizeof(cl_kernel_argument);
	}
	if( NULL != value )
	{
		memcpy(value, m_kernels[KernelName]->m_Arguments, num_args * sizeof(cl_kernel_argument));
	}
	return CL_DEV_SUCCESS;
}
cl_int cCudaProgram::ParseCubin(const char *stCubinName)
{
	cl_kernel_argument prev_parameter;
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
			cCudaKernel *temp_kernel = new cCudaKernel;
			temp_kernel->m_OriginalName = st;
			temp_kernel->m_KernelID.ProgramID = m_ProgramID;
			int index = 13;
			int iTemp = atoi(&st[index]);
			while( isdigit(st[index]) != 0)
			{
				index++;
			}
			temp_kernel->m_KernelID.KernelName.assign(&st[index], iTemp);
			index += iTemp;
			while (st[index] != NULL)
			{
				cl_kernel_argument temp_parameter;
				cl_uint temp_MemFlag = CL_DEV_MEM_READ_WRITE;
				
				/*    char                unsigned char             */
				if( ( 'c' == st[index] ) || ( 'h' == st[index] ) )
				{
					temp_parameter.type = ('c' == st[index]) ? CL_KRNL_ARG_INT : CL_KRNL_ARG_UINT;
					temp_parameter.size_in_bytes = sizeof(char);
					prev_parameter = temp_parameter;
				}
				else if( ( 's' == st[index] ) || ( 't' == st[index] ) )
				{
					temp_parameter.type = ('s' == st[index]) ? CL_KRNL_ARG_INT : CL_KRNL_ARG_UINT;
					temp_parameter.size_in_bytes = sizeof(short);
					prev_parameter = temp_parameter;
				}
				else if( ( 'i' == st[index] ) || ( 'j' == st[index] )  )
				{
					temp_parameter.type = ('i' == st[index]) ? CL_KRNL_ARG_INT : CL_KRNL_ARG_UINT;
					temp_parameter.size_in_bytes = sizeof(int);
					prev_parameter = temp_parameter;
				}
				else if( ( 'l' == st[index] ) || ( 'm' == st[index] ) )
				{
					temp_parameter.type = ('l' == st[index]) ? CL_KRNL_ARG_INT : CL_KRNL_ARG_UINT;
					temp_parameter.size_in_bytes = sizeof(long);
					prev_parameter = temp_parameter;
				}
				else if ( 'f' == st[index] )
				{
					temp_parameter.type = CL_KRNL_ARG_FLOAT;
					temp_parameter.size_in_bytes = sizeof(float);
					prev_parameter = temp_parameter;
				}
				else if( 'd' == st[index] )
				{
					temp_parameter.type = CL_KRNL_ARG_DOUBLE;
					temp_parameter.size_in_bytes = sizeof(double);
					prev_parameter = temp_parameter;
				}
				else if( 'P' == st[index] )
				{
					index++;
					temp_parameter.type = CL_KRNL_ARG_PTR_GLOBAL;
					temp_parameter.size_in_bytes = 0;
					prev_parameter = temp_parameter;
					
					while( 'P' == st[index] )
					{
						index++;
					}
					if( 'K' == st[index] )
					{
						temp_parameter.type = CL_KRNL_ARG_PTR_CONST;
						temp_parameter.size_in_bytes = 0;
						prev_parameter = temp_parameter;
						index++;
					}
					if( 'S' == st[index] )
					{
						while( '_' != st[index] )
						{
							index++;
						}
					}
					if( 0 != isdigit( st[index] ) )
					{
						int num;
						num = atoi(st + index);
						while( 0 != isdigit( st[index] ) )
						{
							index++;
						}
						index += (num - 1);
					}
				}
				else if( ( 'S' == st[index] ) )
				{
					index++;
					while( '_' != st[index] )
					{
						index++;
					}
					temp_parameter = prev_parameter;
					prev_parameter = temp_parameter;
				}

				temp_kernel->m_Arguments[ temp_kernel->m_NumberOfArguments ] = temp_parameter;
				if( CL_KRNL_ARG_PTR_CONST == temp_parameter.type )
				{
					temp_MemFlag = CL_DEV_MEM_READ;
				}
				temp_kernel->m_MemFlags[ temp_kernel->m_NumberOfArguments ] = temp_MemFlag;
				temp_kernel->m_NumberOfArguments++;
				index++;
			}
			m_kernels[temp_kernel->m_KernelID.KernelName] = temp_kernel;
		}
		fCubin >> st;
	}
	fCubin.close();
	return CL_DEV_SUCCESS;
}
cl_int cCudaProgram::Build(const cl_char *options, void *user_data)
{
	KERNELS::iterator it;
	CUresult CuRes = CUDA_SUCCESS;
	cl_int ClRes = CL_DEV_SUCCESS;
	
	CuRes = cuModuleLoad( &m_module, m_ProgramName.c_str() );

	if (CuRes != CUDA_SUCCESS)
	{
		m_device->m_CallBacks.pclDevBuildStatusUpdate( (cl_dev_program)m_ProgramID , user_data, CL_BUILD_ERROR);
		return CL_DEV_INVALID_BINARY;
	}

	ClRes = ParseCubin( m_ProgramName.c_str() );

	if (ClRes != CL_DEV_SUCCESS)
	{
		cuModuleUnload(m_module);
		m_device->m_CallBacks.pclDevBuildStatusUpdate( (cl_dev_program)m_ProgramID , user_data, CL_BUILD_ERROR);
		return CL_DEV_INVALID_BINARY;
	}

	for ( it = m_kernels.begin(); it != m_kernels.end(); it++)
	{
		CuRes = cuModuleGetFunction( &(it->second->m_function), m_module, it->second->m_OriginalName.c_str());
	}

	m_device->m_CallBacks.pclDevBuildStatusUpdate( (cl_dev_program)m_ProgramID , user_data, CL_BUILD_SUCCESS);

	return CL_DEV_SUCCESS;
}
cl_int cCudaProgram::RunKernel( cl_dev_cmd_param_kernel* pKernelParam ,cl_dev_cmd_id id, void* data )
{
	CUresult cuRes = CUDA_SUCCESS;
	float time = 0;

	KERNEL_ID *ID = (KERNEL_ID*)pKernelParam->kernel;
	wstring KernelName = str2wstr( ID->KernelName );
	int ProgramID = ID->ProgramID;
	LOG_INFO(L"executing command of type: kernel, kernel name: \"%ws\" program number: %d", KernelName.c_str(), ProgramID);
	cCudaKernel* tKernel = m_kernels[ID->KernelName];

	cl_uint work_dim = pKernelParam->work_dim;
	const size_t *glb_wrk_offs = pKernelParam->glb_wrk_offs;
	const size_t *glb_wrk_size = pKernelParam->glb_wrk_size;
	const size_t *lcl_wrk_size = pKernelParam->lcl_wrk_size;
	cl_uint arg_count = tKernel->m_NumberOfArguments;
	const cl_kernel_argument*	args = tKernel->m_Arguments;
	const void* arg_values = pKernelParam->arg_values;
	size_t	offset = 0;
	char* ppp = NULL;

	CUevent start;
	cuRes = cuEventCreate(&start,0);
	CUevent finish;
	cuRes = cuEventCreate(&finish,0);

	cCudaMemObject* pMem;


	// Evgeny: Sagi what happens when arg_type == PTR, its size is 0
	// You also need to convert from cl_dev_mem to CUDA buffer ptr
	// Do it here or in clDevCommandListExecute(), i prefere there
	for (cl_uint i = 0; i < arg_count; i++)
	{
		if( args[i].type != tKernel->m_Arguments[i].type || args[i].size_in_bytes != tKernel->m_Arguments[i].size_in_bytes)
		{
			m_device->m_CallBacks.pclDevCmdStatusChanged( id, data, CL_COMPLETE, CL_DEV_ERROR_FAIL , 0);
			return CL_DEV_ERROR_FAIL;
		}

		//int temp = arg_types[i] & 0xF;
		if( CL_KRNL_ARG_PTR_GLOBAL == args[i].type || CL_KRNL_ARG_PTR_CONST == args[i].type)
		{
			cl_dev_mem pDevMem = *((cl_dev_mem*)((char*)arg_values + offset));
			pMem = (cCudaMemObject*)pDevMem->objHandle;
			if( pMem->GetRW() != tKernel->m_MemFlags[i] )
			{
				m_device->m_CallBacks.pclDevCmdStatusChanged( id, data, CL_COMPLETE, CL_DEV_ERROR_FAIL, 0 );
				return CL_DEV_ERROR_FAIL;
			}
			cuRes = cuParamSeti(tKernel->m_function, offset, pMem->GetPtr());
			offset += sizeof(int);
		}
		else
		{
			cuRes = cuParamSetv(tKernel->m_function, offset, (char*)arg_values + offset, args[i].size_in_bytes);
			offset += args[i].size_in_bytes;
		}

	}
	
	cuRes = cuParamSetSize(tKernel->m_function, offset);
	cuRes = cuFuncSetBlockShape(tKernel->m_function, lcl_wrk_size[0], lcl_wrk_size[1], 1);
	cuRes = cuEventRecord(start,0);
	cuRes = cuLaunchGrid(tKernel->m_function, glb_wrk_size[0], glb_wrk_size[1]);
	m_device->m_CallBacks.pclDevCmdStatusChanged( id, data, CL_RUNNING, 0, 0);
	cuRes = cuEventRecord(finish,0);
	cuRes = cuEventSynchronize(finish);
	m_device->m_CallBacks.pclDevCmdStatusChanged( id, data, CL_COMPLETE, CL_DEV_SUCCESS ,0);
	cuRes = cuEventElapsedTime(&time, start, finish );
	cuRes = cuEventDestroy(start);
	cuRes = cuEventDestroy(finish);
	return CL_DEV_SUCCESS;
}
//
//cCudaKernel
//
cCudaKernel::cCudaKernel():m_NumberOfArguments(0), m_function(NULL)
{
}
cCudaKernel::~cCudaKernel()
{
}

//
//cCudaCommandList
//
cCudaCommandList::cCudaCommandList(cl_int ID, cCudaDevice *pDevice):m_commands_execute(pDevice, this)
{
	m_device = pDevice;
	m_ID = ID;
}
cCudaCommandList::~cCudaCommandList()
{
}
void cCudaCommandList::StartThread()
{
	m_commands_execute.Start();
}
cl_dev_cmd_list cCudaCommandList::GetID()
{
	return (cl_dev_cmd_list)(&m_ID);
}
cl_int cCudaCommandList::PushKernel(cl_dev_cmd_desc* cmds)
{
	//need to check work dim
	cl_dev_cmd_param_kernel* tKernelParams = (cl_dev_cmd_param_kernel*)(cmds->params);

	if( ( 2 < tKernelParams->work_dim ) || ( 1 > tKernelParams->work_dim ) )
	{
		LOG_ERROR(L"recived unsuported work dim: %d", tKernelParams->work_dim);
		return CL_DEV_SUCCESS;
	}

	cl_dev_cmd_param_kernel* KernelParams = new cl_dev_cmd_param_kernel;

	KernelParams->kernel = tKernelParams->kernel;
	KernelParams->work_dim = tKernelParams->work_dim;

	memcpy(KernelParams->glb_wrk_offs, tKernelParams->glb_wrk_offs, KernelParams->work_dim * sizeof(size_t));
	memcpy(KernelParams->glb_wrk_size, tKernelParams->glb_wrk_size, KernelParams->work_dim * sizeof(size_t));
	memcpy(KernelParams->lcl_wrk_size, tKernelParams->lcl_wrk_size, KernelParams->work_dim * sizeof(size_t));

	if( 1 == KernelParams->work_dim )
	{
		KernelParams->glb_wrk_size[1] = 1;
		KernelParams->lcl_wrk_size[1] = 1;
	}

	KernelParams->arg_size = tKernelParams->arg_size;
	KernelParams->arg_values = new char[KernelParams->arg_size];
	memcpy((void*)KernelParams->arg_values, tKernelParams->arg_values, KernelParams->arg_size);


	cl_dev_cmd_desc *command = new cl_dev_cmd_desc;
		
	command->param_size = cmds->param_size;
	command->type = cmds->type;
	command->id = cmds->id;
	command->data = cmds->data;
	command->params = KernelParams;


	COMMAND_CONTAINER *Container = new COMMAND_CONTAINER;
	Container->CommandType = CUDA_RUN_KERNEL;
	Container->Command = command;


	m_QueueLock.Lock();
	m_commands.push(Container);
	m_device->m_CallBacks.pclDevCmdStatusChanged( command->id, command->data, CL_QUEUED, 0, 0);
	wstring KernelName = str2wstr( ( (KERNEL_ID*)KernelParams->kernel )->KernelName );
	int KernelID = ( (KERNEL_ID*)KernelParams->kernel )->ProgramID;
	LOG_INFO(L"pushing command of type: kernel, kernel name: \"%ws\" program number: %d", KernelName.c_str(), KernelID);
	m_QueueLock.Unlock();
	m_CommandEnqueued.Signal();
	LOG_INFO(L"sent signal");

	return CL_DEV_SUCCESS;
}
cl_int cCudaCommandList::PushRead(cl_dev_cmd_desc* cmds)
{
	//only support buffers
	cl_dev_cmd_param_rw* tReadParams = (cl_dev_cmd_param_rw*)(cmds->params);


	if( 1 != tReadParams->dim_count )
	{
		LOG_ERROR(L"recived unsuported dim_count: %d", tReadParams->dim_count);
		return CL_DEV_SUCCESS;
	}
	if( NULL == tReadParams->memObj->objHandle )
	{
		LOG_ERROR(L"recived NULL pointer instead of objHandle");
		return CL_DEV_SUCCESS;
	}
	

	cl_dev_cmd_param_rw* ReadParams = new cl_dev_cmd_param_rw;

	ReadParams->memObj = tReadParams->memObj;
	ReadParams->dim_count = tReadParams->dim_count;

	memcpy(ReadParams->origin, tReadParams->origin, tReadParams->dim_count * sizeof(size_t));
	memcpy(ReadParams->region, tReadParams->region, tReadParams->dim_count * sizeof(size_t));
	if ( tReadParams->dim_count > 1 )
	{
		memcpy(ReadParams->pitch, tReadParams->pitch, (tReadParams->dim_count-1) * sizeof(size_t));
	}

	ReadParams->ptr = tReadParams->ptr;

	cl_dev_cmd_desc *command = new cl_dev_cmd_desc;
		
	command->param_size = cmds->param_size;
	command->type = cmds->type;
	command->id = cmds->id;
	command->data = cmds->data;
	command->params = ReadParams;

	COMMAND_CONTAINER *Container = new COMMAND_CONTAINER;
	Container->CommandType = CUDA_MEM_READ;
	Container->Command = command;


	m_QueueLock.Lock();
	m_commands.push(Container);
	m_device->m_CallBacks.pclDevCmdStatusChanged( command->id, command->data, CL_QUEUED, 0, 0);
	LOG_INFO(L"pushing command of type: mem_read");
	m_QueueLock.Unlock();
	m_CommandEnqueued.Signal();
	LOG_INFO(L"sent signal");

	return CL_DEV_SUCCESS;
}
cl_int cCudaCommandList::PushWrite(cl_dev_cmd_desc* cmds)
{
	//only support buffers
	cl_dev_cmd_param_rw* tWriteParams = (cl_dev_cmd_param_rw*)(cmds->params);


	if( 1 != tWriteParams->dim_count )
	{
		LOG_ERROR(L"recived unsuported dim_count: %d", tWriteParams->dim_count);
		return CL_DEV_SUCCESS;
	}
	if( NULL == tWriteParams->memObj->objHandle )
	{
		LOG_ERROR(L"recived NULL pointer instead of objHandle");
		return CL_DEV_SUCCESS;
	}
	

	cl_dev_cmd_param_rw* WriteParams = new cl_dev_cmd_param_rw;

	WriteParams->memObj = tWriteParams->memObj;
	WriteParams->dim_count = tWriteParams->dim_count;

	memcpy(WriteParams->origin, tWriteParams->origin, tWriteParams->dim_count * sizeof(size_t));
	memcpy(WriteParams->region, tWriteParams->region, tWriteParams->dim_count * sizeof(size_t));
	if ( tWriteParams->dim_count > 1 )
	{
		memcpy(WriteParams->pitch, tWriteParams->pitch, (tWriteParams->dim_count-1) * sizeof(size_t));
	}

	WriteParams->ptr = tWriteParams->ptr;

	cl_dev_cmd_desc *command = new cl_dev_cmd_desc;
		
	command->param_size = cmds->param_size;
	command->type = cmds->type;
	command->id = cmds->id;
	command->data = cmds->data;
	command->params = WriteParams;

	COMMAND_CONTAINER *Container = new COMMAND_CONTAINER;
	Container->CommandType = CUDA_MEM_WRITE;
	Container->Command = command;


	m_QueueLock.Lock();
	m_commands.push(Container);
	m_device->m_CallBacks.pclDevCmdStatusChanged( command->id, command->data, CL_QUEUED, 0, 0);
	LOG_INFO(L"pushing command of type: mem_write");
	m_QueueLock.Unlock();
	m_CommandEnqueued.Signal();
	LOG_INFO(L"sent signal");

	return CL_DEV_SUCCESS;
}
cl_int cCudaCommandList::PushBuildProgram(cl_uint prog, const cl_char *options, void *user_data)
{
	CUDA_BUILD_PROGRAM_CONTAINER *BuildProgData = new CUDA_BUILD_PROGRAM_CONTAINER;

	BuildProgData->prog = prog;
	BuildProgData->user_data = user_data;

	if( NULL == options )
	{
		BuildProgData->options = NULL;
	}
	else
	{
		BuildProgData->options = new cl_char[strlen(options) + 1];
		strcpy_s(BuildProgData->options, strlen(options) + 1, options);
	}

	COMMAND_CONTAINER *Container = new COMMAND_CONTAINER;
	Container->CommandType = CUDA_BUILD_PROGRAM;
	Container->Command = BuildProgData;

	m_QueueLock.Lock();
	m_commands.push(Container);
	LOG_INFO(L"pushing command of type: build program, program number: %d", prog);
	m_QueueLock.Unlock();
	m_CommandEnqueued.Signal();
	LOG_INFO(L"sent signal");
	return CL_DEV_SUCCESS;
}
bool cCudaCommandList::IsEmpty()
{
	return m_commands.empty();
}
COMMAND_CONTAINER* cCudaCommandList::Pop()
{
	COMMAND_CONTAINER *Container;
	m_QueueLock.Lock();
	if ( true == m_commands.empty() )
	{
		LOG_INFO(L"queue is empty, waiting for signal");
		m_CommandEnqueued.Wait( & m_QueueLock );
		LOG_INFO(L"got signal");
	}
	else
	{
		LOG_INFO(L"queue is not empty");
	}
	Container = m_commands.front();
	m_commands.pop();
	m_QueueLock.Unlock();

	return Container;
}

//
//cCudaCommandExecuteThread
//
cCudaCommandExecuteThread::cCudaCommandExecuteThread(cCudaDevice *pDevice, cCudaCommandList *pCommandList)
{
	m_device = pDevice;
	m_CommandList = pCommandList;
}
cCudaCommandExecuteThread::~cCudaCommandExecuteThread()
{
}
int cCudaCommandExecuteThread::Run()
{
	LOG_INFO(L"cCudaCommandExecuteThread started");
	COMMAND_CONTAINER *Container;
	CUresult cuRes; 
	cuRes = cuInit(0);
	cuRes = cuDeviceGet( & m_cuDevice , 0);
	cuRes = cuCtxCreate( & m_cuContext, 0,m_cuDevice);
	while(1)
	{
		Container = m_CommandList->Pop();
		
		if( CUDA_BUILD_PROGRAM == Container->CommandType )
		{
			BuildProgram( *(CUDA_BUILD_PROGRAM_CONTAINER*)(Container->Command) );
		}

		if( CUDA_RUN_KERNEL == Container->CommandType )
		{
			RunKernel( *(cl_dev_cmd_desc*)(Container->Command) );
		}

		if( CUDA_MEM_WRITE == Container->CommandType )
		{
			RunWrite( *(cl_dev_cmd_desc*)(Container->Command) );
		}

		if( CUDA_MEM_READ == Container->CommandType )
		{
			RunRead( *(cl_dev_cmd_desc*)(Container->Command) );
		}

		delete Container;
	}
	return 1;
}
cl_int cCudaCommandExecuteThread::RunKernel(cl_dev_cmd_desc cmd)
{
	cl_dev_cmd_param_kernel* pKernelParam = (cl_dev_cmd_param_kernel*)cmd.params;
	cl_uint uiProg = ( (KERNEL_ID*)pKernelParam->kernel )->ProgramID;
	cCudaProgram* tProg = m_device->m_Programs[uiProg];
	return tProg->RunKernel( pKernelParam, cmd.id, cmd.data );
}

cl_int cCudaCommandExecuteThread::RunRead(cl_dev_cmd_desc cmd)
{
	cl_dev_cmd_param_rw* pReadParam = (cl_dev_cmd_param_rw*)cmd.params;
	cCudaMemObject* pMemObj = (cCudaMemObject*)pReadParam->memObj->objHandle;
	int res =  pMemObj->Read(pReadParam->dim_count,
						  pReadParam->origin,
						  pReadParam->region,
						  pReadParam->ptr,
						  pReadParam->pitch);
	m_device->m_CallBacks.pclDevCmdStatusChanged( cmd.id, cmd.data, CL_COMPLETE, CL_DEV_SUCCESS, 0 );
	return res;
}
cl_int cCudaCommandExecuteThread::RunWrite(cl_dev_cmd_desc cmd)
{
	cl_dev_cmd_param_rw* pWriteParam = (cl_dev_cmd_param_rw*)cmd.params;
	cCudaMemObject* pMemObj = (cCudaMemObject*)pWriteParam->memObj->objHandle;
	int res = pMemObj->Write(pWriteParam->dim_count,
						  pWriteParam->origin,
						  pWriteParam->region,
						  pWriteParam->ptr,
						  pWriteParam->pitch);
	m_device->m_CallBacks.pclDevCmdStatusChanged( cmd.id, cmd.data, CL_COMPLETE, CL_DEV_SUCCESS , 0);
	return res;
}
cl_int cCudaCommandExecuteThread::BuildProgram( CUDA_BUILD_PROGRAM_CONTAINER BuildData )
{
	LOG_INFO(L"executing command of type: build program, program number: %d", BuildData.prog);

	cCudaProgram *tProg = m_device->m_Programs[ BuildData.prog ];

	return tProg->Build( BuildData.options, BuildData.user_data );
}
//
//COMMAND_CONTAINER functions
//
_COMMAND_CONTAINER::~_COMMAND_CONTAINER()
{
	if( CUDA_BUILD_PROGRAM == CommandType )
	{
		CUDA_BUILD_PROGRAM_CONTAINER *BuildProgData = (CUDA_BUILD_PROGRAM_CONTAINER*)Command;
		delete[] BuildProgData->options;
		delete BuildProgData;
	}
	else if( CUDA_RUN_KERNEL == CommandType )
	{
		cl_dev_cmd_param_kernel* KernelParams = (cl_dev_cmd_param_kernel*)((cl_dev_cmd_desc*)Command)->params;
		delete[] KernelParams->arg_values;
		delete KernelParams;
	}
}
//
//Global functions
//
const wstring str2wstr(string str)
{
	size_t needed;
	::mbstowcs_s(&needed, NULL, 0, &str[0], str.length());
	std::wstring wstr;
	wstr.resize(needed);
	::mbstowcs_s(&needed, &wstr[0], wstr.length(), &str[0], str.length());
	return wstr;
}