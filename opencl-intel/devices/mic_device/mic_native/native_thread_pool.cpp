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

#include "native_thread_pool.h"
#include "cl_shared_ptr.hpp"
#include "native_common_macros.h"
#include "hw_utils.h"

#include <map>
#include <vector>
#include <fstream>

using namespace Intel::OpenCL::MICDeviceNative;
using namespace Intel::OpenCL::UtilsNative;
using namespace Intel::OpenCL::Utils;
using namespace Intel::OpenCL::TaskExecutor;

ThreadPool* ThreadPool::m_threadPool = NULL;

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

cpu_set_t CoreAffinityDescriptor::get_thread_affinity_mask( unsigned int core_thread_idx ) const
{
    cpu_set_t mask;
    CPU_ZERO( &mask );

    assert( core_thread_idx < MIC_NATIVE_MAX_THREADS_PER_CORE );

    if (is_enabled_thread(core_thread_idx))
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
    m_useNumberOfCores( MIC_NATIVE_MAX_CORES ), m_useThreadsPerCore( MIC_NATIVE_MAX_THREADS_PER_CORE ),m_numOfActivatedThreads(0),
    m_useIgnoreFirstCore( false ), m_useIgnoreLastCore( false ), m_useAffinity(false), m_init_done( false ),
    m_workersCount(0), m_task_executor(NULL)
{
    unsigned int i;
    for (i = 0; i < MIC_NATIVE_MAX_WORKER_THREADS; ++i)
    {
        m_workerId_2_executorCore[i].executor_core = CoreAffinityDescriptor::INVALID_THREAD_AFFINITY;
    }
}

ThreadPool* ThreadPool::getInstance()
{
	if (NULL == m_threadPool)
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
		m_threadPool = NULL;
	}
}

