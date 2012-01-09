#include "cl.h"
#include "cl_types.h"
#include "Logger.h"
#include "cl_objects_map.h"
#include <stdio.h>
#include "FrameworkTest.h"
using namespace Intel::OpenCL::Framework;
 
int main( int argc, char ** argv )
{

	// USAGE
	if (argc < 2) {
        printf( "Usage: %s test_name \n", argv[0] );
        return( -1 );
    }
	// Print parameters
	printf( "Test name is: %s\n", argv[1] );

	bool bResult = true;

	if ((!strcmp(argv[1], "clGetDeviceIDs")) || (!strcmp(argv[1], "ALL")))
	{
		bResult &= clGetDeviceIDsTest();
	}
	if ((!strcmp(argv[1], "clGetPlatformInfo")) || (!strcmp(argv[1], "ALL")))
	{
		bResult &= clGetPlatformInfoTest();
	}
	if ((!strcmp(argv[1], "clGetDeviceInfo")) || (!strcmp(argv[1], "ALL")))
	{
		bResult &= clGetDeviceInfoTest();
	}
	if ((!strcmp(argv[1], "clCreateContext")) || (!strcmp(argv[1], "ALL")))
	{
		bResult &= clCreateContextTest();
	}
	if ((!strcmp(argv[1], "clBuildProgram")) || (!strcmp(argv[1], "ALL")))
	{
		bResult &= clBuildProgramTest();
	}
	if ((!strcmp(argv[1], "clCreateKernel")) || (!strcmp(argv[1], "ALL")))
	{
		bResult &= clCreateKernelTest();
	}
	if ((!strcmp(argv[1], "clCreateBuffer")) || (!strcmp(argv[1], "ALL")))
	{
		bResult &= clCreateBufferTest();
	}
	if ((!strcmp(argv[1], "clExecution")) || (!strcmp(argv[1], "ALL")))
	{
		bResult &= clExecutionTest();
	}
	if ((!strcmp(argv[1], "clOODotProduct")) || (!strcmp(argv[1], "ALL")))
	{
		bResult &= clOODotProductTest();
	}
	if ((!strcmp(argv[1], "clEnqueueCopyBuffer")) || (!strcmp(argv[1], "ALL")))
	{
		bResult &= clEnqueueCopyBufferTest();
	}

	if (bResult)
	{
		printf("\n==============\nTEST SUCCEDDED\n==============\n");
		return 1;
	}
	else
	{
		printf("\n==============\nTEST FAILED\n==============\n");
		return 0;
	}
	return 1;
}