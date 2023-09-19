#include "CL/cl.h"
#include "FrameworkTest.h"
#include "cl_types.h"
#include <algorithm>
#include <cstring>
#include <stdio.h>
#include <time.h>

#define SUB_BUFFER_SIZE 512

bool validateData(unsigned int numDevices, unsigned int numRounds,
                  unsigned int numIncInRound, unsigned int round,
                  unsigned int deviceRound, unsigned int readNum,
                  unsigned int expectedFrom, unsigned int bufferFrom,
                  unsigned int size, cl_long *buff) {
  bool result = true;
  for (unsigned int ui = 0; ui < size; ui++) {
    cl_long initialValue =
        ((expectedFrom + ui) * numIncInRound * numDevices * numRounds) +
        (expectedFrom + ui + 1);
    cl_long expectedValue =
        initialValue + ((round * numDevices * numIncInRound) +
                        (deviceRound * numIncInRound) + readNum);
    if (buff[bufferFrom + ui] != expectedValue) {
#if defined(_WIN32)
      printf("Validation error: Buff[%d] = %lld, expected %lld\n",
             (bufferFrom + ui), buff[bufferFrom + ui], expectedValue);
#else
      printf("Validation error: Buff[%d] = %ld, expected %ld\n",
             (bufferFrom + ui), buff[bufferFrom + ui], expectedValue);
#endif
      result = false;
      break;
    }
  }
  return result;
}

/* test_name - The test name.
        dev_types - list of device types.
        num_expected_devices_of_type - list that indicate in
   num_expected_devices_of_type[i] the devices amount of type dev_types[i] that
   you expected (Can be less or equal to this value). If
   num_expected_devices_of_type == NULL and dev_types[i] != "CL_DEVICE_TYPE_ALL"
   than using one device for each type that exist. If dev_types[i] ==
   "CL_DEVICE_TYPE_ALL" than ignoring num_expected_devices_of_type and using all
   devices. numDeviceTypes - The list size of dev_types and
   num_expected_devices_of_type (If not NULL). releaseBuffTest - True if want to
   run releaseBuffTest. */
