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

#include <cl_shared_ptr.h>
#include <task_executor.h>

#include "cl_thread.h"
#include "tbb_memory_allocator.h"
#include <malloc.h>

#include "native_program_service.h"

namespace Intel { namespace OpenCL { namespace MICDeviceNative {

class TaskReleaseHandler;

//
// TaskHandlerBase - state of specific task handled by QueueOnDevice
//
class TaskHandlerBase : virtual public Intel::OpenCL::Utils::ReferenceCountedObject
{
    friend class TaskReleaseHandler;
public:
    PREPARE_SHARED_PTR(TaskHandlerBase);

    TaskHandlerBase(
        uint32_t lockBufferCount, void** pLockBuffers, QueueOnDevice* pQueue
#ifdef ENABLE_MIC_TRACER
        , size_t* pLockBufferSizes
#endif
        );

    virtual ~TaskHandlerBase() {};
	
 // overload new and delete operators in order to use scalable allocator of tbb for new tasks.
    void* operator new (std::size_t size) throw (std::bad_alloc)
    {
        return Intel::OpenCL::TaskExecutor::ScalableMemAllocator::scalableMalloc(size);
    }

    void operator delete (void* ptr) throw()
    {
        Intel::OpenCL::TaskExecutor::ScalableMemAllocator::scalableFree(ptr);
    }

    // Retrieve a pointer to ITaskBase
    virtual Intel::OpenCL::TaskExecutor::ITaskBase* GetAsITaskBase() = 0;
    virtual TaskHandlerBase*                        GetAsTaskHandlerBase() { return this;}

    // Called before enqueue into the queue
    virtual bool PrepareTask() = 0;

    virtual bool FinishSyncTask() = 0;
    virtual bool FinishAsyncTask() = 0;
    virtual bool CancelAsyncTask() = 0;
    virtual bool CancelSyncTask() = 0;
    virtual long ReleaseSyncTask() { return 0; };
    virtual long ReleaseAsyncTask() { delete this; return 0; };

    void setTaskError( cl_dev_err_code errorCode )
    {
        if ( CL_DEV_SUCCEEDED(m_errorCode) )
        {
            m_errorCode = errorCode;
        }
    }
    cl_dev_err_code getTaskError() const { return m_errorCode; };

#ifdef ENABLE_MIC_TRACER
    virtual const dispatcher_data& getDispatcherData() const = 0;
    CommandTracer& commandTracer() { return m_commandTracer; }
#endif

protected:

    virtual bool releaseResourcesAndSignal() = 0;

    uint32_t              m_bufferCount;
	// m_bufferPointers valid only before completion of PrepareTask() func (before enqueueing to the queue)
    void**                m_bufferPointers;
#ifdef ENABLE_MIC_TRACER
    size_t*               m_bufferSizes;
#endif
    cl_dev_err_code       m_errorCode;

#ifdef ENABLE_MIC_TRACER
    // Command tracer
    CommandTracer         m_commandTracer;
#endif
    TaskReleaseHandler* m_releasehandler;

private:

