#include "command.h"
#include "command_list.h"
#include "cl_sys_info.h"

using namespace Intel::OpenCL::MICDevice;

Command::Command(CommandList* pCommandList, IOCLFrameworkCallbacks* pFrameworkCallBacks, cl_dev_cmd_desc* pCmd) : NotificationPort::CallBack(), m_pCmd(pCmd), m_lastError(CL_DEV_SUCCESS), 
m_pCommandList(pCommandList), m_pFrameworkCallBacks(pFrameworkCallBacks)
{
	m_pCommandSynchHandler = NULL;
	// Create new CommandSynchHandler according to Queue type.
	m_pCommandSynchHandler = ((pCommandList) && (pCommandList->isInOrderCommandList())) ? (CommandSynchHandler*)(new InOrderCommandSynchHandler(pCommandList)) : (CommandSynchHandler*)(new OutOfOrderCommandSynchHandler(pCommandList));
}

Command::~Command()
{
	releaseResources();
}

void Command::notifyCommandStatusChanged(unsigned uStatus, cl_ulong timer)
{
	m_pFrameworkCallBacks->clDevCmdStatusChanged(m_pCmd->id, m_pCmd->data, uStatus, m_lastError, timer);
}

void Command::notifyCommandStatusChanged(unsigned uStatus)
{
	cl_ulong timer = 0;
	// If the user ask for profiling time
	if(m_pCmd->profiling)
	{
		timer = HostTime();
	}
	// Notify runtime that  the command completed
	notifyCommandStatusChanged(CL_COMPLETE, timer);
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
	if (m_pCommandSynchHandler)
	{
		// Unregister the completion barrier
		m_pCommandSynchHandler->unregisterCompletionBarrier(m_completionBarrier);

		delete(m_pCommandSynchHandler);
	}
}



FailureNotification::FailureNotification(IOCLFrameworkCallbacks* pFrameworkCallBacks, cl_dev_cmd_desc* pCmd) : Command(NULL, pFrameworkCallBacks, pCmd)
{
}

cl_dev_err_code FailureNotification::execute()
{
    //TODO
    return CL_DEV_SUCCESS;
}



InOrderCommandSynchHandler::InOrderCommandSynchHandler(CommandList* pCommandList)  : CommandSynchHandler(pCommandList) 
{
}

void InOrderCommandSynchHandler::getLastDependentBarrier(COIEVENT** barrier, unsigned int* numDependencies, bool isExecutionTask)
{
	m_pCommandList->getLastDependentBarrier(barrier, numDependencies, isExecutionTask);
}

void InOrderCommandSynchHandler::setLastDependentBarrier(COIEVENT barrier, bool lastCmdWasExecution)
{
	m_pCommandList->setLastDependentBarrier(barrier, lastCmdWasExecution);
}
