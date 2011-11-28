// Copyright (c) 2006-2008 Intel Corporation
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

#include "cl_device_api.h"
#include "notification_port.h"
#include "device_service_communication.h"
#include "program_service.h"
#include "command.h"

#include "cl_synch_objects.h"

#include <source/COIPipeline_source.h>
#include <common/COITypes_common.h>

using namespace Intel::OpenCL::Utils;

namespace Intel { namespace OpenCL { namespace MICDevice {

/* An abstract class with represent command list.
   The implementation is NOT completely thread safe. (commandListExecute is not thread safe, it assume that the R.T. send one series of commands in a time) */
class CommandList
{

public:

    virtual ~CommandList();

    /* Factory for commandList objects (InOrder / OutOfOrder)
	   props - the properties of the new command list.
	   pNotificationPort - reference to device NotificationPort object.
	   pDeviceServiceComm - reference to device DeviceServiceCommunication object.
	   outCommandList - out parameter which include the new CommandList object if succeeded.
	   It can fail if COIPipelineCreate create fails.
	   Return CL_DEV_SUCCESS if succeeded. */
    static cl_dev_err_code commandListFactory(cl_dev_cmd_list_props IN props, cl_dev_subdevice_id subDeviceId, NotificationPort* pNotificationPort, DeviceServiceCommunication* pDeviceServiceComm,
		                                            IOCLFrameworkCallbacks* pFrameworkCallBacks, ProgramService* pProgramService, CommandList** outCommandList);

    /* Do nothing because the COIPipeline send the command as it enter to it. (Flush is redundant) */
	cl_dev_err_code flushCommandList() { return CL_DEV_SUCCESS; };

	/* Increment the command list reference counter.
       Return CL_DEV_SUCCESS if succeeded and CL_DEV_INVALID_OPERATION if failed. */
	cl_dev_err_code retainCommandList();

	/* Decrement the command list reference counter.
	   outDelete - set to true in case of reference counter = 0 and retain operation succeeded.
	   Return CL_DEV_SUCCESS if succeeded and CL_DEV_INVALID_OPERATION if failed.*/
	cl_dev_err_code releaseCommandList(bool* outDelete);

	/* Perform the commands or enqueue it to COIPipeline.
	   It is NOT thread safe method. */
	cl_dev_err_code commandListExecute(cl_dev_cmd_desc* IN *cmds, cl_uint IN count);

	/* The operation is not supported by device. The runtime should handle wait by itself */
	void commandListWaitCompletion() { return; };

	/* Return this queue COIPIPLINE */
	COIPIPELINE getPipelineHandle() const { return m_pipe; };

	/* Return handle to COIFUNCTION according to the appripriate id */
    COIFUNCTION getDeviceFunction( DeviceServiceCommunication::DEVICE_SIDE_FUNCTION id ) const
                                        { return m_pDeviceServiceComm->getDeviceFunction( id ); };

	/* Return device COIPROCESS handle */
	COIPROCESS getDeviceProcess() const
						{ return m_pDeviceServiceComm->getDeviceProcessHandle(); };

	NotificationPort* getNotificationPort() { return m_pNotificationPort; };

	ProgramService* getProgramService() { return m_pProgramService; };

	/* In case of inOrder command list set:
	       If threre is valid barrier and the previous command is not NDRange or 'isExecutionTask' == false than return the last barrier and set 'numDependencies' to 1.
		   othrewise it means that the barrier is not valid or that the previous command was NDRange and the current command is NDRange in this case We return NULL as barrier 
		   and set 'numDependencies' to 0 because the dependency betweem the commands enforced by COIPIPELINE.
	   In case of outOfOrder command list set:
	      Do nothing.
	   It is NOT thread safe method because 'commandListExecute()' is NOT thread safe.
	*/
	virtual void getLastDependentBarrier(COIEVENT** barrier, unsigned int* numDependencies, bool isExecutionTask) = 0;

	/* set last dependency barrier to be barrier and lastCommandWasExecution flag accordingly (In case of InOrder command list).
	   Do nothing in case of out of order commandList. 
	   It is NOT thread safe method because 'commandListExecute()' is NOT thread safe.
	*/
	virtual void setLastDependentBarrier(COIEVENT barrier, bool lastCmdWasExecution) = 0;

	/* return true if the queue is InOrder command list */
	virtual bool isInOrderCommandList() = 0;

protected:

	/* It is protected constructor because We want that the client will create CommandList only by the factory method */
	CommandList(NotificationPort* pNotificationPort, DeviceServiceCommunication* pDeviceServiceComm, IOCLFrameworkCallbacks* pFrameworkCallBacks, ProgramService* pProgramService, cl_dev_subdevice_id subDeviceId);