	Intel::OpenCL::Utils::SharedPtr<TaskHandlerBase> m_nextTaskToRelease;
};

template<class Command, typename dispatch_data_type > class TaskHandler : public TaskHandlerBase
{
public:
    TaskHandler(
        uint32_t lockBufferCount,
        void** pLockBuffers,
#ifdef ENABLE_MIC_TRACER
        size_t* pLockBufferSizes,
#endif
        dispatch_data_type* pDispatcherData,
        size_t uiDispatchSize,
        QueueOnDevice* pQueue
        ) :
        TaskHandlerBase( lockBufferCount, pLockBuffers, pQueue
#ifdef ENABLE_MIC_TRACER
        , pLockBufferSizes
#endif
        ),
        m_pDispatcherData(pDispatcherData),
        m_uiDispatchSize(uiDispatchSize),
        m_pQueue(pQueue),
        m_releaseDispatcherData(false)
        {
            if (m_pQueue->IsAsyncExecution())
            {
                // In case of Async execution we should copy the dispatcher data content to local address.
                assert(uiDispatchSize > 0 && "dispatch_data_type size should be greater than 0");
                if (uiDispatchSize <= sizeof(m_dispatcherDataLocalMem))
                {
                    m_pDispatcherData = (dispatch_data_type*)(&m_dispatcherDataLocalMem);
                }
                else
                {
                    m_pDispatcherData = (dispatch_data_type *)Intel::OpenCL::TaskExecutor::ScalableMemAllocator::scalableAlignedMalloc(uiDispatchSize, sizeof(size_t));
                    assert(m_pDispatcherData && "Allocate memory to m_dispatcherData failed");
                    m_releaseDispatcherData = true;
                }
                memcpy(m_pDispatcherData, pDispatcherData, uiDispatchSize);
		    }
        };

    virtual ~TaskHandler() 
	{
	    assert(m_pDispatcherData && "m_dispatcherData shuoldn't be NULL pointer");
        if (m_releaseDispatcherData)
        {
            Intel::OpenCL::TaskExecutor::ScalableMemAllocator::scalableAlignedFree(m_pDispatcherData);        }        m_pDispatcherData = nullptr;
	};

    virtual bool FinishSyncTask()
    {
#ifdef ENABLE_MIC_TRACER
        commandTracer().set_current_time_tbb_exe_in_device_time_end();
#endif
        return true;
    };

    virtual bool FinishAsyncTask()
    {
#if defined(USE_ITT) && defined(USE_ITT_INTERNAL)
        if ( gMicGPAData.bUseGPA )
        {
          static __thread __itt_string_handle* pTaskName = nullptr;
          if ( nullptr == pTaskName )
          {
            pTaskName = __itt_string_handle_create("Finish->m_releasehandler->addTask(this)");
          }
          __itt_task_begin(gMicGPAData.pDeviceDomain, __itt_null, __itt_null, pTaskName);
        }
#endif
        // In profiling mode let the TBB thread signal the completion (it also signaled the start) in order ignore races in profiling.
        if (m_pDispatcherData->profiling)
        {
            releaseResourcesAndSignal();
        }
        else
        {
            m_releasehandler->addTask(this);
        }
#if defined(USE_ITT) && defined(USE_ITT_INTERNAL)
        // Monitor only IN-ORDER queue
        if ( gMicGPAData.bUseGPA )
        {
          __itt_task_end(gMicGPAData.pDeviceDomain);
        }
#endif

	    return true;
    };

    virtual bool CancelSyncTask()
    {
#ifdef ENABLE_MIC_TRACER
        commandTracer().set_current_time_tbb_exe_in_device_time_start();
        commandTracer().set_current_time_tbb_exe_in_device_time_end();
#endif
        setTaskError( CL_DEV_COMMAND_CANCELLED );
		return true;
    };

    virtual bool CancelAsyncTask()
    {
        // TODO: What if task already started
        // Notify end if exists
        if ( m_pDispatcherData->startEvent.isRegistered )
        {
            COIEventSignalUserEvent(m_pDispatcherData->startEvent.cmdEvent);
        }
	
#ifdef ENABLE_MIC_TRACER
        commandTracer().set_current_time_tbb_exe_in_device_time_start();
#endif
        setTaskError( CL_DEV_COMMAND_CANCELLED );

	    m_releasehandler->addTask(this);
		return true;
    };



#ifdef ENABLE_MIC_TRACER
    virtual const dispatcher_data& getDispatcherData() const
    {
        return m_dispatcherData;
    }
#endif

protected:
    // The received dispatcher_data
    dispatch_data_type* m_pDispatcherData;

    long    releaseImp() { return m_pQueue->ReleaseTask(this); };

    virtual bool releaseResourcesAndSignal()
	{
#ifdef ENABLE_MIC_TRACER
        commandTracer().set_current_time_tbb_exe_in_device_time_end();
#endif

        // Notify end if exists
        if ( m_pDispatcherData->endEvent.isRegistered )
        {
            COIEventSignalUserEvent(m_pDispatcherData->endEvent.cmdEvent);
        }

	    return true;
	}

    size_t               m_uiDispatchSize;
	
    QueueOnDevice* m_pQueue;

private:
    // Valid on Async task only and only if the dispatcher data size is not greater than this block of memory
    char m_dispatcherDataLocalMem[4096];
    bool m_releaseDispatcherData;

    // operator assigne is not allowed
    TaskHandler& operator= (const TaskHandler& o);
};

class TaskReleaseHandler : public OclThread
{
public:

	static TaskReleaseHandler* getInstance()
	{
		// Assume that only one thread will initiate the singleton
		if (nullptr == m_singleton)
		{
			m_singleton = new TaskReleaseHandler();
			m_singleton->Start();
			// Wait until the thread completes its initialization
			while (!m_singleton->initDone()) 
			{ 
				hw_pause();
			}
		}
		return m_singleton;
	};

	static void releaseInstance() 
	{ 
		// Assume that only one thread will destruct the singleton
		delete m_singleton;
		m_singleton = nullptr;
	}

	void addTask(const Intel::OpenCL::Utils::SharedPtr< TaskHandlerBase >& task);

private:

	class DummyTask : public TaskHandlerBase
	{
	public:
		// TODO: maybe create new dummy constructor to TaskHandlerBase and call it instead.
		DummyTask() :
            TaskHandlerBase( 1, &m_dummyBuffer, nullptr
#ifdef ENABLE_MIC_TRACER
            , &m_dummyBufferSize
#endif
            )
		{
		}
	private:

        virtual Intel::OpenCL::TaskExecutor::ITaskBase* GetAsITaskBase() { return nullptr; };
        virtual bool PrepareTask() { return true; };
        virtual bool releaseResourcesAndSignal() { return true; };
        virtual bool FinishSyncTask() { return true; };
        virtual bool FinishAsyncTask() { return true; };
        virtual bool CancelSyncTask() { return true; };
        virtual bool CancelAsyncTask() { return true; };

        static void* m_dummyBuffer;
        static size_t m_dummyBufferSize;
	};

	TaskReleaseHandler();
	~TaskReleaseHandler();

	RETURN_TYPE_ENTRY_POINT Run();

	bool initDone() { return ( 1 == m_initDone ); };

	Intel::OpenCL::Utils::SharedPtr< TaskHandlerBase > m_head;
	Intel::OpenCL::Utils::SharedPtr< TaskHandlerBase > m_tail;

	OclSpinMutex        m_mutex;
	OclOsDependentEvent m_event;

	volatile bool m_finish;

	volatile long m_initDone;

#ifdef USE_ITT
    __itt_string_handle*        m_pIttTaskReleaseName;
    __itt_domain*               m_pIttTaskReleaseDomain;
#endif

	static TaskReleaseHandler* m_singleton;
};

}}}
