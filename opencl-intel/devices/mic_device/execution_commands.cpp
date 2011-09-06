#include "execution_commands.h"
#include "cl_dev_backend_api.h"
#include "command_list.h"
#include "memory_allocator.h"

#include <source/COIBuffer_source.h>

using namespace Intel::OpenCL::MICDevice;
using namespace Intel::OpenCL::DeviceBackend;

NDRange::NDRange(CommandList* pCommandList, IOCLFrameworkCallbacks* pFrameworkCallBacks, cl_dev_cmd_desc* pCmd) : Command(pCommandList, pFrameworkCallBacks, pCmd),
m_printfBuffer(NULL), m_miscBuffer(NULL), m_extendedDispatcherData(NULL)
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
	cl_dev_err_code returnError = CL_DEV_SUCCESS;
	// array of directive_pack for preExeDirectives
	directive_pack* preExeDirectives = NULL;
	// array of directive_pack for preExeDirectives
	directive_pack* postExeDirectives = NULL;
	// Array of COIBUFFERs that will sent to the process (The first location is for the dispatcher data structure)
	// IT IS CALLER RESPONSIBILITY TO FREE THIS BUFFER (IT WILL SET THE OUT PARAMETER)
	COIBUFFER* coiBuffsArr = NULL;
	// Array of access flags that will sent to the process (The first location is for the dispatcher data structure permission)
	// IT IS CALLER RESPONSIBILITY TO FREE THIS ALLOCATION (IT WILL SET THE OUT PARAMETER)
	COI_ACCESS_FLAGS* accessFlagArr = NULL;
	// The amount of COIBUFFERS to dispatch (At least AMOUNT_OF_CONSTANT_BUFFERS for the 'dispatcher_data' structure and misc_data structure)
	unsigned int numOfDispatchingBuffers = AMOUNT_OF_CONSTANT_BUFFERS;
	do
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

#ifndef NDRANGE_UNIT_TEST
		// Get device side kernel address and set kernel directive (Also increase the reference counter of the Program.
		dispatcherData.kernelDirective.kernelAddress = m_pCommandList->getProgramService()->AcquireKernelOnDevice(cmdParams->kernel);
#else
		// Only for unit test
		dispatcherData.kernelDirective.kernelAddress = pKernel->GetKernelID();
#endif

		if (0 == dispatcherData.kernelDirective.kernelAddress)
		{
			returnError = CL_DEV_INVALID_KERNEL;
			break;
		}
		// set kernel args blob size in bytes for dispatcherData
		dispatcherData.kernelArgSize = cmdParams->arg_size;
		// set isInOrderQueue flag in dispatcherData
		dispatcherData.isInOrderQueue = m_pCommandSynchHandler->isInOrderType();
		// Filling the workDesc structure in dispatcherData
		dispatcherData.workDesc.setParams(cmdParams->work_dim, cmdParams->glb_wrk_offs, cmdParams->glb_wrk_size, cmdParams->lcl_wrk_size);

		// Get device side process in order to create COIBUFFERs for this process.
		COIPROCESS tProcess = m_pCommandList->getDeviceProcess();

		COIRESULT coi_err = COI_SUCCESS;

		// Create coi buffer for misc_data
		coi_err = COIBufferCreate(
								sizeof(misc_data),                               // The number of bytes to allocate for the buffer.
								COI_BUFFER_NORMAL,                               // The type of the buffer to create
								0,                                               // A bitmask of attributes for the newly created buffer.
								NULL,                                            // If non-NULL the buffer will be initialized with the data pointed
								1, &tProcess,                                    // The number of processes with which this buffer might be used, and The process
								&m_miscBuffer);			                         // Pointer to a buffer handle
		// Is the COIBufferCreate succeeded?
		if (COI_SUCCESS != coi_err)
		{
			returnError = CL_DEV_OBJECT_ALLOC_FAIL;
			break;
		}

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
				returnError = CL_DEV_OBJECT_ALLOC_FAIL;
				break;
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
		coiBuffsArr = (COIBUFFER*)malloc(sizeof(COIBUFFER) * numOfDispatchingBuffers);
		if (NULL == coiBuffsArr)
		{
			returnError = CL_DEV_OUT_OF_MEMORY;
			break;
		}
		// Array of access flags that will sent to the process (The first location is for the dispatcher data structure permission)
		// IT IS CALLER RESPONSIBILITY TO FREE THIS ALLOCATION (IT WILL SET THE OUT PARAMETER)
		accessFlagArr = (COI_ACCESS_FLAGS*)malloc(sizeof(COI_ACCESS_FLAGS) * numOfDispatchingBuffers);
		if (NULL == accessFlagArr)
		{
			returnError = CL_DEV_OUT_OF_MEMORY;
			break;
		}

		if (numPreDirectives > 0)
		{
			preExeDirectives = (directive_pack*)malloc(sizeof(directive_pack) * numPreDirectives);
			if (NULL == preExeDirectives)
			{
				returnError = CL_DEV_OUT_OF_MEMORY;
				break;
			}
		}
		if (numPostDirectives > 0)
		{
			postExeDirectives = (directive_pack*)malloc(sizeof(directive_pack) * numPostDirectives);
			if (NULL == postExeDirectives)
			{
				returnError = CL_DEV_OUT_OF_MEMORY;
				break;
			}
		}

		unsigned int currCoiBuffIndex = AMOUNT_OF_CONSTANT_BUFFERS;
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
				// Now check the memObj flags (The flags that the memObj created with)
				switch ( memObj->clDevMemObjGetMemoryFlags() )
				{
				case CL_MEM_READ_ONLY:
					{
						accessFlagArr[currCoiBuffIndex] = COI_SINK_READ;
						break;
					}
				case CL_MEM_WRITE_ONLY:
					{
						accessFlagArr[currCoiBuffIndex] = COI_SINK_WRITE_ENTIRE;
						break;
					}
				default:
					{
						accessFlagArr[currCoiBuffIndex] = COI_SINK_WRITE;
						break;
					}
				}
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

		// set misc coi buffer pointer
		coiBuffsArr[MISC_DATA_INDEX] = m_miscBuffer;
		// Set this COIBUFFER permission flag as write only on device side
		accessFlagArr[MISC_DATA_INDEX] = COI_SINK_WRITE_ENTIRE;

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
		m_extendedDispatcherData = (char*)malloc(dispatcherData.getDispatcherDataSize());
		if (NULL == m_extendedDispatcherData)
		{
			returnError = CL_DEV_OUT_OF_MEMORY;
			break;
		}
		memcpy(m_extendedDispatcherData, &dispatcherData, sizeof(dispatcher_data));
		memcpy(m_extendedDispatcherData + dispatcherData.preExeDirectivesArrOffset, preExeDirectives, sizeof(directive_pack) * numPreDirectives);
		memcpy(m_extendedDispatcherData + dispatcherData.postExeDirectivesArrOffset, postExeDirectives, sizeof(directive_pack) * numPostDirectives);
		memcpy(m_extendedDispatcherData + dispatcherData.kernelArgBlobOffset, cmdParams->arg_values, cmdParams->arg_size);

		// Create coi buffer for dispatcher_data
