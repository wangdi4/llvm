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

#include "mic_device.h"
#include "command_list.h"
#include "execution_commands.h"
#include "buffer_commands.h"
#include "memory_allocator.h"
#include "mic_sys_info.h"

#include "cl_sys_defines.h"
#include <hw_utils.h>

#include <source/COIEvent_source.h>

#include <cstring>
#include <assert.h>

using namespace Intel::OpenCL::MICDevice;

const size_t CommandList::m_sNoBatch = (size_t)-1;
bool CommandList::m_sBatchModeInitialized = false;
size_t CommandList::m_sBatchAfter = CommandList::m_sNoBatch;

CommandList::fnCommandCreate_t* CommandList::m_vCommands[CL_DEV_CMD_MAX_COMMAND_TYPE] = {
    /* CL_DEV_CMD_INVALID      */ nullptr,
    /* CL_DEV_CMD_READ         */ &ReadWriteMemObject::Create,
    /* CL_DEV_CMD_WRITE        */ &ReadWriteMemObject::Create,
    /* CL_DEV_CMD_COPY         */ &CopyMemObject::Create,
    /* CL_DEV_CMD_MAP          */ &MapMemObject::Create,
    /* CL_DEV_CMD_UNMAP        */ &UnmapMemObject::Create,
    /* CL_DEV_CMD_EXEC_KERNEL  */ &NDRange::Create,
    /* CL_DEV_CMD_EXEC_TASK    */ nullptr,
    /* CL_DEV_CMD_EXEC_NATIVE  */ nullptr,
    /* CL_DEV_CMD_FILL_BUFFER  */ &FillMemObject::Create,
    /* CL_DEV_CMD_FILL_IMAGE   */ &FillMemObject::Create,
    /* CL_DEV_CMD_MIGRATE      */ &MigrateMemObject::Create
};

/* Static method that initialized the batch mode. Must call it at the initialization of the device. */
void CommandList::initializeBatchMode()
{
    if (!m_sBatchModeInitialized)
    {
        m_sBatchAfter = MICSysInfo::getInstance().getMicDeviceConfig().Device_BatchAfter();
        const char* batchModeStr = MICSysInfo::getInstance().getMicDeviceConfig().Device_BatchMode().c_str();
        if (0 == STRCMPI_S(batchModeStr, "on", 2))
        {
            // Batch if at least on command exist on device
            m_sBatchAfter = 1;
        }
        else if (0 == STRCMPI_S(batchModeStr, "off", 3))
        {
            // Never batch
            m_sBatchAfter = m_sNoBatch;
        }
        m_sBatchModeInitialized = true;
    }
}


CommandList::CommandList(const SharedPtr<NotificationPort>& pNotificationPort, 
                         DeviceServiceCommunication* pDeviceServiceComm, 
                         IOCLFrameworkCallbacks* pFrameworkCallBacks, 
                         ProgramService* pProgramService, 
                         PerformanceDataStore* pOverheadData,
                         cl_dev_subdevice_id subDeviceId,
                         bool isInOrder,
                         bool isProfiling
#ifdef USE_ITT
                         , const ocl_gpa_data* pGPAData
#endif
                         ) :
        m_validBarrier(false), m_pNotificationPort(pNotificationPort),
        m_pDeviceServiceComm(pDeviceServiceComm), m_pFrameworkCallBacks(pFrameworkCallBacks),
        m_pProgramService(pProgramService), m_pipe(nullptr), m_pDeviceQueueAddress(), m_subDeviceId(subDeviceId),
        m_pOverhead_data(pOverheadData), m_lastCommand(nullptr), m_isInOrderQueue(isInOrder), m_isProfilingEnabled(isProfiling), m_lastCommandCompletionTime(0), m_bIsCanceled(false),
        m_numCommandsEnq(0), m_canUseSyncQueue(true)
#ifdef USE_ITT
        ,m_pGPAData(pGPAData)
#endif
{
    m_refCounter = 1;
    m_pDeviceQueueAddress.init();
#ifdef _DEBUG
    m_numOfConcurrentExecutions = 0;
#endif
}

CommandList::~CommandList()
{
//    assert(m_refCounter == 0 && "Deleting CommandList while reference counter is larger than 0");
    if ((!MICDevice::isDeviceLibraryUnloaded()) && (m_pipe))
    {
        cl_dev_err_code err = CL_DEV_SUCCESS;
        err = releaseCommandListOnDevice();
        assert(CL_DEV_SUCCESS == err);
        COIRESULT result = COI_SUCCESS;
        result = COIPipelineDestroy(m_pipe);
        assert(result == COI_SUCCESS && "COIPipelineDestroy failed");
    }
}

