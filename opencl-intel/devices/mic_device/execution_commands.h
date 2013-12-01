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

#pragma once

#include "command.h"
#include "mic_dev_limits.h"
#include "device_service_communication.h"

#include <vector>

using namespace std;

namespace Intel { namespace OpenCL { namespace MICDevice {

class MICDevMemoryObject;
////////////////////////////////////////////////////////////////////////////////////
class ExecutionCommand : public Command
{
public:

    PREPARE_SHARED_PTR(Command)

    bool commandEnqueuedToPipe() { return true; };

protected:
    ExecutionCommand(CommandList* pCommandList, IOCLFrameworkCallbacks* pFrameworkCallBacks, cl_dev_cmd_desc* pCmd);
    virtual ~ExecutionCommand();

    /* Execute the command (Send it to execution in the device) */
    cl_dev_err_code executeInt(DeviceServiceCommunication::DEVICE_SIDE_FUNCTION funcId, char* commandNameStr);

    void AddMemoryObject( MICDevMemoryObject *memObj, bool isConstAccess );

    /* Initialize the appropriate execution command. */
    virtual cl_dev_err_code init() = 0;

    virtual void fireCallBack(void* arg);

    void init_profiling_mode();

    vector<COIBUFFER>           m_coiBuffsArr;           // List of buffers required for COI command
    vector<COI_ACCESS_FLAGS>    m_accessFlagsArr;        // the access flags of the COIBUFFERs array

    // COI events used in profiling and OOO queue
    command_event_struct        m_startEvent; 

    // A pointer used for COIRunFunction
    const void*                 m_pDispatchData;
    uint16_t                    m_uiDispatchDataSize;
};

////////////////////////////////////////////////////////////////////////////////////
class NDRange : public ExecutionCommand
{
public:
    PREPARE_SHARED_PTR(Command)

    /* static function for NDRange Command creation */
    static cl_dev_err_code Create(CommandList* pCommandList, IOCLFrameworkCallbacks* pFrameworkCallBacks, cl_dev_cmd_desc* pCmd, SharedPtr<Command>& pOutCommand);

    cl_dev_err_code execute() { return executeInt(DeviceServiceCommunication::EXECUTE_NDRANGE, (char*)"NDRange"); };


private:

    static cl_dev_err_code CheckCommandParams(CommandList* pCommandList, cl_dev_cmd_desc* cmd);
    /* Private constructor because We like to create Commands only by the factory method */
    NDRange(CommandList* pCommandList, IOCLFrameworkCallbacks* pFrameworkCallBacks, cl_dev_cmd_desc* pCmd);

    cl_dev_err_code init();
};

////////////////////////////////////////////////////////////////////////////////////
class FillMemObject : public ExecutionCommand
{
public:

    PREPARE_SHARED_PTR(Command)

	/* static function for FillMemObject Command creation */
    static cl_dev_err_code Create(CommandList* pCommandList, IOCLFrameworkCallbacks* pFrameworkCallBacks, cl_dev_cmd_desc* pCmd, SharedPtr<Command>& pOutCommand);

    cl_dev_err_code execute() { return executeInt(DeviceServiceCommunication::FILL_MEM_OBJECT, (char*)"FillMemObject"); };

private:

	/* Private constructor because We like to create Commands only by the factory method */
    FillMemObject(CommandList* pCommandList, IOCLFrameworkCallbacks* pFrameworkCallBacks, cl_dev_cmd_desc* pCmd);

    /* Initialize FillMemObject command. */
    cl_dev_err_code init();

    fill_mem_obj_dispatcher_data        m_fillDispatchData;
};

}}}