bool run_common_rt_sub_buffers_async_test(
    const char *test_name, cl_device_type *dev_types,
    unsigned int *num_expected_devices_of_type, unsigned int numDeviceTypes,
    bool releaseBuffTest) {
  if ((NULL == dev_types) || (0 == numDeviceTypes)) {
    return false;
  }
  unsigned int *pNumExpectedDeviceOfType =
      (NULL == num_expected_devices_of_type)
          ? (unsigned int *)alloca(sizeof(unsigned int) * numDeviceTypes)
          : num_expected_devices_of_type;
  if (NULL == pNumExpectedDeviceOfType) {
    return false;
  }
  if (NULL == num_expected_devices_of_type) {
    for (unsigned int i = 0; i < numDeviceTypes; i++) {
      pNumExpectedDeviceOfType[i] = 1;
    }
  }
  for (unsigned int i = 0; i < numDeviceTypes; i++) {
    if (dev_types[i] == CL_DEVICE_TYPE_ALL) {
      dev_types[0] = CL_DEVICE_TYPE_ALL;
      pNumExpectedDeviceOfType[0] = (unsigned int)-1; // max unsigned int.
      numDeviceTypes = 1;
      break;
    }
  }

  bool bResult = true;
  cl_int iRet = CL_SUCCESS;

  const unsigned int number_of_rounds = 512;
  unsigned int num_inc_in_round = 2;
  if (releaseBuffTest) {
    num_inc_in_round = 16;
  }

  cl_device_type *pDevTypes =
      (cl_device_type *)alloca(sizeof(cl_device_type) * numDeviceTypes);
  unsigned int *pNumDevicesOfType =
      (unsigned int *)alloca(sizeof(unsigned int) * numDeviceTypes);
  if ((NULL == pDevTypes) || (NULL == pNumDevicesOfType)) {
    return false;
  }
  cl_device_id *devices = NULL;
  cl_command_queue *dev_queue = NULL;
  cl_command_queue *tDev_queue = NULL;
  cl_mem *clSubBuffs = NULL;

  size_t global_work_size[1];

  cl_program program;
  cl_kernel kernel;

  printf("=============================================================\n");
  printf("%s\n", test_name);
  printf("=============================================================\n");

  const char *ocl_test_program[] = {
      "__kernel void int_test(__global long* pValues)\n"
      "{\n"
      "   size_t x = get_global_id(0);\n"
      "   long val = pValues[x];\n"
      "   long res = val + 1;\n"
      "   pValues[x] = res;\n"
      "}"};

  cl_platform_id platform = 0;

  iRet = clGetPlatformIDs(1, &platform, NULL);
  bResult &= SilentCheck("clGetPlatformIDs", CL_SUCCESS, iRet);

  if (!bResult) {
    return bResult;
  }

  unsigned int numDevices = 0;
  unsigned int currDeviceNumDevicesRet = 0;
  unsigned int currIndex = 0;
  for (unsigned int i = 0; i < numDeviceTypes; i++) {
    iRet = clGetDeviceIDs(platform, dev_types[i], 0, NULL,
                          &currDeviceNumDevicesRet);
    if ((CL_DEVICE_NOT_FOUND == iRet) && (CL_DEVICE_TYPE_CPU != dev_types[i])) {
      // dev_types[i] not found (which is not CPU device) - skipping the test
      bResult &= SilentCheck("clGetDeviceIDs for ", CL_DEVICE_NOT_FOUND, iRet);
      printf("*** %s is not found and require by the test, skipping the test "
             "***\n",
             dev_types[i] == CL_DEVICE_TYPE_CPU   ? "CL_DEVICE_TYPE_CPU"
             : dev_types[i] == CL_DEVICE_TYPE_GPU ? "CL_DEVICE_TYPE_GPU"
             : dev_types[i] == CL_DEVICE_TYPE_ACCELERATOR
                 ? "CL_DEVICE_TYPE_ACCELERATOR"
                 : "Error");
      return bResult;
    }
    bResult &= SilentCheck("clGetDeviceIDs", CL_SUCCESS, iRet);
    if (!bResult) {
      return bResult;
    }
    // This device type does not exist
    if (0 == currDeviceNumDevicesRet) {
      continue;
    }
    pDevTypes[currIndex] = dev_types[i];
    pNumDevicesOfType[currIndex] =
        std::min(currDeviceNumDevicesRet, pNumExpectedDeviceOfType[i]);
    numDevices += pNumDevicesOfType[currIndex];
    currIndex++;
  }

  if (numDevices < 2) {
    printf("*** This test require at least 2 devices, skipping the test ***\n");
    return bResult;
  }

  devices = (cl_device_id *)alloca(sizeof(cl_device_id) * numDevices);
  dev_queue = (cl_command_queue *)alloca(sizeof(cl_command_queue) * numDevices);
  tDev_queue =
      (cl_command_queue *)alloca(sizeof(cl_command_queue) * numDevices);
  clSubBuffs = (cl_mem *)alloca(sizeof(cl_mem) * numDevices);

  if ((NULL == devices) || (NULL == dev_queue) || (NULL == tDev_queue) ||
      (NULL == clSubBuffs)) {
    return false;
  }

  unsigned int devicesIndex = 0;
  for (unsigned int i = 0; i < numDeviceTypes; i++) {
    unsigned int num_devices_ret = 0;
    iRet = clGetDeviceIDs(platform, pDevTypes[i], pNumDevicesOfType[i],
                          &(devices[devicesIndex]), &num_devices_ret);
    bResult &= SilentCheck("clGetDeviceIDs", CL_SUCCESS, iRet);
    if ((!bResult) || (num_devices_ret < pNumDevicesOfType[i])) {
      return false;
    }
    devicesIndex += pNumDevicesOfType[i];
  }

  printf("%d devices available\n", numDevices);
  for (unsigned int i = 0; i < numDevices; i++) {
    cl_device_type deviceType;
    clGetDeviceInfo(devices[i], CL_DEVICE_TYPE, sizeof(cl_device_type),
                    &deviceType, NULL);
    printf("Device - %d is %s\n", i,
           deviceType == CL_DEVICE_TYPE_CPU   ? "CL_DEVICE_TYPE_CPU"
           : deviceType == CL_DEVICE_TYPE_GPU ? "CL_DEVICE_TYPE_GPU"
           : deviceType == CL_DEVICE_TYPE_ACCELERATOR
               ? "CL_DEVICE_TYPE_ACCELERATOR"
               : "Error");
  }

  const size_t stBuffSize = SUB_BUFFER_SIZE * numDevices;
  const size_t stSubBuffSize = SUB_BUFFER_SIZE;
  const size_t stBuffSizeInBytes = stBuffSize * sizeof(cl_long);
  const size_t stSubBuffSizeInBytes = stSubBuffSize * sizeof(cl_long);
  cl_long *pBuff = (cl_long *)alloca(stBuffSizeInBytes);
  cl_long *pDestBuff = (cl_long *)alloca(sizeof(cl_long) * stBuffSize);
  cl_long **pDstSubBuffs = (cl_long **)alloca(sizeof(cl_long *) * numDevices);
  if ((NULL == pBuff) || (NULL == pDestBuff) || (NULL == pDstSubBuffs)) {
    return false;
  }
  for (unsigned int i = 0; i < numDevices; i++) {
    pDstSubBuffs[i] = (cl_long *)alloca(sizeof(cl_long) * stSubBuffSize);
    if (NULL == pDstSubBuffs[i]) {
      return false;
    }
  }

  cl_mem clParentBuff = NULL;

  //
  // Initiate test infrastructure:
  // Create context, Queue
  //
  cl_context_properties prop[3] = {CL_CONTEXT_PLATFORM,
                                   (cl_context_properties)platform, 0};
  cl_context context =
      clCreateContext(prop, numDevices, devices, NULL, NULL, &iRet);
  bResult &= SilentCheck("clCreateContext", CL_SUCCESS, iRet);
  if (!bResult)
    return bResult;

  // create program with source
  if (!BuildProgramSynch(context, 1, (const char **)&ocl_test_program, NULL,
                         NULL, &program)) {
    goto release_context;
  }

  kernel = clCreateKernel(program, "int_test", &iRet);
  bResult &= SilentCheck("clCreateKernel - int_test", CL_SUCCESS, iRet);
  if (!bResult)
    goto release_program;

  // fill with data.
  for (unsigned int ui = 0; ui < stBuffSize; ui++) {
    pBuff[ui] =
        (cl_long)((ui * num_inc_in_round * numDevices * number_of_rounds) +
                  (ui + 1));
  }

  clParentBuff =
      clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
                     stBuffSizeInBytes, pBuff, &iRet);
  bResult &= SilentCheck("clCreateBuffer", CL_SUCCESS, iRet);
  if (!bResult) {
    goto release_context;
  }

  cl_buffer_region region;
  region.origin = 0;
  region.size = stSubBuffSizeInBytes;
  for (unsigned int i = 0; i < numDevices; i++) {
    clSubBuffs[i] =
        clCreateSubBuffer(clParentBuff, CL_MEM_READ_WRITE,
                          CL_BUFFER_CREATE_TYPE_REGION, &region, &iRet);
    bResult &= SilentCheck("clCreateSubBuffer", CL_SUCCESS, iRet);
    if (!bResult) {
      goto release_buffers;
    }
    if (i < numDevices - 1) {
      region.origin += stSubBuffSizeInBytes;
    }
  }

  for (unsigned int i = 0; i < numDevices; i++) {
    dev_queue[i] = clCreateCommandQueueWithProperties(
        context, devices[i], 0 /*no properties*/, &iRet);
    bResult &=
        SilentCheck("clCreateCommandQueueWithProperties", CL_SUCCESS, iRet);
    if (!bResult)
      goto release_queues;
  }

  if (releaseBuffTest) {
    // cl_CPU_MIC_Common_RT_SubBuffers_Async_With_Buffer_Release test.
    global_work_size[0] = stSubBuffSize;

    for (unsigned int round = 0; round < number_of_rounds; ++round) {
      for (unsigned int curr_dev = 0; curr_dev < numDevices; ++curr_dev) {
        for (unsigned int curr_queue = 0; curr_queue < numDevices;
             ++curr_queue) {
          tDev_queue[curr_queue] =
              dev_queue[(curr_dev + curr_queue) % numDevices];
        }

        cl_event ev = clCreateUserEvent(context, &iRet);
        bResult &= SilentCheck("clCreateUserEvent", CL_SUCCESS, iRet);
        if (!bResult)
          break;
        cl_event *pEv = &ev;
        unsigned int numOfEvents = 1;

        for (unsigned int i = 0; i < num_inc_in_round; i++) {
          for (unsigned int d = 0; d < numDevices; d++) {
            // Set Kernel Arguments
            iRet = clSetKernelArg(kernel, 0, sizeof(cl_mem), &(clSubBuffs[d]));
            bResult &=
                SilentCheck("clSetKernelArg - clSubBuffA", CL_SUCCESS, iRet);
            if (!bResult)
              break;
            // Execute kernel
            iRet = clEnqueueNDRangeKernel(tDev_queue[d], kernel, 1, NULL,
                                          global_work_size, NULL, numOfEvents,
                                          pEv, NULL);
            bResult &= SilentCheck("clEnqueueNDRangeKernel for queueA",
                                   CL_SUCCESS, iRet);
            if (!bResult)
              break;
          }
          pEv = NULL;
          numOfEvents = 0;
          if (!bResult)
            break;
        }
        if (!bResult)
          break;

        // Release the subBuffer before executing all the commands with this
        // subBuffer and create new subBuffer on the same region --> it change
        // the memory mode to overlapping until all the commands will complete.
        clReleaseMemObject(clSubBuffs[numDevices - 1]);
        clSubBuffs[numDevices - 1] = NULL;
        clSubBuffs[numDevices - 1] =
            clCreateSubBuffer(clParentBuff, CL_MEM_READ_WRITE,
                              CL_BUFFER_CREATE_TYPE_REGION, &region, &iRet);
        bResult &= SilentCheck("clCreateSubBuffer", CL_SUCCESS, iRet);
        if (!bResult)
          break;

        // triger the batch ndranges.
        clSetUserEventStatus(ev, CL_COMPLETE);
        for (unsigned int d = 0; d < numDevices; d++) {
          iRet = clFinish(tDev_queue[d]);
          bResult &= SilentCheck("clFinish for queueA", CL_SUCCESS, iRet);
          if (!bResult)
            break;
        }
        if (!bResult)
          break;

        if (round % 2 == 0) {
          // map all subBuffers and validate the content.
          cl_event ev1 = clCreateUserEvent(context, &iRet);
          bResult &= SilentCheck("clCreateUserEvent", CL_SUCCESS, iRet);
          if (!bResult)
            break;

          cl_long **mapPtrArr =
              (cl_long **)alloca(sizeof(cl_long *) * numDevices);
          for (unsigned int d = 0; d < numDevices; d++) {
            mapPtrArr[d] = (cl_long *)clEnqueueMapBuffer(
                tDev_queue[d], clSubBuffs[d], CL_FALSE, CL_MAP_READ, 0,
                stSubBuffSizeInBytes, 1, &ev1, NULL, &iRet);
            bResult &= SilentCheck(
                "clEnqueueMapBuffer from queueB, clSubBuffA to mapPtr1",
                CL_SUCCESS, iRet);
            if (!bResult)
              break;
          }
          if (!bResult)
            break;

          clSetUserEventStatus(ev1, CL_COMPLETE);
          for (unsigned int d = 0; d < numDevices; d++) {
            iRet = clFinish(tDev_queue[d]);
            bResult &= SilentCheck("clFinish for queueA", CL_SUCCESS, iRet);
            if (!bResult)
              break;
          }
          if (!bResult)
            break;
          for (unsigned int d = 0; d < numDevices; d++) {
            if (!validateData(numDevices, number_of_rounds, num_inc_in_round,
                              round, curr_dev, num_inc_in_round,
                              d * stSubBuffSize, 0, stSubBuffSize,
                              mapPtrArr[d])) {
              bResult = false;
              break;
            }
          }
          if (!bResult)
            break;
          for (unsigned int d = 0; d < numDevices; d++) {
            iRet = clEnqueueUnmapMemObject(tDev_queue[d], clSubBuffs[d],
                                           mapPtrArr[d], 0, NULL, NULL);
            bResult &= SilentCheck("clEnqueueUnMapBuffer - clSubBuffA",
                                   CL_SUCCESS, iRet);
            if (!bResult)
              break;
          }
          if (!bResult)
            break;
        } else {
          // Read parent buffer. device B
          iRet = clEnqueueReadBuffer(tDev_queue[numDevices - 1], clParentBuff,
                                     CL_TRUE, 0, stBuffSizeInBytes, pDestBuff,
                                     0, NULL, NULL);
          bResult &=
              SilentCheck("clEnqueueReadBuffer - clParentBuff to pDestBuff",
                          CL_SUCCESS, iRet);
          if (!bResult)
            break;
          if (!validateData(numDevices, number_of_rounds, num_inc_in_round,
                            round, curr_dev, num_inc_in_round, 0, 0, stBuffSize,
                            pDestBuff)) {
            bResult = false;
            break;
          }
        }
        for (unsigned int d = 0; d < numDevices; d++) {
          iRet = clFinish(tDev_queue[d]);
          bResult &= SilentCheck("clFinish for queueA", CL_SUCCESS, iRet);
          if (!bResult)
            break;
        }
        if (!bResult)
          break;
      }

      if (!bResult)
        break;
    }
  } else {
    // cl_CPU_MIC_Common_RT_SubBuffers_Async test
    for (unsigned int round = 0; round < number_of_rounds; ++round) {
      for (unsigned int curr_dev = 0; curr_dev < numDevices; ++curr_dev) {
        for (unsigned int curr_queue = 0; curr_queue < numDevices;
             ++curr_queue) {
          tDev_queue[curr_queue] =
              dev_queue[(curr_dev + curr_queue) % numDevices];
        }

        // Write to Parent buffer on device A.

        // Set Kernel Arguments
        iRet = clSetKernelArg(kernel, 0, sizeof(cl_mem), &clParentBuff);
        bResult &=
            SilentCheck("clSetKernelArg - clParentBuff", CL_SUCCESS, iRet);
        if (!bResult)
          break;
        global_work_size[0] = stBuffSize;
        // Execute kernel
        iRet = clEnqueueNDRangeKernel(tDev_queue[0], kernel, 1, NULL,
                                      global_work_size, NULL, 0, NULL, NULL);
        bResult &=
            SilentCheck("clEnqueueNDRangeKernel for queueA", CL_SUCCESS, iRet);
        if (!bResult)
          break;
        iRet = clFinish(tDev_queue[0]);
        bResult &= SilentCheck("clFinish for queueA", CL_SUCCESS, iRet);
        if (!bResult)
          break;

        cl_event ev1 = clCreateUserEvent(context, &iRet);
        bResult &= SilentCheck("clCreateUserEvent", CL_SUCCESS, iRet);
        if (!bResult)
          break;

        // Read parent from child buffers. will triger when ev1 signal. (on both
        // devices simultaneously)
        for (unsigned int d = 0; d < numDevices; d++) {
          iRet = clEnqueueReadBuffer(tDev_queue[d], clSubBuffs[d], CL_FALSE, 0,
                                     stSubBuffSizeInBytes, pDstSubBuffs[d], 1,
                                     &ev1, NULL);
          bResult &=
              SilentCheck("clEnqueueReadSubBuffer - clSubBuffA to pDstSubBuffA",
                          CL_SUCCESS, iRet);
          if (!bResult)
            break;
        }
        if (!bResult)
          break;
        clSetUserEventStatus(ev1, CL_COMPLETE);
        for (unsigned int d = 0; d < numDevices; d++) {
          iRet = clFinish(tDev_queue[d]);
          bResult &= SilentCheck("clFinish for queueA", CL_SUCCESS, iRet);
          if (!bResult)
            break;
        }
        if (!bResult)
          break;

        for (unsigned int d = 0; d < numDevices; d++) {
          if (!validateData(numDevices, number_of_rounds, num_inc_in_round,
                            round, curr_dev, 1, d * stSubBuffSize, 0,
                            stSubBuffSize, pDstSubBuffs[d])) {
            bResult = false;
            break;
          }
        }
        if (!bResult)
          break;

        // write to child buffers. (on both devices simultaneously)
        cl_event ev2 = clCreateUserEvent(context, &iRet);
        bResult &= SilentCheck("clCreateUserEvent", CL_SUCCESS, iRet);
        if (!bResult)
          break;
        global_work_size[0] = stSubBuffSize;
        // Set Kernel Arguments
        for (unsigned int d = 0; d < numDevices; d++) {
          // Set Kernel Arguments
          iRet = clSetKernelArg(kernel, 0, sizeof(cl_mem), &(clSubBuffs[d]));
          bResult &=
              SilentCheck("clSetKernelArg - clSubBuffA", CL_SUCCESS, iRet);
          if (!bResult)
            break;
          // Execute kernel
          iRet = clEnqueueNDRangeKernel(tDev_queue[numDevices - 1 - d], kernel,
                                        1, NULL, global_work_size, NULL, 1,
                                        &ev2, NULL);
          bResult &= SilentCheck("clEnqueueNDRangeKernel for queueA",
                                 CL_SUCCESS, iRet);
          if (!bResult)
            break;
        }
        if (!bResult)
          break;
        clSetUserEventStatus(ev2, CL_COMPLETE);
        for (int d = numDevices - 1; d >= 0; d--) {
          iRet = clFinish(tDev_queue[d]);
          bResult &= SilentCheck("clFinish for queueB", CL_SUCCESS, iRet);
          if (!bResult)
            break;
        }
        if (!bResult)
          break;

        // Map parent buffer. On device B
        cl_long *mapPtr = (cl_long *)clEnqueueMapBuffer(
            tDev_queue[numDevices - 1], clParentBuff, CL_TRUE, CL_MAP_READ, 0,
            stBuffSizeInBytes, 0, NULL, NULL, &iRet);
        bResult &= SilentCheck(
            "clEnqueueMapBuffer from queueB, clParentBuff to mapPtr",
            CL_SUCCESS, iRet);
        if (!bResult)
          break;
        if (!validateData(numDevices, number_of_rounds, num_inc_in_round, round,
                          curr_dev, 2, 0, 0, stBuffSize, mapPtr)) {
          bResult = false;
          break;
        }

        iRet = clEnqueueUnmapMemObject(tDev_queue[numDevices - 1], clParentBuff,
                                       mapPtr, 0, NULL, NULL);
        bResult &=
            SilentCheck("clEnqueueMapBuffer - clParentBuff", CL_SUCCESS, iRet);
        if (!bResult)
          break;
        for (int d = numDevices - 1; d >= 0; d--) {
          iRet = clFinish(tDev_queue[d]);
          bResult &= SilentCheck("clFinish for queueB", CL_SUCCESS, iRet);
          if (!bResult)
            break;
        }
        if (!bResult)
          break;
      }

      if (!bResult)
        break;
    }
  }

