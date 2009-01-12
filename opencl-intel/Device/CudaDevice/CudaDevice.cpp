// CudaDevice.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "CudaDevice.h"
#include "cl_logger.h"
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
void RunCommand(cl_dev_cmd_desc* cmd);
cl_dev_log_descriptor log;
cl_int LogClientID;

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
	if (devCallbacks != NULL)
	{
		memcpy(&m_CallBacks, devCallbacks, sizeof(m_CallBacks));
	}
	if (logDesc != NULL)
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
	// handle error

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
	// TODO : ADD log
	CUresult CuRes = CUDA_SUCCESS;

	if ( NULL == m_pDevInstance )
	{
		m_pDevInstance = new cCudaDevice(devId, devCallbacks, logDesc);

		cCudaCommandList *CommandList = new cCudaCommandList;
		if( NULL == CommandList )
		{
			delete m_pDevInstance;
			return NULL;
		}

		m_pDevInstance->m_CommandLists.push_back(CommandList);
		CommandList->m_ID = m_pDevInstance->m_CommandLists.size() - 1;
		CommandList->m_device = m_pDevInstance;

		//CuRes = cuInit(0);
		//if ( CuRes != CUDA_SUCCESS )
		//{
		//	delete m_pDevInstance;
		//	return NULL;
		//}

		//CuRes = cuDeviceGet( &( m_pDevInstance->m_device ) , 0);
		//if ( CuRes != CUDA_SUCCESS )
		//{
		//	delete m_pDevInstance;
		//	return NULL;
		//}

		//CuRes = cuCtxCreate( &( m_pDevInstance->m_context ), 0, m_pDevInstance->m_device);
		//if ( CuRes != CUDA_SUCCESS )
		//{
		//	delete m_pDevInstance;
		//	return NULL;
		//}

		

		//CUcontext context;
		//CUresult cuRes = cuCtxAttach( &(CommandList->cuContext), 0); 
//		cuRes = cuCtxAttach( &(m_pDevInstance->m_context), 0); 
		//cuRes = cuCtxPopCurrent( &(CommandList->cuContext) );
		//cuRes = cuCtxPushCurrent( CommandList->cuContext );
		//CUresult cuRes = cuCtxPopCurrent( &(m_pDevInstance->m_context) );


		CommandList->commands_execute.Start();
	}
	
	LOG_DEBUG(L"Device initiated");
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
	// TODO : ADD log
	return CL_DEV_INVALID_OPERATION;
}

cl_int cCudaDevice::clDevCreateCommandList( cl_dev_cmd_list_props IN props, cl_dev_cmd_list* OUT list)
{
	// TODO : ADD log
	if( CL_DEV_LIST_NONE != props)
	{
		return CL_DEV_INVALID_PROPERTIES;
	}

	(*list) = (cl_dev_cmd_list)(&(m_pDevInstance->m_CommandLists[ m_pDevInstance->m_CommandLists.size() - 1 ]->m_ID));
	return CL_DEV_SUCCESS;
}

cl_int cCudaDevice::clDevRetainCommandList( cl_dev_cmd_list IN list)
{
	// TODO : ADD log
	return CL_DEV_INVALID_OPERATION;
}

cl_int cCudaDevice::clDevReleaseCommandList( cl_dev_cmd_list IN list )
{
	// TODO : ADD log
	return CL_DEV_INVALID_OPERATION;
}

