#include "CL/cl.h"
#include "cl_types.h"
#include <iostream>
#include <fstream>
#include <stdio.h>
#include "FrameworkTest.h"
#include "mic_dev_limits.h"

extern cl_device_type gDeviceType;

// build program options that set APF level and dumped asm file name
static const char *options[] = {
    "-dump-opt-asm=\"./test0.s\" -auto-prefetch-level=0",
    "-dump-opt-asm=\"./test1.s\" -auto-prefetch-level=1",
    "-dump-opt-asm=\"./test2.s\" -auto-prefetch-level=2",
    "-dump-opt-asm=\"./test3.s\" -auto-prefetch-level=3"
};

// asm file names
static const char *fnames[] = {
    "test0.s",
    "test1.s",
    "test2.s",
    "test3.s"
};

// expected number of prefetch and gather prefetch instructions for each prefetch level
static struct expected_s{
  int numPF;
  int numGPF;
} expected[] = {
    {0, 0}, {10, 0}, {14, 0}, {14, 4}
};

static const char *ocl_test_program[] = {\
"__kernel void test_kernel_APFlevel(__global int* pBuff0, __global int* pBuff1, __global int* pBuff2,"\
"                                   __global int* pBuff3, __global int* pBuff4, __global int* pBuff5,"\
"                                   __global int* pBuff6)"\
"{"\
" size_t id = get_global_id(0);"\
" pBuff2[id] = pBuff1[id] + pBuff0[id];"\
" if (pBuff2[id] > 0)"
"  pBuff3[id] = pBuff4[id];"
" pBuff2[id] += pBuff5[pBuff1[id]];"
" pBuff6[pBuff1[id]] = pBuff0[id];"
"}"
};

static bool InitProgram(cl_context &context, cl_device_id &pDevice)
{
  printf("cl_APFLevlForce\n");
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
  iRet = clGetDeviceIDs(platform, CL_DEVICE_TYPE_ACCELERATOR, 0, NULL, &uiNumDevices);
  if (CL_SUCCESS != iRet)
  {
    printf("clGetDeviceIDs = %s\n",ClErrTxt(iRet));
    return false;
  }

  // initialize arrays

  iRet = clGetDeviceIDs(platform, CL_DEVICE_TYPE_ACCELERATOR, 1, &pDevice, NULL);
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

static bool BuildProgramTest(cl_context context, cl_device_id device, int level)
{
  cl_program program;
  bool res = true;
  cl_int iRes = CL_SUCCESS;

  program = clCreateProgramWithSource(context, 1, (const char**)&ocl_test_program, NULL, &iRes);
  if (CL_SUCCESS != iRes)
  {
    printf("clCreateProgramWithSource = %s\n",ClErrTxt(iRes));
    return false;
  }

  iRes = clBuildProgram(program, 1, &device, options[level], NULL, NULL);
  if (CL_SUCCESS != iRes)
  {
    printf("clBuildProgram = %s\n",ClErrTxt(iRes));
    res = false;
  }
  else
    printf ("Built program APF level %d successfully\n", level);

  clReleaseProgram(program);
  return res;
}

static bool GrepCount(const char *fname, int &numPref, int &numGPf) {
  ifstream testRes;
  bool res = true;
  numPref = 0;
  numGPf = 0;

  testRes.open(fname);
  if (!testRes.good()) {
    printf ("failed to open asm file %s\n", fname);
    return false;
  }

  string str;
  while (getline(testRes, str).good()) {
    if (str.find("pref") != string::npos)
      numPref++;
    else if (str.find("pf") != string::npos)
      numGPf++;
  }

  testRes.close();
  return res;
}

static bool CheckCounts (const char *fname, int expectedPF, int expectedGPF) {
  int numPF, numGPF;
  bool res = true;
  if (!GrepCount(fname, numPF, numGPF))
    return false;
  if (numPF != expectedPF) {
    printf ("Incorrect number of prefetches in %s, expected %d found %d\n", fname, expectedPF, numPF);
    res = false;
  }
  if (numGPF != expectedGPF) {
    printf ("Incorrect number of gather/scatter prefetches in %s, expected %d found %d\n", fname, expectedGPF, numGPF);
    res = false;
  }
  return res;
}

static void EndProgram(cl_context context) {
  // Release objects
  clReleaseContext(context);
}

bool cl_APFLevelForce() {
  cl_context context;
  cl_device_id device;

  if (gDeviceType != CL_DEVICE_TYPE_ACCELERATOR) {
    printf ("APFLevelForce is not executed on non MIC device: SUCCESS\n");
    return true;
  }

  if (!InitProgram(context, device))
    return false;
  bool finalRes = true;
  for (int i = APFLEVEL_MIN; i <= APFLEVEL_MAX; i++) {
    bool res = BuildProgramTest(context, device, i) && CheckCounts(fnames[i], expected[i].numPF, expected[i].numGPF);
    printf ("APFLevelForce level %d: %s\n", i, res ? "SUCCESS" : "FAIL");
    finalRes &= res;
  }
  EndProgram(context);
  return finalRes;
}