release_queues:
  for (unsigned int curr_dev = 0; curr_dev < numDevices; ++curr_dev) {
    if (NULL == dev_queue[curr_dev]) {
      continue;
    }
    clFinish(dev_queue[curr_dev]);
    clReleaseCommandQueue(dev_queue[curr_dev]);
  }
release_buffers:
  for (unsigned int curr_dev = 0; curr_dev < numDevices; ++curr_dev) {
    if (NULL != clSubBuffs[curr_dev]) {
      clReleaseMemObject(clSubBuffs[curr_dev]);
    }
  }
  if (NULL != clParentBuff) {
    clReleaseMemObject(clParentBuff);
  }
  clReleaseKernel(kernel);
release_program:
  clReleaseProgram(program);
release_context:
  clReleaseContext(context);

  return bResult;
}

/******************************************************************************/
bool run_multi_devices_sub_buffer_simple_test(const char *test_name) {
  const size_t element_size = sizeof(cl_long);
  const size_t num_element_in_sub_buffer = 512;
  const size_t sub_buffer_size_in_bytes =
      num_element_in_sub_buffer * element_size;
  bool bResult = true;
  cl_int iRet = CL_SUCCESS;

  cl_long *pBuff = NULL;

  cl_platform_id platform = 0;
  cl_context context = NULL;
  cl_uint num_devices = 0;
  cl_device_id *pDevices = NULL;
  cl_mem clParentBuff = NULL;
  cl_mem *clSubBuffArr = NULL;

  cl_program program = NULL;
  cl_kernel kernel = NULL;

  cl_command_queue *clCommandQueues = NULL;

  printf("=============================================================\n");
  printf("%s\n", test_name);
  printf("=============================================================\n");

  const char *ocl_test_program[] = {
      "__kernel void int_test(__global long* pValues)\n"
      "{\n"
      "   size_t x = get_global_id(0);\n"
      "   long val = pValues[x];\n"
      "   long res = val + 1;\n"
      "   pValues[x] = res;\n"
      "}"};

  do {
    iRet = clGetPlatformIDs(1, &platform, NULL);
    bResult &= SilentCheck("clGetPlatformIDs", CL_SUCCESS, iRet);

    if (!bResult) {
      break;
    }

    iRet = clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, 0, NULL, &num_devices);
    bResult &= SilentCheck("clGetDeviceIDs", CL_SUCCESS, iRet);

    if ((!bResult) || (0 == num_devices)) {
      break;
    }

    if (num_devices < 2) {
      printf(
          "*** This test require at least 2 devices, skipping the test ***\n");
      break;
    }

    assert(num_devices > 0);
    pDevices = new cl_device_id[num_devices];

    cl_uint num_devices_ret = 0;
    iRet = clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, num_devices, pDevices,
                          &num_devices_ret);
    bResult &= SilentCheck("clGetDeviceIDs", CL_SUCCESS, iRet);

    if ((!bResult) || (num_devices_ret != num_devices)) {
      bResult = false;
      break;
    }

    cl_device_type deviceType;
    printf("%d devices available\n", num_devices);
    for (unsigned int i = 0; i < num_devices; i++) {
      clGetDeviceInfo(pDevices[i], CL_DEVICE_TYPE, sizeof(cl_device_type),
                      &deviceType, NULL);
      printf("Device - %d is %s\n", i,
             deviceType == CL_DEVICE_TYPE_CPU   ? "CL_DEVICE_TYPE_CPU"
             : deviceType == CL_DEVICE_TYPE_GPU ? "CL_DEVICE_TYPE_GPU"
             : deviceType == CL_DEVICE_TYPE_ACCELERATOR
                 ? "CL_DEVICE_TYPE_ACCELERATOR"
                 : "Error");
    }

    cl_context_properties prop[3] = {CL_CONTEXT_PLATFORM,
                                     (cl_context_properties)platform, 0};
    context = clCreateContext(prop, num_devices, pDevices, NULL, NULL, &iRet);
    bResult &= SilentCheck("clCreateContext", CL_SUCCESS, iRet);
    if (!bResult) {
      break;
    }

    pBuff = (cl_long *)malloc(sub_buffer_size_in_bytes * num_devices);
    if (NULL == pBuff) {
      bResult = false;
      break;
    }

    for (unsigned int i = 0; i < num_element_in_sub_buffer * num_devices; i++) {
      pBuff[i] = i * 2;
    }

    clParentBuff =
        clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
                       sub_buffer_size_in_bytes * num_devices, pBuff, &iRet);
    bResult &= SilentCheck("clCreateBuffer", CL_SUCCESS, iRet);
    if (!bResult) {
      break;
    }

    clSubBuffArr = new cl_mem[num_devices];

    cl_buffer_region region;
    region.origin = 0;
    region.size = sub_buffer_size_in_bytes;
    for (unsigned int i = 0; i < num_devices; i++) {
      clSubBuffArr[i] =
          clCreateSubBuffer(clParentBuff, CL_MEM_READ_WRITE,
                            CL_BUFFER_CREATE_TYPE_REGION, &region, &iRet);
      bResult &= SilentCheck("clCreateSubBuffer", CL_SUCCESS, iRet);
      if (!bResult) {
        break;
      }
      region.origin += sub_buffer_size_in_bytes;
    }
    if (!bResult) {
      break;
    }

    // create program with source
    if (!BuildProgramSynch(context, 1, (const char **)&ocl_test_program, NULL,
                           NULL, &program)) {
      bResult = false;
      break;
    }

    kernel = clCreateKernel(program, "int_test", &iRet);
    bResult &= SilentCheck("clCreateKernel - int_test", CL_SUCCESS, iRet);
    if (!bResult) {
      break;
    }

    clCommandQueues = new cl_command_queue[num_devices];

    for (unsigned int i = 0; i < num_devices; i++) {
      clCommandQueues[i] = clCreateCommandQueueWithProperties(
          context, pDevices[i], NULL /*no properties*/, &iRet);
      bResult &=
          SilentCheck("clCreateCommandQueueWithProperties", CL_SUCCESS, iRet);
      if (!bResult) {
        break;
      }
    }
    if (!bResult) {
      break;
    }

    cl_event ev = clCreateUserEvent(context, &iRet);
    bResult &= SilentCheck("clCreateUserEvent", CL_SUCCESS, iRet);
    if (!bResult) {
      break;
    }
    size_t global_work_size[1];
    global_work_size[0] = num_element_in_sub_buffer;
    // write to child buffers. (on all devices simultaneously)
    for (unsigned int i = 0; i < num_devices; i++) {
      // Set Kernel Arguments
      iRet = clSetKernelArg(kernel, 0, sizeof(cl_mem), &(clSubBuffArr[i]));
      bResult &= SilentCheck("clSetKernelArg - clSubBuffArr", CL_SUCCESS, iRet);
      if (!bResult) {
        break;
      }
      // Execute kernel
      iRet = clEnqueueNDRangeKernel(clCommandQueues[i], kernel, 1, NULL,
                                    global_work_size, NULL, 1, &ev, NULL);
      bResult &=
          SilentCheck("clEnqueueNDRangeKernel for queueB", CL_SUCCESS, iRet);
      if (!bResult) {
        break;
      }
    }
    clSetUserEventStatus(ev, CL_COMPLETE);
    if (!bResult) {
      break;
    }
    for (unsigned int i = 0; i < num_devices; i++) {
      iRet = clFinish(clCommandQueues[i]);
      bResult &=
          SilentCheck("clFinish for clCommandQueues[i]", CL_SUCCESS, iRet);
      if (!bResult)
        break;
    }
    if (!bResult) {
      break;
    }

    // Map parent buffer. On first device
    cl_long *mapPtr = (cl_long *)clEnqueueMapBuffer(
        clCommandQueues[0], clParentBuff, CL_TRUE, CL_MAP_READ, 0,
        sub_buffer_size_in_bytes * num_devices, 0, NULL, NULL, &iRet);
    bResult &= SilentCheck(
        "clEnqueueMapBuffer from clCommandQueues[0], clParentBuff to mapPtr",
        CL_SUCCESS, iRet);
    if (!bResult) {
      break;
    }
    // Validating data
    for (unsigned int i = 0; i < num_element_in_sub_buffer * num_devices; i++) {
      if (mapPtr[i] != i * 2 + 1) {
#if defined(_WIN32)
        printf("Error: Validation error at index %u, "
               "expected %u got %lld\n",
               i, i * 2 + 1, mapPtr[i]);
#else
        printf("Error: Validation error at index %u, "
               "expected %u got %ld\n",
               i, i * 2 + 1, mapPtr[i]);

#endif
        bResult = false;
        break;
      }
    }
    if (!bResult) {
      break;
    }

    iRet = clEnqueueUnmapMemObject(clCommandQueues[0], clParentBuff, mapPtr, 0,
                                   NULL, NULL);
    bResult &=
        SilentCheck("clEnqueueMapBuffer - clParentBuff", CL_SUCCESS, iRet);
    if (!bResult) {
      break;
    }
    iRet = clFinish(clCommandQueues[0]);
    bResult &= SilentCheck("clFinish for clCommandQueues[0]", CL_SUCCESS, iRet);
    if (!bResult) {
      break;
    }

  } while (0);

  if (clCommandQueues) {
    for (unsigned int i = 0; i < num_devices; i++) {
      if (clCommandQueues[i]) {
        clFinish(clCommandQueues[i]);
        clReleaseCommandQueue(clCommandQueues[i]);
      }
    }
    delete[] clCommandQueues;
  }

  if (kernel) {
    clReleaseKernel(kernel);
  }

  if (program) {
    clReleaseProgram(program);
  }

  if (clSubBuffArr) {
    for (unsigned int i = 0; i < num_devices; i++) {
      if (clSubBuffArr[i]) {
        clReleaseMemObject(clSubBuffArr[i]);
      }
    }
    delete[] clSubBuffArr;
  }

  if (clParentBuff) {
    clReleaseMemObject(clParentBuff);
  }

  if (context) {
    clReleaseContext(context);
  }

  if (pBuff) {
    free(pBuff);
  }

  if (pDevices) {
    delete[] pDevices;
  }

  return bResult;
}

