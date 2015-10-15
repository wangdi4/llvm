// Copyright (c) 2006-2012 Intel Corporation
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

/*
*
* File tbb_executor.h
*        Implements interface required for task execution on XNTask sub-system
*
*/
#pragma once

#include "task_executor.h"
#include <tbb/tbb.h>

#include "cl_synch_objects.h"
#include "cl_dynamic_lib.h"
#include "cl_shared_ptr.h"
#include "cl_synch_objects.h"
#include "base_command_list.h"
#include "tbb_thread_manager.h"
#include "arena_handler.h"

#ifdef DEVICE_NATIVE
    // no logger on discrete device
    #define LOG_ERROR(...)
    #define LOG_INFO(...)

    #define DECLARE_LOGGER_CLIENT
    #define INIT_LOGGER_CLIENT(...)
    #define RELEASE_LOGGER_CLIENT
#else
    #include "Logger.h"
#endif // DEVICE_NATIVE

using Intel::OpenCL::Utils::SharedPtr;
using Intel::OpenCL::Utils::AtomicPointer;
using Intel::OpenCL::Utils::OclMutex;

namespace Intel { namespace OpenCL { namespace TaskExecutor {

    /**
     * a global flag indicating whether the program has called function exit
     */
    class TBBTaskExecutor : public ITaskExecutor
    {
    public:
        TBBTaskExecutor();
        virtual ~TBBTaskExecutor();

        int  Init(Intel::OpenCL::Utils::FrameworkUserLogger* pUserLogger, unsigned int uiNumOfThreads = TE_AUTO_THREADS, ocl_gpa_data * pGPAData = NULL);
        void Finalize();

        Intel::OpenCL::Utils::SharedPtr<ITEDevice>  CreateRootDevice(
                                                const RootDeviceCreationParam& device_desc = RootDeviceCreationParam(),  
                                                void* user_data = NULL, ITaskExecutorObserver* my_observer = NULL );

        unsigned int GetMaxNumOfConcurrentThreads() const;

        ocl_gpa_data* GetGPAData() const;

        virtual DeviceHandleStruct GetCurrentDevice() const;
        virtual bool IsMaster() const;
        virtual unsigned int GetPosition( unsigned int level = 0 ) const;

        typedef TBB_ThreadManager<TBB_PerActiveThreadData> ThreadManager;
        ThreadManager& GetThreadManager() { return m_threadManager; }
        const ThreadManager& GetThreadManager() const { return m_threadManager; }

        virtual SharedPtr<IThreadLibTaskGroup> CreateTaskGroup(const SharedPtr<ITEDevice>& device);

        ITaskList* GetDebugInOrderDeviceQueue() { return m_pDebugInOrderDeviceQueue.GetPtr(); }

        virtual void CreateDebugDeviceQueue(const SharedPtr<ITEDevice>& rootDevice);

        virtual void DestroyDebugDeviceQueue();

    protected:
        // Load TBB library explicitly
        bool LoadTBBLibrary();

        ThreadManager                          m_threadManager;
        Intel::OpenCL::Utils::OclDynamicLib    m_dllTBBLib;

        /* We need this because of a bug Anton has reported: we should initialize the task_scheduler_init to P+1 threads, instead of P. Apparently, if we explicitly create a task_scheduler_init
           in a certain master thread, TBB creates a global task_scheduler_init object that future created task_arenas will use. Once they fix this bug, we can remove this attribute.
           They seem to have another bug in ~task_scheduler_init(), so we work around it by allocating and not deleting it. */
        tbb::task_scheduler_init*           m_pScheduler;

        SharedPtr<ITaskList>                m_pDebugInOrderDeviceQueue;

        // Logger
        DECLARE_LOGGER_CLIENT;

    private:
        TBBTaskExecutor(const TBBTaskExecutor&);
        TBBTaskExecutor& operator=(const TBBTaskExecutor&);
    };

    class in_order_executor_task
    {
    public:
        in_order_executor_task(const SharedPtr<base_command_list>& list) : m_list(list){}

        void operator()();

    protected:
        SharedPtr<base_command_list> m_list;

        void FreeCommandBatch(TaskVector* pCmdBatch);
    };

    class out_of_order_executor_task
    {
    public:
        out_of_order_executor_task(const SharedPtr<base_command_list>& list) : m_list(list.StaticCast<out_of_order_command_list>())
        {
            assert(m_list != 0);
        }

        void operator()();

    private:
        SharedPtr<ITaskBase> GetTask();
        SharedPtr<out_of_order_command_list> m_list;
    };

    class immediate_executor_task
    {
    public:
        immediate_executor_task(immediate_command_list* list, const SharedPtr<ITaskBase>& pTask ) : 
            m_list(list), m_pTask( pTask ) {}

        void operator()();

    protected:
        immediate_command_list*     m_list;
        SharedPtr<ITaskBase>        m_pTask;
    };

}}}
