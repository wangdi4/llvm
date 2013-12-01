// Copyright (c) 2006-2013 Intel Corporation
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

#include "command.h"
#include "command_list.h"
#include "cl_sys_info.h"
#include "mic_device.h"

using namespace Intel::OpenCL::MICDevice;

Command::Command(CommandList* pCommandList, IOCLFrameworkCallbacks* pFrameworkCallBacks, cl_dev_cmd_desc* pCmd) :
    NotificationPort::CallBack(), m_pCmd(pCmd), m_lastError(CL_DEV_SUCCESS), 
    m_pCommandList(pCommandList), m_cmdRunningTime(0), m_cmdCompletionTime(0),
    m_commandCompleted(false), m_pFrameworkCallBacks(pFrameworkCallBacks)
{
#ifdef ENABLE_MIC_TRACER
  // Set command id for the tracer.
    m_commandTracer.set_command_id((size_t)(pCmd->id));
    // Set start execution time for the tracer.
    m_commandTracer.set_current_time_command_host_time_start();
#endif
}

Command::~Command()
{
    assert(m_commandCompleted == true && "CRITICAL ERROR Trying to delete Command object before the command execution completed");
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
#if defined(USE_ITT) && defined(USE_ITT_INTERNAL)
    if ( (NULL != m_pCommandList->GetGPAInfo()) && m_pCommandList->GetGPAInfo()->bUseGPA )
    {
      static __thread __itt_string_handle* pTaskName = NULL;
      if ( NULL == pTaskName )
      {
        pTaskName = __itt_string_handle_create("Command::executePostDispatchProcess");
      }
      __itt_task_begin(m_pCommandList->GetGPAInfo()->pDeviceDomain, __itt_null, __itt_null, pTaskName);
    }
#endif

    cl_dev_err_code err = m_lastError;

    if ((CL_DEV_SUCCESS == err) && (!otherErr))
    {
        // Set this Command as the last command in the queue.
        m_pCommandList->setLastCommand(this);
        // Register m_completionBarrier.cmdEvent to NotificationPort
        m_pCommandList->getNotificationPort()->addBarrier(m_endEvent.cmdEvent, this, NULL);
    }
    else
    {
          // error path
        notifyCommandStatusChanged(CL_COMPLETE);
    }

#if defined(USE_ITT) && defined(USE_ITT_INTERNAL)
    if ( (NULL != m_pCommandList->GetGPAInfo()) && m_pCommandList->GetGPAInfo()->bUseGPA )
    {
      __itt_task_end(m_pCommandList->GetGPAInfo()->pDeviceDomain);
    }
#endif

    return err;
}

void Command::registerProfilingContext(bool mayReplaceByUserEvent)
{
    // If in order queue or it is command that does not control by user events than set this command as the context otherwise set NULL as the context.
    ((m_pCommandList->isInOrderCommandList()) || (false == mayReplaceByUserEvent)) ? COINotificationCallbackSetContext(this) : COINotificationCallbackSetContext(NULL);
}

void Command::fireCallBack(void* arg)
{
#ifdef ENABLE_MIC_TRACER
    // Set end coi execution time for the tracer. (Notification)
    m_commandTracer.set_current_time_coi_notify_command_time_end();
#endif
    // Remove myself from last command in command list (in case that this command is last command)
    m_pCommandList->resetLastCommand(this);

    // Notify runtime that  the command completed
    assert( (m_pCmd->profiling == false || (m_cmdRunningTime > 0 && m_cmdCompletionTime > 0)) && "When profiling is ON, both RUNNING and COMPLETED must be set");
    notifyCommandStatusChanged(CL_RUNNING, m_cmdRunningTime);

    // Complete notification may delete this Command. Ensure it will not be deleted before a time
    retainCommand();

    notifyCommandStatusChanged(CL_COMPLETE, m_cmdCompletionTime);

    // mark itself as completed
    m_commandCompleted = true;

    // now Command may be deleted
    releaseCommand();
}

