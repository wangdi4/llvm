// Copyright (c) 2014 Intel Corporation
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

///////////////////////////////////////////////////////////
//  isp_task_dispatcher.cpp
//  Implementation of the Class ISPTaskDispatcher
///////////////////////////////////////////////////////////

#include "stdafx.h"

#include "isp_task_dispatcher.h"
#include "isp_logger.h"
#include "isp_commands.h"

#include <cl_shared_ptr.hpp>
#include <task_executor.h>

using namespace Intel::OpenCL::ISPDevice;
using namespace Intel::OpenCL::TaskExecutor;
using Intel::OpenCL::Utils::SharedPtr;

ISPTaskDispatcher::ISPTaskDispatcher(cl_int devId, IOCLDevLogDescriptor *logDesc,
            IOCLFrameworkCallbacks* frameworkCallbacks, CameraShim* pCameraShim) :
m_iDevId(devId), m_pLogDescriptor(logDesc), m_iLogHandle(0), m_pTaskExecutor(NULL),
m_pFrameworkCallBacks(frameworkCallbacks), m_pCameraShim(pCameraShim)
{
    if (NULL != logDesc)
    {
        cl_int ret = m_pLogDescriptor->clLogCreateClient(m_iDevId, TEXT("ISP Device: Task Dispatcher"), &m_iLogHandle);
        if (CL_DEV_SUCCESS != ret)
        {
            m_iLogHandle = 0;
        }
    }

    IspInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("ISPDevice: Task Dispatcher - Constructed"));
}

ISPTaskDispatcher::~ISPTaskDispatcher()
{
    if (NULL != m_pTaskExecutor && NULL != m_pRootDevice)
    {
        // TODO: what are these?
        m_pRootDevice->ShutDown();
        m_pRootDevice->ResetObserver();
        m_pRootDevice = NULL;
        IspInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("Task Executer is deactivated"));
    }

    IspInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("ISPDevice: Task Dispatcher - Destructed"));

    if (0 != m_iLogHandle)
    {
        m_pLogDescriptor->clLogReleaseClient(m_iLogHandle);
    }
}

cl_dev_err_code ISPTaskDispatcher::Init()
{
    // Verify input from framework
    if (NULL == m_pFrameworkCallBacks)
    {
        IspErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("Invalid value for callbacks was provided by framework"));
        return CL_DEV_INVALID_VALUE;
    }
    if (NULL == m_pCameraShim)
    {
        IspErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("Invalid handle to Camera was provided by framework"));
        return CL_DEV_INVALID_VALUE;
    }

    m_pTaskExecutor = GetTaskExecutor();
    assert(NULL != m_pTaskExecutor && "Task Executer is not loaded!");

    // create root device in flat mode with maximum threads, support for masters joining and 
    // one reserved position for master in device
    RootDeviceCreationParam creationParams(TE_AUTO_THREADS, TE_ENABLE_MASTERS_JOIN, 1);
    m_pRootDevice = m_pTaskExecutor->CreateRootDevice(creationParams, NULL, NULL);
    if (NULL == m_pRootDevice)
    {
        IspErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("Failed to get create TaskExecuter device"));
        return CL_DEV_ERROR_FAIL;
    }

    IspInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("Task Executer is activated"));

    return CL_DEV_SUCCESS;
}

