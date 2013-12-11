// Copyright (c) 2009 Intel Corporation
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

///////////////////////////////////////////////////////////
//  TaskDispatcher.h
//  Implementation of the Class TaskDispatcher
////////////////////////////////////////////////////////////

#pragma once

#include "cl_device_api.h"
#include "program_service.h"
#include "memory_allocator.h"
#include "handle_allocator.h"
#include "task_executor.h"
#include "dispatcher_commands.h"
#include "cpu_config.h"
#include "wg_context.h"
#include "wg_context_pool.h"
#include <cl_synch_objects.h>
#include <cl_thread.h>
#include <builtin_kernels.h>

//should be hash_map but cant compile #include <hash_map>
#include <map>
#include <list>

using namespace Intel::OpenCL::TaskExecutor;
using Intel::OpenCL::BuiltInKernels::OMPExecutorThread;
using Intel::OpenCL::Utils::ObjectPool;

namespace Intel { namespace OpenCL { namespace CPUDevice {

typedef struct _cl_dev_internal_subdevice_id
{
    //Arch. data
    cl_uint  num_compute_units;
    bool     is_by_names;
    cl_uint* legal_core_ids;

    //Task dispatcher for this sub-device
    Intel::OpenCL::Utils::AtomicCounter ref_count;
    volatile bool                       is_acquired;
    SharedPtr<ITEDevice>                pSubDevice;
} cl_dev_internal_subdevice_id;

class IAffinityChangeObserver
{
public:
    virtual void NotifyAffinity(threadid_t tid, unsigned int core_index) = 0;
};

class TaskDispatcher : public Intel::OpenCL::TaskExecutor::ITaskExecutorObserver
{
	friend class DispatcherCommand;
	friend class AffinitizeThreads;

public:
    TaskDispatcher(cl_int devId, IOCLFrameworkCallbacks *pDevCallbacks,
      ProgramService	*programService, MemoryAllocator *memAlloc,
      IOCLDevLogDescriptor *logDesc, CPUDeviceConfig *cpuDeviceConfig, IAffinityChangeObserver* pObsserver);
    virtual ~TaskDispatcher();

    virtual cl_dev_err_code init();

    virtual cl_dev_err_code createCommandList( cl_dev_cmd_list_props IN props, ITEDevice* IN pDevice, SharedPtr<ITaskList>* OUT list);
    virtual cl_dev_err_code commandListExecute( const SharedPtr<ITaskList>& IN list, cl_dev_cmd_desc* IN *cmds, cl_uint IN count);

    virtual ProgramService* getProgramService(){ return m_pProgramService; }

    virtual bool            isDestributedAllocationRequired();
    virtual bool            isThreadAffinityRequired();

    bool                    isPredictablePartitioningAllowed()
    {
        cl_dev_internal_subdevice_id* pSubDevID = reinterpret_cast<cl_dev_internal_subdevice_id*>(m_pTaskExecutor->GetCurrentDevice().user_handle);
        return ( (NULL!=pSubDevID) && pSubDevID->is_by_names );
    }

    void                    waitUntilEmpty(ITEDevice* pSubdev);

    void                    setWgContextPool(WgContextPool* pWgContextPool) { m_pWgContextPool = pWgContextPool; }

    ITaskExecutor&          getTaskExecutor() { return *m_pTaskExecutor; }

    queue_t                 GetDefaultQueue() { return m_pDefaultQueue.GetPtr(); }

    ITEDevice*              GetRootDevice() { return m_pRootDevice.GetPtr(); }

    // ITaskExecutorObserver
    void*                   OnThreadEntry();
    void                    OnThreadExit( void* currentThreadData );
    TE_BOOLEAN_ANSWER       MayThreadLeaveDevice( void* currentThreadData );

#ifdef __INCLUDE_MKL__
    OMPExecutorThread*			getOmpExecutionThread() const {return m_pOMPExecutionThread;}
#endif
protected:
    cl_int                    m_iDevId;
    IOCLDevLogDescriptor*     m_pLogDescriptor;
    cl_int                    m_iLogHandle;
    ocl_gpa_data*             m_pGPAData;
    IOCLFrameworkCallbacks*   m_pFrameworkCallBacks;
    ProgramService*           m_pProgramService;
    MemoryAllocator*          m_pMemoryAllocator;
    CPUDeviceConfig*          m_pCPUDeviceConfig;
    ITaskExecutor*	          m_pTaskExecutor;
    SharedPtr<ITEDevice>      m_pRootDevice;
    WgContextPool*            m_pWgContextPool;
    unsigned int              m_uiNumThreads;
    bool                      m_bTEActivated;

    Intel::OpenCL::Utils::SharedPtr<ITaskList> m_pDefaultQueue;

