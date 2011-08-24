#include "execution_commands.h"
#include "cl_dev_backend_api.h"
#include "command_list.h"
#include "memory_allocator.h"

#include <source/COIBuffer_source.h>

using namespace Intel::OpenCL::MICDevice;
using namespace Intel::OpenCL::DeviceBackend;

NDRange::NDRange(CommandList* pCommandList, IOCLFrameworkCallbacks* pFrameworkCallBacks, cl_dev_cmd_desc* pCmd) : Command(pCommandList, pFrameworkCallBacks, pCmd),
m_dispatcherDataBuffer(NULL), m_printfBuffer(NULL), m_profilingBuffer(NULL)
{
}

NDRange::~NDRange()
{
	releaseResources();
}

cl_dev_err_code NDRange::Create(CommandList* pCommandList, IOCLFrameworkCallbacks* pFrameworkCallBacks, cl_dev_cmd_desc* pCmd, Command** pOutCommand)
{
	return verifyCreation(new NDRange(pCommandList, pFrameworkCallBacks, pCmd), pOutCommand);
}

void NDRange::getKernelArgBuffersCount(const unsigned int numArgs, const cl_kernel_argument* pArgs, vector<kernel_arg_buffer_info>& oBuffsInfo)
{
	assert(pArgs);
	size_t stOffset = 0;
	// Lock required memory objects
	for(unsigned int i = 0; i < numArgs; ++i)
	{
		// Argument is buffer object or local memory size
		if (( CL_KRNL_ARG_PTR_GLOBAL == pArgs[i].type ) ||
			( CL_KRNL_ARG_PTR_CONST == pArgs[i].type ) ||
			( CL_KRNL_ARG_PTR_IMG_2D == pArgs[i].type ) ||
			( CL_KRNL_ARG_PTR_IMG_3D == pArgs[i].type )
			)

		{
			oBuffsInfo.push_back( kernel_arg_buffer_info(stOffset, i) );
			// TODO support of 32 / 64 bit on host (device always 64 bit) --> stOffset += sizeof(uint64_t);
			stOffset += sizeof(IOCLDevMemoryObject*);
		}
		else if (CL_KRNL_ARG_PTR_LOCAL == pArgs[i].type)
		{
			// TODO support of 32 / 64 bit on host (device always 64 bit) --> stOffset += sizeof(uint64_t);
			stOffset += sizeof(void*);
		}
		else if (CL_KRNL_ARG_VECTOR == pArgs[i].type)
		{
			unsigned int uiSize = pArgs[i].size_in_bytes;
			uiSize = (uiSize & 0xFFFF) * (uiSize >> 16);
			stOffset += uiSize;
		}
		else if (CL_KRNL_ARG_SAMPLER == pArgs[i].type)
		{
			stOffset += sizeof(cl_int);
		}
		else
		{
			stOffset += pArgs[i].size_in_bytes;
		}
	}
}