/****************************************************************************************************************
 CreateCommandList
    Description
        This function creates a dependent command queue on a device.
    Input
        props                   Properties for the requested command queue (OOO, InPlace, etc.)
        subdevice_id            Subdevice ID (not used for now)
    Output
        list                    The returned command queue
    Returns
        CL_DEV_SUCCESS          The function is executed successfully.
        CL_DEV_OUT_OF_MEMORY    Failed to allocate required memory for the task list.
********************************************************************************************************************/
cl_dev_err_code ISPTaskDispatcher::CreateCommandList(cl_dev_cmd_list_props IN props, cl_dev_subdevice_id IN subdevice_id, cl_dev_cmd_list* OUT list)
{
    IspInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("CreateCommandList enter"));

    assert(NULL != list && "Framework should not have called this with null parameters");
    assert(0 == subdevice_id && "Subdevices feature is currently not supported");

    ISPDeviceQueue* pIspDeviceQueue = new ISPDeviceQueue(m_pLogDescriptor, m_iLogHandle,
                                                        m_pFrameworkCallBacks, m_pCameraShim);
    if (NULL == pIspDeviceQueue)
    {
        IspErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("Cannot allocate memory for command list"));
        return CL_DEV_OUT_OF_MEMORY;
    }

    // read the requested properties
    bool bIsOOO                 = props & CL_DEV_LIST_ENABLE_OOO;
    bool bIsInPlace             = props & CL_DEV_LIST_IN_PLACE;
    bool bIsProfilingEnabled    = props & CL_DEV_LIST_PROFILING;
    bool bIsDefaultQueue        = props & CL_DEV_LIST_QUEUE_DEFAULT;

    // TODO: handle default queue
    assert(!bIsDefaultQueue && "TODO: default queue handling is not implemented yet");

    // default is in-order list
    CommandListCreationParam creationParams(TE_CMD_LIST_IN_ORDER,
                                            TE_CMD_LIST_PREFERRED_SCHEDULING_DYNAMIC,
                                            bIsProfilingEnabled, bIsDefaultQueue);

    // change default list type if necessary
    if (bIsInPlace)
    {
        // TODO: is this correct?
        creationParams.cmdListType = TE_CMD_LIST_IMMEDIATE;
    }
    else if (bIsOOO)
    {
        creationParams.cmdListType = TE_CMD_LIST_OUT_OF_ORDER;
    }

    cl_dev_err_code ret = pIspDeviceQueue->Init(creationParams, m_pRootDevice.GetPtr());
    if (CL_DEV_FAILED(ret))
    {
        IspErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("Creating command list has failed"));
        delete pIspDeviceQueue;
        return CL_DEV_OUT_OF_MEMORY;
    }

    // TODO: do we need to store the default queue ? (if bIsDefaultQueue)

    *list = pIspDeviceQueue;

    IspDbgLog(m_pLogDescriptor, m_iLogHandle, TEXT("TaskList created - List:%X"), pIspDeviceQueue);

    return CL_DEV_SUCCESS;
}

/********************************************************************************************************************
 FlushCommandList
    Description
        Ensures that all command in queue were sent to execution.
    Input
        list                        A valid handle to device command list.
    Output
        None
     Returns
        CL_DEV_SUCCESS              The function is executed successfully.
        CL_DEV_INVALID_COMMAND_LIST If an invalid command handle was passed to function.
********************************************************************************************************************/
cl_dev_err_code ISPTaskDispatcher::FlushCommandList(cl_dev_cmd_list IN list)
{
    IspInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("FlushCommandList enter"));

    ISPDeviceQueue* pIspDeviceQueue = reinterpret_cast<ISPDeviceQueue*>(list);
    if (NULL == pIspDeviceQueue)
    {
        IspErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("Invalid queue handle was passed"));
        return CL_DEV_INVALID_COMMAND_LIST;
    }

    pIspDeviceQueue->Flush();

    return CL_DEV_SUCCESS;
}

/********************************************************************************************************************
 ReleaseCommandList
    Description
        Releases a device command queue.
    Input
        list                        A valid handle to device command list.
    Output
        None
     Returns
        CL_DEV_SUCCESS              The function is executed successfully.
        CL_DEV_INVALID_COMMAND_LIST If an invalid command handle was passed to function.
********************************************************************************************************************/
cl_dev_err_code ISPTaskDispatcher::ReleaseCommandList(cl_dev_cmd_list IN list)
{
    IspInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("ReleaseCommandList enter"));

    ISPDeviceQueue* pIspDeviceQueue = reinterpret_cast<ISPDeviceQueue*>(list);
    if (NULL == pIspDeviceQueue)
    {
        IspErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("Invalid device queue handle was passed"));
        return CL_DEV_INVALID_COMMAND_LIST;
    }

    delete pIspDeviceQueue;

    return CL_DEV_SUCCESS;
}

