// Copyright (c) 2008 Intel Corporation
// All rights reserved.
//
// WARRANTY DISCLAIMER
//
// THESE MATERIALS ARE PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL INTEL OR ITS
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THESE
// MATERIALS, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Intel Corporation is the author of the Materials, and requests that all
// problem reports or change requests be submitted to it directly

#include "test_utils.h"
#include <iostream>

using namespace std;

extern cl_device_type gDeviceType;

static void TestBadProperties(cl_device_id device,
                              cl_context context) /*throw (exception)*/
{
  cl_int iRet = CL_SUCCESS;
  cl_command_queue queue;

  cl_queue_properties props2[] = {CL_QUEUE_PROPERTIES, CL_QUEUE_ON_DEVICE,
                                  0}; // missing OOO
  queue = clCreateCommandQueueWithProperties(context, device, props2, &iRet);
  CheckException("clCreateCommandQueueWithProperties", CL_INVALID_VALUE, iRet);
  CHECK_COND("clCreateCommandQueueWithProperties", NULL == queue);

  cl_queue_properties props3[] = {CL_QUEUE_PROPERTIES,
                                  CL_QUEUE_ON_DEVICE_DEFAULT,
                                  0}; // default, but not device queue
  queue = clCreateCommandQueueWithProperties(context, device, props3, &iRet);
  CheckException("clCreateCommandQueueWithProperties", CL_INVALID_VALUE, iRet);
  CHECK_COND("clCreateCommandQueueWithProperties", NULL == queue);

  cl_queue_properties props4[] = {1, 1, 0}; // invalid name
  queue = clCreateCommandQueueWithProperties(context, device, props4, &iRet);
  CheckException("clCreateCommandQueueWithProperties", CL_INVALID_VALUE, iRet);
  CHECK_COND("clCreateCommandQueueWithProperties", NULL == queue);

  cl_queue_properties props5[] = {CL_QUEUE_PROPERTIES, 0, CL_QUEUE_SIZE, 5,
                                  0}; // queue size for host queue
  queue = clCreateCommandQueueWithProperties(context, device, props5, &iRet);
  CheckException("clCreateCommandQueueWithProperties", CL_INVALID_VALUE, iRet);
  CHECK_COND("clCreateCommandQueueWithProperties", NULL == queue);
}

static const char *sProgSrc[] = {
    "static void do_nothing()"
    "{"
    "}"
    "static void waste_time()"
    "{"
    "    printf(\"child begins execution\\n\");"
    "    for (int i = 0; i < 1000000; i++)"
    "        do_nothing();"
    "}"
    "void kernel parent(int level)"
    "{"
    "    if (level == 0) {"
    "        printf(\"kernel ends execution\\n\");"
    "        return;"
    "    }"
    "    ndrange_t ndrange = ndrange_1D(1);"
    "    clk_event_t child;"
    "    queue_t queue = get_default_queue();"
    "    enqueue_kernel(queue, CLK_ENQUEUE_FLAGS_WAIT_KERNEL, ndrange, 0, NULL,"
    "                   &child, ^{ waste_time(); });"
    "    printf(\"parent ends execution\\n\");"
    "}"};

static void CL_CALLBACK NativeKernel(void *data) {}