cl_dev_err_code NDRange::init(COIBUFFER** ppOutCoiBuffsArr, COI_ACCESS_FLAGS** ppAccessFlagArr, unsigned int* pOutNumBuffers)
{
	// Get command params
	cl_dev_cmd_param_kernel *cmdParams = (cl_dev_cmd_param_kernel*)m_pCmd->params;
	// Get Kernel from params
    const ICLDevBackendKernel_* pKernel = (ICLDevBackendKernel_*)cmdParams->kernel;
	// Get kernel params
	const char*	pKernelParams = (const char*)cmdParams->arg_values;

	// Get the amount of kernel args
	unsigned int uiNumArgs = pKernel->GetKernelParamsCount();
	// Get kernel arguments
    const cl_kernel_argument* pArgs = pKernel->GetKernelParams();

	// Define the directives and function arguments to dispatch to the device side. (Will be store at the first COIBUFFER)
	dispatcher_data dispatcherData;
	// Get device side kernel address and set kernel directive (Also increase the reference counter of the Program.
	dispatcherData.kernelDirective.kernelAddress = m_pCommandList->getProgramService()->AcquireKernelOnDevice(cmdParams->kernel);
	if (0 == dispatcherData.kernelDirective.kernelAddress)
	{
		m_lastError = CL_DEV_INVALID_VALUE;
		return CL_DEV_INVALID_VALUE;
	}
	// set kernel args blob size in bytes for dispatcherData
	dispatcherData.kernelArgSize = cmdParams->arg_size;
	// set isInOrderQueue flag in dispatcherData
	dispatcherData.isInOrderQueue = m_pCommandSynchHandler->isInOrderType();
	// Filling the workDesc structure in dispatcherData
	dispatcherData.workDesc.setParams(cmdParams->work_dim, cmdParams->glb_wrk_offs, cmdParams->glb_wrk_size, cmdParams->lcl_wrk_size);

	// The amount of COIBUFFERS to dispatch (At least 1 for the 'dispatcher_data' structure)
	unsigned int numOfDispatchingBuffers = 1;

	// directives counter.
	unsigned int numPreDirectives = 0;
	unsigned int numPostDirectives = 0;

	// Vector which save the kernel buffer arguments basic info in order to traverse on uiNumArgs only once
	vector<NDRange::kernel_arg_buffer_info> buffsInfoVector;

	// Get the amount of buffers in kernel args and info of their offset in blob and their index in pArgs
	getKernelArgBuffersCount(uiNumArgs, pArgs, buffsInfoVector);
	// The amount of buffers in kernel args
	unsigned int numOfBuffersInKernelArgs = buffsInfoVector.size();

	numOfDispatchingBuffers += numOfBuffersInKernelArgs;
	numPreDirectives += numOfBuffersInKernelArgs;

	// Get device side process in order to create COIBUFFERs for this process.
	COIPROCESS tProcess = m_pCommandList->getDeviceProcess();

	COIRESULT coi_err = COI_SUCCESS;
	// If there is printf operation, add printf directive
	if (pKernel->GetKernelProporties()->HasPrintOperation())
	{
		// Create coi buffer for printf
    	coi_err = COIBufferCreate(
                                MIC_PRINTF_BUFFER_SIZE,                          // The number of bytes to allocate for the buffer.
								COI_BUFFER_NORMAL,                               // The type of the buffer to create
								0,                                               // A bitmask of attributes for the newly created buffer.
								NULL,                                            // If non-NULL the buffer will be initialized with the data pointed
                                1, &tProcess,                                    // The number of processes with which this buffer might be used, and The process
                                &m_printfBuffer);                                // Pointer to a buffer handle
		// Is the COIBufferCreate succeeded?
		if (COI_SUCCESS != coi_err)
		{
			m_lastError = CL_DEV_OBJECT_ALLOC_FAIL;
		    return CL_DEV_OBJECT_ALLOC_FAIL;
		}
		numOfDispatchingBuffers ++;
		numPreDirectives ++;
		numPostDirectives ++;
	}

	if(m_pCmd->profiling)
	{
		// Create coi buffer for profiling
    	coi_err = COIBufferCreate(
                                sizeof(profiling_data),                          // The number of bytes to allocate for the buffer.
								COI_BUFFER_NORMAL,                               // The type of the buffer to create
								0,                                               // A bitmask of attributes for the newly created buffer.
								NULL,                                            // If non-NULL the buffer will be initialized with the data pointed
                                1, &tProcess,                                    // The number of processes with which this buffer might be used, and The process
                                &m_profilingBuffer);                             // Pointer to a buffer handle
		// Is the COIBufferCreate succeeded?
		if (COI_SUCCESS != coi_err)
		{
			m_lastError = CL_DEV_OBJECT_ALLOC_FAIL;
		    return CL_DEV_OBJECT_ALLOC_FAIL;
		}

		numOfDispatchingBuffers ++;
		numPreDirectives ++;
		numPostDirectives ++;
	}

	// If the CommandList is OOO
	if (false == dispatcherData.isInOrderQueue)
	{
		// We should add additional directive for post execution (BARRIER directive)
		numPostDirectives ++;
	}

	// The the amount of pre and post exe directives in 'dispatcherData'
	dispatcherData.preExeDirectivesCount = numPreDirectives;
    dispatcherData.postExeDirectivesCount = numPostDirectives;
	// calculate and set the offset parameters in 'dispatcherData'
	dispatcherData.calcAndSetOffsets();

	// Array of COIBUFFERs that will sent to the process (The first location is for the dispatcher data structure)
	// IT IS CALLER RESPONSIBILITY TO FREE THIS ALLOCATION (IT WILL SET THE OUT PARAMETER)
	COIBUFFER* coiBuffsArr = (COIBUFFER*)malloc(sizeof(COIBUFFER) * numOfDispatchingBuffers);
	assert(coiBuffsArr);
	// Array of access flags that will sent to the process (The first location is for the dispatcher data structure permission)
	// IT IS CALLER RESPONSIBILITY TO FREE THIS ALLOCATION (IT WILL SET THE OUT PARAMETER)
	COI_ACCESS_FLAGS* accessFlagArr = (COI_ACCESS_FLAGS*)malloc(sizeof(COI_ACCESS_FLAGS) * numOfDispatchingBuffers);
	assert(accessFlagArr);

	// array of directive_pack for preExeDirectives
	directive_pack* preExeDirectives = NULL;
	if (numPreDirectives > 0)
	{
		preExeDirectives = (directive_pack*)malloc(sizeof(directive_pack) * numPreDirectives);
		assert(preExeDirectives && "Allocation failed");
	}
	// array of directive_pack for preExeDirectives
	directive_pack* postExeDirectives = NULL;
	if (numPostDirectives > 0)
	{
		postExeDirectives = (directive_pack*)malloc(sizeof(directive_pack) * numPostDirectives);
		assert(postExeDirectives && "Allocation failed");
	}

	unsigned int currCoiBuffIndex = 1;
	unsigned int currPreDirectiveIndex = 0;
	unsigned int currPostDirectiveIndex = 0;

	// Set the buffer arguments of the kernel as directives and COIBUFFERS
	assert(numPreDirectives >= numOfBuffersInKernelArgs);
	for (unsigned int i = 0; i < numOfBuffersInKernelArgs; i++)
	{
		size_t stOffset = buffsInfoVector[i].offsetInBlob;
		MICDevMemoryObject *memObj = (MICDevMemoryObject*)*((IOCLDevMemoryObject**)(pKernelParams+stOffset));
		// Set the COIBUFFER of coiBuffsArr[currCoiBuffIndex] to be the COIBUFFER that hold the data of the buffer.
		coiBuffsArr[currCoiBuffIndex] = memObj->clDevMemObjGetCoiBufferHandler();
		// TODO - Change the flag according to the information about the argument (Not implemented yet by the BE)
		if ( CL_KRNL_ARG_PTR_CONST == pArgs[buffsInfoVector[i].index].type )
		{
			accessFlagArr[currCoiBuffIndex] = COI_SINK_READ;
		}
		else
		{
			accessFlagArr[currCoiBuffIndex] = COI_SINK_WRITE;
		}
		// Set this directive settings
		preExeDirectives[currPreDirectiveIndex].id = BUFFER;
		preExeDirectives[currPreDirectiveIndex].bufferDirective.bufferIndex = currCoiBuffIndex;
		preExeDirectives[currPreDirectiveIndex].bufferDirective.offset_in_blob = stOffset;

		currCoiBuffIndex ++;
		currPreDirectiveIndex ++;
	}

	// If there is printf operation, add printf directive
	if (m_printfBuffer)
	{
		// Set this directive settings
		coiBuffsArr[currCoiBuffIndex] = m_printfBuffer;
		// Set this COIBUFFER permission flag as write only on device side
		accessFlagArr[currCoiBuffIndex] = COI_SINK_WRITE_ENTIRE;
		// Set the directive for PRE and POST
		preExeDirectives[currPreDirectiveIndex].id = PRINTF;
		preExeDirectives[currPreDirectiveIndex].printfDirective.bufferIndex = currCoiBuffIndex;
		preExeDirectives[currPreDirectiveIndex].printfDirective.size = MIC_PRINTF_BUFFER_SIZE;

		postExeDirectives[currPostDirectiveIndex] = preExeDirectives[currPreDirectiveIndex];

		currPreDirectiveIndex ++;
		currPostDirectiveIndex ++;
		currCoiBuffIndex ++;
	}
	// If there is profiling operation, add profiling directive
	if (m_profilingBuffer)
	{
		// Set this directive settings
		coiBuffsArr[currCoiBuffIndex] = m_profilingBuffer;
		// Set this COIBUFFER permission flag as write only on device side
		accessFlagArr[currCoiBuffIndex] = COI_SINK_WRITE_ENTIRE;
		// Set the directive for PRE and POST
		preExeDirectives[currPreDirectiveIndex].id = PROFILING;
		preExeDirectives[currPreDirectiveIndex].profilingDirective.bufferIndex = currCoiBuffIndex;

		postExeDirectives[currPostDirectiveIndex] = preExeDirectives[currPreDirectiveIndex];

		currPreDirectiveIndex ++;
		currPostDirectiveIndex ++;
		currCoiBuffIndex ++;
	}
	// If it is OutOfOrderCommandList, add BARRIER directive to postExeDirectives
	if (false == dispatcherData.isInOrderQueue)
	{
		postExeDirectives[currPostDirectiveIndex].id = BARRIER;
		postExeDirectives[currPostDirectiveIndex].barrierDirective.end_barrier = m_completionBarrier;

		currPostDirectiveIndex ++;
	}

	// We going to send block of bytes as the first COIBUFFER wihch include the following:
	//    * dispatcherData
	//    * preExeDirectives
	//    * postExeDirectives
	//    * cmdParams->arg_values (kernel args blob)
	// Going to collect all the data together
	size_t groupedDataSize = sizeof(dispatcher_data) + (sizeof(directive_pack) * (numPreDirectives + numPostDirectives)) + cmdParams->arg_size;
	char* groupedData = (char*)malloc(groupedDataSize);
	memcpy(groupedData, &dispatcherData, sizeof(dispatcher_data));
	size_t offset = sizeof(dispatcher_data);
	memcpy(groupedData + offset, preExeDirectives, sizeof(directive_pack) * numPreDirectives);
	offset += sizeof(directive_pack) * numPreDirectives;
	memcpy(groupedData + offset, postExeDirectives, sizeof(directive_pack) * numPostDirectives);
	offset += sizeof(directive_pack) * numPostDirectives;
    memcpy(groupedData + offset, cmdParams->arg_values, cmdParams->arg_size);

	// Create coi buffer for dispatcher_data
	coi_err = COIBufferCreateFromMemory(
                            groupedDataSize,                                 // The number of bytes to allocate for the buffer.
							COI_BUFFER_NORMAL,                               // The type of the buffer to create
							0,                                               // A bitmask of attributes for the newly created buffer.
							groupedData,                                     // A pointer to an already allocated memory region on the source that should be turned into a COIBUFFER
                            1, &tProcess,                                    // The number of processes with which this buffer might be used, and The process
                            &(coiBuffsArr[0]));                              // Pointer to a buffer handle

	free(preExeDirectives);
	free(postExeDirectives);
	free(groupedData);

	// Is the COIBufferCreate succeeded?
	if (COI_SUCCESS != coi_err)
	{
		free(coiBuffsArr);
		free(accessFlagArr);
		m_lastError = CL_DEV_OBJECT_ALLOC_FAIL;
	    return CL_DEV_OBJECT_ALLOC_FAIL;
	}

	*ppOutCoiBuffsArr = coiBuffsArr;
	*ppAccessFlagArr = accessFlagArr;

	return CL_DEV_SUCCESS;
}

