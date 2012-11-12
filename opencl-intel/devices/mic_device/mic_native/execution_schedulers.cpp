#include "execution_task.h"
#include "mic_tracer.h"
#include "mic_blocked_ranges.h"
#include "native_globals.h"
#include "native_common_macros.h"

#include <list>

using namespace Intel::OpenCL::MICDeviceNative;

namespace Intel { namespace OpenCL { namespace MICDeviceNative {

#ifdef ENABLE_MIC_TRACER
	struct TaskLoopBodyTrace {
	public:
		TaskLoopBodyTrace(CommandTracer* pCmdTracer, size_t numOfWorkGroups) : m_pCommandTracer(pCmdTracer), m_cpuId(0) { init(numOfWorkGroups); }

		void finish()
		{
			unsigned long long end = CommandTracer::_RDTSC();
			unsigned long long delta = end - m_start;
			m_pCommandTracer->add_delta_time_thread_overall_time(delta, m_cpuId);
		}
	private:

		void init(unsigned long long numOfWorkGroups)
		{
			m_start = CommandTracer::_RDTSC();
			m_cpuId = hw_cpu_idx();
			m_pCommandTracer->increment_thread_num_of_invocations(m_cpuId);
			m_pCommandTracer->add_delta_thread_num_wg_exe(numOfWorkGroups, m_cpuId);
		}

		CommandTracer* m_pCommandTracer;

		unsigned int m_cpuId;
		unsigned long long m_start;
	};
#endif

    template <class blocked_range_1d>
	struct TaskLoopBody1D {
		TaskInterface* task;
		bool* result;
		TaskLoopBody1D(TaskInterface* t, bool* res) : task(t), result(res) {}
		virtual ~TaskLoopBody1D() {}
		void operator()(const blocked_range_1d& r) const {
#ifdef ENABLE_MIC_TRACER
			TaskLoopBodyTrace tTrace = TaskLoopBodyTrace(task->getCommandTracerPtr(), r.size());
#endif
			TlsAccessor tlsAccessor;
			size_t uiWorkerId = ThreadPool::getInstance()->getWorkerID(&tlsAccessor);
			size_t uiNumberOfWorkGroups = r.size();
            assert(uiNumberOfWorkGroups <= CL_MAX_INT32);

#ifdef ENABLE_MIC_TBB_TRACER
            TaskInterface::PerfData& perf_data = task->m_perf_data[uiWorkerId];
            perf_data.work_group_start();
#endif // ENABLE_MIC_TBB_TRACER

			if (CL_DEV_FAILED(task->attachToThread(&tlsAccessor, uiWorkerId)))
			{
				*result = false;
				assert(0);
				return;
			}

			bool tResult = true;
            HWExceptionsJitWrapper hw_wrapper(&tlsAccessor);            
			for(size_t k = r.begin(), f = r.end(); k < f; k++ )
			{
#ifdef ENABLE_MIC_TBB_TRACER                
                perf_data.append_data_item( 1, (unsigned int)k );
#endif // ENABLE_MIC_TBB_TRACER
                tResult = tResult && CL_DEV_SUCCEEDED( task->executeIteration(&tlsAccessor, hw_wrapper, k, 0, 0, uiWorkerId) );
			}
			tResult = tResult && CL_DEV_SUCCEEDED( task->detachFromThread(&tlsAccessor, uiWorkerId) );
			if (false == tResult)
			{
				*result = false;
			}

#ifdef ENABLE_MIC_TBB_TRACER                
            perf_data.work_group_end();
#endif // ENABLE_MIC_TBB_TRACER
#ifdef ENABLE_MIC_TRACER
			tTrace.finish();
#endif
		}
	};

