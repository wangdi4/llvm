// INTEL CONFIDENTIAL
//
// Copyright 2012 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

#include "CL/cl.h"
#include "FrameworkTest.h"
#include "cl_sys_info.h"
#include "cl_utils.h"
#include "gtest_wrapper.h"
#include "options.hpp"
#include <fstream>
#include <map>
#include <memory>
#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
#endif

cl_device_type gDeviceType = CL_DEVICE_TYPE_CPU;

/*******************************************************************************
 * clBuildProgram
 * -------------------
 * (1) get device ids
 * (2) create context
 * (3) create binary
 * (4) create program with source
 * (5) build program
 ******************************************************************************/

static cl_platform_id getPlatformIds() {
  cl_platform_id platform = 0;
  cl_int iRet = clGetPlatformIDs(1, &platform, NULL);
  Check("clGetPlatformIDs", CL_SUCCESS, iRet);
  return platform;
}

static cl_device_id *getDevices(cl_platform_id platform,
                                cl_uint *uiNumDevices) {
  // get device(s)
  cl_int iRet = clGetDeviceIDs(platform, gDeviceType, 0, NULL, uiNumDevices);
  std::unique_ptr<cl_device_id> devices;
  if (CL_SUCCESS != iRet) {
    printf("clGetDeviceIDs = %s\n", ClErrTxt(iRet));
    return nullptr;
  }
  // initialize arrays
  devices.reset(new cl_device_id[*uiNumDevices]);
  iRet =
      clGetDeviceIDs(platform, gDeviceType, *uiNumDevices, devices.get(), NULL);
  if (CL_SUCCESS != iRet) {
    printf("clGetDeviceIDs = %s\n", ClErrTxt(iRet));
    return nullptr;
  }
  return devices.release();
}