/********************************************************************************************************************
 ExecuteCommandList
    Description
        Passes a list of dependent commands into a specified command list for execution.
        The commands are depended by the list index: item[n] depends on item[n-1].
        First item (item[0]) is dependent on the last item that was passed during previous function call on with same list identifier.
    Input
        list                        A valid handle to device command list, where to add list of commands.
        cmds                        A vector of dependent commands, each entry is described by cl_dev_cmd_desc structure
        count                       Number of entries in cmds parameter
    Output
        None
     Returns
        CL_DEV_SUCCESS              The function is executed successfully.
        CL_DEV_INVALID_COMMAND_LIST If an invalid command queue handle was passed to function.
        CL_DEV_INVALID_VALUE        If an invalid command handle was passed to function.
********************************************************************************************************************/
cl_dev_err_code ISPTaskDispatcher::ExecuteCommandList(cl_dev_cmd_list IN list, cl_dev_cmd_desc* IN *cmds, cl_uint IN count)
{
    IspInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("ExecuteCommandList enter"));

    assert(0 != count && "Invalid commands count");

    // TODO: if list is NULL then create temporary list
    assert(NULL != list && "Temporary task list is not implemented yet");

    ISPDeviceQueue* pIspDeviceQueue = reinterpret_cast<ISPDeviceQueue*>(list);

    cl_dev_err_code ret;

    for (cl_uint i = 0; i < count; ++i)
    {
        ret = pIspDeviceQueue->CreateCommand(cmds[i]);
        if (CL_DEV_FAILED(ret))
        {
            IspErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("Invalid command handle was passed"));
            return ret;
        }
    }

    return CL_DEV_SUCCESS;
}

/********************************************************************************************************************
 WaitForCompletion
    Description
        Blocks the calling thread until the command specified finished execution.
    Input
        list                        A valid handle to device command list, which contains the commands to wait.
        cmdToWait                   The command to be waited for. If NULL, then function will wait for all commands in command queue.
    Output
        None
     Returns
        CL_DEV_SUCCESS              The function is executed successfully.
        CL_DEV_INVALID_COMMAND_LIST If an invalid command queue handle was passed to function.
********************************************************************************************************************/
cl_dev_err_code ISPTaskDispatcher::WaitForCompletion(cl_dev_cmd_list IN list, cl_dev_cmd_desc* IN cmdToWait)
{
    IspInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("WaitForCompletion enter"));

    ISPDeviceQueue* pIspDeviceQueue = reinterpret_cast<ISPDeviceQueue*>(list);
    if (NULL == pIspDeviceQueue)
    {
        IspErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("Invalid device queue handle was passed"));
        return CL_DEV_INVALID_COMMAND_LIST;
    }

    if (NULL != cmdToWait)
    {
        // wait for specific command
        return pIspDeviceQueue->WaitForCommand(cmdToWait);
    }
    // else: wait for all commands in command queue
    return pIspDeviceQueue->WaitForAllCommands();
}

/********************************************************************************************************************
 CancelCommandList
    Description
        Cancel the execution of a command list.
    Input
        list                        A valid handle to device command list, which its execution will be cancelled.
    Output
        None
     Returns
        CL_DEV_SUCCESS              The function is executed successfully.
        CL_DEV_INVALID_COMMAND_LIST If an invalid command queue handle was passed to function.
********************************************************************************************************************/
cl_dev_err_code ISPTaskDispatcher::CancelCommandList(cl_dev_cmd_list IN list)
{
    IspInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("CancelCommandList enter"));

    ISPDeviceQueue* pIspDeviceQueue = reinterpret_cast<ISPDeviceQueue*>(list);
    if (NULL == pIspDeviceQueue)
    {
        IspErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("Invalid device queue handle was passed"));
        return CL_DEV_INVALID_COMMAND_LIST;
    }

    pIspDeviceQueue->Cancel();
    pIspDeviceQueue->Flush();

    return CL_DEV_SUCCESS;
}