cl_dev_err_code CommandList::commandListFactory(cl_dev_cmd_list_props IN        props, 
                                                cl_dev_subdevice_id             subDeviceId, 
                                                const SharedPtr<NotificationPort>&    pNotificationPort, 
                                                DeviceServiceCommunication*     pDeviceServiceComm,
                                                IOCLFrameworkCallbacks*         pFrameworkCallBacks,
                                                ProgramService*                 pProgramService,
                                                PerformanceDataStore*           pOverheadData,
#ifdef USE_ITT
                                                const ocl_gpa_data*             pGPAData,
#endif
                                                CommandList**                   outCommandList)
{
    const int MIC_SUPPORTED_COMMAND_QUEUE_PROPERTIES = CL_DEV_LIST_ENABLE_OOO | CL_DEV_LIST_PROFILING;
    cl_dev_err_code result = CL_DEV_SUCCESS;
    //Check for unsupported properties
    if (0 != ((int)props & ~MIC_SUPPORTED_COMMAND_QUEUE_PROPERTIES))
    {
        return CL_DEV_INVALID_PROPERTIES;
    }

    bool isInOrder = (0 == ((int)props & (int)CL_DEV_LIST_ENABLE_OOO) );
    bool isProfiling = (0 != ((int)props & (int)CL_DEV_LIST_PROFILING) );
    CommandList* tCommandList = new CommandList(pNotificationPort, pDeviceServiceComm, 
                                                pFrameworkCallBacks, pProgramService, 
                                                pOverheadData, subDeviceId, isInOrder, isProfiling
#ifdef USE_ITT
                                                ,pGPAData
#endif
                                                );
    if (nullptr == tCommandList)
    {
        return CL_DEV_OUT_OF_MEMORY;
    }
    // create new COIPipeline
    result = tCommandList->createPipeline();
    if (result != CL_DEV_SUCCESS)
    {
        bool tempDeleteFlag = false;
        tCommandList->releaseCommandList(&tempDeleteFlag);
        assert(tempDeleteFlag);
        delete(tCommandList);
        return result;
    }
    // init command list on device side.
    result = tCommandList->initCommandListOnDevice();
    if (result != CL_DEV_SUCCESS)
    {
        bool tempDeleteFlag = false;
        tCommandList->releaseCommandList(&tempDeleteFlag);
        assert(tempDeleteFlag);
        delete(tCommandList);
        return result;
    }

    *outCommandList = tCommandList;
    return result;
}

cl_dev_err_code CommandList::retainCommandList()
{
    long prevVal = m_refCounter ++;
    if (prevVal == 0)
    {
        return CL_DEV_INVALID_OPERATION;
    }
    return CL_DEV_SUCCESS;
}

cl_dev_err_code CommandList::releaseCommandList(bool* outDelete)
{
    long newVal = -- m_refCounter;
    if (newVal < 0)
    {
        return CL_DEV_INVALID_OPERATION;
    }
    *outDelete = (newVal == 0);
    return CL_DEV_SUCCESS;
}

