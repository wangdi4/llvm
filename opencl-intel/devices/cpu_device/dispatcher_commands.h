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
//  dispatcher_commands.h
//  Declaration of internal task dispatcher commands
////////////////////////////////////////////////////////////

#pragma once

#include "cl_device_api.h"
#include "cpu_dev_limits.h"
#include "memory_allocator.h"
#include "program_service.h"
#include "task_executor.h"
#include "wg_context.h"
#include "cl_synch_objects.h"
#if defined(USE_GPA)    
    #include <ittnotify.h>
#endif


#define COLOR_TABLE_SIZE 64

using namespace Intel::OpenCL::TaskExecutor;

namespace Intel { namespace OpenCL { namespace CPUDevice {

class TaskDispatcher;

typedef cl_dev_err_code fnDispatcherCommandCreate_t(TaskDispatcher* pTD, cl_dev_cmd_desc* pCmd, ITaskBase* *pTask);

// Base class for handling dispatcher command execution
// All Commands will be implement this interface
class DispatcherCommand
{
public:
    DispatcherCommand(TaskDispatcher* pTD);

protected:
    void NotifyCommandStatusChanged(cl_dev_cmd_desc* cmd, unsigned uStatus, int iErr);
    inline WGContext*   GetWGContext(unsigned int id);

    TaskDispatcher*             m_pTaskDispatcher;
    MemoryAllocator*            m_pMemAlloc;
    IOCLDevLogDescriptor*       m_pLogDescriptor;
    cl_int                      m_iLogHandle;
    cl_dev_cmd_desc*            m_pCmd;
    ocl_gpa_data*               m_pGPAData;
};

// OCL Read/Write buffer execution
class ReadWriteMemObject : public DispatcherCommand, public ITask
{
public:
    static cl_dev_err_code Create(TaskDispatcher* pTD, cl_dev_cmd_desc* pCmd, ITaskBase* *pTask);

    // ITask interface
    bool    Execute();
    void    Release() {delete this;}

protected:
    ReadWriteMemObject(TaskDispatcher* pTD);
    cl_dev_err_code CheckCommandParams(cl_dev_cmd_desc* cmd);
};

//OCL Copy Mem Obj Command
class CopyMemObject : public DispatcherCommand, public ITask
{
public:
    static cl_dev_err_code Create(TaskDispatcher* pTD, cl_dev_cmd_desc* pCmd, ITaskBase* *pTask);

    // DispatcherCommand interface
    cl_dev_err_code CheckCommandParams(cl_dev_cmd_desc* cmd);

    // ITask interface
    bool    Execute();
    void    Release() {delete this;}

protected:
    CopyMemObject(TaskDispatcher* pTD);
};

// OCL Native function execution
class NativeFunction : public DispatcherCommand, public ITask
{
public:
    static cl_dev_err_code Create(TaskDispatcher* pTD, cl_dev_cmd_desc* pCmd, ITaskBase* *pTask);

    // DispatcherCommand interface
    cl_dev_err_code CheckCommandParams(cl_dev_cmd_desc* cmd);

    // ITask interface
    bool    Execute();
    void    Release() {delete this;}

protected:
    NativeFunction(TaskDispatcher* pTD);

    char*               m_pArgV;
};

// OCL Map function execution
class MapMemObject : public DispatcherCommand, public ITask
{
public:
    static cl_dev_err_code Create(TaskDispatcher* pTD, cl_dev_cmd_desc* pCmd, ITaskBase* *pTask);

    // DispatcherCommand interface
    cl_dev_err_code CheckCommandParams(cl_dev_cmd_desc* cmd);

    // ITask interface
    bool    Execute();
    void    Release() {delete this;}

protected:
    MapMemObject(TaskDispatcher* pTD);
};

// OCL UnMap function execution
class UnmapMemObject : public DispatcherCommand, public ITask
{
public:
    static cl_dev_err_code Create(TaskDispatcher* pTD, cl_dev_cmd_desc* pCmd, ITaskBase* *pTask);

    // DispatcherCommand interface
    cl_dev_err_code CheckCommandParams(cl_dev_cmd_desc* cmd);

    // ITask interface
    bool    Execute();
    void    Release() {delete this;}

protected:
    UnmapMemObject(TaskDispatcher* pTD);
};

// OCL Kernel execution
class NDRange : public DispatcherCommand, public ITaskSet
{
public:
    static cl_dev_err_code Create(TaskDispatcher* pTD, cl_dev_cmd_desc* pCmd, ITaskBase* *pTask);

    static unsigned int RGBTable[COLOR_TABLE_SIZE];
    static AtomicCounter RGBTableCounter;

    // DispatcherCommand interface
    cl_dev_err_code CheckCommandParams(cl_dev_cmd_desc* cmd);

	// ITaskSet interface
	int	    Init(size_t region[], unsigned int &regCount);
	int	    AttachToThread(unsigned int uiWorkerId, size_t uiNumberOfWorkGroups, size_t firstWGID[], size_t lastWGID[]);
	int	    DetachFromThread(unsigned int uiWorkerId);
	void    ExecuteIteration(size_t x, size_t y, size_t z, unsigned int uiWorkerId); 
    void    ExecuteAllIterations(size_t* dims, unsigned int uiWorkerId);
	void    Finish(FINISH_REASON reason);
	void    Release();

protected:
    NDRange(TaskDispatcher* pTD);

    cl_int                      m_lastError;
    char                        m_pLockedParams[CPU_MAX_PARAMETER_SIZE];
    ICLDevBackendBinary_*       m_pBinary;

    // Executable information
    size_t                      m_MemBuffCount;
    size_t*                     m_pMemBuffSizes;


//  LARGE_INTEGER start, stop, freq;

#if defined(USE_GPA)
    // This code was removed for the initial porting of TAL
    // to GPA 4.0 and might be used in later stages
//  TAL_STRING_HANDLE           m_talKernelNameHandle;
//  unsigned int                m_talRGBColor;
#endif
#ifdef _DEBUG
    // For debug
    AtomicCounter m_lExecuting;
    AtomicCounter m_lFinish;
    AtomicCounter m_lAttaching;
#endif
};

}}}
