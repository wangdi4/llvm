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

#ifdef MIC_COMMAND_BATCHING_OPTIMIZATION
#include "cl_thread.h"
#include "tbb_memory_allocator.h"
#endif
#include <malloc.h>

#include "native_program_service.h"

namespace Intel { namespace OpenCL { namespace MICDeviceNative {

#ifdef MIC_COMMAND_BATCHING_OPTIMIZATION
class TaskReleaseHandler;
#endif

//
// TaskHandlerBase - state of specific task handled by QueueOnDevice
//
class TaskHandlerBase : virtual public Intel::OpenCL::Utils::ReferenceCountedObject
{
#ifdef MIC_COMMAND_BATCHING_OPTIMIZATION
    friend class TaskReleaseHandler;
#endif
public:
    PREPARE_SHARED_PTR(TaskHandlerBase);

    TaskHandlerBase(
        uint32_t lockBufferCount, void** pLockBuffers
#ifdef ENABLE_MIC_TRACER
        , size_t* pLockBufferSizes
#endif
        );

    virtual ~TaskHandlerBase() {};
	
#ifdef MIC_COMMAND_BATCHING_OPTIMIZATION	
 // overload new and delete operators in order to use scalable allocator of tbb for new tasks.
    void* operator new (std::size_t size) throw (std::bad_alloc)
    {
        return Intel::OpenCL::TaskExecutor::ScalableMemAllocator::scalableMalloc(size);
    }

    void operator delete (void* ptr) throw()
    {
        Intel::OpenCL::TaskExecutor::ScalableMemAllocator::scalableFree(ptr);
    }
#else
    // Creates a copy of the task
    virtual TaskHandlerBase*     Duplicate() const = 0;
#endif

    // Retrieve a pointer to ITaskBase
    virtual Intel::OpenCL::TaskExecutor::ITaskBase* GetAsITaskBase() = 0;
#ifndef MIC_COMMAND_BATCHING_OPTIMIZATION
    virtual TaskHandlerBase*                        GetAsTaskHandlerBase() { return this;}
#endif

    // Called before enqueue into the queue
    virtual bool PrepareTask() = 0;

    bool FiniTask();

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
#ifndef MIC_COMMAND_BATCHING_OPTIMIZATION
    TaskHandlerBase(const TaskHandlerBase& o);
#else 
    virtual bool releaseResourcesAndSignal() = 0;
#endif

    uint32_t              m_bufferCount;
    void**                m_bufferPointers;
#ifdef ENABLE_MIC_TRACER
    size_t*               m_bufferSizes;
#endif
    cl_dev_err_code       m_errorCode;

#ifndef MIC_COMMAND_BATCHING_OPTIMIZATION
    bool                  m_bDuplicated;
#endif

#ifdef ENABLE_MIC_TRACER
    // Command tracer
    CommandTracer         m_commandTracer;
#endif
#ifdef MIC_COMMAND_BATCHING_OPTIMIZATION
    TaskReleaseHandler* m_releasehandler;

private:

	Intel::OpenCL::Utils::SharedPtr<TaskHandlerBase> m_nextTaskToRelease;
#endif

};

template<class Command, typename dispatch_data_type > class TaskHandler : public TaskHandlerBase
{
public:
    TaskHandler(
        //const QueueOnDevice* pQueue,
        uint32_t lockBufferCount,
        void** pLockBuffers,
#ifdef ENABLE_MIC_TRACER
        size_t* pLockBufferSizes,
#endif
        dispatch_data_type* pDispatcherData,
        size_t uiDispatchSize
        ) :
        TaskHandlerBase(/*pQueue,*/ lockBufferCount, pLockBuffers
#ifdef ENABLE_MIC_TRACER
        , pLockBufferSizes
#endif
        )
#ifndef MIC_COMMAND_BATCHING_OPTIMIZATION
		,
        m_dispatcherData(pDispatcherData),
        m_uiDispatchSize(uiDispatchSize)
#endif
    {
#ifdef MIC_COMMAND_BATCHING_OPTIMIZATION
        assert(uiDispatchSize > 0 && "dispatch_data_type size should be greater than 0");
        m_dispatcherData = (dispatch_data_type *)Intel::OpenCL::TaskExecutor::ScalableMemAllocator::scalableAlignedMalloc(uiDispatchSize, sizeof(size_t));
        assert(m_dispatcherData && "Allocate memory to m_dispatcherData failed");
        memcpy(m_dispatcherData, pDispatcherData, uiDispatchSize);
#endif
    }

    virtual ~TaskHandler()
    {
#ifdef MIC_COMMAND_BATCHING_OPTIMIZATION
        assert(m_dispatcherData && "m_dispatcherData shuoldn't be NULL pointer");
        Intel::OpenCL::TaskExecutor::ScalableMemAllocator::scalableAlignedFree(m_dispatcherData);
        m_dispatcherData = NULL;
#else
        if ( m_bDuplicated )
        {
            free(m_dispatcherData);
            m_dispatcherData = NULL;
        }
#endif 
    }

#ifndef MIC_COMMAND_BATCHING_OPTIMIZATION
    virtual TaskHandlerBase* Duplicate() const
    {
        // TODO: convert to tbb::scalable_allocator and allocate memory for whole object at a time
        TaskHandler* pNewHandler = new Command( this->GetAsCommandTypeConst() );
        if ( NULL == pNewHandler )
        {
            assert (0 && "Task duplication failed" );
            return NULL;
        }
        // Copy buffers and kernel parameters
        return pNewHandler;
    }

    virtual const Command&       GetAsCommandTypeConst() const = 0;
#endif

#ifdef ENABLE_MIC_TRACER
    virtual const dispatcher_data& getDispatcherData() const
    {
        return m_dispatcherData;
    }
#endif

protected:
    // The received dispatcher_data
    dispatch_data_type* m_dispatcherData;

#ifdef MIC_COMMAND_BATCHING_OPTIMIZATION
    virtual bool releaseResourcesAndSignal()
	{
		// Release COI resources, before signaling to runtime
        FiniTask();

#ifdef ENABLE_MIC_TRACER
        commandTracer().set_current_time_tbb_exe_in_device_time_end();
#endif

        // Notify end if exists
        if ( m_dispatcherData->endEvent.isRegistered )
        {
            COIEventSignalUserEvent(m_dispatcherData->endEvent.cmdEvent);
        }

	    return true;
	}
#else
    size_t              m_uiDispatchSize;


    TaskHandler(const TaskHandler& o) :
        TaskHandlerBase(o),
        m_uiDispatchSize(o.m_uiDispatchSize)
    {
        m_dispatcherData = (dispatch_data_type *)memalign(sizeof(size_t), m_uiDispatchSize);
        memcpy(m_dispatcherData, o.m_dispatcherData, m_uiDispatchSize);
    }
#endif
private:
    // operator assigne is not allowed
    TaskHandler& operator= (const TaskHandler& o);
};

#ifdef MIC_COMMAND_BATCHING_OPTIMIZATION
class TaskReleaseHandler : public OclThread
{
public:

	static TaskReleaseHandler* getInstance()
	{
		// Assume that only one thread will initiate the singleton
		if (NULL == m_singleton)
		{
			m_singleton = new TaskReleaseHandler();
			m_singleton->Start();
		}
		return m_singleton;
	};

	static void releaseInstance() 
	{ 
		// Assume that only one thread will destruct the singleton
		delete m_singleton;
		m_singleton = NULL;
	}

	void addTask(const Intel::OpenCL::Utils::SharedPtr< TaskHandlerBase >& task);

private:

	class DummyTask : public TaskHandlerBase
	{
	public:
		// TODO: maybe create new dummy constructor to TaskHandlerBase and call it instead.
		DummyTask() :
            TaskHandlerBase(/*pQueue,*/ 1, &m_dummyBuffer
#ifdef ENABLE_MIC_TRACER
            , &m_dummyBufferSize
#endif
            )
		{
		}
	private:

		virtual Intel::OpenCL::TaskExecutor::ITaskBase* GetAsITaskBase() { return NULL; };
		virtual bool PrepareTask() { return true; };
		virtual bool releaseResourcesAndSignal() { return true; };

		static void* m_dummyBuffer;
		static size_t m_dummyBufferSize;
	};

	TaskReleaseHandler();
	~TaskReleaseHandler();

	RETURN_TYPE_ENTRY_POINT Run();

	Intel::OpenCL::Utils::SharedPtr< TaskHandlerBase > m_head;
	Intel::OpenCL::Utils::SharedPtr< TaskHandlerBase > m_tail;

	OclSpinMutex        m_mutex;
	OclOsDependentEvent m_event;

	volatile bool m_finish;

#ifdef USE_ITT
    __itt_string_handle*        m_pIttTaskReleaseName;
    __itt_domain*               m_pIttTaskReleaseDomain;
#endif

	static TaskReleaseHandler* m_singleton;
};
#endif

}}}