bool ThreadPool::read_device_structure()
{
    ifstream ifs("/proc/cpuinfo", ifstream::in);
    if (ifs == NULL)
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

    bool ok = read_device_structure();
    assert( (true == ok) && "ThreadPool cannot read device structure" );

    CPU_ZERO( &m_globalWorkersAffinityMask );

    if (false == m_useAffinity)
    {
        // initialize global affinity as set by COI/uOS
        if (0 != sched_getaffinity( 0, sizeof(m_globalWorkersAffinityMask), &m_globalWorkersAffinityMask))
        {
            //Report Error
            printf("ThreadPool GetThreadAffinityMask error: %d\n", errno);
            assert( false && "sched_getaffinity returned error" );
            return false;
        }

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
        printf("ThreadPool initialization failed\n");
        assert( false && "ThreadPool::init_base returned error" );
        return false;
    }

    m_task_executor = GetTaskExecutor();

    assert( NULL != m_task_executor );
    if (NULL == m_task_executor)
    {
        //Report Error
        printf("ThreadPool initialization failed - cannot create TaskExecutor\n");
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

        // assign first all 0 threads, then all 1 threads, etc.
        for ( thread_idx = 0; thread_idx < useThreadsPerCore(); ++thread_idx )
        {
            for (core_idx = 0; core_idx < MIC_NATIVE_MAX_CORES; ++core_idx)
            {
                const CoreAffinityDescriptor& core = get_core_affinity_descriptor(core_idx);
                if (core.is_enabled_core())
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

    if (! m_task_executor->Init( m_numOfActivatedThreads, NULL ))
    {
        //Report Error
        printf("TaskExecutor initialization failed\n");
        assert( false && "TaskExecutor initialization failed" );
        return false;        
    }

    // setup WG pool, init WG contextes
    m_task_executor->SetWGContextPool( this );

    // Activate
    if (! m_task_executor->Activate())
    {
        //Report Error
        printf("TaskExecutor activation failed\n");
        assert( false && "TaskExecutor activation failed" );
        return false;        
    }

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

    startup_all_workers();
    
    return true;    
}

void ThreadPool::release()
{
    if (NULL != m_task_executor)
    {
        m_task_executor->Deactivate();
        m_task_executor->Finalize();
        m_task_executor = NULL;
    }
}


inline
unsigned int ThreadPool::getWorkerID( ExecutorTls& executor_tls )
{
    bool alreadyHad = false;
    unsigned int ret = (unsigned int)(size_t)executor_tls.getTls(ExecutorTls::WORKER_ID, alreadyHad);
    return alreadyHad ? ret : INVALID_WORKER_ID;
}

unsigned int ThreadPool::getWorkerID()
{
    TlsAccessor tlsAccessor;
    ExecutorTls execTls( &tlsAccessor );
    return getWorkerID( execTls );
}

inline
unsigned int ThreadPool::setWorkerID( ExecutorTls& executor_tls )
{
    unsigned int my_worker_id = ++m_workersCount;
    executor_tls.setTls(ExecutorTls::WORKER_ID, (void*)(size_t)my_worker_id);
    return my_worker_id;
}

WGContextBase* ThreadPool::GetWGContext(bool bBelongsToMasterThread)
{
    TlsAccessor tlsAccessor;
    ExecutorTls tls(&tlsAccessor);
    
    WGContext* pCtx = NULL;
    
    if ( bBelongsToMasterThread )
    {
        // For master thread we should access it's TLS to retrieve the WGContext
        // Get the WGContext instance of this thread
        pCtx = (WGContext*)tls.getTls(ExecutorTls::WG_CONTEXT);
    }
    else
    {    
        size_t uiWorkerId = getWorkerID(tls);

        if (INVALID_WORKER_ID == uiWorkerId)
        {
            // first worker attachment 
            uiWorkerId = setWorkerID(tls);

            if (useAffinity())
            {
                setCurrentThreadAffinity( uiWorkerId );
            }
        }

        // For workers we should access the global array
        assert( 0 != uiWorkerId );            
        pCtx = &m_contexts[uiWorkerId-1];
    }

    return pCtx;
}

// get WGContext for the current thread if exists
WGContext* ThreadPool::findActiveWGContext() 
{
    TlsAccessor tlsAccessor;
    ExecutorTls tls(&tlsAccessor);
    WGContext* pCtx = NULL;

    // first try as for master thread
    pCtx = (WGContext*)tls.getTls(ExecutorTls::WG_CONTEXT);

    if (NULL != pCtx)
    {
        return pCtx;
    }

    // may be current thread is a worker?
    size_t uiWorkerId = getWorkerID(tls);

    if (INVALID_WORKER_ID == uiWorkerId)
    {
        return NULL;
    }

    // For workers we should access the global array
    assert( 0 != uiWorkerId );            
    pCtx = &m_contexts[uiWorkerId-1];
    return pCtx;
}


bool ThreadPool::registerMasterThread( TlsAccessor* pTlsAccessor )
{
    assert( NULL != pTlsAccessor );
    
	WGContext* pCtx = new WGContext();
    
	assert(NULL!=pCtx && "Failed to allocate execution context (pCtx)");	

    if ( NULL == pCtx )
    {
    	return false;
    }

    // Store local WG context
	ExecutorTls tls(pTlsAccessor);
	tls.setTls(ExecutorTls::WG_CONTEXT, pCtx);

    // affinitize as master thread
    setCurrentThreadAffinity(0);
    return true;
}

void ThreadPool::unregisterMasterThread( TlsAccessor* pTlsAccessor )
{
    assert( NULL != pTlsAccessor );
    
    ExecutorTls tls(pTlsAccessor);
    WGContext* pCtx = (WGContext*)tls.getTls(ExecutorTls::WG_CONTEXT);
    tls.setTls(ExecutorTls::WG_CONTEXT, NULL);

    if (NULL != pCtx)
    {
        delete pCtx;
    }
}

void ThreadPool::setCurrentThreadAffinity( size_t worker_id )
{
    if (worker_id >= m_numOfActivatedThreads)
    {
        // in most cases this happens when device with single executable thread is requested.
        worker_id = 0;
    }
    
    const TaskExecutorCore& executor_core = m_workerId_2_executorCore[ worker_id ];
    const CoreAffinityDescriptor& core_desc = get_core_affinity_descriptor( executor_core.hw_2_level_index.hw_core_idx );
    cpu_set_t affinity = core_desc.get_thread_affinity_mask( executor_core.hw_2_level_index.hw_thread_idx );

    if (0 != sched_setaffinity( 0, sizeof(affinity), &affinity))
    {
        //Report Error
        printf("ThreadPool::setCurrentThreadAffinity sched_setaffinity error: %d\n", errno);
        assert( false && "ThreadPool::setCurrentThreadAffinity sched_setaffinity returned error" );
    }
}

void ThreadPool::NotifyAffinity(unsigned int tid, unsigned int core)
{
    // switch thread affinities
    // now assume only top level device exists - need to rewrite for sub-devices as threads will start moving between subdevices

    assert( false && "ThreadPool::NotifyAffinity not implemented yet" );
}

//
//  Warmup device
//
class WarmUpTask : public ITaskSet
{
public:
    PREPARE_SHARED_PTR(WarmUpTask)
    
    static inline SharedPtr<WarmUpTask> Allocate( unsigned int num_of_workers ) { return new WarmUpTask( num_of_workers ); }

	int		Init(size_t region[], unsigned int& regCount);
	int	AttachToThread(WGContextBase* pWgContext, size_t uiNumberOfWorkGroups, size_t firstWGID[], size_t lastWGID[]) 
	{
        return 0;
	}
	int	DetachFromThread(WGContextBase* pWgContext) { return 0; }

	// "Main loop"
	// The function is called with different 'inx' parameters for each iteration number
	void	ExecuteIteration(size_t x, size_t y, size_t z, WGContextBase* pWgContext = NULL);

    void	ExecuteAllIterations(size_t* dims, WGContextBase* pWgContext = NULL) 
                                { assert( false && "ExecuteAllIterations Not implemented" ); }

	bool	Finish(FINISH_REASON reason) { return true; }
    long    Release() { delete this; return 0; }
    std::string GetTypeName() const { return "WarmUpTask"; }

private:

    WarmUpTask( unsigned int num_of_workers ) : startup_workers_left(num_of_workers) {}
    ~WarmUpTask() {}

    AtomicCounter startup_workers_left;
};

int WarmUpTask::Init(size_t region[], unsigned int& regCount)
{
    regCount = 1;
    region[0] = startup_workers_left;
    return 0;
}

void WarmUpTask::ExecuteIteration(size_t x, size_t y, size_t z, WGContextBase* pWgContext)
{
    --startup_workers_left;
    while (0 != startup_workers_left)
    {
        hw_pause();
    }  
}

void ThreadPool::startup_all_workers() 
{
    if (m_numOfActivatedThreads < 2)
    {
        return;
    }
    
    SharedPtr<ITaskList> list = getTaskExecutor().CreateTaskList( TE_CMD_LIST_IMMEDIATE );

    if (NULL == list)
    {
        return;
    }

    SharedPtr<ITaskSet> warmup_task = WarmUpTask::Allocate( m_numOfActivatedThreads );

    list->Enqueue( warmup_task );
}