    IAffinityChangeObserver*    m_pObserver;

    // Internal implementation of functions
    static fnDispatcherCommandCreate_t*	m_vCommands[CL_DEV_CMD_MAX_COMMAND_TYPE];

    cl_dev_err_code	SubmitTaskArray(ITaskList* pList, cl_dev_cmd_desc* *cmds, cl_uint count);

    void	NotifyCommandStatusChange(const cl_dev_cmd_desc* pCmd, unsigned uStatus, int iErr);

    // Task failure notification
    class TaskFailureNotification : public ITask
    {
    public:

        PREPARE_SHARED_PTR(TaskFailureNotification)

        static SharedPtr<TaskFailureNotification> Allocate(TaskDispatcher* _this, const cl_dev_cmd_desc* pCmd, cl_int retCode)
        {
            return SharedPtr<TaskFailureNotification>(new TaskFailureNotification(_this, pCmd, retCode));
        }

        virtual ~TaskFailureNotification() {};

        // ITask interface
        bool	        CompleteAndCheckSyncPoint() {return false;}
        bool	        SetAsSyncPoint() {assert(0&&"Should not be called");return false;}
        bool	        IsCompleted() const {assert(0&&"Should not be called");return true;}
        bool	        Execute() { return Shoot( CL_DEV_ERROR_FAIL ); }
        void            Cancel()  { Shoot( CL_DEV_COMMAND_CANCELLED ); }
        long	        Release() { return 0; }
        TASK_PRIORITY   GetPriority() const { return TASK_PRIORITY_MEDIUM;}
        Intel::OpenCL::TaskExecutor::ITaskGroup* GetNDRangeChildrenTaskGroup() { return NULL; }
    protected:
        TaskDispatcher*			m_pTaskDispatcher;
        const cl_dev_cmd_desc*	m_pCmd;
        cl_int					m_retCode;

        bool	        Shoot(cl_dev_err_code err);

        TaskFailureNotification(TaskDispatcher* _this, const cl_dev_cmd_desc* pCmd, cl_int retCode) :
        m_pTaskDispatcher(_this), m_pCmd(pCmd), m_retCode(retCode) {}
    };

    cl_dev_err_code NotifyFailure(ITaskList* pList, cl_dev_cmd_desc* cmd, cl_int iRetCode);

#ifdef __INCLUDE_MKL__
    OMPExecutorThread*	m_pOMPExecutionThread;
#endif

private:
    TaskDispatcher(const TaskDispatcher&);
    TaskDispatcher& operator=(const TaskDispatcher&);
};

class AffinitizeThreads : public ITaskSet
{
public:
	
    PREPARE_SHARED_PTR(AffinitizeThreads)

    static SharedPtr<AffinitizeThreads> Allocate(unsigned int numThreads, cl_ulong timeOutInTicks, IAffinityChangeObserver* observer)
    {
        return SharedPtr<AffinitizeThreads>(new AffinitizeThreads(numThreads, timeOutInTicks, observer)); 
    }

    virtual ~AffinitizeThreads();

    // ITaskSet interface
    bool    SetAsSyncPoint()  { return false;}
    bool    CompleteAndCheckSyncPoint() { return true;}
    bool    IsCompleted() const { return true;}
    int	    Init(size_t region[], unsigned int &regCount);
    void*   AttachToThread(void* tls, size_t uiNumberOfWorkGroups, size_t firstWGID[], size_t lastWGID[]);
    void	  DetachFromThread(void* data);
    bool	  ExecuteIteration(size_t x, size_t y, size_t z, void* data);
    bool	  Finish(FINISH_REASON reason) { ++m_endBarrier; return false;}
    long    Release() { return 0;}
    void    Cancel() { Finish(FINISH_EXECUTION_FAILED); };
    Intel::OpenCL::TaskExecutor::ITaskGroup* GetNDRangeChildrenTaskGroup() { return NULL; }

    TASK_PRIORITY	        GetPriority()                       const	{ return TASK_PRIORITY_MEDIUM;}
    TASK_SET_OPTIMIZATION OptimizeBy()                        const { return TASK_SET_OPTIMIZE_DEFAULT; }
    unsigned int          PreferredSequentialItemsPerThread() const { return 1; }

    void WaitForEndOfTask() const;

protected:
    unsigned int						  m_numThreads;
    cl_ulong      						m_timeOut;
    Intel::OpenCL::Utils::AtomicCounter	m_barrier;
    volatile bool 						m_failed;

    Intel::OpenCL::Utils::AtomicCounter	m_endBarrier;

    IAffinityChangeObserver* m_pObserver;

    AffinitizeThreads(unsigned int numThreads, cl_ulong timeOutInTicks, IAffinityChangeObserver* observer);
};

}}}
