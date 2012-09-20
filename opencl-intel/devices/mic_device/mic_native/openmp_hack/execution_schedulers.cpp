#include "execution_task.h"
#include "mic_tracer.h"
#include "mic_blocked_ranges.h"
#include "native_globals.h"
#include "native_common_macros.h"

#include <list>

#include <omp.h>

extern omp_sched_t use_omp_scheduler;

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

        struct OMPTaskLoopBody1D {
            TaskInterface* task;
            bool* result;
            OMPTaskLoopBody1D(TaskInterface* t, bool* res) : task(t), result(res) {}
            virtual ~OMPTaskLoopBody1D() {}
            void operator()(int i) const {
#ifdef ENABLE_MIC_TRACER
                TaskLoopBodyTrace tTrace = TaskLoopBodyTrace(task->getCommandTracerPtr(), 1);
#endif
			    TlsAccessor tlsAccessor;
			    size_t uiWorkerId = ThreadPool::getInstance()->getWorkerID(&tlsAccessor);
                size_t uiNumberOfWorkGroups = 1;
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
                for(size_t k = i, f = i+1; k < f; k++ )
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


	struct OMPTaskLoopBody2D {
		TaskInterface* task;
		bool* result;
		OMPTaskLoopBody2D(TaskInterface* t, bool* res) : task(t), result(res) {}
		virtual ~OMPTaskLoopBody2D() {}
		void operator()(unsigned int i, unsigned int j) const {
#ifdef ENABLE_MIC_TRACER
			TaskLoopBodyTrace tTrace = TaskLoopBodyTrace(task->getCommandTracerPtr(), 1);
#endif
			TlsAccessor tlsAccessor;
			size_t uiWorkerId = ThreadPool::getInstance()->getWorkerID(&tlsAccessor);
			size_t uiNumberOfWorkGroups = 1;
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
				{
#ifdef ENABLE_MIC_TBB_TRACER                
                    perf_data.append_data_item( 2, (unsigned int)j, (unsigned int)i );
#endif // ENABLE_MIC_TBB_TRACER
					tResult = tResult && CL_DEV_SUCCEEDED( task->executeIteration(&tlsAccessor, hw_wrapper, j, i, 0, uiWorkerId) );
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

	struct OMPTaskLoopBody3D {
		TaskInterface* task;
		bool* result;
		OMPTaskLoopBody3D(TaskInterface* t, bool* res) : task(t), result(res) {}
		virtual ~OMPTaskLoopBody3D() {}
		void operator()(unsigned int i, unsigned int j, unsigned int k) const {
#ifdef ENABLE_MIC_TRACER
			TaskLoopBodyTrace tTrace = TaskLoopBodyTrace(task->getCommandTracerPtr(), 1);
#endif
			TlsAccessor tlsAccessor;
			size_t uiWorkerId = ThreadPool::getInstance()->getWorkerID(&tlsAccessor);
			size_t uiNumberOfWorkGroups = 1;
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

tbb::task* TBBNDRangeTask::TBBNDRangeExecutor::execute()
{
	assert(m_pTbbNDRangeTask);
	assert(m_taskHandler);
#ifdef NDRANGE_UNIT_TEST
	foo(m_lockedParams);
	m_pTaskExecutor->finish(m_taskHandler);
	return NULL;
#endif

	unsigned int grainSize = gMicExecEnvOptions.use_TBB_grain_size;

	// Set execution start for tracer
	m_pTbbNDRangeTask->getCommandTracerPtr()->set_current_time_tbb_exe_in_device_time_start();

	bool result = true;

	if (1 == m_dim)
	{
		assert(m_region[0] <= CL_MAX_INT32);

        unsigned int limit = (int)m_region[0];
        omp_set_num_threads( gMicExecEnvOptions.num_of_worker_threads );
        omp_set_schedule(use_omp_scheduler, 0);

#pragma omp parallel for 
    for (unsigned int i = 0; i < limit; ++i)
    {
            OMPTaskLoopBody1D t(m_pTbbNDRangeTask, &result);
            t(i);
    }

	}
	else if (2 == m_dim)
	{
		assert(m_region[0] <= CL_MAX_INT32);
		assert(m_region[1] <= CL_MAX_INT32);

        unsigned int limit_j = (int)m_region[0];
        unsigned int limit_i = (int)m_region[1];

        omp_set_num_threads( gMicExecEnvOptions.num_of_worker_threads );
        omp_set_schedule(use_omp_scheduler, 0);

            #pragma omp parallel for
            for (unsigned int j = 0; j < limit_j; ++j)
            {
                #pragma omp parallel for
                for (unsigned int i = 0; i < limit_i; ++i)
                {
                        OMPTaskLoopBody2D t(m_pTbbNDRangeTask, &result);
                        t(j, i);
                }
            }
	}
	else
	{
		assert(m_region[0] <= CL_MAX_INT32);
		assert(m_region[1] <= CL_MAX_INT32);
		assert(m_region[2] <= CL_MAX_INT32);

        unsigned int limit_j = (int)m_region[0];
        unsigned int limit_i = (int)m_region[1];
        unsigned int limit_k = (int)m_region[2];

        omp_set_num_threads( gMicExecEnvOptions.num_of_worker_threads );
        omp_set_schedule(use_omp_scheduler, 0);

         #pragma omp parallel for
         for (unsigned int j = 0; j < limit_j; ++j)
         {
            #pragma omp parallel for
            for (unsigned int i = 0; i < limit_i; ++i)
            {
                #pragma omp parallel for
                for (unsigned int k = 0; k < limit_k; ++k)
                {
                        OMPTaskLoopBody3D t(m_pTbbNDRangeTask, &result);
                        t(j, i, k);
                }
            }
         }
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

