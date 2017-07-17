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
//  thread_pool.cpp
/////////////////////////////////////////////////////////////

#include <cl_shared_ptr.hpp>
#include <hw_utils.h>
#include <cl_synch_objects.h>

#include "native_thread_pool.h"
#include "native_common_macros.h"

#include <map>
#include <vector>
#include <fstream>

using namespace Intel::OpenCL::MICDeviceNative;
using namespace Intel::OpenCL::UtilsNative;
using namespace Intel::OpenCL::Utils;
using namespace Intel::OpenCL::TaskExecutor;

ThreadPool* ThreadPool::m_threadPool = nullptr;
__thread bool    ThreadPool::m_tbMasterInitalized = false;
__thread cpu_set_t* ThreadPool::m_tMasterAffiniityMask = nullptr;

// 
// CoreAffinityDescriptor
//

CoreAffinityDescriptor::CoreAffinityDescriptor() : m_enabled_threads(0)
{
    for (unsigned int i = 0; i < MIC_NATIVE_MAX_THREADS_PER_CORE; ++i)
    {
        os_hw_thread_ids[i] = INVALID_THREAD_AFFINITY;
    }
}

void CoreAffinityDescriptor::add_thread( unsigned int os_thread_id )
{
    assert( m_enabled_threads < MIC_NATIVE_MAX_THREADS_PER_CORE );

    os_hw_thread_ids[m_enabled_threads] = os_thread_id;
    ++m_enabled_threads;
}

inline
void CoreAffinityDescriptor::update_mask( unsigned int core_thread_idx, cpu_set_t* mask ) const
{
    CPU_SET( os_hw_thread_ids[core_thread_idx], mask );  
}

cpu_set_t CoreAffinityDescriptor::get_thread_affinity_mask( unsigned int core_thread_idx, bool force_enabled_threads ) const
{
    cpu_set_t mask;
    CPU_ZERO( &mask );

    assert( core_thread_idx < MIC_NATIVE_MAX_THREADS_PER_CORE );

    if (!force_enabled_threads || is_enabled_thread(core_thread_idx))
    {
        assert(os_hw_thread_ids[core_thread_idx] != INVALID_THREAD_AFFINITY);
        if (os_hw_thread_ids[core_thread_idx] != INVALID_THREAD_AFFINITY)
        {
            update_mask(core_thread_idx, &mask );
        }
    }

    return mask;
}

cpu_set_t CoreAffinityDescriptor::get_core_affinity_mask() const
{
    cpu_set_t mask;
    CPU_ZERO( &mask );

    for (unsigned int i = 0; i < m_enabled_threads; ++i)
    {
        update_mask( i, &mask );
    }

    return mask;
}

//
// ThreadPool
//

ThreadPool::ThreadPool() : 
    m_useNumberOfCores( MIC_NATIVE_MAX_CORES ), m_useThreadsPerCore( MIC_NATIVE_MAX_THREADS_PER_CORE ),m_numOfActivatedThreads(0), m_numHWThreads(0),
    m_useIgnoreFirstCore( false ), m_useIgnoreLastCore( false ), m_useAffinity(false), m_init_done(false), m_shut_down(false),
    m_task_executor(nullptr), m_RootDevice(nullptr)
{
    unsigned int i;
    for (i = 0; i < MIC_NATIVE_MAX_WORKER_THREADS; ++i)
    {
        m_workerId_2_executorCore[i].executor_core = CoreAffinityDescriptor::INVALID_THREAD_AFFINITY;
    }
}

ThreadPool* ThreadPool::getInstance()
{
    if (nullptr == m_threadPool)
    {
        m_threadPool = new ThreadPool();
        assert(m_threadPool);
    }
    return m_threadPool;
}

void ThreadPool::releaseSingletonInstance()
{
    if (m_threadPool)
    {
        delete m_threadPool;
        m_threadPool = nullptr;
    }
}