cl_dev_err_code CommandList::commandListExecute(cl_dev_cmd_desc* IN *cmds, cl_uint IN count)
{
#ifdef _DEBUG
    long oldVal = m_numOfConcurrentExecutions++;
    assert( ((0 == oldVal) || (false == m_isInOrderQueue)) && "In order commands should not come in parallel" );
#endif

#if defined(USE_ITT) && defined(USE_ITT_INTERNAL)
    if ( (nullptr != m_pGPAData) && m_pGPAData->bUseGPA )
    {
      static __thread __itt_string_handle* pTaskName = nullptr;
      if ( nullptr == pTaskName )
      {
        pTaskName = __itt_string_handle_create("CommandList::commandListExecute()");
      }
      __itt_task_begin(m_pGPAData->pDeviceDomain, __itt_null, __itt_null, pTaskName);
    }
#endif

    cl_dev_err_code rc = CL_DEV_SUCCESS;
    SharedPtr<Command> pCmdObject;
    // run over all the cmds
    for (unsigned int i = 0; i < count; i++)
    {
        
        // Create appropriate Command object, After createCommandObject() complete, Do not change pCmdObject until completion of pCmdObject->execute()!
        rc = createCommandObject(cmds[i], pCmdObject);
        // If there is no enough memory for allocating Command object
        if (CL_DEV_FAILED(rc))
        {
            break;
        }
#if defined(USE_ITT) && defined(USE_ITT_INTERNAL)
      if ( (nullptr != m_pGPAData) && m_pGPAData->bUseGPA )
      {
        static __thread __itt_string_handle* pTaskName = nullptr;
        if ( nullptr == pTaskName )
        {
          pTaskName = __itt_string_handle_create("CommandList::commandListExecute()->Command::Execute()");
        }
        __itt_task_begin(m_pGPAData->pDeviceDomain, __itt_null, __itt_null, pTaskName);
      }
#endif
        // Send the command for execution. pCmdObject will delete itself.
        rc = pCmdObject->execute();
#if defined(USE_ITT) && defined(USE_ITT_INTERNAL)
        if ( (nullptr != m_pGPAData) && m_pGPAData->bUseGPA )
        {
            __itt_task_end(m_pGPAData->pDeviceDomain);
        }
#endif
        if (CL_DEV_FAILED(rc))
        {
            break;
        }
        // Save the new Command pointer in device agent data. ASSUME that calling to clDevReleaseCommand in order to delete this Command!
        cmds[i]->device_agent_data = (Command*)(pCmdObject.GetPtr());
        // Incrementing the reference counter because it is a temporary shared pointer instance. (otherwise the Command object can delete before it complete its execution)
        pCmdObject->retainCommand();
        // In order to decrease the reference count of the last command.
        pCmdObject = nullptr;
    }
#ifdef _DEBUG
    oldVal = m_numOfConcurrentExecutions--;
    assert( ((1 == oldVal) || (false == m_isInOrderQueue)) && "In order commands should not come in parallel" );
#endif

#if defined(USE_ITT) && defined(USE_ITT_INTERNAL)
  if ( (nullptr != m_pGPAData) && m_pGPAData->bUseGPA )
  {
    __itt_task_end(m_pGPAData->pDeviceDomain);
  }
#endif

    return rc;
}

cl_dev_err_code CommandList::commandListWaitCompletion(cl_dev_cmd_desc* cmdDescToWait)
{
#if defined(USE_ITT) && defined(USE_ITT_INTERNAL)
    if ( (nullptr != m_pGPAData) && m_pGPAData->bUseGPA )
    {
        static __thread __itt_string_handle* pTaskName = nullptr;
        if ( nullptr == pTaskName )
        {
            pTaskName = __itt_string_handle_create("CommandList::commandListWaitCompletion()");
        }
        __itt_task_begin(m_pGPAData->pDeviceDomain, __itt_null, __itt_null, pTaskName);
    }
#endif

    SharedPtr<Command> waitToCmd = nullptr;
    // Assume that if cmdDescToWait->device_agent_data != NULL than the Command object exist.
    if ((cmdDescToWait) && (cmdDescToWait->device_agent_data))
    {
        waitToCmd = (Command*)(cmdDescToWait->device_agent_data);
    }
    else
    {
        // Wait to last Command (if didn't finish yet)
        waitToCmd = getLastCommand();
    }

    cl_dev_err_code err = CL_DEV_SUCCESS;
    if (nullptr != waitToCmd)
    {        
        COIRESULT coi_err = COI_SUCCESS;
        coi_err = COIEventWait(1, &(waitToCmd->getCommandCompletionEvent()), -1, true, nullptr, nullptr);
        if (COI_SUCCESS != coi_err)
        {
            assert((COI_SUCCESS == coi_err) || "COIEventWait failed in commandListWaitCompletion");
            err = CL_DEV_ERROR_FAIL;
        }
        else
        {
            waitToCmd->waitForCompletion();
        }
    }    
#if defined(USE_ITT) && defined(USE_ITT_INTERNAL)
    if ( (nullptr != m_pGPAData) && m_pGPAData->bUseGPA )
    {
        __itt_task_end(m_pGPAData->pDeviceDomain);
    }
#endif
    return err;
}