static bool runAndVerify(int numberOfIntParametersToTry, cl_context context,
                         const char *sample_large_parmam_kernel_pattern[],
                         cl_uint uiNumDevices, cl_device_id *pDevices,
                         cl_command_queue queue) {
  int i;
  cl_int iRet;
  std::unique_ptr<char> argumentLine, codeLines, programSrc;
  std::unique_ptr<cl_long> ptrLongs;
  long long result, expectedResult;
  cl_event event;
  cl_int event_status;

  bool bResult = true;
  cl_program prog;
  cl_kernel kernel;
  cl_mem mem;

  // These need to be inside to be deallocated automatically on each loop
  // iteration.
  printf("Trying a kernel with %d long arguments (%d bytes) and one cl_mem (%d "
         "bytes) for %d bytes total.\n",
         (int)numberOfIntParametersToTry,
         (int)(sizeof(cl_long) * numberOfIntParametersToTry),
         (int)(sizeof(cl_mem)),
         (int)(sizeof(cl_mem) + numberOfIntParametersToTry * sizeof(cl_long)));

  // Allocate memory for the program storage
  ptrLongs.reset(new cl_long[sizeof(cl_long) * numberOfIntParametersToTry]);
  cl_long *const longs = ptrLongs.get();
  argumentLine.reset(new char[sizeof(char) * numberOfIntParametersToTry * 32]);
  codeLines.reset(new char[sizeof(char) * numberOfIntParametersToTry * 32]);
  programSrc.reset(
      new char[sizeof(char) * (numberOfIntParametersToTry * 64 + 1024)]);
  char *const strSrc = programSrc.get();
  argumentLine.get()[0] = '\0';
  codeLines.get()[0] = '\0';
  programSrc.get()[0] = '\0';

  // Generate our results
  expectedResult = 0;
  for (i = 0; i < (int)numberOfIntParametersToTry; i++) {
    longs[i] = i;
    expectedResult += i;
  }

  // Build the program
  sprintf(argumentLine.get(), "%s", "long long0");
  sprintf(codeLines.get(), "%s", "result[0] += long0;");
  for (i = 1; i < (int)numberOfIntParametersToTry; i++) {
    sprintf(argumentLine.get(), "%s, long long%d", argumentLine.get(), i);
    sprintf(codeLines.get(), "%s\nresult[0] += long%d;", codeLines.get(), i);
  }

  /* Create a kernel to test with */
  sprintf(strSrc, sample_large_parmam_kernel_pattern[0], argumentLine.get(),
          codeLines.get());

  prog = clCreateProgramWithSource(
      context, 1, const_cast<const char **>(&strSrc), NULL, &iRet);
  bResult &= Check("clCreateProgramWithSource", CL_SUCCESS, iRet);
  assert(bResult && "clCreateProgramWithSource");

  std::unique_ptr<size_t> ptrSize(new size_t[uiNumDevices]);
  size_t *const szSize = ptrSize.get();
  for (unsigned int j = 0; j < uiNumDevices; j++)
    szSize[j] = -1;
  // get the binary, we should receive 0.
  iRet = clGetProgramInfo(prog, CL_PROGRAM_BINARY_SIZES,
                          sizeof(size_t) * uiNumDevices, szSize, NULL);
  bResult &=
      Check("clGetProgramInfo(CL_PROGRAM_BINARY_SIZES)", CL_SUCCESS, iRet);
  if (!bResult)
    clReleaseProgram(prog);
  for (unsigned int j = 0; j < uiNumDevices; j++)
    if (0 != szSize[j])
      clReleaseProgram(prog);
  iRet = clBuildProgram(prog, uiNumDevices, pDevices, NULL, NULL, NULL);
  bResult &= Check("clBuildProgram", CL_SUCCESS, iRet);
  if (!bResult)
    clReleaseProgram(prog);
  kernel = clCreateKernel(prog, "sample_test", &iRet);
  /* Try to set a large argument to the kernel */

  mem = clCreateBuffer(context, (cl_mem_flags)(CL_MEM_READ_WRITE),
                       sizeof(cl_long), NULL, &iRet);
  bResult &= Check("clCreateBuffer", CL_SUCCESS, iRet);
  if (!bResult) {
    clReleaseMemObject(mem);
    clReleaseKernel(kernel);
    clReleaseProgram(prog);
  }

  for (i = 0; i < (int)numberOfIntParametersToTry; i++) {
    iRet = clSetKernelArg(kernel, i, sizeof(cl_long), &longs[i]);
    if (!Check("clSetKernelArg", CL_SUCCESS, iRet))
      break;
  }
  if (CL_FAILED(iRet)) {
    clReleaseMemObject(mem);
    clReleaseKernel(kernel);
    clReleaseProgram(prog);
  }

  iRet = clSetKernelArg(kernel, i, sizeof(cl_mem), &mem);
  if (!Check("clSetKernelArg", CL_SUCCESS, iRet)) {
    clReleaseMemObject(mem);
    clReleaseKernel(kernel);
    clReleaseProgram(prog);
  }

  size_t globalDim[3] = {1, 1, 1}, localDim[3] = {1, 1, 1};
  iRet = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, globalDim, localDim, 0,
                                NULL, &event);
  if (!Check("clEnqueueNDRangeKernel", CL_SUCCESS, iRet)) {
    clReleaseMemObject(mem);
    clReleaseKernel(kernel);
    clReleaseProgram(prog);
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

  clReleaseMemObject(mem);
  clReleaseKernel(kernel);
  clReleaseProgram(prog);

  return result == expectedResult;
}

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
  std::unique_ptr<cl_device_id> ptrDevices;
  cl_device_id *pDevices;
  cl_context context;
  cl_int iRet;
  cl_platform_id platform = getPlatformIds();

  cl_context_properties prop[3] = {CL_CONTEXT_PLATFORM,
                                   (cl_context_properties)platform, 0};
  //
  // devices
  ptrDevices.reset(getDevices(platform, &uiNumDevices));
  pDevices = ptrDevices.get();
  //
  // context
  //
  context = clCreateContext(prop, uiNumDevices, pDevices, NULL, NULL, &iRet);
  bResult &= Check("clCreateContext", CL_SUCCESS, iRet);
  if (!bResult)
    return false;
  cl_command_queue queue = clCreateCommandQueueWithProperties(
      context, pDevices[0], 0 /*no properties*/, &iRet);
  bResult &=
      Check("clCreateCommandQueueWithProperties - queue", CL_SUCCESS, iRet);
  if (!bResult) {
    clReleaseContext(context);
    return false;
  }

  bool ret = runAndVerify(4, context, sample_large_parmam_kernel_pattern,
                          uiNumDevices, pDevices, queue);
  ret &= runAndVerify(5, context, sample_large_parmam_kernel_pattern,
                      uiNumDevices, pDevices, queue);

  clReleaseCommandQueue(queue);
  clReleaseContext(context);
  return ret;
}

static void setRecorderEnvVars() {
  const char *BE_PLUGIN = "OCLBACKEND_PLUGINS";
  const char *PREFIX = "recorder_test";
  const char *BE_PREFIX = "OCLRECORDER_DUMPPREFIX";

#ifdef _WIN32
  const char *recorderName = "OclRecorder.dll";
#else
  const char *recorderName = "libOclRecorder.so";
#endif

  std::string recorderFullName(
      Intel::OpenCL::Utils::GetFullModuleNameForLoad(recorderName));

#ifdef _WIN32
  _putenv_s(BE_PLUGIN, recorderFullName.c_str());
  _putenv_s(BE_PREFIX, PREFIX);
#else
  setenv(BE_PLUGIN, recorderFullName.c_str(), 1);
  setenv(BE_PREFIX, PREFIX, 1);
#endif
  std::cout << recorderFullName.c_str() << std::endl;
}

