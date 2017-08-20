/* ************************************************************************* *\
#               INTEL CORPORATION PROPRIETARY INFORMATION
#     This software is supplied under the terms of a license agreement or
#     nondisclosure agreement with Intel Corporation and may not be copied
#     or disclosed except in accordance with the terms of that agreement.
#        Copyright (C) 2011 Intel Corporation. All Rights Reserved.
#\* ************************************************************************* */

#pragma once

#include <vector>

#include "cl_device_api.h"
#include "mic_config.h"
#include "cl_thread.h"

#include <common/COITypes_common.h>
#include <source/COIPipeline_source.h>


namespace Intel { namespace OpenCL { namespace MICDevice {

class DeviceServiceCommunication : private OclThread
{

public:

    /* Factory which create new DeviceServiceCommunication object and set it to *ppDeviceServiceCom
       The initialization of the device process and pipeline is performed on separate thread.
       return CL_DEV_SUCCESS if succeeded. */
    static cl_dev_err_code deviceSeviceCommunicationFactory(unsigned int uiMicId, 
                                                            DeviceServiceCommunication** ppDeviceServiceCom);

    virtual ~DeviceServiceCommunication();

    /* Return the created process. (If the creation succeeded, otherwise return NULL)
       If the created thread (in factory) didn't finish, it will wait until the thread will finish it's work. */
    COIPROCESS getDeviceProcessHandle();

    /* Supported Device-side functions */
    enum DEVICE_SIDE_FUNCTION
    {
        GET_BACKEND_TARGET_DESCRIPTION_SIZE = 0,
        GET_BACKEND_TARGET_DESCRIPTION,
        COPY_PROGRAM_TO_DEVICE,
        CREATE_BUILT_IN_PROGRAM,
        REMOVE_PROGRAM_FROM_DEVICE,

        EXECUTE_NDRANGE,
        EXECUTE_NATIVE_KERNEL,
        INIT_DEVICE,
        RELEASE_DEVICE,
        INIT_COMMANDS_QUEUE,
        RELEASE_COMMANDS_QUEUE,
        EXECUTE_DEVICE_UTILITY,

        FILL_MEM_OBJECT,

#ifdef ENABLE_MIC_TRACER
        GET_TRACE_SIZE, 
        GET_TRACE,
#endif

        // insert new function ids before this line
        LAST_DEVICE_SIDE_FUNCTION,
        DEVICE_SIDE_FUNCTION_COUNT = LAST_DEVICE_SIDE_FUNCTION // used as a count of functions
    };

    COIFUNCTION getDeviceFunction( DEVICE_SIDE_FUNCTION id );

    /* Send function to run through the service pipeline unless use_pipeline is provided
       It is block operation - wait until the function completed on the device.
       func - COIFunction to run.
       input_data - input raw data to be send to the function without using COI Buffers
       output_data - output raw data to be returned from function without using COI Buffers
       numBuffers - the amount of COI buffers to send.
       buffers - the COI buffers to send.
       bufferAccessFlags - the buffers access flags.
       If the created thread (in factory) didn't finish, it will wait until the thread will finish it's work.
       Return true if suceeeded. */
    bool runServiceFunction(DEVICE_SIDE_FUNCTION func,
                            size_t input_data_size, const void* input_data,
                            size_t output_data_size, void* output_data,
                            unsigned int numBuffers, const COIBUFFER* buffers, 
                            const COI_ACCESS_FLAGS* bufferAccessFlags,
                            COIPIPELINE use_pipeline = nullptr );

    unsigned int GetNumActiveThreads() const { return m_uiNumActiveThreads;}
private:

    // private constructor in order to use factory only
    DeviceServiceCommunication(unsigned int uiMicId);

    RETURN_TYPE_ENTRY_POINT Run();

    /* close the service pipeline and the process on the device.
       If the created thread (in factory) didn't finish, it will wait until the thread will finish it's work.
       Called from destructor only*/
    void freeDevice(bool releaseCoiObjects = true);

    /* Entry point for the initializer thread. */
    static void* initEntryPoint(void* arg);
    
    void waitForInitialization()
    {
        if (true == m_initCompleted)
        {
            return;
        }
        WaitForCompletion();
    }
    
    /* Set in additionalEnvVars all the VTune env variables (Those starts with __OCL_MIC_INTEL_) as name=value. */
    void getVTuneEnvVars(vector<char*>& additionalEnvVars);

    /* setup COI device buffer cache memory pools */
    bool setupBufferMemoryPools( const MICDeviceConfig& tMicConfig );

    unsigned int    m_uiMicId;

    COIPROCESS      m_process;
    COIPIPELINE     m_pipe;

    unsigned int m_uiNumActiveThreads;
    
    volatile bool m_initCompleted;

    static const char* const m_device_function_names[DEVICE_SIDE_FUNCTION_COUNT];
    COIFUNCTION m_device_functions[DEVICE_SIDE_FUNCTION_COUNT];
};

}}}