cl_int cCudaDevice::clDevCommandListExecute( cl_dev_cmd_list IN list, cl_dev_cmd_desc* IN cmds, cl_uint IN count)
{
	// TODO : ADD log
	if( 0 != *(unsigned int*)list )
	{
		return CL_DEV_INVALID_COMMAND_LIST;
	}
	cCudaCommandList *tList = m_pDevInstance->m_CommandLists[*(unsigned int*)list];
	for( unsigned int i = 0; i < count; i++ )
	{
		m_pDevInstance->m_CallBacks.pclDevCmdStatusChanged( cmds[i].id, CL_SUBMITTED);

		cl_dev_cmd_desc *command = new cl_dev_cmd_desc;
		
		command->param_size = cmds[i].param_size;
		command->type = cmds[i].type;
		command->id = cmds[i].id;
		
		cl_dev_cmd_param_kernel* KernelParams = new cl_dev_cmd_param_kernel;

		KernelParams->kernel = ((cl_dev_cmd_param_kernel*)(cmds->params))->kernel;
		KernelParams->work_dim = ((cl_dev_cmd_param_kernel*)(cmds->params))->work_dim;
		memcpy(KernelParams->glb_wrk_offs, ((cl_dev_cmd_param_kernel*)(cmds->params))->glb_wrk_offs, 3 * sizeof(size_t));
		memcpy(KernelParams->glb_wrk_size, ((cl_dev_cmd_param_kernel*)(cmds->params))->glb_wrk_size, 3 * sizeof(size_t));
		memcpy(KernelParams->lcl_wrk_size, ((cl_dev_cmd_param_kernel*)(cmds->params))->lcl_wrk_size, 3 * sizeof(size_t));
		KernelParams->arg_count = ((cl_dev_cmd_param_kernel*)(cmds->params))->arg_count;
		KernelParams->arg_types = new cl_kernel_arg_type[KernelParams->arg_count];
		KernelParams->arg_values = new void*[KernelParams->arg_count];
		for (unsigned int i = 0; i < KernelParams->arg_count; i++)
		{
			KernelParams->arg_types[i] = ((cl_dev_cmd_param_kernel*)(cmds->params))->arg_types[i];
			KernelParams->arg_values[i] = new char[KernelParams->arg_types[i]];
			memcpy(KernelParams->arg_values[i], ((cl_dev_cmd_param_kernel*)(cmds->params))->arg_values[i], KernelParams->arg_types[i]);
		}

		command->params = KernelParams;


		COMMAND_CONTAINER *Container = new COMMAND_CONTAINER;
		Container->CommandType = CUDA_RUN_KERNEL;
		Container->Command = command;


		tList->m_QueueLock.Lock();
		tList->m_commands.push(Container);
		m_pDevInstance->m_CallBacks.pclDevCmdStatusChanged( command->id, CL_QUEUED);
		LOG_DEBUG(L"pushing command: kernel name: \"%s\" program number: %d", ((KERNEL_ID*)(command->id))->KernelName.c_str(), ((KERNEL_ID*)(command->id))->ProgramID);
		tList->m_QueueLock.Unlock();
		tList->m_CommandEnqueued.Signal();
		LOG_DEBUG(L"sent signal");

	}
	
	return CL_DEV_SUCCESS;
}

cl_int cCudaDevice::clDevGetSupportedImageFormats( cl_dev_mem_flags IN flags, cl_dev_mem_object_type IN image_type,
				cl_uint IN num_entries, cl_image_format* OUT formats, cl_uint* OUT num_entries_ret)
{
	// TODO : ADD log
	return CL_DEV_INVALID_OPERATION;
}

cl_int cCudaDevice::clDevCreateMemoryObject( cl_dev_mem_flags IN flags, const cl_image_format* IN format,
						size_t IN width, size_t IN height, size_t IN depth, cl_dev_mem* OUT memObj)
{
	// TODO : ADD log
	return CL_DEV_INVALID_OPERATION;
}

cl_int cCudaDevice::clDevDeleteMemoryObject( cl_dev_mem IN memObj )
{
	// TODO : ADD log
	return CL_DEV_INVALID_OPERATION;
}

cl_int cCudaDevice::clDevCreateMappedRegion( cl_dev_mem IN memObj, const size_t IN origin[3], const size_t IN region[3],
						 void** OUT ptr, size_t* OUT row_pitch, size_t* OUT slice_pitch)
{
	// TODO : ADD log
	return CL_DEV_INVALID_OPERATION;
}

cl_int cCudaDevice::clDevReleaseMappedRegion( cl_dev_mem IN memObj, void* IN ptr)
{
	// TODO : ADD log
	return CL_DEV_INVALID_OPERATION;
}

cl_int cCudaDevice::clDevCheckProgramBinary( size_t IN bin_size, const void* IN bin )
{
	// TODO : ADD log
	return CL_DEV_INVALID_OPERATION;
}


