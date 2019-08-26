#include <stdio.h>

#include "CL/cl.h"
#include "cl_types.h"

#define PROVISIONAL_MALLOC_SIZE 100
#include "cl_provisional.h"

#include "TestsHelpClasses.h"

#include "FrameworkTest.h"

#define IMG_W 128
#define IMG_H 128
#define IMG_D 128
#define BUFFER_CL_ALLOC_IMG (IMG_W * IMG_H * IMG_D * 1)

extern cl_device_type gDeviceType;

/**************************************************************************************************
* clEnqueueRWBuffer
**************************************************************************************************/
static void enqueueAllVariants(cl_command_queue queue, cl_mem img, void *rwBuf, const size_t *buffRectOrigin,
		const size_t *buffRectRegion, const bool canRead, const bool canWrite, const char *bufFlags)
{
	cl_int iRet;

    iRet = clEnqueueReadImage(queue, img, CL_TRUE, buffRectOrigin, buffRectRegion, 0, 0, rwBuf, 0, NULL, NULL);
    if (canRead)
    {
    	EXPECT_EQ(oclErr(CL_SUCCESS),oclErr(iRet)) << "clEnqueueReadImage on image with flags ("<< bufFlags <<") should succeed.";
    } else {
    	EXPECT_EQ(oclErr(CL_INVALID_OPERATION),oclErr(iRet)) << "clEnqueueReadImage on image with flags ("<< bufFlags <<") should fail.";
    }

    iRet = clEnqueueWriteImage(queue, img, CL_TRUE, buffRectOrigin, buffRectRegion, 0, 0, rwBuf, 0, NULL, NULL);
    if (canWrite)
    {
    	EXPECT_EQ(oclErr(CL_SUCCESS),oclErr(iRet)) << "clEnqueueWriteImage on image with flags ("<< bufFlags <<") should succeed.";
    } else {
    	EXPECT_EQ(oclErr(CL_INVALID_OPERATION),oclErr(iRet)) << "clEnqueueWriteImage on image with flags ("<< bufFlags <<") should fail.";
    }
    clFinish(queue);
}

