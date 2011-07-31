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
#include "commands.h"

#include <source/COIPipeline_source.h>
#include <common/COITypes_common.h>

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
    static cl_dev_err_code commandListFactory(cl_dev_cmd_list_props IN props, NotificationPort* pNotificationPort,
	                                                DeviceServiceCommunication* pDeviceServiceComm, CommandList** outCommandList);

    /* Do nothing because the COIPipeline send the command as it enter to it. (Flush is redundant) */
	cl_dev_err_code flushCommandList() { return CL_DEV_SUCCESS; };

	/* Increment the command list reference counter.
       Return CL_DEV_SUCCESS if succeeded and CL_DEV_INVALID_OPERATION if failed. */
	cl_dev_err_code retainCommandList();

	/* Decrement the command list reference counter.
	   outDelete - set to true in case of reference counter = 0 and retain operation succeeded.
	   Return CL_DEV_SUCCESS if succeeded and CL_DEV_INVALID_OPERATION if failed.*/
	cl_dev_err_code releaseCommandList(bool* outDelete);

	/* Perform the commands or enque it to COIPipeline.
	   It is not thread safe method. */
	virtual cl_dev_err_code commandListExecute(cl_dev_cmd_desc* IN *cmds, cl_uint IN count) = 0;

	void commandListWaitCompletion();

	COIPIPELINE getPipelineHandle() const { return m_pipe; };
    COIFUNCTION getDeviceFunction( DeviceServiceCommunication::DEVICE_SIDE_FUNCTION id ) const
                                        { return m_pDeviceServiceComm->getDeviceFunction( id ); };

	NotificationPort* getNotificationPort() { return m_pNotificationPort; };

	virtual void getLastDependentBarrier(COIBARRIER** barrier, unsigned int* numDependencies, bool isExecutionTask) = 0;

	virtual void setLastDependentBarrier(COIBARRIER barrier, bool lastCmdWasExecution) = 0;

protected:

	CommandList(NotificationPort* pNotificationPort, DeviceServiceCommunication* pDeviceServiceComm);

	/* Factory for Command objects.
	   The client responsability is to delete the return object.
	   cmd - cl_dev_cmd_desc input data structure.
	   cmdObject - out paramenter for appropriate Command object according to cmd->type or FailureNotification object in case of failure.
	   Return CL_DEV_SUCCESS if succeeded */
	cl_dev_err_code createCommandObject(cl_dev_cmd_desc* cmd, Command** cmdObject);

	virtual bool isInOrderCommandList() = 0;

private:

    typedef cl_dev_err_code fnCommandCreate_t(bool isInOrder, Command** pOutCommand);

	cl_dev_err_code createPipeline();


	// pointer to device notification port object
	NotificationPort*                 m_pNotificationPort;
	// pointer to device service communication object
	DeviceServiceCommunication*       m_pDeviceServiceComm;
	// reference counter for this object (must be greater than 0 during object lifetime)
	volatile unsigned int             m_refCounter;
	// the pipe line to MIC device
	COIPIPELINE                       m_pipe;
	// pointer to static function that create Command object
	fnCommandCreate_t*                m_vCommands[CL_DEV_CMD_MAX_COMMAND_TYPE];
};

}}}

