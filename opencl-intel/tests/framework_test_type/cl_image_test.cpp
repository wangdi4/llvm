#include "CL/cl.h"
#include "cl_types.h"
#include <stdio.h>
#include <time.h>
#include "FrameworkTest.h"

extern cl_device_type gDeviceType;

/**************************************************************************************************
 * clCopyImageTest
 * -------------------
 * Copy 2D image to 3D image
 **************************************************************************************************/
bool clCopyImageTest()
{
    bool         bResult = true;
    cl_int       iRet = CL_SUCCESS;
    cl_int       expectRet = CL_SUCCESS;
    cl_device_id clDefaultDeviceId;
    cl_uint ui;
    srand( (unsigned)time( NULL ) );


	cl_platform_id platform = 0;

	iRet = clGetPlatformIDs(1, &platform, NULL);
	bResult &= Check("clGetPlatformIDs", CL_SUCCESS, iRet);

	if (!bResult)
	{
		return bResult;
	}

	cl_context_properties prop[3] = { CL_CONTEXT_PLATFORM, (cl_context_properties)platform, 0 };

	printf("=============================================================\n");
	printf("clCopyImageTest\n");
	printf("=============================================================\n");

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
		
		{
			//
			// Create images for testing
			// 2D: 480x640
			// 3D  320x240x4
			size_t szSrcWidth = 640;
			size_t szSrcHeight = 480;
    size_t szDstWidth = 640;
    size_t szDstHeight = 480;
    size_t szDstDepth = 4;

    //
    // Set image info according to the format
    //
    

    size_t szSrcRowPitch   = szSrcWidth * 4; // num channels in CL_RGBA
    size_t szDstRowPitch   = szDstWidth * 4;
    size_t szDstSlicePitch = szDstRowPitch * szDstHeight;

    size_t szSrcByteSize = szSrcRowPitch * szSrcHeight;
    size_t szDstByteSize = szDstSlicePitch * szDstDepth;

    //
    // Allocate and image
    //
    cl_uchar* pSrcImageValues = (cl_uchar*)malloc(szSrcByteSize);
    // fill with random bits no matter what
	for( ui = 0; ui < (cl_uint)szSrcByteSize; ui++ )
    {
        pSrcImageValues[ui] = (cl_uchar)( rand() & 255 );
    }

	cl_image_format clFormat;
    clFormat.image_channel_order = CL_INTENSITY;
    clFormat.image_channel_data_type = CL_SNORM_INT16;

	printf( " - Creating src image %d by %d with unsupported format...\n", (int)szSrcWidth, (int)szSrcHeight );
    clCreateImage2D( context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, &clFormat, szSrcWidth, szSrcHeight, szSrcRowPitch, pSrcImageValues, &iRet );
    // FPGA emulator doesn't support images
    if (gDeviceType == CL_DEVICE_TYPE_ACCELERATOR)
    {
        expectRet = CL_INVALID_OPERATION;
    }
    else
    {
        expectRet = CL_IMAGE_FORMAT_NOT_SUPPORTED;
    }
    bResult &= Check("clCreateImage2D", expectRet, iRet);

	clFormat.image_channel_order = CL_RGBA;
    clFormat.image_channel_data_type = CL_UNORM_INT8;

    // Src img
    printf( " - Creating src image %d by %d...\n", (int)szSrcWidth, (int)szSrcHeight );
    cl_mem clSrcImg = clCreateImage2D( context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, &clFormat, szSrcWidth, szSrcHeight, szSrcRowPitch, pSrcImageValues, &iRet );
    if (gDeviceType == CL_DEVICE_TYPE_ACCELERATOR)
    {
        expectRet = CL_INVALID_OPERATION;
    }
    else
    {
        expectRet = CL_SUCCESS;
    }
    bResult &= Check("clCreateImage2D", expectRet, iRet);
	if (!bResult) goto release_queue;

    // Dst image
    printf( " - Creating image %d by %d by %d...\n", (int)szDstWidth, (int)szDstHeight, (int)szDstDepth );
    cl_mem clDstImg = clCreateImage3D( context, CL_MEM_READ_ONLY, &clFormat, szDstWidth, szDstHeight, szDstDepth, 0, 0, NULL, &iRet );
    bResult &= Check("clCreateImage3D", expectRet, iRet);
	if (!bResult) goto release_queue;

    // 
    // Initialize the destination and result to empty
    //
    cl_uchar* pDstImageValues = (cl_uchar*)malloc(szDstByteSize);
	memset( pDstImageValues, 0xff, szDstByteSize );

    cl_uchar* pResultImageValues = (cl_uchar*)malloc(szDstByteSize);
	memset( pResultImageValues, 0xff, szDstByteSize );

    //
    // Write src data
    //
	size_t origin[ 3 ] = { 0, 0, 0 };
    size_t region[ 3 ] = { szSrcWidth, szSrcHeight, 1 };
	
	iRet = clEnqueueWriteImage(queue, clSrcImg, CL_TRUE, origin, region, 0, 0, pSrcImageValues, 0, NULL, NULL);
	bResult &= Check("clEnqueueWriteImage - src", expectRet, iRet);
	if (!bResult) goto release_image;
	
	//
    // Write empty data
    //
	region[ 0 ] = szDstWidth;
	region[ 1 ] = szDstHeight;
	region[ 2 ] = szDstDepth;
  
	iRet = clEnqueueWriteImage( queue, clDstImg, CL_TRUE, origin, region, 0, 0, pDstImageValues, 0, NULL, NULL );
			bResult &= Check("clEnqueueWriteImage - dst", expectRet, iRet);
			if (!bResult) goto release_image;

			//
			// Copy the 2D image object to the 2nd 2D slice of the 3D image object.
			//
			{
				size_t dstOrigin[ 3 ] = { 0, 0, 1 };
				region[ 0 ] = szSrcWidth;
				region[ 1 ] = szSrcHeight;
				region[ 2 ] = 1;

				iRet = clEnqueueCopyImage( queue, clSrcImg, clDstImg, origin, dstOrigin, region, 0, NULL, NULL );
				bResult &= Check("clEnqueueCopyImage - 2D --> 3D", expectRet, iRet);
			}
			if (!bResult) goto release_image;

			//
			// Verification phase
			//
			// Note: we read back without any pitch, to verify pitch actually WORKED
    region[ 0 ] = szDstWidth;
	region[ 1 ] = szDstHeight;
	region[ 2 ] = szDstDepth;

	iRet = clEnqueueReadImage( queue, clDstImg, CL_TRUE, origin, region, 0, 0, pResultImageValues, 0, NULL, NULL );
	bResult &= Check("clEnqueueReadImage - Dst", expectRet, iRet);
	if (!bResult) goto release_image;
    if (gDeviceType == CL_DEVICE_TYPE_ACCELERATOR) goto release_queue;

    //
    // Compare results according to slices
    //
    for( ui =0; ui < szDstDepth; ui++ )
    {
        cl_uchar* pRes = pResultImageValues + (ui * szDstSlicePitch);
        if( 1 == ui )
        {
            // compare against src values
            if (memcmp( pRes, pSrcImageValues, szSrcByteSize))
            {
                // error
                printf ("*** clEnqueueCopyImage compare verification failed in the copied slice *** \n");
                bResult = false;
                goto release_image;
            }
        }
        else
        {
            // compare against dst values
            cl_uchar* pDstSrc = pDstImageValues + (ui * szDstSlicePitch);
            if (memcmp( pRes, pDstSrc, szDstSlicePitch))
            {
                // error
                printf ("*** clEnqueueCopyImage compare verification failed in the empty slice *** \n");
                bResult = false;
                goto release_image;
            }
        }
    }
    // If got here copy is done!
    printf ("*** clEnqueueCopyImage compare verification succeeded *** \n");

		release_image:
			free(pResultImageValues);
			free(pDstImageValues);
			free(pSrcImageValues);
			clReleaseMemObject(clSrcImg);
			clReleaseMemObject(clDstImg);
		}
	release_queue:
		clFinish(queue);
		clReleaseCommandQueue(queue);
	}
release_context:
    clReleaseContext(context);
release_end:
    return bResult;
}

