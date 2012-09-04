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

#include "mic_device_interface.h"
#include "program_memory_manager.h"
#include "cl_sys_defines.h"
#include "native_synch_objects.h"
#include "mic_tracer.h"
#include "hw_exceptions_handler.h"
#include "native_printf.h"
#include "thread_local_storage.h"

#include "cl_dev_backend_api.h"

#include <tbb/tbb.h>
#include <tbb/task.h>

using namespace Intel::OpenCL::MICDevice;
using namespace Intel::OpenCL::UtilsNative;
using namespace Intel::OpenCL::DeviceBackend;

namespace Intel { namespace OpenCL { namespace MICDeviceNative {

// The maximum amount of worker threads.
#define MIC_NATIVE_MAX_WORKER_THREADS 256

/* Define a class "class_name" that inherit from "task_handler_class", "task_interface_class" and from "TaskContainerInterface" */
#define TASK_HANDLER_AND_TASK_INTERFACE_CLASS_DEFINITION(class_name, task_handler_class, task_interface_class)  \
	class class_name : public task_handler_class, public task_interface_class, public TaskContainerInterface \
	{ \
	public: \
	    \
		virtual ~class_name() {}; \
	    \
		virtual TaskHandler* getMyTaskHandler() { return this; } \
		\
		virtual TaskInterface* getMyTask() { return this; } \
	}

class TaskInterface;
class QueueOnDevice;

/* TaskHandler is an abstract class that manage the execution of a task. */
class TaskHandler
{
public:

	friend class NDRangeTask;
	friend class FillMemObjTask;

	enum TASK_TYPES
	{
		NDRANGE_TASK_TYPE = 0,
		FILL_MEM_OBJ_TYPE
	};

	/* Factory for TaskHandler object (In order or Out of order).
	   DO NOT delete this object, 'FinishTask' metod will delete this object. */
	static TaskHandler* TaskFactory(TASK_TYPES type, QueueOnDevice* queue, dispatcher_data* dispatcherData, misc_data* miscData);

	/* Initializing the task */
	virtual void InitTask(dispatcher_data* dispatcherData, misc_data* miscData, uint32_t in_BufferCount, void** in_ppBufferPointers, uint64_t* in_pBufferLengths, void* in_pMiscData, uint16_t in_MiscDataLength) = 0;

	/* Run the task */
	virtual void RunTask() = 0;

	/* Set errorCode as this task error (Do nothing if errorCode = CL_DEV_SUCCESS) */
	void setTaskError(cl_dev_err_code errorCode);

protected:

	TaskHandler();

	virtual ~TaskHandler();

	/* It will be call from 'run()' method (of m_task) as the last command,
	   It will release the resources and singal the user barrier if needed. 
	   It also delete this object as the last command. 
	   The FinishTask is not public because We don't want the user to release the resource. (It will release itself when completed)*/
	virtual void FinishTask(COIEVENT& completionBarrier, bool isLegalBarrier) = 0;

	// The received dispatcher_data
	dispatcher_data* m_dispatcherData;
	// The received misc_data
	misc_data* m_miscData;

	// The input from the main function
	uint32_t m_lockBufferCount;
	void** m_lockBufferPointers;
	uint64_t* m_lockBufferLengths;

	// a pointer to TaskInterface
	TaskInterface* m_task;
	QueueOnDevice* m_queue;

	// Command tracer
	CommandTracer m_commandTracer;

private:

	// Setter for the TaskInterface pointer.
	void setTaskInterface(TaskInterface* task) { m_task = task; }
	void setQueue(QueueOnDevice* queue) { m_queue = queue; }
};


/* BlockingTaskHandler inherits from "TaskHandler" and implements the functionality for Blocking task managment. */
class BlockingTaskHandler : public TaskHandler
{

public:

	virtual void InitTask(dispatcher_data* dispatcherData, misc_data* miscData, uint32_t in_BufferCount, void** in_ppBufferPointers, uint64_t* in_pBufferLengths, void* in_pMiscData, uint16_t in_MiscDataLength);

	virtual void RunTask();

protected:

	virtual ~BlockingTaskHandler() {};