bool ThreadPool::read_device_structure()
{
    ifstream ifs("/proc/cpuinfo", ifstream::in);
    if (ifs == nullptr)
    {
        return false;
    }

    // map: coreID -> array of 4 thread IDs
    typedef map< unsigned int, vector<unsigned int> >  CoreId2ThreadIsMap;
    CoreId2ThreadIsMap                                 coreToThreadsMap;
    CoreId2ThreadIsMap::iterator                       it;

    string title;
    int coreID = -1;
    int processorID = -1;
    char buff[1024];
    while (ifs.getline(buff,1024, ':'))
    {
        title = string(buff);
        if (title.find("processor") != string::npos)
        {
            assert(-1 == processorID);
            ifs.getline(buff,1024);
            processorID = atoi(buff);
            m_numHWThreads ++;
            continue;
        }
        if (title.find("core id") != string::npos)
        {
            assert(-1 == coreID);
            assert(processorID >= 0);
            if ((coreID != -1) || (processorID < 0))
            {
                ifs.close();
                return false;
            }
            ifs.getline(buff,1024);
            coreID = atoi(buff);

            assert( MIC_NATIVE_MAX_CORES > coreID );
            
            it = coreToThreadsMap.find(coreID);
            if (coreToThreadsMap.end() == it)
            {
                vector<unsigned int> tVec;
                tVec.push_back(processorID);
                coreToThreadsMap.insert( CoreId2ThreadIsMap::value_type(coreID, tVec) );
            }
            else
            {
                it->second.push_back(processorID);
            }
            coreID = -1;
            processorID = -1;
            continue;
        }
    }
    ifs.close();

    // convert map to CoreAffinityDescriptors
    for ( coreID = 0; coreID < MIC_NATIVE_MAX_CORES; ++coreID )
    {
        it = coreToThreadsMap.find(coreID);

        if ( it == coreToThreadsMap.end())
        {
            break;
        }

        const CoreId2ThreadIsMap::value_type& core_desc = *it;

        assert( (core_desc.second.size() == MIC_NATIVE_MAX_THREADS_PER_CORE) && "/proc/cpuinfo listed some core with not MIC_NATIVE_MAX_THREADS_PER_CORE threads" );

        for ( unsigned int thread_idx = 0; thread_idx < MIC_NATIVE_MAX_THREADS_PER_CORE; ++thread_idx )
        {
            if (thread_idx < core_desc.second.size())
            {
                m_cores[coreID].add_thread( core_desc.second[thread_idx] );
            }
        }
    }
    
    coreToThreadsMap.clear();
    return true;
    
}