/******************************************************************************/
#define NUM_SUB_BUFFERS 6
#define NUM_BUFFERS 2
#define BUFFER_SIZE NUM_SUB_BUFFERS * 1024 * 1024
bool run_multi_devices_parallel_ndrange_with_read_sub_buffers() {
  const char *ocl_test_program[] = {
      "__kernel void dot_product (global const float* inArr1, global const "
      "float* inArr2, global const float* inArr3, global float* out, float "
      "inExp1, float inExp2, float inExp3, float outExpBefore, float "
      "outExpAfter, int iter, global int* validationArr)"
      "{"
      "int tid = get_global_id(0);"
      "if (inArr1[tid] != (inExp1))"
      "{"
      "  validationArr[0] = -1;"
      "  validationArr[1] = inExp1;"
      "  validationArr[2] = inArr1[tid];"
      "}"
      "if (inArr2[tid] != (inExp2))"
      "{"
      "  validationArr[3] = -1;"
      "  validationArr[4] = inExp2;"
      "  validationArr[5] = inArr2[tid];"
      "}"
      "if (inArr3[tid] != (inExp3))"
      "{"
      "  validationArr[6] = -1;"
      "  validationArr[7] = inExp3;"
      "  validationArr[8] = inArr3[tid];"
      "}"
      "if (out[tid] != (outExpBefore))"
      "{"
      "  validationArr[9] = -1;"
      "  validationArr[10] = outExpBefore;"
      "  validationArr[11] = out[tid];"
      "}"
      "out[tid] = inArr1[tid] + inArr2[tid] + inArr3[tid];"
      "if (out[tid] != (outExpAfter))"
      "{"
      "  validationArr[9] = -1;"
      "  validationArr[10] = outExpAfter;"
      "  validationArr[11] = out[tid];"
      "}"
      "}"};

  bool bResult = true;
  cl_uint uiNumDevices = 0;
  cl_device_id *pDevices;
  size_t *pBinarySizes;
  cl_int *pBinaryStatus;
  cl_context context;

  cl_platform_id platform = 0;

  cl_int iRet = clGetPlatformIDs(1, &platform, NULL);
  bResult &= SilentCheck("clGetPlatformIDs", CL_SUCCESS, iRet);

  if (!bResult) {
    return bResult;
  }

  cl_context_properties prop[3] = {CL_CONTEXT_PLATFORM,
                                   (cl_context_properties)platform, 0};

  // get device(s)
  iRet = clGetDeviceIDs(platform, CL_DEVICE_TYPE_ACCELERATOR, 0, NULL,
                        &uiNumDevices);
  // If CL_DEVICE_TYPE_ACCELERATOR does not exist, skip this test.
  if ((CL_SUCCESS != iRet) || (0 == uiNumDevices)) {
    return bResult;
  }

  // One CPU device and One MIC device.
  uiNumDevices = 2;

  // initialize arrays
  pDevices = new cl_device_id[uiNumDevices];
  pBinarySizes = new size_t[uiNumDevices];
  pBinaryStatus = new cl_int[uiNumDevices];

  iRet = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 1, &(pDevices[0]), NULL);
  bResult &= SilentCheck("clGetDeviceIDs", CL_SUCCESS, iRet);
  if (!bResult) {
    delete[] pDevices;
    delete[] pBinarySizes;
    delete[] pBinaryStatus;
    return bResult;
  }

  iRet = clGetDeviceIDs(platform, CL_DEVICE_TYPE_ACCELERATOR, 1, &(pDevices[1]),
                        NULL);
  bResult &= SilentCheck("clGetDeviceIDs", CL_SUCCESS, iRet);
  if (!bResult) {
    delete[] pDevices;
    delete[] pBinarySizes;
    delete[] pBinaryStatus;
    return bResult;
  }

  // create context
  context = clCreateContext(prop, uiNumDevices, pDevices, NULL, NULL, &iRet);
  bResult &= SilentCheck("clCreateContext", CL_SUCCESS, iRet);
  if (!bResult) {
    delete[] pDevices;
    delete[] pBinarySizes;
    delete[] pBinaryStatus;
    return bResult;
  }

  size_t alignment = 1;

  for (cl_uint i = 0; i < uiNumDevices; ++i) {
    cl_uint device_alignment_in_bits = 1;

    iRet = clGetDeviceInfo(pDevices[i], CL_DEVICE_MEM_BASE_ADDR_ALIGN,
                           sizeof(device_alignment_in_bits),
                           &device_alignment_in_bits, 0);

    // Supposing that alignment can be power of 2 only,
    // we get maximum of all values for all devices to satisfy all of them.
    alignment = std::max(alignment, size_t(device_alignment_in_bits / 8));
  }

  assert(alignment % sizeof(float) == 0);

  std::vector<float *> hostMem(NUM_BUFFERS);
  for (unsigned int i = 0; i < NUM_BUFFERS; i++) {
    hostMem[i] = (float *)ALIGNED_MALLOC(BUFFER_SIZE, alignment);
  }

  size_t num_elements_in_buffer = BUFFER_SIZE / sizeof(float);
  size_t num_elements_in_subbuffer = num_elements_in_buffer / NUM_SUB_BUFFERS;

  for (unsigned int i = 0; i < NUM_BUFFERS; i++) {
    for (unsigned int j = 0; j < NUM_SUB_BUFFERS; j++) {
      for (unsigned int k = 0; k < num_elements_in_subbuffer; k++) {
        hostMem[i][(j * num_elements_in_subbuffer) + k] = (j + 1);
      }
    }
  }

  std::vector<cl_mem> buffers(NUM_BUFFERS);

  for (size_t i = 0; i < NUM_BUFFERS; ++i) {
    buffers[i] =
        clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR,
                       BUFFER_SIZE, hostMem[i], &iRet);
    bResult &= SilentCheck("clCreateBuffer - srcA", CL_SUCCESS, iRet);
  }

  // create program with source
  cl_program program = clCreateProgramWithSource(
      context, 1, (const char **)&ocl_test_program, NULL, &iRet);
  bResult &= SilentCheck("clCreateProgramWithSource", CL_SUCCESS, iRet);

  iRet = clBuildProgram(program, uiNumDevices, pDevices, "-cl-denorms-are-zero",
                        NULL, NULL);
  bResult &= SilentCheck("clBuildProgram", CL_SUCCESS, iRet);

  //
  // Create Kernel
  //
  cl_kernel kernel1 = clCreateKernel(program, "dot_product", &iRet);
  bResult &= SilentCheck("clCreateKernel - dot_product", CL_SUCCESS, iRet);

  //
  // Create queue
  //
  std::vector<cl_command_queue> queues(uiNumDevices);
  for (unsigned int i = 0; i < uiNumDevices; i++) {
    queues[i] = clCreateCommandQueueWithProperties(
        context, pDevices[i], NULL /*no properties*/, &iRet);
    bResult &= SilentCheck("clCreateCommandQueueWithProperties - queue",
                           CL_SUCCESS, iRet);
  }

  std::vector<cl_mem *> subBuffers(NUM_BUFFERS);
  for (unsigned int i = 0; i < NUM_BUFFERS; i++) {
    subBuffers[i] = new cl_mem[NUM_SUB_BUFFERS];
    for (unsigned int j = 0; j < NUM_SUB_BUFFERS; j++) {
      cl_buffer_region region = {j * (BUFFER_SIZE / NUM_SUB_BUFFERS),
                                 (BUFFER_SIZE / NUM_SUB_BUFFERS)};
      subBuffers[i][j] = clCreateSubBuffer(
          buffers[i], 0, CL_BUFFER_CREATE_TYPE_REGION, &region, &iRet);

      bResult &= SilentCheck("clCreateSubBuffer", CL_SUCCESS, iRet);
    }
  }

  const unsigned int num_iterations = 2;
  const unsigned int total_rounds = uiNumDevices * 2 * num_iterations;
  std::vector<cl_event> events(total_rounds);

  float resArr[9][2][NUM_SUB_BUFFERS] = {
      {{1, 2, 3, 4, 5, 6}, {1, 2, 3, 4, 5, 6}},
      {{1, 2, 3, 4, 5, 6}, {1, 2, 9, 4, 5, 6}},
      {{1, 2, 3, 4, 5, 6}, {1, 6, 9, 4, 5, 6}},
      {{1, 2, 3, 4, 5, 6}, {1, 6, 9, 12, 5, 6}},
      {{1, 2, 3, 4, 5, 6}, {1, 6, 9, 12, 15, 6}},
      {{1, 2, 27, 4, 5, 6}, {1, 6, 9, 12, 15, 6}},
      {{1, 16, 27, 4, 5, 6}, {1, 6, 9, 12, 15, 6}},
      {{1, 16, 27, 36, 5, 6}, {1, 6, 9, 12, 15, 6}},
      {{1, 16, 27, 36, 33, 6}, {1, 6, 9, 12, 15, 6}}};

  float outResArr[8][2] = {{3, 9},  {2, 6},  {4, 12}, {5, 15},
                           {3, 27}, {2, 16}, {4, 36}, {5, 33}};

  const size_t validationSubBufferSize = MAX(sizeof(int) * 3 * 4, alignment);
  int *validationBufferPtr =
      (int *)ALIGNED_MALLOC(validationSubBufferSize * total_rounds, alignment);
  memset(validationBufferPtr, 0, validationSubBufferSize * total_rounds);
  cl_mem validationBuffer = clCreateBuffer(
      context, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR,
      validationSubBufferSize * total_rounds, validationBufferPtr, &iRet);
  bResult &= SilentCheck("clCreateBuffer - validationBuffer", CL_SUCCESS, iRet);

  std::vector<cl_mem> validationSubBuffers(total_rounds);
  for (unsigned int i = 0; i < total_rounds; i++) {
    cl_buffer_region region = {i * validationSubBufferSize,
                               validationSubBufferSize};
    validationSubBuffers[i] = clCreateSubBuffer(
        validationBuffer, 0, CL_BUFFER_CREATE_TYPE_REGION, &region, &iRet);

    bResult &= SilentCheck("validationSubBuffers", CL_SUCCESS, iRet);
  }

  size_t global_work_size[1] = {num_elements_in_subbuffer};
  size_t local_work_size[1] = {1};

  unsigned int round = 0;
  for (unsigned int i = 0; i < num_iterations; i++) {
    for (unsigned int dev = 0; dev < uiNumDevices; dev++) {
      iRet =
          clSetKernelArg(kernel1, 0, sizeof(cl_mem), &(subBuffers[i][1 + dev]));
      bResult &= SilentCheck("clSetKernelArg - buffer_srcA", CL_SUCCESS, iRet);
      iRet =
          clSetKernelArg(kernel1, 1, sizeof(cl_mem), &(subBuffers[i][2 + dev]));
      bResult &= SilentCheck("clSetKernelArg - buffer_srcA", CL_SUCCESS, iRet);
      iRet =
          clSetKernelArg(kernel1, 2, sizeof(cl_mem), &(subBuffers[i][3 + dev]));
      bResult &= SilentCheck("clSetKernelArg - buffer_srcA", CL_SUCCESS, iRet);
      iRet = clSetKernelArg(kernel1, 3, sizeof(cl_mem),
                            &(subBuffers[(i + 1) % 2][2 + dev]));
      bResult &= SilentCheck("clSetKernelArg - buffer_srcA", CL_SUCCESS, iRet);
      iRet = clSetKernelArg(kernel1, 4, sizeof(float),
                            &(resArr[round][i][1 + dev]));
      bResult &= SilentCheck("clSetKernelArg - buffer_srcA", CL_SUCCESS, iRet);
      iRet = clSetKernelArg(kernel1, 5, sizeof(float),
                            &(resArr[round][i][2 + dev]));
      bResult &= SilentCheck("clSetKernelArg - buffer_srcA", CL_SUCCESS, iRet);
      iRet = clSetKernelArg(kernel1, 6, sizeof(float),
                            &(resArr[round][i][3 + dev]));
      bResult &= SilentCheck("clSetKernelArg - buffer_srcA", CL_SUCCESS, iRet);
      iRet = clSetKernelArg(kernel1, 7, sizeof(float), &(outResArr[round][0]));
      bResult &= SilentCheck("clSetKernelArg - buffer_srcA", CL_SUCCESS, iRet);
      iRet = clSetKernelArg(kernel1, 8, sizeof(float), &(outResArr[round][1]));
      bResult &= SilentCheck("clSetKernelArg - buffer_srcA", CL_SUCCESS, iRet);
      iRet = clSetKernelArg(kernel1, 9, sizeof(int), &(round));
      bResult &= SilentCheck("clSetKernelArg - buffer_srcA", CL_SUCCESS, iRet);
      iRet = clSetKernelArg(kernel1, 10, sizeof(cl_mem),
                            &(validationSubBuffers[round]));
      bResult &= SilentCheck("clSetKernelArg - buffer_srcA", CL_SUCCESS, iRet);

      unsigned int num_events = 0;
      cl_event *currEvent = NULL;
      if (round == 4) {
        num_events = 1;
        currEvent = &(events[2]);
      }
      if (round == 6) {
        num_events = 1;
        currEvent = &(events[0]);
      }
      iRet = clEnqueueNDRangeKernel(queues[dev], kernel1, 1, NULL,
                                    global_work_size, local_work_size,
                                    num_events, currEvent, &(events[round]));
      bResult &= SilentCheck("clEnqueueNDRangeKernel", CL_SUCCESS, iRet);
      round++;

      int indexAdd = (dev == 0) ? (-1) : 1;

      iRet = clSetKernelArg(kernel1, 0, sizeof(cl_mem),
                            &(subBuffers[i][1 + dev + indexAdd]));
      bResult &= SilentCheck("clSetKernelArg - buffer_srcA", CL_SUCCESS, iRet);
      iRet = clSetKernelArg(kernel1, 1, sizeof(cl_mem),
                            &(subBuffers[i][2 + dev + indexAdd]));
      bResult &= SilentCheck("clSetKernelArg - buffer_srcA", CL_SUCCESS, iRet);
      iRet = clSetKernelArg(kernel1, 2, sizeof(cl_mem),
                            &(subBuffers[i][3 + dev + indexAdd]));
      bResult &= SilentCheck("clSetKernelArg - buffer_srcA", CL_SUCCESS, iRet);
      iRet = clSetKernelArg(kernel1, 3, sizeof(cl_mem),
                            &(subBuffers[(i + 1) % 2][2 + dev + indexAdd]));
      bResult &= SilentCheck("clSetKernelArg - buffer_srcA", CL_SUCCESS, iRet);
      iRet = clSetKernelArg(kernel1, 4, sizeof(float),
                            &(resArr[round][i][1 + dev + indexAdd]));
      bResult &= SilentCheck("clSetKernelArg - buffer_srcA", CL_SUCCESS, iRet);
      iRet = clSetKernelArg(kernel1, 5, sizeof(float),
                            &(resArr[round][i][2 + dev + indexAdd]));
      bResult &= SilentCheck("clSetKernelArg - buffer_srcA", CL_SUCCESS, iRet);
      iRet = clSetKernelArg(kernel1, 6, sizeof(float),
                            &(resArr[round][i][3 + dev + indexAdd]));
      bResult &= SilentCheck("clSetKernelArg - buffer_srcA", CL_SUCCESS, iRet);
      iRet = clSetKernelArg(kernel1, 7, sizeof(float), &(outResArr[round][0]));
      bResult &= SilentCheck("clSetKernelArg - buffer_srcA", CL_SUCCESS, iRet);
      iRet = clSetKernelArg(kernel1, 8, sizeof(float), &(outResArr[round][1]));
      bResult &= SilentCheck("clSetKernelArg - buffer_srcA", CL_SUCCESS, iRet);
      iRet = clSetKernelArg(kernel1, 9, sizeof(int), &(round));
      bResult &= SilentCheck("clSetKernelArg - buffer_srcA", CL_SUCCESS, iRet);
      iRet = clSetKernelArg(kernel1, 10, sizeof(cl_mem),
                            &(validationSubBuffers[round]));
      bResult &= SilentCheck("clSetKernelArg - buffer_srcA", CL_SUCCESS, iRet);

      num_events = 0;
      currEvent = NULL;

      iRet = clEnqueueNDRangeKernel(queues[dev], kernel1, 1, NULL,
                                    global_work_size, local_work_size,
                                    num_events, currEvent, &(events[round]));
      bResult &= SilentCheck("clEnqueueNDRangeKernel", CL_SUCCESS, iRet);
      round++;
    }
  }

  for (cl_uint i = 0; i < uiNumDevices; ++i) {
    iRet = clFinish(queues[i]);
    bResult &= SilentCheck("clFinish", CL_SUCCESS, iRet);
  }

  // Read the parent buffer in order to validate results
  void *ptr1 = clEnqueueMapBuffer(
      queues[0], // it doesn't matter which queue is used here
      buffers[0], true,
      CL_MAP_READ, // it is imprortant to use minimal required access for
                   // mapping
      0, BUFFER_SIZE, 0, 0, 0, &iRet);
  bResult &= SilentCheck("clEnqueueMapBuffer", CL_SUCCESS, iRet);

  // Read the parent buffer in order to validate results
  void *ptr2 = clEnqueueMapBuffer(
      queues[0], // it doesn't matter which queue is used here
      buffers[1], true,
      CL_MAP_READ, // it is imprortant to use minimal required access for
                   // mapping
      0, BUFFER_SIZE, 0, 0, 0, &iRet);
  bResult &= SilentCheck("clEnqueueMapBuffer", CL_SUCCESS, iRet);

  // Read the parent buffer in order to validate each round results
  void *roundValidPtr = clEnqueueMapBuffer(
      queues[0], // it doesn't matter which queue is used here
      validationBuffer, true,
      CL_MAP_READ, // it is imprortant to use minimal required access for
                   // mapping
      0, validationSubBufferSize * total_rounds, 0, 0, 0, &iRet);
  bResult &= SilentCheck("clEnqueueMapBuffer", CL_SUCCESS, iRet);

  for (unsigned int i = 0; i < NUM_SUB_BUFFERS; i++) {
    for (unsigned int j = 0; j < num_elements_in_subbuffer; j++) {
      if (((float *)ptr1)[(i * num_elements_in_subbuffer) + j] !=
          resArr[round][0][i]) {
        printf("Buffer1 read, invalid value at index %zu, expected %f got %f "
               "(==> At sub-buffer %d at index %d of %zu)\n",
               (i * num_elements_in_subbuffer) + j, resArr[round][0][i],
               ((float *)ptr1)[(i * num_elements_in_subbuffer) + j], i, j,
               num_elements_in_subbuffer - 1);
        bResult = false;
        break;
      }
      if (((float *)ptr2)[(i * num_elements_in_subbuffer) + j] !=
          resArr[round][1][i]) {
        printf("Buffer2 read, invalid value at index %zu, expected %f got %f "
               "(==> At sub-buffer %d at index %d of %zu)\n",
               (i * num_elements_in_subbuffer) + j, resArr[round][0][i],
               ((float *)ptr1)[(i * num_elements_in_subbuffer) + j], i, j,
               num_elements_in_subbuffer - 1);
        bResult = false;
        break;
      }
    }
    if (false == bResult) {
      break;
    }
  }

  for (unsigned int i = 0; i < total_rounds; i++) {
    unsigned int pos = i * (validationSubBufferSize / sizeof(int));
    for (unsigned int j = 0; j < 4; j++) {
      if (((int *)roundValidPtr)[pos + (j * 3)] != 0) {
        printf("Error in round %d at Buffer %s%d, got %d, expexted %d\n", i + 1,
               (j == 3) ? "OutBuffer" : "InBuffer", (j == 3) ? 1 : (j + 1),
               ((int *)roundValidPtr)[pos + (j * 3) + 2],
               ((int *)roundValidPtr)[pos + (j * 3) + 1]);
        bResult = false;
      }
    }
  }

  iRet = clEnqueueUnmapMemObject(queues[0], buffers[0], ptr1, 0, 0, 0);
  bResult &= SilentCheck("clEnqueueUnmapMemObject", CL_SUCCESS, iRet);

  iRet = clEnqueueUnmapMemObject(queues[0], buffers[1], ptr2, 0, 0, 0);
  bResult &= SilentCheck("clEnqueueUnmapMemObject", CL_SUCCESS, iRet);

  iRet = clEnqueueUnmapMemObject(queues[0], validationBuffer, roundValidPtr, 0,
                                 0, 0);
  bResult &= SilentCheck("clEnqueueUnmapMemObject", CL_SUCCESS, iRet);

  for (cl_uint i = 0; i < uiNumDevices; ++i) {
    iRet = clFinish(queues[i]);
    bResult &= SilentCheck("clFinish", CL_SUCCESS, iRet);
  }

  for (unsigned int i = 0; i < total_rounds; i++) {
    iRet = clReleaseMemObject(validationSubBuffers[i]);
    bResult &= SilentCheck("clReleaseBuffer", CL_SUCCESS, iRet);
    iRet = clReleaseEvent(events[i]);
    bResult &= SilentCheck("clReleaseEvent", CL_SUCCESS, iRet);
  }

  iRet = clReleaseMemObject(validationBuffer);
  bResult &= SilentCheck("clReleaseBuffer", CL_SUCCESS, iRet);

  for (unsigned int i = 0; i < NUM_BUFFERS; i++) {
    for (unsigned int j = 0; j < NUM_SUB_BUFFERS; j++) {
      iRet = clReleaseMemObject(subBuffers[i][j]);
      bResult &= SilentCheck("clReleaseBuffer", CL_SUCCESS, iRet);
    }
    delete (subBuffers[i]);

    iRet = clReleaseMemObject(buffers[i]);
    bResult &= SilentCheck("clReleaseBuffer", CL_SUCCESS, iRet);
    ALIGNED_FREE(hostMem[i]);
  }

  for (unsigned int i = 0; i < uiNumDevices; i++) {
    iRet = clReleaseCommandQueue(queues[i]);
    bResult &= SilentCheck("clReleaseCommandQueue - queue", CL_SUCCESS, iRet);
  }

  iRet = clReleaseKernel(kernel1);
  bResult &= SilentCheck("clReleaseKernel - kernel1", CL_SUCCESS, iRet);

  iRet = clReleaseProgram(program);
  bResult &= SilentCheck("clReleaseProgram - program", CL_SUCCESS, iRet);

  iRet = clReleaseContext(context);
  bResult &= SilentCheck("clReleaseContext - context", CL_SUCCESS, iRet);

  ALIGNED_FREE(validationBufferPtr);

  return bResult;
}

