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

#include "task_handler.h"
#include "native_thread_pool.h"
#include "tbb_memory_allocator.h"
#include "device_queue.h"

#include <cl_shared_ptr.hpp>
#include <sink/COIBuffer_sink.h>
#include <common/COIPerf_common.h>

using namespace Intel::OpenCL::MICDeviceNative;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
TaskHandlerBase::TaskHandlerBase(
    uint32_t lockBufferCount, void** pLockBuffers, QueueOnDevice* pQueue
#ifdef ENABLE_MIC_TRACER
    , size_t* pLockBufferSizes,
#endif
    ) :
    m_bufferCount(lockBufferCount), 
    m_bufferPointers(pLockBuffers),
#ifdef ENABLE_MIC_TRACER
    m_bufferSizes(pLockBufferSizes),
#endif
    m_errorCode(CL_DEV_SUCCESS),
    m_releasehandler(nullptr),
    m_nextTaskToRelease(nullptr)
{
#ifdef ENABLE_MIC_TRACER
  // Set arrival time to device for the tracer
  m_commandTracer.set_current_time_cmd_run_in_device_time_start();
  // Set command ID for the tracer
  m_commandTracer.set_command_id((size_t)(getDispatcherData().commandIdentifier));
#endif
  if ((pQueue) && (pQueue->IsAsyncExecution()))
  {
	m_releasehandler = TaskReleaseHandler::getInstance();
  }
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void* TaskReleaseHandler::DummyTask::m_dummyBuffer = nullptr;
size_t TaskReleaseHandler::DummyTask::m_dummyBufferSize = 0;
TaskReleaseHandler* TaskReleaseHandler::m_singleton = nullptr;

TaskReleaseHandler::TaskReleaseHandler() : OclThread(), m_head(nullptr), m_tail(nullptr), m_event(true), m_finish(false), m_initDone(0)
#ifdef USE_ITT
    ,m_pIttTaskReleaseName(nullptr), m_pIttTaskReleaseDomain(nullptr)
#endif
{
}

TaskReleaseHandler::~TaskReleaseHandler()
{
	m_finish = true;
	m_event.Signal();
	WaitForOsThreadCompletion(GetThreadHandle());
	m_initDone = 0;
}

void TaskReleaseHandler::addTask(const Intel::OpenCL::Utils::SharedPtr< TaskHandlerBase >& task)
{
	bool signal = false;
	// Locked area
	{
	    OclAutoMutex lock(&m_mutex);
	    m_tail->m_nextTaskToRelease = task;
	    m_tail = m_tail->m_nextTaskToRelease;
	    if (nullptr == m_head)
	    {
		    signal = true;
		    m_head = m_tail;
	    }
	}
	// END of Locked area
	if (signal)
	{
		m_event.Signal();
	}
}

RETURN_TYPE_ENTRY_POINT TaskReleaseHandler::Run()
{
	// Set the affinity of this thread to use the the HW threads that are not used by TBB threads.
	ThreadPool::getInstance()->AffinitizeMasterThread();
#if defined(USE_ITT)
    if ( gMicGPAData.bUseGPA )
    {
        __itt_thread_set_name("MIC Device ReleaseTasks thread");
	    m_pIttTaskReleaseDomain = __itt_domain_create("com.intel.opencl.device.mic.release_task_handler");
		// Use fillBuffer specific domain if possible, if not available switch to global domain
        if ( nullptr == m_pIttTaskReleaseDomain )
        {
            m_pIttTaskReleaseDomain = gMicGPAData.pDeviceDomain;
        }
		m_pIttTaskReleaseName = __itt_string_handle_create("ReleaseTaskHandler");
    }
#endif
	m_tail = new DummyTask();
	uint64_t freq = COIPerfGetCycleFrequency();
	assert(freq > 0 && "clock freq must be greater than 0");
	if (0 == freq)
	{
		freq = 1000000000;
	}
	// spin 0.1 sec before waiting on condition variable.
	const uint64_t cyclesToSpin = freq / 10;
	unsigned long long spin_up_to = 0;
	Intel::OpenCL::Utils::SharedPtr< TaskHandlerBase > nextTask = nullptr;

	// In this point the initialization completed
	TAS(&m_initDone, 1);

    while ((!m_finish) || (nullptr != m_head))
    {
        if (nullptr == m_head)
        {
            m_event.Wait();
        }
        if (m_head)
        {
#if defined(USE_ITT)
            if ( gMicGPAData.bUseGPA )
            {
                __itt_frame_begin_v3(m_pIttTaskReleaseDomain, nullptr);
            }
#endif
#if defined(USE_ITT) && defined(USE_ITT_INTERNAL)
            if ( gMicGPAData.bUseGPA )
            {
              static __thread __itt_string_handle* pTaskName = nullptr;
              if ( nullptr == pTaskName )
              {
                pTaskName = __itt_string_handle_create("TaskReleaseHandler::Run() releaseResourcesAndSignal()");
              }
              __itt_task_begin(gMicGPAData.pDeviceDomain, __itt_null, __itt_null, pTaskName);
            }
#endif
            m_head->releaseResourcesAndSignal();
#if defined(USE_ITT) && defined(USE_ITT_INTERNAL)
            // Monitor only IN-ORDER queue
            if ( gMicGPAData.bUseGPA )
            {
              __itt_task_end(gMicGPAData.pDeviceDomain);
            }
#endif
#if defined(USE_ITT) && defined(USE_ITT_INTERNAL)
            if ( gMicGPAData.bUseGPA )
            {
              static __thread __itt_string_handle* pTaskName = nullptr;
              if ( nullptr == pTaskName )
              {
                pTaskName = __itt_string_handle_create("TaskReleaseHandler::Run() search new job");
              }
              __itt_task_begin(gMicGPAData.pDeviceDomain, __itt_null, __itt_null, pTaskName);
            }
#endif
            nextTask = m_head->m_nextTaskToRelease;
            spin_up_to = RDTSC() + cyclesToSpin;
            while ((nullptr == nextTask) && (RDTSC() < spin_up_to) && (!m_finish))
            {
                Intel::OpenCL::Utils::InnerSpinloopImpl();
                nextTask = m_head->m_nextTaskToRelease;
            }
            // Locked area
            {
                OclAutoMutex lock(&m_mutex);
                m_head = m_head->m_nextTaskToRelease;
            }
            // END of Locked area
#if defined(USE_ITT) && defined(USE_ITT_INTERNAL)
            // Monitor only IN-ORDER queue
            if ( gMicGPAData.bUseGPA )
            {
              __itt_task_end(gMicGPAData.pDeviceDomain);
            }
#endif
#ifdef USE_ITT
            if ( gMicGPAData.bUseGPA)
            {
                __itt_frame_end_v3(m_pIttTaskReleaseDomain, nullptr);
            }
#endif
        }
    }
    ThreadPool::getInstance()->ReturnAffinitizationResource();
    return (RETURN_TYPE_ENTRY_POINT)nullptr;
}
