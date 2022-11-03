#include "CL/cl.h"
#include "FrameworkTest.h"
#include <stdio.h>

#define BUFFERS_LENGTH 32

extern cl_device_type gDeviceType;

/*******************************************************************************
 * Native Kernel test
 *
 ******************************************************************************/
typedef struct _TEST_STRUCT {
  char xx[3];
  int yy;
  char kk;
  short zz;
  double ww;
} TEST_STRUCT;

#pragma pack(push, 1)
typedef struct _TEST_STRUCT_PACKED {
  char xx[3];
  int yy;
  char kk;
  short zz;
  double ww;
} TEST_STRUCT_PACKED;
#pragma pack(pop)

#define STRUCT_BUFFER_INPUT                                                    \
  {                                                                            \
    {{66, 91, 75}, 96, 39, 61, 59}, {{58, 31, 31}, 82, 55, 74, 1},             \
        {{28, 73, 12}, 20, 97, 6, 6}, {{11, 35, 35}, 65, 34, 48, 86},          \
        {{91, 13, 0}, 83, 26, 59, 80}, {{43, 11, 19}, 50, 85, 20, 66},         \
        {{45, 31, 38}, 42, 89, 96, 72}, {{83, 59, 17}, 69, 7, 55, 22},         \
        {{20, 74, 33}, 98, 34, 13, 34}, {{77, 53, 79}, 14, 73, 97, 80},        \
        {{80, 70, 13}, 69, 19, 37, 73}, {{48, 42, 72}, 55, 97, 46, 58},        \
        {{20, 91, 15}, 6, 57, 2, 35}, {{55, 66, 0}, 81, 64, 81, 52},           \
        {{3, 17, 66}, 22, 7, 91, 77}, {{34, 49, 63}, 31, 95, 21, 1},           \
        {{12, 68, 74}, 69, 70, 61, 55}, {{28, 56, 59}, 44, 89, 63, 40},        \
        {{80, 58, 67}, 39, 50, 44, 95}, {{45, 58, 67}, 92, 31, 68, 59},        \
        {{89, 85, 65}, 59, 99, 72, 37}, {{80, 48, 23}, 21, 11, 15, 66},        \
        {{74, 33, 83}, 24, 29, 30, 60}, {{40, 27, 18}, 71, 48, 29, 67},        \
        {{15, 32, 48}, 14, 56, 38, 93}, {{86, 68, 58}, 98, 83, 76, 42},        \
        {{61, 77, 33}, 90, 8, 45, 16}, {{73, 86, 20}, 73, 16, 87, 10},         \
        {{71, 10, 97}, 79, 48, 42, 68}, {{10, 79, 85}, 45, 55, 79, 3},         \
        {{56, 88, 59}, 64, 34, 28, 65}, {{14, 37, 32}, 82, 76, 42, 65},        \
  }

#define FLOAT_BUFFER_INPUT                                                     \
  {                                                                            \
    92, 84, 69, 43, 82, 79, 52, 17, 70, 28, 34, 77, 72, 32, 94, 51, 78, 81,    \
        76, 39, 32, 33, 88, 54, 91, 89, 87, 98, 52, 47, 78, 23                 \
  }

int global_id = 0;

int __get_global_id(int dim) { return global_id; }

void test_ref(int *out, TEST_STRUCT *a, float *b) {
  int id = __get_global_id(0);
  if (a[id].xx[2] > b[id]) {
    out[id] = a[id].zz * 7 + a[id].ww;
  } else {
    out[id] = a[id].yy * 3 - a[id].kk;
  }
}

/*******************************************************************************
 *
 *
 ******************************************************************************/
static const char *ocl_test_program[] = {"typedef struct _TEST_STRUCT\
  {\
    char xx[3];\
    int yy;\
    char kk;\
    short zz;\
    double ww;\
  } TEST_STRUCT;\
\
typedef struct __attribute__ ((packed)) _TEST_STRUCT_PACKED\
{\
  char xx[3];\
  int yy;\
  char kk;\
  short zz;\
  double ww;\
} TEST_STRUCT_PACKED;\
\
__kernel void test(__global int *out, __global TEST_STRUCT *a, \
                   __global float *b) {\
  int id = get_global_id(0);\
  if (a[id].xx[2] > b[id]) {\
    out[id] = a[id].zz * 7 + a[id].ww;\
  } else {\
    out[id] = a[id].yy * 3 - a[id].kk;\
  }\
}\
__kernel void test_packed(__global int *out, __global TEST_STRUCT_PACKED *a, \
                          __global float *b) {\
  int id = get_global_id(0);\
  if (a[id].xx[2] > b[id]) {\
    out[id] = a[id].zz * 7 + a[id].ww;\
  } else {\
    out[id] = a[id].yy * 3 - a[id].kk;\
  }\
}"};

