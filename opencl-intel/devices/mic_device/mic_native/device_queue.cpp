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

#include "native_ndrange_task.h"
#include "native_buffer_commands.h"

#include <ocl_itt.h>

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
    assert( NULL != in_pMiscData );
    assert( sizeof(INIT_QUEUE_ON_DEVICE_OUTPUT_STRUCT) == in_ReturnValueLength );
    assert( NULL != in_pReturnValue );

    INIT_QUEUE_ON_DEVICE_STRUCT*        data     = (INIT_QUEUE_ON_DEVICE_STRUCT*)in_pMiscData;
    INIT_QUEUE_ON_DEVICE_OUTPUT_STRUCT* data_out = (INIT_QUEUE_ON_DEVICE_OUTPUT_STRUCT*)in_pReturnValue;


    data_out->device_queue_address = NULL;

#ifdef _DEBUG
    TlsAccessor tlsAccessor;
    assert( NULL == QueueOnDevice::getCurrentQueue(&tlsAccessor) && "Queue is already set");
    if ( NULL != QueueOnDevice::getCurrentQueue(&tlsAccessor) )
    {
        data_out->ret_code = CL_DEV_INVALID_OPERATION;
        return;
    }
#endif
    QueueOnDevice* pQueue = NULL;
    
    ThreadPool* thread_pool = ThreadPool::getInstance();
    assert( (NULL != thread_pool) && "Thread pool not exists" );
    if ( NULL == thread_pool )
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

    if (data->is_in_order_queue)
    {
        pQueue = new InOrderQueueOnDevice( *thread_pool );
    }
    else
    {
        pQueue = new OutOfOrderQueueOnDevice( *thread_pool );
    }
    
    if ( NULL == pQueue )
    {
        data_out->ret_code = CL_DEV_OUT_OF_MEMORY;
        return;
    }

    if (!pQueue->Init())
    {
        data_out->ret_code = CL_DEV_OUT_OF_MEMORY;
        delete pQueue;
        return;        
    }

#if defined(USE_ITT)
    if ( gMicGPAData.bUseGPA )
    {
        __itt_thread_set_name("MIC Device Queue Thread");
    }
#endif

#ifdef _DEBUG
    QueueOnDevice::setCurrentQueue( &tlsAccessor, pQueue );
#endif
    data_out->device_queue_address = (uint64_t)(size_t)pQueue;
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
    assert( (NULL!=in_pMiscData) &&  sizeof(QueueOnDevice*) == in_MiscDataLength        && "Expected queue pointer in function parameters");
    assert( (NULL!=in_pReturnValue) &&  sizeof(cl_dev_err_code) == in_ReturnValueLength && "Expected return argument");

    QueueOnDevice* pQueue = (QueueOnDevice*)(*(uint64_t*)in_pMiscData);
    assert(NULL != pQueue && "pQueue must be valid");

#ifdef _DEBUG
    TlsAccessor tlsAccessor;
    const QueueOnDevice* pTLSQueue = QueueOnDevice::getCurrentQueue( &tlsAccessor );
    assert( pTLSQueue == pQueue && "Queue handles doesn't match");
    QueueOnDevice::setCurrentQueue( &tlsAccessor, NULL );
#endif

    if ( NULL != pQueue )
    {
        delete pQueue;
    }
    *((cl_dev_err_code*)in_pReturnValue) = CL_DEV_SUCCESS;
}

void QueueOnDevice::Cancel() const
{
    m_task_list->Cancel();
    m_task_list->Flush();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
InOrderQueueOnDevice::~InOrderQueueOnDevice()
{
    m_thread_pool.DeactivateCurrentMasterThread();
}

// return false on error
bool InOrderQueueOnDevice::Init()
{
    m_task_list = m_thread_pool.getRootDevice()->CreateTaskList( 
                            CommandListCreationParam( TE_CMD_LIST_IMMEDIATE, gMicExecEnvOptions.tbb_scheduler ));

    if (NULL == m_task_list)
    {
        //Report Error
        NATIVE_PRINTF("Cannot create in-order TaskList\n");
        return false;
    }
    
    m_thread_pool.ActivateCurrentMasterThread();

    return true;
}

cl_dev_err_code InOrderQueueOnDevice::Execute( TaskHandlerBase* task_handler )
{
    task_handler->PrepareTask();

    ITaskBase* pTask = task_handler->GetAsITaskBase();

    m_task_list->Enqueue( pTask );
    m_task_list->WaitForCompletion(NULL);

    return CL_DEV_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool OutOfOrderQueueOnDevice::Init()
{
    m_task_list = m_thread_pool.getRootDevice()->CreateTaskList(
                             CommandListCreationParam( TE_CMD_LIST_OUT_OF_ORDER, gMicExecEnvOptions.tbb_scheduler ));

    if (NULL == m_task_list)
    {
        //Report Error
        NATIVE_PRINTF("Cannot create out-of-order TaskList\n");
        return false;
    }

    return true;
}

cl_dev_err_code OutOfOrderQueueOnDevice::Execute( TaskHandlerBase* task_handler )
{
    TaskHandlerBase* newOOOTask = task_handler->Duplicate();

    newOOOTask->PrepareTask();

    ITaskBase* oooTask = newOOOTask->GetAsITaskBase();

    m_task_list->Enqueue( oooTask );
    m_task_list->Flush();

    return CL_DEV_SUCCESS;
}
