#include "execution_commands.h"
#include "cl_dev_backend_api.h"
#include "command_list.h"
#include "memory_allocator.h"

#include <source/COIBuffer_source.h>

using namespace Intel::OpenCL::MICDevice;
using namespace Intel::OpenCL::DeviceBackend;

extern bool gSafeReleaseOfCoiObjects;

NDRange::NDRange(CommandList* pCommandList, IOCLFrameworkCallbacks* pFrameworkCallBacks, cl_dev_cmd_desc* pCmd) : Command(pCommandList, pFrameworkCallBacks, pCmd),
m_printfBuffer(NULL)
{
}

NDRange::~NDRange()
{
	releaseResources(gSafeReleaseOfCoiObjects);
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

cl_dev_err_code NDRange::init(vector<COIBUFFER>& outCoiBuffsArr, vector<COI_ACCESS_FLAGS>& outAccessFlagArr)
{
	cl_dev_err_code returnError = CL_DEV_SUCCESS;
	// array of directive_pack for preExeDirectives
	directive_pack* preExeDirectives = NULL;
	// array of directive_pack for preExeDirectives
	directive_pack* postExeDirectives = NULL;
	do
	{
        ProgramService* program_service = m_pCommandList->getProgramService();

		// Get command params
		cl_dev_cmd_param_kernel *cmdParams = (cl_dev_cmd_param_kernel*)m_pCmd->params;
		// Get Kernel from params
#ifndef NDRANGE_UNIT_TEST
		const ICLDevBackendKernel_* pKernel = program_service->GetBackendKernel(cmdParams->kernel);
#else
		program_service = NULL;
		const ICLDevBackendKernel_* pKernel = (ICLDevBackendKernel_*)cmdParams->kernel;
#endif
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
		dispatcherData.kernelDirective.kernelAddress = program_service->AcquireKernelOnDevice(cmdParams->kernel);
#else
		// Only for unit test
		dispatcherData.kernelDirective.kernelAddress = pKernel->GetKernelID();
#endif

		if (0 == dispatcherData.kernelDirective.kernelAddress)
		{
			returnError = CL_DEV_INVALID_KERNEL;
			break;
		}
		// set unique command identifier
		dispatcherData.commandIdentifier = m_pCmd->id;
		// set kernel args blob size in bytes for dispatcherData
		dispatcherData.kernelArgSize = cmdParams->arg_size;
		// set isInOrderQueue flag in dispatcherData
		dispatcherData.isInOrderQueue = m_pCommandSynchHandler->isInOrderType();
		// Filling the workDesc structure in dispatcherData
		dispatcherData.workDesc.setParams(cmdParams->work_dim, cmdParams->glb_wrk_offs, cmdParams->glb_wrk_size, cmdParams->lcl_wrk_size);

		// directives counter.
		unsigned int numPreDirectives = 0;
		unsigned int numPostDirectives = 0;

		// Vector which save the kernel buffer arguments basic info in order to traverse on uiNumArgs only once
		vector<NDRange::kernel_arg_buffer_info> buffsInfoVector;
		buffsInfoVector.reserve(uiNumArgs);

		// Get the amount of buffers in kernel args and info of their offset in blob and their index in pArgs
		getKernelArgBuffersCount(uiNumArgs, pArgs, buffsInfoVector);
		// The amount of buffers in kernel args
		unsigned int numOfBuffersInKernelArgs = buffsInfoVector.size();

		numPreDirectives += numOfBuffersInKernelArgs;

		// If there is printf operation, add printf directive
		bool tHasPrintOperation = pKernel->GetKernelProporties()->HasPrintOperation();
		if (tHasPrintOperation)
		{
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

		outCoiBuffsArr.reserve(numOfBuffersInKernelArgs + AMOUNT_OF_OPTIONAL_DISPATCH_BUFFERS);
		outAccessFlagArr.reserve(numOfBuffersInKernelArgs + AMOUNT_OF_OPTIONAL_DISPATCH_BUFFERS);

		if (numPreDirectives > 0)
		{
			preExeDirectives = new directive_pack[numPreDirectives];
			if (NULL == preExeDirectives)
			{
				returnError = CL_DEV_OUT_OF_MEMORY;
				break;
			}
		}
		if (numPostDirectives > 0)
		{
			postExeDirectives = new directive_pack[numPostDirectives];
			if (NULL == postExeDirectives)
			{
				returnError = CL_DEV_OUT_OF_MEMORY;
				break;
			}
		}

		unsigned int currPreDirectiveIndex = 0;
		unsigned int currPostDirectiveIndex = 0;

		// Set the buffer arguments of the kernel as directives and COIBUFFERS
		assert(numPreDirectives >= numOfBuffersInKernelArgs);
		for (unsigned int i = 0; i < numOfBuffersInKernelArgs; i++)
		{
			size_t stOffset = buffsInfoVector[i].offsetInBlob;
			MICDevMemoryObject *memObj = (MICDevMemoryObject*)*((IOCLDevMemoryObject**)(pKernelParams+stOffset));
			// Set the COIBUFFER of coiBuffsArr[currCoiBuffIndex] to be the COIBUFFER that hold the data of the buffer.
			outCoiBuffsArr.push_back(memObj->clDevMemObjGetCoiBufferHandler());
			// TODO - Change the flag according to the information about the argument (Not implemented yet by the BE)
			if ( CL_KRNL_ARG_PTR_CONST == pArgs[buffsInfoVector[i].index].type )
			{
				outAccessFlagArr.push_back(COI_SINK_READ);
			}
			else
			{
				// Now check the memObj flags (The flags that the memObj created with)
				switch ( memObj->clDevMemObjGetMemoryFlags() )
				{
				case CL_MEM_READ_ONLY:
					{
						outAccessFlagArr.push_back(COI_SINK_READ);
						break;
					}
				case CL_MEM_WRITE_ONLY:
					{
						outAccessFlagArr.push_back(COI_SINK_WRITE_ENTIRE);
						break;
					}
				default:
					{
						outAccessFlagArr.push_back(COI_SINK_WRITE);
						break;
					}
				}
			}
			assert(outCoiBuffsArr.size() == outAccessFlagArr.size());
			// Set this directive settings
			preExeDirectives[currPreDirectiveIndex].id = BUFFER;
			preExeDirectives[currPreDirectiveIndex].bufferDirective.bufferIndex = outCoiBuffsArr.size() - 1;
			preExeDirectives[currPreDirectiveIndex].bufferDirective.offset_in_blob = stOffset;
			preExeDirectives[currPreDirectiveIndex].bufferDirective.mem_obj_desc = memObj->clDevMemObjGetDescriptorRaw();

			currPreDirectiveIndex ++;
		}

		// Get device side process in order to create COIBUFFERs for this process.
		COIPROCESS tProcess = m_pCommandList->getDeviceProcess();

		COIRESULT coi_err = COI_SUCCESS;
		// If there is printf operation, add printf directive
		if (tHasPrintOperation)
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

			// Set this directive settings
			outCoiBuffsArr.push_back(m_printfBuffer);
			// Set this COIBUFFER permission flag as write only on device side
			outAccessFlagArr.push_back(COI_SINK_WRITE_ENTIRE);
			assert(outCoiBuffsArr.size() == outAccessFlagArr.size());
			// Set the directive for PRE and POST
			preExeDirectives[currPreDirectiveIndex].id = PRINTF;
			preExeDirectives[currPreDirectiveIndex].printfDirective.bufferIndex = outCoiBuffsArr.size() - 1;
			preExeDirectives[currPreDirectiveIndex].printfDirective.size = MIC_PRINTF_BUFFER_SIZE;

			postExeDirectives[currPostDirectiveIndex] = preExeDirectives[currPreDirectiveIndex];

			currPreDirectiveIndex ++;
			currPostDirectiveIndex ++;
		}

		// If it is OutOfOrderCommandList, add BARRIER directive to postExeDirectives
		if (false == dispatcherData.isInOrderQueue)
		{
			postExeDirectives[currPostDirectiveIndex].id = BARRIER;
			postExeDirectives[currPostDirectiveIndex].barrierDirective.end_barrier = m_completionBarrier;

			currPostDirectiveIndex ++;
		}

		// initialize the miscDataHandler
		returnError = m_miscDatahandler.init(!dispatcherData.isInOrderQueue, &tProcess);
		if (CL_DEV_FAILED(returnError))
		{
			break;
		}
		// register misc_data
		m_miscDatahandler.registerMiscData(outCoiBuffsArr, outAccessFlagArr);

		returnError = m_dispatcherDatahandler.init(dispatcherData, preExeDirectives, postExeDirectives, pKernelParams, &tProcess);
		if (CL_DEV_FAILED(returnError))
		{
			break;
		}
		m_dispatcherDatahandler.registerDispatcherData(outCoiBuffsArr, outAccessFlagArr);

		assert(outCoiBuffsArr.size() == outAccessFlagArr.size());
	}
	while (0);

	if (preExeDirectives)
	{
		delete [] preExeDirectives;
	}
	if (postExeDirectives)
	{
		delete [] postExeDirectives;
	}

	if (CL_DEV_FAILED(returnError))
	{
		outCoiBuffsArr.clear();
		outAccessFlagArr.clear();
	}
	m_lastError = returnError;

	return returnError;
}

cl_dev_err_code NDRange::execute()
{
	cl_dev_err_code err = CL_DEV_SUCCESS;
	// the COIBUFFERs to dispatch
	vector<COIBUFFER> coiBuffsArr;
	// the access flags of the COIBUFFERs array
	vector<COI_ACCESS_FLAGS> accessFlagsArr;
	do
	{
		COIEVENT* barrier = NULL;
		unsigned int numDependecies = 0;
		m_pCommandSynchHandler->getLastDependentBarrier(m_pCommandList, &barrier, &numDependecies, true);

		// Get this queue COIPIPELINE handle
		COIPIPELINE pipe = m_pCommandList->getPipelineHandle();

		//Get COIFUNCTION handle according to func name (ask from DeviceServiceCommunication dictionary)
		COIFUNCTION func = m_pCommandList->getDeviceFunction( DeviceServiceCommunication::EXECUTE_NDRANGE );

		err = init(coiBuffsArr, accessFlagsArr);
		if (err != CL_DEV_SUCCESS)
		{
			break;
		}

		/* Run the function pointed by 'func' on the device with 'numBuffersToDispatch' buffers and with dependency on 'barrier' (Can be NULL) and signal m_completionBarrier when finish.
		   'm_pCommandSynchHandler->registerCompletionBarrier(&m_completionBarrier))' can return NULL, in case of Out of order CommandList */
		COIRESULT result = COIPipelineRunFunction(pipe,
												  func,
												  coiBuffsArr.size(), &(coiBuffsArr[0]), &(accessFlagsArr[0]),
												  numDependecies, barrier,
												  m_dispatcherDatahandler.getDispatcherDataPtrForCoiRunFunc(), m_dispatcherDatahandler.getDispatcherDataSizeForCoiRunFunc(),
												  m_miscDatahandler.getMiscDataPtrForCoiRunFunc(), m_miscDatahandler.getMiscDataSizeForCoiRunFunc(), 
												  m_pCommandSynchHandler->registerCompletionBarrier(&m_completionBarrier));
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

	if (CL_DEV_FAILED(err))
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
	misc_data miscData;
	m_miscDatahandler.readMiscData(&miscData);
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

void NDRange::releaseResources(bool releaseCoiObjects)
{
	m_dispatcherDatahandler.release();
	m_miscDatahandler.release();
	if (releaseCoiObjects)
	{
		COIRESULT coiErr = COI_SUCCESS;
		if (m_printfBuffer)
		{
			coiErr = COIBufferDestroy(m_printfBuffer);
			assert(COI_SUCCESS == coiErr && "Buffer destruction failed");
		}
	}
}
