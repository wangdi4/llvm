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

/*
*
* File cpu_device.h
* declares C++ interface between the device and the Open CL frame work.
*
*/
#pragma once

#include "cl_device_api.h"
#include "cl_dynamic_lib.h"
#include "cpu_dev_limits.h"
#include "cpu_config.h"
#include "backend_wrapper.h"
#include "task_executor.h"
#include "wg_context_pool.h"
#include "task_dispatcher.h"
#include "tbb/scalable_allocator.h"
#include "IDeviceCommandManager.h"
#include <cl_synch_objects.h>
#include <map>
#include <vector>

namespace Intel { namespace OpenCL { namespace CPUDevice {

extern const char* CPU_STRING;
extern const char* VENDOR_STRING;

class ProgramService;
class MemoryAllocator;

class CPUDevice : public IOCLDeviceAgent, public IOCLDeviceFECompilerDescription, public IAffinityChangeObserver, public IDeviceCommandManager

{
protected:
    ProgramService*			    m_pProgramService;
    MemoryAllocator*		    m_pMemoryAllocator;
    TaskDispatcher*			    m_pTaskDispatcher;
    CPUDeviceConfig*		    m_pCPUDeviceConfig;
    IOCLFrameworkCallbacks*     m_pFrameworkCallBacks;
    cl_uint					    m_uiCpuId;
    IOCLDevLogDescriptor*	    m_pLogDescriptor;
    cl_int					    m_iLogHandle;
    cl_dev_cmd_list			    m_defaultCommandList;
    OpenCLBackendWrapper        m_backendWrapper;
    WgContextPool               m_wgContextPool;


    unsigned long               m_numCores;           // Architectural data on the underlying HW
    unsigned int*               m_pComputeUnitMap;    // A mapping between an OpenCL-defined core ID (1 is first CPU on second socket) and OS-defined core ID
    std::map<threadid_t, int>   m_threadToCore;       // Maps OS thread ID to core ID
    std::vector<threadid_t>     m_pCoreToThread;      // Maps OpenCL core ID to OS thread id which is pinned to the core, which can then be used access the scoreboard above
    std::vector<bool>           m_pCoreInUse;           // Keeps track over used compute units to prevent overlap

    Intel::OpenCL::Utils::OclSpinMutex m_ComputeUnitScoreboardMutex;

#if 0
	tbb::scalable_allocator<DeviceNDRange> m_deviceNDRangeAllocator;
	tbb::scalable_allocator<char>          m_deviceNDRangeContextAllocator;
#endif	

    static volatile bool    m_bDeviceIsRunning;

    virtual ~CPUDevice();

    // Called once on init to cache information about the underlying architecture
    cl_dev_err_code QueryHWInfo();
    // The functions below are called when a set of cores is about to be "trapped" into a sub-device, or released from such
    // The acquire fails if one of the compute units requested is a part of another sub-device.
    // These are currently called on create/release command list (= command queue)
    bool            AcquireComputeUnits(unsigned int* which, unsigned int how_many);
    void            ReleaseComputeUnits(unsigned int* which, unsigned int how_many);

    // Affinity observer interface
    void            NotifyAffinity(threadid_t tid, unsigned int core_index);

    // Translate an "absolute" core (CPU core) to a core index
    // Needed to allow the user to limit the cores the CPU device will run on
    bool            CoreToCoreIndex(unsigned int* core);


public:

    CPUDevice(cl_uint devId, IOCLFrameworkCallbacks *devCallbacks, IOCLDevLogDescriptor *logDesc);
    cl_dev_err_code	Init();

    static void    WaitUntilShutdown();

    static cl_dev_err_code   clDevGetDeviceInfo(unsigned int IN	dev_id, cl_device_info IN param, size_t IN val_size, void* OUT paramVal, size_t* OUT param_val_size_ret);

    static cl_dev_err_code clDevGetAvailableDeviceList(size_t IN  deviceListSize, unsigned int*   OUT deviceIdsList, size_t*   OUT deviceIdsListSizeRet);

    //Device Fission support

    cl_dev_err_code clDevPartition(  cl_dev_partition_prop IN props, cl_uint IN num_requested_subdevices, cl_dev_subdevice_id IN parent_device_id, cl_uint* INOUT num_subdevices,										void* IN param, cl_dev_subdevice_id* OUT subdevice_ids);
    cl_dev_err_code clDevReleaseSubdevice(  cl_dev_subdevice_id IN subdevice_id);