bool validate_result(int *result, int *expected) {
  for (int i = 0; i < BUFFERS_LENGTH; ++i) {
    if (result[i] != expected[i]) {
      printf(
          "Result did not validate:\n\tresult[%d] = %d\n\texpected[%d] = %d\n",
          i, result[i], i, expected[i]);

      return false;
    }
  }
  return true;
}

bool execute_kernel(const char *const kernel_name, cl_context *context,
                    cl_program *program, cl_command_queue *queue,
                    cl_mem *buffer_src_struct, cl_mem *buffer_src_float,
                    int *output_ref) {
  bool bResult = true;
  cl_int iRet;

  int output[BUFFERS_LENGTH];

  // Create Packed Kernel
  cl_kernel kernel = clCreateKernel(*program, kernel_name, &iRet);
  bResult &= SilentCheck("clCreateKernel", CL_SUCCESS, iRet);
  if (!bResult) {
    return bResult;
  }

  // Create buffers
  cl_mem buffer_dst_int = clCreateBuffer(
      *context, CL_MEM_WRITE_ONLY, BUFFERS_LENGTH * sizeof(int), NULL, &iRet);
  bResult &= SilentCheck("clCreateBuffer - dst", CL_SUCCESS, iRet);
  if (!bResult) {
    return bResult;
  }

  // Set arguments - test
  iRet = clSetKernelArg(kernel, 0, sizeof(cl_mem), &buffer_dst_int);
  bResult &= SilentCheck("clSetKernelArg - buffer_dst_int", CL_SUCCESS, iRet);
  if (!bResult) {
    return bResult;
  }

  iRet = clSetKernelArg(kernel, 1, sizeof(cl_mem), buffer_src_struct);
  bResult &=
      SilentCheck("clSetKernelArg - buffer_src_struct", CL_SUCCESS, iRet);
  if (!bResult) {
    return bResult;
  }

  iRet = clSetKernelArg(kernel, 2, sizeof(cl_mem), buffer_src_float);
  bResult &= SilentCheck("clSetKernelArg - buffer_src_float", CL_SUCCESS, iRet);
  if (!bResult) {
    return bResult;
  }

  // Execute kernel - test
  size_t global_work_size[1] = {BUFFERS_LENGTH};
  size_t local_work_size[1] = {16};

  iRet = clEnqueueNDRangeKernel(*queue, kernel, 1, NULL, global_work_size,
                                local_work_size, 0, NULL, NULL);
  bResult &= SilentCheck("clEnqueueNDRangeKernel", CL_SUCCESS, iRet);
  if (!bResult) {
    return bResult;
  }

  // Read results. wait for completion - blocking!
  iRet =
      clEnqueueReadBuffer(*queue, buffer_dst_int, CL_TRUE, 0,
                          BUFFERS_LENGTH * sizeof(int), output, 0, NULL, NULL);
  bResult &= SilentCheck("clEnqueueReadBuffer", CL_SUCCESS, iRet);
  if (!bResult) {
    return bResult;
  }

  // Release objects
  iRet = clReleaseMemObject(buffer_dst_int);
  bResult &= SilentCheck("clReleaseBuffer - buffer_dst_int", CL_SUCCESS, iRet);

  iRet = clReleaseKernel(kernel);
  bResult &= SilentCheck("clReleaseKernel - kernel", CL_SUCCESS, iRet);

  return validate_result(output, output_ref);
}

