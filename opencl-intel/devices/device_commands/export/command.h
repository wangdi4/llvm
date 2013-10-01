// Copyright (c) 2006-2012 Intel Corporation
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

#pragma once

#include <vector>
#include "task_executor.h"
#include "cl_shared_ptr.h"

using Intel::OpenCL::TaskExecutor::ITaskBase;
using Intel::OpenCL::TaskExecutor::ITaskList;
using Intel::OpenCL::TaskExecutor::ITask;
using Intel::OpenCL::TaskExecutor::TASK_PRIORITY;
using Intel::OpenCL::Utils::ReferenceCountedObject;
using Intel::OpenCL::Utils::SharedPtr;
using Intel::OpenCL::Utils::AtomicCounter;
using Intel::OpenCL::Utils::OclSpinMutex;

namespace Intel { namespace OpenCL { namespace DeviceCommands {

/**
 * This class represents any command that is enqueued to a device, either by the host or a kernel.
 */
class DeviceCommand : virtual public ReferenceCountedObject
{
public:

	PREPARE_SHARED_PTR(DeviceCommand)

	/**
	 * Add dependencies according to an event wait list
	 * @param pEventWaitList		an event wait list
	 * @param uiNumEventsInWaitList size of the wait list
	 * @return whether all events have completed
	 */
	bool AddWaitListDependencies(const clk_event_t* pEventWaitList, cl_uint uiNumEventsInWaitList);

	/**
	 * Notify that a DeviceCommand for which this DeviceCommand is waiting has finished (completed for event wait list or finished execution for parent kernel/work group)
	 * @param err the error code of the completed DeviceCommand
	 */
	void NotifyCommandFinished(cl_dev_err_code err);

	/**
	 * @return the error code of this DeviceCommand
	 */
	cl_dev_err_code GetError() const { return m_err; }		

	/**
	 * add dependency to this DeviceCommand for another DeviceCommand
	 */
	void AddDependency() { m_numDependencies++; }

	/**
	 * @return the ITaskList that implements the command-queue on which this DeviceCommand executes
	 */
	const SharedPtr<ITaskList>& GetList() { return m_list; }

	/**
	 * @return whether this DeviceCommand is a user command
	 */
	virtual bool IsUserCommand() const { return false; }

	/**
	 * @return this DeviceCommand's execution time
	 */
	unsigned long long GetExecutionTime() const { return m_ulExecTime; }

	/**
	 * Set the user pointer where the execution time value will be stored when it is available
	 * @return whether the pointer has been set (if not, then the information is already available when calling GetExecutionTime())
	 */
	bool SetExecTimeUserPtr(volatile void* pExecTimeUserPtr);	

	/**
	 * @return whether this DeviceCommand's state is CL_COMPLETE
	 */
	bool IsCompleted() const { return m_bCompleted; }

	/**
	 * @return a pointer to this DeviceCommand as an ITaskBase object or NULL if it does not inherit from ITaskBase
	 */
	ITaskBase* GetMyTaskBase() { return m_pMyTaskBase; };

    /**
     * Launch this DeviceCommand for execution
     */
    virtual void Launch() = 0;

protected:

	/**
	 * Constructor
	 * @param list			the ITaskList that implements the command-queue on which this DeviceCommand executes
	 * @param pMyTaskBase	a pointer to itself as ITaskBase or NULL if the concrete class does not inherit from ITaskBase
	 */
	DeviceCommand(const SharedPtr<ITaskList>& list, ITaskBase* pMyTaskBase) :
	  m_err(CL_DEV_SUCCESS), m_bCompleted(false), m_ulStartExecTime(0), m_ulExecTime(0),
	  m_pExecTimeUserPtr(NULL), m_list(list),
		m_pMyTaskBase(pMyTaskBase), m_bIsProfilingEnabled(list->IsProfilingEnabled())
    { }

	/**
	 * Signal that this DeviceCommand is changing its state to CL_COMPLETE
	 * @param err the error code of the DeviceCommand
	 */
	void SignalComplete(cl_dev_err_code err);

	/**
	 * Set an error code for this DeviceCommand
	 * @param err the error code to set
	 */
	void SetError(cl_dev_err_code err);

	/**
	 * Signal that this DeviceCommand's execution has started
	 */
	void StartExecutionProfiling();

	/**
	 * Signal that this DeviceCommand's execution has ended
	 */
	void StopExecutionProfiling();    

private:

	AtomicCounter m_numDependencies;
	cl_dev_err_code m_err;
	volatile bool m_bCompleted;
	unsigned long long m_ulStartExecTime;
	unsigned long long m_ulExecTime;
	volatile void* m_pExecTimeUserPtr;
	std::vector<SharedPtr<DeviceCommand> > m_waitingCommands;
	const SharedPtr<ITaskList> m_list;
	mutable OclSpinMutex m_mutex;
	ITaskBase* const m_pMyTaskBase;
	const bool m_bIsProfilingEnabled;
};

/**
 * This class represents a device-side marker command
 */
class Marker : public DeviceCommand, public ITask
{
public:

	PREPARE_SHARED_PTR(Marker)

	/**
	 * Allocate a new Marker
	 * @param list the ITaskList that implements the command-queue on which this DeviceCommand executes
	 */
	static SharedPtr<Marker> Allocate(const SharedPtr<ITaskList>& list) { return new Marker(list); }

	// inherited methods:

	virtual bool Execute()
	{ 
		StartExecutionProfiling();
		SignalComplete(CL_DEV_SUCCESS);
		StopExecutionProfiling();
		return true;
	}

	bool IsTaskSet() const { return false; }

	bool IsCompleted() const  { return IsCompleted(); }

	virtual TASK_PRIORITY GetPriority() const { return Intel::OpenCL::TaskExecutor::TASK_PRIORITY_MEDIUM; }

	virtual bool CompleteAndCheckSyncPoint() { return false; }
	
	virtual bool SetAsSyncPoint()
	{
		ASSERT_RET_VAL(false, "Device commands can't be used as a synchronization point", IsCompleted());
		return IsCompleted();
	}

	virtual long Release() { return 0; }

	void Cancel() { SetError(CL_DEV_COMMAND_CANCELLED); }

	virtual Intel::OpenCL::TaskExecutor::ITaskGroup* GetNDRangeChildrenTaskGroup() { return NULL; }

    virtual void Launch() { Execute(); }

private:

	Marker(const SharedPtr<ITaskList>& list) : DeviceCommand(list, this) { }

};

/**
 * This class represents a device-side user event
 */
class UserEvent : public DeviceCommand
{
public:

	PREPARE_SHARED_PTR(UserEvent)

	/**
	 * Allocate a new UserEvent
	 */
	static SharedPtr<UserEvent> Allocate() { return new UserEvent(); }	

	/**
	 * Set the execution status of the UserEvent
	 * @param iStatus CL_COMPLETE or a negative integer value indicating an error
	 */
	void SetStatus(int iStatus)
	{
		if (CL_COMPLETE == iStatus)
		{
			SetError(CL_DEV_SUCCESS);
		}
		else
		{
			SetError(CL_DEV_ERROR_FAIL);
		}
		SignalComplete(GetError());
	}

	// inherited methods

	bool IsUserCommand() const { return true; }

    virtual void Launch()
    {
        assert(false && "UserEvent shouldn't be launched");
    }

private:

	UserEvent() : DeviceCommand(NULL, NULL) { }

};

}}}