/*******************************************************************************
 * cl_CPU_MIC_Common_RT_SubBuffers_Async
 * -------------------------------------
 * This test create shared context of CPU and MIC device.
 * Create Parent buffer and 2 subBuffers that cover the parent buffer.
 * create 2 command queues.
 * Execute in loop X rounds:
 *     for each device:
 *         (1) Run NDRange on Parent buffer that change the parent content.
 *         (2) Read the parent content by the two subBuffers simultaneously.
 *(each on different device) (3) Run NDRange simultaneously on the two
 *subBuffers that change the content (each subBuffer run on different device
 *that it ran on previous stage). (4) Map The parent buffer from the second
 *device. (Not the device that it was in stage 1). After each Read / Map
 *operation (by parent buffer or subBuffers) validating the content of the
 *buffers.
 ******************************************************************************/
bool cl_CPU_MIC_Common_RT_SubBuffers_Async() {
  cl_device_type dev_types[2] = {CL_DEVICE_TYPE_CPU,
                                 CL_DEVICE_TYPE_ACCELERATOR};
  unsigned int num_expected_devices_of_type[2] = {1, 1};
  unsigned int numDeviceTypes = 2;
  return run_common_rt_sub_buffers_async_test(__FUNCTION__, dev_types,
                                              num_expected_devices_of_type,
                                              numDeviceTypes, false);
}

