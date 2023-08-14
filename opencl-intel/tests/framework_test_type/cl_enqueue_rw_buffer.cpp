#define PROVISIONAL_MALLOC_SIZE 100

#include "CL/cl.h"
#include "FrameworkTest.h"
#include "cl_provisional.h"
#include "cl_types.h"

// Must include this header after "cl_provisional.h",
// because the used macro PROV_INIT will be defined
// if not include "gtest.h" before, which is included
// by "TestsHelpClasses.h".
#include "TestsHelpClasses.h"
#include <stdio.h>

#define BUFFER_CL_ALLOC_SIZE (4 * 128)

extern cl_device_type gDeviceType;

/*******************************************************************************
 * clEnqueueRWBuffer
 ******************************************************************************/
static void enqueueAllVariants(cl_command_queue queue, cl_mem bufferForErr,
                               void *rwBuf, const size_t *buffRectOrigin,
                               const bool canRead, const bool canWrite,
                               const char *bufFlags) {
  cl_int iRet;

  iRet = clEnqueueReadBuffer(queue, bufferForErr, CL_TRUE, 0,
                             BUFFER_CL_ALLOC_SIZE, rwBuf, 0, NULL, NULL);
  if (canRead) {
    EXPECT_EQ(oclErr(CL_SUCCESS), oclErr(iRet))
        << "clEnqueueReadBuffer on buffer with flags (" << bufFlags
        << ") should succeed.";
  } else {
    EXPECT_EQ(oclErr(CL_INVALID_OPERATION), oclErr(iRet))
        << "clEnqueueReadBuffer on buffer with flags (" << bufFlags
        << ") should fail.";
  }

  iRet = clEnqueueWriteBuffer(queue, bufferForErr, CL_TRUE, 0,
                              BUFFER_CL_ALLOC_SIZE, rwBuf, 0, NULL, NULL);
  if (canWrite) {
    EXPECT_EQ(oclErr(CL_SUCCESS), oclErr(iRet))
        << "clEnqueueWriteBuffer on buffer with flags (" << bufFlags
        << ") should succeed.";
  } else {
    EXPECT_EQ(oclErr(CL_INVALID_OPERATION), oclErr(iRet))
        << "clEnqueueWriteBuffer on buffer with flags (" << bufFlags
        << ") should fail.";
  }

  iRet = clEnqueueReadBufferRect(queue, bufferForErr, CL_TRUE, buffRectOrigin,
                                 buffRectOrigin, buffRectOrigin, 0, 0, 0, 0,
                                 rwBuf, 0, NULL, NULL);
  if (canRead) {
    EXPECT_EQ(oclErr(CL_SUCCESS), oclErr(iRet))
        << "clEnqueueReadBufferRect on buffer with flags (" << bufFlags
        << ") should succeed.";
  } else {
    EXPECT_EQ(oclErr(CL_INVALID_OPERATION), oclErr(iRet))
        << "clEnqueueReadBufferRect on buffer with flags (" << bufFlags
        << ") should fail.";
  }

  iRet = clEnqueueWriteBufferRect(queue, bufferForErr, CL_TRUE, buffRectOrigin,
                                  buffRectOrigin, buffRectOrigin, 0, 0, 0, 0,
                                  rwBuf, 0, NULL, NULL);
  if (canWrite) {
    EXPECT_EQ(oclErr(CL_SUCCESS), oclErr(iRet))
        << "clEnqueueWriteBufferRect on buffer with flags (" << bufFlags
        << ") should succeed.";
  } else {
    EXPECT_EQ(oclErr(CL_INVALID_OPERATION), oclErr(iRet))
        << "clEnqueueWriteBufferRect on buffer with flags (" << bufFlags
        << ") should fail.";
  }
  clFinish(queue);
}

