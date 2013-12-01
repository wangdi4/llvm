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

/* QueueOnDevice is an abstract class that manage the execution of a task. */
class QueueOnDevice
{
public:
    virtual ~QueueOnDevice() {};

    // return false on error
    virtual bool Init() = 0;

    /* Run the task */
    virtual cl_dev_err_code Execute( TaskHandlerBase* task_handler) = 0;

    void Cancel() const;

#ifdef _DEBUG
    static QueueOnDevice* getCurrentQueue( UtilsNative::TlsAccessor* tlsAccessor )
        {
            assert( NULL != tlsAccessor );
            UtilsNative::QueueTls queueTls(tlsAccessor);
            return (QueueOnDevice*)(queueTls.getTls(UtilsNative::QueueTls::QUEUE_TLS_ENTRY));
        };

    static void setCurrentQueue( UtilsNative::TlsAccessor* tlsAccessor, QueueOnDevice* queue )
        {
            assert( NULL != tlsAccessor );
            UtilsNative::QueueTls queueTls(tlsAccessor);
            queueTls.setTls(UtilsNative::QueueTls::QUEUE_TLS_ENTRY, queue);
        };
#endif

	static QueueOnDevice* createQueueOnDevice( bool is_in_order );

#ifdef __MIC_DA_OMP__
    static Intel::OpenCL::Utils::AtomicCounter m_sNumQueuesCreated;
#endif

protected:
    QueueOnDevice( ThreadPool& thread_pool ) : m_thread_pool(thread_pool) {};

    ThreadPool&                                                              m_thread_pool;
    Intel::OpenCL::Utils::SharedPtr<Intel::OpenCL::TaskExecutor::ITaskList>  m_task_list;
};

/* BlockingTaskHandler inherits from "TaskHandler" and implements the functionality for Blocking task management. */
class InOrderQueueOnDevice : public QueueOnDevice
{

public:
    InOrderQueueOnDevice( ThreadPool& thread_pool ) : QueueOnDevice( thread_pool ) {}
    ~InOrderQueueOnDevice();

    /* Execute the task */
    virtual cl_dev_err_code Execute( TaskHandlerBase* task_handler);
    //virtual cl_dev_err_code ExecuteImpl( const SharedPtr<Intel::OpenCL::TaskExecutor::ITaskBase>& pTask );

    // return false on error
    bool Init();
};


class OutOfOrderQueueOnDevice : public QueueOnDevice
{
public:
    OutOfOrderQueueOnDevice( ThreadPool& thread_pool ) : QueueOnDevice( thread_pool ) {}
    ~OutOfOrderQueueOnDevice() {};

    /* Execute the task */
    virtual cl_dev_err_code Execute( TaskHandlerBase* task_handler);
    //virtual cl_dev_err_code ExecuteImpl( const TaskHandlerBase* task );

    // return false on error
    bool Init();
};

}}}

