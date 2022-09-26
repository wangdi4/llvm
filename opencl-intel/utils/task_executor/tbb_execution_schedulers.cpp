// INTEL CONFIDENTIAL
//
// Copyright 2006-2020 Intel Corporation.
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

#include "tbb_execution_schedulers.h"

#include "arena_handler.h"
#include "cl_shared_ptr.hpp"
#include "tbb_blocked_ranges.h"
#include "tbb_executor.h"

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
        if (nullptr == tls)
        {
            assert( (nullptr != tls) && "Task executes after thread disconnected from TEDevice or thread is connected after TEDevice shutdown" );
            return;
        }

        void* user_local = m_task->AttachToThread(tls->user_tls, uiNumberOfWorkGroups, firstWGID, lastWGID);
        if ( nullptr == user_local )
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
        if (nullptr == tls)
        {
            assert( (nullptr != tls) && "Task executes after thread disconnected from TEDevice or thread is connected after TEDevice shutdown" );
            return;
        }

        void* user_local = m_task->AttachToThread(tls->user_tls, uiNumberOfWorkGroups, firstWGID, lastWGID);
        if ( nullptr == user_local )
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
        assert( (nullptr != tls) && "Task executes after thread disconnected from TEDevice or thread is connected after TEDevice shutdown" );
        if (nullptr == tls)
        {
            return;
        }

        void* user_local = m_task->AttachToThread(tls->user_tls, uiNumberOfWorkGroups, firstWGID, lastWGID);
        if ( nullptr == user_local )
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
        assert( (nullptr != tls) && "Task executes after thread disconnected from TEDevice or thread is connected after TEDevice shutdown" );
        if (nullptr == tls)
        {
            return;
        }

        void* user_local = m_task->AttachToThread(tls->user_tls, uiNumberOfWorkGroups, firstWGID, lastWGID);
        if ( nullptr == user_local )
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

#endif

#endif

template <class BlockedRange, class TaskLoopBodySpecific>
void TBB_ExecutionSchedulers::auto_executor(
    const size_t dimsBegin[], const size_t dimsEnd[], size_t grainsize,
    const Intel::OpenCL::Utils::SharedPtr<ITaskSet> &task,
    base_command_list & /*cmdList*/) {
  tbb::parallel_for(BlockedRange(dimsBegin, dimsEnd, grainsize),
                    TaskLoopBodySpecific(task), tbb::auto_partitioner());
}

template <class BlockedRange, class TaskLoopBodySpecific>        
void TBB_ExecutionSchedulers::affinity_executor(
    const size_t                                      dimsBegin[],
    const size_t                                      dimsEnd[],
    size_t                                            grainsize,
    const Intel::OpenCL::Utils::SharedPtr<ITaskSet>&  task,
    base_command_list&                                cmdList )
{
    tbb::affinity_partitioner& ap = cmdList.GetAffinityPartitioner();
    tbb::parallel_for(BlockedRange(dimsBegin, dimsEnd, grainsize),
                      TaskLoopBodySpecific(task), ap);
}