TEST(OclRecorder, dupKernels) {
  setRecorderEnvVars();

  if (!clBuildProgramMaxArgsTest()) {
    FAIL() << "===Failed==";
    return;
  }

  const char *const REC_FILE = "OclRecorderTest.recorder_test.10.cl";
  const char *const REC_FILE1 = "OclRecorderTest.recorder_test.2.20.cl";
  const char *const REC_CFG = "OclRecorderTest.recorder_test.cfg";
  const char *const REC_CFG1 = "OclRecorderTest.recorder_test.2.cfg";
  const char *const REC_DAT = "OclRecorderTest.recorder_test.sample_test.dat";
  const char *const REC_DAT1 =
      "OclRecorderTest.recorder_test.2.sample_test.dat";

  std::fstream file(REC_FILE);
  std::fstream cfg_file(REC_CFG);
  std::fstream dat_file(REC_DAT);
#if defined(OCLFRONTEND_PLUGINS)
  ASSERT_TRUE(file.good());
  ASSERT_TRUE(cfg_file.good());
  ASSERT_TRUE(dat_file.good());
#else
  ASSERT_FALSE(file.good());
  ASSERT_FALSE(cfg_file.good());
  ASSERT_FALSE(cfg_file.good());
#endif // defined(OCLFRONTEND_PLUGINS)
  file.close();
  cfg_file.close();
  dat_file.close();
  remove(REC_DAT);
  remove(REC_FILE);
  remove(REC_CFG);

  file.open(REC_FILE1);
  cfg_file.open(REC_CFG1);
  dat_file.open(REC_DAT1);
#if defined(OCLFRONTEND_PLUGINS)
  ASSERT_TRUE(file.good());
  ASSERT_TRUE(cfg_file.good());
  ASSERT_TRUE(dat_file.good());
#else
  ASSERT_FALSE(cfg_file.good());
  ASSERT_FALSE(file.good());
  ASSERT_FALSE(cfg_file.good());
#endif // defined(OCLFRONTEND_PLUGINS)
  file.close();
  cfg_file.close();
  dat_file.close();
  remove(REC_DAT1);
  remove(REC_FILE1);
  remove(REC_CFG1);
}

static bool runAndVerify_forLocalMem(cl_context context, cl_uint uiNumDevices,
                                     cl_device_id *pDevices,
                                     cl_command_queue queue) {
  cl_int iRet;
  std::unique_ptr<char> argumentLine, codeLines, programSrc;
  std::unique_ptr<cl_long> ptrLongs;
  long long result, expectedResult = 2; // expectedResult is the pre-calucated
                                        // result of execution of kernel
  cl_event event;
  cl_int event_status;

  bool bResult = true;
  cl_program prog;
  cl_kernel kernel;
  cl_mem mem0, mem1;

  const char *sample_large_parmam_kernel_pattern[] = {
      "__kernel void sample_test(__global long *in1, __local long *in2, "
      "__global long *result)\n"
      "{\n"
      "result[0] = 0;\n"
      "in1[0] = 1;\n"
      "in2[0] = 1;\n"
      "result[0] += in1[0]+in2[0];\n"
      "}\n"};

  // Allocate memory for the program storage
  programSrc.reset(new char[sizeof(char) * (1024)]);
  char *const strSrc = programSrc.get();
  programSrc.get()[0] = '\0';

  /* Create a kernel to test with */
  sprintf(strSrc, "%s", sample_large_parmam_kernel_pattern[0]);

  prog = clCreateProgramWithSource(
      context, 1, const_cast<const char **>(&strSrc), NULL, &iRet);
  bResult &= Check("clCreateProgramWithSource", CL_SUCCESS, iRet);
  assert(bResult && "clCreateProgramWithSource");

  std::unique_ptr<size_t> ptrSize(new size_t[uiNumDevices]);
  size_t *const szSize = ptrSize.get();
  for (unsigned int j = 0; j < uiNumDevices; j++)
    szSize[j] = -1;
  // get the binary, we should receive 0.
  iRet = clGetProgramInfo(prog, CL_PROGRAM_BINARY_SIZES,
                          sizeof(size_t) * uiNumDevices, szSize, NULL);
  bResult &=
      Check("clGetProgramInfo(CL_PROGRAM_BINARY_SIZES)", CL_SUCCESS, iRet);
  if (!bResult)
    clReleaseProgram(prog);
  for (unsigned int j = 0; j < uiNumDevices; j++)
    if (0 != szSize[j])
      clReleaseProgram(prog);
  iRet = clBuildProgram(prog, uiNumDevices, pDevices, NULL, NULL, NULL);
  bResult &= Check("clBuildProgram", CL_SUCCESS, iRet);
  if (!bResult)
    clReleaseProgram(prog);
  kernel = clCreateKernel(prog, "sample_test", &iRet);
  /* Try to set a large argument to the kernel */

  mem0 = clCreateBuffer(context, (cl_mem_flags)(CL_MEM_READ_WRITE),
                        sizeof(cl_long), NULL, &iRet);
  bResult &= Check("clCreateBuffer", CL_SUCCESS, iRet);
  if (!bResult) {
    clReleaseMemObject(mem0);
    clReleaseKernel(kernel);
    clReleaseProgram(prog);
  }

  mem1 = clCreateBuffer(context, (cl_mem_flags)(CL_MEM_READ_WRITE),
                        sizeof(cl_long), NULL, &iRet);
  bResult &= Check("clCreateBuffer", CL_SUCCESS, iRet);
  if (!bResult) {
    clReleaseMemObject(mem0);
    clReleaseMemObject(mem1);
    clReleaseKernel(kernel);
    clReleaseProgram(prog);
  }

  iRet = clSetKernelArg(kernel, 0, sizeof(cl_mem), &mem0);
  if (!Check("clSetKernelArg", CL_SUCCESS, iRet)) {
    clReleaseMemObject(mem0);
    clReleaseMemObject(mem1);
    clReleaseKernel(kernel);
    clReleaseProgram(prog);
  }

  iRet = clSetKernelArg(kernel, 1, sizeof(cl_long), NULL);
  if (!Check("clSetKernelArg", CL_SUCCESS, iRet)) {
    clReleaseMemObject(mem0);
    clReleaseMemObject(mem1);
    clReleaseKernel(kernel);
    clReleaseProgram(prog);
  }

  iRet = clSetKernelArg(kernel, 2, sizeof(cl_mem), &mem1);
  if (!Check("clSetKernelArg", CL_SUCCESS, iRet)) {
    clReleaseMemObject(mem0);
    clReleaseMemObject(mem1);
    clReleaseKernel(kernel);
    clReleaseProgram(prog);
  }

  size_t globalDim[3] = {1, 1, 1}, localDim[3] = {1, 1, 1};
  iRet = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, globalDim, localDim, 0,
                                NULL, &event);
  if (!Check("clEnqueueNDRangeKernel", CL_SUCCESS, iRet)) {
    clReleaseMemObject(mem0);
    clReleaseMemObject(mem1);
    clReleaseKernel(kernel);
    clReleaseProgram(prog);
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

  iRet = clEnqueueReadBuffer(queue, mem1, CL_TRUE, 0, sizeof(cl_long), &result,
                             0, NULL, NULL);
  Check("clEnqueueReadBuffer", CL_SUCCESS, iRet);

  clReleaseMemObject(mem0);
  clReleaseMemObject(mem1);
  clReleaseKernel(kernel);
  clReleaseProgram(prog);

  return result == expectedResult;
}

