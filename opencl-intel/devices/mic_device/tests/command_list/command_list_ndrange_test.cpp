#include "cl_device_api.h"
#include "mic_device.h"
#include "command_list.h"
#include "ICLDevBackendKernel.h"

#include <cstring>
#include <assert.h>

using namespace Intel::OpenCL::MICDevice;

class MyIOCLDevBackingStore : public IOCLDevBackingStore
{
public:

	MyIOCLDevBackingStore(void* hostPtr, size_t size) : m_ptr(hostPtr), m_size(size)
	{
		m_dim[0] = size;
		m_dim[1] = 0;
		m_dim[2] = 0;
		m_pitch[0] = 0;
		m_pitch[1] = 0;
		m_imageFormat.image_channel_order = 0;
		m_imageFormat.image_channel_data_type = 0;
	}

	//!	Returns pointer to a backing store data
	/*!
		\retval	A pointer to backing store raw data area
	*/
	void* GetRawData() const
	{
		return m_ptr;
	}

	//!	Returns a description of the Raw Data origin
	/*!
		\retval	A pointer to backing store raw data area
	*/
	cl_dev_bs_description GetRawDataDecription() const
	{
		return CL_DEV_BS_USER_ALLOCATED;
	}

	//!	Returns if there is a valid data stored in backing store
	/*!
		\retval	true	The backing store holding the valid data
		\retval	false	No valid data in the backing store
	*/

	//!	Returns a size of the raw data in bytes
    size_t GetRawDataSize() const
	{
		return m_size;
	}

	//!	Returns an offset in bytes to the element identified at origin.
	//!    origin must be an array with MAX_WORK_DIM elements
    size_t GetRawDataOffset( const size_t* origin ) const
	{
		return origin[0];
	}

	bool IsDataValid() const
	{
		return true;
	}

	//!	Returns a number of dimensions used by data in the Backing Store
	size_t GetDimCount() const
	{
		return 1;
	}

	//!	Returns a pointer to array of dimensions in bytes of the backing store The size of the array is dim_count.
	//!		pitch[0] will contain the scan-line pitch in bytes of the mapped region,
	//!		pitch[1] will contain the size in bytes of each 2D slice of the mapped region.
	//!		The NULL value is valid only for 1D buffers.
	const size_t* GetDimentions() const
	{
		return m_dim;
	}

	//!	Returns a pointer to array of dimension pitches in bytes for the mapped region. The size of the array is dim_count-1.
	//!		pitch[0] will contain the scan-line pitch in bytes of the mapped region,
	//!		pitch[1] will contain the size in bytes of each 2D slice of the mapped region.
	const size_t* GetPitch() const
	{ 
		return m_pitch;
	}

	//!	Returns a reference to the image format. If Backing Store does not represent image the value of format is undefined.
    const cl_image_format&  GetFormat() const
	{
		return m_imageFormat;
	}

	//!	Returns a size of the image element or 1 if Backing Store represents buffer.
    size_t GetElementSize() const
	{
		return 1;
	}

	//!	Add pendency on backing store instance and returns the new value
	/*!
		\retval The new pendency count
	*/
	int AddPendency()
	{
		return 1;
	}

	//!	Remove pendency on backing store instance, when pendency is 0 the backing store is released
	/*!
		\retval	The new pendency count
	*/
	int RemovePendency()
	{
		return 1;
	}

private:

	void*				m_ptr;
	size_t				m_size;
	size_t				m_dim[3];
	size_t				m_pitch[2];
	cl_image_format		m_imageFormat;
};


class MyIOCLDevRTMemObjectService : public IOCLDevRTMemObjectService
{
public:

	MyIOCLDevRTMemObjectService(IOCLDevBackingStore* backingStore, IOCLDeviceAgent* micDevAgent) : m_backingStore(backingStore), m_micDeviceAgent(micDevAgent)
	{
	}