cl_dev_err_code NDRange::execute()
{
	cl_dev_err_code err = CL_DEV_SUCCESS;
	// the COIBUFFERs to dispatch
	COIBUFFER* coiBuffsArr = NULL;
	// the access flags of the COIBUFFERs array
	COI_ACCESS_FLAGS* accessFlagsArr = NULL;
	do
	{
		COIEVENT* barrier = NULL;
		unsigned int numDependecies = 0;
		m_pCommandSynchHandler->getLastDependentBarrier(m_pCommandList, &barrier, &numDependecies, true);

		// Get this queue COIPIPELINE handle
		COIPIPELINE pipe = m_pCommandList->getPipelineHandle();

		//Get COIFUNCTION handle according to func name (ask from DeviceServiceCommunication dictionary)
		COIFUNCTION func = m_pCommandList->getDeviceFunction( DeviceServiceCommunication::EXECUTE_NDRANGE );

		// The amount of COIBUFFERs to dispatch
		unsigned int numBuffersToDispatch = 0;
		err = init(&coiBuffsArr, &accessFlagsArr, &numBuffersToDispatch);
		if (err != CL_DEV_SUCCESS)
		{
			break;
		}

		/* Run the function pointed by 'func' on the device with 'numBuffersToDispatch' buffers and with dependency on 'barrier' (Can be NULL) and signal m_completionBarrier when finish.
		   'm_pCommandSynchHandler->registerCompletionBarrier(&m_completionBarrier))' can return NULL, in case of Out of order CommandList */
		COIRESULT result = COIPipelineRunFunction(pipe, func, numBuffersToDispatch, coiBuffsArr, accessFlagsArr, numDependecies, barrier,
														NULL, 0, NULL, 0, m_pCommandSynchHandler->registerCompletionBarrier(&m_completionBarrier));
		if (result != COI_SUCCESS)
		{
			err = CL_DEV_ERROR_FAIL;
			break;
		}
		// Set m_completionBarrier to be the last barrier in case of InOrder CommandList.
		m_pCommandSynchHandler->setLastDependentBarrier(m_pCommandList, m_completionBarrier, true);
		// Register m_completionBarrier to NotificationPort
		m_pCommandList->getNotificationPort()->addBarrier(m_completionBarrier, this, NULL);
	}
	while (0);

	if (coiBuffsArr)
	{
		free(coiBuffsArr);
	}
	if (accessFlagsArr)
	{
		free(accessFlagsArr);
	}
	if (CL_DEV_SUCCESS != err)
	{
	    m_lastError = err;
		delete this;
	}

    return err;
}

