#include <stdio.h>

#include "CL/cl.h"
#include "cl_types.h"
#include "Logger.h"
#include "cl_objects_map.h"

#define PROVISIONAL_MALLOC_SIZE 100
#include "cl_provisional.h"

#include "TestsHelpClasses.h"

#include "FrameworkTest.h"

using namespace Intel::OpenCL::Framework;
using namespace Intel::OpenCL::Utils;

/**************************************************************************************************
* clCreateBufferTest
* -------------------
* Get device ids (CL_DEVICE_TYPE_CPU)
* Create context
* Create context from type (CL_DEVICE_TYPE_CPU)
* Retain context
* Release context
* Get context info (CL_CONTEXT_REFERENCE_COUNT)
**************************************************************************************************/
bool clCreateBufferTest()
{
	PROV_INIT;

	printf("=============================================================\n");
	printf("clCreateBufferTest\n");
	printf("=============================================================\n");
	cl_uint uiNumDevices = 0;
	cl_int iRet = 0;

	cl_platform_id platform = 0;
	bool bResult = true;

	iRet = clGetPlatformIDs(1, &platform, NULL);
	bResult &= Check(L"clGetPlatformIDs", CL_SUCCESS, iRet);

	if (!bResult)
	{
		return bResult;
	}

	cl_context_properties prop[3] = { CL_CONTEXT_PLATFORM, (cl_context_properties)platform, 0 };

	cl_context context = PROV_OBJ( clCreateContextFromType(prop, CL_DEVICE_TYPE_CPU, NULL, NULL, &iRet) );
	if (CL_SUCCESS != iRet)
	{
		printf("clCreateContextFromType = %ls\n",ClErrTxt(iRet));
	    PROV_RETURN_AND_ABANDON(false);
	}

	printf("context = %p\n", context);

	cl_mem buffer1 = PROV_OBJ( clCreateBuffer(context, CL_MEM_READ_ONLY, 100, NULL, &iRet) );
	EXPECT_EQ(oclErr(CL_SUCCESS),oclErr(iRet)) << "clCreateBuffer with flags (CL_MEM_READ_ONLY) should be OK.";
	printf("buffer1 = %p\n", buffer1);

	cl_mem buffer2 = PROV_OBJ( clCreateBuffer(context, CL_MEM_WRITE_ONLY, 100, NULL, &iRet) );
	EXPECT_EQ(oclErr(CL_SUCCESS),oclErr(iRet)) << "clCreateBuffer with flags (CL_MEM_WRITE_ONLY) should be OK.";
	printf("buffer2 = %p\n", buffer2);

	cl_mem buffer3 = PROV_OBJ( clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_WRITE_ONLY, 100, NULL, &iRet) );
	EXPECT_EQ(oclErr(CL_INVALID_VALUE),oclErr(iRet)) << "clCreateBuffer with flags (CL_MEM_READ_ONLY | CL_MEM_WRITE_ONLY) should fail.";

	/*
	 * Start testing host access permissions.
	 */

	// Illegal flag combinations.
	cl_mem bufferForErr;

	bufferForErr = PROV_OBJ( clCreateBuffer(context, CL_MEM_HOST_NO_ACCESS | CL_MEM_HOST_READ_ONLY, 100, NULL, &iRet) );
	EXPECT_EQ(oclErr(CL_INVALID_VALUE),oclErr(iRet)) << "clCreateBuffer with flags (CL_MEM_HOST_NO_ACCESS | CL_MEM_HOST_READ_ONLY) should fail.";

	bufferForErr = PROV_OBJ( clCreateBuffer(context, CL_MEM_HOST_NO_ACCESS | CL_MEM_HOST_WRITE_ONLY, 100, NULL, &iRet) );
	EXPECT_EQ(oclErr(CL_INVALID_VALUE),oclErr(iRet)) << "clCreateBuffer with flags (CL_MEM_HOST_NO_ACCESS | CL_MEM_HOST_WRITE_ONLY) should fail.";

	bufferForErr = PROV_OBJ( clCreateBuffer(context, CL_MEM_HOST_READ_ONLY | CL_MEM_HOST_WRITE_ONLY, 100, NULL, &iRet) );
	EXPECT_EQ(oclErr(CL_INVALID_VALUE),oclErr(iRet)) << "clCreateBuffer with flags (CL_MEM_HOST_READ_ONLY | CL_MEM_HOST_WRITE_ONLY) should fail.";

	cl_mem bufferWithNoHostAccess =
			PROV_OBJ( clCreateBuffer(context, CL_MEM_HOST_NO_ACCESS, 100, NULL, &iRet) );

	PROV_RETURN_AND_ABANDON(true);
}