	// the last dependency barrier COIBarrier.
	COIEVENT          m_lastDependentBarrier;
	bool                m_validBarrier;

private:

	// definition of static function of Commands that create command object (factory)
    typedef cl_dev_err_code fnCommandCreate_t(CommandList* pCommandList, IOCLFrameworkCallbacks* pFrameworkCallBacks, cl_dev_cmd_desc* pCmd, Command** pOutCommand);

	/* Create new COIPIPELINE for this queue */
	cl_dev_err_code createPipeline();

	/* Call init_commands_queue() on device side. Call it after pipeline creation. */
	cl_dev_err_code initCommandListOnDevice();

	/* Call release_commands_queue() on device side. Call it before pipeline destruction. */
	cl_dev_err_code releaseCommandListOnDevice();

	/* Run function on device and wait for completion.
	   Run it without buffers or misc data.
	   Run "func" on device side. */
	cl_dev_err_code runBlockingFuncOnDevice(DeviceServiceCommunication::DEVICE_SIDE_FUNCTION func);

	/* Factory for Command objects.
	   The client responsability is to delete the return object.
	   cmd - cl_dev_cmd_desc input data structure.
	   cmdObject - out paramenter for appropriate Command object according to cmd->type or FailureNotification object in case of failure.
	   Return CL_DEV_SUCCESS if succeeded */
	cl_dev_err_code createCommandObject(cl_dev_cmd_desc* cmd, Command** cmdObject);

	// pointer to device notification port object
	NotificationPort*                 m_pNotificationPort;
	// pointer to device service communication object
	DeviceServiceCommunication*       m_pDeviceServiceComm;
	// pointer to IOCLFrameworkCallbacks object in order to notify framework about completion of command
	IOCLFrameworkCallbacks*           m_pFrameworkCallBacks;
	// pointer to ProgramService object
	ProgramService*                   m_pProgramService;
	// reference counter for this object (must be greater than 0 during object lifetime)
	AtomicCounter		              m_refCounter;
	// the pipe line to MIC device
	COIPIPELINE                       m_pipe;
	// pointer to static function that create Command object
	static fnCommandCreate_t*         m_vCommands[CL_DEV_CMD_MAX_COMMAND_TYPE];
	// Sub device ID
	cl_dev_subdevice_id				  m_subDeviceId;

#ifdef _DEBUG
	AtomicCounter					  m_numOfConcurrentExecutions;
#endif

};



/* Class wich represent In order command list */
class InOrderCommandList : public CommandList
{

public:

    InOrderCommandList(NotificationPort* pNotificationPort, DeviceServiceCommunication* pDeviceServiceComm, IOCLFrameworkCallbacks* pFrameworkCallBacks, ProgramService* pProgramService, cl_dev_subdevice_id subDeviceId);
	
	virtual ~InOrderCommandList();
	
	/* Get the current dependent barrier and its details according to 'isExecutionTask' flag and according to the value in m_lastCommandWasNDRange. */
	void getLastDependentBarrier(COIEVENT** barrier, unsigned int* numDependencies, bool isExecutionTask);
	
	/* Set the current dependent barrier and its details */
	void setLastDependentBarrier(COIEVENT barrier, bool lastCmdWasExecution);
	
	bool isInOrderCommandList() { return true; };
	
private:

	// is the last command was NDRange
	bool                m_lastCommandWasNDRange;

};



class OutOfOrderCommandList : public CommandList
{

public:

    OutOfOrderCommandList(NotificationPort* pNotificationPort, DeviceServiceCommunication* pDeviceServiceComm, IOCLFrameworkCallbacks* pFrameworkCallBacks, ProgramService* pProgramService, cl_dev_subdevice_id subDeviceId)
		: CommandList(pNotificationPort, pDeviceServiceComm, pFrameworkCallBacks, pProgramService, subDeviceId) {};
	
	virtual ~OutOfOrderCommandList() {};
	
	/* In out of order command list there is no dependency between commands so return NULL as barrier and 0 as num dependencies. */
	void getLastDependentBarrier(COIEVENT** barrier, unsigned int* numDependencies, bool isExecutionTask) { *numDependencies = 0; *barrier = NULL; };
	
	/* Set the current dependent barrier and its details. Need it in out of order command list only for the implementation of 'commandListWaitCompletion()' */
	void setLastDependentBarrier(COIEVENT barrier, bool lastCmdWasExecution) { return; };
	
	bool isInOrderCommandList() { return false; };

};

}}}

