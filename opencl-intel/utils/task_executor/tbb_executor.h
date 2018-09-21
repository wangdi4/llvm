// INTEL CONFIDENTIAL
//
// Copyright 2006-2018 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

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

        int  Init(Intel::OpenCL::Utils::FrameworkUserLogger* pUserLogger,
                  unsigned int uiNumOfThreads = TE_AUTO_THREADS,
                  ocl_gpa_data * pGPAData = nullptr,
                  size_t ulAdditionalRequiredStackSize = 0,
                  DeviceMode deviceMode = CPU_DEVICE);
        void Finalize();

        Intel::OpenCL::Utils::SharedPtr<ITEDevice>  CreateRootDevice(
                                                const RootDeviceCreationParam& device_desc = RootDeviceCreationParam(),  
                                                void* user_data = nullptr, ITaskExecutorObserver* my_observer = nullptr );

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
