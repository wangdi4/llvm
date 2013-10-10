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

#include "command.h"

#include "mic_dev_limits.h"
#include "exe_cmd_mem_handler.h"
#include "device_service_communication.h"

#include <vector>

using namespace std;

namespace Intel { namespace OpenCL { namespace MICDevice {

class ExecutionCommand : public Command
{
public:

    PREPARE_SHARED_PTR(Command)

    bool commandEnqueuedToPipe() { return true; };

protected:

    ExecutionCommand(CommandList* pCommandList, IOCLFrameworkCallbacks* pFrameworkCallBacks, cl_dev_cmd_desc* pCmd);

    virtual ~ExecutionCommand() {};

    /* Execute the command (Send it to execution in the device) */
    cl_dev_err_code executeInt(DeviceServiceCommunication::DEVICE_SIDE_FUNCTION funcId, char* commandNameStr);

    /* Initialize the appropriate execution command. */
    virtual cl_dev_err_code init(vector<COIBUFFER>& ppOutCoiBuffsArr, vector<COI_ACCESS_FLAGS>& ppAccessFlagArr) = 0;

    virtual void fireCallBack(void* arg);

    void init_profiling_mode();

    MiscDataHandler m_miscDatahandler;

    DispatcherDataHandler m_dispatcherDatahandler;

    // Contains COIEVENT that will signal when the Command will start.
    command_event_struct m_startBarrier;
};


class NDRange : public ExecutionCommand
{

public:

    PREPARE_SHARED_PTR(Command)

    /* static function for NDRange Command creation */
    static cl_dev_err_code Create(CommandList* pCommandList, IOCLFrameworkCallbacks* pFrameworkCallBacks, cl_dev_cmd_desc* pCmd, SharedPtr<Command>& pOutCommand);

    cl_dev_err_code execute() { return executeInt(DeviceServiceCommunication::EXECUTE_NDRANGE, (char*)"NDRange"); };

    void fireCallBack(void* arg);

protected:

    virtual ~NDRange();

private:

    static cl_dev_err_code CheckCommandParams(CommandList* pCommandList, cl_dev_cmd_desc* cmd);

    /* Private constructor because We like to create Commands only by the factory method */
    NDRange(CommandList* pCommandList, IOCLFrameworkCallbacks* pFrameworkCallBacks, cl_dev_cmd_desc* pCmd);

    cl_dev_err_code init(vector<COIBUFFER>& ppOutCoiBuffsArr, vector<COI_ACCESS_FLAGS>& ppAccessFlagArr);

    /* Release resources. */
    void releaseResources(bool releaseCoiObjects = true);
    void releaseKernel( void );

    bool        m_kernel_locked;
};



class FillMemObject : public ExecutionCommand
{
public:

    PREPARE_SHARED_PTR(Command)

    /* static function for FillMemObject Command creation */
    static cl_dev_err_code Create(CommandList* pCommandList, IOCLFrameworkCallbacks* pFrameworkCallBacks, cl_dev_cmd_desc* pCmd, SharedPtr<Command>& pOutCommand);

    cl_dev_err_code execute() { return executeInt(DeviceServiceCommunication::FILL_MEM_OBJECT, (char*)"FillMemObject"); };

protected:

    virtual ~FillMemObject() {};

private:

    /* Private constructor because We like to create Commands only by the factory method */
    FillMemObject(CommandList* pCommandList, IOCLFrameworkCallbacks* pFrameworkCallBacks, cl_dev_cmd_desc* pCmd);

    /* Initialize FillMemObject command. */
    cl_dev_err_code init(vector<COIBUFFER>& ppOutCoiBuffsArr, vector<COI_ACCESS_FLAGS>& ppAccessFlagArr);
};

}}}
