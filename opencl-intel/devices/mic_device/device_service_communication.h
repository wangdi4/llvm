/* ************************************************************************* *\
#               INTEL CORPORATION PROPRIETARY INFORMATION
#     This software is supplied under the terms of a license agreement or
#     nondisclosure agreement with Intel Corporation and may not be copied
#     or disclosed except in accordance with the terms of that agreement.
#        Copyright (C) 2011 Intel Corporation. All Rights Reserved.
#\* ************************************************************************* */

#pragma once

#include <pthread.h>

#include "cl_device_api.h"

#include <common/COITypes_common.h>
#include <source/COIPipeline_source.h>


namespace Intel { namespace OpenCL { namespace MICDevice {

class DeviceServiceCommunication
{

public:

    /* Factory which create new DeviceServiceCommunication object and set it to *ppDeviceServiceCom
       The initialization of the device process and pipeline is performed on separate thread.
       return CL_DEV_SUCCESS if succeeded. */
    static cl_dev_err_code deviceSeviceCommunicationFactory(unsigned int uiMicId, DeviceServiceCommunication** ppDeviceServiceCom);

    virtual ~DeviceServiceCommunication();

    /* Return the created process. (If the creation succeeded, otherwise return NULL)
       If the created thread (in factory) didn't finish, it will wait until the thread will finish it's work. */
    COIPROCESS getDeviceProcessHandle() const;

    /* Supported Device-side functions */
    enum DEVICE_SIDE_FUNCTION
    {
        GET_BACKEND_TARGET_DESCRIPTION_SIZE = 0,
        GET_BACKEND_TARGET_DESCRIPTION,
        COPY_PROGRAM_TO_DEVICE,
        REMOVE_PROGRAM_FROM_DEVICE,

		EXECUTE_NDRANGE,

        // insert new function ids before this line
        LAST_DEVICE_SIDE_FUNCTION,
        DEVICE_SIDE_FUNCTION_COUNT = LAST_DEVICE_SIDE_FUNCTION // used as a count of functions
    };

    COIFUNCTION getDeviceFunction( DEVICE_SIDE_FUNCTION id ) const;

    /* Send function to run through the service pipeline.
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
                            size_t input_data_size, void* input_data,
                            size_t output_data_size, void* output_data,
                            unsigned int numBuffers, const COIBUFFER* buffers, const COI_ACCESS_FLAGS* bufferAccessFlags);

private:

    // private constructor in order to use factory only
    DeviceServiceCommunication(unsigned int uiMicId);

    /* close the service pipeline and the process on the device.
       If the created thread (in factory) didn't finish, it will wait until the thread will finish it's work.
       Called from destructor only*/
    void freeDevice(bool releaseCoiObjects = true);

    /* Entry point for the initializer thread. */
    static void* initEntryPoint(void* arg);

    /* The calling thread will wait on this function until the initializer thread will finish it's work.
       Unless it already finished. */
    void waitForInitThread() const;

    unsigned int m_uiMicId;

    COIPROCESS m_process;
    COIPIPELINE m_pipe;

    static const char* const m_device_function_names[DEVICE_SIDE_FUNCTION_COUNT];
    COIFUNCTION m_device_functions[DEVICE_SIDE_FUNCTION_COUNT];

    volatile bool m_initDone;

    pthread_t m_initializerThread;
    mutable pthread_mutex_t m_mutex;
    mutable pthread_cond_t m_cond;

};

}}}