cl_int cCudaDevice::clDevCreateProgram( size_t IN bin_size, const void* IN bin, cl_dev_binary_prop IN prop, cl_dev_program* OUT prog )
{
	cl_prog_container *ProgContainer = (cl_prog_container*)bin;
	//memcpy(&ProgContainer, bin, bin_size);

	if (prop != CL_DEV_BINARY_USER)
	{
		return CL_DEV_INVALID_BINARY;
	}

	if (ProgContainer->container_type != CL_PROG_CNT_PRIVATE)
	{
		return CL_DEV_INVALID_BINARY;
	}

	if (ProgContainer->description.bin_type != CL_PROG_BIN_CUBIN)
	{
		return CL_DEV_INVALID_BINARY;
	}

	if (ProgContainer->container_size != ( strlen( (char*)(ProgContainer->container) ) + 1 + sizeof(cl_prog_container)) )
	{
		return CL_DEV_INVALID_BINARY;
	}

	cCudaProgram *tProg = new cCudaProgram();

	tProg->m_ProgramID = m_pDevInstance->m_Programs.size();
	tProg->m_ProgContainer.container_size = ProgContainer->container_size;
	tProg->m_ProgContainer.container_type = ProgContainer->container_type;
	tProg->m_ProgContainer.description = ProgContainer->description;
	strcpy_s( (char*)tProg->m_ProgContainer.container, N_MAX_STRING_SIZE, (char*)ProgContainer->container );
	tProg->m_ProgramName.clear();
	tProg->m_ProgramName.append((char*)ProgContainer->container);


	m_pDevInstance->m_Programs.push_back(tProg);
	*prog = (cl_dev_program*)(&(tProg->m_ProgramID));

	return CL_DEV_SUCCESS;
}

cl_int cCudaDevice::clDevBuildProgram(cl_dev_program prog, const cl_char *options, void *user_data)
{
	CUresult CuRes = CUDA_SUCCESS;
	cl_int ClRes = CL_DEV_SUCCESS;
	KERNELS::iterator it;

	CUDA_BUILD_PROGRAM_CONTAINER *BuildProgData = new CUDA_BUILD_PROGRAM_CONTAINER;
	BuildProgData->prog = *(cl_uint*)prog;

	if( NULL == options )
	{
		BuildProgData->options = NULL;
	}
	else
	{
		BuildProgData->options = new cl_char[strlen(options) + 1];
		strcpy_s(BuildProgData->options, strlen(options) + 1, options);
	}
	BuildProgData->user_data = user_data;

	COMMAND_CONTAINER *Container = new COMMAND_CONTAINER;
	Container->CommandType = CUDA_BUILD_PROGRAM;
	Container->Command = BuildProgData;

	m_pDevInstance->m_CommandLists[0]->m_QueueLock.Lock();
	m_pDevInstance->m_CommandLists[0]->m_commands.push(Container);
	m_pDevInstance->m_CommandLists[0]->m_QueueLock.Unlock();
	m_pDevInstance->m_CommandLists[0]->m_CommandEnqueued.Signal();

	return CL_DEV_SUCCESS;
}

cl_int cCudaDevice::clDevReleaseProgram( cl_dev_program IN prog )
{
	// TODO : ADD log
	return CL_DEV_INVALID_OPERATION;
}

cl_int cCudaDevice::clDevUnloadCompiler()
{
	// TODO : ADD log
	return CL_DEV_INVALID_OPERATION;
}

cl_int cCudaDevice::clDevGetProgramBinary(cl_dev_program prog, size_t size, void *binary, size_t *size_ret)
{
	//// TODO : ADD log
	unsigned int uiProg = *(unsigned int*)prog;
	if( (uiProg < 0) || ( uiProg >= m_pDevInstance->m_Programs.size()) )
	{
		return CL_DEV_INVALID_VALUE;
	}

	cCudaProgram *tProg = m_pDevInstance->m_Programs[uiProg];
	char* stCubinName = (char*)(m_pDevInstance->m_Programs[uiProg]->m_ProgContainer.container);

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
	tContainer->container_size = tProg->m_ProgContainer.container_size;
	tContainer->container_type = tProg->m_ProgContainer.container_type;
	tContainer->description = tProg->m_ProgContainer.description;
	memcpy((void*)tContainer->container, tProg->m_ProgContainer.container, strlen((char*)tProg->m_ProgContainer.container) + 1);
	*(size_ret) = sizeof(cl_prog_container) + strlen(stCubinName) +1;
	return CL_DEV_SUCCESS;
}

cl_int cCudaDevice::clDevGetBuildLog( cl_dev_program IN prog, size_t IN size, char* OUT log, size_t* OUT size_ret)
{
	// TODO : ADD log
	return CL_DEV_INVALID_OPERATION;
}

cl_int cCudaDevice::clDevGetSupportedBinaries( cl_uint IN count, cl_prog_binary_desc* OUT types, size_t* OUT size_ret )
{
	// TODO : ADD log
	return CL_DEV_INVALID_OPERATION;
}

