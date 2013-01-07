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

#include "tbb_execution_schedulers.h"
#include "arena_handler.h"
#include "tbb_blocked_ranges.h"
#include <vector>

using namespace Intel::OpenCL::TaskExecutor;

struct TaskLoopBody
{
protected:

    ArenaHandler& m_devArenaHandler;

    TaskLoopBody(ArenaHandler& devArenaHandler) : m_devArenaHandler(devArenaHandler) { }
};

template <class blocked_range_3d>
struct TaskLoopBody3D : public TaskLoopBody {
    ITaskSet &task;
    TaskLoopBody3D(ArenaHandler& devArenaHandler, ITaskSet &t) : TaskLoopBody(devArenaHandler), task(t) {}
    void operator()(const blocked_range_3d& r) const {
        size_t uiNumberOfWorkGroups;

        size_t firstWGID[3] = {r.pages().begin(), r.rows().begin(),r.cols().begin()}; 
        size_t lastWGID[3] = {r.pages().end(),r.rows().end(),r.cols().end()}; 

        uiNumberOfWorkGroups = (r.pages().size())*(r.rows().size())*(r.cols().size());
        assert(uiNumberOfWorkGroups <= CL_MAX_INT32);

        WGContextBase* const pWGContext = m_devArenaHandler.GetWGContext();
        if ( task.AttachToThread(pWGContext, uiNumberOfWorkGroups, firstWGID, lastWGID) != 0 )
            return;

        for(size_t i = r.pages().begin(), e = r.pages().end(); i < e; i++ )
            for(size_t j = r.rows().begin(), d = r.rows().end(); j < d; j++ )
                for(size_t k = r.cols().begin(), f = r.cols().end(); k < f; k++ )
                {
                    // OpenCL defines dims as (col, row, page)
                    task.ExecuteIteration(k, j, i, pWGContext);
                }
        task.DetachFromThread(pWGContext);
    }
};

template <class blocked_range_2d>
struct TaskLoopBody2D : public TaskLoopBody {
    ITaskSet &task;
    TaskLoopBody2D(ArenaHandler& devArenaHandler, ITaskSet &t) : TaskLoopBody(devArenaHandler), task(t) {}
    void operator()(const blocked_range_2d& r) const {
        size_t uiNumberOfWorkGroups;
        size_t firstWGID[2] = {r.rows().begin(),r.cols().begin()}; 
        size_t lastWGID[2] = {r.rows().end(),r.cols().end()}; 

        uiNumberOfWorkGroups = (r.rows().size())*(r.cols().size());
        assert(uiNumberOfWorkGroups <= CL_MAX_INT32);

        WGContextBase* const pWGContext = m_devArenaHandler.GetWGContext();
        if ( task.AttachToThread(pWGContext, uiNumberOfWorkGroups, firstWGID, lastWGID) != 0 )
            return;
        for(size_t j = r.rows().begin(), d = r.rows().end(); j < d; j++ )
            for(size_t k = r.cols().begin(), f = r.cols().end(); k < f; k++ )
            {
                // OpenCL defines dims as (col, row, page)
                task.ExecuteIteration(k, j, 0, pWGContext);
            }
        task.DetachFromThread(pWGContext);
    }
};    

template <class blocked_range_1d>
struct TaskLoopBody1D : public TaskLoopBody {
    ITaskSet &task;
    TaskLoopBody1D(ArenaHandler& devArenaHandler, ITaskSet &t) : TaskLoopBody(devArenaHandler), task(t) {}
    void operator()(const blocked_range_1d& r) const {
        size_t uiNumberOfWorkGroups;
        size_t firstWGID[1] = {r.begin()}; 
        size_t lastWGID[1] = {r.end()}; 

        uiNumberOfWorkGroups = r.size();
        assert(uiNumberOfWorkGroups <= CL_MAX_INT32);
        
        WGContextBase* const pWGContext = m_devArenaHandler.GetWGContext();
        if ( task.AttachToThread(pWGContext, uiNumberOfWorkGroups, firstWGID, lastWGID) != 0 )
            return;

        for(size_t k = r.begin(), f = r.end(); k < f; k++ )
        {
            // OpenCL defines dims as (col, row, page)
            task.ExecuteIteration(k, 0, 0, pWGContext);
        }
        task.DetachFromThread(pWGContext);
    }
};