    template <class blocked_range_2d>
	struct TaskLoopBody2D {
		TaskInterface* task;
		bool* result;
		TaskLoopBody2D(TaskInterface* t, bool* res) : task(t), result(res) {}
		virtual ~TaskLoopBody2D() {}
		void operator()(const blocked_range_2d& r) const {
#ifdef ENABLE_MIC_TRACER
			TaskLoopBodyTrace tTrace = TaskLoopBodyTrace(task->getCommandTracerPtr(), (r.rows().size())*(r.cols().size()));
#endif
			TlsAccessor tlsAccessor;
			size_t uiWorkerId = ThreadPool::getInstance()->getWorkerID(&tlsAccessor);
			size_t uiNumberOfWorkGroups = (r.rows().size())*(r.cols().size());
            assert(uiNumberOfWorkGroups <= CL_MAX_INT32);

#ifdef ENABLE_MIC_TBB_TRACER
            TaskInterface::PerfData& perf_data = task->m_perf_data[uiWorkerId];
            perf_data.work_group_start();
#endif // ENABLE_MIC_TBB_TRACER

			if (CL_DEV_FAILED(task->attachToThread(&tlsAccessor, uiWorkerId)))
			{
				*result = false;
				assert(0);
				return;
			}
            
			bool tResult = true;
            HWExceptionsJitWrapper hw_wrapper(&tlsAccessor);            
			for(size_t j = r.rows().begin(), d = r.rows().end(); j < d; j++ )
				for(size_t k = r.cols().begin(), f = r.cols().end(); k < f; k++ )
				{
#ifdef ENABLE_MIC_TBB_TRACER                
                    perf_data.append_data_item( 2, (unsigned int)k, (unsigned int)j );
#endif // ENABLE_MIC_TBB_TRACER
					tResult = tResult && CL_DEV_SUCCEEDED( task->executeIteration(&tlsAccessor, hw_wrapper, k, j, 0, uiWorkerId) );
				}
			tResult = tResult && CL_DEV_SUCCEEDED( task->detachFromThread(&tlsAccessor, uiWorkerId) );
			if (false == tResult)
			{
				*result = false;
			}
#ifdef ENABLE_MIC_TBB_TRACER                
            perf_data.work_group_end();
#endif // ENABLE_MIC_TBB_TRACER
#ifdef ENABLE_MIC_TRACER
			tTrace.finish();
#endif
		}
	};

    template <class blocked_range_3d>
	struct TaskLoopBody3D {
		TaskInterface* task;
		bool* result;
		TaskLoopBody3D(TaskInterface* t, bool* res) : task(t), result(res) {}
		virtual ~TaskLoopBody3D() {}
		void operator()(const blocked_range_3d& r) const {
#ifdef ENABLE_MIC_TRACER
			TaskLoopBodyTrace tTrace = TaskLoopBodyTrace(task->getCommandTracerPtr(), (r.pages().size())*(r.rows().size())*(r.cols().size()));
#endif
			TlsAccessor tlsAccessor;
			size_t uiWorkerId = ThreadPool::getInstance()->getWorkerID(&tlsAccessor);
			size_t uiNumberOfWorkGroups = (r.pages().size())*(r.rows().size())*(r.cols().size());
            assert(uiNumberOfWorkGroups <= CL_MAX_INT32);

#ifdef ENABLE_MIC_TBB_TRACER
            TaskInterface::PerfData& perf_data = task->m_perf_data[uiWorkerId];
            perf_data.work_group_start();
#endif // ENABLE_MIC_TBB_TRACER
			if (CL_DEV_FAILED(task->attachToThread(&tlsAccessor, uiWorkerId)))
			{
				*result = false;
				assert(0);
				return;
			}
            
			bool tResult = true;
            HWExceptionsJitWrapper hw_wrapper(&tlsAccessor);            
            for(size_t i = r.pages().begin(), e = r.pages().end(); i < e; i++ )
				for(size_t j = r.rows().begin(), d = r.rows().end(); j < d; j++ )
					for(size_t k = r.cols().begin(), f = r.cols().end(); k < f; k++ )
					{
#ifdef ENABLE_MIC_TBB_TRACER                
                        perf_data.append_data_item( 3, (unsigned int)k, (unsigned int)j, (unsigned int)i );
#endif // ENABLE_MIC_TBB_TRACER
						tResult = tResult && CL_DEV_SUCCEEDED( task->executeIteration(&tlsAccessor, hw_wrapper, k, j, i, uiWorkerId) );
					}
			tResult = tResult && CL_DEV_SUCCEEDED( task->detachFromThread(&tlsAccessor, uiWorkerId) );
			if (false == tResult)
			{
				*result = false;
			}
#ifdef ENABLE_MIC_TBB_TRACER                
            perf_data.work_group_end();
#endif // ENABLE_MIC_TBB_TRACER
#ifdef ENABLE_MIC_TRACER
			tTrace.finish();
#endif
		}
	};
}}};

