#include "CL/cl.h"
#include "cl_types.h"
#include <stdio.h>
#include <time.h>
#include "FrameworkTest.h"

#define BUFFER_SIZE 128

extern cl_device_type gDeviceType;

/**************************************************************************************************
 * clIntegerExecuteTest
 * -------------------
 * Implement image access test
 **************************************************************************************************/
bool clLocalStructTest()
{
    bool         bResult = true;
    cl_int       iRet = CL_SUCCESS;
    cl_device_id clDefaultDeviceId;

	printf("=============================================================\n");
	printf("clILocalSTructAssingTest\n");
	printf("=============================================================\n");

	typedef struct _AABB {int Min[3];int Max[3];} AABB;

	const char *ocl_test_program[] = {
	"typedef struct _AABB {int Min[3];int Max[3];} AABB;\n"
	"kernel void Foo(global AABB* p, local AABB* lcl)\n"
	"{\n"
    "	int g=get_global_id(0);\n"
	"	__local AABB tileBB;\n"
	"	tileBB.Min[0] = -1.0; tileBB.Min[1] = -2.0; tileBB.Min[2] = -3.0;\n"
	"	tileBB.Max[0] = 1.0; tileBB.Max[1] = 2.0; tileBB.Max[2] = 3.0;\n"
	"	*lcl = tileBB;\n"
	"	event_t ev = async_work_group_copy((__global char*)&p[0], (__local char*)&tileBB, sizeof(AABB), 0);\n"
	"	wait_group_events(1, &ev);\n"
    "	p[1]=*lcl;\n"
	"}"
	};

	cl_platform_id platform = 0;

	
	iRet = clGetPlatformIDs(1, &platform, NULL);
	bResult &= Check("clGetPlatformIDs", CL_SUCCESS, iRet);

	if (!bResult)
	{
		return bResult;
	}

	// define local variables 
	cl_context_properties prop[3] = { CL_CONTEXT_PLATFORM, (cl_context_properties)platform, 0 };
	cl_program program;
	cl_command_queue queue;
	cl_kernel kernel;
	
	size_t global_work_size[1] = { 1 };
	
	AABB		pDstBuff[2];
	
	cl_mem clBuff;
    //
    // Initiate test infrastructure:
    // Create context, Queue
    //
	cl_context context = clCreateContextFromType(prop, gDeviceType, NULL, NULL, &iRet);
    bResult &= Check("clCreateContextFromType", CL_SUCCESS, iRet);    
    if (!bResult) goto release_end;

	iRet = clGetDeviceIDs(platform, gDeviceType, 1, &clDefaultDeviceId, NULL);
    bResult &= Check("clGetDeviceIDs", CL_SUCCESS, iRet);    
    if (!bResult) goto release_context;

    queue = clCreateCommandQueue (context, clDefaultDeviceId, 0 /*no properties*/, &iRet);
	bResult &= Check("clCreateCommandQueue - queue", CL_SUCCESS, iRet);
	if (!bResult) goto release_context;

	
	if ( !BuildProgramSynch(context, 1, (const char**)&ocl_test_program, NULL, NULL, &program) )
		goto release_queue;

    kernel = clCreateKernel(program, "Foo", &iRet);
	bResult &= Check("clCreateKernel - Foo", CL_SUCCESS, iRet);
	if (!bResult) goto release_program;



	srand( 0 );

	clBuff = clCreateBuffer(context, CL_MEM_READ_WRITE , sizeof(pDstBuff), NULL, &iRet);
	bResult &= Check("clCreateBuffer", CL_SUCCESS, iRet);
	if (!bResult)
	{
		goto release_kernel;
	}

	// Set Kernel Arguments
	iRet = clSetKernelArg(kernel, 0, sizeof(cl_mem), &clBuff);
	bResult &= Check("clSetKernelArg - clBuff", CL_SUCCESS, iRet);
	if (!bResult) goto release_image;

	iRet = clSetKernelArg(kernel, 1, sizeof(AABB), (void*)NULL);
	bResult &= Check("clSetKernelArg - local", CL_SUCCESS, iRet);
	if (!bResult) goto release_image;

    

	// Execute kernel
    iRet = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, global_work_size, NULL, 0, NULL, NULL);
    bResult &= Check("clEnqueueNDRangeKernel", CL_SUCCESS, iRet);    
    //
    // Verification phase
    //
	iRet = clEnqueueReadBuffer( queue, clBuff, CL_TRUE, 0, sizeof(pDstBuff), &pDstBuff, 0, NULL, NULL );
	bResult &= Check("clEnqueueReadBuffer - Dst", CL_SUCCESS, iRet);
	if (!bResult) goto release_image;

	bResult &= (pDstBuff[0].Min[0] == -1) && (pDstBuff[0].Min[1] == -2) && (pDstBuff[0].Min[2] == -3) &&
				(pDstBuff[0].Max[0] == 1) && (pDstBuff[0].Max[1] == 2) && (pDstBuff[0].Max[2] == 3) &&
				(pDstBuff[1].Min[0] == -1) && (pDstBuff[1].Min[1] == -2) && (pDstBuff[1].Min[2] == -3) &&
				(pDstBuff[1].Max[0] == 1) && (pDstBuff[1].Max[1] == 2) && (pDstBuff[1].Max[2] == 3);
	if ( bResult )
	{
	    printf ("*** clILocalSTructAssingTest compare verification succeeded *** \n");
	}
	else
	{
		printf ("!!!!!! clILocalSTructAssingTest compare verification failed !!!!! \n");
	}

release_image:
    clReleaseMemObject(clBuff);
release_kernel:
	clReleaseKernel(kernel);
release_program:
	clReleaseProgram(program);
release_queue:
    clFinish(queue);
    clReleaseCommandQueue(queue);
release_context:
    clReleaseContext(context);
release_end:
    return bResult;
}