void NDRange::fireCallBack(void* arg)
{
	// Decrement the reference counter of this kernel program.
	m_pCommandList->getProgramService()->releaseKernelOnDevice(((cl_dev_cmd_param_kernel*)m_pCmd->params)->kernel);
	// If printf available
	if (m_printfBuffer)
	{
		// TODO - Read the COIBUFFER and print it
	}
	// if profiling available
	if (m_profilingBuffer)
	{
		// TODO - read the COIBUFFER and send the data to framework
	}

	// Notify runtime that  the command completed
	notifyCommandStatusChanged(CL_COMPLETE);
	// Delete this Command object
	delete this;
}

void NDRange::releaseResources()
{
	COIRESULT coiErr = COI_SUCCESS;
	if (m_dispatcherDataBuffer)
	{
		coiErr = COIBufferDestroy(m_dispatcherDataBuffer);
		assert(COI_SUCCESS == coiErr && "Buffer destruction failed");
	}
	if (m_printfBuffer)
	{
		coiErr = COIBufferDestroy(m_printfBuffer);
		assert(COI_SUCCESS == coiErr && "Buffer destruction failed");
	}
	if (m_profilingBuffer)
	{
		coiErr = COIBufferDestroy(m_profilingBuffer);
		assert(COI_SUCCESS == coiErr && "Buffer destruction failed");
	}
}