bool clBuildRunLocalMemTest() {
  bool bResult = true;

  printf("clBuildRunLocalMemTest\n");
  cl_uint uiNumDevices = 0;
  std::unique_ptr<cl_device_id> ptrDevices;
  cl_device_id *pDevices;
  cl_context context;
  cl_int iRet;
  cl_platform_id platform = getPlatformIds();

  cl_context_properties prop[3] = {CL_CONTEXT_PLATFORM,
                                   (cl_context_properties)platform, 0};
  //
  // devices
  ptrDevices.reset(getDevices(platform, &uiNumDevices));
  pDevices = ptrDevices.get();
  //
  // context
  //
  context = clCreateContext(prop, uiNumDevices, pDevices, NULL, NULL, &iRet);
  bResult &= Check("clCreateContext", CL_SUCCESS, iRet);
  if (!bResult)
    return false;
  cl_command_queue queue = clCreateCommandQueueWithProperties(
      context, pDevices[0], 0 /*no properties*/, &iRet);
  bResult &=
      Check("clCreateCommandQueueWithProperties - queue", CL_SUCCESS, iRet);
  if (!bResult) {
    clReleaseContext(context);
    return false;
  }

  bool ret = runAndVerify_forLocalMem(context, uiNumDevices, pDevices, queue);

  clReleaseCommandQueue(queue);
  clReleaseContext(context);
  return ret;
}

TEST(OclRecorder, recording_local_memory) {
  setRecorderEnvVars();
  if (!clBuildRunLocalMemTest()) {
    FAIL() << "===Failed==";
    return;
  }

  const char *const REC_FILE = "OclRecorderTest.recorder_test.3.30.cl";
  const char *const REC_CFG = "OclRecorderTest.recorder_test.3.cfg";
  const char *const REC_DAT = "OclRecorderTest.recorder_test.3.sample_test.dat";

  std::fstream file(REC_FILE);
  std::fstream cfg_file(REC_CFG);
  std::fstream dat_file(REC_DAT);
#if defined(OCLFRONTEND_PLUGINS)
  ASSERT_TRUE(file.good());
  ASSERT_TRUE(cfg_file.good());
  ASSERT_TRUE(dat_file.good());
#else
  ASSERT_FALSE(file.good());
  ASSERT_FALSE(cfg_file.good());
  ASSERT_FALSE(dat_file.good());
#endif // defined(OCLFRONTEND_PLUGINS)
  file.close();
  cfg_file.close();
  dat_file.close();
  remove(REC_FILE);
  remove(REC_CFG);
  remove(REC_DAT);
}

