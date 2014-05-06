#include "CL/cl.h"
#include "gtest/gtest.h"
#include <stdio.h>
#include "FrameworkTest.h"
#include "cl_utils.h"
#include "cl_sys_info.h"
#include <memory>
#include <fstream>
#ifdef _WIN32
#include <windows.h>
#endif

/**************************************************************************************************
* clBuildProgram
* -------------------
* (1) get device ids
* (2) create context
* (3) create binary
* (4) create program with source
* (5) build program
**************************************************************************************************/

static cl_platform_id getPlatformIds(){
  cl_platform_id platform = 0;
  cl_int iRet = clGetPlatformIDs(1, &platform, NULL);
  assert( Check((wchar_t*)L"clGetPlatformIDs", CL_SUCCESS, iRet));
  return platform;
}

static cl_device_id* getDevices(cl_platform_id platform, cl_uint *uiNumDevices){
  // get device(s)
  cl_int iRet = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 0, NULL, uiNumDevices);
  std::auto_ptr<cl_device_id> devices;
  if (CL_SUCCESS != iRet){
    printf("clGetDeviceIDs = %s\n",ClErrTxt(iRet));
    return false;
  }
  // initialize arrays
  devices.reset(new cl_device_id[*uiNumDevices]);
  iRet = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, *uiNumDevices, devices.get(), NULL);
  if (CL_SUCCESS != iRet){
    printf("clGetDeviceIDs = %s\n",ClErrTxt(iRet));
    return false;
  }
  return devices.release();
}