	//!	Retrieves current memory object backing store.
	/*!
		\param[in]	flags	A flag represents backing store access flags
		\paran[out] ppBS	Runtime memory object backing store,
							may return NULL if BS doesn't exists

		\retval	CL_DEV_SUCCESS		The function is executed successfully.
		\retval	CL_DEV_ERROR_FAIL	When error occured during Backing store retrivals
	*/
	cl_dev_err_code GetBackingStore(cl_dev_bs_flags flags, IOCLDevBackingStore* *ppBS)
	{
		*ppBS = m_backingStore;
		return CL_DEV_SUCCESS;
	}

	//!	Updates current memory object backing store.
	/*!
		\paran[out] pBS		Updated memory object backing store to be used by the runtime

		\retval	CL_DEV_SUCCESS		The function is executed successfully.
		\retval	CL_DEV_ERROR_FAIL	When error occured during backing store update
	*/
	cl_dev_err_code SetBackingStore(IOCLDevBackingStore* pBS)
	{
		m_backingStore = pBS;
		return CL_DEV_SUCCESS;
	}

	//!	Returns the number of Device Agent sharing the runtime memory object
	size_t GetDeviceAgentListSize() const
	{
		return 1;
	}

	//!	Returns the list Device Agent sharing the runtime memory object
	const IOCLDeviceAgent* const *GetDeviceAgentList() const
	{
		return &m_micDeviceAgent;
	}

    void BackingStoreUpdateFinished(void* handle, cl_dev_err_code* dev_error) {};
    
private:

	IOCLDevBackingStore*		m_backingStore;
	IOCLDeviceAgent*			m_micDeviceAgent;
};



class ICLDevBackendKernelProportiesStub : public ICLDevBackendKernelProporties
{
public:

	unsigned int GetKernelPackCount() const
	{
		assert(0);
		return 0;
	}

    const size_t* GetRequiredWorkGroupSize() const 
	{
		assert(0);
		return NULL;
	}

    size_t GetPrivateMemorySize() const
	{
		assert(0);
		return 0;
	}

    size_t GetImplicitLocalMemoryBufferSize() const
	{
		assert(0);
		return 0;
	}

    bool HasPrintOperation() const
	{
		return true;
	}

    bool HasBarrierOperation() const
	{
		assert(0);
		return false;
	}

	bool HasKernelCallOperation() const
	{
		assert(0);
		return false;
	}
};



class ICLDevBackendKernelStub : public ICLDevBackendKernel_
{
#define PARAMS_COUNT 5
public:

	ICLDevBackendKernelStub()
	{
		m_kernelArguments[0].type = CL_KRNL_ARG_INT;
		m_kernelArguments[0].size_in_bytes = sizeof(unsigned int);
		m_kernelArguments[1].type = CL_KRNL_ARG_PTR_CONST;
		m_kernelArguments[1].size_in_bytes = sizeof(IOCLDevMemoryObject**);
		m_kernelArguments[2].type = CL_KRNL_ARG_PTR_CONST;
		m_kernelArguments[2].size_in_bytes = sizeof(IOCLDevMemoryObject**);
		m_kernelArguments[3].type = CL_KRNL_ARG_PTR_GLOBAL;
		m_kernelArguments[3].size_in_bytes = sizeof(IOCLDevMemoryObject**);
		m_kernelArguments[4].type = CL_KRNL_ARG_INT;
		m_kernelArguments[4].size_in_bytes = sizeof(unsigned int);

		m_kernelProperties = new ICLDevBackendKernelProportiesStub();
	}

	virtual ~ICLDevBackendKernelStub()
	{
		delete(m_kernelProperties);
	}

	unsigned long long int GetKernelID() const
	{
		return 54321;
	}

	const char*	GetKernelName() const
	{
		assert(0);
		return NULL;
	}

	int GetKernelParamsCount() const
	{ 
		return PARAMS_COUNT; 
	}

    const cl_kernel_argument* GetKernelParams() const
	{
		return m_kernelArguments;
	}

    const ICLDevBackendKernelProporties* GetKernelProporties() const
	{
		return m_kernelProperties;
	}

private:

