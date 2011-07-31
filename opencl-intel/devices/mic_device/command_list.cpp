#include "command_list.h"
#include "in_order_command_list.h"

#include <cstring>
#include <assert.h>

using namespace Intel::OpenCL::MICDevice;

CommandList::CommandList(NotificationPort* pNotificationPort, DeviceServiceCommunication* pDeviceServiceComm) : m_pNotificationPort(pNotificationPort),
    m_pDeviceServiceComm(pDeviceServiceComm), m_refCounter(1), m_pipe(NULL)
{
	// Init Commands array
	memset(m_vCommands, 0, sizeof(m_vCommands));
	m_vCommands[CL_DEV_CMD_READ] = NULL; //TODO &ReadWriteMemObject::Create;
	m_vCommands[CL_DEV_CMD_WRITE] = NULL; //TODO &ReadWriteMemObject::Create;
	m_vCommands[CL_DEV_CMD_EXEC_KERNEL] = &NDRange1::Create;
	m_vCommands[CL_DEV_CMD_EXEC_NATIVE] = NULL; //TODO &NativeFunction::Create;
	m_vCommands[CL_DEV_CMD_COPY] = NULL; //TODO &CopyMemObject::Create;
	m_vCommands[CL_DEV_CMD_MAP] = NULL; //TODO &MapMemObject::Create;
	m_vCommands[CL_DEV_CMD_UNMAP] = NULL; //TODO &UnmapMemObject::Create;
}

CommandList::~CommandList()
{
    assert(m_refCounter == 0 && "Deleting CommandList while reference counter is larger than 0");
}

cl_dev_err_code CommandList::commandListFactory(cl_dev_cmd_list_props IN props, NotificationPort* pNotificationPort,
	                                                DeviceServiceCommunication* pDeviceServiceComm, CommandList** outCommandList)
{
    cl_dev_err_code result = CL_DEV_SUCCESS;
    CommandList* tCommandList = NULL;
	// If out of order command list
	if (0 != ((int)props & (int)CL_DEV_LIST_ENABLE_OOO) )
	{
	    //TODO tCommandList = new OutOfOrderCommnadList(pNotificationPort, pDeviceServiceComm);
	}
	else  // In order command list
	{
	    tCommandList = new InOrderCommandList(pNotificationPort, pDeviceServiceComm);
	}
	if (NULL == tCommandList)
	{
	    return CL_DEV_OUT_OF_MEMORY;
	}
	// create new COIPipeline
	result = tCommandList->createPipeline();
	if (result != CL_DEV_SUCCESS)
	{
	    delete(tCommandList);
		return result;
	}

	*outCommandList = tCommandList;
	return result;
}

cl_dev_err_code CommandList::retainCommandList()
{
    unsigned int currVal = m_refCounter;
	while ((m_refCounter > 0) && (false == __sync_bool_compare_and_swap(&m_refCounter, currVal, currVal + 1)))
	{
	    currVal = m_refCounter;
		//TODO hwpause();
	}
	if (m_refCounter == 0)
	{
	    return CL_DEV_INVALID_OPERATION;
	}
	return CL_DEV_SUCCESS;
}

cl_dev_err_code CommandList::releaseCommandList(bool* outDelete)
{
    unsigned int currVal = m_refCounter;
	if (currVal == 0)
	{
	    return CL_DEV_INVALID_OPERATION;
	}
	while (false == __sync_bool_compare_and_swap(&m_refCounter, currVal, currVal - 1))
	{
		if (m_refCounter == 0)
		{
			return CL_DEV_INVALID_OPERATION;
		}
	    currVal = m_refCounter;
		//TODO hwpause();
	}
	*outDelete = (m_refCounter == 0);
	return CL_DEV_SUCCESS;
}



void CommandList::commandListWaitCompletion()
{
    //TODO
}

cl_dev_err_code CommandList::createCommandObject(cl_dev_cmd_desc* cmd, Command** cmdObject)
{
    assert(cmd && "cmd is NULL object");
	assert(cmd->type < CL_DEV_CMD_MAX_COMMAND_TYPE  && "INVALID CL_DEV_CMD_TYPE");
	// get function pointer to cmd->type factory function
	fnCommandCreate_t* fnCreate = m_vCommands[cmd->type];
	assert( (NULL != fnCreate) && "Not implemented");
	Command* pCmdObject;
	// create appropriate command object
	cl_dev_err_code	rc = fnCreate(isInOrderCommandList(), &pCmdObject);
    // if failed create FailureNotification command object
	if (CL_DEV_FAILED(rc))
	{
	    pCmdObject = new FailureNotification();
		if (NULL == pCmdObject)
		{
		    rc = CL_DEV_OUT_OF_MEMORY;
		}
	}
	*cmdObject = pCmdObject;
	return rc;
}

cl_dev_err_code CommandList::createPipeline()
{
    COIRESULT result = COI_ERROR;
    result = COIPipelineCreate(m_pDeviceServiceComm->getDeviceProcessHandle(), NULL, NULL, &m_pipe);
    if (result != COI_SUCCESS)
	{
	    return CL_DEV_ERROR_FAIL;
	}
    return CL_DEV_SUCCESS;
}

