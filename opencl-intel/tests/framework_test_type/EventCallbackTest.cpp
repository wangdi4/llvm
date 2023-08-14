#include "CL/cl.h"
#include "FrameworkTest.h"
#include "cl_provisional.h"

extern cl_device_type gDeviceType;

#define BOGUS_ERR_CODE -10

#define NOISY_CHECK(name, expected, actual)                                    \
  if (!SilentCheck(name, expected, actual)) {                                  \
    PROV_RETURN_AND_ABANDON(false);                                            \
  }

typedef struct {
  cl_command_queue queue;
  cl_int submitted_counter;
  cl_int running_counter;
  cl_int complete_counter;
  cl_int sum_of_errors;
} callback_data_type;

void STDCALL SystemEventCallbackFunc(cl_event notifier, cl_int status,
                                     void *user_data) {

  callback_data_type *pdata = static_cast<callback_data_type *>(user_data);
  // fprintf(stderr, "SystemEventCallbackFunc called by notifier %u for status
  // %d\n", notifier, status);

  switch (status) {
  case CL_SUBMITTED:
    pdata->submitted_counter++;
    break;
  case CL_RUNNING:
    pdata->running_counter++;
    break;
  case CL_COMPLETE:
    pdata->complete_counter++;
    break;
  default:
    // treat as complete in case of error.
    pdata->complete_counter++;
    pdata->sum_of_errors += status;
  }
}

struct NativeKernelArgType {
  cl_uchar *input_buf;
  cl_uchar *output_buf;
  size_t bufsize;

  NativeKernelArgType(const size_t bufSize)
      : input_buf((cl_uchar *)NULL), output_buf((cl_uchar *)NULL),
        bufsize(bufSize) {
    output_buf = (cl_uchar *)malloc(bufsize);
    input_buf = (cl_uchar *)malloc(bufsize);
  }

  ~NativeKernelArgType() {
    free(output_buf);
    free(input_buf);
  }
};

static void CL_CALLBACK NativeKernelFunc(void *arg_) {
  NativeKernelArgType *arg = (NativeKernelArgType *)arg_;

  for (size_t i = 0; i < arg->bufsize; ++i)
    arg->output_buf[i] = arg->input_buf[i];
}