//
// Finalize initialization of device HW structure that may be used by TaskExecutor
//
bool ThreadPool::init_base( bool use_affinity,
                            unsigned int number_of_cores, unsigned int threads_per_core, 
                            bool ignore_first_core, bool ignore_last_core )
{
    assert( (false == m_init_done) && "ThreadPool initialized more than once" );

    if (m_init_done)
    {
        return false;
    }

    m_init_done = true;

    m_useAffinity       = use_affinity;
    m_useNumberOfCores  = ((number_of_cores > 0 ) && (number_of_cores <= MIC_NATIVE_MAX_CORES)) ? number_of_cores  : MIC_NATIVE_MAX_CORES;
    m_useThreadsPerCore = ((threads_per_core > 0) && (threads_per_core <= MIC_NATIVE_MAX_THREADS_PER_CORE)) ? threads_per_core : MIC_NATIVE_MAX_THREADS_PER_CORE;
    m_useIgnoreFirstCore= ignore_first_core; 
    m_useIgnoreLastCore = ignore_last_core;
#ifdef __OMP_EXECUTOR__
    m_useAffinity = false;
    if (m_useIgnoreFirstCore)
    {
        m_useNumberOfCores --;
    }
    if (m_useIgnoreLastCore)
    {
        m_useNumberOfCores --;
    }
#endif

    bool ok = read_device_structure();
    assert( (true == ok) && "ThreadPool cannot read device structure" );

    CPU_ZERO( &m_globalWorkersAffinityMask );
    CPU_ZERO( &m_globalMastersAffinityMask );

    if (false == m_useAffinity)
    {
        // initialize global affinity as set by COI/uOS
        if (0 != sched_getaffinity( 0, sizeof(m_globalWorkersAffinityMask), &m_globalWorkersAffinityMask))
        {
            //Report Error
            assert( false && "sched_getaffinity returned error" );
            return false;
        }
        m_globalMastersAffinityMask = m_globalWorkersAffinityMask;

        return true;
    }

    // generate affinity masks
    unsigned int core_idx;
    
    if (m_useIgnoreFirstCore)
    {
        // find the first enabled code and disable it
        for (core_idx = 0; core_idx < MIC_NATIVE_MAX_CORES; ++core_idx)
        {
            CoreAffinityDescriptor& core = m_cores[core_idx];            
            if (core.is_enabled_core())
            {
                core.disable_core();
                break;
            }
        }
    }

    if (m_useIgnoreLastCore)
    {
        // find the last enabled code and disable it
        for (core_idx = MIC_NATIVE_MAX_CORES; core_idx > 0; --core_idx)
        {
            CoreAffinityDescriptor& core = m_cores[core_idx-1];            
            if (core.is_enabled_core())
            {
                core.disable_core();
                break;
            }
        }
    }

    // now ensure that only required number of cores will be enabled
    unsigned int enabled_cores = 0;
    for (core_idx = 0; core_idx < MIC_NATIVE_MAX_CORES; ++core_idx)
    {
        CoreAffinityDescriptor& core = m_cores[core_idx];
        if (core.is_enabled_core())
        {
            if (enabled_cores < m_useNumberOfCores)
            {
                ++enabled_cores;
                core.limit_enabled_threads( m_useThreadsPerCore );
                
                cpu_set_t core_affinity = core.get_core_affinity_mask();
                CPU_OR( &m_globalWorkersAffinityMask, &m_globalWorkersAffinityMask, &core_affinity );
            }
            else
            {
                core.disable_core();
            }
        }
	}
    bool masterMaskSet = false;
    // Find unused HW threads (from HW thread 1, HW thread 0 belongs to application main thread)
    for (unsigned int cpuIndex = 1; cpuIndex < m_numHWThreads; cpuIndex ++)
    {
        if (0 == CPU_ISSET(cpuIndex, &m_globalWorkersAffinityMask))
        {
            CPU_SET(cpuIndex, &m_globalMastersAffinityMask);
            m_mastersAffinityMaskArr.push_back(new cpu_set_t());
            cpu_set_t* tCpuSet = m_mastersAffinityMaskArr[m_mastersAffinityMaskArr.size() - 1];
            CPU_ZERO(tCpuSet);
            CPU_SET(cpuIndex, tCpuSet);
            masterMaskSet = true;
        }
    }
    // If 0 CPUs set to m_globalMastersAffinityMask, than m_globalMastersAffinityMask should be equal to m_globalWorkersAffinityMask
    if (!masterMaskSet)
    {
        m_globalMastersAffinityMask = m_globalWorkersAffinityMask;
     }

    m_useNumberOfCores = enabled_cores;
    return true;
}

