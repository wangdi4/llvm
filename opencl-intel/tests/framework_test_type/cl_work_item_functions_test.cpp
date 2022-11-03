#include "CL/cl.h"
#include "FrameworkTest.h"
#include "cl_types.h"
#include <stdio.h>
#include <time.h>

//|
//| TEST: NativeKernelTest.cl_work_item_functions_test
//|
//| Purpose
//| -------
//|
//| Test all the work items functions (get_global_id\get_local_id ..) in
// OCL 6.11.1 | in both cases in-bound and out-of-bound dimindx, in order to get
// full coverage | for all the cases there's a need for constant and var
// versions of dimindx because | in constant version the in-bound check will be
// done in compile time (offline) and | in var version the inbound check will be
// done in kernel execution (online).
//|
//| Method
//| ------
//|
//| LOOP over all the WI functions:
//| 1. Create the desired kernel to check (each kernel checks one WI function)
//| 2. Create an inout buffer
//| 3. Enqueue a write into the inout buffer
//| 4. Enqueue execution of the selected kernel that tests the WI function (in
// all versions) | 5. Enqueue a read from the output buffer | 6. Compare if the
// output the same as the expected output
//|
//| Pass criteria
//| -------------
//|
//| In each iteration the output buffer should match the expected result.
//|

#define GLOBAL_SIZE 16
#define LOCAL_SIZE 4
#define KERNEL_NUM 7

extern cl_device_type gDeviceType;

size_t expected_output(const char *kernel_name, int work_item) {
  if (strcmp(kernel_name, "get_global_id_test") == 0)
    return work_item;
  if (strcmp(kernel_name, "get_local_id_test") == 0)
    return work_item % LOCAL_SIZE;
  if (strcmp(kernel_name, "get_global_size_test") == 0)
    return GLOBAL_SIZE;
  if (strcmp(kernel_name, "get_local_size_test") == 0)
    return LOCAL_SIZE;
  if (strcmp(kernel_name, "get_num_groups_test") == 0)
    return GLOBAL_SIZE / LOCAL_SIZE;
  if (strcmp(kernel_name, "get_group_id_test") == 0)
    return work_item / LOCAL_SIZE;
  if (strcmp(kernel_name, "get_global_offset_test") == 0)
    return 0;
  return 0;
}

/*******************************************************************************
 * clWorkItemFunctionsTest
 * -------------------
 * tests the OCL work item functions
 ******************************************************************************/
