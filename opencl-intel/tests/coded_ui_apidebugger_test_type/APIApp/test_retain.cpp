/******************************************************************
 //
 //  OpenCL Conformance Tests
 // 
 //  Copyright:	(c) 2008-2013 by Apple Inc. All Rights Reserved.
 //
 ******************************************************************/

#include "testBase.h"
#if !defined(_WIN32)
#include <unistd.h>
#endif // !_WIN32

// Note: According to spec, the various functions to get instance counts should return an error when passed in an object
// that has already been released. However, the spec is out of date. If it gets re-updated to allow such action, re-enable
// this define.
//#define VERIFY_AFTER_RELEASE	1

#define GET_QUEUE_INSTANCE_COUNT(p) numInstances = ( (err = clGetCommandQueueInfo(p, CL_QUEUE_REFERENCE_COUNT, sizeof( numInstances ), &numInstances, NULL)) == CL_SUCCESS ? numInstances : 0 )
#define GET_MEM_INSTANCE_COUNT(p) numInstances = ( (err = clGetMemObjectInfo(p, CL_MEM_REFERENCE_COUNT, sizeof( numInstances ), &numInstances, NULL)) == CL_SUCCESS ? numInstances : 0 )

#define VERIFY_INSTANCE_COUNT(c,rightValue) if( c != rightValue ) { \
log_error( "ERROR: Instance count for test object is not valid! (should be %d, really is %d)\n", rightValue, c ); \
return -1;	}

int test_retain_queue_multiple(cl_device_id deviceID, cl_context context, cl_command_queue queueNotUsed, int num_elements)
{
	cl_command_queue queue;
	unsigned int numInstances, i;
	int err;
	
	
	/* Create a test program */
	queue = clCreateCommandQueue( context, NULL, 0, &err );	
	queue = clCreateCommandQueue( context, deviceID, 0, &err );	
	test_error( err, "Unable to create command queue to test with" );
	
	/* Increment 9 times, which should bring the count to 10 */
	err = clRetainCommandQueue( NULL );
	err = clRetainCommandQueue( queue );
	for( i = 0; i < 8; i++ )
	{
		clRetainCommandQueue( queue );
	}
	
	/* Test the instance count */
	GET_QUEUE_INSTANCE_COUNT( queue );
	test_error( err, "Unable to get queue instance count" );
	VERIFY_INSTANCE_COUNT( numInstances, 10 );
	
	/* Now release 5 times, which should take us to 5 */
	for( i = 0; i < 5; i++ )
	{
		clReleaseCommandQueue( queue );
	}

	GET_QUEUE_INSTANCE_COUNT( queue );
	test_error( err, "Unable to get queue instance count" );
	VERIFY_INSTANCE_COUNT( numInstances, 5 );
	
	/* Retain again three times, which should take us to 8 */
	for( i = 0; i < 3; i++ )
	{
		clRetainCommandQueue( queue );
	}
	
	GET_QUEUE_INSTANCE_COUNT( queue );
	test_error( err, "Unable to get queue instance count" );
	VERIFY_INSTANCE_COUNT( numInstances, 8 );
	
	/* Release 7 times, which should take it to 1 */
	for( i = 0; i < 7; i++ )
	{
		clReleaseCommandQueue( queue );
	}
	
	GET_QUEUE_INSTANCE_COUNT( queue );
	test_error( err, "Unable to get queue instance count" );
	VERIFY_INSTANCE_COUNT( numInstances, 1 );
	
	/* And one last one */
	err = clReleaseCommandQueue( NULL );
	err = clReleaseCommandQueue( queue );
	
#ifdef VERIFY_AFTER_RELEASE	
	/* We're not allowed to get the instance count after the object has been completely released. But that's
	 exactly how we can tell the release worked--by making sure getting the instance count fails! */
	GET_QUEUE_INSTANCE_COUNT( queue );
	if( err != CL_INVALID_COMMAND_QUEUE )
	{
		print_error( err, "Command queue was not properly released" );
		return -1;
	}
#endif
	
	return 0;
}


int test_retain_mem_object_multiple(cl_device_id deviceID, cl_context context, cl_command_queue queue, int num_elements)
{
	cl_mem object;
	unsigned int numInstances, i;
	int err;
	
	
	/* Create a test object */
	object = clCreateBuffer( context, CL_MEM_READ_ONLY, 32, NULL, &err );
	test_error( err, "Unable to create buffer to test with" );
	
	/* Increment 9 times, which should bring the count to 10 */
	err = clRetainMemObject( NULL );
	err = clRetainMemObject( object );
	for( i = 0; i < 8; i++ )
	{
		clRetainMemObject( object );
	}
	
	/* Test the instance count */
	GET_MEM_INSTANCE_COUNT( object );
	test_error( err, "Unable to get mem object count" );
	VERIFY_INSTANCE_COUNT( numInstances, 10 );
	
	/* Now release 5 times, which should take us to 5 */
	for( i = 0; i < 5; i++ )
	{
		clReleaseMemObject( object );
	}
	
	GET_MEM_INSTANCE_COUNT( object );
	test_error( err, "Unable to get mem object count" );
	VERIFY_INSTANCE_COUNT( numInstances, 5 );

	/* Retain again three times, which should take us to 8 */
	for( i = 0; i < 3; i++ )
	{
		clRetainMemObject( object );
	}
	
	GET_MEM_INSTANCE_COUNT( object );
	test_error( err, "Unable to get mem object count" );
	VERIFY_INSTANCE_COUNT( numInstances, 8 );
	
	/* Release 7 times, which should take it to 1 */
	for( i = 0; i < 7; i++ )
	{
		clReleaseMemObject( object );
	}
	
	GET_MEM_INSTANCE_COUNT( object );
	test_error( err, "Unable to get mem object count" );
	VERIFY_INSTANCE_COUNT( numInstances, 1 );
	
	/* And one last one */
	clReleaseMemObject( object );
	
#ifdef VERIFY_AFTER_RELEASE	
	/* We're not allowed to get the instance count after the object has been completely released. But that's
	 exactly how we can tell the release worked--by making sure getting the instance count fails! */
	GET_MEM_INSTANCE_COUNT( object );
	if( err != CL_INVALID_MEM_OBJECT )
	{
		print_error( err, "Mem object was not properly released" );
		return -1;
	}
#endif
	
	return 0;
}