	virtual void FinishTask(COIEVENT& completionBarrier, bool isLegalBarrier);
};


/* NonBlockingTaskHandler inherits from "TaskHandler" and implement the functionality for Non-Blocking task management.
   It is an abstract class becuase it doesn't implement the method "RunTask()" because its implementation is thread specific. */
class NonBlockingTaskHandler : public TaskHandler
{

public:

	virtual void InitTask(dispatcher_data* dispatcherData, misc_data* miscData, uint32_t in_BufferCount, void** in_ppBufferPointers, uint64_t* in_pBufferLengths, void* in_pMiscData, uint16_t in_MiscDataLength);

protected:

	virtual ~NonBlockingTaskHandler() {};

	virtual void FinishTask(COIEVENT& completionBarrier, bool isLegalBarrier);
};


/* TBBNonBlockingTaskHandler inherits from NonBlockingTaskHandler and implement the asynch execution call by using TBB mechanism. */
class TBBNonBlockingTaskHandler : public NonBlockingTaskHandler
{

public:

	virtual void RunTask();

protected:

	virtual ~TBBNonBlockingTaskHandler() {};
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* Interface for Task Execution. */
class TaskInterface
{
public:

	virtual ~TaskInterface() {};

	/* Initialize the task and execute pre-exe operations.
	   Return CL_DEV_SUCCESS if succeeded. */
	virtual cl_dev_err_code init(TaskHandler* pTaskHandler) = 0;

	/* Run the task */
	virtual void run() = 0;

	/* Finish the task and exexute post-exe operations.
	   In regular mode - it will call from "run()" method of this object only. 
	   Call it from other object only in case of error. (In order to execute the post-exe operations). */
	virtual void finish(TaskHandler* pTaskHandler) = 0;

	/* Is called when the task is going to be called for the first time within specific thread. uiWorkerId specifies the worker thread id.
	   Returns CL_DEV_SUCCESS, if attach process succeeded. */
	virtual cl_dev_err_code attachToThread(TlsAccessor* tlsAccessor, size_t uiWorkerId) = 0;

	/* Is called when the task will not be executed by the specific thread uiWorkerId specifies the worker thread id.
	   Returns CL_DEV_SUCCESS, if detach process succeeded. */
	virtual cl_dev_err_code	detachFromThread(TlsAccessor* tlsAccessor, size_t uiWorkerId) = 0;

	// The function is called with different 'inx' parameters for each iteration number
	virtual cl_dev_err_code executeIteration(TlsAccessor* tlsAccessor, HWExceptionsJitWrapper& hw_jit_wrapper, size_t x, size_t y, size_t z, size_t uiWorkerId = (size_t)-1) = 0;

	/* Return CommandTracer */
	virtual CommandTracer* getCommandTracerPtr() = 0;

#ifdef ENABLE_MIC_TBB_TRACER
    class PerfData 
    {
    public:
        void work_group_start();
        void work_group_end();
        
        void append_data_item( unsigned int n_coords, unsigned int col, unsigned int raw = 0, unsigned int page = 0 );
        
        static void global_init();
    private:
        unsigned long long start_time;
        unsigned long long end_time;
        unsigned long long search_time;
        unsigned int*      processed_indices;
        unsigned int       processed_indices_limit;
        unsigned int       processed_indices_current;
        
        unsigned int       m_worker_id;
        
        static const unsigned int INDICES_DELTA  = 64;

        static pthread_key_t g_phys_processor_id_tls_key;

        void resize( unsigned int n_coords ); 
        void getHwInfoForPhysProcessor( unsigned int physical_processor_id, unsigned int& core_id, unsigned int& thread_id_on_core );
        bool is_thread_recorded();
        void dump_thread_attach();

        void construct(unsigned int worker_id);
        void destruct();

        void dump_data_item( char* buffer, unsigned int n_coords, unsigned int index );

        friend class TaskInterface;

    };

    PerfData m_perf_data[MIC_NATIVE_MAX_WORKER_THREADS];

    void PerfDataInit();
    void PerfDataFini( unsigned int command_id, unsigned int dims, size_t cols, size_t raws, size_t pages );
#endif // ENABLE_MIC_TBB_TRACER
    
};

class TBBTaskInterface
{
public:

	virtual ~TBBTaskInterface() {};

	/* Return an instance of tbb:task in order to enqueue it to TBB::task::queue. */
	virtual tbb::task* getTaskExecutorObj() = 0;
};



/* NDRangeTask inherit from TaskInterface, and implements most of its' functionality.
   It is an abstract class, It does not implement the execution methods which thier implementation is depend on specific threading model. */
class NDRangeTask : public TaskInterface
{
public:

	virtual cl_dev_err_code init(TaskHandler* pTaskHandler);

	virtual void finish(TaskHandler* pTaskHandler);

	virtual cl_dev_err_code attachToThread(TlsAccessor* tlsAccessor, size_t uiWorkerId);

	virtual cl_dev_err_code	detachFromThread(TlsAccessor* tlsAccessor, size_t uiWorkerId);

	virtual cl_dev_err_code executeIteration(TlsAccessor* tlsAccessor, HWExceptionsJitWrapper& hw_jit_wrapper, size_t x, size_t y, size_t z, size_t uiWorkerId = (size_t)-1);

	/* Return CommandTracer */
	virtual CommandTracer* getCommandTracerPtr() { return m_pCommandTracer; };
	virtual QueueOnDevice* getQueue() { return m_pQueue; };

protected:

	NDRangeTask();

	virtual ~NDRangeTask();

	// uniqueue identifier for this task (command)
	cl_dev_cmd_id m_commandIdentifier;

    QueueOnDevice*        m_pQueue;
    
	ICLDevBackendKernel_* m_kernel;
	ICLDevBackendBinary_* m_pBinary;
	ProgramMemoryManager* m_progamExecutableMemoryManager;

	// Executable information
    size_t m_MemBuffCount;
    size_t* m_pMemBuffSizes;
	
	// working region
	uint64_t m_region[MAX_WORK_DIM];
	// dimensions
	unsigned int m_dim;

	// The kernel arguments blob
	char* m_lockedParams;

	// Print handle for this command.
	PrintfHandle m_printHandle;

	// CommandTracer object
	CommandTracer* m_pCommandTracer;

private:

	// Array of ICLDevBackendExecutable_ for each worker thread. When task completes, traversing over this array and calling "Relase()" method for each object that is not NULL.
	volatile ICLDevBackendExecutable_* volatile m_contextExecutableArr[MIC_NATIVE_MAX_WORKER_THREADS];
};



/* TBBNDRangeTask inherit from NDRangeTask and from TBBTaskInterface
   It impelement the missing functionality of NDRangeTask that is depenedent on specific threading model. (Using TBB in this case). */
class TBBNDRangeTask : public NDRangeTask, public TBBTaskInterface
{
public:

	/* TBBNDRangeExecutor class Inherit from tbb::task in order to be able to enqueue this object to TBB task queue. */
	class TBBNDRangeExecutor : public tbb::task
	{
	public:

		virtual ~TBBNDRangeExecutor() {};

		TBBNDRangeExecutor(TBBNDRangeTask* pTbbNDRangeTask, TaskHandler* pTaskHandler, const unsigned int& dim, uint64_t* region);

		// This method is an abstract method of tbb:task, have to implement it in order to be tbb:task object.
		// This method execute the NDRange task and call finish at the end of it (In order to release resources and to signal the completion barrier if needed).
		tbb::task* execute();

	private:

		TBBNDRangeTask* m_pTbbNDRangeTask;

		TaskHandler* m_taskHandler;

		// The task dimension.
		unsigned int m_dim;

		// The task region.
		uint64_t* m_region;
	};



	TBBNDRangeTask();

	virtual ~TBBNDRangeTask() {};

	virtual cl_dev_err_code init(TaskHandler* pTaskHandler);

	// It is only delegation method to "execute()" of TBBNDRangeExecutor.
	virtual void run();

	/* Return the instance of TBBNDRangeExecutor in order to enqueue it to TBB::task::queue. */
	virtual tbb::task* getTaskExecutorObj() {   assert(m_pTaskExecutor);
												return m_pTaskExecutor; };

private:

	TBBNDRangeExecutor* m_pTaskExecutor;
};


/* Interface that define two methods that must be implement for each Task class. */
class TaskContainerInterface
{
public:

	virtual ~TaskContainerInterface() {};

	/* Return this object as Taskhandler. */
	virtual TaskHandler* getMyTaskHandler() = 0;

	/* Return this object as TaskInterface. */
	virtual TaskInterface* getMyTask() = 0;
};

// Define the class BlockingNDRangeTask.
TASK_HANDLER_AND_TASK_INTERFACE_CLASS_DEFINITION(BlockingNDRangeTask, BlockingTaskHandler, TBBNDRangeTask);
// Define the class NonBlockingNDRangeTask.
TASK_HANDLER_AND_TASK_INTERFACE_CLASS_DEFINITION(NonBlockingNDRangeTask, TBBNonBlockingTaskHandler, TBBNDRangeTask);

/* A singleton class that provide API for thread pool services */
class ThreadPool
{
public:

	/* Return the singleton instance of ThreadPool.
	   Assume that it calls first when process creates - with single thread so it is NOT thread safe function. */
	static ThreadPool* getInstance();

	/* Release the singleton instance if not NULL. 
	   Assume that it calls before closing the process - it is NOT thread safe function. */
	static void releaseSingletonInstance();

	/* Call this method only once after construction in order to create the worker threads pool (The amount of worker threads are numOfWorkers).*/
	virtual bool init() = 0;

	virtual void release() = 0;

    /* startup all threads in a pool */
    virtual void wakeup_all();

	/* Return current thread worker ID (worker ID of muster thread is always 0 and for worker thread >= 1). */
	virtual size_t getWorkerID(TlsAccessor* pTlsAccessor) = 0;

	/* Register muster thread to thread pool. */
	virtual void registerMasterThread(bool affinitize = true) = 0;

	/* Unregister muster thread from thread pool. */
	virtual void unregisterMasterThread() = 0;

protected:

	ThreadPool();

	virtual ~ThreadPool();

	/* initialize the m_orderHwThreadsIds list for the req. affinity order. */
	bool initializeAffinityThreads();

	/* Take threads IDs from 'm_orderHwThreadsIds' and save them in 'm_reserveHwThreadsIDs' that will be use for special cases affinity. (such as for master threads) */
	virtual void initializeReserveAffinityThreadIds() = 0;

	/* Affinities the current thread. 
	   Set affinity only if mic_exec_env_options.use_affinity = true*/
	bool setAffinityForCurrentThread();

	/* Set affinity for current thread from reserved IDs list (if m_reserveHwThreadsIDs.size() > 0 and didn't affinitized from reserved IDs yet). */
	bool setAffinityFromReservedIDs();

	/* Update 'm_reserveHwThreadsIDs' list (add to it the HW thread ID that I was affinitized to). 
	   This method assume that this thread is going to close, and does not change the affinity setting of it. */
	bool releaseReservedAffinity();

	/* Return the next available worker ID (Use it for workers only. (The first ID is 1) */
	unsigned int getNextWorkerID() { unsigned int myID = m_NextWorkerID++;
									 assert(myID <= MIC_NATIVE_MAX_WORKER_THREADS);
	                                 return myID; };

	// The amount of worker threads.
	unsigned int m_numOfWorkers;
	
	// Atomic counter that define the worker ID of each worker thread. In case of Muster thread the ID is 0.
	AtomicCounterNative m_NextWorkerID;

	// Order list of HW threads IDs, the different from 'm_orderHwThreadsIds' is that the IDs in 'm_orderHwThreadsIds' are for common worker threads and the IDs in 'm_reserveHwThreadsIDs',
	// are reserved for special threads (for example for master threads)
	vector<unsigned int> m_reserveHwThreadsIDs;

	// order list of HW threads IDs - Need it in order to affinities the worker threads. 
	// (The order will be  - all the 1st HW threads of all cores, than all the 2nd, and so on... minus core 0 / last core if it set in "mic_exec_env_options" structure).
	vector<unsigned int> m_orderHwThreadsIds;

    virtual void startup_all_workers() = 0;

private:

	bool setAffinityForCurrentThread(unsigned int hwThreadId);