static void TestProfilingCommandComplete(cl_context context, cl_device_id dev) {
  cl_int iRet = CL_SUCCESS;
  const size_t szLen = strlen(sProgSrc[0]);
  cl_program prog =
      clCreateProgramWithSource(context, 1, sProgSrc, &szLen, &iRet);
  CheckException("clCreateProgramWithSource", CL_SUCCESS, iRet);
  iRet = clBuildProgram(prog, 1, &dev, "-cl-std=CL2.0", NULL, NULL);
  if (iRet != CL_SUCCESS) {
    size_t szParamValSize;
    cl_int iRet = clGetProgramBuildInfo(prog, dev, CL_PROGRAM_BUILD_LOG, 0,
                                        NULL, &szParamValSize);
    CheckException("clGetProgramBuildInfo", CL_SUCCESS, iRet);
    std::vector<char> paramVal(szParamValSize);
    iRet = clGetProgramBuildInfo(prog, dev, CL_PROGRAM_BUILD_LOG,
                                 szParamValSize, &paramVal[0], NULL);
    CheckException("clGetProgramBuildInfo", CL_SUCCESS, iRet);
    cout << "Program build failed:" << endl;
    cout << &paramVal[0] << endl;
  }
  CheckException("clBuildProgram", CL_SUCCESS, iRet);

  cl_kernel kernel = clCreateKernel(prog, "parent", &iRet);
  CheckException("clCreateKernel", CL_SUCCESS, iRet);

  const cl_queue_properties queueProps[] = {CL_QUEUE_PROPERTIES,
                                            CL_QUEUE_PROFILING_ENABLE, 0};
  cl_command_queue queue =
      clCreateCommandQueueWithProperties(context, dev, queueProps, &iRet);
  CheckException("clCreateCommandQueueWithProperties", CL_SUCCESS, iRet);

  cl_event userEvent = clCreateUserEvent(context, &iRet);
  CheckException("clCreateUserEvent", CL_SUCCESS, iRet);

  int level;
  cl_ulong ulCmdEnd, ulCmdComplete;
  const size_t szGlobalWorkOffset = 0, szGlobalWorkSize = 1;

  // Enqueue kernel without children.
  level = 0;
  cl_event kernelEvent0;

  iRet = clSetKernelArg(kernel, 0, sizeof(level), &level);
  CheckException("clSetKernelArg", CL_SUCCESS, iRet);

  iRet = clEnqueueNDRangeKernel(queue, kernel, 1, &szGlobalWorkOffset,
                                &szGlobalWorkSize, NULL, 1, &userEvent,
                                &kernelEvent0);
  CheckException("clEnqueueNDRangeKernel", CL_SUCCESS, iRet);

  // Enqueue kernel with children.
  level = 1;
  cl_event kernelEvent1;
  iRet = clSetKernelArg(kernel, 0, sizeof(level), &level);
  CheckException("clSetKernelArg", CL_SUCCESS, iRet);

  iRet = clEnqueueNDRangeKernel(queue, kernel, 1, &szGlobalWorkOffset,
                                &szGlobalWorkSize, NULL, 1, &userEvent,
                                &kernelEvent1);
  CheckException("clEnqueueNDRangeKernel", CL_SUCCESS, iRet);

  iRet = clGetEventProfilingInfo(kernelEvent1, CL_PROFILING_COMMAND_COMPLETE,
                                 sizeof(ulCmdComplete), &ulCmdComplete, NULL);
  CheckException("clGetEventProfilingInfo", CL_PROFILING_INFO_NOT_AVAILABLE,
                 iRet);

  iRet = clSetUserEventStatus(userEvent, CL_COMPLETE);
  CheckException("clSetUserEventStatus", CL_SUCCESS, iRet);

  // Enqueue native kernel.
  cl_event nativeKernelEvent;
  iRet = clEnqueueNativeKernel(queue, NativeKernel, NULL, 0, 0, NULL, NULL, 0,
                               NULL, &nativeKernelEvent);

  iRet = clFinish(queue);
  CheckException("clFinish", CL_SUCCESS, iRet);

  // Check profiling of kernelEvent0.
  iRet = clGetEventProfilingInfo(kernelEvent0, CL_PROFILING_COMMAND_END,
                                 sizeof(ulCmdEnd), &ulCmdEnd, NULL);
  CheckException("CL_PROFILING_COMMAND_END", CL_SUCCESS, iRet);
  iRet = clGetEventProfilingInfo(kernelEvent0, CL_PROFILING_COMMAND_COMPLETE,
                                 sizeof(ulCmdComplete), &ulCmdComplete, NULL);
  CheckException("CL_PROFILING_COMMAND_COMPLETE", CL_SUCCESS, iRet);
  CheckException("complete time should be after end time",
                 ulCmdEnd == ulCmdComplete, true);

  // Check profiling of kernelEvent1.
  iRet = clGetEventProfilingInfo(kernelEvent1, CL_PROFILING_COMMAND_END,
                                 sizeof(ulCmdEnd), &ulCmdEnd, NULL);
  CheckException("CL_PROFILING_COMMAND_END", CL_SUCCESS, iRet);
  iRet = clGetEventProfilingInfo(kernelEvent1, CL_PROFILING_COMMAND_COMPLETE,
                                 sizeof(ulCmdComplete), &ulCmdComplete, NULL);
  CheckException("CL_PROFILING_COMMAND_COMPLETE", CL_SUCCESS, iRet);
  CheckException("complete time should be after end time",
                 ulCmdEnd < ulCmdComplete, true);

  // Check profiling of nativeKernelEvent.
  iRet = clGetEventProfilingInfo(nativeKernelEvent, CL_PROFILING_COMMAND_END,
                                 sizeof(ulCmdEnd), &ulCmdEnd, NULL);
  CheckException("CL_PROFILING_COMMAND_END", CL_SUCCESS, iRet);
  iRet =
      clGetEventProfilingInfo(nativeKernelEvent, CL_PROFILING_COMMAND_COMPLETE,
                              sizeof(ulCmdComplete), &ulCmdComplete, NULL);
  CheckException("CL_PROFILING_COMMAND_COMPLETE", CL_SUCCESS, iRet);
  CheckException("complete time should equal end time",
                 ulCmdEnd == ulCmdComplete, true);

  clReleaseEvent(userEvent);
  clReleaseEvent(kernelEvent0);
  clReleaseEvent(kernelEvent1);
  clReleaseEvent(nativeKernelEvent);
  clReleaseCommandQueue(queue);
  clReleaseKernel(kernel);
  clReleaseProgram(prog);
}