TBBNDRangeTask::TBBNDRangeExecutor::TBBNDRangeExecutor(TBBNDRangeTask* pTbbNDRangeTask, TaskHandler* pTaskHandler, const unsigned int& dim, uint64_t* region) : m_pTbbNDRangeTask(pTbbNDRangeTask), 
m_taskHandler(pTaskHandler), m_dim(dim), m_region(region)
{
}

template < class blocked_range_1d, class TaskLoopBody >
class SubTaskLoopBody : public tbb::task
{
public:
    SubTaskLoopBody( const blocked_range_1d& r, 
                     const TaskLoopBody& t) :
        m_r(r), m_t(t) {};

    tbb::task* execute()
    {
        tbb::parallel_for(m_r, m_t, tbb::auto_partitioner());
        return NULL;
    }

private:
    blocked_range_1d m_r;
    TaskLoopBody m_t;
};

template< typename block_range >
void split_ranges( unsigned int number_of_slots, const block_range& input, list<block_range>& output )
{
    list<block_range>     list1,    list2;
    list<block_range>     *list_in, *list_out;
    
    list_in   = &list1;
    list_out  = &list2;
    list_in->push_back( input );

    while ((list_in->size() > 0) && 
           ((output.size() + list_in->size()) < number_of_slots))
    {
        while (list_in->size() > 0)
        {
            block_range orig = list_in->front();
            list_in->pop_front();
    
            if (orig.is_divisible())
            {
                block_range  left_part( orig, tbb::split() );
                list_out->push_back( orig );
                list_out->push_back( left_part );
            }
            else
            {
                output.push_back( orig );
            }
        }

        list<block_range>* tmp_list = list_in;
        list_in = list_out;
        list_out = tmp_list;
        
    }

    output.insert( output.end(), list_in->begin(), list_in->end() );
}

class SlotAffinityAssignmentStruct
{
public:
    SlotAffinityAssignmentStruct(unsigned int slots) : m_aff(2), m_slots(slots) 
    {
        if (m_aff > m_slots)
        {
            m_aff = m_slots;
        }
    };

    operator unsigned int() { return m_aff; };

    void update() 
    {
        // if more than slots - lock affinity
        if (m_aff > 0)
        {
            ++m_aff;
            if (m_aff > m_slots)
            {
                m_aff = 1;
            }
        }        
    }
    
private:
    unsigned int m_aff;
    const unsigned int m_slots;
};

template < typename BlockedRange, typename blocked_range_1d >
static
void split_tasks( unsigned int number_of_slots, const BlockedRange& range, 
                  const Intel::OpenCL::MICDeviceNative::TaskLoopBody1D<blocked_range_1d>& task, 
                  tbb::task_list& tasks )
{
    size_t group_size = range.size() / number_of_slots;
    if (0 == group_size)
    {
        group_size = 1;
    }
    
    size_t                       start = 0;
    SlotAffinityAssignmentStruct aff(number_of_slots);
    
    while (start < range.size())
    {
        size_t end = start + group_size;
        if (end > range.size())
        {
            group_size = 1;
            end = start+group_size;
        }

        BlockedRange internal_range( start, end, (unsigned int)range.grainsize() );

        tbb::task* t = new(tbb::task::allocate_root()) 
                           SubTaskLoopBody< blocked_range_1d,
                                            Intel::OpenCL::MICDeviceNative::TaskLoopBody1D<blocked_range_1d> >
                                          ( internal_range, task );
        t->set_affinity( aff );
        tasks.push_back(*t);

        // update slot affinity
        aff.update();            
        start = end;
    }
}