	cl_kernel_argument						m_kernelArguments[PARAMS_COUNT];
	ICLDevBackendKernelProporties*			m_kernelProperties;

};


/* Class inheritance from IOCLFrameworkCallbacks (Framework call back interface) */
class MyCallBackManager : public IOCLFrameworkCallbacks
{
public:

	void clDevCmdStatusChanged(cl_dev_cmd_id IN cmd_id, void* IN data, cl_int IN cmd_status, cl_int	IN completion_result, cl_ulong IN timer)
	{
		// if the command completed
		if ((CL_COMPLETE == cmd_status) && (data))
		{
			*((bool*)((volatile void*)data)) = true;
		}
	}
	
	void clDevBuildStatusUpdate(cl_dev_program prog, void* IN data, cl_build_status IN status)
	{
		return;
	}
};

#define checkClDevErrCode(err, msg) if (CL_DEV_SUCCESS != err) { printf("%s\n", msg); break; }

cl_dev_err_code writeBuffer(MICDevice* pMicDeviceAgent, cl_dev_cmd_list& pCmdList, bool profiling, IOCLDevMemoryObject* memObj, void* pHostBuffer, size_t size, cl_dev_cmd_desc* cmdDesc, volatile bool* dataFlag)
{
	cl_dev_err_code					err = CL_DEV_SUCCESS;
	cl_dev_cmd_param_rw* rwParams = (cl_dev_cmd_param_rw*)malloc(sizeof(cl_dev_cmd_param_rw));
	assert(rwParams);
	
	memset(rwParams, 0, sizeof(cl_dev_cmd_param_rw));
	rwParams->memObj = memObj;
	rwParams->ptr = pHostBuffer;
	rwParams->dim_count = 1;
	rwParams->region[0] = size;
	rwParams->region[1] = 1;
	rwParams->region[2] = 1;
	rwParams->origin[0] = 0;
	cmdDesc->type = CL_DEV_CMD_WRITE;
	cmdDesc->id = (cl_dev_cmd_id)CL_DEV_CMD_WRITE;
	cmdDesc->params = rwParams;
	cmdDesc->param_size = sizeof(cl_dev_cmd_param_rw);
	cmdDesc->profiling = profiling;
	cmdDesc->data = (void*)dataFlag;

	err = pMicDeviceAgent->clDevCommandListExecute(pCmdList, &cmdDesc, 1);
	return err;
}

cl_dev_err_code readBuffer(MICDevice* pMicDeviceAgent, cl_dev_cmd_list& pCmdList, bool profiling, IOCLDevMemoryObject* memObj, void* pHostBuffer, size_t size, cl_dev_cmd_desc* cmdDesc, volatile bool* dataFlag)
{
	cl_dev_err_code					err = CL_DEV_SUCCESS;
	cl_dev_cmd_param_rw* rwParams = (cl_dev_cmd_param_rw*)malloc(sizeof(cl_dev_cmd_param_rw));
	assert(rwParams);
	
	memset(rwParams, 0, sizeof(cl_dev_cmd_param_rw));
	rwParams->memObj = memObj;;
	rwParams->dim_count = 1;
	rwParams->region[0] = size;
	rwParams->region[1] = 1;
	rwParams->region[2] = 1;
	rwParams->origin[0] = 0;
	rwParams->ptr = pHostBuffer;

	cmdDesc->type = CL_DEV_CMD_READ;
	cmdDesc->id = (cl_dev_cmd_id)CL_DEV_CMD_READ;
	cmdDesc->params = rwParams;
	cmdDesc->param_size = sizeof(cl_dev_cmd_param_rw);
	cmdDesc->profiling = profiling;
	cmdDesc->data = (void*)dataFlag;

	err = pMicDeviceAgent->clDevCommandListExecute(pCmdList, &cmdDesc, 1);
	return err;
}