//
// Prepare TaskExecutor
//
bool ThreadPool::init(
          bool use_affinity,
          unsigned int number_of_cores, unsigned int threads_per_core,
          bool ignore_first_core, bool ignore_last_core )
{
    cpu_set_t current_mask;
    
    if (! init_base( use_affinity, number_of_cores, threads_per_core, ignore_first_core, ignore_last_core ))
    {
        //Report Error
        assert( false && "ThreadPool::init_base returned error" );
        return false;
    }

    m_task_executor = GetTaskExecutor();

    assert( nullptr != m_task_executor );
    if (nullptr == m_task_executor)
    {
        //Report Error
        return false;
    }

    // enforce flat device scheduling
    m_numOfActivatedThreads = useNumberOfCores() * useThreadsPerCore();
    assert( m_numOfActivatedThreads > 0 );

    if (useAffinity())
    {
        // use flat arenas only for now
        unsigned int thread_idx;
        unsigned int core_idx;
        unsigned int worker_idx = 0;

        // assign first threads on available core, then all threads on the next available core, etc.
        for (core_idx = 0; core_idx < MIC_NATIVE_MAX_CORES; ++core_idx)
        {
            const CoreAffinityDescriptor& core = get_core_affinity_descriptor(core_idx);
            if (core.is_enabled_core())
            {
                for ( thread_idx = 0; thread_idx < useThreadsPerCore(); ++thread_idx )
                {
                    m_workerId_2_executorCore[worker_idx].hw_2_level_index.hw_core_idx   = core_idx;
                    m_workerId_2_executorCore[worker_idx].hw_2_level_index.hw_thread_idx = thread_idx;
                    ++worker_idx;
                }
            }
        }

        assert( worker_idx == m_numOfActivatedThreads );

        // save current affinity
        if (0 != sched_getaffinity( 0, sizeof(current_mask), &current_mask))
        {
            //Report Error
            printf("ThreadPool GetThreadAffinityMask error: %d\n", errno);
            assert( false && "sched_getaffinity returned error" );
            return false;
        }

        // initialize global affinity as required by TaskExecutor
        if (0 != sched_setaffinity( 0, sizeof(m_globalWorkersAffinityMask), &m_globalWorkersAffinityMask))
        {
            //Report Error
            printf("ThreadPool SetThreadAffinityMask error: %d\n", errno);
            assert( false && "sched_setaffinity returned error" );
            return false;
        }
    }

    if (! m_task_executor->Init( nullptr, m_numOfActivatedThreads + 1, nullptr ))
    {
        //Report Error
        printf("TaskExecutor initialization failed\n");
        assert( false && "TaskExecutor initialization failed" );
        return false;        
    }

    m_RootDevice = m_task_executor->CreateRootDevice( 
                    RootDeviceCreationParam( m_numOfActivatedThreads + 1, TE_ENABLE_MASTERS_JOIN, 1 ),
                    nullptr,
                    this );
    
    if (nullptr == m_RootDevice)
    {
        //Report Error
        printf("TaskExecutor CreateRootDevice() failed\n");
        assert( false && "TaskExecutor CreateRootDevice failed" );
        return false;        
    }

#ifndef __MIC_DA_OMP__
    startup_all_workers();
#endif

    if (useAffinity())
    {
        // restore previous current thread affinity
        if (0 != sched_setaffinity( 0, sizeof(current_mask), &current_mask))
        {
            //Report Error
            printf("ThreadPool SetThreadAffinityMask error: %d\n", errno);
            assert( false && "sched_setaffinity returned error" );
            return false;
        }
    }

#ifdef __HARD_TRAPPING__
    if (gMicExecEnvOptions.trap_workers)
    {
        m_RootDevice->AcquireWorkerThreads( m_RootDevice->GetConcurrency() - 1 ); // do not trap master slot
    }
#endif __HARD_TRAPPING__

    return true;    
}

void ThreadPool::release()
{
    if (nullptr != m_task_executor)
    {
#ifdef __HARD_TRAPPING__
        if (gMicExecEnvOptions.trap_workers)
        {
            m_RootDevice->RelinquishWorkerThreads();
        }
#endif __HARD_TRAPPING__

        m_shut_down = true;
        m_RootDevice->SetObserver(this);

        m_RootDevice->ShutDown();
        m_RootDevice->ResetObserver();
        m_RootDevice = nullptr;
        m_task_executor->Finalize();
        m_task_executor = nullptr;
    }
}

#if (defined(ENABLE_TBB_TRACER) || defined(ENABLE_MIC_TBB_TRACER))
unsigned int ThreadPool::getWorkerID()
{
    bool alreadyHad = false;
    TlsAccessor tlsAccessor;
    ExecutorTls execTls( &tlsAccessor );

    unsigned int ret = (unsigned int)(size_t)execTls.getTls(ExecutorTls::WORKER_ID, alreadyHad);
    return alreadyHad ? ret : INVALID_WORKER_ID;
}

