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
//  isp_commands.h
//  Implementation of the ISP commands classes
///////////////////////////////////////////////////////////

#pragma once

#include <cl_device_api.h>
#include <cl_shared_ptr.hpp>
#include <task_executor.h>

#include "isp_program_service.h"
#include "isp_task_dispatcher.h"
#include "res/Ilibshim.h"

#include <vector>

namespace Intel { namespace OpenCL { namespace ISPDevice {

class ISPDeviceQueue;

// Base class for all ISP commands
class ISPCommandBase : public TaskExecutor::ITask
{
public:
    // TODO: add ISPDeviceQueue here and get other parameters from it
    ISPCommandBase(IOCLDevLogDescriptor* logDesc, cl_int logHandle, cl_dev_cmd_desc* pCmd,
                   IOCLFrameworkCallbacks* frameworkCallbacks) :
                m_pLogDescriptor(logDesc), m_iLogHandle(logHandle), m_pCmdDesc(pCmd),
                m_pFrameworkCallBacks(frameworkCallbacks), m_aIsSyncPoint(FALSE) {};

    void NotifyCommandStatusChanged(cl_int status, cl_dev_err_code errorCode);
    // TODO: add check commands params in create functions

    // ITask interfaces
    TaskExecutor::TASK_PRIORITY GetPriority() const { return TaskExecutor::TASK_PRIORITY_MEDIUM; }
    TaskExecutor::IThreadLibTaskGroup* GetNDRangeChildrenTaskGroup() { return nullptr; }

    bool IsCompleted() const { return m_bCompleted; }
    void Cancel() { NotifyCommandStatusChanged(CL_COMPLETE, CL_DEV_COMMAND_CANCELLED); }

    // TODO: what is this interface for ?
    // TODO: add implementation in NotifyCommandStatusChanged if this changed
    long Release() { assert(false && "Where am I?"); return 0; }

    // ITask's sync point interfaces:
    // The queue some how need to be signaled stop processing of the job list.
    // On other side RT should be aware that the command was done.
    // And this must be done by single instruction, otherwise there is a race.
    // NOTE: exchange() returns the previous vlaue of m_aIsSyncPoint
    bool CompleteAndCheckSyncPoint() { return 1 == m_aIsSyncPoint.exchange(1); }
    bool SetAsSyncPoint() { return 1 == m_aIsSyncPoint.exchange(1); }

protected:
    // logger variables
    IOCLDevLogDescriptor*   m_pLogDescriptor;
    cl_int                  m_iLogHandle;

    cl_dev_cmd_desc*        m_pCmdDesc;
    IOCLFrameworkCallbacks* m_pFrameworkCallBacks;

    volatile bool           m_bCompleted;

    // For ITask's sync point interfaces
    Utils::AtomicCounter    m_aIsSyncPoint;
};


class NDRange : public ISPCommandBase
{
public:
    static cl_dev_err_code Create(ISPDeviceQueue* pDeviceQueue, cl_dev_cmd_desc* pCmd, Utils::SharedPtr<TaskExecutor::ITaskBase>* ppTask);

    // ITask interface
    bool Execute();

protected:
    NDRange(IOCLDevLogDescriptor* logDesc, cl_int logHandle, cl_dev_cmd_desc* pCmd,
            IOCLFrameworkCallbacks* frameworkCallbacks, CameraShim* pCameraShim,
            ISPExtensionManager* pIspExtensionManager);

    void ExtractNDRangeKernelArguments();

    // called from Execute()
    bool ExecuteStartPreview();
    bool ExecuteStopPreview();
    bool ExecuteAutoFocus();
    bool ExecuteTakePicture(bool withAutoFocus);
    bool ExecuteSetPictureSize();
    bool ExecuteSetPreviewSize();
    bool ExecuteCopyPreviewBuffer();

    // TODO: maybe move to new class ?
    bool ExecuteStandaloneCustomKernel();
    bool MapArgumentsToISP(void* pArgsBuffer);
    bool UnmapArgumentsFromISP(void* pMappedArgsBuffer);

    CameraShim*             m_pCameraShim;
    const ISPKernel*        m_pIspKernel;
    std::vector<void*>      m_vArgs;
    ISPExtensionManager*    m_pIspExtensionManager;
};

class ReadWriteMemoryObject : public ISPCommandBase
{
public:
    static cl_dev_err_code Create(ISPDeviceQueue* pDeviceQueue, cl_dev_cmd_desc* pCmd, Utils::SharedPtr<TaskExecutor::ITaskBase>* ppTask);

    // ITask interface
    bool Execute();

protected:
    ReadWriteMemoryObject(IOCLDevLogDescriptor* logDesc, cl_int logHandle, cl_dev_cmd_desc* pCmd,
                    IOCLFrameworkCallbacks* frameworkCallbacks) :
            ISPCommandBase(logDesc, logHandle, pCmd, frameworkCallbacks) {}
};

class CopyMemoryObject : public ISPCommandBase
{
public:
    static cl_dev_err_code Create(ISPDeviceQueue* pDeviceQueue, cl_dev_cmd_desc* pCmd, Utils::SharedPtr<TaskExecutor::ITaskBase>* ppTask);