bool clEnqueueRWBuffer() {
  PROV_INIT;

  printf("=============================================================\n");
  printf("clCreateBufferTest\n");
  printf("=============================================================\n");
  cl_int iRet = 0;

  cl_platform_id platform = 0;
  bool bResult = true;
  cl_device_id clDefaultDeviceId;

  iRet = clGetPlatformIDs(1, &platform, NULL);
  bResult &= Check("", CL_SUCCESS, iRet);

  if (!bResult) {
    return bResult;
  }

  cl_context_properties prop[3] = {CL_CONTEXT_PLATFORM,
                                   (cl_context_properties)platform, 0};

  cl_context context =
      PROV_OBJ(clCreateContextFromType(prop, gDeviceType, NULL, NULL, &iRet));
  if (CL_SUCCESS != iRet) {
    printf("clCreateContextFromType = %s\n", ClErrTxt(iRet));
    PROV_RETURN_AND_ABANDON(false);
  }
  printf("context = %p\n", (void *)context);

  iRet = clGetDeviceIDs(platform, gDeviceType, 1, &clDefaultDeviceId, NULL);
  if (CL_SUCCESS != iRet) {
    printf("clGetDeviceIDs = %s\n", ClErrTxt(iRet));
    PROV_RETURN_AND_ABANDON(false);
  }
  printf("device = %p\n", (void *)clDefaultDeviceId);

  cl_command_queue queue = PROV_OBJ(clCreateCommandQueueWithProperties(
      context, clDefaultDeviceId, NULL /*no properties*/, &iRet));
  if (CL_SUCCESS != iRet) {
    printf("clCreateCommandQueueWithProperties = %s\n", ClErrTxt(iRet));
    PROV_RETURN_AND_ABANDON(false);
  }
  cl_mem bufferForErr;
  void *rwBuf = PROV_MALLOC(BUFFER_CL_ALLOC_SIZE);
  const size_t buffRectOrigin[MAX_WORK_DIM] = {1, 1, 1};

  bufferForErr = PROV_OBJ(clCreateBuffer(context, CL_MEM_HOST_NO_ACCESS,
                                         BUFFER_CL_ALLOC_SIZE, NULL, &iRet));
  EXPECT_EQ(oclErr(CL_SUCCESS), oclErr(iRet))
      << "clCreateBuffer with flags (CL_MEM_HOST_NO_ACCESS) should be "
         "OK.";
  if (CL_SUCCESS != iRet) {
    printf("clCreateBuffer (CL_MEM_HOST_NO_ACCESS) = %s\n", ClErrTxt(iRet));
    PROV_RETURN_AND_ABANDON(false);
  }
  enqueueAllVariants(queue, bufferForErr, rwBuf, buffRectOrigin, false, false,
                     "CL_MEM_HOST_NO_ACCESS");
  clReleaseMemObject(bufferForErr);

  bufferForErr = PROV_OBJ(clCreateBuffer(context, CL_MEM_HOST_READ_ONLY,
                                         BUFFER_CL_ALLOC_SIZE, NULL, &iRet));
  EXPECT_EQ(oclErr(CL_SUCCESS), oclErr(iRet))
      << "clCreateBuffer with flags (CL_MEM_HOST_READ_ONLY) should be OK.";
  if (CL_SUCCESS != iRet) {
    printf("clCreateBuffer (CL_MEM_HOST_READ_ONLY) = %s\n", ClErrTxt(iRet));
    PROV_RETURN_AND_ABANDON(false);
  }
  enqueueAllVariants(queue, bufferForErr, rwBuf, buffRectOrigin, true, false,
                     "CL_MEM_HOST_READ_ONLY");
  clReleaseMemObject(bufferForErr);

  bufferForErr = PROV_OBJ(clCreateBuffer(context, CL_MEM_HOST_WRITE_ONLY,
                                         BUFFER_CL_ALLOC_SIZE, NULL, &iRet));
  EXPECT_EQ(oclErr(CL_SUCCESS), oclErr(iRet))
      << "clCreateBuffer with flags (CL_MEM_HOST_WRITE_ONLY) should be OK.";
  if (CL_SUCCESS != iRet) {
    printf("clCreateBuffer (CL_MEM_HOST_WRITE_ONLY) = %s\n", ClErrTxt(iRet));
    PROV_RETURN_AND_ABANDON(false);
  }
  enqueueAllVariants(queue, bufferForErr, rwBuf, buffRectOrigin, false, true,
                     "CL_MEM_HOST_WRITE_ONLY");
  clReleaseMemObject(bufferForErr);

  clReleaseCommandQueue(queue);
  clReleaseContext(context);

  PROV_RETURN_AND_ABANDON(true);
}