static bool runAndVerify(int numberOfIntParametersToTry, cl_context context,
  const char* sample_large_parmam_kernel_pattern[], cl_uint uiNumDevices,
    cl_device_id *pDevices, cl_command_queue queue){
  int retVal, i;
  cl_int iRet;
  std::auto_ptr<char> argumentLine, codeLines, programSrc;
  std::auto_ptr<cl_long> ptrLongs;
  long long result, expectedResult;
  cl_event event;
  cl_int event_status;

  bool bResult = true;
  cl_program prog;
  cl_kernel kernel;
  cl_mem  mem;

  // These need to be inside to be deallocated automatically on each loop iteration.
  printf("Trying a kernel with %d long arguments (%d bytes) and one cl_mem (%d bytes) for %d bytes total.\n",
       (int)numberOfIntParametersToTry, (int)(sizeof(cl_long)*numberOfIntParametersToTry), (int)(sizeof(cl_mem)),
       (int)(sizeof(cl_mem)+numberOfIntParametersToTry*sizeof(cl_long)));

  // Allocate memory for the program storage
  ptrLongs.reset( new cl_long[sizeof(cl_long)*numberOfIntParametersToTry] );
  cl_long*const longs = ptrLongs.get();
  argumentLine.reset( new char[sizeof(char)*numberOfIntParametersToTry*32] );
  codeLines.reset( new char[sizeof(char)*numberOfIntParametersToTry*32] );
  programSrc.reset( new char[sizeof(char)*(numberOfIntParametersToTry*64+1024)]);
  char*const strSrc = programSrc.get();
  argumentLine.get()[0] = '\0';
  codeLines.get()[0] = '\0';
  programSrc.get()[0] = '\0';

  // Generate our results
  expectedResult = 0;
  for (i=0; i<(int)numberOfIntParametersToTry; i++) {
    longs[i] = i;
    expectedResult += i;
  }

  // Build the program
  sprintf(argumentLine.get(), "%s", "long long0");
  sprintf(codeLines.get(), "%s", "result[0] += long0;");
  for (i=1; i<(int)numberOfIntParametersToTry; i++) {
    sprintf(argumentLine.get(), "%s, long long%d", argumentLine.get(), i);
    sprintf(codeLines.get(), "%s\nresult[0] += long%d;", codeLines.get(), i);
  }

  /* Create a kernel to test with */
  sprintf( strSrc, sample_large_parmam_kernel_pattern[0], argumentLine.get(), codeLines.get());

  prog = clCreateProgramWithSource(context, 1, (const char**)&strSrc, NULL, &iRet);
  bResult &= Check((wchar_t*)L"clCreateProgramWithSource", CL_SUCCESS, iRet);
  assert(bResult && "clCreateProgramWithSource");

  std::auto_ptr<size_t> ptrSize (new size_t[uiNumDevices]);
  size_t*const szSize = ptrSize.get();
  for (unsigned int j = 0; j < uiNumDevices; j++)
    szSize[j] = -1;
  // get the binary, we should receive 0.
  iRet = clGetProgramInfo(prog, CL_PROGRAM_BINARY_SIZES, sizeof(size_t) * uiNumDevices, szSize, NULL);
  bResult &= Check((wchar_t*)L"clGetProgramInfo(CL_PROGRAM_BINARY_SIZES)", CL_SUCCESS, iRet);
  if (!bResult)
    clReleaseProgram(prog);
  for (unsigned int j = 0; j < uiNumDevices; j++)
    if (0 != szSize[j])
      clReleaseProgram(prog);
  iRet = clBuildProgram(prog, uiNumDevices, pDevices, NULL, NULL, NULL);
  bResult &= Check((wchar_t*)L"clBuildProgram", CL_SUCCESS, iRet);
  if (!bResult)
    clReleaseProgram(prog);
  kernel = clCreateKernel(prog, "sample_test", &iRet);
  /* Try to set a large argument to the kernel */
  retVal = 0;

  mem = clCreateBuffer(context, (cl_mem_flags)(CL_MEM_READ_WRITE), sizeof(cl_long), NULL, &iRet);
  bResult &= Check((wchar_t*)L"clCreateBuffer", CL_SUCCESS, iRet);
  if (!bResult) {
    clReleaseMemObject(mem);
    clReleaseKernel(kernel);
    clReleaseProgram(prog);
  }

  for (i=0; i<(int)numberOfIntParametersToTry; i++) {
    iRet = clSetKernelArg(kernel, i, sizeof(cl_long), &longs[i]);
    if (!Check((wchar_t*)L"clSetKernelArg", CL_SUCCESS, iRet))
      break;
  }
  if ( CL_FAILED(iRet)) {
    clReleaseMemObject(mem);
    clReleaseKernel(kernel);
    clReleaseProgram(prog);
  }

  iRet = clSetKernelArg(kernel, i, sizeof(cl_mem), &mem);
  if (!Check((wchar_t*)L"clSetKernelArg", CL_SUCCESS, iRet)) {
    clReleaseMemObject(mem);
    clReleaseKernel(kernel);
    clReleaseProgram(prog);
  }

  size_t globalDim[3]={1,1,1}, localDim[3]={1,1,1};
  iRet = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, globalDim, localDim, 0, NULL, &event);
  if (!Check((wchar_t*)L"clEnqueueNDRangeKernel", CL_SUCCESS, iRet)) {
    clReleaseMemObject(mem);
    clReleaseKernel(kernel);
    clReleaseProgram(prog);
  }

  // Verify that the event does not return an error from the execution
  iRet = clWaitForEvents(1, &event);
  Check((wchar_t*)L"clWaitForEvents", CL_SUCCESS, iRet);
  iRet = clGetEventInfo(event, CL_EVENT_COMMAND_EXECUTION_STATUS, sizeof(event_status), &event_status, NULL);
  Check((wchar_t*)L"clGetEventInfo", CL_SUCCESS, iRet);
  clReleaseEvent(event);
  if (event_status < 0)
    Check((wchar_t*)L"Kernel execution event returned error", CL_SUCCESS, iRet);

  iRet = clEnqueueReadBuffer(queue, mem, CL_TRUE, 0, sizeof(cl_long), &result, 0, NULL, NULL);
  Check((wchar_t*)L"clEnqueueReadBuffer", CL_SUCCESS, iRet);

  clReleaseMemObject(mem);
  clReleaseKernel(kernel);
  clReleaseProgram(prog);

  return result == expectedResult;
}

