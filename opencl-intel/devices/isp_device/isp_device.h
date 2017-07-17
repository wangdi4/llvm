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

/*
*
* File isp_device.h
* declares C++ interface between the device and the Open CL frame work.
*
*/
#pragma once

#include <cl_device_api.h>

#include "res/Ilibshim.h"

#include "isp_program_service.h"
#include "isp_memory_allocator.h"
#include "isp_task_dispatcher.h"


namespace Intel { namespace OpenCL { namespace ISPDevice {

class ISPDevice : public IOCLDeviceAgent, IOCLDevRawMemoryAllocator
{
public:
    ISPDevice(cl_uint uiDevId, IOCLFrameworkCallbacks* frameworkCallbacks, IOCLDevLogDescriptor *logDesc);
    cl_dev_err_code Init();

    // static methods for DeviceAgent exported functions
    static cl_dev_err_code clDevGetDeviceInfo( unsigned int IN dev_id, cl_device_info IN param, size_t IN val_size, void* OUT paramVal, size_t* OUT param_val_size_ret );
    static cl_dev_err_code clDevGetAvailableDeviceList( size_t IN  deviceListSize, unsigned int* OUT deviceIdsList, size_t* OUT deviceIdsListSizeRet );


    /// --- IOCLDeviceAgent interfaces ---

    // Device fission interfaces
    cl_dev_err_code clDevPartition( cl_dev_partition_prop IN props, cl_uint IN num_requested_subdevices, cl_dev_subdevice_id IN parent_device_id,
                            cl_uint* INOUT num_subdevices, void* IN param, cl_dev_subdevice_id* OUT subdevice_ids );
    cl_dev_err_code clDevReleaseSubdevice( cl_dev_subdevice_id IN subdevice_id );

    // Execution commands interfaces
    cl_dev_err_code clDevCreateCommandList( cl_dev_cmd_list_props IN props, cl_dev_subdevice_id IN subdevice_id, cl_dev_cmd_list* OUT list );
    cl_dev_err_code clDevFlushCommandList( cl_dev_cmd_list IN list );
    cl_dev_err_code clDevReleaseCommandList( cl_dev_cmd_list IN list );
    cl_dev_err_code clDevCommandListExecute( cl_dev_cmd_list IN list, cl_dev_cmd_desc* IN *cmds, cl_uint IN count );
    cl_dev_err_code clDevCommandListWaitCompletion( cl_dev_cmd_list IN list, cl_dev_cmd_desc* IN cmdToWait );
    cl_dev_err_code clDevCommandListCancel( cl_dev_cmd_list IN list );
    cl_dev_err_code clDevReleaseCommand(cl_dev_cmd_desc* IN cmdToRelease );

    // Images interfaces
    cl_dev_err_code clDevGetSupportedImageFormats( cl_mem_flags IN flags, cl_mem_object_type IN imageType,
                            cl_uint IN numEntries, cl_image_format* OUT formats, cl_uint* OUT numEntriesRet ) const;

    // Memory interfaces
    cl_dev_err_code clDevGetMemoryAllocProperties( cl_mem_object_type IN memObjType, cl_dev_alloc_prop* OUT pAllocProp );
    cl_dev_err_code clDevCreateMemoryObject( cl_dev_subdevice_id IN node_id, cl_mem_flags IN flags, const cl_image_format* IN format,
                            size_t  IN dim_count, const size_t* IN dim_size, IOCLDevRTMemObjectService* IN pRTMemObjService,
                            IOCLDevMemoryObject* OUT *memObj );

    // Program and Kernel device build interfaces
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
    cl_dev_err_code clDevGetGlobalVariableTotalSize( cl_dev_program IN prog, size_t* OUT size );
    cl_dev_err_code clDevGetKernelInfo( cl_dev_kernel IN kernel, cl_dev_kernel_info IN param, size_t IN valueSize,
                            void* OUT value, size_t* OUT valueSizeRet );

    // Profiling interfaces
    cl_ulong        clDevGetPerformanceCounter();

    // Logger interfaces
    cl_dev_err_code clDevSetLogger(IOCLDevLogDescriptor *);

    void            clDevCloseDevice(void);

    const IOCLDeviceFECompilerDescription* clDevGetFECompilerDecription() const { return nullptr; };
    IOCLDevRawMemoryAllocator* clDevGetRawMemoryAllocator() { return this; };

    /// --- IOCLDevRawMemoryAllocator interfaces ---
    void* clDevAllocateRawMemory( size_t IN allocSize, size_t IN alignment );
    void  clDevFreeRawMemory( void* IN allocatedMemory );

protected:
    cl_uint                 m_uiIspId;
    IOCLDevLogDescriptor*   m_pLogDescriptor;
    cl_int                  m_iLogHandle;
    IOCLFrameworkCallbacks* m_pFrameworkCallbacks;
    CameraShim*             m_pCameraShim;
    ISPProgramService*      m_pProgramService;
    ISPMemoryAllocator*     m_pMemoryAllocator;
    ISPTaskDispatcher*      m_pTaskDispatcher;

    ~ISPDevice(); // deleting the object should be done by clDevCloseDevice only !

};

}}}
