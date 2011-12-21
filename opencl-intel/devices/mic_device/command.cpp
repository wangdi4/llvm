#include "command.h"
#include "command_list.h"
#include "cl_sys_info.h"

using namespace Intel::OpenCL::MICDevice;

InOrderCommandSynchHandler		CommandSynchHandler::m_singletonInOrderCommandSynchHandler;
OutOfOrderCommandSynchHandler	CommandSynchHandler::m_singletonOuOfOrderCommandSynchHandler;

Command::Command(CommandList* pCommandList, IOCLFrameworkCallbacks* pFrameworkCallBacks, cl_dev_cmd_desc* pCmd) : NotificationPort::CallBack(), m_pCmd(pCmd), m_lastError(CL_DEV_SUCCESS), 
m_pCommandList(pCommandList), m_pFrameworkCallBacks(pFrameworkCallBacks)
{
	m_pCommandSynchHandler = NULL;
	// Get CommandSynchHandler singleton according to Queue type.
	m_pCommandSynchHandler = CommandSynchHandler::getCommandSyncHandler(pCommandList->isInOrderCommandList());
	assert(m_pCommandSynchHandler);
}

Command::~Command()
{
	releaseResources();
}

void Command::notifyCommandStatusChanged(unsigned uStatus, cl_ulong timer)
{
	if ((timer == 0) && (m_pCmd->profiling))
	{
		timer = HostTime();
	}
	m_pFrameworkCallBacks->clDevCmdStatusChanged(m_pCmd->id, m_pCmd->data, uStatus, m_lastError, timer);
}

cl_dev_err_code Command::executePostDispatchProcess(bool lastCmdWasExecution, bool otherErr)
{
	cl_dev_err_code err = m_lastError;

	if ((CL_DEV_SUCCESS == err) && (!otherErr))
    {
    	// Set m_completionBarrier to be the last barrier in case of InOrder CommandList.
    	m_pCommandSynchHandler->setLastDependentBarrier(m_pCommandList, m_completionBarrier, lastCmdWasExecution);
    	// Register m_completionBarrier to NotificationPort
    	m_pCommandList->getNotificationPort()->addBarrier(m_completionBarrier, this, NULL);
    }
    else
    {
        // error path
    	notifyCommandStatusChanged(CL_COMPLETE);
    	delete this;
    }

	return err;
}

void Command::fireCallBack(void* arg)
{
	// Notify runtime that  the command completed
	notifyCommandStatusChanged(CL_COMPLETE);
	// Delete this Command object
	delete this;
}

void Command::releaseResources()
{
	// Delete Synchronization handler
	assert(m_pCommandSynchHandler);
	// Unregister the completion barrier
	m_pCommandSynchHandler->unregisterCompletionBarrier(m_completionBarrier);
}



FailureNotification::FailureNotification(IOCLFrameworkCallbacks* pFrameworkCallBacks, cl_dev_cmd_desc* pCmd, cl_dev_err_code returnCode) : Command(NULL, pFrameworkCallBacks, pCmd)
{
	m_lastError = returnCode;
}

cl_dev_err_code FailureNotification::execute()
{
	COIEVENT* barrier = NULL;
	unsigned int numDependecies = 0;
	m_pCommandSynchHandler->getLastDependentBarrier(m_pCommandList, &barrier, &numDependecies, false);
	// If OOO or first command fireCallBack in order to complete, otherwise add the last dependent barrier as my callback.
	if (0 == numDependecies)
	{
		fireCallBack(NULL);
	}
	else
	{
		m_pCommandList->getNotificationPort()->addBarrier(*barrier, this, NULL);
	}
    return CL_DEV_SUCCESS;
}



void InOrderCommandSynchHandler::getLastDependentBarrier(CommandList* pCommandList, COIEVENT** barrier, unsigned int* numDependencies, bool isExecutionTask)
{
	pCommandList->getLastDependentBarrier(barrier, numDependencies, isExecutionTask);
}

void InOrderCommandSynchHandler::setLastDependentBarrier(CommandList* pCommandList, COIEVENT barrier, bool lastCmdWasExecution)
{
	pCommandList->setLastDependentBarrier(barrier, lastCmdWasExecution);
}
