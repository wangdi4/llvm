// Copyright (c) 2014 Intel Corporation
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
//  isp_task_dispatcher.h
//  Implementation of the Class ISPTaskDispatcher
///////////////////////////////////////////////////////////

#pragma once

#include <cl_device_api.h>

#include <cl_shared_ptr.hpp>
#include <task_executor.h>

#include "isp_program_service.h"

#include "res/Ilibshim.h"

#include <vector>

namespace Intel { namespace OpenCL { namespace ISPDevice {

class ISPExtensionManager;
class ISPDeviceQueue;

typedef cl_dev_err_code fnDispatcherCommandCreate_t(ISPDeviceQueue* pDeviceQueue, cl_dev_cmd_desc* pCmd, Utils::SharedPtr<TaskExecutor::ITaskBase>* ppTask);


class ISPTaskDispatcher
{
public:
    ISPTaskDispatcher(cl_int devId, IOCLDevLogDescriptor *logDesc, IOCLFrameworkCallbacks* frameworkCallbacks, CameraShim* pCameraShim);
    virtual ~ISPTaskDispatcher();

    cl_dev_err_code Init();

    cl_dev_err_code CreateCommandList(cl_dev_cmd_list_props IN props, cl_dev_subdevice_id IN subdevice_id, cl_dev_cmd_list* OUT list);
    cl_dev_err_code FlushCommandList(cl_dev_cmd_list IN list);
    cl_dev_err_code ReleaseCommandList(cl_dev_cmd_list IN list);
    cl_dev_err_code ExecuteCommandList(cl_dev_cmd_list IN list, cl_dev_cmd_desc* IN *cmds, cl_uint IN count);
    cl_dev_err_code WaitForCompletion(cl_dev_cmd_list IN list, cl_dev_cmd_desc* IN cmdToWait);
    cl_dev_err_code CancelCommandList(cl_dev_cmd_list IN list);
    cl_dev_err_code ReleaseCommand(cl_dev_cmd_desc* IN cmdToRelease);

protected:
    // logger variables
    cl_int                  m_iDevId;
    IOCLDevLogDescriptor*   m_pLogDescriptor;
    cl_int                  m_iLogHandle;
    IOCLFrameworkCallbacks* m_pFrameworkCallBacks;
    CameraShim*             m_pCameraShim;

    // for extension mode
    ISPExtensionManager*    m_pIspExtensionManager;

    TaskExecutor::ITaskExecutor*                m_pTaskExecutor;
    Utils::SharedPtr<TaskExecutor::ITEDevice>   m_pRootDevice;
};


class ISPDeviceQueue
{
public:
    ISPDeviceQueue(IOCLDevLogDescriptor* logDesc, cl_int logHandle, IOCLFrameworkCallbacks* frameworkCallbacks, CameraShim* pCameraShim,
                   ISPExtensionManager* pIspExtensionManager);
    ~ISPDeviceQueue();

    cl_dev_err_code Init(TaskExecutor::CommandListCreationParam params, TaskExecutor::ITEDevice* device);
    cl_dev_err_code CreateCommand(cl_dev_cmd_desc* cmd);
    cl_dev_err_code NotifyFailure(cl_dev_cmd_desc* cmd);
    cl_dev_err_code NotifyFailureForAll(std::vector<cl_dev_cmd_desc*>& cmds);
    cl_dev_err_code Cancel();
    cl_dev_err_code Flush();
    cl_dev_err_code WaitForCommand(cl_dev_cmd_desc* cmd);
    cl_dev_err_code WaitForAllCommands();


    IOCLDevLogDescriptor*   GetLogDescriptor() const { return m_pLogDescriptor; }
    cl_int                  GetLogHandle() const { return m_iLogHandle; }
    IOCLFrameworkCallbacks* GetFrameworkCallbacks() const { return m_pFrameworkCallBacks; }
    CameraShim*             GetCameraHandle() const { return m_pCameraShim; }
    ISPExtensionManager*    GetExtensionManager() const { return m_pIspExtensionManager; }

protected:
    // logger variables
    IOCLDevLogDescriptor*   m_pLogDescriptor;
    cl_int                  m_iLogHandle;

    IOCLFrameworkCallbacks* m_pFrameworkCallBacks;
    CameraShim*             m_pCameraShim;

    // for extension mode
    ISPExtensionManager*    m_pIspExtensionManager;

    // TODO: do we really need shared pointer ?
    Utils::SharedPtr<TaskExecutor::ITaskList>    m_pTaskList;

    // extension mode
    std::vector<cl_dev_cmd_desc*>   m_bufferedCommands;
    bool m_bExtModeEnabled;


    // Internal implementation of functions
    static fnDispatcherCommandCreate_t* m_vCommands[CL_DEV_CMD_MAX_COMMAND_TYPE];

private:
    // TODO: support multiple device queues
    static cl_uint m_deviceQueuesCount;
};

class RequestedExtension
{
public:
    RequestedExtension(const ISPKernel* pIspKernel, const void* pKernelArgs) :
        m_pIspKernel(pIspKernel), m_pKernelArgs(pKernelArgs) {}

// TODO: protected
    const ISPKernel*  m_pIspKernel;
    const void*       m_pKernelArgs;
};

class RunningExtension
{
public:
    RunningExtension() : m_pCameraShim(nullptr), m_pOriginalArgs(nullptr) {}

    cl_dev_err_code Init(RequestedExtension* requestedExtension, CameraShim* pCameraShim);
    cl_dev_err_code Clean();

    bool isDifferentFrom(RequestedExtension* requestedExtension);

// TODO: protected
    CameraShim* m_pCameraShim;
    fw_info     m_blob;
    void*       m_pOriginalArgs;
    std::vector<void*> m_mappedPointers;
    std::vector<void*> m_allocatedBuffers;
};

class ISPExtensionManager
{
public:
    ISPExtensionManager(CameraShim* pCameraShim) :
        m_pCameraShim(pCameraShim) {}

    ~ISPExtensionManager()
    {
        ClearRequested();
        ClearRunning();
    }

    cl_dev_err_code ClearRequested();
    cl_dev_err_code ClearRunning();
    bool NeedToRecreate();

    cl_dev_err_code RequestExtension(const ISPKernel* pIspKernel, const void* pKernelArgs);
    cl_dev_err_code CreatePipelineFromRequested();

protected:
    CameraShim* m_pCameraShim;
    std::vector<RunningExtension>   m_vRunningExtensions;
    std::vector<RequestedExtension> m_vRequestedExtensions;


};

}}} // namespace Intel { namespace OpenCL { namespace ISPDevice {
