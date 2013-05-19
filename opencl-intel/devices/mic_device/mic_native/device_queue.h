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

/////////////////////////////////////////////////////////////
//  ExecutionTask.h
//  Implementation of the Class ExecutionTask
//  Class Object is responsible on execution of NDRange task
/////////////////////////////////////////////////////////////

#pragma once

#include "thread_local_storage.h"
#include "mic_device_interface.h"
#include "cl_device_api.h"
#include "mic_tracer.h"
#include "cl_shared_ptr.h"
#include "task_executor.h"

#include <common/COIEvent_common.h>

namespace Intel { namespace OpenCL { namespace MICDeviceNative {

using namespace Intel::OpenCL::MICDevice;
using namespace Intel::OpenCL::UtilsNative;

class QueueOnDevice;
class ThreadPool;

// Note: this enum must be parallel to the TaskHandler::m_native_command_allocators[] array
enum TASK_TYPES
{
    NDRANGE_TASK_TYPE = 0,
    FILL_MEM_OBJ_TYPE,

    LAST_TASK_TYPE   // must be the last entry
};

//
// TaskHandler - state of specific task handled by QueueOnDevice
//
class TaskHandler : virtual public Intel::OpenCL::Utils::ReferenceCountedObject
{
public:
    PREPARE_SHARED_PTR(TaskHandler)
    
    // called immediately after creation and after filling the COI-passed data
    virtual bool InitTask() = 0;

    // must be called at the very end of the ITaskBase finish stage and 
    // must call to QueueOnDevice->FinishTask() at the very end of itself
    virtual void FinishTask() = 0;

    void setTaskError( cl_dev_err_code errorCode );
    cl_dev_err_code getTaskError() const { return m_errorCode; };

    const QueueOnDevice& queue() const { return m_queue; }
    CommandTracer& commandTracer() { return m_commandTracer; }

    // Factory for TaskHandler object 
    static SharedPtr<TaskHandler> TaskFactory(TASK_TYPES type, QueueOnDevice& queue, dispatcher_data* dispatcherData, misc_data* miscData);

protected:
    
    virtual ~TaskHandler();
    TaskHandler( const QueueOnDevice& queue );

    // The received dispatcher_data
    dispatcher_data* m_dispatcherData;
    // The received misc_data
    misc_data*       m_miscData;

    // The input from the main function
    uint32_t         m_lockBufferCount;
    void**           m_lockBufferPointers;
    uint64_t*        m_lockBufferLengths;

private:
    const QueueOnDevice&  m_queue;

    cl_dev_err_code       m_errorCode;
    
    // Command tracer
    CommandTracer         m_commandTracer;
    
    typedef SharedPtr<TaskHandler> (*CommandAllocateFunc)( const QueueOnDevice& queue );
    
    // Note: this array must be parallel to TASK_TYPES enumerator
    static CommandAllocateFunc m_native_command_allocators[ LAST_TASK_TYPE ];

    friend class QueueOnDevice;
    friend class InOrderQueueOnDevice;
    friend class OutOfOrderQueueOnDevice;
};

/* QueueOnDevice is an abstract class that manage the execution of a task. */
class QueueOnDevice
{
public:
    // return false on error
    virtual bool Init() = 0;
	virtual ~QueueOnDevice() {};

	/* Initializing the task */
	virtual bool InitTask(  const SharedPtr<TaskHandler>& task_handler,
	                        dispatcher_data* dispatcherData, misc_data* miscData, 
	                        uint32_t in_BufferCount, void** in_ppBufferPointers, uint64_t* in_pBufferLengths, 
	                        void* in_pMiscData, uint16_t in_MiscDataLength) const = 0;

    /* Task Allocation Failed */
	virtual void NotifyTaskAllocationFailed(  
	                        dispatcher_data* dispatcherData, misc_data* miscData ) const = 0;

	/* Run the task */
	virtual void RunTask( const SharedPtr<TaskHandler>& task_handler ) const;

	/* It will call from 'run()' method (of m_task) as the first command in order to inform that the command start in case of NonBlocking task. */
	virtual void SignalTaskStart( const SharedPtr<TaskHandler>& task_handler ) const = 0;

