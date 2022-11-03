#include "CL/cl.h"
#include "FrameworkTest.h"
#include "cl_types.h"
#include <stdio.h>

extern cl_device_type gDeviceType;

/*******************************************************************************
 * clBuildProgram
 * -------------------
 * (1) get device ids
 * (2) create context
 * (3) create binary
 * (4) create program with source
 * (5) build program
 ******************************************************************************/

bool clBuildProgramMaxArgsTest() {
  bool bResult = true;
  const char *sample_large_parmam_kernel_pattern[] = {
      "__kernel void sample_test(%s, __global long *result)\n"
      "{\n"
      "result[0] = 0;\n"
      "%s"
      "\n"
      "}\n"};

  printf("clBuildProgramMaxArgsTest\n");
  cl_uint uiNumDevices = 0;
  cl_device_id *pDevices;
  size_t *pBinarySizes;
  cl_int *pBinaryStatus;
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
  if (CL_SUCCESS != iRet) {
    printf("clGetDeviceIDs = %s\n", ClErrTxt(iRet));
    return false;
  }

  // initialize arrays
  pDevices = new cl_device_id[uiNumDevices];
  pBinarySizes = new size_t[uiNumDevices];
  pBinaryStatus = new cl_int[uiNumDevices];

  iRet = clGetDeviceIDs(platform, gDeviceType, uiNumDevices, pDevices, NULL);
  if (CL_SUCCESS != iRet) {
    delete[] pDevices;
    delete[] pBinarySizes;
    delete[] pBinaryStatus;
    printf("clGetDeviceIDs = %s\n", ClErrTxt(iRet));
    return false;
  }

  // create context
  context = clCreateContext(prop, uiNumDevices, pDevices, NULL, NULL, &iRet);
  bResult &= Check("clCreateContext - queue", CL_SUCCESS, iRet);
  if (!bResult) {
    delete[] pDevices;
    delete[] pBinarySizes;
    delete[] pBinaryStatus;
    return false;
  }
  cl_command_queue queue = clCreateCommandQueueWithProperties(
      context, pDevices[0], NULL /*no properties*/, &iRet);
  bResult &=
      Check("clCreateCommandQueueWithProperties - queue", CL_SUCCESS, iRet);
  if (!bResult) {
    clReleaseContext(context);
    delete[] pDevices;
    delete[] pBinarySizes;
    delete[] pBinaryStatus;
    return false;
  }

  int i;
  size_t maxSize;
  char *programSrc;
  char *ptr;
  size_t numberOfIntParametersToTry, numberExpected;
  char *argumentLine, *codeLines;
  cl_long *longs;
  long long result, expectedResult;
  size_t decrement;
  cl_event event;
  cl_int event_status;

  /* Get the max param size */
  iRet = clGetDeviceInfo(pDevices[0], CL_DEVICE_MAX_PARAMETER_SIZE,
                         sizeof(maxSize), &maxSize, NULL);
  bResult &= Check("clGetDeviceInfo", CL_SUCCESS, iRet);
  if (!bResult) {
    clReleaseCommandQueue(queue);
    clReleaseContext(context);
    delete[] pDevices;
    delete[] pBinarySizes;
    delete[] pBinaryStatus;
    return bResult;
  }

  if (maxSize < 256) {
    printf("ERROR: Reported max parameter size is less than required! (%d)\n",
           (int)maxSize);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);
    delete[] pDevices;
    delete[] pBinarySizes;
    delete[] pBinaryStatus;
    return false;
  }

  numberOfIntParametersToTry = numberExpected =
      (maxSize - sizeof(cl_mem)) / sizeof(cl_long);
  decrement = (size_t)(numberOfIntParametersToTry / 8);
  if (decrement < 1)
    decrement = 1;
  printf("Reported max parameter size of %d bytes.\n", (int)maxSize);

  while (numberOfIntParametersToTry > 0) {
    bResult = true;
    cl_program prog;
    cl_kernel kernel;
    cl_mem mem;

    // These need to be inside to be deallocated automatically on each loop
    // iteration.
    printf(
        "Trying a kernel with %d long arguments (%d bytes) and one cl_mem (%d "
        "bytes) for %d bytes total.\n",
        (int)numberOfIntParametersToTry,
        (int)(sizeof(cl_long) * numberOfIntParametersToTry),
        (int)(sizeof(cl_mem)),
        (int)(sizeof(cl_mem) + numberOfIntParametersToTry * sizeof(cl_long)));

    // Allocate memory for the program storage
    longs = (cl_long *)malloc(sizeof(cl_long) * numberOfIntParametersToTry);
    argumentLine =
        (char *)malloc(sizeof(char) * numberOfIntParametersToTry * 32);
    codeLines = (char *)malloc(sizeof(char) * numberOfIntParametersToTry * 32);
    programSrc =
        (char *)malloc(sizeof(char) * (numberOfIntParametersToTry * 64 + 1024));
    argumentLine[0] = '\0';
    codeLines[0] = '\0';
    programSrc[0] = '\0';

    // Generate our results
    expectedResult = 0;
    for (i = 0; i < (int)numberOfIntParametersToTry; i++) {
      longs[i] = i;
      expectedResult += i;
    }

    // Build the program
    sprintf(argumentLine, "%s", "long long0");
    sprintf(codeLines, "%s", "result[0] += long0;");
    for (i = 1; i < (int)numberOfIntParametersToTry; i++) {
      sprintf(argumentLine, "%s, long long%d", argumentLine, i);
      sprintf(codeLines, "%s\nresult[0] += long%d;", codeLines, i);
    }

    /* Create a kernel to test with */
    sprintf(programSrc, sample_large_parmam_kernel_pattern[0], argumentLine,
            codeLines);
    ptr = programSrc;

    prog = clCreateProgramWithSource(
        context, 1, const_cast<const char **>(&ptr), NULL, &iRet);
    bResult &= Check("clCreateProgramWithSource", CL_SUCCESS, iRet);
    if (!bResult) {
      numberOfIntParametersToTry -= decrement;
      continue;
    }

    size_t *szSize = new size_t[uiNumDevices];
    for (unsigned int j = 0; j < uiNumDevices; j++) {
      szSize[j] = -1;
    }
    // get the binary, we should receive 0.
    iRet = clGetProgramInfo(prog, CL_PROGRAM_BINARY_SIZES,
                            sizeof(size_t) * uiNumDevices, szSize, NULL);
    bResult &=
        Check("clGetProgramInfo(CL_PROGRAM_BINARY_SIZES)", CL_SUCCESS, iRet);
    if (!bResult) {
      numberOfIntParametersToTry -= decrement;
      clReleaseProgram(prog);
      delete[] szSize;
      continue;
    }
    for (unsigned int j = 0; j < uiNumDevices; j++) {
      if (0 != szSize[j]) {
        numberOfIntParametersToTry -= decrement;
        clReleaseProgram(prog);
        delete[] szSize;
        continue;
      }
    }

    iRet = clBuildProgram(prog, uiNumDevices, pDevices, NULL, NULL, NULL);
    bResult &= Check("clBuildProgram", CL_SUCCESS, iRet);
    if (!bResult) {
      numberOfIntParametersToTry -= decrement;
      clReleaseProgram(prog);
      delete[] szSize;
      continue;
    }

    kernel = clCreateKernel(prog, "sample_test", &iRet);
    /* Try to set a large argument to the kernel */

    mem = clCreateBuffer(context, (cl_mem_flags)(CL_MEM_READ_WRITE),
                         sizeof(cl_long), NULL, &iRet);
    bResult &= Check("clCreateBuffer", CL_SUCCESS, iRet);
    if (!bResult) {
      numberOfIntParametersToTry -= decrement;
      clReleaseMemObject(mem);
      clReleaseKernel(kernel);
      clReleaseProgram(prog);
      delete[] szSize;
      continue;
    }

    for (i = 0; i < (int)numberOfIntParametersToTry; i++) {
      iRet = clSetKernelArg(kernel, i, sizeof(cl_long), &longs[i]);
      if (!Check("clSetKernelArg", CL_SUCCESS, iRet)) {
        numberOfIntParametersToTry -= decrement;
        break;
      }
    }
    if (CL_FAILED(iRet)) {
      numberOfIntParametersToTry -= decrement;
      clReleaseMemObject(mem);
      clReleaseKernel(kernel);
      clReleaseProgram(prog);
      delete[] szSize;
      continue;
    }

    iRet = clSetKernelArg(kernel, i, sizeof(cl_mem), &mem);
    if (!Check("clSetKernelArg", CL_SUCCESS, iRet)) {
      numberOfIntParametersToTry -= decrement;
      clReleaseMemObject(mem);
      clReleaseKernel(kernel);
      clReleaseProgram(prog);
      delete[] szSize;
      continue;
    }

    size_t globalDim[3] = {1, 1, 1}, localDim[3] = {1, 1, 1};
    iRet = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, globalDim, localDim,
                                  0, NULL, &event);
    if (!Check("clEnqueueNDRangeKernel", CL_SUCCESS, iRet)) {
      numberOfIntParametersToTry -= decrement;
      clReleaseMemObject(mem);
      clReleaseKernel(kernel);
      clReleaseProgram(prog);
      delete[] szSize;
      continue;
    }

    // Verify that the event does not return an error from the execution
    iRet = clWaitForEvents(1, &event);
    Check("clWaitForEvents", CL_SUCCESS, iRet);
    iRet = clGetEventInfo(event, CL_EVENT_COMMAND_EXECUTION_STATUS,
                          sizeof(event_status), &event_status, NULL);
    Check("clGetEventInfo", CL_SUCCESS, iRet);
    clReleaseEvent(event);
    if (event_status < 0)
      Check("Kernel execution event returned error", CL_SUCCESS, iRet);

    iRet = clEnqueueReadBuffer(queue, mem, CL_TRUE, 0, sizeof(cl_long), &result,
                               0, NULL, NULL);
    Check("clEnqueueReadBuffer", CL_SUCCESS, iRet);

    free(longs);
    free(argumentLine);
    free(codeLines);
    free(programSrc);
    delete[] szSize;

    clReleaseMemObject(mem);
    clReleaseKernel(kernel);
    clReleaseProgram(prog);
    if (result != expectedResult) {
      printf("Expected result (%lld) does not equal actual result (%lld).\n",
             expectedResult, result);
      numberOfIntParametersToTry -= decrement;
      continue;
    } else {
      printf(
          "Results verified at %d bytes of arguments.\n",
          (int)(sizeof(cl_mem) + numberOfIntParametersToTry * sizeof(cl_long)));
      break;
    }
  }

  clReleaseCommandQueue(queue);
  clReleaseContext(context);

  delete[] pDevices;
  delete[] pBinarySizes;
  delete[] pBinaryStatus;
  if (numberOfIntParametersToTry == numberExpected)
    return true;

  return false;
}