bool runNDRangeUnitTest(unsigned int numOfNDRangeCommands, unsigned int numOfContinuousNDRanges)
{
	bool testResult = false;

	cl_dev_err_code					err = CL_DEV_SUCCESS;
	cl_uint							workDim = 1;
	MICDevice*						pMicDeviceAgent;
	IOCLFrameworkCallbacks*			pFrameworkCallback;
	cl_dev_cmd_list					commandList = NULL;
	cl_dev_cmd_desc*				commandDescriptor = NULL;
	commandDescriptor = (cl_dev_cmd_desc*)malloc(sizeof(cl_dev_cmd_desc) * numOfNDRangeCommands);
	assert(commandDescriptor);
	cl_dev_cmd_param_kernel*		commandParams = NULL;
	commandParams = (cl_dev_cmd_param_kernel*)malloc(sizeof(cl_dev_cmd_param_kernel) * numOfNDRangeCommands);
	assert(commandParams);
	ICLDevBackendKernel_*			pBackendKernel = NULL;

	volatile bool* completionFlag = NULL;
	completionFlag = (volatile bool*)malloc(sizeof(bool) * numOfNDRangeCommands);
	assert(completionFlag);
	volatile bool* readCompletionFlag = NULL;
	readCompletionFlag = (volatile bool*)malloc(sizeof(bool) * numOfNDRangeCommands);
	assert(readCompletionFlag);

	unsigned int* hostMemObj2 = NULL;
	unsigned int* hostMemObj3 = NULL;
	unsigned int** hostMemObj4 = NULL;
	hostMemObj4 = (unsigned int**)malloc(sizeof(unsigned int*) * numOfNDRangeCommands);
	assert(hostMemObj4);
	memset(hostMemObj4, 0, sizeof(unsigned int*) * numOfNDRangeCommands);
	// Decleration of kernel args
	unsigned int					uiArg1 = 256;
	IOCLDevMemoryObject*			memObj2 = NULL;
	IOCLDevMemoryObject*			memObj3 = NULL;
	IOCLDevMemoryObject**			memObj4 = NULL;
	memObj4 = (IOCLDevMemoryObject**)malloc(sizeof(IOCLDevMemoryObject*) * numOfNDRangeCommands);
	assert(memObj4);
	memset(memObj4, 0, sizeof(IOCLDevMemoryObject*) * numOfNDRangeCommands);
	unsigned int					uiArg5 = 55555;
	cl_dev_cmd_desc					writeCommandDesc1;
	cl_dev_cmd_desc					writeCommandDesc2;
	cl_dev_cmd_desc*				readCommandDesc = (cl_dev_cmd_desc*)malloc(sizeof(cl_dev_cmd_desc) * numOfNDRangeCommands);
	assert(readCommandDesc);
	hostMemObj2 = (unsigned int*)malloc(sizeof(unsigned int) * uiArg1);
	IOCLDevBackingStore* MyIOCLDevBackingStoreOfMemObj2 = new MyIOCLDevBackingStore(hostMemObj2, sizeof(unsigned int) * uiArg1);
	hostMemObj3 = (unsigned int*)malloc(sizeof(unsigned int) * uiArg1);
	IOCLDevBackingStore* MyIOCLDevBackingStoreOfMemObj3 = new MyIOCLDevBackingStore(hostMemObj3, sizeof(unsigned int) * uiArg1);
	IOCLDevBackingStore** MyIOCLDevBackingStoreOfMemObj4 = (IOCLDevBackingStore**)malloc(sizeof(IOCLDevBackingStore*) * numOfNDRangeCommands);
	assert(MyIOCLDevBackingStoreOfMemObj4);
	for (unsigned int i = 0; i < uiArg1; i++)
	{
		hostMemObj2[i] = i;
		hostMemObj3[i] = i * 2;
	}
	// Set up kernel args blob
	size_t							blobSize = sizeof(unsigned int) * 2 + sizeof(IOCLDevMemoryObject*) * 3;
	char**							blob = (char**)malloc(sizeof(char*) * numOfNDRangeCommands);
	assert(blob);
	memset(blob, 0, sizeof(char*) * numOfNDRangeCommands);
	IOCLDevRTMemObjectService* rtMemObjService2 = NULL;
	IOCLDevRTMemObjectService* rtMemObjService3 = NULL;
	IOCLDevRTMemObjectService** rtMemObjService4 = NULL;
	rtMemObjService4 = (IOCLDevRTMemObjectService**)malloc(sizeof(IOCLDevRTMemObjectService*) * numOfNDRangeCommands);
	assert(rtMemObjService4);
	memset(rtMemObjService4, 0, sizeof(IOCLDevRTMemObjectService*) * numOfNDRangeCommands);
	do
	{
		pFrameworkCallback = new MyCallBackManager();
		assert(pFrameworkCallback);
		pMicDeviceAgent = new MICDevice(0, 0, pFrameworkCallback, NULL);
		assert(pMicDeviceAgent);

		// Init mic device agent
		err = pMicDeviceAgent->Init();
		checkClDevErrCode(err, "pMicDeviceAgent->Init() FAILED\n");

		rtMemObjService2 = new MyIOCLDevRTMemObjectService(MyIOCLDevBackingStoreOfMemObj2, pMicDeviceAgent);
		rtMemObjService3 = new MyIOCLDevRTMemObjectService(MyIOCLDevBackingStoreOfMemObj3, pMicDeviceAgent);

		err = pMicDeviceAgent->clDevCreateMemoryObject(NULL,CL_MEM_READ_ONLY, NULL, 1,(const size_t*)&uiArg1, rtMemObjService2, (IOCLDevMemoryObject**)&memObj2);
		checkClDevErrCode(err, "pMicDeviceAgent->clDevCreateMemoryObject() FAILED\n");
		err = pMicDeviceAgent->clDevCreateMemoryObject(NULL,CL_MEM_READ_ONLY, NULL, 1,(const size_t*)&uiArg1, rtMemObjService3, (IOCLDevMemoryObject**)&memObj3);
		checkClDevErrCode(err, "pMicDeviceAgent->clDevCreateMemoryObject() FAILED\n");


		pBackendKernel = new ICLDevBackendKernelStub();
		assert(pBackendKernel);

		char*	tempBlob = NULL;
		for (unsigned int i = 0; i < numOfNDRangeCommands; i++)
		{
			memset(&(commandDescriptor[i]), 0, sizeof(cl_dev_cmd_desc));
			memset(&(commandParams[i]), 0, sizeof(cl_dev_cmd_param_kernel));
			completionFlag[i] = false;
			readCompletionFlag[i] = false;
			hostMemObj4[i] = (unsigned int*)malloc(sizeof(unsigned int) * uiArg1);
			assert(hostMemObj4[i]);
			memset(hostMemObj4[i], 0, sizeof(unsigned int) * uiArg1);
			MyIOCLDevBackingStoreOfMemObj4[i] = new MyIOCLDevBackingStore(hostMemObj4[i], sizeof(unsigned int) * uiArg1);
			blob[i] = (char*)malloc(blobSize);
			assert(blob[i]);
			memset(blob[i], 0, blobSize);
			rtMemObjService4[i] = new MyIOCLDevRTMemObjectService(MyIOCLDevBackingStoreOfMemObj4[i], pMicDeviceAgent);
			err = pMicDeviceAgent->clDevCreateMemoryObject(NULL,CL_MEM_WRITE_ONLY, NULL, 1,(const size_t*)&uiArg1, rtMemObjService4[i], (IOCLDevMemoryObject**)&(memObj4[i]));
			checkClDevErrCode(err, "pMicDeviceAgent->clDevCreateMemoryObject() FAILED\n");

			tempBlob = blob[i];
			*((unsigned int*)tempBlob) = uiArg1;
			tempBlob += sizeof(unsigned int);
			*((IOCLDevMemoryObject**)tempBlob) = memObj2;
			tempBlob += sizeof(IOCLDevMemoryObject*);
			*((IOCLDevMemoryObject**)tempBlob) = memObj3;
			tempBlob += sizeof(IOCLDevMemoryObject*);
			*((IOCLDevMemoryObject**)tempBlob) = memObj4[i];
			tempBlob += sizeof(IOCLDevMemoryObject*);
			*((unsigned int*)tempBlob) = uiArg5;

			// set up cl_dev_cmd_param_kernel
			commandParams[i].kernel = pBackendKernel;
			commandParams[i].work_dim = workDim;
			memset(commandParams[i].glb_wrk_offs, 0, sizeof(size_t) * MAX_WORK_DIM);
			memset(commandParams[i].glb_wrk_size, 0, sizeof(size_t) * MAX_WORK_DIM);
			memset(commandParams[i].lcl_wrk_size, 0, sizeof(size_t) * MAX_WORK_DIM);
			commandParams[i].glb_wrk_size[0] = 8;
			commandParams[i].lcl_wrk_size[0] = 4;
			commandParams[i].arg_values = blob[i];
			commandParams[i].arg_size = blobSize;
			// set up cl_dev_cmd_desc
			commandDescriptor[i].type = CL_DEV_CMD_EXEC_KERNEL;
			commandDescriptor[i].id = (void*)(size_t)i;
			commandDescriptor[i].data = (void*)&(completionFlag[i]);
			commandDescriptor[i].params = &(commandParams[i]);
			commandDescriptor[i].param_size = sizeof(cl_dev_cmd_param_kernel);
			commandDescriptor[i].profiling = true;
		}
		checkClDevErrCode(err, "pMicDeviceAgent->clDevCreateMemoryObject() FAILED\n");

		// create command list
		err = pMicDeviceAgent->clDevCreateCommandList(CL_DEV_LIST_NONE, /* cl_dev_subdevice_id */ NULL, &commandList);
		checkClDevErrCode(err, "pMicDeviceAgent->clDevCreateCommandList() FAILED\n");
		assert(commandList);

		err = writeBuffer(pMicDeviceAgent, commandList, false, memObj2, hostMemObj2, sizeof(unsigned int) * uiArg1, &writeCommandDesc1, NULL);
		checkClDevErrCode(err, "writeBuffer1 FAILED\n");
		err = writeBuffer(pMicDeviceAgent, commandList, false, memObj3, hostMemObj3, sizeof(unsigned int) * uiArg1, &writeCommandDesc2, NULL);
		checkClDevErrCode(err, "writeBuffer2 FAILED\n");
		cl_dev_cmd_desc* pTempCommandDesc = NULL;
		for (unsigned int i = 0; i < numOfNDRangeCommands; i++)
		{
			pTempCommandDesc = &(commandDescriptor[i]);
			err = pMicDeviceAgent->clDevCommandListExecute(commandList, &pTempCommandDesc, 1);
			checkClDevErrCode(err, "pMicDeviceAgent->clDevCommandListExecute() FAILED\n");
			if ((i % numOfContinuousNDRanges == 0) && (i > 0))
			{
				err = readBuffer(pMicDeviceAgent, commandList, false, memObj4[i], hostMemObj4[i], sizeof(unsigned int) * uiArg1, &(readCommandDesc[i]), &(readCompletionFlag[i]));
				checkClDevErrCode(err, "readBuffer FAILED\n");
			}
		}
		checkClDevErrCode(err, "pMicDeviceAgent->clDevCommandListExecute() or readBuffer FAILED\n");
		for (unsigned int i = 0; i < numOfNDRangeCommands; i++)
		{
			while (false == completionFlag[i]) 
			{ 
				sleep(1);
			}
		}
		for (unsigned int i = 0; i < numOfNDRangeCommands; i++)
		{
			if ((i % numOfContinuousNDRanges != 0) || (i == 0))
			{
				err = readBuffer(pMicDeviceAgent, commandList, false, memObj4[i], hostMemObj4[i], sizeof(unsigned int) * uiArg1, &(readCommandDesc[i]), &(readCompletionFlag[i]));
				checkClDevErrCode(err, "readBuffer FAILED\n");
			}
		}
		for (unsigned int i = 0; i < numOfNDRangeCommands; i++)
		{
			while (false == readCompletionFlag[i]) 
			{ 
				sleep(1);
			}
		}
		checkClDevErrCode(err, "readBuffer FAILED\n");
		testResult = true;
		for (unsigned int i = 0; ((i < numOfNDRangeCommands) && (testResult)); i++)
		{
			for (unsigned int j = 0; j < uiArg1; j++)
			{
				if (hostMemObj4[i][j] != hostMemObj2[j] + hostMemObj3[j] + i)
				{
					testResult = false;
					break;
				}
			}
		}
	}
	while (0);

	if (completionFlag)
	{
		free((void*)completionFlag);
	}
	if (readCompletionFlag)
	{
		free((void*)readCompletionFlag);
	}
	if (writeCommandDesc1.params)
	{
		free(writeCommandDesc1.params);
	}
	if (writeCommandDesc2.params)
	{
		free(writeCommandDesc2.params);
	}
	if (readCommandDesc)
	{
		for (unsigned int i = 0; i < numOfNDRangeCommands; i++)
		{
			if (readCommandDesc[i].params)
			{
				free(readCommandDesc[i].params);
			}
		}
		free(readCommandDesc);
	}
	if (memObj2)
	{
		memObj2->clDevMemObjRelease();
	}
	if (memObj3)
	{
		memObj3->clDevMemObjRelease();
	}
	if (memObj4)
	{
		for (unsigned int i = 0; i < numOfNDRangeCommands; i++)
		{
			memObj4[i]->clDevMemObjRelease();
		}
	}
	if (hostMemObj2)
	{
		free(hostMemObj2);
	}
	if (hostMemObj3)
	{
		free(hostMemObj3);
	}
	if (hostMemObj4)
	{
		for (unsigned int i = 0; i < numOfNDRangeCommands; i++)
		{
			if (hostMemObj4[i])
			{
				free(hostMemObj4[i]);
			}
		}
		free(hostMemObj4);
	}
	if (MyIOCLDevBackingStoreOfMemObj2)
	{
		delete(MyIOCLDevBackingStoreOfMemObj2);
	}
	if (MyIOCLDevBackingStoreOfMemObj3)
	{
		delete(MyIOCLDevBackingStoreOfMemObj3);
	}
	if (MyIOCLDevBackingStoreOfMemObj4)
	{
		for (unsigned int i = 0; i < numOfNDRangeCommands; i++)
		{
			if (MyIOCLDevBackingStoreOfMemObj4[i])
			{
				delete(MyIOCLDevBackingStoreOfMemObj4[i]);
			}
		}
		free(MyIOCLDevBackingStoreOfMemObj4);
	}
	if (rtMemObjService2)
	{
		delete(rtMemObjService2);
	}
	if (rtMemObjService3)
	{
		delete(rtMemObjService3);
	}
	if (rtMemObjService4)
	{
		for (unsigned int i = 0; i < numOfNDRangeCommands; i++)
		{
			if (rtMemObjService4[i])
			{
				delete(rtMemObjService4[i]);
			}
		}
		free(rtMemObjService4);
	}
	if (commandDescriptor)
	{
		free(commandDescriptor);
	}
	if (commandParams)
	{
		free(commandParams);
	}
	if (blob)
	{
		for (unsigned int i = 0; i < numOfNDRangeCommands; i++)
		{
			if (blob[i])
			{
				free(blob[i]);
			}
		}
		free(blob);
	}
	if (pBackendKernel)
	{
		delete (pBackendKernel);
	}
	if (commandList)
	{
		pMicDeviceAgent->clDevReleaseCommandList(commandList);
	}
	pMicDeviceAgent->clDevCloseDevice();
	delete (pFrameworkCallback);
	if (testResult)
	{
		printf("runNDRangeUnitTest - Pass\n");
	}
	else
	{
		printf("runNDRangeUnitTest - Fail\n");
	}
	return testResult;
}

















int main()
{
	runNDRangeUnitTest(500, 5);
	return 0;
}