cl_dev_err_code CommandList::createCommandObject(cl_dev_cmd_desc* cmd, SharedPtr<Command>& cmdObject)
{
#if defined(USE_ITT) && defined(USE_ITT_INTERNAL)
    if ( (nullptr != m_pGPAData) && m_pGPAData->bUseGPA )
    {
      static __thread __itt_string_handle* pTaskName = nullptr;
      if ( nullptr == pTaskName )
      {
        pTaskName = __itt_string_handle_create("CommandList::createCommandObject()");
      }
      __itt_task_begin(m_pGPAData->pDeviceDomain, __itt_null, __itt_null, pTaskName);
    }
#endif

    if ((nullptr == cmd) || (cmd->type >= CL_DEV_CMD_MAX_COMMAND_TYPE))
    {
        return CL_DEV_INVALID_VALUE;
    }
    // get function pointer to cmd->type factory function
    fnCommandCreate_t* fnCreate = m_vCommands[cmd->type];
    assert( (nullptr != fnCreate) && "Not implemented");
    SharedPtr<Command> pCmdObject;
    // create appropriate command object. In order to delete it call to clDevReleaseCommand()
    cl_dev_err_code    rc = fnCreate(this, m_pFrameworkCallBacks, cmd, pCmdObject);
    // if failed create FailureNotification command object
    if (CL_DEV_FAILED(rc))
    {
        pCmdObject = FailureNotification::Create(m_pFrameworkCallBacks, cmd, rc);
        if (nullptr == pCmdObject)
        {
            rc = CL_DEV_OUT_OF_MEMORY;
        }
    }
    cmdObject = pCmdObject;
#if defined(USE_ITT) && defined(USE_ITT_INTERNAL)
  if ( (nullptr != m_pGPAData) && m_pGPAData->bUseGPA )
  {
    __itt_task_end(m_pGPAData->pDeviceDomain);
  }
#endif

	return rc;
}

cl_dev_err_code CommandList::createPipeline()
{
    COIRESULT result = COI_ERROR;
    // Create COIPipeline for this queue.
    result = COIPipelineCreate(m_pDeviceServiceComm->getDeviceProcessHandle(),        // A handle to an already existing process
                               nullptr,                                                  // An optional mask of the set of hardware threads
                               nullptr,                                                  // StackSize
                               &m_pipe);                                              // Handle returned to uniquely identify the pipeline
    if (result != COI_SUCCESS)
    {
        return CL_DEV_ERROR_FAIL;
    }
    return CL_DEV_SUCCESS;
}

cl_dev_err_code CommandList::initCommandListOnDevice()
{
    INIT_QUEUE_ON_DEVICE_STRUCT data;
    data.is_in_order_queue = isInOrderCommandList();

    INIT_QUEUE_ON_DEVICE_OUTPUT_STRUCT data_out;
    data_out.device_queue_addresses.device_sync_queue_address = 0;
    data_out.device_queue_addresses.device_async_queue_address = 0;
    data_out.ret_code             = CL_DEV_SUCCESS;

    cl_dev_err_code res = runBlockingFuncOnDevice(DeviceServiceCommunication::INIT_COMMANDS_QUEUE,
                                                  &data, sizeof(data),
                                                  &data_out, sizeof(data_out));
    if ( CL_DEV_FAILED(res) )
    {
        assert ( 0 && "Queue creation function call failed" );
        return res;
    }

    res = data_out.ret_code;
    if ( CL_DEV_FAILED(res) )
    {
        assert ( 0 && "Queue creation failed on device" );
        return res;
    }

    assert( ((0 != data_out.device_queue_addresses.device_sync_queue_address) || (0 != data_out.device_queue_addresses.device_async_queue_address)) && "Queue creationFailed to create queue on device");
    m_pDeviceQueueAddress.device_sync_queue_address = data_out.device_queue_addresses.device_sync_queue_address;
    m_pDeviceQueueAddress.device_async_queue_address = data_out.device_queue_addresses.device_async_queue_address;
    return res;
}

cl_dev_err_code CommandList::releaseCommandListOnDevice()
{
    cl_dev_err_code res;
    cl_dev_err_code devRes;

    res = runBlockingFuncOnDevice(DeviceServiceCommunication::RELEASE_COMMANDS_QUEUE,
        &m_pDeviceQueueAddress, sizeof(m_pDeviceQueueAddress),
        &devRes, sizeof(devRes));

    assert( CL_DEV_SUCCEEDED(res) && CL_DEV_SUCCEEDED(devRes) && "Failed to release queue");
    return CL_DEV_SUCCEEDED(res) && CL_DEV_SUCCEEDED(devRes) ? CL_DEV_SUCCESS : CL_DEV_ERROR_FAIL;
}