/*******************************************************************************
 * cl_CPU_MIC_Common_RT_SubBuffers_Async_With_Buffer_Release
 * -------------------------------------
 * This test create shared context of CPU and MIC device.
 * Create Parent buffer and 2 subBuffers that cover the parent buffer.
 * create 2 command queues.
 * Execute in loop X rounds:
 *     for each device:
 *      for 1000 times:
 *             (1) Enqueue NDRange simultaneously with the two
 *subBuffers that will change the content.(It wait for event triggering) (2)
 *Release one of the subBuffers (3) Create new subBuffer instead of the deleted
 *subBuffer --> change the memory mode to Overlapping. (4) Triger the event. (5)
 *Read the data from parent buffer or the subBuffers and validate the content.
 ******************************************************************************/
bool cl_CPU_MIC_Common_RT_SubBuffers_Async_With_Buffer_Release() {
  cl_device_type dev_types[2] = {CL_DEVICE_TYPE_CPU,
                                 CL_DEVICE_TYPE_ACCELERATOR};
  unsigned int num_expected_devices_of_type[2] = {1, 1};
  unsigned int numDeviceTypes = 2;
  return run_common_rt_sub_buffers_async_test(__FUNCTION__, dev_types,
                                              num_expected_devices_of_type,
                                              numDeviceTypes, true);
}

