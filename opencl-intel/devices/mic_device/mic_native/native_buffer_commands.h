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
//  ExecutionTask.h
//  Implementation of the Class ExecutionTask
//  Class Object is responsible on execution of NDRange task
/////////////////////////////////////////////////////////////

#pragma once
#include "task_executor.h"
#include "device_queue.h"
#include "mic_device_interface.h"
#include "mic_tracer.h"

namespace Intel { namespace OpenCL { namespace MICDeviceNative {

extern Intel::OpenCL::MICDevice::mic_exec_env_options gMicExecEnvOptions;

class FillMemObjTask : virtual public Intel::OpenCL::TaskExecutor::ITaskSet, virtual public TaskHandler<FillMemObjTask, Intel::OpenCL::MICDevice::fill_mem_obj_dispatcher_data >
{
public:
    PREPARE_SHARED_PTR(FillMemObjTask)

    FillMemObjTask( uint32_t lockBufferCount, void** pLockBuffers, Intel::OpenCL::MICDevice::fill_mem_obj_dispatcher_data* pDispatcherData, size_t uiDispatchSize );
    
    // TaskHandler methods
    bool PrepareTask();

    // TaskHandler methods
    const FillMemObjTask& GetAsCommandTypeConst() const { return *this; }
    Intel::OpenCL::TaskExecutor::ITaskBase* GetAsITaskBase() { return static_cast<Intel::OpenCL::TaskExecutor::ITaskBase*>(this);}

    // ITask methods 
    // Returns true in case current task is a synchronization point
    // No more tasks will be executed in this case
    bool    CompleteAndCheckSyncPoint() { return false; }
    
    // Set current command as synchronization point
    // Returns true if command is already completed
    bool    SetAsSyncPoint() { return false; }

    // Returns true if command is already completed
    bool    IsCompleted() const { return false; }

    // Task execution routine, will be called by task executor instead of Execute() if CommandList is canceled
    void    Cancel();    

    // Releases task object, shall be called instead of delete operator.
    long    Release() { delete this; return 0; }

    Intel::OpenCL::TaskExecutor::TASK_PRIORITY   GetPriority() const { return Intel::OpenCL::TaskExecutor::TASK_PRIORITY_MEDIUM;}

    Intel::OpenCL::TaskExecutor::ITaskGroup* GetNDRangeChildrenTaskGroup() { return NULL;}
	
	// Initialization function. This functions is called before the "main loop"
    // Generally initializes internal data structures
    // Fills the buffer with 3D number of iterations to run
    // Fills regCount with actual number of regions
    // Returns 0 if initialization success, otherwise an error code
    int Init(size_t region[], unsigned int& regCount);

    // Is called when the task is going to be called for the first time within specific thread. 
    // @param currentThreadData - data returned by OnThreadEntry()
    // Returns data to be passed to ExecuteIteration methods, if attach process succeeded, otherwise NULL to abort
	void* AttachToThread(void* currentThreadData, size_t uiNumberOfWorkGroups, size_t firstWGID[], size_t lastWGID[]) { return this; }

    // Is called when the task will not be executed by the specific thread    
    // Receives data returned by AttachToThread.
	void DetachFromThread(void* data_from_AttachToThread) {};

    // "Main loop"
    // The function is called with different 'inx' parameters for each iteration number
    // Return false to abort 
    bool ExecuteIteration(size_t x, size_t y, size_t z, void* data_from_AttachToThread);

    // Final stage, free execution resources
    // Return false when command execution fails
    bool Finish(Intel::OpenCL::TaskExecutor::FINISH_REASON reason);

    // Task execution routine, will be called by task executor instead of Init() if CommandList is canceled. If Init() was already called,
    // Cancel() is not called - normal processing is continued
    // virtual void    Cancel() = 0;

    // Optimize By
    Intel::OpenCL::TaskExecutor::TASK_SET_OPTIMIZATION OptimizeBy() const { return gMicExecEnvOptions.tbb_block_optimization; }
    unsigned int PreferredSequentialItemsPerThread() const { return gMicExecEnvOptions.use_TBB_grain_size; }

    typedef Intel::OpenCL::MICDevice::fill_mem_obj_dispatcher_data dispatcher_data_type;
	
protected:
    friend class TaskHandler<FillMemObjTask, Intel::OpenCL::MICDevice::fill_mem_obj_dispatcher_data >;
    // Copy constructor used for task duplication
    FillMemObjTask(const FillMemObjTask& o);

private:
    // The Buffer to fill pointer
    char*                            m_fillBufPtr;
    char*                            m_fillBufPtrAnchor;

    double                           m_patternToUse[MAX_PATTERN_SIZE / sizeof(double)] __attribute__((aligned(64)));
    size_t                           m_coveredSize;
    size_t                           m_numIterationsPerWorker;
    bool                             m_serialExecution;

	struct tasksForWorkerConfStruct
	{
		size_t buffSize;
		unsigned int numTasks;
	};

	const static tasksForWorkerConfStruct m_tasksForWorkerConf[];
        
    inline int intrinCopy(int from, int to)
    {
        __m512d paternIntrin1 = _mm512_load_pd(m_patternToUse);
        __m512d paternIntrin2 = _mm512_load_pd(m_patternToUse + 8);
        const size_t tHalfStepSize = sizeof(double) * 8;
        assert(tHalfStepSize == (MAX_PATTERN_SIZE / 2));
        int fromIndex = from;
        const int toIndex = to - MAX_PATTERN_SIZE;
        for (; fromIndex <= toIndex; fromIndex += MAX_PATTERN_SIZE)
        {
            _mm512_storenr_pd(m_fillBufPtr + fromIndex, paternIntrin1);
            _mm512_storenr_pd(m_fillBufPtr + (fromIndex + tHalfStepSize), paternIntrin2);
        }
        return (fromIndex - from);
    }
};

}}}
