#include "command.h"
#include "command_list.h"
#include "cl_sys_info.h"
#include "mic_device.h"

using namespace Intel::OpenCL::MICDevice;

InOrderCommandSynchHandler*		CommandSynchHandler::m_singletonInOrderCommandSynchHandler = NULL;
OutOfOrderCommandSynchHandler*	CommandSynchHandler::m_singletonOuOfOrderCommandSynchHandler = NULL;

Command::Command(CommandList* pCommandList, IOCLFrameworkCallbacks* pFrameworkCallBacks, cl_dev_cmd_desc* pCmd) : NotificationPort::CallBack(), m_pCmd(pCmd), m_lastError(CL_DEV_SUCCESS), 
m_pCommandList(pCommandList), m_cmdRunningTime(0), m_cmdCompletionTime(0), m_pFrameworkCallBacks(pFrameworkCallBacks)
{
	// Set command id for the tracer.
	m_commandTracer.set_command_id((size_t)(pCmd->id));
	// Set start execution time for the tracer.
	m_commandTracer.set_current_time_command_host_time_start();

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
	m_pFrameworkCallBacks->clDevCmdStatusChanged(m_pCmd->id, 
                                                 m_pCmd->data, 
                                                 uStatus, 
                                                 CL_DEV_SUCCEEDED( m_lastError ) ? CL_SUCCESS : CL_ERR_EXECUTION_FAILED, 
                                                 timer);
}

cl_dev_err_code Command::executePostDispatchProcess(bool lastCmdWasExecution, bool otherErr)
{
	cl_dev_err_code err = m_lastError;

	if ((CL_DEV_SUCCESS == err) && (!otherErr))
    {
    	// Set m_completionBarrier.cmdEvent to be the last barrier in case of InOrder CommandList.
    	m_pCommandSynchHandler->setLastDependentBarrier(m_pCommandList, m_completionBarrier.cmdEvent, lastCmdWasExecution);
    	// Register m_completionBarrier.cmdEvent to NotificationPort
    	m_pCommandList->getNotificationPort()->addBarrier(m_completionBarrier.cmdEvent, this, NULL);
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
	// Set end coi execution time for the tracer. (Notification)
	m_commandTracer.set_current_time_coi_notify_command_time_end();
	// Notify runtime that  the command completed
	assert(m_pCmd->profiling == false || (m_cmdRunningTime > 0 && m_cmdCompletionTime > 0));
	notifyCommandStatusChanged(CL_RUNNING, m_cmdRunningTime);
	notifyCommandStatusChanged(CL_COMPLETE, m_cmdCompletionTime);
	// Delete this Command object
	delete this;
}

void Command::eventProfilingCall(COI_NOTIFICATIONS& type)
{
	switch (type)
	{
	case RUN_FUNCTION_START:
		assert(0 == m_cmdCompletionTime);
		// Set end coi execution time for the tracer. (COI RUNNING)
		m_commandTracer.set_current_time_coi_execution_time_start();
		if (m_pCmd->profiling) 
		{
			m_cmdRunningTime = HostTime();
		}
		break;
	case BUFFER_OPERATION_READY:
		// This case should be overwrite in Buffer command (In case of Buffer command only)
		break;
	case BUFFER_OPERATION_COMPLETE:
		// This case should be overwrite in Buffer command (In case of Buffer command only)
		break;
	case RUN_FUNCTION_COMPLETE:
	case USER_EVENT_SIGNALED:
		// Set end coi execution time for the tracer. (COI COMPLETED)
		m_commandTracer.set_current_time_coi_execution_time_end();
		if (m_pCmd->profiling) 
		{
			m_cmdCompletionTime = HostTime();
		}
		break;
	case RUN_FUNCTION_READY:
		break;
	default:
		assert(0 && "Unknow COI_NOTIFICATIONS type");
	};
}

void Command::releaseResources()
{
	// Set end execution time for the tracer.
	m_commandTracer.set_current_time_command_host_time_end();
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


//
// Helper class
//
class CommandSynchHandler::StaticInitializer
{
public:
    StaticInitializer() 
    {
        m_singletonInOrderCommandSynchHandler   = new InOrderCommandSynchHandler;
        m_singletonOuOfOrderCommandSynchHandler = new OutOfOrderCommandSynchHandler;
    };
};

CommandSynchHandler::StaticInitializer CommandSynchHandler::init_statics;

void InOrderCommandSynchHandler::getLastDependentBarrier(CommandList* pCommandList, COIEVENT** barrier, unsigned int* numDependencies, bool isExecutionTask)
{
	pCommandList->getLastDependentBarrier(barrier, numDependencies, isExecutionTask);
}

void InOrderCommandSynchHandler::setLastDependentBarrier(CommandList* pCommandList, COIEVENT barrier, bool lastCmdWasExecution)
{
	pCommandList->setLastDependentBarrier(barrier, lastCmdWasExecution);
}