cl_dev_err_code CommandList::cancelCommandList()
{
    assert( (0 != m_pDeviceQueueAddress.device_sync_queue_address) && (0 != m_pDeviceQueueAddress.device_async_queue_address) );

    if ((0 == m_pDeviceQueueAddress.device_sync_queue_address) || (0 == m_pDeviceQueueAddress.device_async_queue_address))
    {
        return CL_DEV_INVALID_VALUE;
    }

    m_bIsCanceled = true;

    // run cancel command in a service pipeline - in parallel to running and queued commands
    utility_function_options    data;
    data.request                            = QUEUE_CANCEL;
    data.options.queue_cancel.queue_address = m_pDeviceQueueAddress;

    bool ok = m_pDeviceServiceComm->runServiceFunction( DeviceServiceCommunication::EXECUTE_DEVICE_UTILITY,
                                                        sizeof(data),  &data, 
                                                        0,             nullptr,
                                                        0,             nullptr, nullptr );

    return (ok) ? CL_DEV_SUCCESS : CL_DEV_ERROR_FAIL;
}

cl_dev_err_code CommandList::runBlockingFuncOnDevice(DeviceServiceCommunication::DEVICE_SIDE_FUNCTION func,
                                                     void* in_data,  size_t in_data_size,
                                                     void* out_data, size_t out_data_size )
{
    assert( ((nullptr != in_data)  && (0 != in_data_size))  || ((nullptr == in_data)  && (0 == in_data_size))  );
    assert( ((nullptr != out_data) && (0 != out_data_size)) || ((nullptr == out_data) && (0 == out_data_size)) );

    if (nullptr == in_data)
    {
        in_data_size = 0;
    }
    else if (0 == in_data_size)
    {
        in_data = nullptr;
    }

    if (nullptr == out_data)
    {
        out_data_size = 0;
    }
    else if (0 == out_data_size)
    {
        out_data = nullptr;
    }
    
    // Run func on device with no dependencies, assign a barrier in order to wait until the function execution complete.
    bool ok = runQueueServiceFunction( func,
                                       in_data_size,  in_data,
                                       out_data_size, out_data,
                                       0, nullptr,nullptr );

    return (ok) ? CL_DEV_SUCCESS : CL_DEV_ERROR_FAIL;
}

SharedPtr<Command> CommandList::getLastCommand()
{
  // TODO: not clear why mutex is requried. it's single read operation
    OclAutoMutex lock(&m_lastCommandMutex);
    SharedPtr<Command> retCommand = m_lastCommand;
    return retCommand;
}

void CommandList::setLastCommand(const SharedPtr<Command>& newCommand)
{
    OclAutoMutex lock(&m_lastCommandMutex);
    m_lastCommand = newCommand;
}

void CommandList::resetLastCommand(const SharedPtr<Command>& oldCommand)
{
    assert((nullptr != oldCommand) && "Cannot reset NULL command");
    OclAutoMutex lock(&m_lastCommandMutex);
    if (oldCommand == m_lastCommand)
    {
        m_lastCommand = nullptr;
    }
}

uint64_t CommandList::acquireDeviceQueue() 
{
    uint64_t address = m_pDeviceQueueAddress.device_sync_queue_address;
    // In case of out of order use async queue always
    if (false == m_isInOrderQueue)
    {
        address = m_pDeviceQueueAddress.device_async_queue_address;
    }
    else if (m_sBatchAfter != m_sNoBatch)
    {
        m_batchSpinMutex.Lock();
        m_numCommandsEnq ++;
        if ((false == m_canUseSyncQueue) || (m_numCommandsEnq > m_sBatchAfter))
        {
            address = m_pDeviceQueueAddress.device_async_queue_address;
            m_canUseSyncQueue = false;
        }
        m_batchSpinMutex.Unlock();
    }
    return address; 
}

void CommandList::getLastDependentBarrier(COIEVENT* barrier, unsigned int* numDependencies, bool isExecutionTask)
{
    assert( nullptr != numDependencies && "invalid input, NULL is not expected" );
    assert( nullptr != barrier         && "invalid input, NULL is not expected" );

    if ( !m_isInOrderQueue )
    {
      *numDependencies = 0;
      return;
    }

    // Need to obtain local reference to the last command
    SharedPtr<Command> lastCmd = getLastCommand();

    /* If last command is NULL --> Last command completed or not exist; or the current command is going to enqueue to COIPipe and the last Command also enqueued to COIPipe
      Then we can return NULL as the barrier. */
    if ((nullptr == lastCmd) || ((isExecutionTask) && (lastCmd->commandEnqueuedToPipe())))
    {
      *numDependencies = 0;
    }
    else
    {
      assert(lastCmd && "lastCmd Must be valid pointer");
      *barrier = lastCmd->getCommandCompletionEvent();
      *numDependencies = 1;
    }
}