TEST(OclRecorder, recording_local_memory2) {
  setRecorderEnvVars();

  cl_int returnResult = CL_SUCCESS;
  cl_platform_id computePlatform;

  returnResult = clGetPlatformIDs(1, &computePlatform, NULL);
  ASSERT_EQ(CL_SUCCESS, returnResult) << "Function: clGetPlatformIDs";

  cl_device_id computeDevices;
  returnResult =
      clGetDeviceIDs(computePlatform, gDeviceType, 1, &computeDevices, NULL);
  ASSERT_EQ(CL_SUCCESS, returnResult) << "Function: clGetDeviceIDs";

  cl_context_properties prop[] = {
      CL_CONTEXT_PLATFORM,
      reinterpret_cast<cl_context_properties>(computePlatform), 0};
  cl_context context =
      clCreateContext(prop, 1, &computeDevices, NULL, NULL, &returnResult);
  ASSERT_EQ(CL_SUCCESS, returnResult) << "Function: clCreateContext";

  cl_command_queue queue = clCreateCommandQueueWithProperties(
      context, computeDevices, 0, &returnResult);
  ASSERT_EQ(CL_SUCCESS, returnResult)
      << "Function: clCreateCommandQueueWithProperties";

  const char *kernelSrc = "__kernel void sample_test(const int a, __local long "
                          "*b, __global long *c) { }";

  cl_program program =
      clCreateProgramWithSource(context, 1, &kernelSrc, NULL, &returnResult);
  ASSERT_EQ(CL_SUCCESS, returnResult) << "Function: clCreateProgramWithSource";

  returnResult = clBuildProgram(program, 1, &computeDevices, NULL, NULL, NULL);
  ASSERT_EQ(CL_SUCCESS, returnResult) << "Function: clBuildProgram";

  cl_kernel kernel = clCreateKernel(program, "sample_test", &returnResult);
  ASSERT_EQ(CL_SUCCESS, returnResult) << "Function: clCreateKernel";

  cl_mem globalMemBuf =
      clCreateBuffer(context, static_cast<cl_mem_flags>(CL_MEM_READ_WRITE),
                     sizeof(cl_long), NULL, &returnResult);
  ASSERT_EQ(CL_SUCCESS, returnResult) << "Function: clCreateBuffer";

  cl_int arg_a = 1;
  returnResult = clSetKernelArg(kernel, 0, sizeof(cl_int), &arg_a);
  ASSERT_EQ(CL_SUCCESS, returnResult) << "Function: clSetKernelArg";

  returnResult = clSetKernelArg(kernel, 1, sizeof(cl_long), NULL);
  ASSERT_EQ(CL_SUCCESS, returnResult) << "Function: clSetKernelArg";

  returnResult = clSetKernelArg(kernel, 2, sizeof(cl_mem), &globalMemBuf);
  ASSERT_EQ(CL_SUCCESS, returnResult) << "Function: clSetKernelArg";

  size_t globalDim[] = {1, 0, 0}, localDim[] = {1, 0, 0};

  returnResult = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, globalDim,
                                        localDim, 0, NULL, NULL);
  ASSERT_EQ(CL_SUCCESS, returnResult) << "Function: clEnqueueNDRangeKernel";

  returnResult = clFinish(queue);
  ASSERT_EQ(CL_SUCCESS, returnResult) << "Function: clFinish";

  clReleaseMemObject(globalMemBuf);
  clReleaseKernel(kernel);
  clReleaseProgram(program);
  clReleaseCommandQueue(queue);
  clReleaseContext(context);

  const char *const REC_CL_FILE_NAME = "OclRecorderTest.recorder_test.4.40.cl";
  const char *const REC_CFG_FILE_NAME = "OclRecorderTest.recorder_test.4.cfg";
  const char *const REC_DAT_FILE_NAME =
      "OclRecorderTest.recorder_test.4.sample_test.dat";

  std::fstream recordClFile(REC_CL_FILE_NAME);
  std::fstream recordCfgFile(REC_CFG_FILE_NAME);
  std::fstream recordDatFile(REC_DAT_FILE_NAME);

#if defined(OCLFRONTEND_PLUGINS)
  ASSERT_TRUE(recordClFile.good());
  ASSERT_TRUE(recordCfgFile.good());
  ASSERT_TRUE(recordDatFile.good());
#else
  ASSERT_FALSE(recordClFile.good());
  ASSERT_FALSE(recordCfgFile.good());
  ASSERT_FALSE(recordDatFile.good());
#endif // defined(OCLFRONTEND_PLUGINS)

  recordClFile.close();
  recordCfgFile.close();
  recordDatFile.close();

  remove(REC_CL_FILE_NAME);
  remove(REC_CFG_FILE_NAME);
  remove(REC_DAT_FILE_NAME);
}

