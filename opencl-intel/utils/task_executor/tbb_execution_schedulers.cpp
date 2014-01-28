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

#include <opencl_partitioner.h>
#include <vector>

#include "tbb_blocked_ranges.h"
#include "tbb_execution_schedulers.h"
#include "arena_handler.h"
#include "tbb_executor.h"
#include "cl_shared_ptr.hpp"

using namespace Intel::OpenCL::TaskExecutor;

struct TaskLoopBody
{
protected:
    const Intel::OpenCL::Utils::SharedPtr<ITaskSet>&  m_task;

    TaskLoopBody(
        const Intel::OpenCL::Utils::SharedPtr<ITaskSet>&  task
        ) : m_task(task) { }
};

template <class blocked_range_3d>
struct TaskLoopBody3D : public TaskLoopBody {
    TaskLoopBody3D(
        const Intel::OpenCL::Utils::SharedPtr<ITaskSet>&  task
        ) : TaskLoopBody(task) {}
    void operator()(const blocked_range_3d& r) const {
        size_t uiNumberOfWorkGroups;

        size_t firstWGID[3] = {r.cols().begin(), r.rows().begin(), r.pages().begin()};
        size_t lastWGID[3] = {r.cols().end(), r.rows().end(), r.pages().end()};

        uiNumberOfWorkGroups = (r.pages().size())*(r.rows().size())*(r.cols().size());
        assert(uiNumberOfWorkGroups <= CL_MAX_INT32);

        TBB_PerActiveThreadData* tls = TBB_ThreadManager<TBB_PerActiveThreadData>::GetCurrentThreadDescriptor();
        if (NULL == tls)
        {
            assert( (NULL != tls) && "Task executes after thread disconnected from TEDevice or thread is connected after TEDevice shutdown" );
            return;
        }

        void* user_local = m_task->AttachToThread(tls->user_tls, uiNumberOfWorkGroups, firstWGID, lastWGID);
        if ( NULL == user_local )
        {
            assert( 0 && "Thread local execution environment is NULL" );
            return;
        }

        for(size_t i = r.pages().begin(), e = r.pages().end(); i < e; i++ )
        {
            for(size_t j = r.rows().begin(), d = r.rows().end(); j < d; j++ )
            {
                for(size_t k = r.cols().begin(), f = r.cols().end(); k < f; k++ )
                {
                    // OpenCL defines dims as (col, row, page)
                    if (!m_task->ExecuteIteration(k, j, i, user_local))
                    {
                        assert( 0 && "Failed to execute iteration" );
                        goto error_exit;
                    }
                }
            }
        }
        error_exit:
        m_task->DetachFromThread(user_local);
        return;
    }
};

template <class blocked_range_2d>
struct TaskLoopBody2D : public TaskLoopBody {
    TaskLoopBody2D(
        const Intel::OpenCL::Utils::SharedPtr<ITaskSet>&  task
        ) : TaskLoopBody(task) {}

    void operator()(const blocked_range_2d& r) const {
        size_t uiNumberOfWorkGroups;
        size_t firstWGID[2] = {r.cols().begin(), r.rows().begin()};
        size_t lastWGID[2] = {r.cols().end(), r.rows().end()};

        uiNumberOfWorkGroups = (r.rows().size())*(r.cols().size());
        assert(uiNumberOfWorkGroups <= CL_MAX_INT32);

        TBB_PerActiveThreadData* tls = TBB_ThreadManager<TBB_PerActiveThreadData>::GetCurrentThreadDescriptor();
        if (NULL == tls)
        {
            assert( (NULL != tls) && "Task executes after thread disconnected from TEDevice or thread is connected after TEDevice shutdown" );
            return;
        }

        void* user_local = m_task->AttachToThread(tls->user_tls, uiNumberOfWorkGroups, firstWGID, lastWGID);
        if ( NULL == user_local )
        {
            assert( 0 && "Thread local execution environment is NULL" );
            return;
        }

        for(size_t j = r.rows().begin(), d = r.rows().end(); j < d; j++ )
        {
            for(size_t k = r.cols().begin(), f = r.cols().end(); k < f; k++ )
            {
                // OpenCL defines dims as (col, row, page)
                if (!m_task->ExecuteIteration(k, j, 0, user_local))
                {
                    assert( 0 && "Failed to execute iteration" );
                    goto error_exit;
                }
            }
        }
		
        error_exit:
        m_task->DetachFromThread(user_local);
        return;
    }
};    