template < typename BlockedRange, typename blocked_range_2d >
static
void split_tasks( unsigned int number_of_slots, const BlockedRange& range, 
                  const Intel::OpenCL::MICDeviceNative::TaskLoopBody2D<blocked_range_2d>& task, 
                  tbb::task_list& tasks )
{
    BlockedRange internal_range( range.rows().begin(), range.rows().end(),
                                 range.cols().begin(), range.cols().end(), 1 );
                                                
        
    list<BlockedRange> list_of_ranges;
    
    split_ranges( number_of_slots, internal_range, list_of_ranges );

    SlotAffinityAssignmentStruct aff(number_of_slots);
    
    typename list<BlockedRange>::iterator it     = list_of_ranges.begin();
    typename list<BlockedRange>::iterator it_end = list_of_ranges.end();

    for(; it != it_end; ++it)
    {
        BlockedRange& r = *it;

        BlockedRange internal_range( r.rows().begin(), r.rows().end(),
                                     r.cols().begin(), r.cols().end(), range.grainsize() );
        
        tbb::task* t = new(tbb::task::allocate_root()) 
                       SubTaskLoopBody< blocked_range_2d,
                                        Intel::OpenCL::MICDeviceNative::TaskLoopBody2D<blocked_range_2d> >
                                      ( internal_range, task );

        t->set_affinity( aff );
        tasks.push_back(*t);

        // update slot affinity
        aff.update();                    
    }
}

template < typename BlockedRange, typename blocked_range_3d >
static
void split_tasks( unsigned int number_of_slots, const BlockedRange& range, 
                  const Intel::OpenCL::MICDeviceNative::TaskLoopBody3D<blocked_range_3d>& task, 
                  tbb::task_list& tasks )
{
    BlockedRange    internal_range( range.pages().begin(),range.pages().end(),
                                    range.rows().begin(), range.rows().end(), 
                                    range.cols().begin(), range.cols().end(), 1 );
                                                
        
    list<BlockedRange> list_of_ranges;
    
    split_ranges( number_of_slots, internal_range, list_of_ranges );

    SlotAffinityAssignmentStruct aff(number_of_slots);
    
    typename list<BlockedRange>::iterator it     = list_of_ranges.begin();
    typename list<BlockedRange>::iterator it_end = list_of_ranges.end();

    for(; it != it_end; ++it)
    {
        BlockedRange& r = *it;

        BlockedRange internal_range( r.pages().begin(), r.pages().end(),
                                     r.rows().begin(),  r.rows().end(),
                                     r.cols().begin(),  r.cols().end(), range.grainsize() );
        
        tbb::task* t = new(tbb::task::allocate_root()) 
                       SubTaskLoopBody< blocked_range_3d,
                                        Intel::OpenCL::MICDeviceNative::TaskLoopBody3D<blocked_range_3d> >
                                      ( internal_range, task );

        t->set_affinity( aff );
        tasks.push_back(*t);

        // update slot affinity
        aff.update();                    
    }
}

template <class TbbBlockedRange, class BlockedRange, class TaskLoopBody>        
void TBBNDRangeTask::TBBNDRangeExecutor::auto_executor( const BlockedRangeBase& range, 
                                                        tbb::affinity_partitioner* ap, 
                                                        tbb::task_list* task_list,
                                                        bool* result )
{
    const SpecificBlockedRange<TbbBlockedRange>* my_range = (const SpecificBlockedRange<TbbBlockedRange>*)(&range);
    tbb::parallel_for(BlockedRange(my_range->m_range), TaskLoopBody(m_pTbbNDRangeTask, result), tbb::auto_partitioner());
}

template <class TbbBlockedRange, class BlockedRange, class TaskLoopBody>        
void TBBNDRangeTask::TBBNDRangeExecutor::affinity_executor( const BlockedRangeBase& range, 
                                                        tbb::affinity_partitioner* ap, 
                                                        tbb::task_list* task_list,
                                                        bool* result )
{
    const SpecificBlockedRange<TbbBlockedRange>* my_range = (const SpecificBlockedRange<TbbBlockedRange>*)(&range);
    tbb::parallel_for(BlockedRange(my_range->m_range), TaskLoopBody(m_pTbbNDRangeTask, result), *ap);
}