template <class BlockedRange, class TaskLoopBodySpecific>        
void TBB_ExecutionSchedulers::auto_executor( const BlockedRangeBase& range, 
                                             ITaskSet&               task,
                                             ArenaHandler&           arena,
                                             tbb::affinity_partitioner* ap )
{
    tbb::parallel_for(BlockedRange(range), TaskLoopBodySpecific(arena,task), tbb::auto_partitioner());
}

template <class BlockedRange, class TaskLoopBodySpecific>        
void TBB_ExecutionSchedulers::affinity_executor( const BlockedRangeBase& range, 
                                                 ITaskSet&               task,
                                                 ArenaHandler&           arena,
                                                 tbb::affinity_partitioner* ap )
{
    tbb::parallel_for(BlockedRange(range), TaskLoopBodySpecific(arena,task), *ap);
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

DEFINE_EXECUTOR_DIMS_ARRAY( auto_block_row,         auto_executor,      BlockedRangeByRow );
DEFINE_EXECUTOR_DIMS_ARRAY( affinity_block_row,     affinity_executor,  BlockedRangeByRow );

DEFINE_EXECUTOR_DIMS_ARRAY( auto_block_column,      auto_executor,      BlockedRangeByColumn );   
DEFINE_EXECUTOR_DIMS_ARRAY( affinity_block_column,  affinity_executor,  BlockedRangeByColumn );

DEFINE_EXECUTOR_DIMS_ARRAY( auto_block_tile,        auto_executor,      BlockedRangeByTile );
DEFINE_EXECUTOR_DIMS_ARRAY( affinity_block_tile,    affinity_executor,  BlockedRangeByTile );

TBB_ExecutionSchedulers::ExecutorFunc*  
TBB_ExecutionSchedulers::g_executor[TE_CMD_LIST_PREFERRED_SCHEDULING_LAST][TASK_SET_OPTIMIZE_BY_LAST] = 
{
                  /* default by TBB tile */   /* by_row  */         /* by_column */         /* by_tile */
/* auto     */{   auto_block_default,         auto_block_row,       auto_block_column,      auto_block_tile         },
/* affinity */{   affinity_block_default,     affinity_block_row,   affinity_block_column,  affinity_block_tile     }
};

/////////////////////////////////////////////////////////////////////////////////////
//
// Get appropriate executor and run it.
//
/////////////////////////////////////////////////////////////////////////////////////
void TBB_ExecutionSchedulers::parallel_execute( base_command_list& cmdList,
                                                const size_t       dims[],          // MAX_WORK_DIM size
                                                unsigned int       actual_dims_size,
                                                ITaskSet&          task )
{
    TE_CMD_LIST_PREFERRED_SCHEDULING scheduler    = cmdList.GetPreferredScheduler();
    TASK_SET_OPTIMIZATION            optimization = task.OptimizeBy();
    
    assert( scheduler < TE_CMD_LIST_PREFERRED_SCHEDULING_LAST );
    assert( optimization < TASK_SET_OPTIMIZE_BY_LAST );
    assert( (actual_dims_size > 0) && (actual_dims_size <= 3) );

    if ((scheduler >= TE_CMD_LIST_PREFERRED_SCHEDULING_LAST) ||
        (optimization >= TASK_SET_OPTIMIZE_BY_LAST)          ||
        ((actual_dims_size == 0) || (actual_dims_size > 3)))
    {
        return;
    }

    tbb::affinity_partitioner*       ap           = cmdList.GetAffinityPartitioner();
    unsigned int                     grainSize    = task.PreferredSequentialItemsPerThread();
    ArenaHandler&                    arenaHandler = cmdList.GetDevArenaHandler();

    ExecutorFunc execute = g_executor[scheduler][optimization][actual_dims_size-1];
    size_t actual_dims[MAX_WORK_DIM] = {dims[0], dims[1], dims[2]};
  
    switch (actual_dims_size)
    {
        case 1:
            actual_dims[1] = 1;
            // pass through

        case 2:
            actual_dims[2] = 1;
            // pass through
            
        case 3:
        default:
            ;
    }

    assert(actual_dims[0] <= CL_MAX_INT32);
    assert(actual_dims[1] <= CL_MAX_INT32);
    assert(actual_dims[2] <= CL_MAX_INT32);

    execute( BlockedRangeBase( actual_dims, grainSize ), task, arenaHandler, ap  );

}