template <class blocked_range_1d>
struct TaskLoopBody1D : public TaskLoopBody {
    TaskLoopBody1D(
        const Intel::OpenCL::Utils::SharedPtr<ITaskSet>&  task
        ) : TaskLoopBody(task) {}

    void operator()(const blocked_range_1d& r) const {
        size_t uiNumberOfWorkGroups;
        size_t firstWGID[1] = {r.begin()}; 
        size_t lastWGID[1] = {r.end()}; 

        uiNumberOfWorkGroups = r.size();
        assert(uiNumberOfWorkGroups <= CL_MAX_INT32);
        
        TBB_PerActiveThreadData* tls = TBB_ThreadManager<TBB_PerActiveThreadData>::GetCurrentThreadDescriptor();
        assert( (NULL != tls) && "Task executes after thread disconnected from TEDevice or thread is connected after TEDevice shutdown" );
        if (NULL == tls)
        {
            return;
        }

        void* user_local = m_task->AttachToThread(tls->user_tls, uiNumberOfWorkGroups, firstWGID, lastWGID);
        if ( NULL == user_local )
        {
            assert( 0 && "Thread local execution environment is NULL" );
            return;
        }

        for(size_t k = r.begin(), f = r.end(); k < f; k++ )
        {
            // OpenCL defines dims as (col, row, page)
            if (!m_task->ExecuteIteration(k, 0, 0, user_local))
            {
                assert( 0 && "Failed to execute iteration" );
                break;
            }
        }

        m_task->DetachFromThread(user_local);
    }
};

template <class BlockedRange, class TaskLoopBodySpecific>
void TBB_ExecutionSchedulers::opencl_executor(
    const size_t                                      dims[],
    size_t                                            grainsize,
    const Intel::OpenCL::Utils::SharedPtr<ITaskSet>&  task,
    base_command_list&                                cmdList )
{
    //int nThreads = executor.GetCurrentDevice().teDevice->GetConcurrency();
    tbb::task_group_context& context = cmdList.GetTBBContext();

#ifdef MIC_COMMAND_BATCHING_OPTIMIZATION
    tbb::uneven::parallel_for(BlockedRange(dims, grainsize), TaskLoopBodySpecific(task), tbb::opencl_partitioner(tbb::task_arena::current_slot(), cmdList.GetTEDevice()->GetConcurrency() - 1, false), context);
#else
    tbb::uneven::parallel_for(BlockedRange(dims, grainsize), TaskLoopBodySpecific(task), tbb::opencl_partitioner(), context);
#endif
}

#ifdef __MIC__
// TODO: Fall back to auto-partitioner, because of the outliers when number of workgoups is bellow concurrency level
//       until https://bugzilla.inn.intel.com/SSG/bugzilla/show_bug.cgi?id=2002 is resolved.
// Please don't remove this code
#if 0
template <class blocked_range_1d>
struct TaskLoopBody2D_1D : public TaskLoopBody {
    TaskLoopBody2D_1D(
        const Intel::OpenCL::Utils::SharedPtr<ITaskSet>&  task,
        const size_t sizeX
        ) : TaskLoopBody(task), m_sizeX(sizeX) {}