template <class TbbBlockedRange, class BlockedRange, class TaskLoopBody>        
void TBBNDRangeTask::TBBNDRangeExecutor::openmp_executor( const BlockedRangeBase& range,  
                                                        tbb::affinity_partitioner* ap, 
                                                        tbb::task_list* task_list,
                                                        bool* result )
{
    const SpecificBlockedRange<TbbBlockedRange>* my_range = (const SpecificBlockedRange<TbbBlockedRange>*)(&range);
    split_tasks( gMicExecEnvOptions.num_of_worker_threads, 
                 BlockedRange(my_range->m_range), 
                 TaskLoopBody(m_pTbbNDRangeTask, result), *task_list );
}

#define DEFINE_EXECUTOR_DIMS_ARRAY( name, ExecutorFuncName, BlockedRangeNamePrefix )                             \
    TBBNDRangeTask::TBBNDRangeExecutor::ExecutorFunc TBBNDRangeTask::TBBNDRangeExecutor::name[MAX_WORK_DIM] =    \
    {                                                                                                            \
        &TBBNDRangeTask::TBBNDRangeExecutor::ExecutorFuncName < tbb::blocked_range<int>,                         \
                                                                BlockedRangeNamePrefix ## 1d,                    \
                                                                TaskLoopBody1D< BlockedRangeNamePrefix ## 1d > >,\
        &TBBNDRangeTask::TBBNDRangeExecutor::ExecutorFuncName < tbb::blocked_range2d<int>,                       \
                                                                BlockedRangeNamePrefix ## 2d,                    \
                                                                TaskLoopBody2D< BlockedRangeNamePrefix ## 2d > >,\
        &TBBNDRangeTask::TBBNDRangeExecutor::ExecutorFuncName < tbb::blocked_range3d<int>,                       \
                                                                BlockedRangeNamePrefix ## 3d,                    \
                                                                TaskLoopBody3D< BlockedRangeNamePrefix ## 3d > > \
    }

DEFINE_EXECUTOR_DIMS_ARRAY( auto_block_default,     auto_executor,      BlockedRangeByDefaultTBB );
DEFINE_EXECUTOR_DIMS_ARRAY( affinity_block_default, affinity_executor,  BlockedRangeByDefaultTBB );
DEFINE_EXECUTOR_DIMS_ARRAY( openmp_block_default,   openmp_executor,    BlockedRangeByDefaultTBB );

DEFINE_EXECUTOR_DIMS_ARRAY( auto_block_row,         auto_executor,      BlockedRangeByRow );
DEFINE_EXECUTOR_DIMS_ARRAY( affinity_block_row,     affinity_executor,  BlockedRangeByRow );
DEFINE_EXECUTOR_DIMS_ARRAY( openmp_block_row,       openmp_executor,    BlockedRangeByRow );

DEFINE_EXECUTOR_DIMS_ARRAY( auto_block_column,      auto_executor,      BlockedRangeByColumn );   
DEFINE_EXECUTOR_DIMS_ARRAY( affinity_block_column,  affinity_executor,  BlockedRangeByColumn );
DEFINE_EXECUTOR_DIMS_ARRAY( openmp_block_column,    openmp_executor,    BlockedRangeByColumn);

DEFINE_EXECUTOR_DIMS_ARRAY( auto_block_tile,        auto_executor,      BlockedRangeByTile );
DEFINE_EXECUTOR_DIMS_ARRAY( affinity_block_tile,    affinity_executor,  BlockedRangeByTile );
DEFINE_EXECUTOR_DIMS_ARRAY( openmp_block_tile,      openmp_executor,    BlockedRangeByTile );