// All runs with different local and global size.
// First and second runs with equal arguments, but third run have one another
// argument.
void RunKernel(const cl_device_id &computeDevices, const cl_context &context,
               const cl_command_queue &queue, const char *kernelSrc,
               cl_int returnResult, const char *kernel_name) {
  cl_program program =
      clCreateProgramWithSource(context, 1, &kernelSrc, NULL, &returnResult);
  ASSERT_EQ(CL_SUCCESS, returnResult) << "Function: clCreateProgramWithSource";

  returnResult = clBuildProgram(program, 1, &computeDevices, NULL, NULL, NULL);
  ASSERT_EQ(CL_SUCCESS, returnResult) << "Function: clBuildProgram";

  cl_kernel kernel = clCreateKernel(program, kernel_name, &returnResult);
  ASSERT_EQ(CL_SUCCESS, returnResult) << "Function: clCreateKernel";

  cl_image_format format;
  format.image_channel_order = CL_RGBA;
  format.image_channel_data_type = CL_UNORM_INT8;

  const size_t channels = 4;
  const size_t width = 50;
  const size_t height = 50;
  char buffer_for_image[channels * width * height];
  memset(buffer_for_image, 0, channels * width * height);

  cl_image_desc image_desc;
  memset(&image_desc, 0, sizeof(image_desc));
  image_desc.image_type = CL_MEM_OBJECT_IMAGE2D;
  image_desc.image_width = width;
  image_desc.image_height = height;
  image_desc.image_array_size = 1;
  image_desc.mem_object = NULL;
  image_desc.buffer = NULL;

  cl_mem img =
      clCreateImage(context, (CL_MEM_WRITE_ONLY | CL_MEM_COPY_HOST_PTR),
                    &format, &image_desc, buffer_for_image, &returnResult);
  ASSERT_EQ(CL_SUCCESS, returnResult) << "Function: clCreateImage";

  cl_long value_for_buffer = 5;
  cl_mem globalMemBuf =
      clCreateBuffer(context, (CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR),
                     sizeof(cl_long), &value_for_buffer, &returnResult);
  ASSERT_EQ(CL_SUCCESS, returnResult) << "Function: clCreateBuffer";

  cl_int arg_a = 1;
  returnResult = clSetKernelArg(kernel, 0, sizeof(cl_int), &arg_a);
  ASSERT_EQ(CL_SUCCESS, returnResult) << "Function: clSetKernelArg";

  returnResult = clSetKernelArg(kernel, 1, sizeof(cl_long), NULL);
  ASSERT_EQ(CL_SUCCESS, returnResult) << "Function: clSetKernelArg";

  returnResult = clSetKernelArg(kernel, 2, sizeof(cl_mem), &globalMemBuf);
  ASSERT_EQ(CL_SUCCESS, returnResult) << "Function: clSetKernelArg";

  returnResult = clSetKernelArg(kernel, 3, sizeof(cl_mem), &img);
  ASSERT_EQ(CL_SUCCESS, returnResult) << "Function: clSetKernelArg";

  size_t globalDim[] = {1, 0, 0}, localDim[] = {1, 0, 0};

  // First run kernel.
  returnResult = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, globalDim,
                                        localDim, 0, NULL, NULL);
  ASSERT_EQ(CL_SUCCESS, returnResult) << "Function: clEnqueueNDRangeKernel";

  globalDim[0] = 2;
  localDim[0] = 2;

  // Second run kernel. With another local and global dimensions.
  returnResult = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, globalDim,
                                        localDim, 0, NULL, NULL);
  ASSERT_EQ(CL_SUCCESS, returnResult) << "Function: clEnqueueNDRangeKernel";

  globalDim[0] = 3;
  localDim[0] = 3;

  arg_a = 2;
  returnResult = clSetKernelArg(kernel, 0, sizeof(cl_int), &arg_a);
  ASSERT_EQ(CL_SUCCESS, returnResult) << "Function: clSetKernelArg";

  // Third run kernel. With another local, global dimensions and argument value.
  returnResult = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, globalDim,
                                        localDim, 0, NULL, NULL);
  ASSERT_EQ(CL_SUCCESS, returnResult) << "Function: clEnqueueNDRangeKernel";

  returnResult = clFinish(queue);
  ASSERT_EQ(CL_SUCCESS, returnResult) << "Function: clFinish";

  clReleaseMemObject(globalMemBuf);
  clReleaseKernel(kernel);
  clReleaseProgram(program);
}