bool EventCallbackTest() {
  PROV_INIT;

  cl_uint uiNumDevices = 0;
  cl_device_id *pDevices;
  cl_context context;

  cl_platform_id platform = 0;

  cl_int iRet = clGetPlatformIDs(1, &platform, NULL);
  NOISY_CHECK("clGetPlatformIDs", CL_SUCCESS, iRet);

  cl_context_properties prop[3] = {CL_CONTEXT_PLATFORM,
                                   (cl_context_properties)platform, 0};

  // get device(s)
  iRet = clGetDeviceIDs(platform, gDeviceType, 0, NULL, &uiNumDevices);
  NOISY_CHECK("clGetDeviceIDs", CL_SUCCESS, iRet);

  // initialize arrays
  pDevices = PROV_ARR(new cl_device_id[uiNumDevices]);

  iRet = clGetDeviceIDs(platform, gDeviceType, uiNumDevices, pDevices, NULL);
  NOISY_CHECK("clGetDeviceIDs", CL_SUCCESS, iRet);

  // create context
  context = PROV_OBJ(
      clCreateContext(prop, uiNumDevices, pDevices, NULL, NULL, &iRet));
  NOISY_CHECK("clCreateContext", CL_SUCCESS, iRet);

  // Create queue
  cl_command_queue queue = PROV_OBJ(clCreateCommandQueueWithProperties(
      context, pDevices[0], NULL /*no properties*/, &iRet));
  NOISY_CHECK("clCreateCommandQueueWithProperties", CL_SUCCESS, iRet);

  cl_int errCode;
  NativeKernelArgType kernel_arg(128);
  cl_event kernelWaitList[1];

  /** first iteration, check for proper call order. **/

  // triggers to start all events.
  cl_event kernelEvent1;
  cl_event user_event1 = PROV_OBJ(clCreateUserEvent(context, &errCode));
  NOISY_CHECK("clCreateUserEvent 1", CL_SUCCESS, errCode);

  callback_data_type data1;
  data1.queue = queue;
  data1.submitted_counter = 0;
  data1.running_counter = 0;
  data1.complete_counter = 0;
  data1.sum_of_errors = 0;

  kernelWaitList[0] = user_event1;

  iRet = clEnqueueNativeKernel(queue, NativeKernelFunc, &kernel_arg,
                               sizeof(kernel_arg), 0, NULL, NULL, 1,
                               kernelWaitList, &kernelEvent1);
  NOISY_CHECK("clEnqueueNativeKernel 1", CL_SUCCESS, iRet);
  PROV_OBJ(kernelEvent1);

  iRet = clSetEventCallback(kernelEvent1, CL_COMPLETE, SystemEventCallbackFunc,
                            &data1);
  NOISY_CHECK("clSetEventCallback", CL_SUCCESS, iRet);

  iRet = clSetEventCallback(kernelEvent1, CL_RUNNING, SystemEventCallbackFunc,
                            &data1);
  NOISY_CHECK("clSetEventCallback", CL_SUCCESS, iRet);

  iRet = clSetEventCallback(kernelEvent1, CL_SUBMITTED, SystemEventCallbackFunc,
                            &data1);
  NOISY_CHECK("clSetEventCallback", CL_SUCCESS, iRet);

  iRet = clSetEventCallback(kernelEvent1, CL_COMPLETE, SystemEventCallbackFunc,
                            &data1);
  NOISY_CHECK("clSetEventCallback", CL_SUCCESS, iRet);

  iRet = clSetEventCallback(kernelEvent1, CL_RUNNING, SystemEventCallbackFunc,
                            &data1);
  NOISY_CHECK("clSetEventCallback", CL_SUCCESS, iRet);

  iRet = clSetEventCallback(kernelEvent1, CL_SUBMITTED, SystemEventCallbackFunc,
                            &data1);
  NOISY_CHECK("clSetEventCallback", CL_SUCCESS, iRet);

  // trigger the first test:
  iRet = clSetUserEventStatus(user_event1, CL_COMPLETE);
  NOISY_CHECK("clSetUserEventStatus", CL_SUCCESS, iRet);

  iRet = clFlush(queue);
  NOISY_CHECK("clFlush after case 1", CL_SUCCESS, iRet);

  /** second iteration, check for proper call in case of error. **/

  cl_event kernelEvent2;
  cl_event user_event2 = PROV_OBJ(clCreateUserEvent(context, &errCode));
  NOISY_CHECK("clCreateUserEvent 2", CL_SUCCESS, errCode);

  callback_data_type data2;
  data2.queue = queue;
  data2.submitted_counter = 0;
  data2.running_counter = 0;
  data2.complete_counter = 0;
  data2.sum_of_errors = 0;

  kernelWaitList[0] = user_event2;

  iRet = clEnqueueNativeKernel(queue, NativeKernelFunc, &kernel_arg,
                               sizeof(kernel_arg), 0, NULL, NULL, 1,
                               kernelWaitList, &kernelEvent2);
  NOISY_CHECK("clEnqueueNativeKernel 2", CL_SUCCESS, iRet);
  PROV_OBJ(kernelEvent2);

  iRet = clSetEventCallback(user_event2, CL_COMPLETE, SystemEventCallbackFunc,
                            &data2);
  NOISY_CHECK("clSetEventCallback", CL_SUCCESS, iRet);

  iRet = clSetEventCallback(user_event2, CL_SUBMITTED, SystemEventCallbackFunc,
                            &data2);
  NOISY_CHECK("clSetEventCallback", CL_SUCCESS, iRet);

  iRet = clSetEventCallback(user_event2, CL_RUNNING, SystemEventCallbackFunc,
                            &data2);
  NOISY_CHECK("clSetEventCallback", CL_SUCCESS, iRet);

  // trigger the second test:
  iRet = clSetUserEventStatus(user_event2, BOGUS_ERR_CODE);
  NOISY_CHECK("clSetUserEventStatus", CL_SUCCESS, iRet);

  iRet = clFinish(queue);
  NOISY_CHECK("clFinish after case 2", CL_SUCCESS, iRet);

  // check for results:
  bool bResult = true;
  bResult &=
      SilentCheckInt("data1 submitted counter", 2, data1.submitted_counter);
  bResult &= SilentCheckInt("data1 running counter", 2, data1.running_counter);
  bResult &=
      SilentCheckInt("data1 complete counter", 2, data1.complete_counter);
  bResult &= SilentCheckInt("data1 errors counter", 0, data1.sum_of_errors);

  bResult &= SilentCheckInt("data2 all state counters", 3,
                            data2.complete_counter + data2.running_counter +
                                data2.submitted_counter);
  bResult &= SilentCheckInt("data2 errors counter",
                            data2.complete_counter * BOGUS_ERR_CODE,
                            data2.sum_of_errors);

  NOISY_CHECK("Execution Results", true, bResult);

  clReleaseEvent(kernelEvent1);
  clReleaseEvent(kernelEvent2);
  clReleaseEvent(user_event1);
  clReleaseEvent(user_event2);
  clReleaseCommandQueue(queue);
  clReleaseContext(context);

  PROV_RETURN_AND_ABANDON(true);
}