    void operator()(const blocked_range_1d& r) const {
        size_t uiNumberOfWorkGroups;
        size_t startX = r.begin() % m_sizeX;
        size_t startY = r.begin() / m_sizeX;
        size_t endX = r.end() % m_sizeX;
        size_t endY = r.end() / m_sizeX;
        size_t firstWGID[2] = {startX, startY};
        size_t lastWGID[2] = {endX, endY};

        uiNumberOfWorkGroups = r.size();
        assert(uiNumberOfWorkGroups <= CL_MAX_INT32);

        TBB_PerActiveThreadData* tls = TBB_ThreadManager<TBB_PerActiveThreadData>::GetCurrentThreadDescriptor();
        assert( (NULL != tls) && "Task executes after thread disconnected from TEDevice or thread is connected after TEDevice shutdown" );
        if (NULL == tls)
        {
            return;
        }

        void* user_local = m_task->AttachToThread(tls->user_tls, uiNumberOfWorkGroups, firstWGID, lastWGID);
        if ( NULL == user_local )
        {
            assert( 0 && "Thread local execution environment is NULL" );
            return;
        }

        for(size_t k = r.begin(), f = r.end(); k < f; k++ )
        {
            size_t x = k / m_sizeX;
            size_t y = k % m_sizeX;
            // OpenCL defines dims as (col, row, page)
            if (!m_task->ExecuteIteration(x, y, 0, user_local))
            {
                assert( 0 && "Failed to execute iteration" );
                break;
            }
        }

        m_task->DetachFromThread(user_local);
    }
protected:
    const size_t m_sizeX;
};

// specialization for 2D case
template <>
void TBB_ExecutionSchedulers::opencl_executor<BlockedRangeByUnevenTBB2d, TaskLoopBody2D<BlockedRangeByUnevenTBB2d> >(
    const size_t                                      dims[],
    size_t                                            grainsize,
    const Intel::OpenCL::Utils::SharedPtr<ITaskSet>&  task,
    base_command_list&                                cmdList )
{
    TBB_PerActiveThreadData* tls = TBB_ThreadManager<TBB_PerActiveThreadData>::GetCurrentThreadDescriptor();

    unsigned int nThreads = (unsigned int)tls->device->GetConcurrency();

    tbb::task_group_context& context = cmdList.GetTBBContext();

    size_t total_size = dims[0]*dims[1];
    if ( total_size < nThreads )
    {
        tbb::uneven::parallel_for(BlockedRangeByUnevenTBB1d(dims, grainsize), TaskLoopBody2D_1D<BlockedRangeByUnevenTBB1d>(task, dims[0]), tbb::opencl_partitioner(), context);
    }
    else
    {
        tbb::uneven::parallel_for(BlockedRangeByUnevenTBB2d(dims, grainsize), TaskLoopBody2D<BlockedRangeByUnevenTBB2d>(task), tbb::opencl_partitioner(), context);
    }
}
#endif

#endif

template <class BlockedRange, class TaskLoopBodySpecific>        
void TBB_ExecutionSchedulers::auto_executor(
    const size_t                                      dims[],
    size_t                                            grainsize,
    const Intel::OpenCL::Utils::SharedPtr<ITaskSet>&  task,
    base_command_list&                                cmdList )
{
    tbb::parallel_for(BlockedRange(dims, grainsize), TaskLoopBodySpecific(task), tbb::auto_partitioner());
}

template <class BlockedRange, class TaskLoopBodySpecific>        
void TBB_ExecutionSchedulers::affinity_executor(
    const size_t                                      dims[],
    size_t                                            grainsize,
    const Intel::OpenCL::Utils::SharedPtr<ITaskSet>&  task,
    base_command_list&                                cmdList )
{
    tbb::affinity_partitioner& ap = cmdList.GetAffinityPartitioner();
    tbb::parallel_for(BlockedRange(dims, grainsize), TaskLoopBodySpecific(task), ap);
}

#define DEFINE_EXECUTOR_DIMS_ARRAY( name, ExecutorFuncName, BlockedRangeNamePrefix )                  \
    TBB_ExecutionSchedulers::ExecutorFunc TBB_ExecutionSchedulers::name[MAX_WORK_DIM] =               \
    {                                                                                                 \
        &TBB_ExecutionSchedulers::ExecutorFuncName < BlockedRangeNamePrefix ## 1d,                    \
                                                     TaskLoopBody1D< BlockedRangeNamePrefix ## 1d > >,\
        &TBB_ExecutionSchedulers::ExecutorFuncName < BlockedRangeNamePrefix ## 2d,                    \
                                                     TaskLoopBody2D< BlockedRangeNamePrefix ## 2d > >,\
        &TBB_ExecutionSchedulers::ExecutorFuncName < BlockedRangeNamePrefix ## 3d,                    \
                                                     TaskLoopBody3D< BlockedRangeNamePrefix ## 3d > > \
    }

                        /* 1D,2D,3D array name */  /* use function */ /* use blocked range class */