TEST(OclRecorder, equal_arguments) {
  setRecorderEnvVars();

  // Create general objects:
  cl_int returnResult = CL_SUCCESS;
  cl_platform_id computePlatform;

  returnResult = clGetPlatformIDs(1, &computePlatform, NULL);
  ASSERT_EQ(CL_SUCCESS, returnResult) << "Function: clGetPlatformIDs";

  cl_device_id computeDevices;
  returnResult =
      clGetDeviceIDs(computePlatform, gDeviceType, 1, &computeDevices, NULL);
  ASSERT_EQ(CL_SUCCESS, returnResult) << "Function: clGetDeviceIDs";

  cl_context_properties prop[] = {
      CL_CONTEXT_PLATFORM,
      reinterpret_cast<cl_context_properties>(computePlatform), 0};
  cl_context context =
      clCreateContext(prop, 1, &computeDevices, NULL, NULL, &returnResult);
  ASSERT_EQ(CL_SUCCESS, returnResult) << "Function: clCreateContext";

  cl_command_queue queue = clCreateCommandQueueWithProperties(
      context, computeDevices, 0, &returnResult);
  ASSERT_EQ(CL_SUCCESS, returnResult)
      << "Function: clCreateCommandQueueWithProperties";

  const char *kernelSrc = "__kernel void sample(const int a, __local long *b, "
                          "__global long *c, __read_only image2d_t img) { }";
  const char *kernel_name = "sample";
  const char *another_kernelSrc =
      "__kernel void another_sample(const int a, __local long *b, __global "
      "long *c, __read_only image2d_t img) { }";
  const char *another_kernel_name = "another_sample";

  // Create program, build program, create kernel twice:
  RunKernel(computeDevices, context, queue, kernelSrc, returnResult,
            kernel_name);
  RunKernel(computeDevices, context, queue, another_kernelSrc, returnResult,
            another_kernel_name);

  clReleaseCommandQueue(queue);
  clReleaseContext(context);

  // List of all file names that can exist:
  const char *const REC_CL1_FILE_NAME = "OclRecorderTest.recorder_test.5.50.cl";
  const char *const REC_CL2_FILE_NAME = "OclRecorderTest.recorder_test.6.60.cl";
  const char *const REC_CFG1_FILE_NAME = "OclRecorderTest.recorder_test.5.cfg";
  const char *const REC_CFG2_FILE_NAME = "OclRecorderTest.recorder_test.6.cfg";
  const char *const REC_DAT1_FILE_NAME =
      "OclRecorderTest.recorder_test.5.sample.dat";
  const char *const REC_DAT2_FILE_NAME =
      "OclRecorderTest.recorder_test.5.sample.1.dat";
  const char *const REC_DAT3_FILE_NAME =
      "OclRecorderTest.recorder_test.5.sample.2.dat";
  const char *const REC_DAT4_FILE_NAME =
      "OclRecorderTest.recorder_test.6.another_sample.dat";
  const char *const REC_DAT5_FILE_NAME =
      "OclRecorderTest.recorder_test.6.another_sample.1.dat";
  const char *const REC_DAT6_FILE_NAME =
      "OclRecorderTest.recorder_test.6.another_sample.2.dat";

  std::fstream recordCl1File(REC_CL1_FILE_NAME);
  std::fstream recordCl2File(REC_CL2_FILE_NAME);
  std::fstream recordCfgFile(REC_CFG1_FILE_NAME);
  std::fstream recordCfg2File(REC_CFG2_FILE_NAME);
  std::fstream recordDat1File(REC_DAT1_FILE_NAME);
  std::fstream recordDat2File(REC_DAT2_FILE_NAME);
  std::fstream recordDat3File(REC_DAT3_FILE_NAME);
  std::fstream recordDat4File(REC_DAT4_FILE_NAME);
  std::fstream recordDat5File(REC_DAT5_FILE_NAME);
  std::fstream recordDat6File(REC_DAT6_FILE_NAME);

#if defined(OCLFRONTEND_PLUGINS)

  // These files should exist:
  ASSERT_TRUE(recordCl1File.good());
  ASSERT_TRUE(recordCl2File.good());
  ASSERT_TRUE(recordCfgFile.good());
  ASSERT_TRUE(recordCfg2File.good());
  ASSERT_TRUE(recordDat1File.good());
  ASSERT_TRUE(recordDat3File.good());

  // These files should NOT exist:
  ASSERT_FALSE(recordDat2File.good());
  ASSERT_FALSE(recordDat4File.good());
  ASSERT_FALSE(recordDat5File.good());
  ASSERT_FALSE(recordDat6File.good());

  // Delete files that exist
  remove(REC_CL1_FILE_NAME);
  remove(REC_CL2_FILE_NAME);
  remove(REC_CFG1_FILE_NAME);
  remove(REC_CFG2_FILE_NAME);
  remove(REC_DAT1_FILE_NAME);
  remove(REC_DAT3_FILE_NAME);
#else
  ASSERT_FALSE(recordCl1File.good());
  ASSERT_FALSE(recordCl2File.good());
  ASSERT_FALSE(recordCfgFile.good());
  ASSERT_FALSE(recordCfg2File.good());
  ASSERT_FALSE(recordDat1File.good());
  ASSERT_FALSE(recordDat2File.good());
  ASSERT_FALSE(recordDat3File.good());
  ASSERT_FALSE(recordDat4File.good());
  ASSERT_FALSE(recordDat5File.good());
  ASSERT_FALSE(recordDat6File.good());
#endif // defined(OCLFRONTEND_PLUGINS)

  recordCl1File.close();
  recordCl2File.close();
  recordCfgFile.close();
  recordCfg2File.close();
  recordDat1File.close();
  recordDat2File.close();
  recordDat3File.close();
  recordDat4File.close();
  recordDat5File.close();
  recordDat6File.close();
}