TBBNDRangeTask::TBBNDRangeExecutor::ExecutorFunc*  
TBBNDRangeTask::TBBNDRangeExecutor::g_executor[mic_TBB_scheduler_LAST][mic_TBB_block_by_LAST] = 
{
                  /* default by TBB tile */   /* by_row  */         /* by_column */         /* by_tile */
/* auto     */{   auto_block_default,         auto_block_row,       auto_block_column,      auto_block_tile         },
/* affinity */{   affinity_block_default,     affinity_block_row,   affinity_block_column,  affinity_block_tile     },
/* openmp   */{   openmp_block_default,       openmp_block_row,     openmp_block_column,    openmp_block_tile       }
};

tbb::task* TBBNDRangeTask::TBBNDRangeExecutor::execute()
{
	assert(m_pTbbNDRangeTask);
	assert(m_taskHandler);

	m_taskHandler->StartTaskSignaling();

#ifdef NDRANGE_UNIT_TEST
	foo(m_lockedParams);
	m_pTaskExecutor->finish(m_taskHandler);
	return NULL;
#endif

	unsigned int grainSize = gMicExecEnvOptions.use_TBB_grain_size;

	// Set execution start for tracer
	m_pTbbNDRangeTask->getCommandTracerPtr()->set_current_time_tbb_exe_in_device_time_start();

    tbb::affinity_partitioner* ap = m_pTbbNDRangeTask->getQueue()->getAffinityPartitioner();
    mic_TBB_scheduler use_scheduler = gMicExecEnvOptions.tbb_scheduler;

    if ((NULL == ap) && (mic_TBB_affinity == use_scheduler))
    {
        use_scheduler = mic_TBB_auto;
    }

	bool result = true;

    tbb::task_list task_list;
    
	if (1 == m_dim)
	{
		assert(m_region[0] <= CL_MAX_INT32);
        
        typedef tbb::blocked_range<int> my_block_range;
        ExecutorFunc func = g_executor[gMicExecEnvOptions.tbb_scheduler][gMicExecEnvOptions.tbb_block_optimization][0];

        (*this.*func)( SpecificBlockedRange<my_block_range>(my_block_range
                       (
                         0, (int)m_region[0], grainSize
                       )), 
                       ap, &task_list, &result );
	}
	else if (2 == m_dim)
	{
		assert(m_region[0] <= CL_MAX_INT32);
		assert(m_region[1] <= CL_MAX_INT32);

        typedef tbb::blocked_range2d<int> my_block_range;
        ExecutorFunc func = g_executor[gMicExecEnvOptions.tbb_scheduler][gMicExecEnvOptions.tbb_block_optimization][1];

        (*this.*func)( SpecificBlockedRange<my_block_range>(my_block_range
                       (
                         0, (int)m_region[1], grainSize,
                         0, (int)m_region[0], grainSize
                       )), 
                       ap, &task_list, &result );
    }
	else
	{
		assert(m_region[0] <= CL_MAX_INT32);
		assert(m_region[1] <= CL_MAX_INT32);
		assert(m_region[2] <= CL_MAX_INT32);

        typedef tbb::blocked_range3d<int> my_block_range;
        ExecutorFunc func = g_executor[gMicExecEnvOptions.tbb_scheduler][gMicExecEnvOptions.tbb_block_optimization][2];

        (*this.*func)( SpecificBlockedRange<my_block_range>(my_block_range
                       (
                         0, (int)m_region[2], grainSize,
                         0, (int)m_region[1], grainSize,
                         0, (int)m_region[0], grainSize
                       )), 
                       ap, &task_list, &result );
	}

    if (!task_list.empty())
    {
        tbb::task::spawn_root_and_wait(task_list);
    }

	if (false == result)
	{
		m_taskHandler->setTaskError( CL_DEV_ERROR_FAIL );
	}

	// Set execution start for tracer
	m_pTbbNDRangeTask->getCommandTracerPtr()->set_current_time_tbb_exe_in_device_time_end();

	// Call to "finish()" as the last command in order to release resources and notify for completion (in case of OOO).
	m_pTbbNDRangeTask->finish(m_taskHandler);
	return NULL;
}

