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

/////////////////////////////////////////////////////////////
//  thread_pool.h
/////////////////////////////////////////////////////////////

#pragma once

#include "task_executor.h"
#include "native_globals.h"
#include "thread_local_storage.h"
#include <cl_synch_objects.h>
#include <sched.h>

namespace Intel { namespace OpenCL { namespace MICDeviceNative {

using Intel::OpenCL::Utils::SharedPtr;
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class ThreadPool;

//
// Describe device cores structure - a set of cores with threads inside.
//   each core and thread may be either enabled for use of disabled
//
class CoreAffinityDescriptor
{
public:
    CoreAffinityDescriptor();
    
    // if core of thread description was not found - return zero affinity
    bool is_enabled_thread( unsigned int core_thread_idx ) const { return (core_thread_idx < m_enabled_threads); };
    cpu_set_t get_thread_affinity_mask( unsigned int core_thread_idx, bool force_enabled_threads = true ) const;
    unsigned int get_thread_os_hw_thread( unsigned int core_thread_idx ) const 
    { 
        return (core_thread_idx < m_enabled_threads) ? os_hw_thread_ids[core_thread_idx] : os_hw_thread_ids[0];
    }

    bool is_enabled_core() const { return (m_enabled_threads > 0); };
    cpu_set_t get_core_affinity_mask() const;

    unsigned int get_enabled_threads_count() const { return m_enabled_threads; };

    static const unsigned int INVALID_THREAD_AFFINITY = (unsigned int)(-1);
private:
    unsigned int       os_hw_thread_ids[MIC_NATIVE_MAX_THREADS_PER_CORE];
    unsigned int       m_enabled_threads;

    void add_thread( unsigned int os_thread_id );
    void disable_core() { m_enabled_threads = 0; };
    void limit_enabled_threads( unsigned int count ) 
    { 
        m_enabled_threads = (count < m_enabled_threads) ? count : m_enabled_threads; 
    };

    void update_mask( unsigned int core_thread_idx, cpu_set_t* mask ) const;
    friend class ThreadPool;
};

/* A singleton class that provide API for thread pool services */
class ThreadPool : public Intel::OpenCL::TaskExecutor::ITaskExecutorObserver
{
public:

    /* Return the singleton instance of ThreadPool.
       Assume that it calls first when process creates - with single thread so it is NOT thread safe function. */
    static ThreadPool* getInstance();

    /* Release the singleton instance if not NULL.
       Assume that it calls before closing the process - it is NOT thread safe function. */
    static void releaseSingletonInstance();

    /* Call this method only once after construction in order to create the worker threads pool (The amount of worker threads are numOfWorkers).*/
    bool init(bool use_affinity,
              unsigned int number_of_cores, unsigned int threads_per_core, // number_of_cores or threads_per_core == 0 means default
              bool ignore_first_core, bool ignore_last_core );

    Intel::OpenCL::TaskExecutor::ITaskExecutor&  getTaskExecutor() { return *m_task_executor; };

    /* Returns global workers affinity mask */
    const cpu_set_t* getGlobalWorkersAffinityMask() const { return &m_globalWorkersAffinityMask; };

    unsigned int useNumberOfCores()     const { return m_useNumberOfCores; };
    unsigned int useThreadsPerCore()    const { return m_useThreadsPerCore; };
    bool         ignoreFirstCore()      const { return m_useIgnoreFirstCore; };
    bool         ignoreLastCore()       const { return m_useIgnoreLastCore; };
    bool         useAffinity()          const { return m_useAffinity; };

    const CoreAffinityDescriptor& get_core_affinity_descriptor( unsigned int core_idx ) const
    {
        assert( core_idx < MIC_NATIVE_MAX_CORES );
        return ( core_idx < MIC_NATIVE_MAX_CORES ) ? m_cores[ core_idx ] : m_cores[0];
    }

    // get TE Root Device
    const SharedPtr<Intel::OpenCL::TaskExecutor::ITEDevice>& getRootDevice() const { return m_RootDevice; }
    