/********************************************************************************************************************
 ReleaseCommand
    Description
        Releases a command.
    Input
        cmdToRelease                The command to be released.
    Output
        None
     Returns
        CL_DEV_SUCCESS              The function is executed successfully.
        CL_DEV_INVALID_VALUE        If an invalid command handle was passed to function.
********************************************************************************************************************/
cl_dev_err_code ISPTaskDispatcher::ReleaseCommand(cl_dev_cmd_desc* IN cmdToRelease)
{
    IspInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%s"), TEXT("ReleaseCommand enter"));

    if (NULL == cmdToRelease)
    {
        return CL_DEV_INVALID_VALUE;
    }

    assert(NULL != cmdToRelease->device_agent_data && "TODO: is this valid command ?");

    // SharedPtr will take care of the de-allocation
    SharedPtr<ITaskBase> pCommand = static_cast<ITaskBase*>(cmdToRelease->device_agent_data);
    // see ISPDeviceQueue::CreateCommand
    pCommand.DecRefCnt();

    return CL_DEV_SUCCESS;
}



// TODO: support multiple device queues
cl_uint ISPDeviceQueue::m_deviceQueuesCount = 0;
ISPDeviceQueue::ISPDeviceQueue(IOCLDevLogDescriptor* logDesc, cl_int logHandle,
                               IOCLFrameworkCallbacks* frameworkCallbacks, CameraShim* pCameraShim) :
m_pLogDescriptor(logDesc), m_iLogHandle(logHandle), m_pFrameworkCallBacks(frameworkCallbacks),
m_pCameraShim(pCameraShim)
{
    assert(0 == m_deviceQueuesCount && "multiple device queues is not supported yet");
    m_deviceQueuesCount++;
}
ISPDeviceQueue::~ISPDeviceQueue()
{
    m_deviceQueuesCount--;
}

cl_dev_err_code ISPDeviceQueue::Init(CommandListCreationParam params, ITEDevice* device)
{
    if (NULL == device)
    {
        return CL_DEV_INVALID_VALUE;
    }
    m_pTaskList = device->CreateTaskList(params);
    if (NULL == m_pTaskList)
    {
        return CL_DEV_OUT_OF_MEMORY;
    }

    return CL_DEV_SUCCESS;
}

fnDispatcherCommandCreate_t* ISPDeviceQueue::m_vCommands[] =
{
    NULL,
    &ReadWriteMemoryObject::Create,     //    CL_DEV_CMD_READ = 1,
    &ReadWriteMemoryObject::Create,     //    CL_DEV_CMD_WRITE,
    &CopyMemoryObject::Create,          //    CL_DEV_CMD_COPY,
    &MapMemoryObject::Create,           //    CL_DEV_CMD_MAP,
    &UnmapMemoryObject::Create,         //    CL_DEV_CMD_UNMAP,
    &NDRange::Create,                   //    CL_DEV_CMD_EXEC_KERNEL,
    NULL, //&NDRange2::Create,               //    CL_DEV_CMD_EXEC_TASK,
    NULL, //&NativeFunction::Create,        //    CL_DEV_CMD_EXEC_NATIVE,
    &FillMemoryObject::Create,          //    CL_DEV_CMD_FILL_BUFFER
    &FillMemoryObject::Create,          //    CL_DEV_CMD_FILL_IMAGE
    NULL, //&MigrateMemObject::Create       //    CL_DEV_CMD_MIGRATE
};