bool cl20ExecutionModel() {
  cl_int iRet = CL_SUCCESS;
  cl_platform_id platform = 0;
  bool bResult = true;
  cl_device_id device = NULL;
  cl_context context = NULL;
  cl_command_queue queue = NULL, defaultQueue = NULL;

  cout << "============================================================="
       << endl;
  cout << "cl20ExecutionModel" << endl;
  cout << "============================================================="
       << endl;

  try {
    // create context
    iRet = clGetPlatformIDs(1, &platform, NULL);
    CheckException("clGetPlatformIDs", CL_SUCCESS, iRet);
    iRet = clGetDeviceIDs(platform, gDeviceType, 1, &device, NULL);
    CheckException("clGetDeviceIDs", CL_SUCCESS, iRet);
    // check that device supports OpenCL 2.0
    size_t szParamValSize;
    iRet = clGetDeviceInfo(device, CL_DEVICE_VERSION, 0, NULL, &szParamValSize);
    CheckException("clGetDeviceInfo", CL_SUCCESS, iRet);
    vector<char> devVersion(szParamValSize);
    iRet = clGetDeviceInfo(device, CL_DEVICE_VERSION, szParamValSize,
                           &devVersion[0], NULL);
    CheckException("clGetDeviceInfo", CL_SUCCESS, iRet);
    if (string(&devVersion[0]) >= string("OpenCL 2.0")) {
      cout << "device doesn't support OpenCL 2.0 - skip this sub-test" << endl;
      return true;
    }

    const cl_context_properties prop[3] = {CL_CONTEXT_PLATFORM,
                                           (cl_context_properties)platform, 0};
    context = clCreateContextFromType(prop, gDeviceType, NULL, NULL, &iRet);
    CheckException("clCreateContextFromType", CL_SUCCESS, iRet);

    // do the real testing

    cl_queue_properties props[] = {
        CL_QUEUE_PROPERTIES,
        CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE | CL_QUEUE_ON_DEVICE, 0};
    queue = clCreateCommandQueueWithProperties(context, device, props, &iRet);
    CheckException("clCreateCommandQueueWithProperties", CL_SUCCESS, iRet);
    CHECK_COND("clCreateCommandQueueWithProperties", NULL != queue);

    cl_uint uiQueueSize, uiDevQueuePreferredSize;
    size_t szQueueSizeSize, szDevQueuePreferredSizeSize;
    iRet =
        clGetDeviceInfo(device, CL_DEVICE_QUEUE_ON_DEVICE_PREFERRED_SIZE,
                        sizeof(uiDevQueuePreferredSize),
                        &uiDevQueuePreferredSize, &szDevQueuePreferredSizeSize);
    CheckException("clGetDeviceInfo", CL_SUCCESS, iRet);
    CheckException("clGetDeviceInfo", sizeof(uiDevQueuePreferredSize),
                   szDevQueuePreferredSizeSize);

    cl_uint uiOnDeviceQueues;
    size_t szOnDeviceQueuesSize;
    iRet = clGetDeviceInfo(device, CL_DEVICE_MAX_ON_DEVICE_QUEUES,
                           sizeof(uiOnDeviceQueues), &uiOnDeviceQueues,
                           &szOnDeviceQueuesSize);
    CheckException("clGetDeviceInfo", CL_SUCCESS, iRet);
    CheckException("clGetDeviceInfo", sizeof(uiOnDeviceQueues),
                   szOnDeviceQueuesSize);

    cl_queue_properties defaultProps[] = {
        CL_QUEUE_PROPERTIES,
        CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE | CL_QUEUE_ON_DEVICE |
            CL_QUEUE_ON_DEVICE_DEFAULT,
        0};
    defaultQueue = clCreateCommandQueueWithProperties(context, device,
                                                      defaultProps, &iRet);
    CheckException("clCreateCommandQueueWithProperties", CL_SUCCESS, iRet);
    CHECK_COND("clCreateCommandQueueWithProperties", NULL != queue);

    iRet = clGetCommandQueueInfo(queue, CL_QUEUE_SIZE, sizeof(uiQueueSize),
                                 &uiQueueSize, &szQueueSizeSize);
    CheckException("clGetCommandQueueInfo", CL_SUCCESS, iRet);
    CheckException("clGetCommandQueueInfo", sizeof(uiQueueSize),
                   szQueueSizeSize);
    CheckException("uiQueueSize == uiDevQueuePreferredSize", uiQueueSize,
                   uiDevQueuePreferredSize);

    cl_command_queue_properties reportedProps;
    size_t szPropsSize;
    iRet = clGetCommandQueueInfo(defaultQueue, CL_QUEUE_PROPERTIES,
                                 sizeof(reportedProps), &reportedProps,
                                 &szPropsSize);
    CheckException("clGetCommandQueueInfo", CL_SUCCESS, iRet);
    CheckException("clGetCommandQueueInfo", sizeof(reportedProps), szPropsSize);
    CheckException("props == props[1]",
                   (cl_command_queue_properties)defaultProps[1], reportedProps);

    cl_device_id reportedDevice;
    size_t szDevIdSize;
    iRet = clGetCommandQueueInfo(defaultQueue, CL_QUEUE_DEVICE,
                                 sizeof(reportedDevice), &reportedDevice,
                                 &szDevIdSize);
    CheckException("clGetCommandQueueInfo", CL_SUCCESS, iRet);
    CheckException("clGetCommandQueueInfo", sizeof(reportedDevice),
                   szDevIdSize);
    CheckException("device == reportedDevice", device, reportedDevice);

    TestBadProperties(device, context);

    TestProfilingCommandComplete(context, device);
  } catch (const std::exception &) {
    bResult = false;
  }
  if (NULL != queue) {
    iRet = clReleaseCommandQueue(queue);
    if (!SilentCheck("clReleaseCommandQueue", CL_SUCCESS, iRet)) {
      return false;
    }
  }
  if (NULL != defaultQueue) {
    iRet = clReleaseCommandQueue(defaultQueue);
    if (!SilentCheck("clReleaseCommandQueue", CL_SUCCESS, iRet)) {
      return false;
    }
  }
  if (NULL != context) {
    iRet = clReleaseContext(context);
    if (!SilentCheck("clReleaseContext", CL_SUCCESS, iRet)) {
      return false;
    }
  }
  return bResult;
}