/*******************************************************************************
 * cl_CPU_MIC_Parallel_NDRange_Execution_With_Read_Of_Same_Buffer
 * -------------------------------------
 * This test create shared context of CPU and MIC device.
 * Create 2 Parent buffers and 6 subBuffers that cover each parent buffer.
 * Run NDRanges with 3 input subbuffer (that sent as R/W) but actually only
 *reading the content, and One out buffer that writing to it. Check inside the
 *NDRange the correctness of the subbuffers content. Read the parent buffers and
 *validate their content.
 ******************************************************************************/
bool cl_CPU_MIC_Parallel_NDRange_Execution_With_Read_Of_Same_Buffer() {
  printf("---------------------------------------\n");
  printf("clParallelNDRangeExecutionWithReadOfSameBuffer\n");
  printf("---------------------------------------\n");

  bool res = true;
  for (unsigned int i = 0; i < 100; i++) {
    res &= run_multi_devices_parallel_ndrange_with_read_sub_buffers();
    printf(".");
    fflush(0);
    if (!res) {
      break;
    }
  }
  printf("\n");
  return res;
}

/*******************************************************************************
 * cl_ALL_Devices_Common_RT_SubBuffers_Async
 * -------------------------------------
 * This test create shared context of ALL devices.
 * Create Parent buffer and subBuffer for each device that cover the parent
 *buffer. create command queue for each device. Execute in loop X rounds: for
 *each device: (1) Run NDRange on Parent buffer that change the parent content.
 *         (2) Read the parent content by the subBuffers simultaneously. (each
 *on different device) (3) Run NDRange simultaneously on the subBuffers that
 *change the content (each subBuffer run on different device that it ran on
 *previous stage). (4) Map The parent buffer from the last device. (Not the
 *device that it was in stage 1). After each Read / Map operation (by parent
 *buffer or subBuffers) validating the content of the buffers.
 ******************************************************************************/