template <class BlockedRange, class TaskLoopBodySpecific>
void TBB_ExecutionSchedulers::static_executor(
    const size_t                                      dimsBegin[],
    const size_t                                      dimsEnd[],
    size_t                                            grainsize,
    const Intel::OpenCL::Utils::SharedPtr<ITaskSet>&  task,
    base_command_list&                                cmdList )
{
    tbb::static_partitioner& sp = cmdList.GetStaticPartitioner();
    tbb::parallel_for(BlockedRange(dimsBegin, dimsEnd, grainsize),
                      TaskLoopBodySpecific(task), sp);
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

#define DEFINE_EXECUTOR_DIMS_ARRAY_SPLIT(name, ExecutorFuncName,               \
                                         BlockedRangeNamePrefix, SplitTy)      \
  TBB_ExecutionSchedulers::ExecutorFunc                                        \
      TBB_ExecutionSchedulers::name[MAX_WORK_DIM] = {                          \
          &TBB_ExecutionSchedulers::ExecutorFuncName<                          \
              BlockedRangeNamePrefix##1d < SplitTy>,                           \
          TaskLoopBody1D<BlockedRangeNamePrefix##1d < SplitTy> >>              \
          ,                                                                    \
          &TBB_ExecutionSchedulers::ExecutorFuncName<                          \
              BlockedRangeNamePrefix##2d < SplitTy>,                           \
          TaskLoopBody2D<BlockedRangeNamePrefix##2d < SplitTy> >>              \
          ,                                                                    \
          &TBB_ExecutionSchedulers::ExecutorFuncName<                          \
              BlockedRangeNamePrefix##3d < SplitTy>,                           \
          TaskLoopBody3D<BlockedRangeNamePrefix##3d < SplitTy> >> }

/* 1D,2D,3D array name */ /* use function */ /* use blocked range class */
DEFINE_EXECUTOR_DIMS_ARRAY_SPLIT(auto_block_default, auto_executor,
                                 BlockedRangeByDefaultTBB, );
DEFINE_EXECUTOR_DIMS_ARRAY_SPLIT(affinity_block_default, affinity_executor,
                                 BlockedRangeByDefaultTBB,
                                 HasProportionalSplit);
DEFINE_EXECUTOR_DIMS_ARRAY_SPLIT(static_block_default, static_executor,
                                 BlockedRangeByDefaultTBB,
                                 HasProportionalSplit);

DEFINE_EXECUTOR_DIMS_ARRAY( auto_block_row,         auto_executor,      BlockedRangeByRow );
DEFINE_EXECUTOR_DIMS_ARRAY( affinity_block_row,     affinity_executor,  BlockedRangeByRow );
DEFINE_EXECUTOR_DIMS_ARRAY( static_block_row,       static_executor,    BlockedRangeByRow );

DEFINE_EXECUTOR_DIMS_ARRAY( auto_block_column,      auto_executor,      BlockedRangeByColumn );
DEFINE_EXECUTOR_DIMS_ARRAY( affinity_block_column,  affinity_executor,  BlockedRangeByColumn );
DEFINE_EXECUTOR_DIMS_ARRAY( static_block_column,    static_executor,    BlockedRangeByColumn );

DEFINE_EXECUTOR_DIMS_ARRAY( auto_block_tile,        auto_executor,      BlockedRangeByTile );
DEFINE_EXECUTOR_DIMS_ARRAY( affinity_block_tile,    affinity_executor,  BlockedRangeByTile );
DEFINE_EXECUTOR_DIMS_ARRAY( static_block_tile,      static_executor,    BlockedRangeByTile );

TBB_ExecutionSchedulers::ExecutorFunc*
TBB_ExecutionSchedulers::g_executor[TE_CMD_LIST_PREFERRED_SCHEDULING_LAST][TASK_SET_OPTIMIZE_BY_LAST] =
{
                  /* default by TBB tile */   /* by_row  */         /* by_column */         /* by_tile */
/* auto     */{   auto_block_default,         auto_block_row,       auto_block_column,      auto_block_tile         },
/* affinity */{   affinity_block_default,     affinity_block_row,   affinity_block_column,  affinity_block_tile     },
/* static   */{   static_block_default,       static_block_row,     static_block_column,    static_block_tile       }
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

    TEDevice* rootDevice = cmdList.GetTEDevice();
    int res = task->Init(dims, dimCount, rootDevice->GetConcurrency());

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

    size_t grainSize = task->PreferredSequentialItemsPerThread();

    ExecutorFunc execute = g_executor[scheduler][optimization][dimCount-1];

    const TBBTaskExecutor& taskExecutor = rootDevice->GetTaskExecutor();
    bool useNuma = false;
    if (taskExecutor.IsTBBNumaEnabled() && task->PreferNumaNodes())
    {
        unsigned int nodesCount = taskExecutor.GetTBBNumaNodesCount();
        assert(nodesCount > 1 && "Need at least 2 NUMA nodes");
        unsigned int lastDimIdx = dimCount - 1;
        while (lastDimIdx > 0 && dims[lastDimIdx] == 1)
          lastDimIdx--;
        size_t lastDimSize = dims[lastDimIdx];
        if (lastDimSize > nodesCount)
        {
            useNuma = true;

            ArenaHandler *lowLevelArenas = rootDevice->GetLowLevelArenas();
            tbb::task_group *taskGroups = cmdList.GetNumaTaskGroups();
            std::vector<std::vector<size_t>>& dimsBegin =
                cmdList.GetNumaDimsBegin();
            std::vector<std::vector<size_t>>& dimsEnd =
                cmdList.GetNumaDimsEnd();
            // Uniformly distribute Range to low level arenas which are bound to
            // NUMA nodes.
            size_t interval = lastDimSize / nodesCount;
            for (unsigned i = 0; i < nodesCount; ++i)
            {
                std::fill(dimsBegin[i].begin(), dimsBegin[i].end(), 0);
                dimsBegin[i][lastDimIdx] = interval * i;
                MEMCPY_S(dimsEnd[i].data(), MAX_WORK_DIM * sizeof(size_t),
                         dims, sizeof(dims));
                dimsEnd[i][lastDimIdx] = (i == (nodesCount - 1)) ?
                    lastDimSize : (interval * i + interval);
            }
            for (unsigned i = 0; i < nodesCount; ++i)
            {
                lowLevelArenas[i].Execute([&, i]{
                    taskGroups[i].run([&, i](){
                        execute(dimsBegin[i].data(), dimsEnd[i].data(),
                                grainSize, task, cmdList);
                    });
                });
            }
            for (unsigned i = 0; i < nodesCount; ++i)
            {
                lowLevelArenas[i].Execute([&, i]{ taskGroups[i].wait(); });
            }
        }
    }

    if (!useNuma)
    {
        size_t dimsBegin[MAX_WORK_DIM] = { 0 };
        execute(dimsBegin, dims, grainSize, task, cmdList);
    }

    return task->Finish(FINISH_COMPLETED);
}
