// INTEL CONFIDENTIAL
//
// Copyright 2020 Intel Corporation.
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

// The test checks --print-before/--print-after work correctly.

// The test can only run on Linux as fork() is used to check the output on
// stderr.
#if (!defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)) && !defined(_WIN32)

#include "CL/cl.h"
#include "FrameworkTest.h"
#include "gtest_wrapper.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>

extern cl_device_type gDeviceType;

static void InitProgram(cl_context &context, cl_device_id &device) {
  cl_platform_id platform = 0;
  cl_int rc;
  rc = clGetPlatformIDs(1, &platform, NULL);
  ASSERT_EQ(rc, CL_SUCCESS) << "Unable to get platform: " << ClErrTxt(rc);

  rc = clGetDeviceIDs(platform, gDeviceType, 1, &device, NULL);
  ASSERT_EQ(rc, CL_SUCCESS) << "Unable to get device: " << ClErrTxt(rc);

  cl_context_properties prop[3] = {CL_CONTEXT_PLATFORM,
                                   (cl_context_properties)platform, 0};
  context = clCreateContext(prop, 1, &device, NULL, NULL, &rc);
  ASSERT_EQ(rc, CL_SUCCESS) << "Unable to create context: " << ClErrTxt(rc);
}

static void EndProgram(cl_context context) {
  cl_int rc = clReleaseContext(context);
  ASSERT_EQ(rc, CL_SUCCESS) << "Unable to release context: " << ClErrTxt(rc);
}

static void BuildProgram(cl_context context, cl_device_id device) {
  static const char *ocl_test_program[] = {
      "__kernel void test_kernel_simple(){}"};

  cl_program program;
  cl_int rc;

  program = clCreateProgramWithSource(
      context, 1, (const char **)&ocl_test_program, NULL, &rc);
  ASSERT_EQ(rc, CL_SUCCESS) << "Unable to create program: " << ClErrTxt(rc);

  rc = clBuildProgram(program, 1, &device, NULL, NULL, NULL);
  ASSERT_EQ(rc, CL_SUCCESS) << "Unable to build program: " << ClErrTxt(rc);

  rc = clReleaseProgram(program);
  ASSERT_EQ(rc, CL_SUCCESS) << "Unable to release program: " << ClErrTxt(rc);
}

static void CheckOutput(int fd) {
  static const int TEST_NUM = 2;
  static char *refs[TEST_NUM] = {
      "*** IR Dump Before SimplifyCFGPass",
      "*** IR Dump After SYCLKernelWGLoopCreatorPass"};

  size_t size = 256;
  char *buf = (char *)malloc(size);
  ASSERT_NE(buf, nullptr) << "malloc: " << strerror(errno);

  FILE *fin = fdopen(fd, "r");
  ASSERT_NE(fin, nullptr) << "fdopen: " << strerror(errno);

  int i = 0;
  while (getline(&buf, &size, fin) != -1 && i < TEST_NUM) {
    if (strstr(buf, refs[i]) != NULL)
      i++;
  }
  // consume all output
  while (fread(buf, size, 1, fin))
    ;
  free(buf);
  fclose(fin);

  ASSERT_EQ(i, TEST_NUM) << "Expected string not found";
}

void cl_DumpIRBeforeAndAfterPasses() {
  int rc;

  rc = setenv(
      "CL_CONFIG_LLVM_OPTIONS",
      "-print-before=simplifycfg -print-after=sycl-kernel-wgloop-creator", 1);
  ASSERT_EQ(rc, 0) << "setenv: " << strerror(errno);

  int pipe_fds[2];
  rc = pipe(pipe_fds);
  ASSERT_EQ(rc, 0) << "pipe: " << strerror(errno);

  // Create a child process and attach its stderr to a pipe, so that we can
  // check the output.
  pid_t pid = fork();
  ASSERT_NE(pid, -1) << "fork: " << strerror(errno);

  if (pid == 0) {
    close(2);
    close(pipe_fds[0]);
    rc = dup2(pipe_fds[1], 2);
    ASSERT_NE(rc, -1) << "dup2: " << strerror(errno);
    cl_context context;
    cl_device_id device;
    ASSERT_NO_FATAL_FAILURE(InitProgram(context, device));
    ASSERT_NO_FATAL_FAILURE(BuildProgram(context, device));
    ASSERT_NO_FATAL_FAILURE(EndProgram(context));
    close(pipe_fds[1]);
    exit(0); // child process exits with 0 on success
  }

  close(pipe_fds[1]);
  ASSERT_NO_FATAL_FAILURE(CheckOutput(pipe_fds[0]));

  close(pipe_fds[0]);

  waitpid(pid, &rc, 0);
  ASSERT_EQ(rc, 0) << "Child process exited with non-zero status";

  rc = unsetenv("CL_CONFIG_LLVM_OPTIONS");
  ASSERT_EQ(rc, 0) << "Unable to unset environment variable" << strerror(errno);
}
#endif // (!defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)) && !defined(_WIN32)
