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

class PerformanceDataStore;

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
    static cl_dev_err_code commandListFactory(cl_dev_cmd_list_props IN      props, 
                                              cl_dev_subdevice_id           subDeviceId, 
                                              NotificationPort*             pNotificationPort, 
                                              DeviceServiceCommunication*   pDeviceServiceComm,
		                                      IOCLFrameworkCallbacks*       pFrameworkCallBacks, 
		                                      ProgramService*               pProgramService, 
		                                      PerformanceDataStore*         pOverheadData,
		                                      CommandList**                 outCommandList);

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

	/* If cmdDescToWait != NULL waiting on the Command that store in cmdDescToWait->device_agent_data, otherwise wait to m_lastCommand Command. */
	void commandListWaitCompletion(cl_dev_cmd_desc* cmdDescToWait = NULL);

	/* Return this queue COIPIPLINE */
	COIPIPELINE getPipelineHandle() const { return m_pipe; };

	/* Return handle to COIFUNCTION according to the appripriate id */
    COIFUNCTION getDeviceFunction( DeviceServiceCommunication::DEVICE_SIDE_FUNCTION id ) const
                                        { return m_pDeviceServiceComm->getDeviceFunction( id ); };

	/* Return device COIPROCESS handle */
	COIPROCESS getDeviceProcess() const
						{ return m_pDeviceServiceComm->getDeviceProcessHandle(); };

    /* Run service function on a queue pipeline */
    bool runQueueServiceFunction(DeviceServiceCommunication::DEVICE_SIDE_FUNCTION id,
                                 size_t input_data_size, void* input_data,
                                 size_t output_data_size, void* output_data,
                                 unsigned int numBuffers, const COIBUFFER* buffers, 
                                 const COI_ACCESS_FLAGS* bufferAccessFlags ) const 
                                 
                        {   
                            assert(m_pipe);
                            return m_pDeviceServiceComm->runServiceFunction( id, 
                                                                           input_data_size,  input_data, 
                                                                           output_data_size, output_data,
                                                                           numBuffers, buffers, bufferAccessFlags, m_pipe ); 
                        };

	NotificationPort* getNotificationPort() { return m_pNotificationPort; };

	ProgramService* getProgramService() { return m_pProgramService; };

    PerformanceDataStore* getOverheadData() { return m_pOverhead_data; };

	/* Get the last Command that enqueued to this command_list. (Can be NULL if not command or the command deleted) */
	SharedPtr<Command> getLastCommand();

	/* Set the last command of this command_list to be newCommand.
		If newCommand == NULL and oldCommand == m_lastCommand than setting the last command to be NULL. Command MUST Call it with the optional argument Before deleting command. */
	void setLastCommand(const SharedPtr<Command>& newCommand, const SharedPtr<Command>& oldCommand = NULL);

	/* return true if the queue is InOrder command list */
	bool isInOrderCommandList() { return m_isInOrderQueue; };

protected:

	/* It is protected constructor because We want that the client will create CommandList only by the factory method */
	CommandList(NotificationPort*           pNotificationPort, 
	            DeviceServiceCommunication* pDeviceServiceComm, 
	            IOCLFrameworkCallbacks*     pFrameworkCallBacks, 
	            ProgramService*             pProgramService, 
	            PerformanceDataStore*       pOverheadData,
	            cl_dev_subdevice_id subDeviceId,
				bool isInOrder);

	// the last dependency barrier COIBarrier.
	COIEVENT          m_lastDependentBarrier;
	bool              m_validBarrier;

private:

	// definition of static function of Commands that create command object (factory)
    typedef cl_dev_err_code fnCommandCreate_t(CommandList* pCommandList, IOCLFrameworkCallbacks* pFrameworkCallBacks, cl_dev_cmd_desc* pCmd, SharedPtr<Command>& pOutCommand);

	/* Create new COIPIPELINE for this queue */
	cl_dev_err_code createPipeline();

	/* Call init_commands_queue() on device side. Call it after pipeline creation. */
	cl_dev_err_code initCommandListOnDevice();

	/* Call release_commands_queue() on device side. Call it before pipeline destruction. */
	cl_dev_err_code releaseCommandListOnDevice();

	/* Run function on device and wait for completion.
	   Run it without buffers data.
	   Run "func" on device side. */
	cl_dev_err_code runBlockingFuncOnDevice(DeviceServiceCommunication::DEVICE_SIDE_FUNCTION func,
	                                        void* in_data = NULL, size_t in_data_size = 0 );

	/* Factory for Command objects.
	   The client responsability is to delete the return object.
	   cmd - cl_dev_cmd_desc input data structure.
	   cmdObject - out paramenter for appropriate Command object according to cmd->type or FailureNotification object in case of failure.
	   Return CL_DEV_SUCCESS if succeeded */
	cl_dev_err_code createCommandObject(cl_dev_cmd_desc* cmd, SharedPtr<Command>& cmdObject);

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
    // per device overhead storage
    PerformanceDataStore*             m_pOverhead_data;

	// Shared pointer that store the last command enqueued (Can be NULL)
	SharedPtr<Command>				  m_lastCommand;

	// Spin mutex to guard the last Command object
	OclSpinMutex					  m_lastCommandMutex;

	// True if this is in order CommandList, otherwise False.
	bool							  m_isInOrderQueue;

#ifdef _DEBUG
	AtomicCounter					  m_numOfConcurrentExecutions;
#endif

};

}}}
