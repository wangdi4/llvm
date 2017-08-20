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

#include "device_queue.h"
#include "native_thread_pool.h"
#include "thread_local_storage.h"
#include "native_common_macros.h"
#include "cl_shared_ptr.hpp"

#include <sink/COIBuffer_sink.h>
#include <common/COIEvent_common.h>
#include <sink/COIPipeline_sink.h>

#ifdef USE_ITT
#include <ocl_itt.h>
#endif

using namespace Intel::OpenCL::MICDevice;
using namespace Intel::OpenCL::MICDeviceNative;
using namespace Intel::OpenCL::TaskExecutor;
using namespace Intel::OpenCL::Utils;

#ifdef _DEBUG
using namespace Intel::OpenCL::UtilsNative;
#endif
#ifdef __MIC_DA_OMP__
    Intel::OpenCL::Utils::AtomicCounter QueueOnDevice::m_sNumQueuesCreated;
#endif

// Initialize current pipeline command queue. Call it after Pipeline creation of Command list.
COINATIVELIBEXPORT
void init_commands_queue(uint32_t         in_BufferCount,
                         void**           in_ppBufferPointers,
                         uint64_t*        in_pBufferLengths,
                         void*            in_pMiscData,
                         uint16_t         in_MiscDataLength,
                         void*            in_pReturnValue,
                         uint16_t         in_ReturnValueLength)
{
    assert( sizeof(INIT_QUEUE_ON_DEVICE_STRUCT) == in_MiscDataLength );
    assert( nullptr != in_pMiscData );
    assert( sizeof(INIT_QUEUE_ON_DEVICE_OUTPUT_STRUCT) == in_ReturnValueLength );
    assert( nullptr != in_pReturnValue );

    INIT_QUEUE_ON_DEVICE_STRUCT*        data     = (INIT_QUEUE_ON_DEVICE_STRUCT*)in_pMiscData;
    INIT_QUEUE_ON_DEVICE_OUTPUT_STRUCT* data_out = (INIT_QUEUE_ON_DEVICE_OUTPUT_STRUCT*)in_pReturnValue;

    data_out->device_queue_addresses.device_sync_queue_address = nullptr;
    data_out->device_queue_addresses.device_async_queue_address = nullptr;

    QueueOnDevice* pAsyncQueue = nullptr;
    QueueOnDevice* pSyncQueue = nullptr;
    
    ThreadPool* thread_pool = ThreadPool::getInstance();
    assert( (nullptr != thread_pool) && "Thread pool not exists" );
    if ( nullptr == thread_pool )
    {
        data_out->ret_code = CL_DEV_INVALID_OPERATION;
        return;
    }
#ifdef __MIC_DA_OMP__
    if (0 < QueueOnDevice::m_sNumQueuesCreated++)
    {
        // At least one queue already created during the lifetime of this process
        data_out->ret_code = CL_DEV_INVALID_OPERATION;
        return;
    }
    thread_pool->startup_all_workers();
#endif

    pAsyncQueue = new QueueOnDevice( *thread_pool );
    if (nullptr == pAsyncQueue)
    {
        data_out->ret_code = CL_DEV_OUT_OF_MEMORY;
        return;
    }
    if (!pAsyncQueue->Init(data->is_in_order_queue))
    {
        data_out->ret_code = CL_DEV_OUT_OF_MEMORY;
        delete pAsyncQueue;
        return;        
    }
	// create SyncQueueOnDevice only for inorder queue
    if (data->is_in_order_queue)
    {
        pSyncQueue = new SyncQueueOnDevice( *thread_pool );
        if (nullptr == pSyncQueue)
        {
            data_out->ret_code = CL_DEV_OUT_OF_MEMORY;
            return;
        }
        if (!pSyncQueue->Init())
        {
            data_out->ret_code = CL_DEV_OUT_OF_MEMORY;
            delete pAsyncQueue;
            delete pSyncQueue;
            return;        
        }
    }

#if defined(USE_ITT)
    if ( gMicGPAData.bUseGPA )
    {
        __itt_thread_set_name("MIC Device Queue Thread");
    }
#endif

    data_out->device_queue_addresses.device_sync_queue_address = (uint64_t)(size_t)pSyncQueue;
    data_out->device_queue_addresses.device_async_queue_address = (uint64_t)(size_t)pAsyncQueue;
    data_out->ret_code             = CL_DEV_SUCCESS;
}

