/////////////////////////////////////////////////////////////////////////
// queue_worker_thread.cpp
/////////////////////////////////////////////////////////////////////////
// INTEL CONFIDENTIAL
// Copyright 2007-2008 Intel Corporation All Rights Reserved.
//
// The source code contained or described herein and all documents related 
// to the source code ("Material") are owned by Intel Corporation or its 
// suppliers or licensors. Title to the Material remains with Intel Corporation
// or its suppliers and licensors. The Material may contain trade secrets and 
// proprietary and confidential information of Intel Corporation and its 
// suppliers and licensors, and is protected by worldwide copyright and trade 
// secret laws and treaty provisions. No part of the Material may be used, copied, 
// reproduced, modified, published, uploaded, posted, transmitted, distributed, 
// or disclosed in any way without Intel’s prior express written permission. 
//
// No license under any patent, copyright, trade secret or other intellectual
// property right is granted to or conferred upon you by disclosure or delivery 
// of the Materials, either expressly, by implication, inducement, estoppel or 
// otherwise. Any license under such intellectual property rights must be express
// and approved by Intel in writing.
//
// Unless otherwise agreed by Intel in writing, you may not remove or alter this notice 
// or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors 
// in any way.
/////////////////////////////////////////////////////////////////////////
#include "queue_worker_thread.h"
#include "command_queue.h"
#include "enqueue_commands.h"

using namespace Intel::OpenCL::Utils;
using namespace Intel::OpenCL::Framework;

/************************************************************************
 * Creates worker object.
/************************************************************************/
QueueWorkerThread::QueueWorkerThread():
m_pCommandsQueue(NULL)
{
}

/************************************************************************
 * Destroys the thread object.
 * If the thread is running, the function wait for it completion
/************************************************************************/
QueueWorkerThread::~QueueWorkerThread()
{
    m_pCommandsQueue = NULL;
}

/************************************************************************
 * Sets the queue list. 
 * Queue can be replaced during execution... however, this can happen only
 * if the current queue is empty.
 * 
/************************************************************************/
cl_err_code QueueWorkerThread::SetQueue(ICommandQueue* pCommandsQueue)
{    
    // TODO: lock the queue from anyone else to add to it.
    // First test that current queue is empty
    if( (NULL != m_pCommandsQueue) && (!m_pCommandsQueue->IsEmpty()))        
    {
        return CL_ERR_EXECUTION_FAILED;
    }
    m_pCommandsQueue = pCommandsQueue;
    return CL_SUCCESS;
}

/************************************************************************
 * Gets the next available job in the job list.
 * If there is no job, this function waits until job is ready or join
 * has been called.
 *
/************************************************************************/
Command* QueueWorkerThread::GetNextCommand()
{
    // The wait process is done by the list itself
    return m_pCommandsQueue->GetNextCommand(); 
}

/************************************************************************
 * This is the working loop.
 * If the thread is running, the function wait for it completion
/************************************************************************/
int QueueWorkerThread::Run()
{
    Command*        p_nextCommand;
    int             status = CL_SUCCESS;

    // Don't ever start processing without a queue
    if(NULL == m_pCommandsQueue)
    {
        return CL_ERR_EXECUTION_FAILED;
    }

    // the infinite loop of execution
    while (1) 
    {

        // Get next command from the list
        // If there isn't any command the function is blocked
        p_nextCommand = GetNextCommand();
        if (NULL == p_nextCommand)
        {
            if(m_join)
            {
                // Someone asked the thread to finish. Time to join
                break;
            }
            // if got here some bad situation has happened
            // stop the execution with the fatal error
            status = CL_ERR_EXECUTION_FAILED;
            break;
        }
        // if got here we have a job in the hands and can proceed it
        // THe command knows hoe to handle it self
        status = p_nextCommand->Execute();        

        if (CL_FAILED(status))
        {
            // some problem happened - exit with the error status
            break;
        }        
        if(m_join)
        {
            // Someone asked the thread to finish. Time to join
            break;
        }
    }
    return status;
}

/************************************************************************
 * Notify the worker that it done it job and he can
 * stop process.
 * the thread will exit as soon as it finish it last/current job.
 * 
/************************************************************************/
cl_err_code QueueWorkerThread::CancelProcessing()
{
    m_join = true;
    return CL_SUCCESS;
}