cl_dev_err_code ISPDeviceQueue::CreateCommand(cl_dev_cmd_desc* pCmd)
{
    if (NULL == pCmd)
    {
        return CL_DEV_INVALID_VALUE;
    }

    // TODO: add support for extension mode
    // TODO: if not buffering commands -> execute

    // TODO: if (CL_DEV_CMD_EXEC_KERNEL == pCmd->type)

    assert(pCmd->type < CL_DEV_CMD_MAX_COMMAND_TYPE);


    fnDispatcherCommandCreate_t* fnCreate = m_vCommands[pCmd->type];

    assert(NULL != fnCreate && "Invalid create command function");

    SharedPtr<ITaskBase> pCommand;
    cl_dev_err_code ret = fnCreate(this, pCmd, &pCommand);
    if (CL_DEV_SUCCEEDED(ret))
    {
        pCommand->IncRefCnt();
        pCmd->device_agent_data = pCommand.GetPtr();
        m_pTaskList->Enqueue(pCommand);
    }
    else
    {
        // Try to notify about the error in the same list
        ret = NotifyFailure(pCmd);
        if (CL_DEV_FAILED(ret))
        {
            return ret;
        }
    }

    return CL_DEV_SUCCESS;
}

cl_dev_err_code ISPDeviceQueue::NotifyFailure(cl_dev_cmd_desc* pCmd)
{
    IspErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("Failed to submit command[id:%d,type:%d] to execution"), pCmd->id, pCmd->type);

    SharedPtr<ITaskBase> pCommand;
    cl_dev_err_code ret = CommandFailureNotification::Create(this, pCmd, &pCommand);
    if (CL_FAILED(ret))
    {
        return ret;
    }

    m_pTaskList->Enqueue(pCommand);

    return CL_DEV_SUCCESS;
}

cl_dev_err_code ISPDeviceQueue::Cancel()
{
    m_pTaskList->Cancel();
    return CL_DEV_SUCCESS;
}

cl_dev_err_code ISPDeviceQueue::Flush()
{
    // TODO: what to do with buffered commands ?

    m_pTaskList->Flush();
    return CL_DEV_SUCCESS;
}

cl_dev_err_code ISPDeviceQueue::WaitForCommand(cl_dev_cmd_desc* pCmd)
{
    // TODO: what about buffered commands?
    // TODO: cancel all buffered commands ? or insert implicit END_PIPELINE ? or return error ?

    assert(NULL != pCmd && "Invalid command descriptor, should call WaitForAllCommands() instead");

    // At this stage we assume that pCmd is a valid pointer
    // Appropriate reference count is done in runtime
    void* pCommand = pCmd->device_agent_data;

    // TODO: possible race ?
    //       Check if the command is already completed but it can be just before a call to the notification function.
    //       and we are not sure that RT got the completion notification.
    //       Therefore, we MUST return error value and RT will take appropriate action in order to monitor event status
    assert(NULL != pCommand && "Race issue, this should be fixed in Runtime");

    SharedPtr<ITaskBase> pCommandToWait = static_cast<ITaskBase*>(pCommand);

    // No need in lock
    te_wait_result res = m_pTaskList->WaitForCompletion(pCommandToWait);

    // TODO: maybe add ITask->Release() ?

    if ((!pCommandToWait->IsCompleted() && (TE_WAIT_COMPLETED == res)) || TE_WAIT_NOT_SUPPORTED == res)
    {
        // The command is not completed while the wait has returned meaning waiting is not supported
        Flush();
        return CL_DEV_NOT_SUPPORTED;
    }

    return CL_DEV_SUCCESS;
}

cl_dev_err_code ISPDeviceQueue::WaitForAllCommands()
{
    // TODO: what about buffered commands?
    // TODO: cancel all buffered commands ? or insert implicit END_PIPELINE ? or return error ?

    // Wait for all tasks in tasklist
    // No need in lock
    te_wait_result res = m_pTaskList->WaitForCompletion(NULL);

    if (TE_WAIT_COMPLETED == res)
    {
        return CL_DEV_SUCCESS;
    }
    else if (TE_WAIT_MASTER_THREAD_BLOCKING == res)
    {
        return CL_DEV_BUSY;
    }
    // else: waiting is not supported
    return CL_DEV_NOT_SUPPORTED;
}
