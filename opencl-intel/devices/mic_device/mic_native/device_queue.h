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

#pragma once

#include "thread_local_storage.h"
#include "mic_device_interface.h"
#include "native_thread_pool.h"
#include "mic_tracer.h"
#include "task_handler.h"

#include <cl_device_api.h>
#include <cl_shared_ptr.h>
#include <task_executor.h>

#include <common/COIEvent_common.h>

namespace Intel { namespace OpenCL { namespace MICDeviceNative {

/* QueueOnDevice is a class that manage the execution of a task. */
class QueueOnDevice
{
public:

    QueueOnDevice( ThreadPool& thread_pool ) : m_thread_pool(thread_pool) {};

    virtual ~QueueOnDevice() 
    {
        m_thread_pool.ReturnAffinitizationResource();
    };

    // return false on error
    virtual bool Init( bool isInOrder = true );

    /* Run the task */
    virtual cl_dev_err_code Execute( TaskHandlerBase* task_handler);

    virtual bool IsAsyncExecution() { return true; };

    void Cancel() const;

    virtual bool FinishTask( TaskHandlerBase* pTask ) { return pTask->FinishAsyncTask(); };

    virtual void CancelTask( TaskHandlerBase* pTask ) { pTask->CancelAsyncTask(); };

    virtual long ReleaseTask( TaskHandlerBase* pTask ) { return pTask->ReleaseAsyncTask(); };

#ifdef __MIC_DA_OMP__
    static Intel::OpenCL::Utils::AtomicCounter m_sNumQueuesCreated;
#endif

    Intel::OpenCL::TaskExecutor::ITaskList* GetTaskList() const { return m_task_list.GetPtr();}
protected:

	// return false on error
    bool initInt( Intel::OpenCL::TaskExecutor::TE_CMD_LIST_TYPE cmdListType );

	/* Run the task */
    cl_dev_err_code executeInt( TaskHandlerBase* task_handler);

    ThreadPool&                                                              m_thread_pool;
    Intel::OpenCL::Utils::SharedPtr<Intel::OpenCL::TaskExecutor::ITaskList>  m_task_list;
};

/* Immediate execution of a Task. */
class SyncQueueOnDevice : public QueueOnDevice
{
public:

    SyncQueueOnDevice( ThreadPool& thread_pool ) : QueueOnDevice( thread_pool ) {};
    ~SyncQueueOnDevice();

	// return false on error
    virtual bool Init( bool isInOrder = true );

    /* Execute the task */
    virtual cl_dev_err_code Execute( TaskHandlerBase* task_handler);

    virtual bool IsAsyncExecution() { return false; };

    virtual bool FinishTask( TaskHandlerBase* pTask ) { return pTask->FinishSyncTask(); };

    virtual void CancelTask( TaskHandlerBase* pTask ) { pTask->CancelSyncTask(); };
	
    virtual long ReleaseTask( TaskHandlerBase* pTask ) { return pTask->ReleaseSyncTask(); };
};

}}}