	// The next index in "m_orderHwThreadsIds" for affinity. (If m_nextAffinitiesThreadIndex == m_orderHwThreadsIds.size() ==> set m_nextAffinitiesThreadIndex = 0)
	AtomicCounterNative m_nextAffinitiesThreadIndex;

	// map from OS thread ID to HW thread ID.
	map<pthread_t, unsigned int> m_osThreadToHwThread;

	// Lock keeper for m_reserveHwThreadsIDs
	pthread_mutex_t m_reserveHwThreadsLock;

	// The singleton thread pool
	static ThreadPool* m_singleThreadPool;
    
    static OclMutexNative m_workers_initialization_lock;
    static volatile bool  m_workers_initialized;
};


#define INVALID_WORKER_ID 0xFFFFFFFF
#define INVALID_SCHEDULER_ID 0xFFFFFFFF

/* Class TBBThreadPool inherits from ThreadPool and from tbb::task_scheduler_observer. */ 
class TBBThreadPool : public ThreadPool, public tbb::task_scheduler_observer
{
private:

    class TrapWorkers;
    class TrapperTask : public tbb::task 
    {
    private:
        TrapWorkers& m_owner;
    
    public:
        TrapperTask( TrapWorkers& o ) : m_owner(o) {};
    
        tbb::task* execute ();
    };
    
    class  TrapWorkers
    {
    public:
        TrapWorkers();

        void fire();
        void release();
        
    private:
        tbb::task *my_root;
        tbb::task_group_context my_context;
        AtomicCounterNative startup_workers_left;
        AtomicCounterNative shutdown_workers_left;
        unsigned int m_workers_count;

        friend class TrapperTask;
    };
    
public:

	TBBThreadPool() {};

	virtual ~TBBThreadPool() { release(); };

	virtual bool init();

	virtual void release();

	virtual size_t getWorkerID(TlsAccessor* pTlsAccessor);

	virtual void registerMasterThread(bool affinitize = true);

	virtual void unregisterMasterThread();

	/* The task scheduler invokes this method on each thread that starts participating in task scheduling, if observing is enabled. */
	virtual void on_scheduler_entry(bool is_worker);
	
	/* The task scheduler invokes this method when a thread stops participating in task scheduling, if observing is enabled. */
	virtual void on_scheduler_exit(bool is_worker);

protected:

	/* It is not thread safe implementation because assuming that it calls by one thread when calling to 'init_device()' function. */
	void initializeReserveAffinityThreadIds();

    virtual void startup_all_workers();
    
private:

	tbb::task_scheduler_init* getScheduler(TlsAccessor* pTlsAccessor);

	void setScheduler(TlsAccessor* pTlsAccessor, tbb::task_scheduler_init* init);

	void setWorkerID(TlsAccessor* pTlsAccessor, size_t id);
	
	bool isWorkerScheduler(TlsAccessor* pTlsAccessor) { return (INVALID_SCHEDULER_ID == (cl_ulong)getScheduler(pTlsAccessor)); };

    struct WorkersWakeup;
    TrapWorkers m_workers_trapper;
};

//
// Queue - saved in COI thread TLS
//
class QueueOnDevice
{
public:
    QueueOnDevice( bool is_in_order ) : m_is_in_order(is_in_order) {};

    bool isInOrder() const { return m_is_in_order; };

    tbb::affinity_partitioner* getAffinityPartitioner() { return (m_is_in_order) ? &m_affinity_partitioner : NULL; };

    static QueueOnDevice* getCurrentQueue() 
        {
			TlsAccessor tlsAccessor;
			QueueTls queueTls(&tlsAccessor);
            return (QueueOnDevice*)(queueTls.getTls(QueueTls::QUEUE_TLS_ENTRY)); 
        };
    
    static void setCurrentQueue( QueueOnDevice* queue ) 
        {
			TlsAccessor tlsAccessor;
			QueueTls queueTls(&tlsAccessor);
            queueTls.setTls(QueueTls::QUEUE_TLS_ENTRY, queue); 
        };
    
private:
    bool                        m_is_in_order;
    tbb::affinity_partitioner   m_affinity_partitioner;
};

}}}