	/* It will be call from 'run()' method (of m_task) as the last command,
	   It will release the resources and singal the user barrier if needed. 
	   It also delete this object as the last command. 
	   The FinishTask is not public because We don't want the user to release the resource. (It will release itself when completed)*/
	virtual void FinishTask(const SharedPtr<TaskHandler>& task_handler, COIEVENT& completionBarrier, bool isLegalBarrier) const = 0;

	virtual bool isInOrder() const = 0;

	static QueueOnDevice* getCurrentQueue( TlsAccessor* tlsAccessor )
        {
            assert( NULL != tlsAccessor );
            QueueTls queueTls(tlsAccessor);
            return (QueueOnDevice*)(queueTls.getTls(QueueTls::QUEUE_TLS_ENTRY)); 
        };

	static void setCurrentQueue( TlsAccessor* tlsAccessor, QueueOnDevice* queue )
        {
            assert( NULL != tlsAccessor );
            QueueTls queueTls(tlsAccessor);
            queueTls.setTls(QueueTls::QUEUE_TLS_ENTRY, queue); 
        };

	static QueueOnDevice* createQueueOnDevice( bool is_in_order );

	// helper routine
	static void execute_command(
				uint32_t				in_BufferCount,
				void**					in_ppBufferPointers,
				uint64_t*				in_pBufferLengths,
				void*						in_pMiscData,
				uint16_t				in_MiscDataLength,
				void*						in_pReturnValue,
				uint16_t				in_ReturnValueLength,
				TASK_TYPES	    taskType);

protected:

	QueueOnDevice( ThreadPool& thread_pool ) : m_thread_pool(thread_pool) {};

    ThreadPool&                                 m_thread_pool;
    Intel::OpenCL::Utils::SharedPtr<Intel::OpenCL::TaskExecutor::ITaskList>  m_task_list;
};

/* BlockingTaskHandler inherits from "TaskHandler" and implements the functionality for Blocking task managment. */
class InOrderQueueOnDevice : public QueueOnDevice
{

public:
    InOrderQueueOnDevice( ThreadPool& thread_pool ) : QueueOnDevice( thread_pool ) {}
    ~InOrderQueueOnDevice();

    // return false on error
    bool Init();

    bool InitTask(  const SharedPtr<TaskHandler>& task_handler,
                    dispatcher_data* dispatcherData, misc_data* miscData, 
                    uint32_t in_BufferCount, void** in_ppBufferPointers, uint64_t* in_pBufferLengths, 
                    void* in_pMiscData, uint16_t in_MiscDataLength) const;

    void NotifyTaskAllocationFailed(
 	                dispatcher_data* dispatcherData, misc_data* miscData ) const {};

    /* Do nothing in case of in order because the task start immediately. */
    void SignalTaskStart( const SharedPtr<TaskHandler>& task_handler ) const {};

    void FinishTask(const SharedPtr<TaskHandler>& task_handler, COIEVENT& completionBarrier, bool isLegalBarrier) const;
    
    bool isInOrder() const { return true; };
};


class OutOfOrderQueueOnDevice : public QueueOnDevice
{
public:
    OutOfOrderQueueOnDevice( ThreadPool& thread_pool ) : QueueOnDevice( thread_pool ) {}
    ~OutOfOrderQueueOnDevice() {};

    // return false on error
    bool Init();

    bool InitTask(  const SharedPtr<TaskHandler>& task_handler,
                    dispatcher_data* dispatcherData, misc_data* miscData, 
                    uint32_t in_BufferCount, void** in_ppBufferPointers, 
                    uint64_t* in_pBufferLengths, void* in_pMiscData, uint16_t in_MiscDataLength) const;

    void NotifyTaskAllocationFailed(
	                dispatcher_data* dispatcherData, misc_data* miscData ) const;

    void SignalTaskStart( const SharedPtr<TaskHandler>& task_handler ) const;

    void FinishTask(const SharedPtr<TaskHandler>& task_handler, COIEVENT& completionBarrier, bool isLegalBarrier) const;

    bool isInOrder() const { return false; };
};

}}}