TEST(OclRecorder, recording_half_ptr) {
  setRecorderEnvVars();

  cl_int returnResult = CL_SUCCESS;

  cl_platform_id computePlatform = 0;

  returnResult = clGetPlatformIDs(1, &computePlatform, NULL);
  ASSERT_EQ(CL_SUCCESS, returnResult) << "Function: clGetPlatformIDs";

  cl_context_properties prop[3] = {CL_CONTEXT_PLATFORM,
                                   (cl_context_properties)computePlatform, 0};

  cl_device_id computeDevices;
  cl_uint num_devices_available = 0;
  returnResult = clGetDeviceIDs(computePlatform, gDeviceType, 1,
                                &computeDevices, &num_devices_available);
  ASSERT_EQ(CL_SUCCESS, returnResult) << "Function: clGetDeviceIDs";
  ASSERT_TRUE(num_devices_available >= 1);

  cl_context context = clCreateContext(
      prop, num_devices_available, &computeDevices, NULL, NULL, &returnResult);
  ASSERT_EQ(CL_SUCCESS, returnResult) << "Function: clCreateContext";

  cl_command_queue queue = clCreateCommandQueueWithProperties(
      context, computeDevices, 0, &returnResult);
  ASSERT_EQ(CL_SUCCESS, returnResult)
      << "Function: clCreateCommandQueueWithProperties";

  const char *kernelSrc =
      "__kernel void test_half( __global float *p, __global half *f ) { size_t "
      "i = get_global_id(0); vstore_half_rtn( p[i], i, f ); }";
  const char *kernel_name = "test_half";

  cl_program program =
      clCreateProgramWithSource(context, 1, &kernelSrc, NULL, &returnResult);
  ASSERT_EQ(CL_SUCCESS, returnResult) << "Function: clCreateProgramWithSource";

  returnResult = clBuildProgram(program, 1, &computeDevices, NULL, NULL, NULL);
  ASSERT_EQ(CL_SUCCESS, returnResult) << "Function: clBuildProgram";

  cl_kernel kernel = clCreateKernel(program, kernel_name, &returnResult);
  ASSERT_EQ(CL_SUCCESS, returnResult) << "Function: clCreateKernel";

  cl_float value_for_float_buffer = 5.5;
  cl_mem globalMemFloatBuf =
      clCreateBuffer(context, (CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR),
                     sizeof(cl_float), &value_for_float_buffer, &returnResult);
  ASSERT_EQ(CL_SUCCESS, returnResult) << "Function: clCreateBuffer";

  cl_half value_for_half_buffer = (cl_half)10.5;
  cl_mem globalMemHalfBuf =
      clCreateBuffer(context, (CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR),
                     sizeof(cl_float), &value_for_half_buffer, &returnResult);
  ASSERT_EQ(CL_SUCCESS, returnResult) << "Function: clCreateBuffer";

  returnResult = clSetKernelArg(kernel, 0, sizeof(cl_mem), &globalMemFloatBuf);
  ASSERT_EQ(CL_SUCCESS, returnResult) << "Function: clSetKernelArg";

  returnResult = clSetKernelArg(kernel, 1, sizeof(cl_mem), &globalMemHalfBuf);
  ASSERT_EQ(CL_SUCCESS, returnResult) << "Function: clSetKernelArg";

  size_t globalDim[] = {1, 0, 0}, localDim[] = {1, 0, 0};

  // Run the kernel
  returnResult = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, globalDim,
                                        localDim, 0, NULL, NULL);
  ASSERT_EQ(CL_SUCCESS, returnResult) << "Function: clEnqueueNDRangeKernel";

  returnResult = clFinish(queue);
  ASSERT_EQ(CL_SUCCESS, returnResult) << "Function: clFinish";

  clReleaseMemObject(globalMemFloatBuf);
  clReleaseMemObject(globalMemHalfBuf);

  clReleaseKernel(kernel);
  clReleaseProgram(program);

  clReleaseCommandQueue(queue);
  clReleaseContext(context);

  // List of all expected file names:
  const char *const REC_CL_FILE_NAME = "OclRecorderTest.recorder_test.10.cl";
  const char *const REC_CFG_FILE_NAME = "OclRecorderTest.recorder_test.cfg";
  const char *const REC_DAT_FILE_NAME =
      "OclRecorderTest.recorder_test.test_half.dat";

  std::fstream recordClFile(REC_CL_FILE_NAME);
  std::fstream recordCfgFile(REC_CFG_FILE_NAME);
  std::fstream recordDatFile(REC_DAT_FILE_NAME);

#if defined(OCLFRONTEND_PLUGINS)

  // These files must exist:
  ASSERT_TRUE(recordClFile.good());
  ASSERT_TRUE(recordCfgFile.good());
  ASSERT_TRUE(recordDatFile.good());

  // Delete files that exist
  remove(REC_CL_FILE_NAME);
  remove(REC_CFG_FILE_NAME);
  remove(REC_DAT_FILE_NAME);
#else
  ASSERT_FALSE(recordClFile.good());
  ASSERT_FALSE(recordCfgFile.good());
  ASSERT_FALSE(recordDatFile.good());
#endif // defined(OCLFRONTEND_PLUGINS)

  recordClFile.close();
  recordCfgFile.close();
  recordDatFile.close();
}

CommandLineOption<std::string> deviceOption("--device_type");

int main(int argc, char **argv) {
  std::map<std::string, cl_device_type> clDeviceTypeMap;
  clDeviceTypeMap["cpu"] = CL_DEVICE_TYPE_CPU;
  clDeviceTypeMap["mic"] = CL_DEVICE_TYPE_ACCELERATOR;
  clDeviceTypeMap["fpga_fast_emu"] = CL_DEVICE_TYPE_ACCELERATOR;
  clDeviceTypeMap["gpu"] = CL_DEVICE_TYPE_GPU;
  clDeviceTypeMap["default"] = CL_DEVICE_TYPE_DEFAULT;
  clDeviceTypeMap["all"] = CL_DEVICE_TYPE_ALL;
  ::testing::InitGoogleTest(&argc, argv);
  if (argc > 1) {
    for (int i = 1; i < argc; i++)
      if (deviceOption.isMatch(argv[i])) {
        std::string deviceTypeStr = deviceOption.getValue(argv[i]);
        auto iter = clDeviceTypeMap.find(deviceTypeStr);
        if (iter == clDeviceTypeMap.end()) {
          printf("error: unkown device option: %s\n", deviceTypeStr.c_str());
          return 1;
        }
        printf("detected device %lu\n", iter->second);
        gDeviceType = iter->second;
      }
  }
  return RUN_ALL_TESTS();
}