bool clWorkItemFunctionsTest() {
  bool bResult = true;
  cl_int iRet = CL_SUCCESS;
  cl_device_id clDefaultDeviceId;

  printf("=============================================================\n");
  printf("clWorkItemFunctionsTest\n");
  printf("=============================================================\n");

  const char *ocl_test_program[] = {
      "__kernel void get_global_size_test(__global size_t* pOutputs)\n"
      "{\n"
      "pOutputs[get_global_id(0) + 2 * get_global_size(0) ] = "
      "get_global_size(pOutputs[get_global_id(0) + 0 * get_global_size(0) ]);\n"
      "pOutputs[get_global_id(0) + 3 * get_global_size(0) ] = "
      "get_global_size(pOutputs[get_global_id(0) + 1 * get_global_size(0) ]);\n"
      "pOutputs[get_global_id(0) + 0 * get_global_size(0) ] = "
      "get_global_size(0);\n"
      "pOutputs[get_global_id(0) + 1 * get_global_size(0) ] = "
      "get_global_size(3);\n"
      "}"
      "__kernel void get_global_id_test(__global size_t* pOutputs)\n"
      "{\n"
      "pOutputs[get_global_id(0) + 2 * get_global_size(0) ] = "
      "get_global_id(pOutputs[get_global_id(0) + 0 * get_global_size(0) ]);\n"
      "pOutputs[get_global_id(0) + 3 * get_global_size(0) ] = "
      "get_global_id(pOutputs[get_global_id(0) + 1 * get_global_size(0) ]);\n"
      "pOutputs[get_global_id(0) + 0 * get_global_size(0) ] = "
      "get_global_id(0);\n"
      "pOutputs[get_global_id(0) + 1 * get_global_size(0) ] = "
      "get_global_id(3);\n"
      "}"
      "__kernel void get_local_size_test(__global size_t* pOutputs)\n"
      "{\n"
      "pOutputs[get_global_id(0) + 2 * get_global_size(0) ] = "
      "get_local_size(pOutputs[get_global_id(0) + 0 * get_global_size(0) ]);\n"
      "pOutputs[get_global_id(0) + 3 * get_global_size(0) ] = "
      "get_local_size(pOutputs[get_global_id(0) + 1 * get_global_size(0) ]);\n"
      "pOutputs[get_global_id(0) + 0 * get_global_size(0) ] = "
      "get_local_size(0);\n"
      "pOutputs[get_global_id(0) + 1 * get_global_size(0) ] = "
      "get_local_size(3);\n"
      "}"
      "__kernel void get_local_id_test(__global size_t* pOutputs)\n"
      "{\n"
      "pOutputs[get_global_id(0) + 2 * get_global_size(0) ] = "
      "get_local_id(pOutputs[get_global_id(0) + 0 * get_global_size(0) ]);\n"
      "pOutputs[get_global_id(0) + 3 * get_global_size(0) ] = "
      "get_local_id(pOutputs[get_global_id(0) + 1 * get_global_size(0) ]);\n"
      "pOutputs[get_global_id(0) + 0 * get_global_size(0) ] = "
      "get_local_id(0);\n"
      "pOutputs[get_global_id(0) + 1 * get_global_size(0) ] = "
      "get_local_id(3);\n"
      "}"
      "__kernel void get_num_groups_test(__global size_t* pOutputs)\n"
      "{\n"
      "pOutputs[get_global_id(0) + 2 * get_global_size(0) ] = "
      "get_num_groups(pOutputs[get_global_id(0) + 0 * get_global_size(0) ]);\n"
      "pOutputs[get_global_id(0) + 3 * get_global_size(0) ] = "
      "get_num_groups(pOutputs[get_global_id(0) + 1 * get_global_size(0) ]);\n"
      "pOutputs[get_global_id(0) + 0 * get_global_size(0) ] = "
      "get_num_groups(0);\n"
      "pOutputs[get_global_id(0) + 1 * get_global_size(0) ] = "
      "get_num_groups(3);\n"
      "}"
      "__kernel void get_group_id_test(__global size_t* pOutputs)\n"
      "{\n"
      "pOutputs[get_global_id(0) + 2 * get_global_size(0) ] = "
      "get_group_id(pOutputs[get_global_id(0) + 0 * get_global_size(0) ]);\n"
      "pOutputs[get_global_id(0) + 3 * get_global_size(0) ] = "
      "get_group_id(pOutputs[get_global_id(0) + 1 * get_global_size(0) ]);\n"
      "pOutputs[get_global_id(0) + 0 * get_global_size(0) ] = "
      "get_group_id(0);\n"
      "pOutputs[get_global_id(0) + 1 * get_global_size(0) ] = "
      "get_group_id(3);\n"
      "}"
      "__kernel void get_global_offset_test(__global size_t* pOutputs)\n"
      "{\n"
      "pOutputs[get_global_id(0) + 2 * get_global_size(0) ] = "
      "get_global_offset(pOutputs[get_global_id(0) + 0 * get_global_size(0) "
      "]);\n"
      "pOutputs[get_global_id(0) + 3 * get_global_size(0) ] = "
      "get_global_offset(pOutputs[get_global_id(0) + 1 * get_global_size(0) "
      "]);\n"
      "pOutputs[get_global_id(0) + 0 * get_global_size(0) ] = "
      "get_global_offset(0);\n"
      "pOutputs[get_global_id(0) + 1 * get_global_size(0) ] = "
      "get_global_offset(3);\n"
      "}"};

  cl_platform_id platform = 0;

  iRet = clGetPlatformIDs(1, &platform, NULL);
  bResult &= Check("clGetPlatformIDs", CL_SUCCESS, iRet);
  if (!bResult) {
    return bResult;
  }

  cl_context_properties prop[3] = {CL_CONTEXT_PLATFORM,
                                   (cl_context_properties)platform, 0};

  // Initiate test infrastructure:
  // Create context, Queue
  cl_context context =
      clCreateContextFromType(prop, gDeviceType, NULL, NULL, &iRet);
  bResult &= Check("clCreateContextFromType", CL_SUCCESS, iRet);
  if (!bResult) {
    return bResult;
  }

  iRet = clGetDeviceIDs(platform, gDeviceType, 1, &clDefaultDeviceId, NULL);
  bResult &= Check("clGetDeviceIDs", CL_SUCCESS, iRet);
  if (!bResult) {
    return bResult;
  }

  cl_command_queue queue = clCreateCommandQueueWithProperties(
      context, clDefaultDeviceId, NULL /*no properties*/, &iRet);
  bResult &=
      Check("clCreateCommandQueueWithProperties - queue", CL_SUCCESS, iRet);
  if (!bResult) {
    return bResult;
  }

  // create program
  cl_program program;
  if (!BuildProgramSynch(context, 1, (const char **)&ocl_test_program, NULL,
                         NULL, &program)) {
    return false;
  }

  const char *kernel_name[] = {"get_global_size_test",  "get_global_id_test",
                               "get_local_size_test",   "get_local_id_test",
                               "get_num_groups_test",   "get_group_id_test",
                               "get_global_offset_test"};

  size_t out_of_bound_value[] = {1, 0, 1, 0, 1, 0, 0};

  bool results[KERNEL_NUM];
  for (int i = 0; i < KERNEL_NUM; ++i) {
    results[i] = true;
  }

  cl_kernel kernel;
  cl_mem clBuff;

  const size_t stBuffSize = GLOBAL_SIZE * 4;
  size_t pBuff[stBuffSize];
  size_t pDstBuff[stBuffSize];

  size_t global_work_size[] = {GLOBAL_SIZE};
  size_t local_work_size[] = {LOCAL_SIZE};
  size_t global_work_offset[] = {0};

  for (int i = 0; i < KERNEL_NUM; ++i) {
    kernel = clCreateKernel(program, kernel_name[i], &iRet);
    results[i] &= Check("clCreateKernel", CL_SUCCESS, iRet);
    if (!results[i]) {
      return results[i];
    }

    // fill with in bound
    for (unsigned ui = 0; ui < GLOBAL_SIZE; ui++) {
      pBuff[ui] = 0;
    }

    // fill with out bound
    for (unsigned ui = GLOBAL_SIZE; ui < 2 * GLOBAL_SIZE; ui++) {
      pBuff[ui] = 5;
    }

    clBuff = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
                            sizeof(pBuff), pBuff, &iRet);
    results[i] &= Check("clCreateBuffer", CL_SUCCESS, iRet);
    if (!results[i]) {
      return false;
    }

    // Set Kernel Arguments
    iRet = clSetKernelArg(kernel, 0, sizeof(cl_mem), &clBuff);
    results[i] &= Check("clSetKernelArg - clBuff", CL_SUCCESS, iRet);
    if (!results[i]) {
      return false;
    }

    // Execute kernel
    iRet = clEnqueueNDRangeKernel(queue, kernel, 1, global_work_offset,
                                  global_work_size, local_work_size, 0, NULL,
                                  NULL);
    results[i] &= Check("clEnqueueNDRangeKernel", CL_SUCCESS, iRet);

    // Verification phase
    iRet = clEnqueueReadBuffer(queue, clBuff, CL_TRUE, 0, sizeof(pDstBuff),
                               pDstBuff, 0, NULL, NULL);
    results[i] &= Check("clEnqueueReadBuffer - Dst", CL_SUCCESS, iRet);
    if (!results[i]) {
      return false;
    }

    // 0. const in-bound with dimindx = 0
    for (unsigned y = 0; y < GLOBAL_SIZE; ++y) {
      if (pDstBuff[y] != expected_output(kernel_name[i], y)) {
        results[i] = false;
      }
    }
    // 1. const out-of-bound
    for (unsigned y = GLOBAL_SIZE; y < 2 * GLOBAL_SIZE; ++y) {
      if (pDstBuff[y] != out_of_bound_value[i]) {
        results[i] = false;
      }
    }
    // 2. var  in-bound
    for (unsigned y = 2 * GLOBAL_SIZE; y < 3 * GLOBAL_SIZE; ++y) {
      if (pDstBuff[y] != expected_output(kernel_name[i], y - 2 * GLOBAL_SIZE)) {
        results[i] = false;
      }
    }
    // 3. var  out-of-bound
    for (unsigned y = 3 * GLOBAL_SIZE; y < 4 * GLOBAL_SIZE; ++y) {
      if (pDstBuff[y] != out_of_bound_value[i]) {
        results[i] = false;
      }
    }

    if (results[i]) {
      printf("*** %s compare verification succeeded *** \n", kernel_name[i]);
    } else {
      printf("!!!!!! %s compare verification failed !!!!! \n", kernel_name[i]);
    }

    // release_buffer
    clReleaseMemObject(clBuff);
    // release_kernel
    clReleaseKernel(kernel);
  }

  // release_program
  clReleaseProgram(program);
  // release_queue
  clFinish(queue);
  clReleaseCommandQueue(queue);
  // release_context
  clReleaseContext(context);
  // release_end
  for (int i = 0; i < KERNEL_NUM; i++) {
    bResult = bResult & results[i];
  }
  return bResult;
}
