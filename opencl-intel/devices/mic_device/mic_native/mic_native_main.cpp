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
#include "memory_manager.h"
#include "thread_local_storage.h"
#include "execution_task.h"

#include <sink/COIPipeline_sink.h>
#include <sink/COIProcess_sink.h>

using namespace Intel::OpenCL::MICDevice;
using namespace Intel::OpenCL::MICDeviceNative;

// main is automatically called whenever the source creates a process.
// In order for the source to create and use pipelines, the process must
// still exist.
// However, once main exits, the process that was created exits.
// Thus, main must live until there is no more work to be done.
int main(int , char**)
{
    NATIVE_PRINTF("main called on the  sink\n");

    // init device
    TlsAccessor::tls_initialize();
    MemoryManager::createMemoryManager();

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
    MemoryManager::releaseMemoryManager();
    TlsAccessor::tls_finalize();

    NATIVE_PRINTF("main shut down on the  sink\n");

    return 0;
}
