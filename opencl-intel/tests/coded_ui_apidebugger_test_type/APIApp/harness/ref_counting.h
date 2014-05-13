/******************************************************************
 //
 //  OpenCL Conformance Tests
 // 
 //  Copyright:	(c) 2008-2013 by Apple Inc. All Rights Reserved.
 //
 ******************************************************************/

#ifndef _ref_counting_h
#define _ref_counting_h

#define MARK_REF_COUNT_BASE( c, type, bigType ) \
	cl_uint c##_refCount; \
	error = clGet##type##Info( c, CL_##bigType##_REFERENCE_COUNT, sizeof( c##_refCount ), &c##_refCount, NULL ); \
	test_error( error, "Unable to check reference count for " #type );

#define TEST_REF_COUNT_BASE( c, type, bigType ) \
	cl_uint c##_refCount_new; \
	error = clGet##type##Info( c, CL_##bigType##_REFERENCE_COUNT, sizeof( c##_refCount_new ), &c##_refCount_new, NULL ); \
	test_error( error, "Unable to check reference count for " #type ); \
	if( c##_refCount != c##_refCount_new ) \
	{	\
		log_error( "ERROR: Reference count for " #type " changed! (was %d, now %d)\n", c##_refCount, c##_refCount_new );	\
		return -1; \
	}

#define MARK_REF_COUNT_CONTEXT( c ) MARK_REF_COUNT_BASE( c, Context, CONTEXT )
#define TEST_REF_COUNT_CONTEXT( c ) TEST_REF_COUNT_BASE( c, Context, CONTEXT )

#define MARK_REF_COUNT_DEVICE( c ) MARK_REF_COUNT_BASE( c, Device, DEVICE )
#define TEST_REF_COUNT_DEVICE( c ) TEST_REF_COUNT_BASE( c, Device, DEVICE )

#define MARK_REF_COUNT_QUEUE( c ) MARK_REF_COUNT_BASE( c, CommandQueue, QUEUE )
#define TEST_REF_COUNT_QUEUE( c ) TEST_REF_COUNT_BASE( c, CommandQueue, QUEUE )

#define MARK_REF_COUNT_PROGRAM( c ) MARK_REF_COUNT_BASE( c, Program, PROGRAM )
#define TEST_REF_COUNT_PROGRAM( c ) TEST_REF_COUNT_BASE( c, Program, PROGRAM )

#define MARK_REF_COUNT_MEM( c ) MARK_REF_COUNT_BASE( c, MemObject, MEM )
#define TEST_REF_COUNT_MEM( c ) TEST_REF_COUNT_BASE( c, MemObject, MEM )

#endif // _ref_counting_h
