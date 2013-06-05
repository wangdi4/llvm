#include "command_list.h"
#include "execution_commands.h"
#include "buffer_commands.h"
#include "hw_utils.h"
#include "memory_allocator.h"

#include <source/COIEvent_source.h>

#include <cstring>
#include <assert.h>

using namespace Intel::OpenCL::MICDevice;

extern bool gSafeReleaseOfCoiObjects;

CommandList::fnCommandCreate_t* CommandList::m_vCommands[CL_DEV_CMD_MAX_COMMAND_TYPE] = {
    /* CL_DEV_CMD_INVALID      */ NULL,
	/* CL_DEV_CMD_READ         */ &ReadWriteMemObject::Create,
	/* CL_DEV_CMD_WRITE        */ &ReadWriteMemObject::Create,
	/* CL_DEV_CMD_COPY         */ &CopyMemObject::Create,
	/* CL_DEV_CMD_MAP          */ &MapMemObject::Create,
	/* CL_DEV_CMD_UNMAP        */ &UnmapMemObject::Create,
	/* CL_DEV_CMD_EXEC_KERNEL  */ &NDRange::Create,
	/* CL_DEV_CMD_EXEC_TASK    */ NULL,
	/* CL_DEV_CMD_EXEC_NATIVE  */ NULL,
	/* CL_DEV_CMD_FILL_BUFFER  */ &FillMemObject::Create,
	/* CL_DEV_CMD_FILL_IMAGE   */ &FillMemObject::Create,
	/* CL_DEV_CMD_MIGRATE      */ &MigrateMemObject::Create
};


CommandList::CommandList(const SharedPtr<NotificationPort>& pNotificationPort, 
                         DeviceServiceCommunication* pDeviceServiceComm, 
                         IOCLFrameworkCallbacks* pFrameworkCallBacks, 
                         ProgramService* pProgramService, 
                         PerformanceDataStore* pOverheadData,
                         cl_dev_subdevice_id subDeviceId,
						 bool isInOrder) :
m_validBarrier(false), m_pNotificationPort(pNotificationPort), m_pDeviceServiceComm(pDeviceServiceComm), m_pFrameworkCallBacks(pFrameworkCallBacks), 
m_pProgramService(pProgramService), m_pipe(NULL), m_subDeviceId(subDeviceId), m_pOverhead_data(pOverheadData), m_lastCommand(NULL), m_isInOrderQueue(isInOrder)
{
	m_refCounter.exchange(1);
#ifdef _DEBUG
	m_numOfConcurrentExecutions.exchange(0);
#endif
}

CommandList::~CommandList()
{
//    assert(m_refCounter == 0 && "Deleting CommandList while reference counter is larger than 0");
	if ((gSafeReleaseOfCoiObjects) && (m_pipe))
	{
		cl_dev_err_code err = CL_DEV_SUCCESS;
		err = releaseCommandListOnDevice();
		assert(CL_DEV_SUCCESS == err);
		COIRESULT result = COI_SUCCESS;
	    result = COIPipelineDestroy(m_pipe);
		assert(result == COI_SUCCESS && "COIPipelineDestroy failed");
	}
}