void Command::eventProfilingCall(COI_NOTIFICATIONS& type)
{
    switch (type)
    {
    case RUN_FUNCTION_START:
#if defined(USE_ITT) && defined(USE_ITT_INTERNAL)
        if ( (NULL != m_pCommandList->GetGPAInfo()) && m_pCommandList->GetGPAInfo()->bUseGPA )
        {
          static __thread __itt_string_handle* pTaskName = NULL;
          if ( NULL == pTaskName )
          {
            pTaskName = __itt_string_handle_create("Command::eventProfilingCall(RUN_FUNCTION_START)");
          }
          __itt_task_begin(m_pCommandList->GetGPAInfo()->pDeviceDomain, __itt_null, __itt_null, pTaskName);
        }
#endif
        assert(0 == m_cmdCompletionTime && "On function start, COMPLETION time must be 0");
#ifdef ENABLE_MIC_TRACER
        // Set end coi execution time for the tracer. (COI RUNNING)
        m_commandTracer.set_current_time_coi_execution_time_start();
#endif
        if (m_pCmd->profiling) 
        {
            m_cmdRunningTime = HostTime();
        }
#if defined(USE_ITT) && defined(USE_ITT_INTERNAL)
        if ( (NULL != m_pCommandList->GetGPAInfo()) && m_pCommandList->GetGPAInfo()->bUseGPA )
        {
          __itt_task_end(m_pCommandList->GetGPAInfo()->pDeviceDomain);
        }
#endif
        break;
    case BUFFER_OPERATION_READY:
        // This case should be overwrite in Buffer command (In case of Buffer command only)
        break;
    case BUFFER_OPERATION_COMPLETE:
        // This case should be overwrite in Buffer command (In case of Buffer command only)
        break;
    case USER_EVENT_SIGNALED:
        // If profiling is enabled and RUNNING time is not set, need to update
        // if profiling is not enabled, probably it's OOO command, so this section should be skipped to RUN_FUNCTION_COMPLETED.
#ifdef ENABLE_MIC_TRACER
        if (0 == m_cmdRunningTime )
#else
        if ( m_pCmd->profiling && (0 == m_cmdRunningTime) )
#endif
        {
            assert(0 == m_cmdCompletionTime && "When proiling is enabled and RUNNING == 0, COMPLETED time must be not set");
#if defined(USE_ITT) && defined(USE_ITT_INTERNAL)
            if ( (NULL != m_pCommandList->GetGPAInfo()) && m_pCommandList->GetGPAInfo()->bUseGPA )
            {
              static __thread __itt_string_handle* pTaskName = NULL;
              if ( NULL == pTaskName )
              {
                pTaskName = __itt_string_handle_create("Command::eventProfilingCall(USER_EVENT_SIGNALED->RUN_FUNCTION_START)");
              }
              __itt_task_begin(m_pCommandList->GetGPAInfo()->pDeviceDomain, __itt_null, __itt_null, pTaskName);
            }
#endif
            //change m_cmdRunningTime to 1 in order to ensure that next time this "if" will return false; (otherwise m_commandTracer.set_current_time_coi_execution_time_end(); will not invoke in case that profiling is off)
#ifdef ENABLE_MIC_TRACER
            m_cmdRunningTime = 1;
            // Set end coi execution time for the tracer. (COI RUNNING)
            m_commandTracer.set_current_time_coi_execution_time_start();
#else
            // Take current host time
            m_cmdRunningTime = HostTime();
#endif

#if defined(USE_ITT) && defined(USE_ITT_INTERNAL)
            if ( (NULL != m_pCommandList->GetGPAInfo()) && m_pCommandList->GetGPAInfo()->bUseGPA )
            {
              __itt_task_end(m_pCommandList->GetGPAInfo()->pDeviceDomain);
            }
#endif
            break;
        }
        // Continue to RUN_FUNCTION_COMPLETE functionality if 0 != m_cmdRunningTime
    case RUN_FUNCTION_COMPLETE:
#if defined(USE_ITT) && defined(USE_ITT_INTERNAL)
        if ( (NULL != m_pCommandList->GetGPAInfo()) && m_pCommandList->GetGPAInfo()->bUseGPA )
        {
            static __thread __itt_string_handle* pTaskName = NULL;
            if ( NULL == pTaskName )
            {
                pTaskName = __itt_string_handle_create("Command::eventProfilingCall(RUN_FUNCTION_COMPLETE)");
            }
            __itt_task_begin(m_pCommandList->GetGPAInfo()->pDeviceDomain, __itt_null, __itt_null, pTaskName);
        }
#endif
#ifdef ENABLE_MIC_TRACER
        // Set end coi execution time for the tracer. (COI COMPLETED)
        m_commandTracer.set_current_time_coi_execution_time_end();
#endif
        if (m_pCmd->profiling) 
        {
            assert( m_cmdRunningTime > 0 && "RUNNING must be already set at this point");
            m_cmdCompletionTime = HostTime();
        }
#if defined(USE_ITT) && defined(USE_ITT_INTERNAL)
        if ( (NULL != m_pCommandList->GetGPAInfo()) && m_pCommandList->GetGPAInfo()->bUseGPA )
        {
            __itt_task_end(m_pCommandList->GetGPAInfo()->pDeviceDomain);
        }
#endif
        break;
    case RUN_FUNCTION_READY:
        break;
    default:
        assert(0 && "Unknown COI_NOTIFICATIONS type");
    };
}

void Command::releaseResources()
{
    unregisterBarrier(m_endEvent);

#ifdef ENABLE_MIC_TRACER
    // Set end execution time for the tracer.
    m_commandTracer.set_current_time_command_host_time_end();
#endif
}

COIEVENT* Command::registerBarrier(command_event_struct& completionBarrier)
{
  	// If not register yet
  	if (false == completionBarrier.isRegistered)
  	{
        registerProfilingContext();
        COIEventRegisterUserEvent(&(completionBarrier.cmdEvent));
        unregisterProfilingContext();
        completionBarrier.isRegistered = true;
    }
    return NULL;
}

void Command::unregisterBarrier(command_event_struct& completionBarrier)
{
    // If already registered
    if (completionBarrier.isRegistered)
    {
        COIEventUnregisterUserEvent(completionBarrier.cmdEvent);
        completionBarrier.isRegistered = false;
    }
}

//////////////////////////////////////////////////////////////////////////////////
FailureNotification::FailureNotification(IOCLFrameworkCallbacks* pFrameworkCallBacks, cl_dev_cmd_desc* pCmd, cl_dev_err_code returnCode) : Command(NULL, pFrameworkCallBacks, pCmd)
{
    m_lastError = returnCode;
}

cl_dev_err_code FailureNotification::execute()
{
    COIEVENT barrier;
    COIEVENT* pBarrier = &barrier;
    unsigned int numDependecies = 0;
    m_pCommandList->getLastDependentBarrier(pBarrier, &numDependecies, false);
    // If OOO or first command fireCallBack in order to complete, otherwise add the last dependent barrier as my callback.
    if (0 == numDependecies)
    {
        fireCallBack(NULL);
    }
    else
    {
        m_pCommandList->getNotificationPort()->addBarrier(*pBarrier, this, NULL);
    }
    return CL_DEV_SUCCESS;
}