bool clBuildProgramMaxArgsTest(){
  bool bResult = true;
  const char *sample_large_parmam_kernel_pattern[] = {
    "__kernel void sample_test(%s, __global long *result)\n"
    "{\n"
    "result[0] = 0;\n"
    "%s"
    "\n"
    "}\n" };

  printf("clBuildProgramMaxArgsTest\n");
  cl_uint uiNumDevices = 0;
  std::auto_ptr<cl_device_id> ptrDevices;
  cl_device_id *pDevices;
  cl_context context;
  cl_int iRet;
  cl_platform_id platform = getPlatformIds();

  cl_context_properties prop[3] = { CL_CONTEXT_PLATFORM, (cl_context_properties)platform, 0 };
  //
  //devices
  ptrDevices.reset(getDevices(platform, &uiNumDevices));
  pDevices = ptrDevices.get();
  //
  //context
  //
  context = clCreateContext(prop, uiNumDevices, pDevices, NULL, NULL, &iRet);
  bResult &= Check((wchar_t*)L"clCreateContext", CL_SUCCESS, iRet);
  if (!bResult)
    return false;
  cl_command_queue queue = clCreateCommandQueue (context, pDevices[0], 0 /*no properties*/, &iRet);
  bResult &= Check((wchar_t*)L"clCreateCommandQueue - queue", CL_SUCCESS, iRet);
  if (!bResult){
    clReleaseContext(context);
    return false;
  }

  bool ret = runAndVerify(
      4,
      context,
      sample_large_parmam_kernel_pattern,
      uiNumDevices, pDevices, queue);
  ret &= runAndVerify(
      5,
      context,
      sample_large_parmam_kernel_pattern,
      uiNumDevices, pDevices, queue);

  clReleaseCommandQueue(queue);
  clReleaseContext(context);
  return ret;
}

static void setRecorderEnvVars(){
  const char* BE_PLUGIN = "OCLBACKEND_PLUGINS";
  const char* PREFIX    = "recorder_test";
  const char* BE_PREFIX = "OCLRECORDER_DUMPPREFIX";

#ifdef _WIN32
  const char* recorderName = "OclRecorder.dll";
#else
  const char* recorderName = "libOclRecorder.so";
#endif

  std::string recorderFullName(Intel::OpenCL::Utils::GetFullModuleNameForLoad(recorderName));

#ifdef _WIN32
  SetEnvironmentVariable(BE_PLUGIN, recorderFullName.c_str());
  SetEnvironmentVariable(BE_PREFIX, PREFIX);
#else
  setenv(BE_PLUGIN, recorderFullName.c_str(), 1);
  setenv(BE_PREFIX, PREFIX, 1);
#endif
  std::cout << recorderFullName.c_str() << std::endl;
}


TEST(OclRecorder, dupKernels){
  setRecorderEnvVars();

  if (!clBuildProgramMaxArgsTest()){
    FAIL() << "===Failed==";
    return;
  }

  const char*const REC_FILE = "OclRecorderTest.recorder_test.sample_test0.cl";
  const char*const REC_FILE1= "OclRecorderTest.recorder_test.2.sample_test0.1.cl";
  const char*const REC_CFG= "OclRecorderTest.recorder_test.cfg";
  const char*const REC_CFG1= "OclRecorderTest.recorder_test.2.cfg";
  const char*const REC_DAT = "OclRecorderTest.recorder_test.sample_test.dat";
  const char*const REC_DAT1 = "OclRecorderTest.recorder_test.2.sample_test.dat";

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
#endif// defined(OCLFRONTEND_PLUGINS)
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
#endif// defined(OCLFRONTEND_PLUGINS)
  file.close();
  cfg_file.close();
  dat_file.close();
  remove(REC_DAT1);
  remove(REC_FILE1);
  remove(REC_CFG1);
}