cl_dev_err_code CommandList::commandListFactory(cl_dev_cmd_list_props IN        props, 
                                                cl_dev_subdevice_id             subDeviceId, 
                                                const SharedPtr<NotificationPort>&    pNotificationPort, 
                                                DeviceServiceCommunication*     pDeviceServiceComm,
												IOCLFrameworkCallbacks*         pFrameworkCallBacks, 
												ProgramService*                 pProgramService, 
												PerformanceDataStore*           pOverheadData,
												CommandList**                   outCommandList)
{
    cl_dev_err_code result = CL_DEV_SUCCESS;
    CommandList* tCommandList = NULL;
	// If out of order command list
	if (0 != ((int)props & (int)CL_DEV_LIST_ENABLE_OOO) )
	{
	    tCommandList = new CommandList(pNotificationPort, pDeviceServiceComm, 
                                                 pFrameworkCallBacks, pProgramService, 
                                                 pOverheadData, subDeviceId, false);
	}
	else  // In order command list
	{
	    tCommandList = new CommandList(pNotificationPort, pDeviceServiceComm, 
                                              pFrameworkCallBacks, pProgramService, 
                                              pOverheadData, subDeviceId, true);
	}
	if (NULL == tCommandList)
	{
	    return CL_DEV_OUT_OF_MEMORY;
	}
	// create new COIPipeline
	result = tCommandList->createPipeline();
	if (result != CL_DEV_SUCCESS)
	{
		bool tempDeleteFlag = false;
		tCommandList->releaseCommandList(&tempDeleteFlag);
		assert(tempDeleteFlag);
	    delete(tCommandList);
		return result;
	}
	// init command list on device side.
	result = tCommandList->initCommandListOnDevice();
	if (result != CL_DEV_SUCCESS)
	{
		bool tempDeleteFlag = false;
		tCommandList->releaseCommandList(&tempDeleteFlag);
		assert(tempDeleteFlag);
		delete(tCommandList);
		return result;
	}

	*outCommandList = tCommandList;
	return result;
}

cl_dev_err_code CommandList::retainCommandList()
{
	long prevVal = m_refCounter ++;
	if (prevVal == 0)
	{
		return CL_DEV_INVALID_OPERATION;
	}
	return CL_DEV_SUCCESS;
}

cl_dev_err_code CommandList::releaseCommandList(bool* outDelete)
{
	long newVal = -- m_refCounter;
	if (newVal < 0)
	{
		return CL_DEV_INVALID_OPERATION;
	}
	*outDelete = (newVal == 0);
	return CL_DEV_SUCCESS;
}

cl_dev_err_code CommandList::commandListExecute(cl_dev_cmd_desc* IN *cmds, cl_uint IN count)
{
#ifdef _DEBUG
	long oldVal = m_numOfConcurrentExecutions++;
	assert(oldVal == 0);
#endif

	cl_dev_err_code rc = CL_DEV_SUCCESS;
    SharedPtr<Command> pCmdObject;
	// run over all the cmds
    for (unsigned int i = 0; i < count; i++)
	{
	    // Create appropriate Command object, After createCommandObject() complete, Do not change pCmdObject until completion of pCmdObject->execute()!
		rc = createCommandObject(cmds[i], pCmdObject);
		// If there is no enough memory for allocating Command object
		if (CL_DEV_FAILED(rc))
		{
			break;
		}
		// Send the command for execution. pCmdObject will delete itself.
		rc = pCmdObject->execute();
		if (CL_DEV_FAILED(rc))
		{
			break;
		}
		// Save the new Command pointer in device agent data. ASSUME that calling to clDevReleaseCommand in order to delete this Command!
		cmds[i]->device_agent_data = pCmdObject.GetPtr();
		// Incrementing the reference counter because it is a temporary shared pointer instance. (otherwise the Command object can delete before it complete its execution)
		pCmdObject.IncRefCnt();
		// In order to decrease the reference count of the last command.
		pCmdObject = NULL;
	}
#ifdef _DEBUG
	oldVal = m_numOfConcurrentExecutions--;
	assert(oldVal == 1);
#endif
	return rc;
}

void CommandList::commandListWaitCompletion(cl_dev_cmd_desc* cmdDescToWait)
{
	SharedPtr<Command> waitToCmd = NULL;
	// Assume that if cmdDescToWait->device_agent_data != NULL than the Command object exist.
	if ((cmdDescToWait) && (cmdDescToWait->device_agent_data))
	{
		waitToCmd = (Command*)(cmdDescToWait->device_agent_data);
	}
	else
	{
		// Wait to last Command (if didn't finish yet)
		waitToCmd = getLastCommand();
	}
	if (NULL != waitToCmd)
	{
		COIRESULT err = COI_SUCCESS;
		err = COIEventWait(1, &(waitToCmd->getCommandCompletionEvent()), -1, true, NULL, NULL);
		assert(COI_SUCCESS == err);
	}	
}

