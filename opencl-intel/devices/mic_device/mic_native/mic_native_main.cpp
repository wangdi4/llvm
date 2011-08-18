/** ************************************************************************ *\
                  INTEL CORPORATION PROPRIETARY INFORMATION
      This software is supplied under the terms of a license agreement or
      nondisclosure agreement with Intel Corporation and may not be copied
      or disclosed except in accordance with the terms of that agreement.
          Copyright (C) 2007 - 2010 Intel Corporation. All Rights Reserved.
\* ************************************************************************* */


#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>
#include <assert.h>
#include <stdlib.h>

#include "native_common_macros.h"
#include "native_program_service.h"
#include "memory_manager.h"
#include "thread_local_storage.h"
#include "execution_task.h"

#include <sink/COIPipeline_sink.h>
#include <sink/COIProcess_sink.h>

using namespace Intel::OpenCL::MICDeviceNative;

#define MIC_MAX_PARAMETER_SIZE            1024
struct packed_params{
    size_t region[3];
    unsigned int dimCount;
    unsigned int kernel_size;
    unsigned int bufs_count;
    unsigned int bufs_offsets[ MIC_MAX_PARAMETER_SIZE / sizeof( unsigned int ) ];
    char         locked_params[ MIC_MAX_PARAMETER_SIZE ];
};

typedef void (*pFunc)(int idx, int* in, int* out);

COINATIVELIBEXPORT
void hello_world(uint32_t         in_BufferCount,
              void**           in_ppBufferPointers,
              uint64_t*        in_pBufferLengths,
              void*            in_pMiscData,
              uint16_t         in_MiscDataLength,
              void*            in_pReturnValue,
              uint16_t         in_ReturnValueLength)
{

    char         locked_params[ MIC_MAX_PARAMETER_SIZE ];
    pFunc          func;
    int            oo;

    NATIVE_PRINTF("Enter SINK worker\n");

    if ((NULL == in_pMiscData) || (sizeof(packed_params) != in_MiscDataLength))
    {
        NATIVE_PRINTF("SINK: no in_pMiscData were passed!\n");
        return;
    }

    packed_params* params = (packed_params*)in_pMiscData;

    if (in_BufferCount != params->bufs_count+1)
    {
        NATIVE_PRINTF("SINK: different buff count: COI=%d my=%d!\n", in_BufferCount, params->bufs_count );
        return;
    }

    memcpy( locked_params, params->locked_params, MIC_MAX_PARAMETER_SIZE );

    for (unsigned int i = 0; i < params->bufs_count; ++i)
    {
        *((void**)(locked_params+params->bufs_offsets[i])) = in_ppBufferPointers[i+1];
    }

    NATIVE_PRINTF("KernelBuf = %p\n", in_ppBufferPointers[0]);
    NATIVE_PRINTF("Buffer0 = %p\n", in_ppBufferPointers[1]);
    NATIVE_PRINTF("Buffer1 = %p\n", in_ppBufferPointers[2]);

    NATIVE_PRINTF("dimCount = %d\n", params->dimCount);
    for (unsigned int i = 0; i < 3; ++i)
    {
        NATIVE_PRINTF("region[%d] = %d\n", i, (int)params->region[i]) ;
    }

    func = (pFunc)in_ppBufferPointers[0];
    oo = mprotect( (void*)func, params->kernel_size, PROT_EXEC|PROT_READ );
    if (0 != oo)
    {
        NATIVE_PRINTF("mprotect returned %d\n", oo);
        perror(NULL);
        return;
    }

    int* in  = (int*)in_ppBufferPointers[1];
    int* out = (int*)in_ppBufferPointers[2];

    for (unsigned int i = 0; i < params->region[0]; ++i)
    {
        //out[i] = in[i] + 1;
        //NATIVE_PRINTF("in[%d] = %d  out[%d] = %d \n", i, in[i], i, out[i] );
        func(i, in, out);
    }

    oo = mprotect( (void*)func, params->kernel_size, PROT_READ );
    if (0 != oo)
    {
        NATIVE_PRINTF("mprotect returned %d\n", oo);
        perror(NULL);
        return;
    }

    return;

}

COINATIVELIBEXPORT
void execute_NDRange(uint32_t         in_BufferCount,
              void**           in_ppBufferPointers,
              uint64_t*        in_pBufferLengths,
              void*            in_pMiscData,
              uint16_t         in_MiscDataLength,
              void*            in_pReturnValue,
              uint16_t         in_ReturnValueLength)
{
	NATIVE_PRINTF("Enter execute_NDRange\n");
	assert(in_BufferCount >= 1 && "Should be at least One buffer");
	assert(in_pBufferLengths[0] >= sizeof(dispatcher_data) && "in_pBufferLengths[0] should be at least as the size of dispatcher_data");
	dispatcher_data* tDispatcherData = (dispatcher_data*)(in_ppBufferPointers[0]);
	// DO NOT delete this object, It will delete itself after kernel execution
	ExecutionTask* exeTask = ExecutionTask::ExecutionTaskFactory(tDispatcherData);
	exeTask->init(in_BufferCount, in_ppBufferPointers, in_pBufferLengths);
	exeTask->runTask();
	NATIVE_PRINTF("Exit execute_NDRange\n");
}

// main is automatically called whenever the source creates a process.
// In order for the source to create and use pipelines, the process must
// still exist.
// However, once main exits, the process that was created exits.
// Thus, main must live until there is no more work to be done.
int main(int , char**)
{
    NATIVE_PRINTF("AAAAAAAAAAAAAAAAAAAAAAAAA main called on the  sink\n");

    // init device
    TlsAccessor::tls_initialize();
    MemoryManager::createMemoryManager();
    ProgramService::createProgramService();


    COIRESULT result;

    // Functions enqueued on the sink side will not start executing until
    // you call COIPipelineStartExecutingRunFunctions()
    // This call is to synchronize any initialization required on the sink side

    result = COIPipelineStartExecutingRunFunctions();

    assert(result == COI_SUCCESS);

    //    This call will wait till COIProcessDestroy() gets called on the source side
    //    If COIProcessDestroy is called without force flag set, this call will make
    //    sure all the functions enqueued are executed and does all clean up required to
    //    exit gracefully.

    COIProcessWaitForShutdown();

    // shutdown
    ProgramService::releaseProgramService();
    MemoryManager::releaseMemoryManager();
    TlsAccessor::tls_finalize();

    NATIVE_PRINTF("main shut down on the  sink\n");

    return 0;
}
