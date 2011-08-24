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



FailureNotification::FailureNotification(IOCLFrameworkCallbacks* pFrameworkCallBacks, cl_dev_cmd_desc* pCmd) : Command(NULL, pFrameworkCallBacks, pCmd)
{
}

cl_dev_err_code FailureNotification::execute()
{
    //TODO
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