cl_dev_err_code CommandList::createCommandObject(cl_dev_cmd_desc* cmd, SharedPtr<Command>& cmdObject)
{
	if ((NULL == cmd) || (cmd->type >= CL_DEV_CMD_MAX_COMMAND_TYPE))
	{
		return CL_DEV_INVALID_VALUE;
	}
	// get function pointer to cmd->type factory function
	fnCommandCreate_t* fnCreate = m_vCommands[cmd->type];
	assert( (NULL != fnCreate) && "Not implemented");
	SharedPtr<Command> pCmdObject;
	// create appropriate command object. In order to delete it call to clDevReleaseCommand()
	cl_dev_err_code	rc = fnCreate(this, m_pFrameworkCallBacks, cmd, pCmdObject);
    // if failed create FailureNotification command object
	if (CL_DEV_FAILED(rc))
	{
	    pCmdObject = FailureNotification::Create(m_pFrameworkCallBacks, cmd, rc);
		if (NULL == pCmdObject)
		{
		    rc = CL_DEV_OUT_OF_MEMORY;
		}
	}
	cmdObject = pCmdObject;
	return rc;
}

cl_dev_err_code CommandList::createPipeline()
{
    COIRESULT result = COI_ERROR;
	// Create COIPipeline for this queue.
    result = COIPipelineCreate(m_pDeviceServiceComm->getDeviceProcessHandle(),        // A handle to an already existing process
		                       NULL,                                                  // An optional mask of the set of hardware threads
							   NULL,                                                  // StackSize
							   &m_pipe);                                              // Handle returned to uniquely identify the pipeline
    if (result != COI_SUCCESS)
	{
	    return CL_DEV_ERROR_FAIL;
	}
    return CL_DEV_SUCCESS;
}

cl_dev_err_code CommandList::initCommandListOnDevice()
{
    INIT_QUEUE_ON_DEVICE_STRUCT data;
    data.is_in_order_queue = isInOrderCommandList();
    
	return runBlockingFuncOnDevice(DeviceServiceCommunication::INIT_COMMANDS_QUEUE,
                                   &data, sizeof(data));
}

cl_dev_err_code CommandList::releaseCommandListOnDevice()
{
	return runBlockingFuncOnDevice(DeviceServiceCommunication::RELEASE_COMMANDS_QUEUE);
}

cl_dev_err_code CommandList::runBlockingFuncOnDevice(DeviceServiceCommunication::DEVICE_SIDE_FUNCTION func,
                                                     void* in_data, size_t in_data_size )
{
    assert( ((NULL != in_data) && (0 != in_data_size)) || ((NULL == in_data) && (0 == in_data_size)) );

    if (NULL == in_data)
    {
        in_data_size = 0;
    }
    else if (0 == in_data_size)
    {
        in_data = NULL;
    }

    
    
	// Run func on device with no dependencies, assign a barrier in order to wait until the function execution complete.
    bool ok = runQueueServiceFunction( func,
                                       in_data_size, in_data,
                                       0, NULL,
                                       0, NULL,NULL );

    return (ok) ? CL_DEV_SUCCESS : CL_DEV_ERROR_FAIL;
}

SharedPtr<Command> CommandList::getLastCommand()
{
	m_lastCommandMutex.Lock();
	SharedPtr<Command> retCommand = m_lastCommand;
	m_lastCommandMutex.Unlock();
	return retCommand;
}

void CommandList::setLastCommand(const SharedPtr<Command>& newCommand, const SharedPtr<Command>& oldCommand)
{
	assert(((NULL != newCommand) || (NULL != oldCommand)) && "If NULL == newCommand than u must supply valid oldCommand");
	m_lastCommandMutex.Lock();
	if ((oldCommand == m_lastCommand) && (NULL == newCommand))
	{
		m_lastCommand = NULL;
	}
	else if (NULL != newCommand)
	{
		m_lastCommand = newCommand;
	}
	m_lastCommandMutex.Unlock();
}
