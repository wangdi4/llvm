/******************************************************************
//
//  OpenCL Conformance Tests
// 
//  Copyright:	(c) 2008-2013 by Apple Inc. All Rights Reserved.
//
******************************************************************/

#include "testBase.h"

#if ! defined( _WIN32 )
	#include "unistd.h" // for "sleep" used in the "while (1)" busy wait loop in 
#endif
// test_event_flush

const char *sample_long_test_kernel[] = {
"__kernel void sample_test(__global float *src, __global int *dst)\n"
"{\n"
"    int  tid = get_global_id(0);\n"
"	 int  i;\n"
"\n"
"    for( i = 0; i < 10000; i++ )\n"
"    {\n"
"        dst[tid] = (int)src[tid] * 3;\n"
"    }\n"
"\n"
"}\n" };

int create_and_execute_kernel( cl_context inContext, cl_command_queue inQueue, cl_program *outProgram, cl_kernel *outKernel, cl_mem *streams, 
							  unsigned int lineCount, const char **lines, const char *kernelName, cl_event *outEvent )
{
	size_t threads[1] = { 1000 }, localThreads[1];
	int error;
	
	if( create_single_kernel_helper( inContext, outProgram, outKernel, lineCount, lines, kernelName ) )
	{
		return -1;
	}
	
	error = get_max_common_work_group_size( inContext, *outKernel, threads[0], &localThreads[0] );
	test_error( error, "Unable to get work group size to use" );
	
	streams[0] = clCreateBuffer(inContext, (cl_mem_flags)(CL_MEM_READ_WRITE),  sizeof(cl_float) * 1000, NULL, &error);
	test_error( error, "Creating test array failed" );
	streams[1] = clCreateBuffer(inContext, (cl_mem_flags)(CL_MEM_READ_WRITE),  sizeof(cl_int) * 1000, NULL, &error);
	test_error( error, "Creating test array failed" );
	
	/* Set the arguments */
    error = clSetKernelArg( *outKernel, 0, sizeof( streams[0] ), &streams[0] );
    test_error( error, "Unable to set kernel arguments" );
    error = clSetKernelArg( *outKernel, 1, sizeof( streams[1] ), &streams[1] );
    test_error( error, "Unable to set kernel arguments" );
	
    error = clEnqueueNDRangeKernel(inQueue, *outKernel, 1, NULL, threads, localThreads, 0, NULL, outEvent);
	test_error( error, "Unable to execute test kernel" );
	
	return 0;
}

#define SETUP_EVENT( c, q ) \
clProgramWrapper program; \
clKernelWrapper kernel; \
clMemWrapper streams[2]; \
clEventWrapper event; \
int error; \
if( create_and_execute_kernel( c, q, &program, &kernel, &streams[0], 1, sample_long_test_kernel, "sample_test", &event ) ) return -1;

#define FINISH_EVENT(_q) clFinish(_q)

const char *IGetStatusString( cl_int status )
{
	static char tempString[ 128 ];
	switch( status )
	{
		case CL_COMPLETE:	return "CL_COMPLETE"; 
		case CL_RUNNING:	return "CL_RUNNING";
		case CL_QUEUED:		return "CL_QUEUED";
		case CL_SUBMITTED:	return "CL_SUBMITTED";
		default:			
			sprintf( tempString, "<unknown: %d>", (int)status );
			return tempString;
	}
}

int test_event_flush( cl_device_id deviceID, cl_context context, cl_command_queue queue, int num_elements)
{
    int loopCount = 0;
	cl_int status;
	SETUP_EVENT( context, queue );
	
	/* Now flush. Note that we can't guarantee this actually lets the op finish, but we can guarantee it's no longer queued */
	error = clFlush( NULL );
	error = clFlush( queue );
	test_error( error, "Unable to flush events" );
	
	/* Make sure it worked */
         while (1) {
	    error = clGetEventInfo( event, CL_EVENT_COMMAND_EXECUTION_STATUS,
                                                                sizeof( status ), &status, NULL );
	test_error( error, "Calling clGetEventStatus didn't work!" );	
	
	    if( status != CL_QUEUED )
                  break;

#if ! defined( _WIN32 )
	    sleep(1); // give it some time here.
#else // _WIN32
            Sleep(1000);
#endif
	    ++loopCount;
          }
	
/*
CL_QUEUED (command has been enqueued in the command-queue),
CL_SUBMITTED (enqueued command has been submitted by the host to the device associated with the command-queue),
CL_RUNNING (device is currently executing this command),
CL_COMPLETE (the command has completed), or
Error code given by a negative integer value. (command was abnormally terminated – this may be caused by a bad memory access etc.).
*/
	 if(status != CL_COMPLETE && status != CL_SUBMITTED && 
	    status != CL_RUNNING && status != CL_COMPLETE)
	{
		log_error( "ERROR: Incorrect status returned from clGetErrorStatus after event flush (%d:%s)\n", status, IGetStatusString( status ) );
		return -1;
	}
	
	/* Now wait */
	error = clFinish( queue );
	test_error( error, "Unable to finish events" );
	
	FINISH_EVENT(queue);
	return 0;
}