inline
void ThreadPool::setWorkerID()
{
    TlsAccessor tlsAccessor;
    ExecutorTls execTls( &tlsAccessor );

    unsigned int my_worker_id = ++m_workersCount;
    executor_tls.setTls(ExecutorTls::WORKER_ID, (void*)(size_t)my_worker_id);
}
#endif // #if (defined(ENABLE_TBB_TRACER) || defined(ENABLE_MIC_TBB_TRACER))

void* ThreadPool::OnThreadEntry()
{
    bool          isMaster   = m_task_executor->IsMaster();
    unsigned int  thread_idx = m_task_executor->GetPosition();

    assert( MIC_NATIVE_MAX_WORKER_THREADS > thread_idx );    
    if (MIC_NATIVE_MAX_WORKER_THREADS <= thread_idx)
    {
        return nullptr;
    }

    bool bActivateMaster = isMaster ? !m_tbMasterInitalized : false;

    if ( bActivateMaster )
    {
        ActivateCurrentMasterThread();
        m_tbMasterInitalized = true;
    }

    if ( !isMaster )
    {
        assert(thread_idx > 0 && "thread_idx of worker thread should be greater than 0");
        setCurrentThreadAffinity(thread_idx - 1);
    }

#if (defined(ENABLE_TBB_TRACER) || defined(ENABLE_MIC_TBB_TRACER))
    if (!isMaster)
    {
        setWorkerID();
    }
#endif // #if (defined(ENABLE_TBB_TRACER) || defined(ENABLE_MIC_TBB_TRACER))

    return (void*)this;
}

// Allocate all necessary resources for the current calling thread
bool ThreadPool::ActivateCurrentMasterThread()
{
    if ( !m_tbMasterInitalized )
    {
        AffinitizeMasterThread();
        m_RootDevice->AttachMasterThread((void*)this);
        m_tbMasterInitalized = true;
    }
    
    return true;
}

bool ThreadPool::DeactivateCurrentMasterThread()
{
    if ( m_tbMasterInitalized )
    {
        m_RootDevice->DetachMasterThread();
        m_tbMasterInitalized = false;
    }

    return true;
}

bool ThreadPool::AffinitizeMasterThread()
{
    if ((useAffinity()) && (nullptr == m_tMasterAffiniityMask))
    {
        cpu_set_t* myAffinityMask = nullptr;
        {
            OclAutoMutex cs(&m_affinityMasksGaurd);
            if (m_mastersAffinityMaskArr.size() > 0)
            {
                myAffinityMask = m_mastersAffinityMaskArr[m_mastersAffinityMaskArr.size() - 1];
                assert(myAffinityMask && "It must be valid pointer of cpu_set_t");
                m_mastersAffinityMaskArr.pop_back();
            }
        }
        if (nullptr == myAffinityMask)
        {
            myAffinityMask = &m_globalMastersAffinityMask;
        }
        if (0 != sched_setaffinity( 0, sizeof(cpu_set_t), myAffinityMask))
        {
            //Report Error
            printf("ThreadPool::ActivateCurrentMasterThread() sched_setaffinity error: %d\n", errno);
            assert( false && "ThreadPool::ActivateCurrentMasterThread() sched_setaffinity returned error" );
            return false;
        }
        m_tMasterAffiniityMask = myAffinityMask;
    }

	return true;
}

void ThreadPool::ReturnAffinitizationResource()
{
    if ((m_tMasterAffiniityMask) && (m_tMasterAffiniityMask != &m_globalMastersAffinityMask))
    {
        OclAutoMutex cs(&m_affinityMasksGaurd);
        m_mastersAffinityMaskArr.push_back(m_tMasterAffiniityMask);
    }
    m_tMasterAffiniityMask = nullptr;
}

void  ThreadPool::OnThreadExit( void* currentThreadData )
{
}

