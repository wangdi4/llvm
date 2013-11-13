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
#include "cl_shared_ptr.hpp"

#include <source/COIEvent_source.h>
#include <source/COIPipeline_source.h>
#include <source/COIProcess_source.h>

namespace Intel { namespace OpenCL { namespace MICDevice {

class CommandList;
class CommandSynchHandler;

class Command : public NotificationPort::CallBack, public ReferenceCountedObject
{
public:

	PREPARE_SHARED_PTR(Command)

    /* Send the command for execution on the device (In case of NDRange command) or execute it on the host. */
    virtual cl_dev_err_code execute() = 0;

    // Call to releaseCommand in order to decrease the reference counter in order to delete the Command.
    void releaseCommand() { DecRefCnt(); }
    void retainCommand()  { IncRefCnt(); }

    const COIEVENT& getCommandCompletionEvent() { return m_endEvent.cmdEvent; };

    /* Hint to the next command about this command, For example if this command enqueued to COIPipe and the next command is going to enqueue to COIPipe than the next command can avoid COIEvent dependency. */
    virtual bool commandEnqueuedToPipe() = 0;

    virtual void fireCallBack(void* arg);

    virtual void eventProfilingCall(COI_NOTIFICATIONS& type);

    /* Register this command to COIContext if needed */
    void registerProfilingContext(bool mayReplaceByUserEvent = false);

    /* Unregister COIContext */
    static void unregisterProfilingContext() { COINotificationCallbackSetContext(NULL); };

    COIEVENT* registerBarrier(command_event_struct& completionBarrier);
    void      unregisterBarrier(command_event_struct& completionBarrier);

    void waitForCompletion() { while (!m_commandCompleted) { hw_pause(); } }
protected:

    /* Protected constructor because We like to create Commands only by the factory method */
    Command(CommandList* pCommandList, IOCLFrameworkCallbacks*	pFrameworkCallBacks, cl_dev_cmd_desc* pCmd);

    virtual ~Command();

    /* static function for Command creation */
    static inline cl_dev_err_code verifyCreation(Command* pInCommand, SharedPtr<Command>& ppOutCommand)
    {
        if (NULL == pInCommand)
        {
            return CL_DEV_OUT_OF_MEMORY;
        }
        ppOutCommand = pInCommand;
        return CL_DEV_SUCCESS;
    };

    /* Notify runtime that the command status changed */
    void notifyCommandStatusChanged(unsigned uStatus, cl_ulong timer = 0);

    /* Execute post commands dispatch operations (Such as Setting last dependent barrier, etc...).
        lastCmdWasExecution - True if the calling command is execution (NDRange) command.
        otherErr - flag that equal to True in case that there is error and m_lastError = CL_DEV_SUCCESS.
        In case of error which indicate by 'm_lastError' or 'otherErr' will notify the framework for completion. */
    cl_dev_err_code executePostDispatchProcess(bool lastCmdWasExecution, bool otherErr = false);

    command_event_struct        m_endEvent;

    // cl_dev_cmd_desc data structure.
    cl_dev_cmd_desc* m_pCmd;

    // Save the last error code
    cl_dev_err_code m_lastError;

    // Pointer to the Command list that create this Command
    CommandList* m_pCommandList;

    // Save the start running time of the command
    cl_ulong m_cmdRunningTime;

    // Save the completion time of the command
    cl_ulong m_cmdCompletionTime;

#ifdef ENABLE_MIC_TRACER
  // Command tracer
  CommandTracer m_commandTracer;
#endif

private:

    volatile bool    m_commandCompleted;
    
    void releaseResources();

    // Pointer for runtime callback object.
    IOCLFrameworkCallbacks*    m_pFrameworkCallBacks;
};

class FailureNotification : public Command
{

public:

	PREPARE_SHARED_PTR(Command)

	static SharedPtr<FailureNotification> Create(IOCLFrameworkCallbacks* pFrameworkCallBacks, cl_dev_cmd_desc* pCmd, cl_dev_err_code returnCode)
    {
        return new FailureNotification(pFrameworkCallBacks, pCmd, returnCode);
    }

	cl_dev_err_code execute();

	bool commandEnqueuedToPipe() { return false; };

protected:

    FailureNotification(IOCLFrameworkCallbacks* pFrameworkCallBacks, cl_dev_cmd_desc* pCmd, cl_dev_err_code returnCode);

};
}}}
