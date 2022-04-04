#include "CL/cl.h"
#include "FrameworkTest.h"
#include "cl_types.h"
#include <stdio.h>

#define BUFFERS_LENGTH 20000
#define NUM_LOOPS 4

extern cl_device_type gDeviceType;

/******************************************************************************
 * cl_map_buffer_test - Implement dot product example with map/unmap API.
 * -------------------
 *
 *
 *****************************************************************************/

/******************************************************************************
 *
 *
 *****************************************************************************/
bool clMapBufferTest() {
  printf("---------------------------------------\n");
  printf("clMapBufferTest\n");
  printf("---------------------------------------\n");
  const char *ocl_test_program[] = {
      "__kernel void dot_product (__global const float4 *a, __global const "
      "float4 *b, __global float *c)"
      "{"
      "int tid = get_global_id(0);"
      "c[tid] = dot(a[tid], b[tid]);"
      "}"};
  bool bResult = true;

  cl_uint uiNumDevices = 0;
  cl_device_id *pDevices = nullptr;
  cl_context context;

  cl_platform_id platform = 0;

  cl_int iRet = clGetPlatformIDs(1, &platform, NULL);
  bResult &= Check("clGetPlatformIDs", CL_SUCCESS, iRet);

  if (!bResult) {
    return bResult;
  }

  cl_context_properties prop[3] = {CL_CONTEXT_PLATFORM,
                                   (cl_context_properties)platform, 0};

  // get device(s)
  iRet = clGetDeviceIDs(platform, gDeviceType, 0, NULL, &uiNumDevices);
  bResult &= Check("clGetDeviceIDs", CL_SUCCESS, iRet);
  if (!bResult)
    goto end;

  // initialize arrays
  pDevices = new cl_device_id[uiNumDevices];

  iRet = clGetDeviceIDs(platform, gDeviceType, uiNumDevices, pDevices, NULL);
  bResult &= Check("clGetDeviceIDs", CL_SUCCESS, iRet);
  if (!bResult)
    goto end;

  // create context
  context = clCreateContext(prop, uiNumDevices, pDevices, NULL, NULL, &iRet);
  bResult &= Check("clCreateContext", CL_SUCCESS, iRet);
  if (!bResult)
    goto end;

  {
    // create program with source
    cl_program program = clCreateProgramWithSource(
        context, 1, (const char **)&ocl_test_program, NULL, &iRet);
    bResult &= Check("clCreateProgramWithSource", CL_SUCCESS, iRet);
    if (!bResult)
      goto release_context;

    iRet = clBuildProgram(program, uiNumDevices, pDevices, NULL, NULL, NULL);
    bResult &= Check("clBuildProgram", CL_SUCCESS, iRet);
    if (!bResult)
      goto release_program;

    //
    // From here down it is the program execution implementation
    //
    cl_float *buffA; // if I initialized buffA here, I would get an error in
                     // gcc: "jump to label release_program from here crosses
                     // initialization of cl_float* buffA"
    buffA = (cl_float *)ALIGNED_MALLOC(
        BUFFERS_LENGTH * sizeof(cl_float),
        sizeof(cl_float4)); // Buffer to use for A storage
    cl_float *srcA;
    cl_float *srcA2;
    cl_float *srcB;
    cl_float *dsts[NUM_LOOPS];

    {
      //
      // Create Kernel
      //
      cl_kernel kernel1 = clCreateKernel(program, "dot_product", &iRet);
      bResult &= Check("clCreateKernel - dot_product", CL_SUCCESS, iRet);
      if (!bResult)
        goto release_kernel;

      {
        //
        // Create buffer for mem/unmap
        //
        size_t size = sizeof(cl_float);

        {
          cl_mem buffer_srcA =
              clCreateBuffer(context, CL_MEM_USE_HOST_PTR | CL_MEM_READ_ONLY,
                             size * BUFFERS_LENGTH, buffA, &iRet);
          bResult &= Check("clCreateBuffer - srcA", CL_SUCCESS, iRet);
          if (!bResult)
            goto release_srcA;

          {
            cl_mem buffer_srcB = clCreateBuffer(
                context, CL_MEM_ALLOC_HOST_PTR | CL_MEM_READ_ONLY,
                size * BUFFERS_LENGTH, NULL, &iRet);
            bResult &= Check("clCreateBuffer - srcB", CL_SUCCESS, iRet);
            if (!bResult)
              goto release_srcB;

            {
              cl_mem buffer_dst = clCreateBuffer(
                  context, CL_MEM_ALLOC_HOST_PTR | CL_MEM_READ_WRITE,
                  size * BUFFERS_LENGTH, NULL, &iRet);
              bResult &= Check("clCreateBuffer - Dst", CL_SUCCESS, iRet);
              if (!bResult)
                goto release_dst;

              //
              // Set arguments
              //
              iRet = clSetKernelArg(kernel1, 0, sizeof(cl_mem), &buffer_srcA);
              bResult &=
                  Check("clSetKernelArg - buffer_srcA", CL_SUCCESS, iRet);
              if (!bResult)
                goto release_dst;

              iRet = clSetKernelArg(kernel1, 1, sizeof(cl_mem), &buffer_srcB);
              bResult &=
                  Check("clSetKernelArg - buffer_srcB", CL_SUCCESS, iRet);
              if (!bResult)
                goto release_dst;

              iRet = clSetKernelArg(kernel1, 2, sizeof(cl_mem), &buffer_dst);
              bResult &= Check("clSetKernelArg - buffer_dst", CL_SUCCESS, iRet);
              if (!bResult)
                goto release_dst;

              {
                //
                // Create queue
                //
                cl_command_queue queue1 = clCreateCommandQueueWithProperties(
                    context, pDevices[0], NULL /*no properties*/, &iRet);
                bResult &= Check("clCreateCommandQueueWithProperties - queue1",
                                 CL_SUCCESS, iRet);
                if (!bResult)
                  goto release_queue;

                //
                // Execute commands - Write buffers using map/unmap
                //
                clEnqueueMapBuffer(queue1, buffer_srcA, CL_TRUE, CL_MAP_WRITE,
                                   0, size * BUFFERS_LENGTH, 1, NULL, NULL,
                                   &iRet);
                bResult &= Check("clEnqueueMapBuffer - invalid waiting list",
                                 CL_INVALID_EVENT_WAIT_LIST, iRet);
                if (!bResult)
                  goto release_queue;

                srcA = (cl_float *)clEnqueueMapBuffer(
                    queue1, buffer_srcA, CL_TRUE, CL_MAP_WRITE, 0,
                    size * BUFFERS_LENGTH, 0, NULL, NULL, &iRet);
                bResult &= Check("clEnqueueMapBuffer - srcA", CL_SUCCESS, iRet);
                if (!bResult)
                  goto release_queue;

                // Test multiple mapping
                srcA2 = (cl_float *)clEnqueueMapBuffer(
                    queue1, buffer_srcA, CL_TRUE, CL_MAP_WRITE, 0, size, 0,
                    NULL, NULL, &iRet);
                bResult &=
                    Check("clEnqueueMapBuffer - srcA2", CL_SUCCESS, iRet);
                if (!bResult)
                  goto release_queue;

                srcB = (cl_float *)clEnqueueMapBuffer(
                    queue1, buffer_srcB, CL_TRUE, CL_MAP_WRITE, 0,
                    size * BUFFERS_LENGTH, 0, NULL, NULL, &iRet);
                bResult &= Check("clEnqueueMapBuffer - srcB", CL_SUCCESS, iRet);
                if (!bResult)
                  goto release_queue;

                //
                // Init map buffer and unmap them to flush it to the device
                //
                for (int j = 0; j < BUFFERS_LENGTH; j++) {
                  srcA[j] = (cl_float)j;
                  srcB[j] = 1;
                }

                iRet = clEnqueueUnmapMemObject(queue1, buffer_srcA, srcA, 0,
                                               NULL, NULL);
                bResult &=
                    Check("clEnqueueUnmapMemObject - srcA", CL_SUCCESS, iRet);
                if (!bResult)
                  goto release_queue;

                iRet = clEnqueueUnmapMemObject(queue1, buffer_srcA, srcA2, 0,
                                               NULL, NULL);
                bResult &=
                    Check("clEnqueueUnmapMemObject - srcA2", CL_SUCCESS, iRet);
                if (!bResult)
                  goto release_queue;

                iRet = clEnqueueUnmapMemObject(queue1, buffer_srcB, srcB, 0,
                                               NULL, NULL);
                bResult &=
                    Check("clEnqueueUnmapMemObject - srcB", CL_SUCCESS, iRet);
                if (!bResult)
                  goto release_queue;

                {
                  //
                  // Execute kernel - dot_product
                  //
                  size_t global_work_size[1] = {BUFFERS_LENGTH / 4};
                  size_t local_work_size[1] = {1};

                  for (int index = 0; index < NUM_LOOPS; index++) {
                    iRet = clEnqueueNDRangeKernel(
                        queue1, kernel1, 1, NULL, global_work_size,
                        local_work_size, 0, NULL, NULL);
                    bResult &=
                        Check("clEnqueueNDRangeKernel", CL_SUCCESS, iRet);
                    if (!bResult)
                      goto release_queue;

                    //
                    // Read results. use map
                    //
                    cl_event waitOn;
                    dsts[index] = (cl_float *)clEnqueueMapBuffer(
                        queue1, buffer_dst, CL_FALSE, CL_MAP_READ, 0,
                        size * BUFFERS_LENGTH, 0, NULL, &waitOn, &iRet);
                    bResult &=
                        Check("clEnqueueMapBuffer - Dst", CL_SUCCESS, iRet);
                    if (!bResult)
                      goto release_queue;

                    //
                    // Wait for map done and print kernel output
                    //
                    clWaitForEvents(1, &waitOn);
                    printf("\n ==== \n");
                    for (int i = 0; i < 10; i++) {
                      printf("%lf, ", dsts[index][i]);
                    }
                    printf("\n ==== \n");
                    clReleaseEvent(waitOn);

                    // Unmap
                    iRet = clEnqueueUnmapMemObject(
                        queue1, buffer_dst, dsts[index], 0, NULL, &waitOn);
                    bResult &= Check("clEnqueueUnmapMemObject - Dst",
                                     CL_SUCCESS, iRet);
                    if (!bResult)
                      goto release_queue;
                    // Wait for Unmap to complete
                    clWaitForEvents(1, &waitOn);
                    clReleaseEvent(waitOn);
                  }
                }

              release_queue:
                clFinish(queue1);
                clReleaseCommandQueue(queue1);
              }
            release_dst:
              clReleaseMemObject(buffer_dst);
            }
          release_srcB:
            clReleaseMemObject(buffer_srcB);
          }
        release_srcA:
          clReleaseMemObject(buffer_srcA);
          ALIGNED_FREE(buffA);
        }
      }
    release_kernel:
      clReleaseKernel(kernel1);
    }
  release_program:
    clReleaseProgram(program);
  }
release_context:
  clReleaseContext(context);
end:
  delete[] pDevices;
  return bResult;
}