    // Device entry points
    cl_dev_err_code clDevCreateCommandList( cl_dev_cmd_list_props IN props, cl_dev_subdevice_id IN subdevice_id, cl_dev_cmd_list* OUT list);
    cl_dev_err_code clDevFlushCommandList( cl_dev_cmd_list IN list);
    cl_dev_err_code clDevReleaseCommandList( cl_dev_cmd_list IN list );
    cl_dev_err_code clDevCommandListExecute( cl_dev_cmd_list IN list, cl_dev_cmd_desc* IN *cmds, cl_uint IN count);
    cl_dev_err_code clDevCommandListWaitCompletion(cl_dev_cmd_list IN list, cl_dev_cmd_desc* IN cmdToWait);
    cl_dev_err_code clDevCommandListCancel( cl_dev_cmd_list IN list );
    cl_dev_err_code clDevGetSupportedImageFormats( cl_mem_flags IN flags, cl_mem_object_type IN imageType,
            cl_uint IN numEntries, cl_image_format* OUT formats, cl_uint* OUT numEntriesRet) const;
    cl_dev_err_code clDevGetMemoryAllocProperties( cl_mem_object_type IN memObjType,	cl_dev_alloc_prop* OUT pAllocProp );
    cl_dev_err_code clDevCreateMemoryObject( cl_dev_subdevice_id IN node_id, cl_mem_flags IN flags, const cl_image_format* IN format,
                size_t	IN dim_count, const size_t* IN dim_size, IOCLDevRTMemObjectService*	IN pRTMemObjService,
                IOCLDevMemoryObject* OUT *memObj);
    cl_dev_err_code clDevCheckProgramBinary( size_t IN binSize, const void* IN bin );
    cl_dev_err_code clDevCreateProgram( size_t IN binSize, const void* IN bin, cl_dev_binary_prop IN prop, cl_dev_program* OUT prog );
    cl_dev_err_code clDevCreateBuiltInKernelProgram( const char* IN szBuiltInNames, cl_dev_program* OUT prog );
    cl_dev_err_code clDevBuildProgram( cl_dev_program IN prog, const char* IN options, cl_build_status* OUT buildStatus);
    cl_dev_err_code clDevReleaseProgram( cl_dev_program IN prog );
    cl_dev_err_code clDevUnloadCompiler();
    cl_dev_err_code clDevGetProgramBinary( cl_dev_program IN prog, size_t IN size, void* OUT binary, size_t* OUT sizeRet );
    cl_dev_err_code clDevGetBuildLog( cl_dev_program IN prog, size_t IN size, char* OUT log, size_t* OUT size_ret);
    cl_dev_err_code clDevGetSupportedBinaries( size_t IN count, cl_prog_binary_desc* OUT types, size_t* OUT sizeRet );
    cl_dev_err_code clDevGetKernelId( cl_dev_program IN prog, const char* IN name, cl_dev_kernel* OUT kernelId );
    cl_dev_err_code clDevGetProgramKernels( cl_dev_program IN prog, cl_uint IN numKernels, cl_dev_kernel* OUT kernels,
                 cl_uint* OUT numKernelsRet );
    cl_dev_err_code clDevGetKernelInfo( cl_dev_kernel IN kernel, cl_dev_kernel_info IN param, size_t IN valueSize,
						void* OUT value, size_t* OUT valueSizeRet );
    cl_ulong	clDevGetPerformanceCounter();
    cl_dev_err_code	clDevSetLogger(IOCLDevLogDescriptor *);
    void		clDevCloseDevice(void);
    void        clDevReleaseCommand(cl_dev_cmd_desc* IN cmdToRelease);

    const IOCLDeviceFECompilerDescription& clDevGetFECompilerDecription() const {return *this;}

    // IOCLDeviceFECompilerDescription
    const char* clDevFEModuleName() const;
    const void* clDevFEDeviceInfo() const;
    size_t		clDevFEDeviceInfoSize() const;

    // TODO: Move the implemenations to be part of NDRange command / KernelCommand
    //       Will help with 2.0 porting to MIC
    //       Wait for new BE interface
    // IDeviceCommandManager
    int EnqueueKernel(queue_t queue, kernel_enqueue_flags_t flags, cl_uint uiNumEventsInWaitList, const clk_event_t* pEventWaitList, clk_event_t* pEventRet,
      const ICLDevBackendKernel_* pKernel, const void* pContext, size_t szContextSize, const cl_work_description_type* pNdrange
      );

    int EnqueueMarker(queue_t queue, cl_uint uiNumEventsInWaitList, const clk_event_t* pEventWaitList, clk_event_t* pEventRet);

    int RetainEvent(clk_event_t event);

    int ReleaseEvent(clk_event_t event);

    clk_event_t CreateUserEvent(int* piErrcodeRet);

    int SetEventStatus(clk_event_t event, int iStatus);

    void CaptureEventProfilingInfo(clk_event_t event, clk_profiling_info name, volatile void* pValue);

    queue_t GetDefaultQueueForDevice() const;

    unsigned int GetNumComputeUnits() const;
};

}}}
