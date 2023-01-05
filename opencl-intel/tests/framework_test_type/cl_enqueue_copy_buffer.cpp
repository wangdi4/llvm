#include "CL/cl.h"
#include "FrameworkTest.h"
#include "cl_types.h"
#include <stdio.h>

extern cl_device_type gDeviceType;

#define BUFFER_LEN (30)
#define UINT_BUFFER_LEN (BUFFER_LEN * sizeof(cl_uint))
/*******************************************************************************
 * clEnqueueCopyBufferTest
 * -------------------
 *
 ******************************************************************************/
bool clEnqueueCopyBufferTest() {
  printf("=============================================================\n");
  printf("clEnqueueCopyBuffer\n");
  printf("=============================================================\n");
  cl_device_id clDefaultDeviceId;
  cl_int iRet = 0;

  cl_platform_id platform = 0;
  bool bResult = true;

  iRet = clGetPlatformIDs(1, &platform, NULL);
  bResult &= Check("clGetPlatformIDs", CL_SUCCESS, iRet);

  if (!bResult) {
    return bResult;
  }

  cl_context_properties prop[3] = {CL_CONTEXT_PLATFORM,
                                   (cl_context_properties)platform, 0};

  //
  // Create context
  //
  cl_context context =
      clCreateContextFromType(prop, gDeviceType, NULL, NULL, &iRet);
  if (CL_SUCCESS != iRet) {
    printf("clCreateContextFromType = %s\n", ClErrTxt(iRet));
    goto release_end;
  }
  printf("context = %p\n", (void *)context);

  //
  // Get context devices
  //
  iRet = clGetDeviceIDs(platform, gDeviceType, 1, &clDefaultDeviceId, NULL);
  if (CL_SUCCESS != iRet) {
    printf("clGetDeviceIDs = %s\n", ClErrTxt(iRet));
    goto release_context;
  }
  printf("device = %p\n", (void *)clDefaultDeviceId);

  //
  // Create 3 buffers
  //
  {
    cl_mem buffer1 = clCreateBuffer(context, CL_MEM_READ_WRITE, UINT_BUFFER_LEN,
                                    NULL, &iRet);
    if (CL_SUCCESS != iRet) {
      printf("clCreateBuffer (CL_MEM_READ_WRITE)= %s\n", ClErrTxt(iRet));
      goto release_context;
    }
    printf("buffer1 = %p\n", (void *)buffer1);

    {
      cl_mem buffer2 = clCreateBuffer(context, CL_MEM_READ_WRITE,
                                      UINT_BUFFER_LEN, NULL, &iRet);

      if (CL_SUCCESS != iRet) {
        printf("clCreateBuffer (CL_MEM_READ_WRITE)= %s\n", ClErrTxt(iRet));
        goto release_buffer1;
      }
      printf("buffer2 = %p\n", (void *)buffer2);

      {
        cl_mem buffer3 = clCreateBuffer(context, CL_MEM_READ_WRITE,
                                        UINT_BUFFER_LEN, NULL, &iRet);

        if (CL_SUCCESS != iRet) {
          printf("clCreateBuffer (CL_MEM_READ_WRITE)= %s\n", ClErrTxt(iRet));
          goto release_buffer2;
        }
        printf("buffer3 = %p\n", (void *)buffer3);

        {
          //
          // Create queue
          //
          cl_command_queue queue = clCreateCommandQueueWithProperties(
              context, clDefaultDeviceId, NULL /*NO PROPERTIES*/, &iRet);
          if (CL_SUCCESS != iRet) {
            printf("clCreateCommandQueueWithProperties "
                   "= %s\n",
                   ClErrTxt(iRet));
            goto release_memory;
          }
          printf("queue = %p\n", (void *)queue);

          //
          // Write 2 bufferrs with 1/2, than copy 1 to
          // the upper of the 3rd buffer, and 2 to the
          // lower.
          //
          cl_uint array1[BUFFER_LEN];
          cl_uint array2[BUFFER_LEN];
          cl_uint array3[BUFFER_LEN];

          {
            int i = 0;
            for (; i < BUFFER_LEN; i++) {
              array1[i] = 1;
              array2[i] = 2;
              array3[i] = 3;
            }

            iRet = clEnqueueWriteBuffer(queue, buffer1, CL_FALSE, 0,
                                        UINT_BUFFER_LEN, array1, 0, NULL, NULL);
            if (CL_SUCCESS != iRet) {
              printf("clEnqueueWriteBuffer(1) = %s\n", ClErrTxt(iRet));
              goto release_queue;
            }

            iRet = clEnqueueWriteBuffer(queue, buffer2, CL_FALSE, 0,
                                        UINT_BUFFER_LEN, array2, 0, NULL, NULL);
            if (CL_SUCCESS != iRet) {
              printf("clEnqueueWriteBuffer(2) = %s\n", ClErrTxt(iRet));
              goto release_queue;
            }

            iRet = clEnqueueCopyBuffer(queue, buffer1, buffer3, 0,
                                       UINT_BUFFER_LEN / 2, UINT_BUFFER_LEN / 2,
                                       0, NULL, NULL);
            if (CL_SUCCESS != iRet) {
              printf("clEnqueueCopyBuffer(1) = %s\n", ClErrTxt(iRet));
              goto release_queue;
            }

            iRet = clEnqueueCopyBuffer(queue, buffer2, buffer3,
                                       UINT_BUFFER_LEN / 2, 0,
                                       UINT_BUFFER_LEN / 2, 0, NULL, NULL);
            if (CL_SUCCESS != iRet) {
              printf("clEnqueueCopyBuffer(1) = %s\n", ClErrTxt(iRet));
              goto release_queue;
            }

            //
            // Read and print output buffer
            //
            iRet = clEnqueueReadBuffer(queue, buffer3, CL_TRUE, 0,
                                       UINT_BUFFER_LEN, array3, 0, NULL, NULL);
            if (CL_SUCCESS != iRet) {
              printf("clEnqueueReadBuffer = %s\n", ClErrTxt(iRet));
              goto release_queue;
            }

            printf("Output Buffer: ");
            for (i = 0; i < BUFFER_LEN; i++) {
              printf("%d,", array3[i]);
            }
            printf("\n");
          }

        release_queue:
          clFinish(queue);
          clReleaseCommandQueue(queue);
        }
      release_memory:
        clReleaseMemObject(buffer3);
      }
    release_buffer2:
      clReleaseMemObject(buffer2);
    }
  release_buffer1:
    clReleaseMemObject(buffer1);
  }
release_context:
  clReleaseContext(context);
release_end:
  if (CL_SUCCESS != iRet) {
    return false;
  }
  return true;
}