// release current pipeline command queue. Call it before Pipeline destruction of Command list.
COINATIVELIBEXPORT
void release_commands_queue(uint32_t         in_BufferCount,
                            void**           in_ppBufferPointers,
                            uint64_t*        in_pBufferLengths,
                            void*            in_pMiscData,
                            uint16_t         in_MiscDataLength,
                            void*            in_pReturnValue,
                            uint16_t         in_ReturnValueLength)
{
    assert( (nullptr!=in_pMiscData) &&  sizeof(DEVICE_QUEUE_STRUCT) == in_MiscDataLength        && "Expected queue pointer in function parameters");
    assert( (nullptr!=in_pReturnValue) &&  sizeof(cl_dev_err_code) == in_ReturnValueLength && "Expected return argument");

    DEVICE_QUEUE_STRUCT* pDeviceQueueAddresses = (DEVICE_QUEUE_STRUCT*)in_pMiscData;
    QueueOnDevice* pAsyncQueue = (QueueOnDevice*)pDeviceQueueAddresses->device_async_queue_address;
    QueueOnDevice* pSyncQueue = (QueueOnDevice*)pDeviceQueueAddresses->device_sync_queue_address;

    if ( nullptr != pAsyncQueue )
    {
        delete pAsyncQueue;
    }
    if ( nullptr != pSyncQueue )
    {
        delete pSyncQueue;
    }
    *((cl_dev_err_code*)in_pReturnValue) = CL_DEV_SUCCESS;
}

bool QueueOnDevice::Init( bool isInOrder)
{
    bool res = initInt( isInOrder ? TE_CMD_LIST_IN_ORDER : TE_CMD_LIST_OUT_OF_ORDER );
    if (res)
    {
        // affinitize the thread in OOO queue only, in order queue create also SyncQueueOnDevice that affinitize the thread.
        if (!isInOrder)
        {
            m_thread_pool.AffinitizeMasterThread();
        }
    }
    return res;
}

cl_dev_err_code QueueOnDevice::Execute( TaskHandlerBase* task_handler )
{
    executeInt( task_handler );
    m_task_list->Flush();
    return CL_DEV_SUCCESS;
}

void QueueOnDevice::Cancel() const
{
    m_task_list->Cancel();
    m_task_list->Flush();
}

bool QueueOnDevice::initInt( TE_CMD_LIST_TYPE cmdListType )
{
    m_task_list = m_thread_pool.getRootDevice()->CreateTaskList(
                             CommandListCreationParam( cmdListType, gMicExecEnvOptions.tbb_scheduler ));

    if (nullptr == m_task_list)
    {
        //Report Error
        NATIVE_PRINTF("Cannot create out-of-order TaskList\n");
        return false;
    }

    return true;
}

cl_dev_err_code QueueOnDevice::executeInt( TaskHandlerBase* task_handler )
{
    task_handler->PrepareTask();
    ITaskBase* task = task_handler->GetAsITaskBase();
    m_task_list->Enqueue( task );
    return CL_DEV_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
SyncQueueOnDevice::~SyncQueueOnDevice()
{
    m_task_list = nullptr;
    m_thread_pool.DeactivateCurrentMasterThread();
}

// return false on error
bool SyncQueueOnDevice::Init( bool isInOrder )
{
    bool res = initInt( TE_CMD_LIST_IMMEDIATE );
    if (res)
    {
        m_thread_pool.ActivateCurrentMasterThread();
    }
    return res;
}

cl_dev_err_code SyncQueueOnDevice::Execute( TaskHandlerBase* task_handler )
{
    return executeInt( task_handler );
}