bool clImagePermissions()
{
	PROV_INIT;

	printf("=============================================================\n");
	printf("clImagePermissions\n");
	printf("=============================================================\n");
	cl_int iRet = 0;

	cl_platform_id platform = 0;
	bool bResult = true;
	cl_device_id clDefaultDeviceId;

	iRet = clGetPlatformIDs(1, &platform, NULL);
	bResult &= Check("", CL_SUCCESS, iRet);

	if (!bResult)
	{
		return bResult;
	}

	cl_context_properties prop[3] = { CL_CONTEXT_PLATFORM, (cl_context_properties)platform, 0 };

	cl_context context = PROV_OBJ( clCreateContextFromType(prop, gDeviceType, NULL, NULL, &iRet) );
	if (CL_SUCCESS != iRet)
	{
		printf("clCreateContextFromType = %s\n",ClErrTxt(iRet));
	    PROV_RETURN_AND_ABANDON(false);
	}
	printf("context = %p\n", (void*)context);

	iRet = clGetDeviceIDs(platform, gDeviceType, 1, &clDefaultDeviceId, NULL);
	if (CL_SUCCESS != iRet)
	{
		printf("clGetDeviceIDs = %s\n",ClErrTxt(iRet));
	    PROV_RETURN_AND_ABANDON(false);
	}
	printf("device = %p\n", (void*)clDefaultDeviceId);

	cl_command_queue queue = PROV_OBJ( clCreateCommandQueue (context, clDefaultDeviceId, 0 /*no properties*/, &iRet) );
	if (CL_SUCCESS != iRet)
	{
		printf("clCreateCommandQueue = %s\n",ClErrTxt(iRet));
	    PROV_RETURN_AND_ABANDON(false);
	}
    cl_mem imgForErr;
    void *rwBuf = PROV_MALLOC(BUFFER_CL_ALLOC_IMG);
    const size_t buffRectOrigin[MAX_WORK_DIM] = {0, 0, 0};
    const size_t buffRectRegion3D[MAX_WORK_DIM] = {64, 64, 64};
    const size_t buffRectRegion2D[MAX_WORK_DIM] = {64, 64, 1};

    cl_image_format imgFormat;
    imgFormat.image_channel_order = CL_BGRA;
    imgFormat.image_channel_data_type = CL_UNORM_INT8;

    cl_image_desc   imgDesc;
    imgDesc.image_type = CL_MEM_OBJECT_IMAGE3D;
    imgDesc.image_width  = IMG_W;
    imgDesc.image_height = IMG_H;
    imgDesc.image_depth  = IMG_D;
    imgDesc.image_array_size = 1;
    imgDesc.image_row_pitch = 0;
    imgDesc.image_slice_pitch = 0;
    imgDesc.num_mip_levels = 0;
    imgDesc.num_samples = 0;
    imgDesc.mem_object = NULL;

    //TODO: once new clCreateImage() interface is here, use it!
    //imgForErr = PROV_OBJ( clCreateImage(context, CL_MEM_HOST_NO_ACCESS, &imgFormat, &imgDesc, NULL, &iRet) );
    imgForErr = PROV_OBJ( clCreateImage3D(context, CL_MEM_HOST_NO_ACCESS, &imgFormat, IMG_W, IMG_H, IMG_D, 0, 0, NULL, &iRet) );
    if (gDeviceType == CL_DEVICE_TYPE_ACCELERATOR)
	{
		EXPECT_EQ(oclErr(CL_INVALID_OPERATION), oclErr(iRet))
			<< "clCreateImage3D with flags (CL_MEM_HOST_NO_ACCESS) should fail "
				"for CL_DEVICE_TYPE_ACCELERATOR.";
		if (CL_INVALID_OPERATION != iRet)
		{
			PROV_RETURN_AND_ABANDON(false);
		}
		else
		{
			PROV_RETURN_AND_ABANDON(true);
		}
	}
	else
	{
		EXPECT_EQ(oclErr(CL_SUCCESS), oclErr(iRet))
			<< "clCreateImage3D with flags (CL_MEM_HOST_NO_ACCESS) should be OK.";
	}

    if (CL_SUCCESS != iRet)
	{
		printf("clCreateImage (CL_MEM_HOST_NO_ACCESS) = %s\n",ClErrTxt(iRet));
	    PROV_RETURN_AND_ABANDON(false);
	}
    enqueueAllVariants(queue, imgForErr, rwBuf, buffRectOrigin, buffRectRegion3D, false, false, "CL_MEM_HOST_NO_ACCESS");
    clReleaseMemObject(imgForErr);

    imgForErr = PROV_OBJ( clCreateImage3D(context, CL_MEM_HOST_READ_ONLY, &imgFormat, IMG_W, IMG_H, IMG_D, 0, 0, NULL, &iRet) );
    EXPECT_EQ(oclErr(CL_SUCCESS),oclErr(iRet)) << "clCreateImage3D with flags (CL_MEM_HOST_READ_ONLY) should be OK.";
    if (CL_SUCCESS != iRet)
	{
		printf("clCreateImage (CL_MEM_HOST_READ_ONLY) = %s\n",ClErrTxt(iRet));
	    PROV_RETURN_AND_ABANDON(false);
	}
    enqueueAllVariants(queue, imgForErr, rwBuf, buffRectOrigin, buffRectRegion3D, true, false, "CL_MEM_HOST_READ_ONLY");
    clReleaseMemObject(imgForErr);

    imgForErr = PROV_OBJ( clCreateImage2D(context, CL_MEM_HOST_WRITE_ONLY, &imgFormat, IMG_W, IMG_H, 0, NULL, &iRet) );
    EXPECT_EQ(oclErr(CL_SUCCESS),oclErr(iRet)) << "clCreateImage3D with flags (CL_MEM_HOST_WRITE_ONLY) should be OK.";
    if (CL_SUCCESS != iRet)
	{
		printf("clCreateImage (CL_MEM_HOST_WRITE_ONLY) = %s\n",ClErrTxt(iRet));
	    PROV_RETURN_AND_ABANDON(false);
	}
    enqueueAllVariants(queue, imgForErr, rwBuf, buffRectOrigin, buffRectRegion2D, false, true, "CL_MEM_HOST_WRITE_ONLY");
    clReleaseMemObject(imgForErr);

    clReleaseCommandQueue(queue);
    clReleaseContext(context);

    PROV_RETURN_AND_ABANDON(true);
}
