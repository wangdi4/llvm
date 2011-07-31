#include "in_order_command_list.h"

using namespace Intel::OpenCL::MICDevice;

InOrderCommandList::InOrderCommandList(NotificationPort* pNotificationPort, DeviceServiceCommunication* pDeviceServiceComm) :
    CommandList(pNotificationPort, pDeviceServiceComm), m_lastCommandWasNDRange(false), m_validBarrier(false)
{
}

InOrderCommandList::~InOrderCommandList()
{
}

cl_dev_err_code InOrderCommandList::commandListExecute(cl_dev_cmd_desc* IN *cmds, cl_uint IN count)
{
    Command* pCmdObject;
	// run over all the cmds
    for (unsigned int i = 0; i < count; i++)
	{
	    // Create appropriate Command object
		cl_dev_err_code rc = createCommandObject(cmds[i], &pCmdObject);
		// If there is no enough memory for allocating Command object
		if (CL_DEV_FAILED(rc))
		{
			return rc;
		}
		rc = pCmdObject->execute(this);
		if (CL_DEV_FAILED(rc))
		{
			return rc;
		}
	}
	return CL_DEV_SUCCESS;
}


void InOrderCommandList::getLastDependentBarrier(COIBARRIER** barrier, unsigned int* numDependencies, bool isExecutionTask)
{
    *numDependencies = 1;
    COIBARRIER* tBarrier = &m_lastDependentBarrier;
	// if last command was ExecutionTask and the current command is ExecutionTask or it is NOT the first execution (validBarrier = true) than perform the operation without explicit barrier
	if (((isExecutionTask) && (m_lastCommandWasNDRange)) || (false == m_validBarrier))
	{
	    tBarrier = NULL;
		*numDependencies = 0;
	}
	*barrier = tBarrier;
}

void InOrderCommandList::setLastDependentBarrier(COIBARRIER barrier, bool lastCmdWasExecution)
{
    m_lastDependentBarrier = barrier;
	m_lastCommandWasNDRange = lastCmdWasExecution;
	m_validBarrier = true;
}