cl_int cCudaDevice::clDevGetKernelId( cl_dev_program IN prog, const char* IN name, cl_dev_kernel* OUT kernel_id )
{
	//// TODO : ADD log

	unsigned int uiProg = *(unsigned int*)prog;
	if ( name == NULL )
	{
		return CL_DEV_INVALID_VALUE;
	}
	if ( uiProg >= m_pDevInstance->m_Programs.size() )
	{
		return CL_DEV_INVALID_PROGRAM;
	}
	if ( NULL == m_pDevInstance->m_Programs[uiProg] )
	{
		return CL_DEV_INVALID_PROGRAM;
	}
	cCudaProgram *tprog = m_pDevInstance->m_Programs[uiProg];
	if ( tprog->m_kernels.find(name) == tprog->m_kernels.end() )
	{
		return CL_INVALID_KERNEL_NAME;
	}
	cCudaKernel *tKernel = tprog->m_kernels[name];
	*kernel_id = (cl_dev_kernel)(& (tprog->m_kernels[name]->m_KernelID) );
	return CL_SUCCESS;
}

cl_int cCudaDevice::clDevGetProgramKernels( cl_dev_program IN prog, cl_uint IN num_kernels, cl_dev_kernel* OUT kernels,
						 cl_uint* OUT num_kernels_ret )
{
	// TODO : ADD log
	return CL_DEV_INVALID_OPERATION;
}

cl_int cCudaDevice::clDevGetKernelInfo( cl_dev_kernel IN kernel, cl_dev_kernel_info IN param, size_t IN value_size,
					void* OUT value, size_t* OUT value_size_ret )
{
	// TODO : ADD log
	return CL_DEV_INVALID_OPERATION;
}



cl_int cCudaDevice::m_ParseCubin( const char* stCubinName , cCudaProgram OUT *Prog)
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
			cCudaKernel *temp_kernel = new cCudaKernel;
			temp_kernel->m_OriginalName = st;
			temp_kernel->m_KernelID.ProgramID = Prog->m_ProgramID;
			//strcpy_s(temp_kernel->m_OriginalName, N_MAX_STRING_SIZE, st);
			int index = 13;
			int iTemp = atoi(&st[index]);
			while( isdigit(st[index]) != 0)
			{
				index++;
			}
			temp_kernel->m_KernelID.KernelName.assign(&st[index], iTemp);
			//strncpy_s(temp_kernel->m_Name, N_MAX_STRING_SIZE, &st[index], iTemp);
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
						delete temp_kernel;
						return CL_DEV_ERROR_FAIL;
					}
					break;
				default : fCubin.close();
					delete temp_kernel;
					return CL_DEV_ERROR_FAIL;
				}
				temp_kernel->m_Arguments[ temp_kernel->m_NumberOfArguments ] = temp_parameter;
				temp_kernel->m_NumberOfArguments++;
				index++;
			}
			Prog->m_kernels[temp_kernel->m_KernelID.KernelName] = temp_kernel;
		}
		fCubin >> st;
	}
	fCubin.close();
	return CL_DEV_SUCCESS;
}