    // Allocate all nessary resources for the current calling thread
    bool    ActivateCurrentMasterThread();
    bool    DeactivateCurrentMasterThread();
    bool    AffinitizeMasterThread();
    void    ReturnAffinitizationResource();

    // ITaskExecutorObserver
    virtual void* OnThreadEntry();
    virtual void  OnThreadExit( void* currentThreadData );
    
    virtual Intel::OpenCL::TaskExecutor::TE_BOOLEAN_ANSWER MayThreadLeaveDevice( void* currentThreadData );

#if (defined(ENABLE_TBB_TRACER) || defined(ENABLE_MIC_TBB_TRACER))
    /* Return current thread worker ID (worker ID of muster thread is always 0 and for worker thread >= 1). */
    static const unsigned int INVALID_WORKER_ID = (unsigned int)(-1);
    static unsigned int getWorkerID();
#endif // #if (defined(ENABLE_TBB_TRACER) || defined(ENABLE_MIC_TBB_TRACER))
    
    unsigned int GetNumberOfActiveThreads() const { return m_numOfActivatedThreads; }
protected:

    ThreadPool();
    virtual ~ThreadPool() { release(); }
    bool init_base(bool use_affinity,
                   unsigned int number_of_cores, unsigned int threads_per_core, 
                   bool ignore_first_core, bool ignore_last_core );
    void release();

private:

    // Define TaskExecutor Core as 2 level hw index (hw core, hw thread)
    union TaskExecutorCore
    {
        struct
        {
            unsigned short hw_core_idx;
            unsigned short hw_thread_idx;
        } hw_2_level_index;
    
        unsigned int executor_core;
    };

    unsigned int  m_useNumberOfCores;
    unsigned int  m_useThreadsPerCore;
    unsigned int  m_numOfActivatedThreads;
    unsigned int  m_numHWThreads;
    bool          m_useIgnoreFirstCore;
    bool          m_useIgnoreLastCore;
    bool          m_useAffinity;
    bool          m_init_done;
    volatile bool m_shut_down;

    CoreAffinityDescriptor m_cores[ MIC_NATIVE_MAX_CORES ];
    TaskExecutorCore       m_workerId_2_executorCore[ MIC_NATIVE_MAX_WORKER_THREADS ];

    // Global workers affinity mask 
    cpu_set_t m_globalWorkersAffinityMask;
    // Vector of affinity masks (each mask contain only one HW thread that is not in m_globalWorkersAffinityMask and not HW thread 0 that belongs to app. main thread)
    vector<cpu_set_t*> m_mastersAffinityMaskArr;
    // Global master threads affinity mask (union of m_mastersAffinityMaskArr)
    cpu_set_t m_globalMastersAffinityMask;
    // Mutex guard for m_mastersAffinityMaskArr access.
    Intel::OpenCL::Utils::OclNonReentrantSpinMutex m_affinityMasksGaurd;

    Intel::OpenCL::TaskExecutor::ITaskExecutor*         m_task_executor;
    SharedPtr<Intel::OpenCL::TaskExecutor::ITEDevice>   m_RootDevice;

    // read device hw structure
    bool read_device_structure();

    // set thread affinity
    // worker_id == 0 means master thread
    void setCurrentThreadAffinity( unsigned int thread_idx );

#ifdef __MIC_DA_OMP__
    public:
#endif
    void startup_all_workers();
#ifdef __MIC_DA_OMP__
    private:
#endif

#if (defined(ENABLE_TBB_TRACER) || defined(ENABLE_MIC_TBB_TRACER))
    void setWorkerID();
    Intel::OpenCL::Utils::AtomicCounter m_workersCount; // used for worker id allocation also
#endif // #if (defined(ENABLE_TBB_TRACER) || defined(ENABLE_MIC_TBB_TRACER))        

    static ThreadPool*     m_threadPool;
    static __thread bool  m_tbMasterInitalized;
    static __thread cpu_set_t* m_tMasterAffiniityMask;
};

}}}
