// Copyright (c) 2014 Intel Corporation
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

#include "CL/cl.h"
#include "cl_types.h"
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include "FrameworkTest.h"

#ifdef _WIN32
#include <windows.h>
#endif

extern cl_device_type gDeviceType;

static const char *ocl_test_program[] = {\
"__kernel void test_kernel_simple(__global int* pBuff0, __global int* pBuff1, "
"                                 __global int* pBuff2)"
"{"
" size_t id = get_global_id(0);"
" pBuff2[id] = pBuff1[id] + pBuff0[id];"
"}"
};

static bool InitProgram(cl_context &context, cl_device_id &pDevice)
{
  printf("cl_GenStats\n");
  cl_uint uiNumDevices = 0;

  cl_platform_id platform = 0;

  cl_int iRet = clGetPlatformIDs(1, &platform, NULL);
  if (CL_SUCCESS != iRet)
  {
    printf("clGetPlatformIDs = %s\n",ClErrTxt(iRet));
    return false;
  }

  cl_context_properties prop[3] = { CL_CONTEXT_PLATFORM, (cl_context_properties)platform, 0 };

  // get device(s)
  iRet = clGetDeviceIDs(platform, gDeviceType, 0, NULL, &uiNumDevices);
  if (CL_SUCCESS != iRet)
  {
    printf("clGetDeviceIDs = %s\n",ClErrTxt(iRet));
    return false;
  }

  // initialize arrays

  iRet = clGetDeviceIDs(platform, gDeviceType, 1, &pDevice, NULL);
  if (CL_SUCCESS != iRet)
  {
    printf("clGetDeviceIDs = %s\n",ClErrTxt(iRet));
    return false;
  }

  // create context
  context = clCreateContext(prop, 1, &pDevice, NULL, NULL, &iRet);
  if (CL_SUCCESS != iRet)
  {
    printf("clCreateContext = %s\n",ClErrTxt(iRet));
    return false;
  }
  printf("Initialized context\n");
  return true;
}

static bool BuildProgramTest(cl_context context, cl_device_id device)
{
  cl_program program;
  bool res = true;
  cl_int iRes = CL_SUCCESS;

  program = clCreateProgramWithSource(context, 1,
      (const char**)&ocl_test_program, NULL, &iRes);
  if (CL_SUCCESS != iRes)
  {
    printf("clCreateProgramWithSource = %s\n",ClErrTxt(iRes));
    return false;
  }

  iRes = clBuildProgram(program, 1, &device, NULL, NULL, NULL);
  if (CL_SUCCESS != iRes)
  {
    printf("clBuildProgram = %s\n",ClErrTxt(iRes));
    res = false;
  }
  else
    printf ("Built program GenStats successfully\n");

  clReleaseProgram(program);
  return res;
}

static bool CheckStats () {

  ifstream testRes;
  const char *fname = "StatFile1.ll";

  testRes.open(fname);
  if (!testRes.good()) {
    printf ("ERROR: Failed to open IR file %s\n", fname);
    return false;
  }

  bool res = false;
  string str;

  while (getline(testRes, str).good()) {
    if (str.length() > 0 && *str.begin() == '!' &&
        str.find("= !{!\"Vectorizer@CanVect\", i32 1}")
          != string::npos) {
      res = true;
      break;
    }
  }

  if (res == false)
    printf ("ERROR: Expected stat not found in file\n");

  testRes.close();

  if (remove(fname)) {
    printf ("ERROR: failed to remove stat file\n");
    res = false;
  }

  testRes.close();
  return res;
}

static void EndProgram(cl_context context) {
  // Release objects
  clReleaseContext(context);
}

#if defined(_WIN32)
#define SETENV(NAME,VALUE)      (SetEnvironmentVariableA(NAME,VALUE) != 0)
#define UNSETENV(NAME)          (SetEnvironmentVariableA(NAME,NULL) != 0)
#else
#define SETENV(NAME,VALUE)      (setenv(NAME,VALUE,1) == 0)
#define UNSETENV(NAME)          (unsetenv(NAME) == 0)
#endif

bool cl_GenStats() {
  cl_context context;
  cl_device_id device;

  bool res = true;

  if (!SETENV("VOLCANO_STATS", "") ||
      !SETENV("VOLCANO_IR_FILE_BASE_NAME", "StatFile")) {
    printf ("ERROR: GenStat: Can't set stat environment variables. Test FAILED\n");
    return false;
  }

  if ((res = InitProgram(context, device)) == true) {
    res = BuildProgramTest(context, device) && CheckStats();
    printf ("GenStats: %s\n", res ? "SUCCESS" : "FAIL");
    EndProgram(context);
  }

  if (!UNSETENV("VOLCANO_STATS") ||
      !UNSETENV("VOLCANO_IR_FILE_BASE_NAME")) {
    printf ("ERROR GenStat: Can't unset VOLCANO_STATS environment variable. Test FAILED\n");
    return false;
  }
  return res;
}