static bool runAndVerify_forLocalMem(cl_context context, cl_uint uiNumDevices,
    cl_device_id *pDevices, cl_command_queue queue){
  int retVal;
  cl_int iRet;
  std::auto_ptr<char> argumentLine, codeLines, programSrc;
  std::auto_ptr<cl_long> ptrLongs;
  long long result, expectedResult = 2; // expectedResult is the pre-calucated result of execution of kernel
  cl_event event;
  cl_int event_status;

  bool bResult = true;
  cl_program prog;
  cl_kernel kernel;
  cl_mem  mem0, mem1;

  const char *sample_large_parmam_kernel_pattern[] = {
    "__kernel void sample_test(__global long *in1, __local long *in2, __global long *result)\n"
    "{\n"
    "result[0] = 0;\n"
    "in1[0] = 1;\n"
    "in2[0] = 1;\n"
    "result[0] += in1[0]+in2[0];\n"
    "}\n" };

  // Allocate memory for the program storage
  programSrc.reset( new char[sizeof(char)*(1024)]);
  char*const strSrc = programSrc.get();
  programSrc.get()[0] = '\0';

  /* Create a kernel to test with */
  sprintf( strSrc, sample_large_parmam_kernel_pattern[0]);

  prog = clCreateProgramWithSource(context, 1, (const char**)&strSrc, NULL, &iRet);
  bResult &= Check((wchar_t*)L"clCreateProgramWithSource", CL_SUCCESS, iRet);
  assert(bResult && "clCreateProgramWithSource");

  std::auto_ptr<size_t> ptrSize (new size_t[uiNumDevices]);
  size_t*const szSize = ptrSize.get();
  for (unsigned int j = 0; j < uiNumDevices; j++)
    szSize[j] = -1;
  // get the binary, we should receive 0.
  iRet = clGetProgramInfo(prog, CL_PROGRAM_BINARY_SIZES, sizeof(size_t) * uiNumDevices, szSize, NULL);
  bResult &= Check((wchar_t*)L"clGetProgramInfo(CL_PROGRAM_BINARY_SIZES)", CL_SUCCESS, iRet);
  if (!bResult)
    clReleaseProgram(prog);
  for (unsigned int j = 0; j < uiNumDevices; j++)
    if (0 != szSize[j])
      clReleaseProgram(prog);
  iRet = clBuildProgram(prog, uiNumDevices, pDevices, NULL, NULL, NULL);
  bResult &= Check((wchar_t*)L"clBuildProgram", CL_SUCCESS, iRet);
  if (!bResult)
    clReleaseProgram(prog);
  kernel = clCreateKernel(prog, "sample_test", &iRet);
  /* Try to set a large argument to the kernel */
  retVal = 0;

  mem0 = clCreateBuffer(context, (cl_mem_flags)(CL_MEM_READ_WRITE), sizeof(cl_long), NULL, &iRet);
  bResult &= Check((wchar_t*)L"clCreateBuffer", CL_SUCCESS, iRet);
  if (!bResult) {
    clReleaseMemObject(mem0);
    clReleaseKernel(kernel);
    clReleaseProgram(prog);
  }


  mem1 = clCreateBuffer(context, (cl_mem_flags)(CL_MEM_READ_WRITE), sizeof(cl_long), NULL, &iRet);
  bResult &= Check((wchar_t*)L"clCreateBuffer", CL_SUCCESS, iRet);
  if (!bResult) {
    clReleaseMemObject(mem0);
    clReleaseMemObject(mem1);
    clReleaseKernel(kernel);
    clReleaseProgram(prog);
  }

  iRet = clSetKernelArg(kernel, 0, sizeof(cl_mem), &mem0);
  if (!Check((wchar_t*)L"clSetKernelArg", CL_SUCCESS, iRet)) {
    clReleaseMemObject(mem0);
    clReleaseMemObject(mem1);
    clReleaseKernel(kernel);
    clReleaseProgram(prog);
  }

  iRet = clSetKernelArg(kernel, 1, sizeof(cl_long), NULL);
  if (!Check((wchar_t*)L"clSetKernelArg", CL_SUCCESS, iRet)) {
    clReleaseMemObject(mem0);
    clReleaseMemObject(mem1);
    clReleaseKernel(kernel);
    clReleaseProgram(prog);
  }

  iRet = clSetKernelArg(kernel, 2, sizeof(cl_mem), &mem1);
  if (!Check((wchar_t*)L"clSetKernelArg", CL_SUCCESS, iRet)) {
    clReleaseMemObject(mem0);
    clReleaseMemObject(mem1);
    clReleaseKernel(kernel);
    clReleaseProgram(prog);
  }

  size_t globalDim[3]={1,1,1}, localDim[3]={1,1,1};
  iRet = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, globalDim, localDim, 0, NULL, &event);
  if (!Check((wchar_t*)L"clEnqueueNDRangeKernel", CL_SUCCESS, iRet)) {
    clReleaseMemObject(mem0);
    clReleaseMemObject(mem1);
    clReleaseKernel(kernel);
    clReleaseProgram(prog);
  }

  // Verify that the event does not return an error from the execution
  iRet = clWaitForEvents(1, &event);
  Check((wchar_t*)L"clWaitForEvents", CL_SUCCESS, iRet);
  iRet = clGetEventInfo(event, CL_EVENT_COMMAND_EXECUTION_STATUS, sizeof(event_status), &event_status, NULL);
  Check((wchar_t*)L"clGetEventInfo", CL_SUCCESS, iRet);
  clReleaseEvent(event);
  if (event_status < 0)
    Check((wchar_t*)L"Kernel execution event returned error", CL_SUCCESS, iRet);

  iRet = clEnqueueReadBuffer(queue, mem1, CL_TRUE, 0, sizeof(cl_long), &result, 0, NULL, NULL);
  Check((wchar_t*)L"clEnqueueReadBuffer", CL_SUCCESS, iRet);

  clReleaseMemObject(mem0);
  clReleaseMemObject(mem1);
  clReleaseKernel(kernel);
  clReleaseProgram(prog);

  return result == expectedResult;
}


