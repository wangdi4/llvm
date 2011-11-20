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
#include "cl_synch_objects.h"
#include "cl_thread.h"

//should be hash_map but cant compile #include <hash_map>
#include <map>
#include <list>

using namespace Intel::OpenCL::TaskExecutor;

namespace Intel { namespace OpenCL { namespace CPUDevice {

class TaskDispatcher
{
	friend class DispatcherCommand;
	friend class AffinitizeThreads;

public:
	TaskDispatcher(cl_int devId, IOCLFrameworkCallbacks *pDevCallbacks,
		ProgramService	*programService, MemoryAllocator *memAlloc,
		IOCLDevLogDescriptor *logDesc, CPUDeviceConfig *cpuDeviceConfig, IAffinityChangeObserver* pObsserver);
	virtual ~TaskDispatcher();

	virtual cl_dev_err_code init();

	virtual cl_dev_err_code createCommandList( cl_dev_cmd_list_props IN props, void** OUT list);
	virtual cl_dev_err_code retainCommandList( cl_dev_cmd_list IN list);
	virtual cl_dev_err_code releaseCommandList( cl_dev_cmd_list IN list );
	virtual cl_dev_err_code flushCommandList( cl_dev_cmd_list IN list);
	virtual cl_dev_err_code commandListExecute( cl_dev_cmd_list IN list, cl_dev_cmd_desc* IN *cmds, cl_uint IN count);
	virtual cl_dev_err_code commandListWaitCompletion(cl_dev_cmd_list IN list);

    virtual ProgramService* getProgramService(){ return m_pProgramService; }

	virtual bool            isDestributedAllocationRequried();
	virtual bool			isThreadAffinityRequried();
	
protected:
	cl_int						m_iDevId;
	IOCLDevLogDescriptor*		m_pLogDescriptor;
	cl_int						m_iLogHandle;
	ocl_gpa_data*				m_pGPAData;
	IOCLFrameworkCallbacks*		m_pFrameworkCallBacks;
	ProgramService*				m_pProgramService;
	MemoryAllocator*			m_pMemoryAllocator;
	CPUDeviceConfig*			m_pCPUDeviceConfig;
	ITaskExecutor*				m_pTaskExecutor;
    unsigned int				m_uiNumThreads;

	// Contexts required for execution of NDRange
	WGContext*						m_pWGContexts;
	WGContext*						GetWGContext(unsigned int id);

	IAffinityChangeObserver*        m_pObserver;
	// Internal implementation of functions
	static fnDispatcherCommandCreate_t*	m_vCommands[CL_DEV_CMD_MAX_COMMAND_TYPE];

	cl_dev_err_code	SubmitTaskArray(ITaskList* pList, cl_dev_cmd_desc* *cmds, cl_uint count);

	void	NotifyCommandStatusChange(const cl_dev_cmd_desc* pCmd, unsigned uStatus, int iErr);

	// Task failure notification
	class TaskFailureNotification : public ITask
	{
	public:
		TaskFailureNotification(TaskDispatcher* _this, const cl_dev_cmd_desc* pCmd, cl_int retCode) :
		  m_pTaskDispatcher(_this), m_pCmd(pCmd), m_retCode(retCode) {}

		// ITask interface
		bool	Execute();
		void	Release() {delete this;}
	protected:
		TaskDispatcher*			m_pTaskDispatcher;
		const cl_dev_cmd_desc*	m_pCmd;
		cl_int					m_retCode;
	};
	cl_dev_err_code NotifyFailure(ITaskList* pList, cl_dev_cmd_desc* cmd, cl_int iRetCode);
};

class SubdeviceTaskDispatcherThread;

class SubdeviceTaskDispatcher : public TaskDispatcher
{
    friend class SubdeviceTaskDispatcherThread;
public:
    SubdeviceTaskDispatcher(int numThreads, unsigned int* legalCoreIDs, cl_int devId, IOCLFrameworkCallbacks *pDevCallbacks,
        ProgramService	*programService, MemoryAllocator *memAlloc,
        IOCLDevLogDescriptor *logDesc, CPUDeviceConfig *cpuDeviceConfig, IAffinityChangeObserver* pObserver);
    virtual ~SubdeviceTaskDispatcher();
    
    virtual cl_dev_err_code createCommandList( cl_dev_cmd_list_props IN props, cl_dev_cmd_list* OUT list);
    virtual cl_dev_err_code flushCommandList( cl_dev_cmd_list IN list);
	virtual cl_dev_err_code commandListWaitCompletion(cl_dev_cmd_list IN list);

  	virtual cl_dev_err_code init();

	virtual bool isThreadAffinityRequried();
	virtual bool isDestributedAllocationRequried();
	
protected:
    SubdeviceTaskDispatcherThread*          m_thread;

	OclBinarySemaphore      				m_threadInitComplete;
	volatile bool        					m_threadInitSuccessful;

	unsigned int*       					m_legalCoreIDs;

	Intel::OpenCL::Utils::OclNaiveConcurrentQueue<cl_dev_cmd_list> m_commandListsForFlushing;

    virtual cl_dev_err_code internalFlushCommandList( cl_dev_cmd_list IN list);
};

class SubdeviceTaskDispatcherThread : public OclThread
{
    friend class SubdeviceTaskDispatcher;
public:
    SubdeviceTaskDispatcherThread(SubdeviceTaskDispatcher* dispatcher); 
    virtual ~SubdeviceTaskDispatcherThread(); 

    virtual int Run();
protected:
    SubdeviceTaskDispatcher* m_dispatcher;
    IThreadPoolPartitioner*  m_partitioner;
};

class AffinitizeThreads : public ITaskSet
{
public:
	AffinitizeThreads(unsigned int numThreads, cl_ulong timeOutInTicks, IAffinityChangeObserver* pObserver, TaskDispatcher* pTD);
	virtual ~AffinitizeThreads();

	// ITaskSet interface
	int	Init(size_t region[], unsigned int &regCount);
	int	AttachToThread(unsigned int uiWorkerId, size_t uiNumberOfWorkGroups, size_t firstWGID[], size_t lastWGID[]);
	int	DetachFromThread(unsigned int uiWorkerId);
	void	ExecuteIteration(size_t x, size_t y, size_t z, unsigned int uiWorkerId); 
	void	ExecuteAllIterations(size_t* dims, unsigned int uiWorkerId) {}
	void	Finish(FINISH_REASON reason) {};
	void    Release() { delete this;}

protected:
	unsigned int						m_numThreads;
	cl_ulong      						m_timeOut;
	Intel::OpenCL::Utils::AtomicCounter	m_barrier;
	IAffinityChangeObserver* 			m_pObserver;
	TaskDispatcher* 					m_pTD;
	volatile bool 						m_failed;
};

}}}