    // ITask interface
    bool Execute();

protected:
    CopyMemoryObject(IOCLDevLogDescriptor* logDesc, cl_int logHandle, cl_dev_cmd_desc* pCmd,
                    IOCLFrameworkCallbacks* frameworkCallbacks) :
            ISPCommandBase(logDesc, logHandle, pCmd, frameworkCallbacks) {}
};

class MapMemoryObject : public ISPCommandBase
{
public:
    static cl_dev_err_code Create(ISPDeviceQueue* pDeviceQueue, cl_dev_cmd_desc* pCmd, Utils::SharedPtr<TaskExecutor::ITaskBase>* ppTask);

    // ITask interface
    bool Execute();

protected:
    MapMemoryObject(IOCLDevLogDescriptor* logDesc, cl_int logHandle, cl_dev_cmd_desc* pCmd,
                    IOCLFrameworkCallbacks* frameworkCallbacks) :
            ISPCommandBase(logDesc, logHandle, pCmd, frameworkCallbacks) {}
};

class UnmapMemoryObject : public ISPCommandBase
{
public:
    static cl_dev_err_code Create(ISPDeviceQueue* pDeviceQueue, cl_dev_cmd_desc* pCmd, Utils::SharedPtr<TaskExecutor::ITaskBase>* ppTask);

    // ITask interface
    bool Execute();

protected:
    UnmapMemoryObject(IOCLDevLogDescriptor* logDesc, cl_int logHandle, cl_dev_cmd_desc* pCmd,
                    IOCLFrameworkCallbacks* frameworkCallbacks) :
            ISPCommandBase(logDesc, logHandle, pCmd, frameworkCallbacks) {}
};

class FillMemoryObject : public ISPCommandBase
{
public:
    static cl_dev_err_code Create(ISPDeviceQueue* pDeviceQueue, cl_dev_cmd_desc* pCmd, Utils::SharedPtr<TaskExecutor::ITaskBase>* ppTask);

    // ITask interface
    bool Execute();

protected:
    FillMemoryObject(IOCLDevLogDescriptor* logDesc, cl_int logHandle, cl_dev_cmd_desc* pCmd,
                    IOCLFrameworkCallbacks* frameworkCallbacks) :
            ISPCommandBase(logDesc, logHandle, pCmd, frameworkCallbacks) {}
};

class MigrateMemoryObject : public ISPCommandBase
{
public:
    static cl_dev_err_code Create(ISPDeviceQueue* pDeviceQueue, cl_dev_cmd_desc* pCmd, Utils::SharedPtr<TaskExecutor::ITaskBase>* ppTask);

    // ITask interface
    bool Execute();

protected:
    MigrateMemoryObject(IOCLDevLogDescriptor* logDesc, cl_int logHandle, cl_dev_cmd_desc* pCmd,
                    IOCLFrameworkCallbacks* frameworkCallbacks) :
            ISPCommandBase(logDesc, logHandle, pCmd, frameworkCallbacks) {}
};


// Task failure notification
class CommandFailureNotification : public ISPCommandBase
{
public:
    static cl_dev_err_code Create(ISPDeviceQueue* pDeviceQueue, cl_dev_cmd_desc* pCmd, Utils::SharedPtr<TaskExecutor::ITaskBase>* ppTask);

    // ITask interface
    bool Execute() { NotifyCommandStatusChanged(CL_COMPLETE, CL_DEV_ERROR_FAIL); return true; }

    bool IsCompleted() const { assert(false && "Shouldn't be called"); return m_bCompleted; }
    bool SetAsSyncPoint() { assert(false && "Shouldn't be called"); return false; }
    bool CompleteAndCheckSyncPoint() { return false; }

protected:
    CommandFailureNotification(IOCLDevLogDescriptor* logDesc, cl_int logHandle, cl_dev_cmd_desc* pCmd,
                            IOCLFrameworkCallbacks* frameworkCallbacks) :
            ISPCommandBase(logDesc, logHandle, pCmd, frameworkCallbacks) {};
};

// extension mode
class PipelineNDRange : public ISPCommandBase
{
public:
    static cl_dev_err_code Create(ISPDeviceQueue* pDeviceQueue, std::vector<cl_dev_cmd_desc*>& commands, Utils::SharedPtr<TaskExecutor::ITaskBase>* ppTask);

    // override from ISPCommandBase
    void NotifyCommandStatusChanged(cl_int status, cl_dev_err_code errorCode);

    // ITask interface
    bool Execute();

protected:
    PipelineNDRange(IOCLDevLogDescriptor* logDesc, cl_int logHandle, std::vector<cl_dev_cmd_desc*>& commands,
                    IOCLFrameworkCallbacks* frameworkCallbacks, CameraShim* pCameraShim, ISPExtensionManager* pIspExtensionManager);

    CameraShim*             m_pCameraShim;
    ISPExtensionManager*    m_pIspExtensionManager;

    std::vector<cl_dev_cmd_desc*> m_vCommandsDesc;

    std::vector<cl_dev_cmd_desc*> m_vPipelineStages;
    std::vector<cl_dev_cmd_desc*>::iterator m_itrBeginPipelineCmdDesc;
    std::vector<cl_dev_cmd_desc*>::iterator m_itrEndPipelineCmdDesc;
};


}}} // namespace Intel { namespace OpenCL { namespace ISPDevice {
