/******************************************************************
//
//  OpenCL Conformance Tests
//
//  Copyright:  (c) 2008-2009 by Apple Inc. All Rights Reserved.
//
******************************************************************/

#ifndef _threadTesting_h
#define _threadTesting_h

#ifdef __APPLE__
    #include <OpenCL/opencl.h>
#else
    #include <CL/cl.h>
#endif

#define TEST_NOT_IMPLEMENTED        -99

typedef int (*basefn)(cl_device_id deviceID, cl_context context, cl_command_queue queue, int num_elements);
extern int test_threaded_function( basefn fnToTest, cl_device_id device, cl_context context, cl_command_queue queue, int numElements );

#endif // _threadTesting_h