bool clBuildRunLocalMemTest(){
  bool bResult = true;

  printf("clBuildRunLocalMemTest\n");
  cl_uint uiNumDevices = 0;
  std::auto_ptr<cl_device_id> ptrDevices;
  cl_device_id *pDevices;
  cl_context context;
  cl_int iRet;
  cl_platform_id platform = getPlatformIds();

  cl_context_properties prop[3] = { CL_CONTEXT_PLATFORM, (cl_context_properties)platform, 0 };
  //
  //devices
  ptrDevices.reset(getDevices(platform, &uiNumDevices));
  pDevices = ptrDevices.get();
  //
  //context
  //
  context = clCreateContext(prop, uiNumDevices, pDevices, NULL, NULL, &iRet);
  bResult &= Check((wchar_t*)L"clCreateContext", CL_SUCCESS, iRet);
  if (!bResult)
    return false;
  cl_command_queue queue = clCreateCommandQueue (context, pDevices[0], 0 /*no properties*/, &iRet);
  bResult &= Check((wchar_t*)L"clCreateCommandQueue - queue", CL_SUCCESS, iRet);
  if (!bResult){
    clReleaseContext(context);
    return false;
  }

  bool ret = runAndVerify_forLocalMem(context,
      uiNumDevices, pDevices, queue);

  clReleaseCommandQueue(queue);
  clReleaseContext(context);
  return ret;
}


TEST(OclRecorder, recording_local_memory){
  setRecorderEnvVars();
  if (!clBuildRunLocalMemTest()){
    FAIL() << "===Failed==";
    return;
  }

  const char*const REC_FILE = "OclRecorderTest.recorder_test.3.sample_test0.2.cl";
  const char*const REC_CFG= "OclRecorderTest.recorder_test.3.cfg";
  const char*const REC_DAT= "OclRecorderTest.recorder_test.3.sample_test.dat";

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
#endif// defined(OCLFRONTEND_PLUGINS)
  file.close();
  cfg_file.close();
  dat_file.close();
  remove(REC_FILE);
  remove(REC_CFG);
  remove(REC_DAT);
}

