// Copyright (c) 2006-2008 Intel Corporation
// All rights reserved.
//
// WARRANTY DISCLAIMER
//
// THESE MATERIALS ARE PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL INTEL OR ITS
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THESE
// MATERIALS, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Intel Corporation is the author of the Materials, and requests that all
// problem reports or change requests be submitted to it directly

#pragma once

#include "notification_port.h"
#include "mic_device_interface.h"
#include "mic_tracer.h"

#include <source/COIEvent_source.h>
#include <source/COIPipeline_source.h>
#include <source/COIProcess_source.h>

namespace Intel { namespace OpenCL { namespace MICDevice {

class CommandList;
class CommandSynchHandler;

class Command : public NotificationPort::CallBack
{

public:

	struct command_event_struct
	{
		command_event_struct() : isRegister(false) {};

		COIEVENT cmdEvent;
		bool isRegister;
	};

	/* Send the command for execution on the device (In case of NDRange command) or execute it on the host. */
    virtual cl_dev_err_code execute() = 0;

	virtual void fireCallBack(void* arg);

	virtual void eventProfilingCall(COI_NOTIFICATIONS& type);

protected:

	/* Protected constructor because We like to create Commands only by the factory method */
    Command(CommandList* pCommandList, IOCLFrameworkCallbacks*	pFrameworkCallBacks, cl_dev_cmd_desc* pCmd);

	virtual ~Command();

	/* static function for Command creation */
	static inline cl_dev_err_code verifyCreation(Command* pInCommand, Command** ppOutCommand) 
	{
		if (NULL == pInCommand)
		{
			return CL_DEV_OUT_OF_MEMORY;
		}
		*ppOutCommand = pInCommand;
		return CL_DEV_SUCCESS;
	};

	/* Notify runtime that the command status changed */
	void notifyCommandStatusChanged(unsigned uStatus, cl_ulong timer = 0);

	/* Execute post commands dispatch operations (Such as Setting last dependent barrier, etc...).
	   lastCmdWasExecution - True if the calling command is execution (NDRange) command.
	   otherErr - flag that equal to True in case that there is error and m_lastError = CL_DEV_SUCCESS. 
	   In case of error which indicate by 'm_lastError' or 'otherErr' will notify the framework for completion and delete this object. */
	cl_dev_err_code executePostDispatchProcess(bool lastCmdWasExecution, bool otherErr = false);

	// Contains COIEVENT that will signal when the Command will finish.
    command_event_struct m_completionBarrier;

	// cl_dev_cmd_desc data structure.
	cl_dev_cmd_desc* m_pCmd;

	// Save the last error code
	cl_dev_err_code m_lastError;

	// Pointer to the Command list that create this Command
	CommandList* m_pCommandList;

	// Synchronization handler for the command according to the queue type.
	CommandSynchHandler* m_pCommandSynchHandler;

	// Save the start running time of the command
	cl_ulong m_cmdRunningTime;

	// Save the completion time of the command
	cl_ulong m_cmdCompletionTime;

	// Command tracer
	CommandTracer m_commandTracer;

private:
	
	void releaseResources();

	// Pointer for runtime callback object.
	IOCLFrameworkCallbacks*	m_pFrameworkCallBacks;

};



class FailureNotification : public Command
{

public:

    FailureNotification(IOCLFrameworkCallbacks* pFrameworkCallBacks, cl_dev_cmd_desc* pCmd, cl_dev_err_code returnCode);

	cl_dev_err_code execute();

};



class InOrderCommandSynchHandler;
class OutOfOrderCommandSynchHandler;

class CommandSynchHandler
{
	
public:
	/* Return the appropriate singleton object */
	static CommandSynchHandler* getCommandSyncHandler(bool isInOrder) 
	{ 
		return (isInOrder == true) ? (CommandSynchHandler*)(m_singletonInOrderCommandSynchHandler) : (CommandSynchHandler*)(m_singletonOuOfOrderCommandSynchHandler) ;
	};
	/* Return true if the CommandList is In order */
	virtual bool isInOrderType() = 0;

	/* In case of InOrder CommandList get the last dependent barrier and the amount of dependent barriers.
	   In case of OutOfOrder CommandList set *barrier to NULL and the numDependencies to 0 because there is no dependency. */
	virtual void getLastDependentBarrier(CommandList* pCommandList, COIEVENT** barrier, unsigned int* numDependencies, bool isExecutionTask) = 0;

	/* Set the last dependent barrier to be barrier and in case of InOrder CommandList set if it was NDRange command (For optimization) */
	virtual void setLastDependentBarrier(CommandList* pCommandList, COIEVENT barrier, bool lastCmdWasExecution) = 0;

	/* In case of InOrder CommandList just return completionBarrier. (Because the COIPipelineRunFunction will register it).
	   In case of OutOfOrder CommandList it will COIBarrierRegisterUserBarrier on completionBarrier and return NULL. 
	   (Because We don't want to register the barrier when the COIPipelineRunFunction returns) */
	virtual COIEVENT* registerCompletionBarrier(Command::command_event_struct& completionBarrier, Command* pCommand) = 0;

	/* In case of InOrder CommandList do nothing.
	   In case of OutOfOrder CommandList unregister the COIEVENT. */
	virtual void unregisterCompletionBarrier(Command::command_event_struct& completionBarrier) = 0;

private:

	static InOrderCommandSynchHandler*		m_singletonInOrderCommandSynchHandler;
	static OutOfOrderCommandSynchHandler*	m_singletonOuOfOrderCommandSynchHandler;

	class StaticInitializer;
    static StaticInitializer init_statics;
};


class InOrderCommandSynchHandler : public CommandSynchHandler
{

public:

	bool isInOrderType() { return true; };

	void getLastDependentBarrier(CommandList* pCommandList, COIEVENT** barrier, unsigned int* numDependencies, bool isExecutionTask);

	void setLastDependentBarrier(CommandList* pCommandList, COIEVENT barrier, bool lastCmdWasExecution);

	COIEVENT* registerCompletionBarrier(Command::command_event_struct& completionBarrier, Command* pCommand) { return &(completionBarrier.cmdEvent); };

	void unregisterCompletionBarrier(Command::command_event_struct& completionBarrier) { return; };
};


class OutOfOrderCommandSynchHandler : public CommandSynchHandler
{

public:

	OutOfOrderCommandSynchHandler() : CommandSynchHandler() {};

	bool isInOrderType() { return false; };

	void getLastDependentBarrier(CommandList* pCommandList, COIEVENT** barrier, unsigned int* numDependencies, bool isExecutionTask) { *barrier = NULL, *numDependencies = 0; };

	void setLastDependentBarrier(CommandList* pCommandList, COIEVENT barrier, bool lastCmdWasExecution) { return; };

	COIEVENT* registerCompletionBarrier(Command::command_event_struct& completionBarrier, Command* pCommand) 
	{ 
		// If not register yet
		if (false == completionBarrier.isRegister)
		{
			COINotificationCallbackSetContext(pCommand);
			COIEventRegisterUserEvent(&(completionBarrier.cmdEvent));
			COINotificationCallbackSetContext(NULL);
			completionBarrier.isRegister = true;
		}
		return NULL;
	};

	void unregisterCompletionBarrier(Command::command_event_struct& completionBarrier) 
	{ 
		// If already registered
		if (completionBarrier.isRegister)
		{
			COIEventUnregisterUserEvent(completionBarrier.cmdEvent);
			completionBarrier.isRegister = false;
		}
	};
};

}}}