#define NUM_EVENT_RUNS 100

int test_event_enqueue_marker( cl_device_id deviceID, cl_context context, cl_command_queue queue, int num_elements)
{
	cl_int status;
	SETUP_EVENT( context, queue );
	
	/* Now we queue a marker and wait for that, which--since it queues afterwards--should guarantee the execute finishes too */
	clEventWrapper markerEvent;
	//error = clEnqueueMarker( queue, &markerEvent );

#ifdef CL_VERSION_1_2
    error = clEnqueueMarkerWithWaitList(NULL, 0, NULL, &markerEvent );
    error = clEnqueueMarkerWithWaitList(queue, 0, NULL, &markerEvent );
#else      
    error = clEnqueueMarker( queue, &markerEvent );
#endif
   	test_error( error, "Unable to queue marker" );
    /* Now we wait for it to be done, then test the status again */
	error = clWaitForEvents( 1, &markerEvent );     
	test_error( error, "Unable to wait for marker event" );
	
	/* Check the status of the first event */
	error = clGetEventInfo( event, CL_EVENT_COMMAND_EXECUTION_STATUS, sizeof( status ), &status, NULL );
	test_error( error, "Calling clGetEventInfo didn't work!" );	
	if( status != CL_COMPLETE )
	{
		log_error( "ERROR: Incorrect status returned from clGetEventInfo after event complete (%d:%s)\n", status, IGetStatusString( status ) );
		return -1;
	}
	
	FINISH_EVENT(queue);
	return 0;	
}

#ifdef CL_VERSION_1_2
int test_event_enqueue_barrier_with_list( cl_device_id deviceID, cl_context context, cl_command_queue queue, int num_elements)
{
    
    cl_int status;
	SETUP_EVENT( context, queue );
	cl_event event_list[3]={ NULL, NULL, NULL};
    
    size_t threads[1] = { 10 }, localThreads[1]={1};
    cl_uint event_count=2;  
    error= clEnqueueNDRangeKernel( queue,kernel,1,NULL, threads, localThreads, 0, NULL, &event_list[0]);
    test_error( error, " clEnqueueBarrierWithWaitList   1 " );
    
    error= clEnqueueNDRangeKernel( queue,kernel,1,NULL, threads, localThreads, 0, NULL, &event_list[1]);
    test_error( error, " clEnqueueBarrierWithWaitList 2" );
    
    error= clEnqueueNDRangeKernel( queue,kernel,1,NULL, threads, localThreads, 0, NULL, NULL);
    test_error( error, " clEnqueueBarrierWithWaitList  20" );
    
    // test the case event returned 
    error =clEnqueueBarrierWithWaitList(queue, event_count, NULL,  &event_list[2]);
    error =clEnqueueBarrierWithWaitList(queue, event_count, event_list,  &event_list[2]);
    test_error( error, " clEnqueueBarrierWithWaitList " );
    
    clReleaseEvent(event_list[0]);
    clReleaseEvent(event_list[1]);

    error= clEnqueueNDRangeKernel( queue,kernel,1,NULL, threads, localThreads, 0, NULL, &event_list[0]);
    test_error( error, " clEnqueueBarrierWithWaitList   1 " );
    
    error= clEnqueueNDRangeKernel( queue,kernel,1,NULL, threads, localThreads, 0, NULL, &event_list[1]);
    test_error( error, " clEnqueueBarrierWithWaitList 2" );

    // test the case event =NULL,   caused [CL_INVALID_VALUE] : OpenCL Error : clEnqueueMarkerWithWaitList failed: event is a NULL value
    error = clEnqueueBarrierWithWaitList(queue, event_count, event_list,  NULL);
    test_error( error, " clEnqueueBarrierWithWaitList " );    
    
    clReleaseEvent(event_list[0]);
    clReleaseEvent(event_list[1]);
    clReleaseEvent(event_list[2]);
	
	FINISH_EVENT(queue);
	return 0;	
}
#endif