bool clAoSFieldScatterGather() {
  bool bResult = true;

  cl_platform_id platform = 0;
  cl_device_id device;
  cl_context context;

  // get platform
  cl_int iRet = clGetPlatformIDs(1, &platform, NULL);
  bResult &= SilentCheck("clGetPlatformIDs", CL_SUCCESS, iRet);
  if (!bResult) {
    return bResult;
  }

  // get device
  iRet = clGetDeviceIDs(platform, gDeviceType, 1, &device, NULL);
  bResult &= SilentCheck("clGetDeviceIDs", CL_SUCCESS, iRet);
  if (!bResult) {
    return bResult;
  }

  // create context
  context = clCreateContext(NULL, 1, &device, NULL, NULL, &iRet);
  bResult &= SilentCheck("clCreateContext", CL_SUCCESS, iRet);
  if (!bResult) {
    return bResult;
  }

  // create program with source
  cl_program program = clCreateProgramWithSource(
      context, 1, (const char **)&ocl_test_program, NULL, &iRet);
  bResult &= SilentCheck("clCreateProgramWithSource", CL_SUCCESS, iRet);
  if (!bResult) {
    return bResult;
  }

  // build program
  iRet = clBuildProgram(program, 1, &device, NULL, NULL, NULL);
  bResult &= SilentCheck("clBuildProgram", CL_SUCCESS, iRet);
  if (!bResult) {
    return bResult;
  }

  //
  // From here down it is the program execution implementation
  //
  TEST_STRUCT input_struct[BUFFERS_LENGTH] = STRUCT_BUFFER_INPUT;
  TEST_STRUCT_PACKED input_struct_packed[BUFFERS_LENGTH] = STRUCT_BUFFER_INPUT;
  float input_float[BUFFERS_LENGTH] = FLOAT_BUFFER_INPUT;
  int output_ref[BUFFERS_LENGTH];

  for (int i = 0; i < BUFFERS_LENGTH; ++i) {
    global_id = i;
    test_ref(output_ref, input_struct, input_float);
  }

  // Create queue
  cl_command_queue queue =
      clCreateCommandQueueWithProperties(context, device, NULL, &iRet);
  bResult &= SilentCheck("clCreateCommandQueueWithProperties - queue",
                         CL_SUCCESS, iRet);
  if (!bResult) {
    return bResult;
  }

  // Create buffers
  cl_mem buffer_src_struct =
      clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
                     BUFFERS_LENGTH * sizeof(TEST_STRUCT), input_struct, &iRet);
  bResult &= SilentCheck("clCreateBuffer - src struct", CL_SUCCESS, iRet);
  if (!bResult) {
    return bResult;
  }

  cl_mem buffer_src_struct_packed = clCreateBuffer(
      context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
      BUFFERS_LENGTH * sizeof(TEST_STRUCT_PACKED), input_struct_packed, &iRet);
  bResult &=
      SilentCheck("clCreateBuffer - src struct packed", CL_SUCCESS, iRet);
  if (!bResult) {
    return bResult;
  }

  // Create buffers
  cl_mem buffer_src_float =
      clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
                     BUFFERS_LENGTH * sizeof(float), input_float, &iRet);
  bResult &= SilentCheck("clCreateBuffer - src float", CL_SUCCESS, iRet);
  if (!bResult) {
    return bResult;
  }

  // execute kernel
  bResult &= execute_kernel("test", &context, &program, &queue,
                            &buffer_src_struct, &buffer_src_float, output_ref);
  bResult &=
      execute_kernel("test_packed", &context, &program, &queue,
                     &buffer_src_struct_packed, &buffer_src_float, output_ref);

  // Release objects

  iRet = clReleaseMemObject(buffer_src_struct);
  bResult &=
      SilentCheck("clReleaseBuffer - buffer_src_struct", CL_SUCCESS, iRet);

  iRet = clReleaseMemObject(buffer_src_float);
  bResult &=
      SilentCheck("clReleaseBuffer - buffer_src_float", CL_SUCCESS, iRet);

  iRet = clReleaseProgram(program);
  bResult &= SilentCheck("clReleaseProgram - program", CL_SUCCESS, iRet);

  iRet = clReleaseCommandQueue(queue);
  bResult &= SilentCheck("clReleaseCommandQueue - queue", CL_SUCCESS, iRet);

  iRet = clReleaseContext(context);
  bResult &= SilentCheck("clReleaseContext - context", CL_SUCCESS, iRet);

  return bResult;
}
