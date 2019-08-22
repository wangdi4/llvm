#include "CL/cl.h"
#include "cl_types.h"
#include <stdio.h>
#include <time.h>
#include "FrameworkTest.h"


extern cl_device_type gDeviceType;

/**************************************************************************************************
 * clImageExecuteTest
 * -------------------
 * Implement image access test
 **************************************************************************************************/
bool clImageExecuteTest()
{
    bool         bResult = true;
    cl_int       iRet = CL_SUCCESS;
    cl_device_id clDefaultDeviceId;
    srand( (unsigned)time( NULL ) );


	printf("=============================================================\n");
	printf("clImageExecuteTest\n");
	printf("=============================================================\n");

	const char *ocl_test_program[] = {\
	"__kernel void image_test(__read_only image2d_t srcImg, sampler_t samp, __global uint4* pPixels)\n"\
	"{\n"\
	"size_t x = get_global_id(0);\n"\
	"size_t y = get_global_id(1);\n"\
	"size_t pitch = get_image_width(srcImg);\n"\
	"\n"\
	"size_t pxlOff = y*pitch+x;\n"\
	"pPixels[pxlOff] = read_imageui(srcImg, samp, (int2)(x,y));\n"\
	"}"
	};

	cl_platform_id platform = 0;

	iRet = clGetPlatformIDs(1, &platform, NULL);
	bResult &= Check("clGetPlatformIDs", CL_SUCCESS, iRet);

	if (!bResult)
	{
		return bResult;
	}

	cl_context_properties prop[3] = { CL_CONTEXT_PLATFORM, (cl_context_properties)platform, 0 };

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

	{

		cl_command_queue queue = clCreateCommandQueue (context, clDefaultDeviceId, 0 /*no properties*/, &iRet);
		bResult &= Check("clCreateCommandQueue - queue", CL_SUCCESS, iRet);
		if (!bResult) goto release_context;


	#if 0
		cl_program program = clCreateProgramWithSource(context, 1, (const char**)&ocl_test_program, NULL, &iRet);
		bResult &= Check("clCreateProgramWithSource", CL_SUCCESS, iRet);
		if (!bResult) goto release_queue;
		iRet = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
		bResult &= Check("clBuildProgram", CL_SUCCESS, iRet);
		if (!bResult) goto release_program;
	#else
		cl_program program;
		if ( !BuildProgramSynch(context, 1, (const char**)&ocl_test_program, NULL, NULL, &program) )
			goto release_queue;
	#endif

		{

			cl_kernel kernel = clCreateKernel(program, "image_test", &iRet);
			bResult &= Check("clCreateKernel - image_test", CL_SUCCESS, iRet);
			if (!bResult) goto release_program;
			if (gDeviceType != CL_DEVICE_TYPE_ACCELERATOR)
			{

				//
				// Create images for testing
				// 2D: 480x640
				// 3D  320x240x4
				size_t szSrcWidth = 100;
				size_t szSrcHeight = 100;

				//
				// Set image info according to the format
				//
				cl_image_format clFormat;
				clFormat.image_channel_order = CL_RGBA;
				clFormat.image_channel_data_type = CL_UNSIGNED_INT8;

				size_t szSrcRowPitch   = szSrcWidth * 4; // num channels in CL_RGBA
				size_t szSrcByteSize = szSrcRowPitch * szSrcHeight;

				// Src img
				printf( " - Creating src image %d by %d...\n", (int)szSrcWidth, (int)szSrcHeight );
				cl_mem clSrcImg = clCreateImage2D( context, CL_MEM_READ_ONLY, &clFormat, szSrcWidth, szSrcHeight, 0, NULL, &iRet );
				bResult &= Check("clCreateImage2D", CL_SUCCESS, iRet);
				if (!bResult) goto release_kernel;
			
				// Destination buffer for 2D -> 1D convertion
				size_t stBuffSize = szSrcWidth*szSrcHeight*sizeof(cl_uint4);
				printf( " - Creating buffer %zu...\n", stBuffSize );
				cl_mem clDstBuff = clCreateBuffer(context, CL_MEM_WRITE_ONLY, stBuffSize, NULL, &iRet);
				bResult &= Check("clCreateBuffer", CL_SUCCESS, iRet);
				if (!bResult)
				{
					clReleaseMemObject(clSrcImg);
					goto release_kernel;
				}

			
				//
				// Allocate and image
				//
				cl_uchar* pSrcImageValues = (cl_uchar*)malloc(szSrcByteSize);
				// fill with random bits no matter what
				for( unsigned ui = 0; ui < szSrcByteSize; ui++ )
				{
					pSrcImageValues[ui] = (cl_uchar)( rand() & 255 );
				}

			
				cl_uint* pDstBuffer = (cl_uint*)malloc(stBuffSize);
				memset( pDstBuffer, 0xff, stBuffSize );

				{
					//
					// Write src data
					//
					size_t origin[ 3 ] = { 0, 0, 0 };
					size_t region[ 3 ] = { szSrcWidth, szSrcHeight, 1 };
				
					cl_sampler sampler = clCreateSampler(context, CL_FALSE, CL_ADDRESS_NONE, CL_FILTER_NEAREST, &iRet);
					bResult &= Check("clCreateSampler", CL_SUCCESS, iRet);
					if (!bResult) goto release_image;

					cl_sampler samplerLinear = clCreateSampler(context, CL_FALSE, CL_ADDRESS_NONE, CL_FILTER_LINEAR, &iRet);
					bResult &= Check("clCreateSampler", CL_SUCCESS, iRet);
					if (!bResult) goto release_sampler;

					// Set Kernel Arguments
					iRet = clSetKernelArg(kernel, 0, sizeof(cl_mem), &clSrcImg);
					bResult &= Check("clSetKernelArg - clSrcImg", CL_SUCCESS, iRet);
					if (!bResult) goto release_linear_sampler;
					iRet = clSetKernelArg(kernel, 1, sizeof(cl_sampler), &sampler);
					bResult &= Check("clSetKernelArg - sampler", CL_SUCCESS, iRet);
					if (!bResult) goto release_linear_sampler;
					iRet = clSetKernelArg(kernel, 2, sizeof(cl_mem), &clDstBuff);
					bResult &= Check("clSetKernelArg - clDstBuff", CL_SUCCESS, iRet);
					if (!bResult) goto release_linear_sampler;

					iRet = clEnqueueWriteImage(queue, clSrcImg, CL_TRUE, origin, region, 0, 0, pSrcImageValues, 0, NULL, NULL);
					bResult &= Check("clEnqueueWriteImage - src", CL_SUCCESS, iRet);
					if (!bResult) goto release_linear_sampler;

					{
						//size_t global_work_size[2] = { szSrcWidth, szSrcHeight };
						size_t global_work_size[2] = { 100, 100 };
						size_t local_work_size[2] = { 1, 1 };

						// Execute kernel
						iRet = clEnqueueNDRangeKernel(queue, kernel, 2, NULL, global_work_size, local_work_size, 0, NULL, NULL);
					}
					bResult &= Check("clEnqueueNDRangeKernel", CL_SUCCESS, iRet);    
					//
					// Verification phase
					//
					iRet = clEnqueueReadBuffer( queue, clDstBuff, CL_TRUE, 0, stBuffSize, pDstBuffer, 0, NULL, NULL );
					bResult &= Check("clEnqueueReadBuffer - Dst", CL_SUCCESS, iRet);
					if (!bResult) goto release_linear_sampler;

					for( unsigned y=0; y < szSrcHeight; ++y )
					{
						for (unsigned x=0; x<szSrcRowPitch; ++x )
						{
							size_t pixel = y*szSrcRowPitch+x;
							if ( pDstBuffer[pixel] != pSrcImageValues[pixel] )
							{
								bResult = false;
							}
						}
					}

					if ( bResult )
					{
						printf ("*** clImageExecuteTest compare verification succeeded *** \n");
					}
					else
					{
						printf ("!!!!!! clImageExecuteTest compare verification failed !!!!! \n");
					}

					/// Test invalid sampler not to crash
					printf ("Testing that invalid sampler usage don't lead to app crash..\n");
					iRet = clSetKernelArg(kernel, 1, sizeof(cl_sampler), &samplerLinear);
					bResult &= Check("clSetKernelArg - sampler", CL_SUCCESS, iRet);
					if (!bResult) goto release_linear_sampler;

					// we don't really care about image content here, so just left it as in previous test
					{
						//size_t global_work_size[2] = { szSrcWidth, szSrcHeight };
						size_t global_work_size[2] = { 100, 100 };
						size_t local_work_size[2] = { 1, 1 };

						// Execute kernel
						iRet = clEnqueueNDRangeKernel(queue, kernel, 2, NULL, global_work_size, local_work_size, 0, NULL, NULL);
					}
					printf ("Invalid sampler passed successfully!\n");

				release_linear_sampler:
					clReleaseSampler(samplerLinear);
				release_sampler:
					clReleaseSampler(sampler);
				}
			release_image:
				free(pSrcImageValues);
				free(pDstBuffer);
				clReleaseMemObject(clSrcImg);
				clReleaseMemObject(clDstBuff);
			}
		release_kernel:
			clReleaseKernel(kernel);
		}
	release_program:
		clReleaseProgram(program);
	release_queue:
		clFinish(queue);
		clReleaseCommandQueue(queue);
	}
release_context:
    clReleaseContext(context);
release_end:
    return bResult;
}