/*		coi_err = COIBufferCreateFromMemory(
								dispatcherData.getDispatcherDataSize(),          // The number of bytes to allocate for the buffer.
								COI_BUFFER_NORMAL,                               // The type of the buffer to create
								0,                                               // A bitmask of attributes for the newly created buffer.
								m_extendedDispatcherData,                        // A pointer to an already allocated memory region on the source that should be turned into a COIBUFFER
								1, &tProcess,                                    // The number of processes with which this buffer might be used, and The process
								&(coiBuffsArr[DISPATCHER_DATA_INDEX]));          // Pointer to a buffer handle
*/

		// TODO - switch it with COIBufferCreateFromMemory
		coi_err = COIBufferCreate(
								dispatcherData.getDispatcherDataSize(),          // The number of bytes to allocate for the buffer.
								COI_BUFFER_NORMAL,                               // The type of the buffer to create
								0,                                               // A bitmask of attributes for the newly created buffer.
								m_extendedDispatcherData,                        // A pointer to an already allocated memory region on the source that should be turned into a COIBUFFER
								1, &tProcess,                                    // The number of processes with which this buffer might be used, and The process
								&(coiBuffsArr[DISPATCHER_DATA_INDEX]));          // Pointer to a buffer handle

		// Is the COIBufferCreate succeeded?
		if (COI_SUCCESS != coi_err)
		{
			returnError = CL_DEV_INVALID_VALUE;
			break;
		}

		accessFlagArr[DISPATCHER_DATA_INDEX] = COI_SINK_READ;
	}
	while (0);

	if (preExeDirectives)
	{
		free(preExeDirectives);
	}
	if (postExeDirectives)
	{
		free(postExeDirectives);
	}

	if (CL_DEV_SUCCESS != returnError)
	{
		if (coiBuffsArr)
		{
			free(coiBuffsArr);
		}
		if (accessFlagArr)
		{
			free(accessFlagArr);
		}
	}
	else
	{
		*ppOutCoiBuffsArr = coiBuffsArr;
		*ppAccessFlagArr = accessFlagArr;
		*pOutNumBuffers = numOfDispatchingBuffers;
	}
	m_lastError = returnError;

	return returnError;
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
		delete this;
	}

    return err;
}

void NDRange::fireCallBack(void* arg)
{
#ifndef NDRANGE_UNIT_TEST
	// Decrement the reference counter of this kernel program.
	m_pCommandList->getProgramService()->releaseKernelOnDevice(((cl_dev_cmd_param_kernel*)m_pCmd->params)->kernel);
#endif
	// If printf available
	if (m_printfBuffer)
	{
		// TODO - Read the COIBUFFER and print it
	}
	assert(m_miscBuffer);
	misc_data miscData;
	// read m_miscBuffer in order to get kernel execution result and profiling data (synchronous)
	COIRESULT coiErr = COIBufferRead ( m_miscBuffer,
										0,
										&miscData,
										sizeof(miscData),
										COI_COPY_USE_CPU,
										0,
										NULL,
										NULL );

	assert(COI_SUCCESS == coiErr);
	if (CL_DEV_SUCCESS == m_lastError)
	{
		m_lastError = miscData.errCode;
	}
	// if profiling available
	if (m_pCmd->profiling)
	{
		// TODO - read the profiling data from miscData convert the data to host time profiling and send the data to framework
	}
	// Notify runtime that  the command completed
	notifyCommandStatusChanged(CL_COMPLETE);
	// Delete this Command object
	delete this;
}

void NDRange::releaseResources()
{
	COIRESULT coiErr = COI_SUCCESS;
	if (m_printfBuffer)
	{
		coiErr = COIBufferDestroy(m_printfBuffer);
		assert(COI_SUCCESS == coiErr && "Buffer destruction failed");
	}
	if (m_miscBuffer)
	{
		coiErr = COIBufferDestroy(m_miscBuffer);
		assert(COI_SUCCESS == coiErr && "Buffer destruction failed");
	}
	if (m_extendedDispatcherData)
	{
		free(m_extendedDispatcherData);
	}
}
