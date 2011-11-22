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
#include "mic_dev_limits.h"
#include "mic_config.h"
#include "notification_port.h"
#include "cl_synch_objects.h"

#include <set>

using namespace std;

namespace Intel { namespace OpenCL { namespace MICDevice {

extern const cl_image_format suportedImageFormats[];
extern const unsigned int NUM_OF_SUPPORTED_IMAGE_FORMATS;

class ProgramService;
class MemoryAllocator;
class DeviceServiceCommunication;
class CommandList;

class MICDevice : public IOCLDeviceAgent, public IOCLDeviceFECompilerDescription
{

private:
    ProgramService*                 m_pProgramService;
    MemoryAllocator*                m_pMemoryAllocator;
    MICDeviceConfig*                m_pMICDeviceConfig;
    IOCLFrameworkCallbacks*         m_pFrameworkCallBacks;
    cl_uint                         m_uiMicId;      // internal MIC device ID
    cl_uint                         m_uiOclDevId;   // Runtime device ID
    IOCLDevLogDescriptor*           m_pLogDescriptor;
    cl_int                          m_iLogHandle;
    OclDynamicLib                   m_dlRunTime;
    cl_dev_cmd_list                 m_defaultCommandList;
    DeviceServiceCommunication*     m_pDeviceServiceComm;
    NotificationPort                m_notificationPort;

    set<CommandList*>               m_commandListsSet;

    // static set to handle all existing MIC DA instancies
    static set<IOCLDeviceAgent*>    m_mic_instancies;
    static OclMutex                 m_mic_instancies_mutex;

    // manage multiple DAs
    static void RegisterMicDevice( MICDevice* dev );
    static void UnregisterMicDevice( MICDevice* dev );

protected:
    ~MICDevice();

public:

    //
    // IOCLDeviceAgent
    //

    MICDevice(cl_uint devId, cl_uint micId, IOCLFrameworkCallbacks *devCallbacks, IOCLDevLogDescriptor *logDesc);
    cl_dev_err_code                     Init();

    const DeviceServiceCommunication&   GetDeviceService(void) const { return *m_pDeviceServiceComm; };

    static cl_dev_err_code   clDevGetDeviceInfo(cl_device_info IN param, size_t IN val_size, void* OUT paramVal, size_t* OUT param_val_size_ret);

    //Device Fission support

    cl_dev_err_code clDevPartition(  cl_dev_partition_prop IN props, cl_uint IN num_requested_subdevices, cl_dev_subdevice_id IN parent_id,
                                     cl_uint* INOUT num_subdevices, void* IN param, cl_dev_subdevice_id* OUT subdevice_ids);
    cl_dev_err_code clDevReleaseSubdevice(  cl_dev_subdevice_id IN subdevice_id);

    // Device entry points
    cl_dev_err_code clDevCreateCommandList( cl_dev_cmd_list_props IN props, cl_dev_subdevice_id IN subdevice_id, cl_dev_cmd_list* OUT list);
    cl_dev_err_code clDevFlushCommandList( cl_dev_cmd_list IN list);
    cl_dev_err_code clDevRetainCommandList( cl_dev_cmd_list IN list);
    cl_dev_err_code clDevReleaseCommandList( cl_dev_cmd_list IN list );
    cl_dev_err_code clDevCommandListExecute( cl_dev_cmd_list IN list, cl_dev_cmd_desc* IN *cmds, cl_uint IN count);
    cl_dev_err_code clDevCommandListWaitCompletion(cl_dev_cmd_list IN list);
    cl_dev_err_code clDevGetSupportedImageFormats( cl_mem_flags IN flags, cl_mem_object_type IN imageType,
                    cl_uint IN numEntries, cl_image_format* OUT formats, cl_uint* OUT numEntriesRet);
    cl_dev_err_code clDevGetMemoryAllocProperties( cl_mem_object_type IN memObjType,    cl_dev_alloc_prop* OUT pAllocProp );
    cl_dev_err_code clDevCreateMemoryObject( cl_dev_subdevice_id IN node_id, cl_mem_flags IN flags, const cl_image_format* IN format,
                    size_t IN dim_count, const size_t* IN dim_size,
                    IOCLDevRTMemObjectService* IN pBSService, IOCLDevMemoryObject* OUT *pMemObj );
    cl_dev_err_code clDevCheckProgramBinary( size_t IN binSize, const void* IN bin );
    cl_dev_err_code clDevCreateProgram( size_t IN binSize, const void* IN bin, cl_dev_binary_prop IN prop, cl_dev_program* OUT prog );
    cl_dev_err_code clDevBuildProgram( cl_dev_program IN prog, const char* IN options, void* IN userData);
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
    cl_ulong    clDevGetPerformanceCounter();
    cl_dev_err_code    clDevSetLogger(IOCLDevLogDescriptor *);
    const IOCLDeviceFECompilerDescription& clDevGetFECompilerDecription() const { return *this; };
    void        clDevCloseDevice(void);

    static void loadingInit();
    static void unloadRelease();

    // manage multiple DAs
    typedef set<MICDevice*>  TMicsSet;
    static TMicsSet FilterMicDevices( size_t count, const IOCLDeviceAgent* const *dev_arr );
    static TMicsSet GetActiveMicDevices( void );

    //
    // IOCLDeviceFECompilerDescription
    //
	const char* clDevFEModuleName() const;
	const void* clDevFEDeviceInfo() const;
	size_t		clDevFEDeviceInfoSize() const;

};

}}}