cCudaProgram::cCudaProgram()
{
	m_module = NULL;
	m_ProgramID = -1;
	m_ProgContainer.container = new char[N_MAX_STRING_SIZE];
	m_ProgContainer.container_size = 0;
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


cCudaKernel::cCudaKernel():m_NumberOfArguments(0), m_function(NULL)
{
}
cCudaKernel::~cCudaKernel()
{
}

cCudaCommandList::cCudaCommandList()
{
	commands_execute.CommandList = this;
}
cCudaCommandList::~cCudaCommandList()
{
}
int cCudaCommandExecuteThread::Run()
{
	LOG_DEBUG(L"cCudaCommandExecuteThread started");
	COMMAND_CONTAINER *Container;
	CUresult cuRes; 
	cuRes = cuInit(0);
	cuRes = cuDeviceGet( &(CommandList->m_cuDevice) , 0);
	cuRes = cuCtxCreate( &(CommandList->m_cuContext), 0, CommandList->m_cuDevice);
	while(1)
	{
		CommandList->m_QueueLock.Lock();
		if ( true == CommandList->m_commands.empty() )
		{
			LOG_DEBUG(L"queue is empty, waiting for signal");
			CommandList->m_CommandEnqueued.Wait( &(CommandList->m_QueueLock) );
			LOG_DEBUG(L"got signal");
		}
		else
		{
			LOG_DEBUG(L"queue is not empty");
		}
		Container = CommandList->m_commands.front();
		CommandList->m_commands.pop();
		CommandList->m_QueueLock.Unlock();
		
		if( CUDA_BUILD_PROGRAM == Container->CommandType )
		{
			BuildProgram( *(CUDA_BUILD_PROGRAM_CONTAINER*)(Container->Command) );
		}

		if( CUDA_RUN_KERNEL == Container->CommandType )
		{
			RunCommand( *(cl_dev_cmd_desc*)(Container->Command) );
		}

		delete Container->Command;
		delete Container;
	}
	return 1;
}
cl_int cCudaCommandExecuteThread::RunCommand(cl_dev_cmd_desc cmd)
{
	cl_dev_cmd_param_kernel* pKernelParam = (cl_dev_cmd_param_kernel*)cmd.params;
	KERNEL_ID *ID = (KERNEL_ID*)pKernelParam->kernel;
	cCudaKernel* pKernel = CommandList->m_device->m_Programs[ID->ProgramID]->m_kernels[ID->KernelName];
	cCudaProgram* pProgram = CommandList->m_device->m_Programs[ID->ProgramID];
	cl_uint work_dim = pKernelParam->work_dim;
	size_t *glb_wrk_offs = pKernelParam->glb_wrk_offs;
	size_t *glb_wrk_size = pKernelParam->glb_wrk_size;
	size_t *lcl_wrk_size = pKernelParam->lcl_wrk_size;
	cl_uint arg_count = pKernelParam->arg_count;
	cl_kernel_arg_type*	arg_types = pKernelParam->arg_types;
	void** arg_values = pKernelParam->arg_values;
	int	offset = 0;
	CUresult cuRes = CUDA_SUCCESS;
	CUevent start;
	cuRes = cuEventCreate(&start,0);
	CUevent finish;
	cuRes = cuEventCreate(&finish,0);
	float time = 0;

	for (cl_uint i = 0; i < arg_count; i++)
	{
		cuRes = cuParamSetv(pKernel->m_function, offset, arg_values[i], arg_types[i]);
		offset += arg_types[i];
	}
	cuRes = cuParamSetSize(pKernel->m_function, offset);
	cuRes = cuFuncSetBlockShape(pKernel->m_function, lcl_wrk_size[0], lcl_wrk_size[1], 1);
	cuRes = cuEventRecord(start,0);
	cuRes = cuLaunchGrid(pKernel->m_function, glb_wrk_size[0], glb_wrk_size[1]);
	CommandList->m_device->m_CallBacks.pclDevCmdStatusChanged( cmd.id, CL_RUNNING );
	cuRes = cuEventRecord(finish,0);
	cuRes = cuEventSynchronize(finish);
	CommandList->m_device->m_CallBacks.pclDevCmdStatusChanged( cmd.id, CL_COMPLETE );
	cuRes = cuEventElapsedTime(&time, start, finish );
	cuRes = cuEventDestroy(start);
	cuRes = cuEventDestroy(finish);

	return CL_DEV_SUCCESS;
}

cl_int cCudaCommandExecuteThread::BuildProgram( CUDA_BUILD_PROGRAM_CONTAINER BuildData )
{
	KERNELS::iterator it;

	cCudaProgram *tProg = CommandList->m_device->m_Programs[ BuildData.prog ];

	CUresult CuRes = cuModuleLoad( &( tProg->m_module ), tProg->m_ProgramName.c_str() );
	if (CuRes != CUDA_SUCCESS)
	{
		return CL_DEV_INVALID_BINARY;
	}

	cl_int ClRes = CommandList->m_device->m_ParseCubin(tProg->m_ProgramName.c_str(), tProg);
	if (ClRes != CL_DEV_SUCCESS)
	{
		cuModuleUnload(tProg->m_module);
		return CL_DEV_INVALID_BINARY;
	}

	for ( it = tProg->m_kernels.begin(); it != tProg->m_kernels.end(); it++)
	{
		CuRes = cuModuleGetFunction( &(it->second->m_function), tProg->m_module, it->second->m_OriginalName.c_str());
	}

	CommandList->m_device->m_CallBacks.pclDevBuildStatusUpdate( (cl_dev_program)BuildData.prog , BuildData.user_data, CL_BUILD_SUCCESS);

	return CL_DEV_SUCCESS;
}