/******************************************************************
 //
 //  OpenCL Conformance Tests
 // 
 //  Copyright:	(c) 2008-2013 by Apple Inc. All Rights Reserved.
 //
 ******************************************************************/

#include "action_classes.h"

#pragma mark -------------------- Base Action Class -------------------------

#pragma mark -------------------- Execution Sub-Classes -------------------------


#pragma mark -------------------- Buffer Sub-Classes -------------------------

cl_int BufferAction::Setup( cl_device_id device, cl_context context, cl_command_queue queue, bool allocate )
{
	cl_int error;
	cl_ulong maxAllocSize;
	
	
	// Get the largest possible buffer we could allocate
	error = clGetDeviceInfo( device, CL_DEVICE_MAX_MEM_ALLOC_SIZE, sizeof( maxAllocSize ), &maxAllocSize, NULL );
	
	// Don't create a buffer quite that big, just so we have some space left over for other work
	mSize = (size_t)( maxAllocSize / 5 );
	
	// Cap at 128M so tests complete in a reasonable amount of time.
	if (mSize > 128 << 20)
		mSize = 128 << 20;
	
	mSize /=2;
	
	log_info("\tBuffer size: %gMB\n", (double)mSize/(1024.0*1024.0));
	
	mBuffer = clCreateBuffer( context, CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR, mSize, NULL, &error );
	test_error( error, "Unable to create buffer to test against" );
	
	mOutBuffer = malloc( mSize );
	if( mOutBuffer == NULL )
	{
		log_error( "ERROR: Unable to allocate temp buffer (out of memory)\n" );
		return CL_OUT_OF_RESOURCES;
	}
	
	return CL_SUCCESS;
}

cl_int ReadBufferAction::Setup( cl_device_id device, cl_context context, cl_command_queue queue )
{
	return BufferAction::Setup( device, context, queue, true );
}

cl_int	ReadBufferAction::Execute( cl_command_queue queue, cl_uint numWaits, cl_event *waits, cl_event *outEvent )
{
	cl_int error = clEnqueueReadBuffer( queue, mBuffer, CL_FALSE, 0, mSize, mOutBuffer, numWaits, waits, outEvent );
	test_error( error, "Unable to enqueue buffer read" );
	
	return CL_SUCCESS;
}