bool clCreateSubBufferTest()
{
	PROV_INIT;

	printf("=============================================================\n");
	printf("clCreateSubBufferTest\n");
	printf("=============================================================\n");
	cl_uint uiNumDevices = 0;
	cl_int iRet = 0;

	cl_platform_id platform = 0;
	bool bResult = true;

	iRet = clGetPlatformIDs(1, &platform, NULL);
	bResult &= Check(L"", CL_SUCCESS, iRet);

	if (!bResult)
	{
		return bResult;
	}

	cl_context_properties prop[3] = { CL_CONTEXT_PLATFORM, (cl_context_properties)platform, 0 };

	cl_context context = PROV_OBJ( clCreateContextFromType(prop, CL_DEVICE_TYPE_CPU, NULL, NULL, &iRet) );
	if (CL_SUCCESS != iRet)
	{
		printf("clCreateContextFromType = %ls\n",ClErrTxt(iRet));
		PROV_RETURN_AND_ABANDON(false);
	}

	cl_buffer_region region;
	region.origin = (1 * 128);
	region.size   = (2 * 128);

	cl_mem subBufferForTesting;

	/* sub buffers for CL_MEM_HOST_NO_ACCESS */
	cl_mem bufferNoAccess = PROV_OBJ( clCreateBuffer(context, CL_MEM_HOST_NO_ACCESS, (4 * 128), NULL, &iRet) );
	EXPECT_EQ(oclErr(CL_SUCCESS),oclErr(iRet)) << "clCreateBuffer with flags (CL_MEM_HOST_NO_ACCESS) should be OK.";

	subBufferForTesting = PROV_OBJ( clCreateSubBuffer(bufferNoAccess, CL_MEM_HOST_READ_ONLY, CL_BUFFER_CREATE_TYPE_REGION,
			(void *)&region, &iRet) );
	EXPECT_EQ(oclErr(CL_INVALID_VALUE),oclErr(iRet)) << "clCreateBuffer with flags (CL_MEM_HOST_READ_ONLY) for parent with (CL_MEM_HOST_NO_ACCESS) should fail.";

	subBufferForTesting = PROV_OBJ( clCreateSubBuffer(bufferNoAccess, CL_MEM_HOST_WRITE_ONLY, CL_BUFFER_CREATE_TYPE_REGION,
			(void *)&region, &iRet) );
	EXPECT_EQ(oclErr(CL_INVALID_VALUE),oclErr(iRet)) << "clCreateBuffer with flags (CL_MEM_HOST_WRITE_ONLY) for parent with (CL_MEM_HOST_NO_ACCESS) should fail.";

	subBufferForTesting = PROV_OBJ( clCreateSubBuffer(bufferNoAccess, CL_MEM_HOST_NO_ACCESS, CL_BUFFER_CREATE_TYPE_REGION,
			(void *)&region, &iRet) );
	EXPECT_EQ(oclErr(CL_SUCCESS),oclErr(iRet)) << "clCreateBuffer with flags (CL_MEM_HOST_NO_ACCESS) for parent with (CL_MEM_HOST_NO_ACCESS) should be fine.";


	/* sub buffers for CL_MEM_HOST_READ_ONLY */
	cl_mem bufferReadOnlyAccess = PROV_OBJ( clCreateBuffer(context, CL_MEM_HOST_READ_ONLY, (4 * 128), NULL, &iRet) );
	EXPECT_EQ(oclErr(CL_SUCCESS),oclErr(iRet)) << "clCreateBuffer with flags (CL_MEM_HOST_READ_ONLY) should be OK.";

	subBufferForTesting = PROV_OBJ( clCreateSubBuffer(bufferReadOnlyAccess, CL_MEM_HOST_WRITE_ONLY, CL_BUFFER_CREATE_TYPE_REGION,
			(void *)&region, &iRet) );
	EXPECT_EQ(oclErr(CL_INVALID_VALUE),oclErr(iRet)) << "clCreateBuffer with flags (CL_MEM_HOST_WRITE_ONLY) for parent with (CL_MEM_HOST_READ_ONLY) should fail.";

	subBufferForTesting = PROV_OBJ( clCreateSubBuffer(bufferReadOnlyAccess, CL_MEM_HOST_READ_ONLY, CL_BUFFER_CREATE_TYPE_REGION,
			(void *)&region, &iRet) );
	EXPECT_EQ(oclErr(CL_SUCCESS),oclErr(iRet)) << "clCreateBuffer with flags (CL_MEM_HOST_READ_ONLY) for parent with (CL_MEM_HOST_READ_ONLY) should be fine.";

	subBufferForTesting = PROV_OBJ( clCreateSubBuffer(bufferReadOnlyAccess, CL_MEM_HOST_NO_ACCESS, CL_BUFFER_CREATE_TYPE_REGION,
			(void *)&region, &iRet) );
	EXPECT_EQ(oclErr(CL_SUCCESS),oclErr(iRet)) << "clCreateBuffer with flags (CL_MEM_HOST_NO_ACCESS) for parent with (CL_MEM_HOST_READ_ONLY) should be fine.";


	/* sub buffers for CL_MEM_HOST_WRITE_ONLY */
	cl_mem bufferWriteOnlyAccess = PROV_OBJ( clCreateBuffer(context, CL_MEM_HOST_WRITE_ONLY, (4 * 128), NULL, &iRet) );
	EXPECT_EQ(oclErr(CL_SUCCESS),oclErr(iRet)) << "clCreateBuffer with flags (CL_MEM_HOST_WRITE_ONLY) should be OK.";

	subBufferForTesting = PROV_OBJ( clCreateSubBuffer(bufferWriteOnlyAccess, CL_MEM_HOST_READ_ONLY, CL_BUFFER_CREATE_TYPE_REGION,
			(void *)&region, &iRet) );
	EXPECT_EQ(oclErr(CL_INVALID_VALUE),oclErr(iRet)) << "clCreateBuffer with flags (CL_MEM_HOST_READ_ONLY) for parent with (CL_MEM_HOST_WRITE_ONLY) should fail.";

	subBufferForTesting = PROV_OBJ( clCreateSubBuffer(bufferWriteOnlyAccess, CL_MEM_HOST_WRITE_ONLY, CL_BUFFER_CREATE_TYPE_REGION,
			(void *)&region, &iRet) );
	EXPECT_EQ(oclErr(CL_SUCCESS),oclErr(iRet)) << "clCreateBuffer with flags (CL_MEM_HOST_WRITE_ONLY) for parent with (CL_MEM_HOST_WRITE_ONLY) should be fine.";

	subBufferForTesting = PROV_OBJ( clCreateSubBuffer(bufferWriteOnlyAccess, CL_MEM_HOST_NO_ACCESS, CL_BUFFER_CREATE_TYPE_REGION,
			(void *)&region, &iRet) );
	EXPECT_EQ(oclErr(CL_SUCCESS),oclErr(iRet)) << "clCreateBuffer with flags (CL_MEM_HOST_NO_ACCESS) for parent with (CL_MEM_HOST_WRITE_ONLY) should be fine.";

	PROV_RETURN_AND_ABANDON(true);
}