bool cl_ALL_Devices_Common_RT_SubBuffers_Async() {
  cl_device_type dev_types[1] = {CL_DEVICE_TYPE_ALL};
  unsigned int numDeviceTypes = 1;
  return run_common_rt_sub_buffers_async_test(__FUNCTION__, dev_types, NULL,
                                              numDeviceTypes, false);
}

/*******************************************************************************
 * cl_ALL_Devices_Common_RT_SubBuffers_Async_With_Buffer_Release
 * -------------------------------------
 * This test create shared context of ALL devices.
 * Create Parent buffer and subBuffer for each device that cover the parent
 *buffer. create command queues for each device. Execute in loop X rounds: for
 *each device: for 1000 times: (1) Enqueue NDRange simultaneously with the
 *subBuffers that will change the content.(It wait for event triggering) (2)
 *Release one of the subBuffers (3) Create new subBuffer instead of the deleted
 *subBuffer --> change the memory mode to Overlapping. (4) Triger the event. (5)
 *Read the data from parent buffer or the subBuffers and validate the content.
 ******************************************************************************/
bool cl_ALL_Devices_Common_RT_SubBuffers_Async_With_Buffer_Release() {
  cl_device_type dev_types[1] = {CL_DEVICE_TYPE_ALL};
  unsigned int numDeviceTypes = 1;
  return run_common_rt_sub_buffers_async_test(__FUNCTION__, dev_types, NULL,
                                              numDeviceTypes, true);
}

bool cl_ALL_Devices_SubBuffer_Simple_Test() {
  return run_multi_devices_sub_buffer_simple_test(__FUNCTION__);
}
