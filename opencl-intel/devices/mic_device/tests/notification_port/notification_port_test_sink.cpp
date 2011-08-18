/** ************************************************************************ *\
                  INTEL CORPORATION PROPRIETARY INFORMATION
      This software is supplied under the terms of a license agreement or
      nondisclosure agreement with Intel Corporation and may not be copied
      or disclosed except in accordance with the terms of that agreement.
          Copyright (C) 2011 Intel Corporation. All Rights Reserved.
\* ************************************************************************* */


#include <stdio.h>
#include <unistd.h>
#include <cstdlib>
#include <malloc.h>
#include <pthread.h>

#include <sink/COIPipeline_sink.h>
#include <sink/COIProcess_sink.h>
#include <common/COIMacros_common.h>
#include <common/COISysInfo_common.h>
#include <common/COIEvent_common.h>

pthread_t* gThreads = NULL;
COIEVENT* gBarrier = NULL;

// main is automatically called whenever the source creates a process.
// However, once main exits, the process that was created exits.
int main(int argc, char** argv)
{
    COIRESULT result;
    UNREFERENCED_PARAM (argc);
    UNREFERENCED_PARAM (argv);

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
	
	if (gBarrier)
	{
	    free(gBarrier);
		gBarrier = NULL;
	}
	if (gThreads)
	{
	    free(gThreads);
		gThreads = NULL;
	}

    return 0;
}

// Prototype of run function that can be retrieved on the source side
// Copies Misc Data to Return Pointer
COINATIVELIBEXPORT
void DelayFunction (    uint32_t         in_BufferCount,
              void**           in_ppBufferPointers,
              uint64_t*        in_pBufferLengths,
              void*            in_pMiscData,
              uint16_t         in_MiscDataLength,
              void*            in_pReturnValue,
              uint16_t         in_ReturnValueLength)
{

    UNREFERENCED_PARAM(in_BufferCount);
    UNREFERENCED_PARAM(in_ppBufferPointers);
    UNREFERENCED_PARAM(in_pBufferLengths);
    UNREFERENCED_PARAM(in_pMiscData);
    UNREFERENCED_PARAM(in_MiscDataLength);

	COIEVENT* barrier = (COIEVENT*)in_pMiscData;

	usleep(rand() % 10000);

	COIRESULT result = COIEventSignalUserEvent(*barrier);

}


unsigned int counter = 0;

void* delayThreadEntry(void* arg)
{
	unsigned int curr = __sync_add_and_fetch(&counter, 1);
	
	COIEVENT* barrier = (COIEVENT*)arg;

	usleep(rand() % 10000);

	COIRESULT result = COIEventSignalUserEvent(*barrier);
	if (result != COI_SUCCESS)
	{
	    printf("FAIL fire index %d, barrier %ld %ld\n", curr, barrier->opaque[0], barrier->opaque[1]);
		fflush(0);
	}
	return NULL;
}

// Prototype of run function that can be retrieved on the source side
// Copies Misc Data to Return Pointer
COINATIVELIBEXPORT
void DelayFunctionMultiThreaded(    uint32_t         in_BufferCount,
              void**           in_ppBufferPointers,
              uint64_t*        in_pBufferLengths,
              void*            in_pMiscData,
              uint16_t         in_MiscDataLength,
              void*            in_pReturnValue,
              uint16_t         in_ReturnValueLength)
{

    UNREFERENCED_PARAM(in_BufferCount);
    UNREFERENCED_PARAM(in_ppBufferPointers);
    UNREFERENCED_PARAM(in_pBufferLengths);
    UNREFERENCED_PARAM(in_pMiscData);
    UNREFERENCED_PARAM(in_MiscDataLength);
	
	printf("Enter DelayFunctionMultiThreaded\n");
	fflush(0);
	
	COIEVENT* barrier = (COIEVENT*)in_pMiscData;
	assert(barrier);

	unsigned int numOfBarriers = in_MiscDataLength / sizeof(COIEVENT);
	
	gBarrier = (COIEVENT*)malloc(sizeof(COIEVENT) * numOfBarriers);
	assert(gBarrier);
	
	pthread_attr_t tattr;
	
	pthread_attr_init(&tattr);
	pthread_attr_setdetachstate(&tattr, PTHREAD_CREATE_DETACHED);
	
	gThreads = (pthread_t*)malloc(sizeof(pthread_t) * numOfBarriers);
	assert(gThreads);

	int err = 0;
	for (unsigned int i = 0; i < numOfBarriers; i++)
	{
	    gBarrier[i] = barrier[i];
		err = pthread_create(&gThreads[i], &tattr, delayThreadEntry, &gBarrier[i]);
		if (err != 0)
		{
			printf("thread creation failed on device\n");
			fflush(0);
		}
		assert(err == 0);
	}
	pthread_attr_destroy(&tattr);
}