Intel::OpenCL::TaskExecutor::TE_BOOLEAN_ANSWER ThreadPool::MayThreadLeaveDevice( void* currentThreadData )
{
    if ( !m_shut_down )
        return Intel::OpenCL::TaskExecutor::TE_NO;
    else
        return Intel::OpenCL::TaskExecutor::TE_YES;
}

void ThreadPool::setCurrentThreadAffinity( unsigned int worker_id )
{
    if (!useAffinity())
    {
        return;
    }
    
    if (worker_id >= m_numOfActivatedThreads)
    {
        // in most cases this happens when device with single executable thread is requested.
        assert( 0 && "Number of workers should not exceed the total amount of workers");
        worker_id = 0;
    }

    const TaskExecutorCore& executor_core = m_workerId_2_executorCore[ worker_id ];
    const CoreAffinityDescriptor& core_desc = get_core_affinity_descriptor( executor_core.hw_2_level_index.hw_core_idx );
    cpu_set_t affinity = core_desc.get_thread_affinity_mask( executor_core.hw_2_level_index.hw_thread_idx );

    cpu_set_t curr_affinity;
    CPU_ZERO( &curr_affinity );

    // may be we already set the right affinity?
    if (0 == sched_getaffinity(0, sizeof(curr_affinity), &curr_affinity))
    {
        // if ok
        if (!CPU_EQUAL( &affinity, &curr_affinity))
        {
            // Affinity was not already set
            if (0 != sched_setaffinity( 0, sizeof(affinity), &affinity))
            {
                //Report Error
                assert( false && "ThreadPool::setCurrentThreadAffinity sched_setaffinity returned error" );
            }
            
        }
    }
    
}

//
//  Warm-up device
//
class WarmUpTask : public ITaskSet
{
public:
    PREPARE_SHARED_PTR(WarmUpTask)
    
    static inline SharedPtr<WarmUpTask> Allocate( unsigned int num_of_workers ) { return new WarmUpTask( num_of_workers ); }

    void    Cancel() { Finish(FINISH_INIT_FAILED); }

    int     Init(size_t region[], unsigned int& regCount);
    void*   AttachToThread(void* pWgContextBase, size_t uiNumberOfWorkGroups, size_t firstWGID[], size_t lastWGID[]) 
    {
        return (void*)1;
    }
    void    DetachFromThread(void * pWgContext) {}

    // "Main loop"
    // The function is called with different 'inx' parameters for each iteration number
    bool    ExecuteIteration(size_t x, size_t y, size_t z, void* pWgContext = nullptr);

    bool    Finish(FINISH_REASON reason) { m_completed = 1; return true; }

    // Returns true in case current task is a synchronization point
    // No more tasks will be executed in this case
    bool    CompleteAndCheckSyncPoint() { return false; }

    // Set current command as synchronization point
    // Returns true if command is already completed
    bool    SetAsSyncPoint() { return false; }

    // Returns true if command is already completed
    bool    IsCompleted() const { return (0 != m_completed); }

    // Returns true if command is already completed
    void    WaitAllWorkersJoined()
    {
      masterWaitEvent.Wait();
    }

    // Master operation is ready, workers can be released
    void    SetMasterReady()
    {
      long val = --startup_workers_left;
      assert(val == 0 && "Expected all workers to wakeup on this stage");
      workersWaitEvent.Signal();
    }

    // Releases task object, shall be called instead of delete operator.
    long    Release() { delete this; return 0; }

    // Optimize By
    TASK_PRIORITY         GetPriority() const { return TASK_PRIORITY_MEDIUM;}
    TASK_SET_OPTIMIZATION OptimizeBy() const { return TASK_SET_OPTIMIZE_DEFAULT; }
    unsigned int          PreferredSequentialItemsPerThread() const { return 1; }

    IThreadLibTaskGroup* GetNDRangeChildrenTaskGroup() { return nullptr; }

private:
    WarmUpTask( unsigned int num_of_workers ) :
        startup_workers_left(num_of_workers), masterWaitEvent(true), workersWaitEvent(false) {}