DEFINE_EXECUTOR_DIMS_ARRAY( auto_block_default,     auto_executor,      BlockedRangeByDefaultTBB );
DEFINE_EXECUTOR_DIMS_ARRAY( affinity_block_default, affinity_executor,  BlockedRangeByDefaultTBB );
DEFINE_EXECUTOR_DIMS_ARRAY( opencl_block_default,   opencl_executor,    BlockedRangeByUnevenTBB );

DEFINE_EXECUTOR_DIMS_ARRAY( auto_block_row,         auto_executor,      BlockedRangeByRow );
DEFINE_EXECUTOR_DIMS_ARRAY( affinity_block_row,     affinity_executor,  BlockedRangeByRow );
DEFINE_EXECUTOR_DIMS_ARRAY( opencl_block_row,       opencl_executor,    BlockedRangeByUnevenTBB );

DEFINE_EXECUTOR_DIMS_ARRAY( auto_block_column,      auto_executor,      BlockedRangeByColumn );   
DEFINE_EXECUTOR_DIMS_ARRAY( affinity_block_column,  affinity_executor,  BlockedRangeByColumn );
DEFINE_EXECUTOR_DIMS_ARRAY( opencl_block_column,    opencl_executor,    BlockedRangeByUnevenTBB );

DEFINE_EXECUTOR_DIMS_ARRAY( auto_block_tile,        auto_executor,      BlockedRangeByTile );
DEFINE_EXECUTOR_DIMS_ARRAY( affinity_block_tile,    affinity_executor,  BlockedRangeByTile );
DEFINE_EXECUTOR_DIMS_ARRAY( opencl_block_tile,      opencl_executor,    BlockedRangeByUnevenTBB );

TBB_ExecutionSchedulers::ExecutorFunc*  
TBB_ExecutionSchedulers::g_executor[TE_CMD_LIST_PREFERRED_SCHEDULING_LAST][TASK_SET_OPTIMIZE_BY_LAST] = 
{
                  /* default by TBB tile */   /* by_row  */         /* by_column */         /* by_tile */
/* auto     */{   auto_block_default,         auto_block_row,       auto_block_column,      auto_block_tile         },
/* affinity */{   affinity_block_default,     affinity_block_row,   affinity_block_column,  affinity_block_tile     },
/* opencl   */{   opencl_block_default,       opencl_block_row,     opencl_block_column,    opencl_block_tile       }
};

/////////////////////////////////////////////////////////////////////////////////////
//
// Get appropriate executor and run it.
//
/////////////////////////////////////////////////////////////////////////////////////
bool TBB_ExecutionSchedulers::parallel_execute( base_command_list& cmdList,
                                                const Intel::OpenCL::Utils::SharedPtr<ITaskSet>&  task)
{
    size_t dims[MAX_WORK_DIM];
    unsigned int dimCount;

    TE_CMD_LIST_PREFERRED_SCHEDULING scheduler    = cmdList.GetPreferredScheduler();
    TASK_SET_OPTIMIZATION            optimization = task->OptimizeBy();

    int res = task->Init(dims, dimCount);

    assert( scheduler < TE_CMD_LIST_PREFERRED_SCHEDULING_LAST && "Invalid preferred scheduling" );
    assert( optimization < TASK_SET_OPTIMIZE_BY_LAST && "Invalid ");
    assert( (dimCount > 0) && (dimCount <= 3) && "Invalid work dimensions");

    if ((0 != res) ||
        (scheduler >= TE_CMD_LIST_PREFERRED_SCHEDULING_LAST) ||
        (optimization >= TASK_SET_OPTIMIZE_BY_LAST)          ||
        ((dimCount == 0) || (dimCount > 3)))
    {
        task->Finish(FINISH_INIT_FAILED);
        return false;
    }

    unsigned int grainSize    = task->PreferredSequentialItemsPerThread();

    ExecutorFunc execute = g_executor[scheduler][optimization][dimCount-1];

    execute( dims, grainSize, task, cmdList  );

    return task->Finish(FINISH_COMPLETED);
}
