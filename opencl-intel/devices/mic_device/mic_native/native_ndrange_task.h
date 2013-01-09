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
#include "device_queue.h"
#include "task_executor.h"
#include "cl_dev_backend_api.h"
#include "program_memory_manager.h"
#include "native_printf.h"
#include "mic_tbb_tracer.h"

namespace Intel { namespace OpenCL { namespace MICDeviceNative {

using namespace Intel::OpenCL::TaskExecutor;
using namespace Intel::OpenCL::DeviceBackend;

class NDRangeTask : virtual public ITaskSet, virtual public TaskHandler
{
public:
    PREPARE_SHARED_PTR(NDRangeTask)
    
    static inline SharedPtr<NDRangeTask> Allocate( const QueueOnDevice& queue ) { return new NDRangeTask( queue ); }

    // TaskHandler methods
    
    // called immediately after creation and after filling the COI-passed data
    bool InitTask();

    // must be called at the very end of the ITaskBase finish stage and 
    // must call to QueueOnDevice->FinishTask() at the very end of itself
    void FinishTask();

    // ITaskSet methods 
    
	// Initialization function. This functions is called before the "main loop"
	// Generally initializes internal data structures
	// Fills the buffer with 3D number of iterations to run
	// Fills regCount with actual number of regions
	// Returns 0 if initialization success, otherwise an error code
	int		Init(size_t region[], unsigned int& regCount);

	// Is called when the task is going to be called for the first time
	// within specific thread. 
	// Returns 0, if attach process succeeded, otherwise -1
	int	AttachToThread(WGContextBase* pWgContext, size_t uiNumberOfWorkGroups, size_t firstWGID[], size_t lastWGID[]);

	// Is called when the task will not be executed by the specific thread	
	// Returns 0, if detach process succeeded, otherwise -1
	int	DetachFromThread(WGContextBase* pWgContext);

	// "Main loop"
	// The function is called with different 'inx' parameters for each iteration number
	void	ExecuteIteration(size_t x, size_t y, size_t z, WGContextBase* pWgContext = NULL);

    void	ExecuteAllIterations(size_t* dims, WGContextBase* pWgContext = NULL) 
                                { assert( false && "ExecuteAllIterations Not implemented" ); }

    // Final stage, free execution resources
	// Return false when command execution fails
	bool	Finish(FINISH_REASON reason);

    // Releases task object, shall be called instead of delete operator.
    long    Release() { delete this; return 0; }

    // Optimize By
    TASK_SET_OPTIMIZATION OptimizeBy() const { return gMicExecEnvOptions.tbb_block_optimization; }
    unsigned int          PreferredSequentialItemsPerThread() const { return gMicExecEnvOptions.use_TBB_grain_size; }

    // overriden from ReferenceCountedObject
    std::string GetTypeName() const { return "NDRangeTask"; }

protected:
    NDRangeTask( const QueueOnDevice& queue );
    ~NDRangeTask();

private:
    
	// uniqueue identifier for this task (command)
	cl_dev_cmd_id m_commandIdentifier;

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

    NDRangePerfData m_tbb_perf_data;
    friend class NDRangePerfData;
};

}}}