    ~WarmUpTask() {}

    AtomicCounter startup_workers_left;
    AtomicCounter m_completed;
    Intel::OpenCL::Utils::OclOsDependentEvent masterWaitEvent;
    Intel::OpenCL::Utils::OclOsDependentEvent workersWaitEvent;
};

int WarmUpTask::Init(size_t region[], unsigned int& regCount)
{
    regCount = 1;
#if defined(__MIC_DA_OMP__)
    region[0] = startup_workers_left; // The master also join in OMP
#else
    region[0] = startup_workers_left-1;    // Less one for the master slot.
#endif
    return 0;
}

bool WarmUpTask::ExecuteIteration(size_t x, size_t y, size_t z, void* pWgContext)
{
    long val = --startup_workers_left;
#if defined(USE_ITT)
    if ( gMicGPAData.bUseGPA )
    {
        __itt_thread_set_name("MIC Device Worker Thread");
    }
#endif
#if defined(USE_ITT) && defined(USE_ITT_INTERNAL)
    if ( gMicGPAData.bUseGPA)
    {
        static __itt_string_handle* pTaskName = nullptr;
        if ( nullptr == pTaskName )
        {
            pTaskName = __itt_string_handle_create("WarmUp Task");
        }
        __itt_task_begin(gMicGPAData.pDeviceDomain, __itt_null, __itt_null, pTaskName);
    }
#endif

	// if single worker left wake-up master
    if ( val == 1 )
    {
      masterWaitEvent.Signal();
    }
    // If last worker joined wake-up others
    if ( val == 0 )
    {
      workersWaitEvent.Signal();
    }
    else
    {
      workersWaitEvent.Wait();
    }

    #if defined(USE_ITT) && defined(USE_ITT_INTERNAL)
    if ( gMicGPAData.bUseGPA)
    {
        __itt_task_end(gMicGPAData.pDeviceDomain);
    }
#endif
    return true;
}

void ThreadPool::startup_all_workers() 
{
    if (m_numOfActivatedThreads < 2)
    {
        return;
    }

#if defined(USE_ITT) && defined(USE_ITT_INTERNAL)
    if ( gMicGPAData.bUseGPA)
    {
        __itt_frame_begin_v3(gMicGPAData.pDeviceDomain, nullptr);
        static __itt_string_handle* pTaskName = nullptr;
        if ( nullptr == pTaskName )
        {
        pTaskName = __itt_string_handle_create("Thread Pool initialization");
        }
        __itt_task_begin(gMicGPAData.pDeviceDomain, __itt_null, __itt_null, pTaskName);
    }
#endif

#ifdef __MIC_DA_OMP__
    // __OMP_EXECUTOR__ support only immediate queue
    SharedPtr<ITaskList> list = m_RootDevice->CreateTaskList( TE_CMD_LIST_IMMEDIATE );
#else
    SharedPtr<ITaskList> list = m_RootDevice->CreateTaskList( TE_CMD_LIST_IN_ORDER );
#endif 
    SharedPtr<WarmUpTask> warmup_task = WarmUpTask::Allocate( m_numOfActivatedThreads + 1 );
    list->Enqueue( warmup_task );
    list->Flush();

#ifndef __MIC_DA_OMP__
    
    // Wait workers to join execution
    warmup_task->WaitAllWorkersJoined();

    // All workers joined, now we can shutdown observer
    m_RootDevice->SetObserver(nullptr);
    
#endif

	// Workers might be released
    warmup_task->SetMasterReady();

    // Wait for all task completed
    list->WaitForCompletion(nullptr);

#if defined(USE_ITT) && defined(USE_ITT_INTERNAL)
    if ( gMicGPAData.bUseGPA)
    {
        __itt_task_end(gMicGPAData.pDeviceDomain);
        __itt_frame_end_v3(gMicGPAData.pDeviceDomain, nullptr);
    }
#endif

}