TEST(OclRecorder, recording_local_memory2)
{
    setRecorderEnvVars();

    cl_int returnResult = CL_SUCCESS;
    cl_platform_id computePlatform;

    returnResult = clGetPlatformIDs(1, &computePlatform, NULL);
    ASSERT_EQ(CL_SUCCESS, returnResult) << "Function: clGetPlatformIDs";

    cl_device_id computeDevices;
    returnResult = clGetDeviceIDs(computePlatform, CL_DEVICE_TYPE_CPU, 1, &computeDevices, NULL);
    ASSERT_EQ(CL_SUCCESS, returnResult) << "Function: clGetDeviceIDs";

    cl_context_properties prop[] = { CL_CONTEXT_PLATFORM, reinterpret_cast<cl_context_properties>(computePlatform), 0 };
    cl_context context = clCreateContext(prop, 1, &computeDevices, NULL, NULL, &returnResult);
    ASSERT_EQ(CL_SUCCESS, returnResult) << "Function: clCreateContext";

    cl_command_queue queue = clCreateCommandQueue(context, computeDevices, 0, &returnResult);
    ASSERT_EQ(CL_SUCCESS, returnResult) << "Function: clCreateCommandQueue";

    const char *kernelSrc = "__kernel void sample_test(const int a, __local long *b, __global long *c) { }";

    cl_program program = clCreateProgramWithSource(context, 1, &kernelSrc, NULL, &returnResult);
    ASSERT_EQ(CL_SUCCESS, returnResult) << "Function: clCreateProgramWithSource";

    returnResult = clBuildProgram(program, 1, &computeDevices, NULL, NULL, NULL);
    ASSERT_EQ(CL_SUCCESS, returnResult) << "Function: clBuildProgram";

    cl_kernel kernel = clCreateKernel(program, "sample_test", &returnResult);
    ASSERT_EQ(CL_SUCCESS, returnResult) << "Function: clCreateKernel";

    cl_mem globalMemBuf = clCreateBuffer(context, static_cast<cl_mem_flags>(CL_MEM_READ_WRITE), sizeof(cl_long), NULL, &returnResult);
    ASSERT_EQ(CL_SUCCESS, returnResult) << "Function: clCreateBuffer";

    cl_int arg_a = 1;
    returnResult = clSetKernelArg(kernel, 0, sizeof(cl_int), &arg_a);
    ASSERT_EQ(CL_SUCCESS, returnResult) << "Function: clSetKernelArg";

    returnResult = clSetKernelArg(kernel, 1, sizeof(cl_long), NULL);
    ASSERT_EQ(CL_SUCCESS, returnResult) << "Function: clSetKernelArg";

    returnResult = clSetKernelArg(kernel, 2, sizeof(cl_mem), &globalMemBuf);
    ASSERT_EQ(CL_SUCCESS, returnResult) << "Function: clSetKernelArg";

    size_t globalDim[]={ 1, 0, 0 }, localDim[]={ 1, 0, 0 };

    returnResult = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, globalDim, localDim, 0, NULL, NULL);
    ASSERT_EQ(CL_SUCCESS, returnResult) << "Function: clEnqueueNDRangeKernel";

    returnResult = clFinish(queue);
    ASSERT_EQ(CL_SUCCESS, returnResult) << "Function: clFinish";

    clReleaseMemObject(globalMemBuf);
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);

    const char* const REC_CL_FILE_NAME = "OclRecorderTest.recorder_test.4.sample_test0.3.cl";
    const char* const REC_CFG_FILE_NAME = "OclRecorderTest.recorder_test.4.cfg";
    const char* const REC_DAT_FILE_NAME = "OclRecorderTest.recorder_test.4.sample_test.dat";

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
#endif// defined(OCLFRONTEND_PLUGINS)

    recordClFile.close();
    recordCfgFile.close();
    recordDatFile.close();

    remove(REC_CL_FILE_NAME);
    remove(REC_CFG_FILE_NAME);
    remove(REC_DAT_FILE_NAME);
}

int main(int argc, char** argv){
   ::testing::InitGoogleTest(&argc, argv);
   return RUN_ALL_TESTS();
}
